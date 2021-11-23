#include "read_wav.h"

File sFile;

void init_SD() {

  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  Serial.println("SD initialization done.");

}


void open_wav(String filename) {
  sFile = SD.open(filename, FILE_READ);
  sFile.seek(44);
}

void read_wav(int16_t* audio_buffer) {
  sFile.read(audio_buffer, BUF_SIZE*2);
  
  //for (int i = 0; i < BUF_SIZE; i++) {
  //Serial.println(audio_buffer[i]);
  //}
}

void close_wav() {
  sFile.close();
}
