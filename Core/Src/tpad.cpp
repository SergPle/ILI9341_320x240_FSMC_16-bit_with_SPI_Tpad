/*
 * tpad.cpp
 *
 *  Created on: Dec 29, 2023
 *      Author: Сергей
 */
#include "tpad.h"
#include "stm32f4xx_hal.h"

void TpadInit(){
  HAL_GPIO_WritePin(TPAD_EN_GPIO_Port, TPAD_EN_Pin, GPIO_PIN_SET );
  HAL_Delay(10);
  if (HAL_SPI_DeInit(&SPI_PORT) != HAL_OK){
     Error_Handler();
   }
    SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    if(HAL_SPI_Init(&SPI_PORT) != HAL_OK){
	Error_Handler();
      };
    HAL_GPIO_WritePin(TPAD_EN_GPIO_Port, TPAD_EN_Pin, GPIO_PIN_RESET );
}

void TpadDeInit(){
  HAL_GPIO_WritePin(TPAD_EN_GPIO_Port, TPAD_EN_Pin, GPIO_PIN_SET );
  if (HAL_SPI_DeInit(&SPI_PORT) != HAL_OK)
   {
     Error_Handler();
   }
}

bool TpadPenPresent(){
  if(HAL_GPIO_ReadPin(PEN_PRESENT_GPIO_Port, PEN_PRESENT_Pin) == 0){
      return true;
  }
  return false;
}

bool TpadGetCoordinates(uint16_t& Xcoord, uint16_t& Ycoord){

  uint8_t  ReadXcom[1] = {0xD0},
	   ReadYcom[1] = {0x90};
  uint16_t rawX = 0,
	   rawY = 0;
  uint8_t  sample = 8;
  uint8_t _x[2], _y[2];


  for(uint8_t i = 0; i < sample; ++i){
      if(! TpadPenPresent()) {
	  return false;} // no pen detected

      HAL_SPI_Transmit(&SPI_PORT,ReadXcom, 1, HAL_MAX_DELAY);
      HAL_SPI_Receive(&SPI_PORT, _x, 2, HAL_MAX_DELAY);

      HAL_SPI_Transmit(&SPI_PORT,ReadYcom, 1, HAL_MAX_DELAY);
      HAL_SPI_Receive(&SPI_PORT, _y, 2, HAL_MAX_DELAY);
      rawX += (((uint16_t) _x[0]) << 8) | ((uint16_t) _x[1]);
      rawY += (((uint16_t) _y[0]) << 8) | ((uint16_t) _y[1]);
  }
  if (sample == 0) {
      return false;}
  Xcoord = rawX / sample;
  Ycoord = rawY / sample;
  return true;
}
