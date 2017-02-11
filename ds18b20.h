#define TPIN PINB
#define TDDR DDRB
#define IN 0
#define OUT 1

int readTempData_ds18b20(int data_pin, int *temperature);
uint8_t resetSensor();
//void OWBlock(unsigned char *data, int data_len);
//int OWTouchByte(int data);
int OWReadByte(void);
void OWWriteByte(int data);
uint8_t ds18b20_readBit();
void ds18b20_writeBit(uint8_t bit);
int readScratchpad(int bytes);
