#ifndef _I2C_LCD_H_
#define _I2C_LCD_H_

#include "stm32f10x_i2c.h"

//Endereço do módulo PCF8574
#define LCD_ADDR	0x20

//Define do tipo de LCD

//#define LCD_2004
#define LCD_1602


//void delay(uint32_t t);
#define LCD_I2C_pause	delay(1000)


#define PCF_P0	   0
#define PCF_P1 	1
#define PCF_P2	    2
#define PCF_P3	    3
#define PCF_P4   	4
#define PCF_P5   	5
#define PCF_P6	    6
#define PCF_P7	     7


#define DB4		PCF_P4
#define DB5		PCF_P5
#define DB6		PCF_P6
#define DB7		PCF_P7
#define EN		PCF_P2
#define RW		PCF_P1
#define RS		PCF_P0
#define BL		PCF_P3


uint8_t backlightState;


void LCD_I2C_Init(void);
void LCD_I2C_Backlight(uint8_t state);
void LCD_I2C_Goto(uint8_t row, uint8_t col);
void LCD_I2C_PrintC(const uint8_t *str);


void LCD_I2C_Send(uint8_t data);
void LCD_I2C_Command(uint8_t com);
void LCD_I2C_Data(uint8_t com);
void LCD_I2C_Clear(void);
void I2C_Configuration(void);
void delay(uint32_t t);
void delay_clear(__IO uint32_t num);


#endif
