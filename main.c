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
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "BasicSerial3.h"

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
void uart_out(const char * str);
int esp8266_get_ack(const char c);

uint8_t wdt_counter = 0;
int negative = 0;
char * device_id;

const char command_0[] = "AT+CIPSTART=\"TCP\"";
const char command_1[] = "AT+CIPSEND=";
const char command_2[] = "AT+CIPCLOSE\r\n\n";
const char command_3[] = "AT+GSLP=294967294\r\n\n";

void disableWIFI(){
    uart_out(command_3);
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

void uart_out(const char * str){
    while(*str) {
        TxByte(*str++);
    }
}

char softuart_getchar(){
    return RxByte();
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
    TxByte('h');
	_delay_ms(DELAY_5_SEC);
	while (1) {
		if(getSensorType()){
            uart_out("Ready to send AM2302\r\n");
			if (readAM2302Data(SENSOR, data) == 0) {
				ftemperature = getTemperature(data);
				fhumidity = getHumidity(data);
				ftoa(ftemperature, tdata, 1);
				ftoa(fhumidity, hdata, 1);
				sendMessage(tdata, hdata);
			}
		}
		else{
            uart_out("Ready to send DS18B20\r\n");
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

char * getPort(){
    char * port = "5555";
    return port;
}

void connect_server(){
    uart_out(command_0);
    uart_out(",\"");
    uart_out(getIPAddress());
    uart_out("\",");
    uart_out(getPort());
    uart_out("\r\n\n");
       
}

void sendMessage(char tdata[], char hdata[]){
	char charStr[4];
    int i = 0;
    int charCounter=0;
    enableWIFI();
    esp8266_get_ack('P');
	//snprintf(msgString, sizeof msgString, "%s,\"%s\",%d\r\n\n", command_0,getIPAddress(),getPort());
	
    connect_server();
    
    /*uart_out("Awaiting input...\n");
    while(softuart_getchar() != 'h'){
    }
    uart_out("Got input\n");*/
	esp8266_get_ack('K');
    
    for(i=0; i<6; i++){
        if(tdata[i] == '\0'){
            break;
        }
        charCounter++;
    }
    for(i=0; i<6; i++){
        if(hdata[i] == '\0'){
            break;
        }
        charCounter++;
    }
    charCounter+=14;
    
    intToStr(charCounter, charStr, 0);
	//snprintf(msgString, sizeof msgString, "ID:%s;T:%s;H:%s;", device_id, tdata, hdata);
	//snprintf(msgString2, sizeof msgString2,"%s%d\r\n\n", command_1,strlen(msgString));
	//uart_out(msgString2);
    uart_out(command_1);
    uart_out(charStr);
    uart_out("\r\n\n");
	esp8266_get_ack('>');
	//uart_out(msgString);
    uart_out("ID:");
    uart_out(device_id);
    uart_out(";T:");
    uart_out(tdata);
    uart_out(";H:");
    uart_out(hdata);
    uart_out(";");
    esp8266_get_ack('K');
	uart_out(command_2);
    esp8266_get_ack('K');
    //_delay_ms(DELAY_1_SEC);
    disableWIFI();
    esp8266_get_ack('K');
}

int esp8266_get_ack(const char c){
    while(softuart_getchar() != c){
    }
    return 0;
}

int getSensorType(){
    return 1;
	//setdirection(PB1,IN);
	//return ((PINB & _BV(PB1)) >> PB1);
}

void initPins() {
	/*
	 *
	 *  pb3 - AM2302 / DS18B20 comms 
	 *  pb4 - OUT Clock (Debug)
	 *  pb0 - comms OUT
     *  pb1 - comms IN
     *  pb2 - WIFI RST
	 */
	//setdirection(PB2, OUT);
	//setdirection(PB1, IN);
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
