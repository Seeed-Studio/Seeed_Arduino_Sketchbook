#include <TensorFlowLite.h>
//#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <unistd.h>

#include "mfcc.h"
#include "read_wav.h"
#include "slu_model.h"

#define NUM_FRAMES 100
#define NUM_MFCC_COEFFS 13
#define MFCC_DEC_BITS 4
#define FRAME_SHIFT_MS 30
#define FRAME_SHIFT ((int16_t)(SAMP_FREQ * 0.001 * FRAME_SHIFT_MS))
#define MFCC_BUFFER_SIZE (NUM_FRAMES*NUM_MFCC_COEFFS)
#define FRAME_LEN_MS 30
#define FRAME_LEN ((int16_t)(SAMP_FREQ * 0.001 * FRAME_LEN_MS))

#define DEBUG

int recording_win = 100; 

String filelist[] = {"change_language_to_chinese_wt.wav", "decrease_volume_wt.wav", "turn_on_the_lights_in_the_kitchen_wt.wav"};
uint8_t file_num = 0;

int num_frames = NUM_FRAMES;
int num_mfcc_features = NUM_MFCC_COEFFS;
int frame_shift = FRAME_SHIFT;
int frame_len = FRAME_LEN;
int mfcc_dec_bits = MFCC_DEC_BITS;

int16_t audio_buffer[BUF_SIZE];

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

constexpr int kTensorArenaSize = 1024*47;
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

void setup() {
  Serial.begin(115200);
  while (!Serial) {delay(10);}
  Serial.println(frame_shift);
  Serial.println(frame_len);

  init_SD();
  
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(checkpoints_2021_08_10_17_23_00_slu_model_tflite);
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
  Serial.println(freeMemory());
}


void loop() {
  
  open_wav(filelist[file_num]);
  
  while (recording_win > 0)
  {
  read_wav(audio_buffer);
  extract_features(audio_buffer);
  recording_win--;
  }
  
  close_wav();
  recording_win = 100;

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
  Serial.print(filelist[file_num]);
  Serial.print(" Intent: ");
  
  for (uint8_t i = 0; i < 6; i = i + 1) {
    y_intent[i] = (output_intent->data.int8[i] - output_intent->params.zero_point) * output_intent->params.scale;
    if (y_intent[i] > max_val) { max_val = y_intent[i]; y_intent_max = i; }
    } 
    
#ifdef DEBUG    
  Serial.print(y_intent_max);
  Serial.print(" ");
  Serial.print("\t");
#endif

  Serial.print(y_intent_max);
  Serial.print(" ");
  Serial.print("\t");
  
  Serial.print("Slot: ");
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
      
    Serial.print(y_slot_max[i]);
    Serial.print(" ");   
  }
  
  Serial.print("\n");
  
  file_num++;
  if (file_num > 2) { file_num = 0; }
}
