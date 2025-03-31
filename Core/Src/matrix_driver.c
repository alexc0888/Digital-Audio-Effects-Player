
#include "matrix_driver.h"
#include "fft_driver.h"
#include <string.h>

// structure to contain frame data ready to be directly DMA'ed to matrix pins
hub75_gpio_t screen [SCREEN_ROW][SCREEN_COL];
// structure to contain current and previous frame data
FrameBuffer frameBuffers[MAX_FRAME_BUFFERS];

void initMatrix(void)
{
	// ensure all matrix GPIO  pins are 0 by default
	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_All & ~(GPIO_PIN_0 | GPIO_PIN_1)), GPIO_PIN_RESET);

	// clear structure of any lingering data
	initScreen();

	setup_DMA2_S1();
	setup_TIM8(1, 2);
}


void drawFrame(color_t frame[ROW][COL])
{
	for(uint8_t row = 0; row < ROW; row++)
	{
			updateRow(row, frame[row]); // update all 32 rows according to the incoming frame
	}
}

// takes a row 0-31 as input and maps each row to new color of incoming frame
void updateRow(uint8_t r, color_t rowColors[COL])
{
	uint8_t tf_r;
	tf_r = transformRowNum(r); // partial row transformation (still need to split by rgb1/rgb2!)

	for(uint8_t col = 0; col < COL; col++)
	{
		updatePixel(tf_r, col, rowColors[col]);
	}
}

// update a particular pixel of the screen according to how the screen gets mapped into the matrix pins via DMA
void updatePixel(uint8_t tf_r, uint8_t c, color_t color)
{
	uint8_t tf_c = c * 2;
	if(tf_r < 16)
	{
		screen[tf_r % 16][tf_c].rgb1     = color;
		screen[tf_r % 16][tf_c + 1].rgb1 = color;
	}
	else
	{
		screen[tf_r % 16][tf_c].rgb2     = color;
		screen[tf_r % 16][tf_c + 1].rgb2 = color;
	}
}

void initScreen()
{
	for(uint8_t row = 0; row < SCREEN_ROW; row++)
	{
		for(uint8_t col = 0; col < SCREEN_COL; col++)
		{
			screen[row][col].dead = 0;
			screen[row][col].rgb1 = 0;
			screen[row][col].rgb2 = 0;
			screen[row][col].addr = row;

			// setup clk to automatically rise and fall over each DMA transaction
			if(col % 2)
			{
				screen[row][col].clk = 0;
			}
			else
			{
				screen[row][col].clk = 1;
			}

			// once the final pixel is shifted in, latch it and raise oe
			if(col >= (SCREEN_COL - 2))
			{
				screen[row][col].lat = 1;
				screen[row][col].oe  = 1;
			}
			else
			{
				screen[row][col].lat = 0;
				screen[row][col].oe  = 0;
			}
		}
	}
}

