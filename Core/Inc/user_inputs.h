/*
 * audio_processing.h
 *
 *  Created on: Apr 22, 2025
 *      Authors: alexc0888 (Alex Chitsazzadeh)
 *      				           (Zichen Zhu)
 *
 */
#ifndef __USER_INPUTS__
#define __USER_INPUTS__

#include "shared_consts.h"
#include "main.h"
#include "stm32f4xx_it.h"

// defined constants


// function declarations
char getButton();
void setup_TIM7(uint16_t, uint16_t);


#endif
