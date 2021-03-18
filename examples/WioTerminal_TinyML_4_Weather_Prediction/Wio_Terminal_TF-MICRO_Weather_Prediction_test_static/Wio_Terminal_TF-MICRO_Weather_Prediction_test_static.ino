#include <TensorFlowLite.h>
//#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model_Conv1D.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output_type = nullptr;
TfLiteTensor* output_precip = nullptr;

constexpr int kTensorArenaSize = 1024*25;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial) {delay(10);}
  
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(Conv1D_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  //static tflite::MicroMutableOpResolver<1> resolver;
  static tflite::AllOpsResolver resolver;
  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output_type = interpreter->output(1);
  output_precip = interpreter->output(0);
  
  Serial.println(input->dims->size);
  Serial.println(input->dims->data[1]);
  Serial.println(input->dims->data[2]);
  Serial.println(input->type);

  Serial.println(output_type->dims->size);
  Serial.println(output_type->dims->data[1]);
  Serial.println(output_type->type);

  Serial.println(output_precip->dims->size);
  Serial.println(output_precip->dims->data[1]);
  Serial.println(output_precip->type);
}

void loop() {

  int8_t x_quantized[72];
  float x[72] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0};        

  for (byte i = 0; i < 72; i = i + 1) {
        input->data.int8[i] = x[i] / input->params.scale + input->params.zero_point;
  }

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
    return;
  }

  // Obtain the quantized output from model's output tensor
  float y_type[4];

  // Dequantize the output from integer to floating-point
  int8_t y_precip_q = output_precip->data.int8[0];
  Serial.println(y_precip_q);
  float y_precip = (y_precip_q - output_precip->params.zero_point) * output_precip->params.scale;  
  Serial.print("Precip: ");
  Serial.print(y_precip);
  Serial.print("\t");
  Serial.print("Type: ");
  for (byte i = 0; i < 4; i = i + 1) {
    y_type[i] = (output_type->data.int8[i] - output_type->params.zero_point) * output_type->params.scale;
    Serial.print(y_type[i]);
    Serial.print(" ");
  }
  Serial.print("\n");
}
