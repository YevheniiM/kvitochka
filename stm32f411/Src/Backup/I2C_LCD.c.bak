/*
 * I2C_LCD.c
 *
 *  Created on: Dec 2, 2018
 *      Author: Markiian
 */


#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "gpio.h"
#include "main.h"


volatile uint8_t LCD_ADDR = 0; //(uint8_t)(0x27 << 1);

#define PIN_RS    (1 << 0)
#define PIN_EN    (1 << 2)
#define BACKLIGHT (1 << 3)
volatile int LIGHT = BACKLIGHT;
#define NOBACKLIGHT (0x00)

#define I2C_variable (hi2c1)

#define LCD_DELAY_MS 5



HAL_StatusTypeDef LCD_SendInternal(uint8_t lcd_addr, uint8_t data, uint8_t flags) {
    HAL_StatusTypeDef res;
    for(;;) {
        res = HAL_I2C_IsDeviceReady(&I2C_variable, lcd_addr, 1,
                                    HAL_MAX_DELAY);
        if(res == HAL_OK)
            break;
    }

    uint8_t up = data & 0xF0;
    uint8_t lo = (data << 4) & 0xF0;

    uint8_t data_arr[4];
    data_arr[0] = up|flags|LIGHT|PIN_EN;
    data_arr[1] = up|flags|LIGHT;
    data_arr[2] = lo|flags|LIGHT|PIN_EN;
    data_arr[3] = lo|flags|LIGHT;

    res = HAL_I2C_Master_Transmit(&I2C_variable, lcd_addr, data_arr,
                                  sizeof(data_arr), HAL_MAX_DELAY);
    HAL_Delay(LCD_DELAY_MS);
    return res;
}

void search_I2C_bus(I2C_HandleTypeDef i2c){
	// This function scan I2C bus
	// It need semihosting or another tool for printf
	printf("Scanning I2C bus:\r\n");
	HAL_StatusTypeDef result;
	uint8_t i;
	 	for (i=1; i<128; i++)
	 	{
	 	  result = HAL_I2C_IsDeviceReady(&i2c, (uint16_t)(i<<1), 2, 2);
	 	  if (result != HAL_OK)
	 	  {
	 		  printf(".");
	 	  }
	 	  if (result == HAL_OK)
	 	  {
	 		  printf("0x%X", i);
	 	  }
	 	}
	 	printf("\r\n");
}

void LCD_SendCommand(uint8_t lcd_addr, uint8_t cmd) {
    LCD_SendInternal(lcd_addr, cmd, 0);
}

void LCD_SendData(uint8_t lcd_addr, uint8_t data) {
    LCD_SendInternal(lcd_addr, data, PIN_RS);
}

void LCD_Init(uint8_t lcd_addr, I2C_HandleTypeDef i2c_variable) {
	I2C_variable = i2c_variable;
    // 4-bit mode, 2 lines, 5x7 format
    LCD_SendCommand(lcd_addr, 0b00110000);
    // display & cursor home (keep this!)
    LCD_SendCommand(lcd_addr, 0b00000010);
    // display on, right shift, underline off, blink off
    LCD_SendCommand(lcd_addr, 0b00001100);
    // clear display (optional here)
    LCD_SendCommand(lcd_addr, 0b00000001);
}

void LCD_SendString(uint8_t lcd_addr, char *str) {
    while(*str) {
        LCD_SendData(lcd_addr, (uint8_t)(*str));
        str++;
    }
}

void LCD_Clear(){
	LCD_SendCommand(LCD_ADDR, 0b00000001);
}

void LCD_AddChar(int location, char* c){
	LCD_SendCommand(LCD_ADDR, (0x40 | (location << 3)));
		for (int i=0; i<8; i++) {
				LCD_SendData(LCD_ADDR,c[i]);
			}
}

void LCD_Print(int row, char* c) {
	if (!row) {
		 LCD_SendCommand(LCD_ADDR, 0b10000000);
	}
	if(row==1) {
		 LCD_SendCommand(LCD_ADDR, 0b11000000);
	}
	if(row==2) {
			 LCD_SendCommand(LCD_ADDR, 0b10000000|20);
	}
	if(row==3) {
			 LCD_SendCommand(LCD_ADDR, 0b11000000|20);
	}

	LCD_SendString(LCD_ADDR, c);
}

