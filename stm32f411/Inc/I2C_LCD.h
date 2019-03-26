/*
 * I2C_LCD.h
 *
 *  Created on: Dec 2, 2018
 *      Author: Markiian
 */

#ifndef I2C_LCD_H_
#define I2C_LCD_H_




HAL_StatusTypeDef LCD_SendInternal(uint8_t lcd_addr, uint8_t data, uint8_t flags);
void search_I2C_bus(I2C_HandleTypeDef i2c_variable);
void search_I2C_bus_without_semihosting(I2C_HandleTypeDef i2c_variable);
void LCD_SendCommand(uint8_t lcd_addr, uint8_t cmd);
void LCD_SendData(uint8_t lcd_addr, uint8_t data);
void LCD_Init(uint8_t lcd_addr, I2C_HandleTypeDef i2c_variable);
void LCD_SendString(uint8_t lcd_addr, char *str);
void LCD_Clear();
void LCD_AddChar(int location, char* c);
void LCD_PPrint(int row, int column, char* c);
void LCD_SendChar(int row, int column, char c);
void LCD_Print(int row, char* c);
void LCD_SendInt(int number, int base);
void LCD_PrintInt(int row, int number, int base);
void LCD_PPrintInt(int row, int column, int number, int base);
void LCD_SendFloat(float number);
void LCD_PrintFloat(int row, float number);
void LCD_PPrintFloat(int row, int column, float number);
void LCD_SendDouble(double number);
void LCD_PrintDouble(int row, int number);
void LCD_PPrintDouble(int row, int column, double number);
void LCD_PowerOn();
void LCD_PowerOff();
void LCD_POWER_SAVING_MODE();
void LCD_NOPOWER_SAVING_MODE();

extern volatile uint8_t LCD_ADDR;

#endif /* I2C_LCD_H_ */
