/*
 * tpad.h
 *
 *  Created on: Dec 29, 2023
 *      Author: Сергей
 */

#ifndef SRC_TPAD_H_
#define SRC_TPAD_H_

#include "main.h"
#include <stdbool.h>

#define READ_X 0xD0
#define READ_Y 0x90

#define SPI_PORT hspi2
extern  SPI_HandleTypeDef SPI_PORT;

#define ILI9341_TOUCH_SCALE_X 240
#define ILI9341_TOUCH_SCALE_Y 320

void TpadInit();

void TpadDeInit();

bool PenPresent();



#endif /* SRC_TPAD_H_ */
