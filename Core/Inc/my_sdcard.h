#ifndef __MY_SDCARD__
#define __MY_SDCARD__

#include "stm32f4xx_hal.h"
#include "my_debug.h"
#include <string.h>
#include <stdio.h>

#define SD_EOF     0x2
#define SD_SUCCESS 0x1
#define SD_FAIL    0x0
#define ROOT_DIR   "0:/"
#define SONG_DIR   "0:/Songs"
#define MAX_FILE_NUM 64 // maximum files allowed in a given directory (due to statically allocated arrays)
#define MAX_PATH_LEN 256 + 1 // include an extra byte for the '/' to append file to pathname
#define SD_BLOCK_SIZE 2048
#define SONG_BUFF_SIZE SD_BLOCK_SIZE / (sizeof(int16_t))

// This header includes wrapper functions that use the fatFS API
// for the needs of this project.

int sdMount();
int sdOpenFile(char *);
int sdReadFile(void *, uint32_t);
int sdCloseFile();
int sdUnmount();

int sdGetFileList(char [MAX_FILE_NUM][64 + 1], char *, int *);
int sdLoadSong(char *);
int sdReadSong(int16_t [SONG_BUFF_SIZE]);


#endif
