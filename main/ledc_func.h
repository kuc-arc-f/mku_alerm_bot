/* Ledc fade example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#ifndef LEDC_FUNC
	#define LEDC_FUNC
	#include <stdio.h>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/xtensa_api.h"
	#include "freertos/queue.h"
	#include "driver/ledc.h"
	#include "esp_attr.h"   
	#include "esp_err.h"
	#include "pitches.h"
#endif

void set_freq_delay( uint32_t  iFreq , const TickType_t xDelay  );
//void set_freq_delay( uint32_t  iFreq , const TickType_t xDelay , uint32_t iDuty );
void play_tempHigh();
void play_tempLow();