// this remapping is required because something is screwy in the LED matrix HW
uint8_t transformRowNum(uint8_t r)
{
	if(r == 0)
	{
		r = 15;
	}
	else if(r == 16)
	{
		r = 31;
	}
	else
	{
		r--;
	}
	return r;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize frame buffers for interpolation
 */
void initFrameBuffers(void)
{
    //Initialize each frame buffer in the array
    //  Mark frames as invalid to start
	//  Set all pixels to BLACK
	for (int frame = 0; frame < MAX_FRAME_BUFFERS; frame++) {
		frameBuffers[frame].isValid = FALSE;
		for (int row = 0; row < ROW; row++) {
			for (int col = 0; col < COL; col++) {
				frameBuffers[frame].pixels[row][col] = BLACK;
			}
		}
	}
}

/**
 * @brief Store a new frame, shifting the previous one
 *
 * @param newFrame Pointer to the new frame data to store
 */
void storeFrame(color_t newFrame[ROW][COL])
{
    // If we already have a current frame, move it to previous position
    if (frameBuffers[CURR_FRAME].isValid) {
        // Copy current frame to previous frame slot
        for (int row = 0; row < ROW; row++) {
            for (int col = 0; col < COL; col++) {
                frameBuffers[PREV_FRAME].pixels[row][col] = frameBuffers[CURR_FRAME].pixels[row][col];
            }
        }
        // Mark previous frame as valid
        frameBuffers[PREV_FRAME].isValid = 1;
    }

    // Copy new frame into current frame slot
    for (int row = 0; row < ROW; row++) {
        for (int col = 0; col < COL; col++) {
            frameBuffers[CURR_FRAME].pixels[row][col] = newFrame[row][col];
        }
    }

    // Mark current frame as valid
    frameBuffers[CURR_FRAME].isValid = 1;
}

/**
 * @brief Create an interpolated frame between stored frames
 *
 * @param factor Interpolation factor (0.0 = previous frame, 1.0 = current frame)
 * @param outputFrame Buffer to store the interpolated frame
 */
void interpolateFrame(float factor, color_t outputFrame[ROW][COL])
{
	// Edge case for when only 1 frame is available
	if (frameBuffers[PREV_FRAME].isValid != TRUE) {
		if (frameBuffers[CURR_FRAME].isValid == TRUE) {
			for (int row = 0; row < ROW; row++) {
				for (int col = 0; col < COL; col++) {
					outputFrame[row][col] = frameBuffers[CURR_FRAME].pixels[row][col];
				}
			}
		}
		// No frames are available
		else {
			for (int row = 0; row < ROW; row++) {
				for (int col = 0; col < COL; col++) {
					outputFrame[row][col] = BLACK;
				}
			}
		}
		return;
	}
	// Both frames are valid

	// clear the output frame
	for (int row = 0; row < ROW; row++) {
		for (int col = 0; col < COL; col++) {
			outputFrame[row][col] = BLACK;
		}
  }

  // Process each column individualy
  for (int bin = 0; bin < (COL / BIN_WIDTH_SCREEN); bin++) {
    // Calculate the column range for this bin
    int startCol = bin * BIN_WIDTH_SCREEN;
    int endCol = startCol + BIN_WIDTH_SCREEN - 1;

    // Find height of this bin in prev frame
    int prevHeight = 0;
    for (int row = 0; row < ROW; row++) {
      if (frameBuffers[PREV_FRAME].pixels[row][startCol] != BLACK) {
        prevHeight = ROW - row;
        break;
      }
    }

      // Find the height of this bin in curr frame
      int currHeight = 0;
      for (int row = 0; row < ROW; row++) {
        if (frameBuffers[CURR_FRAME].pixels[row][startCol] != BLACK) {
          currHeight = ROW - row;
          break;
        }
      }

      // Calculate interpolated height
      float interpHeight = prevHeight * (1.0f - factor) + currHeight * factor;
      int targetHeight = (int)round(interpHeight);

      // Draw the interpolated bar for this bin across all its columns
      for (int col = startCol; col <= endCol; col++) {
        for (int h = 0; h < targetHeight; h++) {
          int row = (ROW - 1) - h;
          if (row >= 0) { // ensure we clip to top of screen in case targetHeight exceeds ROW somehow
          	outputFrame[row][col] = (bin % (NUM_COLORS - 1)) + 1;
          }
        }
      }
    }
}

/**
 * @brief Interpolate and draw a frame in one step
 *
 * @param factor Interpolation factor (0.0 = previous frame, 1.0 = current frame)
 */
void drawInterpFrame(float factor)
{

	// Create temp buffer for the interpolated frame
	color_t interpFrame[ROW][COL];

	// Generate the interpolated frame
	interpolateFrame(factor, interpFrame);

	// Draw it to display
	drawFrame(interpFrame);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup_TIM8(uint16_t psc, uint16_t arr)
{
	// Turn on the clock for timer 8
	RCC  -> APB2ENR |= RCC_APB2ENR_TIM8EN; // TIM8 clock
	TIM8 -> CR1     &= ~TIM_CR1_DIR; // count up

	TIM8 -> PSC   = psc - 1;
	TIM8 -> ARR   = arr - 1; // f_tim = fclk / (psc * arr) Hz
	TIM8 -> DIER |= TIM_DIER_UDE; // on timer update, trigger the DMA
	TIM8 -> CR1  |= TIM_CR1_CEN; // enable the timer
}

void setup_DMA2_S1(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  DMA2_Stream1 -> CR &= ~DMA_SxCR_EN;

  DMA2_Stream1 -> CR  |= (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0); // channel 7 - triggering off of TIM8_UP
  DMA2_Stream1 -> CR  |= (DMA_SxCR_MSIZE_1); // memory size is 32-bit per transaction
  DMA2_Stream1 -> CR  |= (DMA_SxCR_PSIZE_1); // peripheral size is 32-bit per transaction
  DMA2_Stream1 -> CR  |= DMA_SxCR_MINC;
  DMA2_Stream1 -> CR  |= DMA_SxCR_CIRC;
  DMA2_Stream1 -> CR  |= DMA_SxCR_DIR_0; // memory to peripheral transfer


  DMA2_Stream1 -> NDTR = SCREEN_ROW * SCREEN_COL; // 2048 transfers
  DMA2_Stream1 -> PAR  = (uint32_t) (&(GPIOE->ODR));
  DMA2_Stream1 -> M0AR = (uint32_t) screen;

  DMA2_Stream1 -> CR  |= DMA_SxCR_EN; // Enable DMA
}


