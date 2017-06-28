#include "ledc_func.h"
/*
 * About this example
 * 1. init LEDC module:
 *    a. You need to set the timer of LEDC first, this decide the frequency and resolution of PWM.
 *    b. You need to set the LEDC channel you want to use, and bind with one of the timers.
 *
 * 2. You can install a default fade function, then you can use fade APIs.
 *
 * 3. You can also set a target duty directly without fading.
 *
 * 4. This example use GPIO18/19/4/5 as LEDC ouput, and it will change the duty repeatedly.
 *
 * 5. GPIO18/19 are from high speed channel group. GPIO4/5 are from low speed channel group.
 *
 */
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (19)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO       (4)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (5)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM       (4)
typedef struct {
    int channel;
    int io;
    int mode;
    int timer_idx;
} ledc_info_t;

// void set_freq_delay( uint32_t  iFreq , const TickType_t xDelay , uint32_t iDuty )
void set_freq_delay( uint32_t  iFreq , const TickType_t xDelay )
{
    int ch;
    ledc_info_t ledc_ch[LEDC_TEST_CH_NUM] = {
        {
            .channel   = LEDC_HS_CH0_CHANNEL,
            .io        = LEDC_HS_CH0_GPIO,
            .mode      = LEDC_HS_MODE,
            .timer_idx = LEDC_HS_TIMER
        },
        {
            .channel   = LEDC_HS_CH1_CHANNEL,
            .io        = LEDC_HS_CH1_GPIO,
            .mode      = LEDC_HS_MODE,
            .timer_idx = LEDC_HS_TIMER
        },
        {
            .channel   = LEDC_LS_CH2_CHANNEL,
            .io        = LEDC_LS_CH2_GPIO,
            .mode      = LEDC_LS_MODE,
            .timer_idx = LEDC_LS_TIMER
        },
        {
            .channel   = LEDC_LS_CH3_CHANNEL,
            .io        = LEDC_LS_CH3_GPIO,
            .mode      = LEDC_LS_MODE,
            .timer_idx = LEDC_LS_TIMER
        }
    };
// .freq_hz = 5000,              //set frequency of pwm
    ledc_timer_config_t ledc_timer = {
        .bit_num = LEDC_TIMER_13_BIT , //set timer counter bit number
        .freq_hz = iFreq ,              //set frequency of pwm
        .speed_mode = LEDC_HS_MODE,   //timer mode,
        .timer_num = LEDC_HS_TIMER    //timer index
    };
    //configure timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    //configure timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_LS_MODE;
    ledc_timer.timer_num = LEDC_LS_TIMER;
    ledc_timer_config(&ledc_timer);

//.duty = 0,
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config_t ledc_channel = {
            //set LEDC channel 0
            .channel = ledc_ch[ch].channel,
            //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
            .duty = 0,
            //GPIO number
            .gpio_num = ledc_ch[ch].io,
            //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
            .intr_type = LEDC_INTR_FADE_END,
            //set LEDC mode, from ledc_mode_t
            .speed_mode = ledc_ch[ch].mode,
            //set LEDC timer source, if different channel use one timer,
            //the frequency and bit_num of these channels should be the same
            .timer_sel = ledc_ch[ch].timer_idx,
        };
        //set the configuration
        ledc_channel_config(&ledc_channel);
    }

    //initialize fade service.
    ledc_fade_func_install(0);
    printf("LEDC set duty without fade\n");
    /*
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_set_duty(ledc_ch[ch].mode, ledc_ch[ch].channel, 2000 );
        ledc_update_duty(ledc_ch[ch].mode, ledc_ch[ch].channel);
    }
    */
    ledc_set_duty(ledc_ch[1].mode, ledc_ch[1].channel, 2000 );
    ledc_update_duty(ledc_ch[1].mode, ledc_ch[1].channel);
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
    vTaskDelay( xDelay );
    printf("LEDC set duty without fade\n");
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_set_duty(ledc_ch[ch].mode, ledc_ch[ch].channel, 0);
        ledc_update_duty(ledc_ch[ch].mode, ledc_ch[ch].channel);
    }
}

//
void play_tempLow()
{
	uint32_t iWait = 100;
	const TickType_t xDelay = 250 / portTICK_PERIOD_MS;
    set_freq_delay(NOTE_C4, xDelay );
    vTaskDelay(iWait / portTICK_PERIOD_MS);
    set_freq_delay(NOTE_C4, xDelay );
    vTaskDelay(iWait / portTICK_PERIOD_MS);
    set_freq_delay(NOTE_B3, xDelay );
    vTaskDelay(iWait / portTICK_PERIOD_MS);
    set_freq_delay(NOTE_C4, xDelay );
    vTaskDelay( 3000 / portTICK_PERIOD_MS);
    
}
void play_tempHigh()
{
	uint32_t iWait = 100;
	const TickType_t xDelay = 250 / portTICK_PERIOD_MS;
    for(int i=0; i<2; i++){
		set_freq_delay(NOTE_A6, xDelay );
		vTaskDelay(iWait / portTICK_PERIOD_MS);
		set_freq_delay(NOTE_B6, xDelay );
		vTaskDelay(iWait / portTICK_PERIOD_MS);
		set_freq_delay(NOTE_C7, xDelay );
		vTaskDelay(iWait / portTICK_PERIOD_MS);
    }
}
