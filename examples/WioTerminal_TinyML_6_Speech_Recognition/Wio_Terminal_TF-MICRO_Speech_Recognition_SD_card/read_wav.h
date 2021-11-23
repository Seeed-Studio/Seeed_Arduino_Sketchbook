#ifndef READ_WAV
#define READ_WAV

#include <SPI.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#define SAMPLES 48000
#define BUF_SIZE 480

void init_SD();
void open_wav(String filename);
void read_wav(int16_t* audio_buffer);
void close_wav();

#endif
