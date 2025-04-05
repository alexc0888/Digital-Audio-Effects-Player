#ifndef __MY_SDCARD__
#define __MY_SDCARD__

#include "stm32f4xx_hal.h"
#include "my_debug.h"
#include <string.h>
#include <stdio.h>


#define SD_SUCCESS 0x1
#define SD_FAIL    0x0
#define ROOT_DIR   "0:/"
#define SONG_DIR   "0:/Songs"
#define MAX_FILE_NUM 64 // maximum files allowed in a given directory (due to statically allocated arrays)

// This header includes wrapper functions that use the fatFS API
// for the needs of this project.

int sdMount();
int sdGetFileList(char [MAX_FILE_NUM][64 + 1], char *, int *);

#endif
