/*
 * main.c
 *
 *  Created on: Jun 20, 2016
 *      Author: d
 */
//#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "gpio.h"
#include "am2302Sensor.h"
#include "ds18b20.h"
#include <string.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "softuart.h"

#define WIFI_RST PB2
#define SENSOR PB3
#define DELAY_5_SEC 5000
#define DELAY_1_SEC 1000
#define DELAY_100_MSEC 100

void initPins();
void ftoa(float n, char *res, int afterpoint);
int intToStr(int x, char str[], int d);
void reverse(char *str, int len);
void setupWatchdog();
void sendMessage(char tdata[], char hdata[]);
int getSensorType();
char * getDeviceID();
void enableWIFI();
void disableWIFI();

uint8_t wdt_counter = 0;
int negative = 0;
char * device_id;

const char command_0[] = "AT+CIPSTART=\"TCP\"";
const char command_1[] = "AT+CIPSEND=";
const char command_2[] = "AT+CIPCLOSE\r\n\n";
const char command_3[] = "AT+GSLP=294967294\r\n\n";

void disableWIFI(){
    softuart_puts(command_3);
}

void enableWIFI(){
    setpin(WIFI_RST,0);
    _delay_ms(DELAY_100_MSEC);
    setpin(WIFI_RST,1);
}

void setupWatchdog() {
	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	MCUSR = 0;
	WDTCR = (1 << WDCE) | (1 << WDE);
	WDTCR = 0;
	sei();
}

ISR(WDT_vect) {
// Flip bit 3 of PORTB
	//PORTB ^= (1);
	/* ReEnable the watchdog interrupt, as this gets reset when entering this ISR and automatically enables the WDE signal that resets the MCU the next time the  timer overflows */
	WDTCR |= (1 << WDIE);
	wdt_counter++;
}

int main(void) {
	uint8_t data[5];
	float ftemperature = 0;
	float fhumidity = 0;
	char hdata[6];
	char tdata[6];

	//float temp = 0;
	int ds_temp = 0;
	device_id = getDeviceID();
	initPins();
	_delay_ms(DELAY_5_SEC);

	while (1) {
		if(getSensorType()){
			if (readAM2302Data(SENSOR, data) == 0) {
				ftemperature = getTemperature(data);
				fhumidity = getHumidity(data);
				ftoa(ftemperature, tdata, 1);
				ftoa(fhumidity, hdata, 1);
				sendMessage(tdata, hdata);
			}
		}
		else{
			if (readTempData_ds18b20(SENSOR,&ds_temp) == 0) {
				if (ds_temp & 0x8000) {
					ds_temp=0x10000-ds_temp;
					negative=1;
				} else {
					negative=0;
				}
				ftemperature = ds_temp / 16.0;
				ftoa(ftemperature, tdata, 1);
				ftoa(0.0, hdata,1);
				sendMessage(tdata,hdata);
			}
		}
		//_delay_ms(1000);
		setupWatchdog();
		WDTCR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); //8 sec
		while (wdt_counter < 16) { //8 sec * x
			WDTCR |= (1 << WDIE);
			sleep_mode();
		}
		wdt_counter = 0;
		WDTCR |= (0 << WDIE);
		

	}
}

char * getDeviceID(){
	char * devID = "AFFE";
	return devID;
}

char * getIPAddress(){
    char * ipAddress = "192.168.0.103";
    return ipAddress;
}

int getPort(){
    int port = 5555;
    return port;
}

void sendMessage(char tdata[], char hdata[]){
	char msgString[64];
	char msgString2[64];

    enableWIFI();
    _delay_ms(DELAY_5_SEC);
    
	snprintf(msgString, sizeof msgString, "%s,\"%s\",%d\r\n\n", command_0,getIPAddress(),getPort());
	//snprintf(msgString, sizeof msgString, "T:%s;H:%s;", tdata, hdata);
	//snprintf(msgSize, sizeof msgSize, "%d", strlen(msgString));
	softuart_init();
	softuart_puts(msgString);
	//softuart_puts("AT+CIPSTART=\"TCP\",\"192.168.0.101\",5555\r\n\n");
	_delay_ms(DELAY_5_SEC);
	snprintf(msgString, sizeof msgString, "ID:%s;T:%s;H:%s;", device_id, tdata, hdata);
	snprintf(msgString2, sizeof msgString2,"%s%d\r\n\n", command_1,strlen(msgString));
	softuart_puts(msgString2);
	_delay_ms(DELAY_1_SEC);
	softuart_puts(msgString);
	/*softuart_puts("ID:");
	softuart_puts(device_id);
	softuart_puts("T:");
	if(negative){
		softuart_puts("-");
	}
	softuart_puts(tdata);
	softuart_puts(";H:");
	softuart_puts(hdata);
	softuart_puts(";");
	softuart_puts("");*/
	_delay_ms(DELAY_1_SEC);
	softuart_puts(command_2);
    
    _delay_ms(DELAY_1_SEC);
    disableWIFI();
}

int getSensorType(){
	setdirection(PB1,IN);
	return ((PINB & _BV(PB1)) >> PB1);
}

void initPins() {
	/*
	 *
	 *  pb1 - Sensor select
	 *  pb3 - AM2302 / DS18B20 comms 
	 *  pb4 - OUT Clock
	 *  pb0 - comms out
	 */
	//setdirection(PB2, OUT);
	setdirection(PB1, IN);
	setdirection(SENSOR, OUT);
    setdirection(WIFI_RST, OUT);
	//setdirection(PB0, OUT);
	//setpin(PB1, 0);
	setpin(SENSOR, 1);
    setpin(WIFI_RST,1);
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d) {
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}

	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		str[i++] = '0';

	reverse(str, i);
	str[i] = '\0';
	return i;
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len) {
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint) {
	// Extract integer part
	int ipart = (int) n;

	// Extract floating part
	float fpart = n - (float) ipart;

	// convert integer part to string
	int i = intToStr(ipart, res, 0);

	// check for display option after point
	if (afterpoint != 0) {
		res[i] = '.';  // add dot

		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);

		intToStr((int) fpart, res + i + 1, afterpoint);
	}
}
