

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#ifndef FFS_FUNC_H
	#define FFS_FUNC_H
	#include <errno.h>
	#include <sys/fcntl.h>
	#include "esp_vfs.h"
	#include "esp_vfs_fat.h"
	#include "esp_log.h"
	#include "spiffs_vfs.h"
#endif

void list(char *path, char *match);
void readTest(char *fname);
// int read_alermData(char *fname, char *sMatch );
int read_alermData(char *fname, char *sMatch, char *cReturn );
int write_alermData(char *fname , char *cTime, char *cTemp);
void delete_allFiles(char *path );
void read_mp3(char *fname);


