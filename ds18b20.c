#include <avr/io.h>
#include <util/delay.h>
#include "ds18b20.h"
#include "gpio.h"

int TDATA;

int readTempData_ds18b20(int data_pin, int *temperature) {
	TDATA=data_pin;
	if(resetSensor() != 0){
		return 1;
	}
	OWWriteByte(0xCC); //skip rom

	OWWriteByte(0x44); //convert T
	_delay_ms(800);
	if(resetSensor() != 0){
			return 1;
	}
	OWWriteByte(0xCC); //skip rom
	OWWriteByte(0xBE); //read scratchpad
	*temperature = readScratchpad(2);
	//*temperature = 0xFC90;
	return 0;
}

uint8_t resetSensor() {
	uint8_t result;
	//_delay_ms(2000);
	setdirection(TDATA, OUT); //PB1 output
	setpin(TDATA, 1); //PB1 high VDD init
	_delay_ms(100);
	setpin(TDATA, 0);
	_delay_us(490);
	setdirection(TDATA, IN);
	_delay_us(70);
	result = (TPIN & (1 << TDATA));
	_delay_us(410);
	return result;
}

void ds18b20_writeBit(uint8_t bit) {
	setdirection(TDATA, OUT);
	if (bit) {
		setpin(TDATA, 0);
		_delay_us(6);
		setpin(TDATA, 1);
		_delay_us(64);
	} else {
		setpin(TDATA, 0);
		_delay_us(60);
		setpin(TDATA, 1);
		_delay_us(10);
	}
}

uint8_t ds18b20_readBit() {
	uint8_t result;
	setdirection(TDATA, OUT);
	setpin(TDATA,0);
	_delay_us(6);
	setpin(TDATA,1);
	_delay_us(9);
	setdirection(TDATA, IN);
	result = TPIN & (1 << TDATA);
	_delay_us(55);
	return result;
}

void OWWriteByte(int data)
{
        int loop;

        // Loop to write each bit in the byte, LS-bit first
        for (loop = 0; loop < 8; loop++)
        {
                ds18b20_writeBit(data & 0x01);

                // shift the data byte for the next bit
                data >>= 1;
        }
}

//-----------------------------------------------------------------------------
// Read 1-Wire data byte and return it
//
int OWReadByte(void)
{
        int loop, result=0;

        for (loop = 0; loop < 8; loop++)
        {
                // shift the result to get it ready for the next bit
                result >>= 1;

                // if result is one, then set MS bit
                if (ds18b20_readBit())
                        result |= 0x80;
        }
        return result;
}

//-----------------------------------------------------------------------------
// Write a 1-Wire data byte and return the sampled result.
//
/*int OWTouchByte(int data)
{
        int loop, result=0;

        for (loop = 0; loop < 8; loop++)
        {
                // shift the result to get it ready for the next bit
                result >>= 1;

                // If sending a '1' then read a bit else write a '0'
                if (data & 0x01)
                {
                        if (ds18b20_readBit())
                                result |= 0x80;
                }
                else
                        ds18b20_writeBit(0);

                // shift the data byte for the next bit
                data >>= 1;
        }
        return result;
}*/

//-----------------------------------------------------------------------------
// Write a block 1-Wire data bytes and return the sampled result in the same
// buffer.
//
/*void OWBlock(unsigned char *data, int data_len)
{
        int loop;

        for (loop = 0; loop < data_len; loop++)
        {
                data[loop] = OWTouchByte(data[loop]);
        }
}
*/
int readScratchpad(int bytes){
	//int byte[2];
	int result = 0;
	int i = 0;
	int intermediate = 0;
	for(i = 0; i < bytes; i++){
		intermediate = OWReadByte();
		//byte[i] = intermediate;
		result |= intermediate << (8*i);
	}
	/*
	resetSensor();
	_delay_ms(2000);
	OWWriteByte(byte[0]);
	OWWriteByte(byte[1]);
	*/
	return result;
}

