
#ifndef HTTP_REQEST_H
	#define HTTP_REQEST_H
	#include <string.h>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/event_groups.h"
	#include "esp_system.h"
	#include "esp_wifi.h"
	#include "esp_event_loop.h"
	#include "esp_log.h"
	#include "nvs_flash.h"

	#include "lwip/err.h"
	#include "lwip/sockets.h"
	#include "lwip/sys.h"
	#include "lwip/netdb.h"
	#include "lwip/dns.h"
#endif

//
void initialise_wifi(void);
void http_get_task(void *pvParameters);
void set_mDHT_temp(int num);
void set_mDHT_Humi(int num);


