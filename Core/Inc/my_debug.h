#ifndef __MY_DEBUG__
#define __MY_DEBUG__

#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...)
#endif

#endif
