#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/fcntl.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "spiffs_vfs.h"

#include "ffs_func.h" 
#include "http_request.h" 

#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sys/time.h"
#include "esp_deep_sleep.h"
#include "driver/gpio.h"
#include "DHT11.h"
	
static const char tag[] = "MAIN";
//
#include "esp_system.h"
#include <math.h>
#include "ledc_func.h"

const int mPin_dht11 = 23;
static const int mOK_CODE=1;
static  const int mNG_CODE=0;
double mRd_ffsTemp=0.0;
// #define BLINK_GPIO CONFIG_BLINK_GPIO
#define BLINK_GPIO 22
//deep-sleep
#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION      300  // sleep XX seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory

//
static void execDeepSleep(){
	struct timeval now;

	printf("start Deep Sleep\n");
	gettimeofday(&now, NULL);
	printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);
	last = now.tv_sec;
	printf("config Timer\n");
	esp_deep_sleep_enable_timer_wakeup(1000000LL * GPIO_DEEP_SLEEP_DURATION); // set timer but don't sleep now

	printf("config IO\n");
	esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO); //!< Keep power domain enabled in deep sleep, if it is needed by one of the wakeup options. Otherwise power it down.
	gpio_pullup_en(GPIO_INPUT_IO_TRIGGER);		// use pullup on GPIO
	gpio_pulldown_dis(GPIO_INPUT_IO_TRIGGER);       // not use pulldown on GPIO

	esp_deep_sleep_enable_ext0_wakeup(GPIO_INPUT_IO_TRIGGER, 0); // Wake if GPIO is low

	printf("deep sleep #Start#\n");
	esp_deep_sleep_start();	
}

//
void blink_task(void *pvParameter)
{
	const int iWait= 100;
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelay(iWait / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(iWait / portTICK_PERIOD_MS);
    }
}
//
static int read_ffs(char *fname )
{
	int iRet =mNG_CODE;
    char cSensor[30+1] , cTemp[30+1];
	if(read_alermData(fname, "S:" ,cSensor )==0){
		printf("Error, readFFS  \n");
		return iRet;
	}
	printf( "cSensor=%s \n" ,cSensor);
	int iSt =0;
	for(int i=2; i< strlen(cSensor); i++){
		cTemp[iSt] =cSensor[i];
		iSt++;
	}
//		cTemp[strlen(cTemp) ]="\0";
	printf( "cTemp=%s \n" ,cTemp);
	mRd_ffsTemp= atof(cTemp);
	printf( "rd-f=%f \n" ,mRd_ffsTemp);
	cSensor[0]=0x00;	
	iRet= mOK_CODE;
	return iRet;
}

static void proc_ffs_rw(int iTemp ){
    vfs_spiffs_register();
    printf("\r\n\n");
    char *dirName = "/spiffs/";
   	if (spiffs_is_mounted) {
		vTaskDelay( 100 / portTICK_RATE_MS);
		char fname[20+1];
		sprintf(fname,  "%salerm.txt", dirName );
		//R
		if(read_ffs(fname )== mNG_CODE){
			// return;		
		}
		//w
		char sTemp[10+1];
		sprintf(sTemp ,"%d", iTemp);
		if(write_alermData(fname , "0926", sTemp )== 0){
			printf("Error ,write\n");
		}
		list("/spiffs/", NULL);
	    printf("\r\n");
    }
}
//
static int get_dht11_temp()
{
	int ret=0;
	for(int i=0;i <10; i++){
	    ret= getTemp();
//	    printf("get_dht11_temp.ret=%d \n",ret);
	    vTaskDelay( 50 / portTICK_RATE_MS);
	}
	return ret;
}
//
static int get_dht11_humi()
{
	int ret=0;
	for(int i=0;i <10; i++){
	    ret= getHumidity();
//	    printf("get_dht11_humi.ret=%d \n",ret);
	    vTaskDelay(50 / portTICK_RATE_MS);
	}
	return ret;
}

//
void watit_forSec(int sec){
    for(sec = 3 ; sec > 0; sec--) {
        ESP_LOGI( tag , "watit_forSec now,  %d... ", sec );
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
//================
int app_main(void)
{
	printf("\r\n\n");
	ESP_LOGI(tag, "==== STARTING BOT ====\r\n");
	setDHTPin(mPin_dht11 );
//	printf("Temperature reading %d\n",getTemp());
    int iNowTemp= get_dht11_temp();
    int iHumi   = get_dht11_humi();
    set_mDHT_temp(iNowTemp);
    set_mDHT_Humi(iHumi );
    printf("iNowTemp=%d \n",iNowTemp );
	//WIFI
	ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
//    vTaskDelay(1000 / portTICK_RATE_MS);
    watit_forSec(3 );
    http_get_task(NULL );
    //LED
    xTaskCreate(&blink_task, "blink_task", 512, NULL, 5, NULL);
    //ffs
    proc_ffs_rw( iNowTemp  );
    double dTemp = mRd_ffsTemp;
    printf("dTemp=%f,iNowTemp=%d \n",dTemp , iNowTemp);
    //comp - alerm
    if(iNowTemp  > ( dTemp + 0.5) ){
//        printf("now-H=%f \n", (double)iNowTemp + 0.5 );
    	printf("#temp=H \n");
    	play_tempHigh();
    }else if(iNowTemp < ( dTemp - 0.5) ){
//        printf("now-L=%f \n", (double)iNowTemp - 0.5 );
    	printf("#temp=L \n");
    	play_tempLow();
    }else{
        printf("temp is same value. \n");
    }
    ESP_LOGI(tag, "==== restart, waiting ====\r\n");
    /*
    for(int countdown = 3 ; countdown > 0; countdown--) {
        ESP_LOGI( tag , "%d... ", countdown);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */
    watit_forSec(3 );
    execDeepSleep();
    return 0;
}

