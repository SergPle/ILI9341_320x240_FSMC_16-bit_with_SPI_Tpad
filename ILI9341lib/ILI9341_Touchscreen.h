//	MIT License
//
//	Sergey Plekhanov Copyright (c)2024
//
//	This library is based on code:
//	Matej Artnak Copyright (c) 2017 with optimizations and additions
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.
//
//
//-----------------------------------
//	Resistive  touch screen of ILI9341 library for STM32
//-----------------------------------
//
//	Library is written for STM32 HAL library and supports STM32CUBEMX. To use the library with Cube software
//	you need to tick the box that generates peripheral initialization code in their own respective .c and .h file
//
//
//-----------------------------------
//	How to use this library
//-----------------------------------
//
//	-If using MCUs other than STM32F4 you will have to change the #include "stm32f4xx_hal.h" in the ILI9341_Touchscreen.h to your respective .h file
//	-define GPIO inputs and outputs then map the Pins and Ports inside the ILI9341_Touchscreen.h
//	-Library does not require any initialization calls apart from GPIO initialization. Initialize GPIOs before calling library functions!
//	-Perform calibration the first time you turn it on and you can save this data (e.g. in flash RAM)
//
//		Touchpad GPIO defines
//			Outputs:
//				CLK
//				MOSI
//				CS -chip select
//
//			Inputs:
//				IRQ - touch present
//				MISO
//
//
//	-Touch pad library bit bangs SPI interface and only requires basic GPIOs.
//	-Setting GPIOs as FREQ_VERY_HIGH Recommended
//	-If using different layout you will have to Re-Map X and Y coordinates of the Screen
//	-N_OF_POSITION_SAMPLES makes location output less noisy but increases reading time. Increase and decrease to your liking
//
//----------EXAMPLE------------------
/*
if(TP_Touchpad_Pressed())
	{					
		uint16_t x_pos = 0;
		uint16_t y_pos = 0;
		
		uint16_t position_array[2];
		if(TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK)
		{
		x_pos = position_array[0];
		y_pos = position_array[1];
		}							
	}
*/
//-----------------------------------


#ifndef TOUCH_H
#define TOUCH_H
#include "main.h"
#include "stm32f4xx_hal.h"

#define CALIBRATION_OK			1
#define CALIBRATION_FAILS		0


#define TP_CLK_PORT		GPIOB
#define TP_CLK_PIN		T_CLK_Pin

#define TP_CS_PORT		GPIOB
#define TP_CS_PIN		T_CS_Pin

#define TP_MISO_PORT		GPIOB
#define TP_MISO_PIN		T_MISO_Pin

#define TP_MOSI_PORT		GPIOB
#define TP_MOSI_PIN		T_MOSI_Pin

#define TP_IRQ_PORT		GPIOC
#define TP_IRQ_PIN		T_PEN_Pin

#define CMD_RDX             			0X90
#define CMD_RDY             			0XD0

//RETURN VALUES FOR TP_Touchpad_Pressed 
#define TOUCHPAD_NOT_PRESSED			0
#define TOUCHPAD_PRESSED			1

//RETURN VALUES FOR TP_Read_Coordinates
#define TOUCHPAD_DATA_OK			1
#define TOUCHPAD_DATA_NOISY			0

// Touch pad size
#define X_SIZE 					320
#define Y_SIZE 					240

//In order to increase accuracy of location reads library samples
//N_OF_POSITION_SAMPLES numbers of locations and averages them
//If library runs too slow decrease NO_OF_POSITION_SAMPLES, but
//expect inreasingly noisy or incorrect locations returned

#define N_OF_POSITION_SAMPLES	 	128

// Time to calibration of touch pad in ms
#define CALIBRATION_TIME		5000 // ms

//Read coordinates of touchscreen press. [0] = X, [1] = Y, [2] = RawX, [3] = RawY
uint8_t TP_Read_Coordinates(uint16_t Coordinates[]);

//Check if Touchpad was pressed. Returns TOUCHPAD_PRESSED (1) or TOUCHPAD_NOT_PRESSED (0)
uint8_t TP_Touchpad_Pressed();

// Touch pad calibration. Lets move stilus from upper left corner of screen to down right corner few time in CALIBRATION_TIME millisecond.
uint8_t TP_calibration();

#endif
