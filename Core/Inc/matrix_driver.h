#ifndef __MATRIX_DRIVER__
#define __MATRIX_DRIVER__

#include "stm32f4xx_hal.h"

// 32 x 64 LED matrix for high-level abstracted "frame" data structure
#define ROW 32
#define COL 64

// screen data structure only needs half the rows, but needs twice the cols to allow room for clk pin to change
// for low-level led matrix data structure
#define SCREEN_ROW ROW / 2
#define SCREEN_COL COL * 2

#define NUM_COLORS 8

#define INTERP_STEPS 10
#define MAX_FRAME_BUFFERS 2   // We need to store at least 2 frames for interpolation
#define CURR_FRAME 0
#define PREV_FRAME 1
#define TRUE 1
#define FALSE 0
// structure for DMA output (16 bits/ 2B)
typedef struct
{
	unsigned int dead:  2;
	unsigned int rgb1:  3;
	unsigned int rgb2:  3;
	unsigned int addr:  5;
	unsigned int oe:    1;
	unsigned int lat:   1;
	unsigned int clk:   1;
} hub75_gpio_t;


// Colors enumerated as 0bRBG
typedef enum
{
	BLACK,
	RED,
	BLUE,
	PURPLE,
	GREEN,
	YELLOW,
	TEAL,
	WHITE
} color_t;

// Structure to represent a single frame of data
typedef struct {
    color_t pixels[ROW][COL];   // Full frame pixel data
    uint8_t isValid;            // Flag to indicate if the frame contains valid data
} FrameBuffer;

// Driver Functions
void initMatrix(void);
void drawFrame(color_t [ROW][COL]);
void updateRow(uint8_t, color_t [COL]);
void updatePixel(uint8_t, uint8_t, color_t);
void initScreen(void);
uint8_t transformRowNum(uint8_t);

void initFrameBuffers(void);
void storeFrame(color_t newFrame[ROW][COL]);
void interpolateFrame(float factor, color_t outputFrame[ROW][COL]);
void drawInterpFrame(float factor);

// STM32 Peripheral Settings
void setup_TIM8(uint8_t, uint8_t);
void setup_DMA2_S1(void);


// External declarations
extern FrameBuffer frameBuffers[MAX_FRAME_BUFFERS];  // [0] = current frame, [1] = previous frame

#endif
