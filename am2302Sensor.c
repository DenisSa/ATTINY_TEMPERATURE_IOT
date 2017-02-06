#include <util/delay.h>
#include <avr/io.h>
#include "gpio.h"
#include "am2302Sensor.h"
#include <stdlib.h>
#include <avr/interrupt.h>
//#define F_CPU 8000000UL
uint8_t data[5] = { 0 };
uint64_t sensor_reading = 0;

float getTemperature(uint8_t * data) {
	uint16_t temperature;

	float result = 0.0;

	temperature = data[2];
	temperature <<= 8;
	temperature |= data[3];

	result = temperature / 10.0;
	return result;
}

float getHumidity(uint8_t * data) {
	uint16_t humidity;

	float result = 0.0;

	humidity = data[0];
	humidity <<= 8;
	humidity |= data[1];

	result = humidity / 10.0;
	return result;
}

int readAM2302Data(int data_pin, uint8_t * data) {
	uint8_t i = 0;
	uint8_t lastBit = 0;
	uint16_t counter = 0;
	uint8_t chksum = 0;
	//uint8_t returnVal = 0;
	cli();
	setdirection(data_pin, OUT);

	//uint8_t result=0;
	setdirection(data_pin, OUT);
	_delay_us(1000);
	setpin(data_pin, 0);
	//setpin(PB1, 0);
	_delay_us(1500);
	setpin(data_pin, 1);
	//setpin(PB1, 1);
	_delay_us(40);
	setdirection(data_pin, IN);
	_delay_us(200);

	for (i = 0; i < 40; i++) {
		_delay_us(26);
		//setpin(PB1, 1); //4us
		counter = 0;
		lastBit = readBit(data_pin);
		if (i != 0) {
			sensor_reading <<= 1;
		}
		sensor_reading |= lastBit;

		if (lastBit == 1) {
			_delay_us(60);
			lastBit = 0;
		}
		//setpin(PB1, 0);
		while (!readBit(data_pin)) {
			counter++;
			if (counter >= 255) {
				break;
			}
		}
		//_delay_us();
	}

	data[0] = (sensor_reading >> 32) & 0x7F;
	data[1] = (sensor_reading >> 24) & 0xFF;
	data[2] = (sensor_reading >> 16) & 0xFF;
	data[3] = (sensor_reading >> 8) & 0xFF;
	data[4] = (sensor_reading) & 0xFF;

	chksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	sei();
	if (data[4] == chksum) {
		return 0;
	}
	return -1;
}
