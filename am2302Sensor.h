#define IN 0
#define OUT 1
#define MAX 85

int readAM2302Data(int data_pin, uint8_t * data);
uint8_t readBit(int data_pin);
int readByte(int data_pin);
uint8_t * getMeasurement();
float getTemperature(uint8_t * data);
float getHumidity(uint8_t * data);
