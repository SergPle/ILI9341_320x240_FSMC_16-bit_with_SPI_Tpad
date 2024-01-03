//
//	ILI9341 + Resistive Touchscreen library for STM32
//
//	Sergey Plekhanov Copyright (c) 2024
//
//	This library is based on code
//	Matej Artnak Copyright (c) 2017
//	with optimizations and additions
//-----------------------------------
//-----------------------------------

#include "ILI9341_Touchscreen.h"
#include "stm32f4xx_hal.h"

//INITIAL HARDCODED CALIBRATION, CHANGE AND SAVE IF REQUIRED !

volatile uint16_t X_OFFSET = 4270;
volatile uint16_t Y_OFFSET = 3855;
volatile float X_MAGNITUDE = 0x433F;
volatile float Y_MAGNITUDE = 0x436E00F;

//Internal Touch pad command, do not call directly
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

//Internal Touch pad command, do not call directly
static void TP_Write(uint8_t value)
{
    uint8_t i = 0x08;

		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);	
	
    while(i)
    {

		HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, (value & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET ); // prepare bit of data

        value <<= 1;
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET); 	//  CLK pulse
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
        --i;
    }
}



//Read coordinates of touchscreen press. [0] = X, [1] = Y, [2] = RawX, [3] = RawY
uint8_t TP_Read_Coordinates(uint16_t Coordinates[])
{
    uint32_t samples = N_OF_POSITION_SAMPLES;
    uint16_t rawx, rawy = 0;
    uint32_t calculating_x, calculating_y = 0;
    uint32_t counted_samples = 0;
											// Prepare T-pad controller
		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);			// CS - High (inactively)
		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);		// CLK   - High
		HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_SET);		// MOSI  - High
		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_RESET); 		// CS - active

	
    while((samples)&&( ! HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN)))
    {			
        TP_Write(CMD_RDY);
		rawy = TP_Read();
		calculating_y += rawy;

        TP_Write(CMD_RDX);
		rawx = TP_Read();
		calculating_x += rawx;
        samples--;
	counted_samples++;
    }
		
	HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);   			//T-pad controller inactive
		
	if((counted_samples == N_OF_POSITION_SAMPLES)&&(HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == 0))
	{
	    calculating_x /= counted_samples;
	    calculating_y /= counted_samples;
	    rawx = calculating_x;
	    Coordinates[2] = rawx;
	    rawy = calculating_y;
	    Coordinates[3] = rawy;
		
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

// Touch pad calibration. Lets move stilus from upper left corner of screen to down right corner few time. You have CALIBRATION_TIME millisecond)).
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
      if (TP_Read_Coordinates(rawCoord))
	{
	  if (rawCoord[2] < Xmin) { Xmin = rawCoord[2]; }
	  if (rawCoord[2] > Xmax) { Xmax = rawCoord[2]; }
	  if (rawCoord[3] < Ymin) { Ymin = rawCoord[3]; }
	  if (rawCoord[3] > Ymax) { Ymax = rawCoord[3]; }
	}
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

