/*
 * app_main.c
 *
 *  Created on: 10.10.2017
 *      Author: karawin
 */

#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2s.h"

#define TAG "i2sbridge"

static void init_i2s()
{
   const int sample_rate = 44100;

   /* TX: I2S_NUM_0 */
    i2s_config_t i2s_config_tx = {
   .mode = I2S_MODE_MASTER | I2S_MODE_TX,
   .sample_rate = sample_rate,
   .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,   // 2-channels
   .communication_format = I2S_COMM_FORMAT_I2S_MSB,
   .dma_buf_count = 32,                            // number of buffers, 128 max.
   .dma_buf_len = 32 * 2,                          // size of each buffer
   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1        // Interrupt level 1
    };

    i2s_pin_config_t pin_config_tx = {
         .bck_io_num = GPIO_NUM_26,
         .ws_io_num = GPIO_NUM_25,
         .data_out_num = GPIO_NUM_22,
         .data_in_num = GPIO_NUM_23
   };
    i2s_driver_install(I2S_NUM_0, &i2s_config_tx, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config_tx);


    /* RX: I2S_NUM_1 */
    i2s_config_t i2s_config_rx = {
   .mode = I2S_MODE_MASTER | I2S_MODE_RX, // Only TX
   .sample_rate = sample_rate,
   .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,    // Only 8-bit DAC support
   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,   // 2-channels
   .communication_format = I2S_COMM_FORMAT_I2S_MSB,
   .dma_buf_count = 32,                            // number of buffers, 128 max.
   .dma_buf_len = 32 * 2,                          // size of each buffer
   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1        // Interrupt level 1
   };

   i2s_pin_config_t pin_config_rx = {
      .bck_io_num = GPIO_NUM_17,
      .ws_io_num = GPIO_NUM_18,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = GPIO_NUM_5
   };

   i2s_driver_install(I2S_NUM_1, &i2s_config_rx, 0, NULL);
   i2s_set_pin(I2S_NUM_1, &pin_config_rx);

}


void task_i2sbridge(void *pvParams)
{
   uint16_t buf_len = 1024;
   char *buf = calloc(buf_len, sizeof(char));

   struct timeval tv = {0};
   struct timezone *tz = {0};
   gettimeofday(&tv, &tz);
   uint64_t micros = tv.tv_usec + tv.tv_sec * 1000000;
   uint64_t micros_prev = micros;
   uint64_t delta = 0;

   init_i2s();

   int cnt = 0;
   int bytes_written = 0;

   while(1)
   {
      char *buf_ptr_read = buf;
      char *buf_ptr_write = buf;

      // read whole block of samples
      int bytes_read = 0;
      while(bytes_read == 0) {
         bytes_read = i2s_read_bytes(I2S_NUM_1, buf, buf_len, 0);
      }

      uint32_t samples_read = bytes_read / 2 / (I2S_BITS_PER_SAMPLE_32BIT / 8);

      //  convert 2x 32 bit stereo -> 1 x 16 bit mono
      for(int i = 0; i < samples_read; i++) {

         // const char samp32[4] = {ptr_l[0], ptr_l[1], ptr_r[0], ptr_r[1]};

         // left
         buf_ptr_write[0] = buf_ptr_read[2]; // mid
         buf_ptr_write[1] = buf_ptr_read[3]; // high

         // right
         buf_ptr_write[2] = buf_ptr_write[0]; // mid
         buf_ptr_write[3] = buf_ptr_write[1]; // high


         buf_ptr_write += 2 * (I2S_BITS_PER_SAMPLE_16BIT / 8);
         buf_ptr_read += 2 * (I2S_BITS_PER_SAMPLE_32BIT / 8);
      }

      // local echo
      bytes_written = samples_read * 2 * (I2S_BITS_PER_SAMPLE_16BIT / 8);
      i2s_write_bytes(I2S_NUM_0, buf, bytes_written, portMAX_DELAY);

      cnt += samples_read;

      if(cnt >= 44100) {
         gettimeofday(&tv, &tz);
         micros = tv.tv_usec + tv.tv_sec * 1000000;
         delta = micros - micros_prev;
         micros_prev = micros;
         printf("%d samples in %" PRIu64 " usecs\n", cnt, delta);

         cnt = 0;
      }
   }
}
