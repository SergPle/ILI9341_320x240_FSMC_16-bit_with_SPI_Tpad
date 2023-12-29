/*
 * tpad.cpp
 *
 *  Created on: Dec 29, 2023
 *      Author: Сергей
 */
#include "tpad.h"

void TpadInit(){

  if (HAL_SPI_DeInit(&SPI_PORT) != HAL_OK){
     Error_Handler();
   }

    SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;

    if(HAL_SPI_Init(&SPI_PORT) != HAL_OK){
	Error_Handler();
      };
}

void TpadDeInit(){

  if (HAL_SPI_DeInit(&SPI_PORT) != HAL_OK)
   {
     Error_Handler();
   }
}

bool PenPresent(){
  if(HAL_GPIO_ReadPin(PEN_PRESENT_GPIO_Port, PEN_PRESENT_Pin) == 0){
      return true;
  }
  return false;
}