void LCD_PPrint(int row, int column, char* c){
	if (!row) {
		 LCD_SendCommand(LCD_ADDR, 0b10000000|column);
	}
	if(row==1) {
		LCD_SendCommand(LCD_ADDR, 0b11000000|column);
	}
	if(row==2) {
			 LCD_SendCommand(LCD_ADDR, 0b10000000|(column + 20));
	}
	if(row==3) {
			 LCD_SendCommand(LCD_ADDR, 0b11000000|(column + 20));
	}
	LCD_SendString(LCD_ADDR, c);
}

void search_I2C_bus_without_semihosting(I2C_HandleTypeDef i2c_Variable){
	// This function scan I2C bus and print address on display
	// You can work only with one I2C controller
		HAL_StatusTypeDef result;
		uint8_t i;
		 for (i=1; i<128; i++)
		 	{
		 	  result = HAL_I2C_IsDeviceReady(&i2c_Variable, (uint16_t)(i<<1), 2, 2);
		 	  if (result == HAL_OK)
		 	  {
			 	 LCD_Init(i<<1, i2c_Variable);
			 	 LCD_ADDR = i << 1;
			 	 LCD_SendString(LCD_ADDR, "Adress is: 0x");
			 	 LCD_SendInt(i, 16);
		 	  }
		 	}
		 	//printf("\r\n");
}

void LCD_SendChar(int row, int column, uint8_t c){
	if (row) {
				 LCD_SendCommand(LCD_ADDR, 0b11000000|column);
			} else {
				 LCD_SendCommand(LCD_ADDR, 0b10000000|column);
			}
		LCD_SendData(LCD_ADDR, c);
}

void LCD_SendInt(int number, int base){
	char sentence[10];
	itoa(number, sentence, base);
	LCD_SendString(LCD_ADDR, sentence);
}

void LCD_PrintInt(int row, int number, int base){
	char sentence[10];
	itoa(number, sentence, base);
	LCD_Print(row, sentence);
}
void  LCD_PPrintInt(int row, int column, int number, int base){
	char sentence[10];
	//itoa(number, sentence, base);
	//LCD_PPrint(row, column, sentence);
	sprintf(sentence, "%3ld", number);
	LCD_PPrint(row, column, sentence);
	//LCD_SendString(LCD_ADDR, sentence);
}

void LCD_SendFloat(float number){
	char sentence[10];
	sprintf(sentence, "%g", number);
	LCD_SendString(LCD_ADDR, sentence);
}

void LCD_PrintFloat(int row, float number){
	char sentence[10];
	sprintf(sentence, "%g", number);
	LCD_Print(row, sentence);
}
void LCD_PPrintFloat(int row, int column, float number){
	char sentence[10];
	sprintf(sentence, "%g", number);
	LCD_PPrint(row, column, sentence);
}


void LCD_SendDouble(double number){
	char sentence[10];
	sprintf(sentence, "%g", number);
	LCD_SendString(LCD_ADDR, sentence);
}

void LCD_PrintDouble(int row, int number){
	char sentence[10];
	sprintf(sentence, "%g", number);
	LCD_Print(row, sentence);
}
void  LCD_PPrintDouble(int row, int column, double number){
	char sentence[10];
	sprintf(sentence, "%g", number);
	LCD_PPrint(row, column, sentence);
}

void LCD_PowerOn(){
	LIGHT = BACKLIGHT;
	LCD_SendCommand(0x27<<1, 0b00001100);
}

void LCD_PowerOff(){
	LIGHT = NOBACKLIGHT;
	LCD_SendCommand(0x27<<1, 0b00001000);
}

void LCD_POWER_SAVING_MODE(){
	LIGHT = NOBACKLIGHT;
	LCD_SendCommand(0x27<<1, 0b00000000);
}

void LCD_NOPOWER_SAVING_MODE(){
	LIGHT = BACKLIGHT;
	LCD_SendCommand(0x27<<1, 0b00000000);
}
