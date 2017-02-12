#ifndef ILI9163_H
#define ILI9163_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "string.h"

extern volatile uint32_t MSec;


typedef enum WModesS{
	Dat,
	Reg,
} WModes;

typedef enum SleepModesS{
	Sleep,
	Awake,
} SleepModes;

/*
//0b XX 00(Red) 00(Green) 00(Blue)
//Lx = Light colour
//Dx = Dark colour
typedef enum Colours8B{
	//Greyscale
	White = 0b111111,
	DGrey = 0b010101,
	LGrey = 0b101010,
	Black = 0b000000,
	//Blues
	DBlue = 0b000001,
	Blue = 0b000010,
	LBlue = 0b000011,
	//Sky Blue
	DSBlue = 0b000101,
	SBlue = 0b001010,
	LSBlue = 0b001111,
	//Greens
	DGreen = 0b000100,
	Green = 0b001000,
	LGreen = 0b001100,
	//Reds
	DRed = 0b010000,
	Red = 0b100000,
	LRed = 0b110000,
	//Pinks
	DPink = 0b010001,
	Pink = 0b100010,
	LPink = 0b110011,
	//Yellow
	DYellow = 0b010100,
	Yellow = 0b101000,
	LYellow = 0b111100,
	Orange = 0b111000,
	Brown = 0b100100,
	Purple = 0b100010,
};
*/

/*
typedef enum Colours8B{
	Black = 0,
	Blue = 1,
	Red = 2,
	Magenta = 3,
	Green = 4,
	Cyan = 5,
	Yellow = 6,
	White = 7,
} Colours8;
*/

typedef enum Colours16B{
	Black = 0x0000,
	Blue = 0xF800,
	Red = 0x001F,
	Green = 0x07E0,
	White = 0xFFFF,
	Magenta = 0x8010,
	Cyan 	= 0x8400,
    Yellow =0x0410,
} Colours8;
/*
Black = 0x0000,
	Blue = 0x001F,
	Red = 0xF800,
	Green = 0x07E0,
	White = 0xFFFF,*/

typedef enum{
	DEG0 = 0,
	DEG90 = 1,
	DEG180 = 2,
	DEG270 = 3
} S_Rotation;

#define BKGCol White

#define CSPin 		GPIO_Pin_12
#define ResPin 		GPIO_Pin_8
#define AOPin 		GPIO_Pin_9

#define P_MOSI 	GPIO_Pin_15
#define P_CLK 		GPIO_Pin_13
#define BL_Pin 		GPIO_Pin_5

//#define XPix 128
//#define YPix 128
extern uint16_t XPix, YPix;

void Delay(uint32_t);

void ILI9163Init(void);

uint16_t EToS(uint8_t);

void SB(uint8_t, WModes);
void SW(uint16_t, WModes);

void SetAddr(uint8_t, uint8_t, uint8_t, uint8_t);
void SetScrn(Colours8);
void ClrScrn(void);
void WritePix(uint16_t, uint16_t,  Colours8);
void PCol(Colours8);

void SleepMode(uint8_t);
void InvMode(uint8_t);



#endif
