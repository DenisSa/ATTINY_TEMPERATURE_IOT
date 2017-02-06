/*
 * gpio.h
 *
 *  Created on: Jun 21, 2016
 *      Author: d
 */

#ifndef GPIO_H_
#define GPIO_H_

void setpin(int pn, int val);
void setdirection(int pn, int val);
uint8_t readBit(int data_pin);

#endif /* GPIO_H_ */
