#include <TensorFlowLite.h>
//#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <unistd.h>

#include "mfcc.h"
#include "dma_rec.h"
#include "slu_model.h"
#include"TFT_eSPI.h"

TFT_eSPI tft;

#define NUM_FRAMES 150
#define NUM_MFCC_COEFFS 10
#define MFCC_DEC_BITS 4
#define FRAME_SHIFT_MS 20
#define FRAME_SHIFT ((int16_t)(SAMP_FREQ * 0.001 * FRAME_SHIFT_MS))
#define MFCC_BUFFER_SIZE (NUM_FRAMES*NUM_MFCC_COEFFS)
#define FRAME_LEN_MS 20
#define FRAME_LEN ((int16_t)(SAMP_FREQ * 0.001 * FRAME_LEN_MS))

int recording_win = NUM_FRAMES; 

int num_frames = NUM_FRAMES;
int num_mfcc_features = NUM_MFCC_COEFFS;
int frame_shift = FRAME_SHIFT;
int frame_len = FRAME_LEN;
int mfcc_dec_bits = MFCC_DEC_BITS;

extern int16_t audio_buffer[BUF_SIZE];
extern bool buf_ready;
extern uint8_t recording;
volatile static bool record_ready;

String intents[] = {"decrease", "activate", "change language", "bring", "deactivate", "increase"}; 
String slots[] = {"juice", "english", "lights", "washroom", "socks", "heat", "volume", "chinese", "shoes", "lamp", "korean", "bedroom", "music", "german", "kitchen", "newspaper", "none"};

float *mfcc_buffer = new float[num_frames*num_mfcc_features];
MFCC *mfcc = new MFCC(num_mfcc_features, frame_len, mfcc_dec_bits);

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output_intent = nullptr;
TfLiteTensor* output_slot = nullptr;

constexpr int kTensorArenaSize = 1024*55;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void extract_features(int16_t* audio_buffer) 
{
  //if(num_frames>recording_win) {
  //memmove(mfcc_buffer, mfcc_buffer+(recording_win*num_mfcc_features), (num_frames-recording_win)*num_mfcc_features);
  //}

  int32_t mfcc_buffer_head = (num_frames - recording_win)*num_mfcc_features; 
  //Serial.println(mfcc_buffer_head);
  mfcc->mfcc_compute(audio_buffer, &mfcc_buffer[mfcc_buffer_head]);
  //mfcc_buffer_head += num_mfcc_features;

}

void print_to_scr(String toPrint, int x, int y){
  tft.drawString(toPrint, x, y);
  Serial.print(toPrint);
  }
  
void setup() {
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN);
  
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  Serial.begin(115200);
#ifdef DEBUG
  while (!Serial) {delay(10);}
#endif
  Serial.println(frame_shift);
  Serial.println(frame_len);
  
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(checkpoints_2021_08_20_18_02_16_slu_model_tflite);
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
  output_slot = interpreter->output(1);
  output_intent = interpreter->output(0);
  
  Serial.println(input->dims->size);
  Serial.println(input->dims->data[1]);
  Serial.println(input->dims->data[2]);
  Serial.println(input->type);

  Serial.println(output_intent->dims->size);
  Serial.println(output_intent->dims->data[1]);
  Serial.println(output_intent->type);

  Serial.println(output_slot->dims->size);
  Serial.println(output_slot->dims->data[1]);
  Serial.println(output_slot->type);

  config_dma_adc();
  
  Serial.println(freeMemory());

}


void loop() {

if (digitalRead(WIO_KEY_A) == LOW && !recording) {

  delay(100);
  tft.fillScreen(TFT_BLACK);
  //Serial.println("Starting sampling");
  print_to_scr("Sampling - Speak", 20, 120);
  recording = 1;
  record_ready = false; 
  
  while (recording_win > 0)
  {   
    delay(1);  
    if (buf_ready == true) {

      extract_features(audio_buffer);
      recording_win--;
      buf_ready = false;
      }
  }
  
  recording = 0;
  record_ready = true;
  recording_win = NUM_FRAMES;

#ifdef DEBUG
  for (uint16_t f = 0; f < num_frames*num_mfcc_features; f++) {
  Serial.println(mfcc_buffer[f], 8);
  }
#endif
  
  for (uint16_t i = 0; i < MFCC_BUFFER_SIZE; i = i + 1) {
        input->data.int8[i] = mfcc_buffer[i] / input->params.scale + input->params.zero_point;
        //Serial.println(i);  
  }

  TfLiteStatus invoke_status = interpreter->Invoke();
    //Serial.println("ok");  
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
    return;
  }

  float y_intent[6];
  float y_slot[2][17];
  
  uint8_t y_slot_max[2];
  uint8_t y_intent_max;  
  
  float max_val = 0.0;
  tft.fillScreen(TFT_BLACK);
  print_to_scr("Intent: ", 10, 10);
  
  for (uint8_t i = 0; i < 6; i = i + 1) {
    y_intent[i] = (output_intent->data.int8[i] - output_intent->params.zero_point) * output_intent->params.scale;
    if (y_intent[i] > max_val) { max_val = y_intent[i]; y_intent_max = i; }
    } 
    
#ifdef DEBUG    
  Serial.print(y_intent_max);
  Serial.print(" ");
  Serial.print("\t");
#endif
  print_to_scr(String(intents[y_intent_max]) + " \t", 10, 40); 
  //Serial.print(intents[y_intent_max]);
  //Serial.print(" ");
  //Serial.print("\t");
  
  //Serial.print("Slot: ");
  print_to_scr("Slot: ", 10, 80);   
  for (uint8_t i = 0; i < 2; i = i + 1) {
    
    max_val = 0.0;
    
    for (uint8_t j = 0; j < 17; j = j + 1) {
      
    y_slot[i][j] = (output_slot->data.int8[i*17 + j] - output_slot->params.zero_point) * output_slot->params.scale;
    if (y_slot[i][j] > max_val) { max_val = y_slot[i][j]; y_slot_max[i] = j; }
    
#ifdef DEBUG
    Serial.print(y_slot[i][j]);
    Serial.print(" ");
#endif
    }
    
#ifdef DEBUG
    Serial.print(y_slot_max[i]);
    Serial.print("\t");   
#endif
    print_to_scr(String(slots[y_slot_max[i]]) + " ", 10, 110+i*30);   
    //Serial.print(slots[y_slot_max[i]]);
    //Serial.print(" ");   
  }
  
  Serial.print("\n");
 }
}
