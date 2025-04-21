/*
 * oled_driver.h
 *
 *  Created on: Apr 8, 2025
 *      Author: vjroc (Vinay Jagan)
 */

#ifndef __OLED_DRIVER__
#define __OLED_DRIVER__

#include "shared_consts.h"
#include "main.h"

#define OLED_1in5_RGB_WIDTH  128 //OLED width
#define OLED_1in5_RGB_HEIGHT 128 //OLED height

void OLED_1in5_rgb_Init(void);
void OLED_1in5_rgb_Clear(uint8_t r, uint8_t g, uint8_t b);
void OLED_1in5_rgb_Display(uint32_t *Image);
void draw_char(uint32_t *image, uint32_t startX, uint32_t startY, char c, uint32_t color);
void draw_text(uint32_t *image, uint32_t startX, uint32_t startY, char *text, uint32_t color);
void render_track_list(int current_selection);
void render_track_playing(int current_selection, int current_duration, int total_duration);

void OLED_InitReg(void);

#endif /* __OLED_DRIVER__ */
