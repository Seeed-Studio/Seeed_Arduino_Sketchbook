#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "get_hist_weather.h"
#include "gui.h"
#include "model_Conv1D.h"
#include "Seeed_BME280.h"
#include <Wire.h>
#include <MemoryFree.h>

BME280 bme280;

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output_type = nullptr;
TfLiteTensor* output_precip = nullptr;

constexpr int kTensorArenaSize = 1024*10;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

// A buffer holding the last 24 sets of 3-channel values
//float save_data[72]; 
// Most recent position in the save_data buffer
int begin_index = 0;
unsigned long previousMillis = 0;
const long interval = 1000*60*60; //1 hour


void collect_data() {
  
    stack.unshift(bme280.getPressure() / 100000.0);
    stack.unshift(bme280.getHumidity() / 100.0);
    stack.unshift(bme280.getTemperature() / 60.0);

    print_buffer();
    
    }

void run_inference() {
  LogTFT("Running inference\n");
  delay(100);
  
  for (byte i = 0; i < 72; i = i + 1) {
        input->data.int8[i] = stack[i] / input->params.scale + input->params.zero_point;
  }

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  
  if (invoke_status != kTfLiteOk) {
    Log("Invoke failed\n");
    return;
  }

  // Obtain the quantized output from model's output tensor
  float y_type[4];

  // Dequantize the output from integer to floating-point
  int8_t y_precip_q = output_precip->data.int8[0];
  float y_precip = (y_precip_q - output_precip->params.zero_point) * output_precip->params.scale;
    
  Serial.print(F("Precip: "));
  Serial.print(y_precip, 4);
  Serial.print(F("\t"));
  Serial.print(F("Type: "));
  for (byte i = 0; i < 4; i = i + 1) {
    y_type[i] = (output_type->data.int8[i] - output_type->params.zero_point) * output_type->params.scale;
    Serial.print(y_type[i], 4);
    Serial.print(F(" "));
  }
  Serial.print(F("\n"));

  uint8_t maxI = 0;
  for(int i = 0; i < 4; i++) 
    if(y_type[i] > y_type[maxI])
       maxI = i;

  update_screen(stack[69]*60, stack[71]*1000, stack[70], y_precip*100, maxI);
  LogTFT("Waiting\n");
}

void print_buffer() {
  LogTFT("Data received\n");
  Serial.print(F("Data buffer content:\n"));
  for (byte i = 1; i < 73; i = i + 1) {
    Serial.print(stack[i-1], 4);
    Serial.print(F(" "));
    delay(10);
    if (i % 3 == 0) { Serial.print(F("\n")); }
  }
  Serial.print(F("\n"));
}
void setup() {

  Serial.begin(115200);
  //while (!Serial) { delay(10); }
  if (!bme280.init()) { LogTFT("Sensor init error!\n"); while (1) {} }
  setupLVGL();
  update_screen(bme280.getTemperature(),  bme280.getPressure()/100, bme280.getHumidity(), 0.0, 0);
  setupWIFI();
  getCurrent();
  getHistorical();
  LogTFT("Free memory available: %d\n", freeMemory()/1024);
  delay(250);
  print_buffer();
  
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(Conv1D_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) { 
    Log("Model provided is schema version %d not equal to supported version %d. \n", model->version(), TFLITE_SCHEMA_VERSION);
    while (1) {}
  }

  static tflite::MicroMutableOpResolver<8> resolver;
  resolver.AddConv2D();
  resolver.AddFullyConnected();
  resolver.AddAveragePool2D();
  resolver.AddSoftmax();
  resolver.AddRelu();
  resolver.AddExpandDims();
  resolver.AddReshape();
  resolver.AddLogistic();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Log("AllocateTensors() failed \n"); while (1) {}
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output_type = interpreter->output(1);
  output_precip = interpreter->output(0);

  run_inference();
}

void loop() { 
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      collect_data();
      run_inference();
    }

    lv_task_handler();
}
