#include <avr/eeprom.h>

uint8_t EEMEM dev_id = 1;
uint32_t EEMEM ip_addr = 0xC0A80067; //192.168.0.103
uint16_t EEMEM ip_port = 5555;
uint8_t EEMEM wifi_username[32];
uint8_t EEMEM wifi_password[32];
uint8_t EEMEM sensor_type = 1;
uint8_t EEMEM report_frequency = 1;
