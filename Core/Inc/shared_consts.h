#ifndef __SHARED_CONSTS__
#define __SHARED_CONSTS__


#define MAX_VAL_UINT16_T 0xffff
#define MAX_VAL_INT16_T  0x7fff
#define AUDIO_SAMPLE_RATE 44100
#define MAX_AMPLITUDE 3.3f
#define SONG_BUFF_SIZE 2048 / (sizeof(int16_t))



#define TRUE 0x1
#define FALSE 0x0



#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...)
#endif

#endif
