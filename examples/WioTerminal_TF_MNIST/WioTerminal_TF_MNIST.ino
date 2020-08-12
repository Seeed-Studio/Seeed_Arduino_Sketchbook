#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "model.h"
#include "mnist.h"
#include"TFT_eSPI.h"

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite

namespace {
// Globals
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
tflite::ErrorReporter* reporter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 8000; // Just pick a big enough number
uint8_t tensor_arena[ kTensorArenaSize ] = { 0 };
float *input_buffer=nullptr;
}  // namespace


float display_buffer[28224];

void bitmap_to_float_array( float* dest, const unsigned char* bitmap ) { // Populate input_vec with the monochrome 1bpp bitmap
  int pixel = 0;
  for( int y = 0; y < 28; y++ ) {
    for( int x = 0; x < 28; x++ ) {
      int B = x / 8; // the Byte # of the row
      int b = x % 8; // the Bit # of the Byte
      dest[ pixel ] = ( bitmap[ y * 4 + B ] >> ( 7 - b ) ) & 
                        0x1 ? 1.0f : 0.0f;
      pixel++;
    }
  }
}

void draw_input_buffer() {

  int pre_i, pre_j, after_i, after_j;//缩放前后对应的像素点坐标

  for (int i = 0; i<168; i++)
  {
    for (int j = 0; j<168; j++)
    {
        after_i = i;
        after_j = j;
        pre_i = (int)(after_i / 6);/////取整，插值方法为：最邻近插值（近邻取样法）
        pre_j = (int)(after_j / 6);
        if (pre_i >= 0 && pre_i < 28 && pre_j >= 0 && pre_j < 28)//在原图范围内
          *(display_buffer + i * 168 + j) = *(input_buffer + pre_i * 28 + pre_j);
    }
  }

    for(int cy = 0; cy < 168; cy++)
    {
        for(int cx = 0; cx < 168; cx++)
        {

           tft.drawPixel( cx + 76, cy, display_buffer[ cy * 168  + cx  ] > 0 ? 0xFFFFFFFF : 0xFF000000 );

        }
    }

    
}

void print_input_buffer() {
  char output[ 28 * 29 ]; // Each row should end row newline
  for( int y = 0; y < 28; y++ ) {
    for( int x = 0; x < 28; x++ ) {
      output[ y * 29 + x ] = input_buffer[ y * 28 + x ] > 0 ? ' ' : '#';
    }
    output[ y * 29 + 28 ] = '\n';
  }
  reporter->Report( output );
}



void setup() {
  // Load Model
  //Serial.begin(115200);
  static tflite::MicroErrorReporter error_reporter;
  reporter = &error_reporter;
  reporter->Report( "Let's use AI to recognize some numbers!" );

  model = tflite::GetModel( tf_model );
  if( model->version() != TFLITE_SCHEMA_VERSION ) {
    reporter->Report( "Model is schema version: %d\nSupported schema version is: %d", model->version(), TFLITE_SCHEMA_VERSION );
    return;
  }
  // Setup our TF runner
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, reporter );
  interpreter = &static_interpreter;
  
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if( allocate_status != kTfLiteOk ) {
    reporter->Report( "AllocateTensors() failed" );
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Save the input buffer to put our MNIST images into
  input_buffer = input->data.f;
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK); // fills entire the screen with colour red
  tft.setTextColor(TFT_RED);   
  tft.setTextSize(2);
  tft.drawString("It looks like the number:",0,200);//prints string at (70,80)
  TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite


}

void loop() {
  // Pick a random test image for input
  const int num_test_images = ( sizeof( test_images ) / sizeof( *test_images ) );


  bitmap_to_float_array( input_buffer, test_images[ rand() % num_test_images ] );

  draw_input_buffer();
  //print_input_buffer();
  // Run our model
  TfLiteStatus invoke_status = interpreter->Invoke();
  if( invoke_status != kTfLiteOk ) {
    reporter->Report( "Invoke failed" );
    return;
  }
  
  float* result = output->data.f;

  reporter->Report( "It looks like the number: %d", std::distance( result, std::max_element( result, result + 10 ) ) );
//   tft.drawNumber(std::distance( result, std::max_element( result, result + 10 ) ), 300, 200),
  spr.createSprite(20, 20);
//   spr.fillSprite(TFT_RED);
  spr.setTextColor(TFT_RED);
  spr.setTextSize(2);
  spr.drawNumber(std::distance( result, std::max_element( result, result + 10 ) ),0, 0); 
  spr.pushSprite(300, 200);
  spr.deleteSprite();
  // Wait 1-sec til before running again
  delay( 200 );

}
