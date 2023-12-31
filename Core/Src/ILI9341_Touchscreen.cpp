//	MIT License
//
//	Copyright (c) 2017 Matej Artnak
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
//	ILI9341 Touchscreen library for STM32
//-----------------------------------
//
//	Very simple Touchscreen library for ILI9341.
//	Extremly basic reading of position. No runtime calibration, No prediction, basic noise removal. Simple but stupid.
//	Basic hardcoded calibration values saved in .h file
//
//	Library is written for STM32 HAL library and supports STM32CUBEMX. To use the library with Cube software
//	you need to tick the box that generates peripheral initialization code in their own respective .c and .h file
//
//
//-----------------------------------
//	How to use this library
//-----------------------------------
//
//	-If using MCUs other than STM32F7 you will have to change the #include "stm32f7xx_hal.h" in the ILI9341_Touchscreen.h to your respective .h file
//	-define GPIO inputs and outputs then map the Pins and Ports inside the ILI9341_Touchscreen.h
//	-Library does not require any initialization calls apart from GPIO initialization. Initialize GPIOs before calling library functions!
//
//		Touchpad GPIO defines
//			Outputs:
//				CLK
//				MOSI
//				CS
//
//			Inputs:
//				IRQ
//				MISO
//
//
//	-Touchpad library bitbangs SPI interface and only requires basic GPIOs.
//	-Setting GPIOs as FREQ_VERY_HIGH Recommended
//
//	-Warning! Library is written for "ILI9341_Set_Rotation(SCREEN_VERTICAL_1)"	(See ILI9341_STM32_Driver for information about function)
//	-If using different layout you will have to Re-Map X and Y coordinates of the Screen
//
//
//	-NO_OF_POSITION_SAMPLES makes location output less noisy but increases reading time. Increase and decrease to your liking
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


#include "ILI9341_Touchscreen.h"
#include "stm32f4xx_hal.h"

//INITIAL HARDCODED CALIBRATION, CHANGE IF REQUIRED

volatile uint16_t X_OFFSET = 4270;
volatile uint16_t Y_OFFSET = 3855;
volatile float X_MAGNITUDE = 0x433F;
volatile float Y_MAGNITUDE = 0x436E00F;

//Internal Touchpad command, do not call directly
static uint16_t TP_Read(void)
{
    uint8_t i = 16;
    uint16_t value = 0;

    while(i)
    {
        value <<= 1;

		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
			
        if(HAL_GPIO_ReadPin(TP_MISO_PORT, TP_MISO_PIN))
        {
            ++value;
        }

        --i;
    }

    return value;
}

//Internal Touchpad command, do not call directly
static void TP_Write(uint8_t value)
{
    uint8_t i = 0x08;

		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);	
	
    while(i)
    {

		HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, (value & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET );

        value <<= 1;
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET); 	//  CLK pulse
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
        --i;
    }
}



//Read coordinates of touchscreen press. [0] = X, [1] = Y, [2] = RawX, [3] = RawY
uint8_t TP_Read_Coordinates(uint16_t Coordinates[])
{
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);			// CLK   - High
		HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_SET);			// MOSI  - High
		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);				// CS - High (inactively)

	
	
    uint32_t avg_x, avg_y = 0;		
    uint16_t rawx, rawy = 0;
    uint32_t calculating_x, calculating_y = 0;
	
    uint32_t samples = N_OF_POSITION_SAMPLES;
    uint32_t counted_samples = 0;

		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_RESET); 			// CS - active

	
    while((samples)&&( ! HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN)))
    {			
        TP_Write(CMD_RDY);

				rawy = TP_Read();	
				avg_y += rawy;
				calculating_y += rawy;

				
        TP_Write(CMD_RDX);
				rawx = TP_Read();
				avg_x += rawx;
				calculating_x += rawx;
        samples--;
				counted_samples++;
    }
		
	HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);

		
	if((counted_samples == N_OF_POSITION_SAMPLES)&&(HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == 0))
	{
		
	    calculating_x /= counted_samples;
	    calculating_y /= counted_samples;
		
	    rawx = calculating_x;
	    Coordinates[2] = rawx;
	    rawy = calculating_y;
	    Coordinates[3] = rawy;
		
		//CONVERTING 16bit Value to Screen coordinates
		// 65535/273 = 240!
		// 65535/204 = 320!
	    Coordinates[0] = (rawx - X_OFFSET) / X_MAGNITUDE;
	    Coordinates[1] = (rawy - Y_OFFSET) / Y_MAGNITUDE;
		
		return TOUCHPAD_DATA_OK;			
		}

			Coordinates[0] = 0;
			Coordinates[1] = 0;
			return TOUCHPAD_DATA_NOISY;

}

//Check if Touchpad was pressed. Returns TOUCHPAD_PRESSED (1) or TOUCHPAD_NOT_PRESSED (0)
uint8_t TP_Touchpad_Pressed()
{
	if(HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == 0)
	{
		return TOUCHPAD_PRESSED;
	}
	else
	{
		return TOUCHPAD_NOT_PRESSED;
	}
}

// Touch pad calibration. Lets move stilus from upper left corner of screen to down right corner few time in CALIBRATION_TIME millisecond.
uint8_t TP_calibration()
{
  uint16_t rawCoord[4];
  uint16_t Xmin = 0xFFFF;
  uint16_t Ymin = 0xFFFF;
  uint16_t Xmax = 0;
  uint16_t Ymax = 0;

  uint32_t end_time = HAL_GetTick() + CALIBRATION_TIME;
  while (end_time > HAL_GetTick())
    {
      TP_Read_Coordinates(rawCoord);
      if (rawCoord[2] < Xmin) { Xmin = rawCoord[2]; }
      if (rawCoord[2] > Xmax) { Xmax = rawCoord[2]; }
      if (rawCoord[3] < Ymin) { Ymin = rawCoord[3]; }
      if (rawCoord[3] > Ymax) { Ymax = rawCoord[3]; }
    }
  if ((Xmin < Xmax) && (Ymin < Ymax))
  {
      X_OFFSET = Xmin;
      Y_OFFSET = Ymin;
      X_MAGNITUDE = (Xmax - Xmin) / X_SIZE;
      Y_MAGNITUDE = (Ymax- Ymin) /  Y_SIZE;

      return CALIBRATION_OK;
  }
  return CALIBRATION_FAILS;
}

