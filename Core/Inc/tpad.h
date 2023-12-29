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

#define TOUCH_SCALE_X 320
#define TOUCH_SCALE_Y 240

void TpadInit();

void TpadDeInit();

bool PenPresent();

bool TpadGetCoordinates(uint16_t& Xcoord, uint16_t& Ycoord);

#endif /* SRC_TPAD_H_ */
