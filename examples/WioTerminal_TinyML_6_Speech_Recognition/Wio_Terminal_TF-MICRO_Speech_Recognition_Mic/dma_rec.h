#ifndef REC
#define REC

#include "Arduino.h"

#define SAMPLES 48000
#define BUF_SIZE 320
//#define DEBUG

class  FilterBuHp
{
  public:
    FilterBuHp();
  private:
    float v[2];
  public:
    float step(float x);
};

static void audio_rec_callback(uint16_t *buf, uint32_t buf_len);
void DMAC_1_Handler();
void config_dma_adc();

#endif
