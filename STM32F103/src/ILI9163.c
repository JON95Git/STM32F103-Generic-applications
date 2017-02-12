#include "ILI9163.h"
#include "GFXC.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"

uint16_t XPix = 128;
uint16_t YPix = 160;
uint16_t DataSPI;

void ILI9163Init(void)
{

		GPIO_InitTypeDef GPIO_InitStructure;
		SPI_InitTypeDef SPI_InitStructure;

		//Init SPI ILI9163
		GPIO_InitStructure.GPIO_Pin =  ResPin | AOPin |BL_Pin | CSPin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = P_MOSI | P_CLK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);


		SPI_I2S_DeInit(SPI2);

	    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		///Modificado 8 para 16
	    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
		SPI_InitStructure.SPI_CRCPolynomial = 10;
		SPI_Init(SPI2, &SPI_InitStructure);

		//SPI_I2S_DMACmd (SPI2, SPI_I2S_DMAReq_Tx, ENABLE );


		SPI_Cmd(SPI2, ENABLE);

		GPIO_SetBits(GPIOB, BL_Pin);
		GPIO_SetBits(GPIOB, ResPin);
		Delay(10);
		GPIO_ResetBits(GPIOB, ResPin);
		Delay(10);
		GPIO_SetBits(GPIOB, ResPin);
		Delay(10);
		GPIO_ResetBits(GPIOB, CSPin);


		SB(0x01, Reg); //Software reset
		Delay(10);
		SB(0x11, Reg); //Exit Sleep
		Delay(120);

		SB(0x26, Reg); //Set default gamma
		SB(0x04, Dat);

		SB(0xB1, Reg); //Set Frame Rate
		SB(0x08, Dat);// DIVA = 8
		SB(0x08, Dat);// VPA = 9

		SB(0xC0, Reg); //Set Power Control 1
		SB(0x1F, Dat);
		//SB(0x04, Dat);

		SB(0xC1, Reg); //Set Power Control 2
		SB(0x00, Dat);
		//AVDD: 2xVCI1
		//VCL: -VCI1
		//VGH: 5xVCI1
		//VGL: 5xVCI1
		//VGH-VGL<=32v!

		SB(0xC2, Reg); //Set Power Control 3
		SB(0x00, Dat);
		SB(0x07, Dat);

		SB(0xC3, Reg); //Set Power Control 4 (Idle mode)
		SB(0x00, Dat);
		SB(0x07, Dat);

		SB(0xC5, Reg); //Set VCom Control 1
		SB(0x24, Dat); // VComH = 3v
		SB(0xC8, Dat); // VComL = 0v

		SB(0x36, Reg); //Set Memory access mode
		SB(0x08, Dat);

		RotateScreen(DEG180, 0);

		SB(0x13, Reg); //Normal mode on

		SB(0x3A, Reg); //Set pixel mode
		SB(0x05, Dat);

		SB(0x29, Reg); //Display on

		InvMode(0);
		ClrScrn();
}

void WriteString (char string, Colours8 background, Colours8 txt, char x, char y, char size, char select)
{
		static TextParamStruct TS;
		 TS.Size = size;
		 TS.Font = Font1;
		 TS.XPos = x;
		 TS.YPos = y;
		 TS.TxtCol = txt;
		 TS.BkgCol = background;
		 	 if ( select == 1)
		 	 	 	 {
		 		 	 	 PStr((char)string, &TS);
		 	 	 	 }
		 	 else PChar(string, &TS);
}

void DrawCircle (Colours8 fill, Colours8 line, char xs, char ys, char xe, char ye, char radius)
{
		static ShapeParamStruct S;

		S.Fill = ENABLE;  //Preenchimento
		S.LineCol = line;
		S.FillCol = fill;
		S.Thickness = 25;
		S.XSPos = xs;
		S.YSPos = ys;
		S.XEPos = xe;
		S.YEPos = ye;
		S.Radius = radius;
		S.SemiUD = SemiU;
		S.Dashed = ENABLE;  //Tracejadas
		Circle(S);
}

void Delay(uint32_t MS)
{
	volatile uint32_t MSS = MSec;

	while((MSec - MSS)<MS) asm volatile ("nop");
}

uint16_t EToS(uint8_t Col)
{
	uint16_t Temp = 0;

	/* 8 bit
	Temp |= (Col&3)<<3;
	Temp |= ((Col>>2)&7)<<8;
	Temp |= (Col>>5)<<13;
	 */

	Temp |= (Col&3)<<3;
	Temp |= ((Col>>2)&3)<<9;
	Temp |= ((Col>>4)&3)<<14;

	return Temp;
}

void SB(uint8_t Data, uint8_t DR)
{

	//SPI_I2S_DMACmd (SPI2, SPI_I2S_DMAReq_Tx, ENABLE );


	DataSPI = Data;

	if	(DR == Dat)

			GPIO_SetBits(GPIOB, AOPin);

	else
			GPIO_ResetBits(GPIOB, AOPin);


		SPI_I2S_SendData(SPI2, DataSPI);


		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);


}

void SW(uint16_t Data, uint8_t DR)
{

	DataSPI = Data>>8;

	if (DR == Dat)

		GPIO_SetBits(GPIOB, AOPin);

	else

	GPIO_ResetBits(GPIOB, AOPin);

	SPI_I2S_SendData(SPI2, DataSPI);

	//while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
//________________________________________________________________________

	DataSPI = Data;

	SPI_I2S_SendData(SPI2, DataSPI);

	//while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);

}

void SetAddr(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2)
{

	SB(0x2A, Reg); //Set column address
	SB(0x00, Dat);
	SB(X1, Dat);
	SB(0x00, Dat);
	SB(X2, Dat);

	SB(0x2B, Reg);
	SB(0x00, Dat);
	SB(0+Y1, Dat); // SB(32+Y1, Dat);
	SB(0x00, Dat);
	SB(0+Y2, Dat);  // SB(32+Y2, Dat);

	SB(0x2C, Reg);
}

void SetScrn(Colours8 Colour)
{
	uint16_t XCnt, YCnt;

	SetAddr(0, 0, XPix-1, YPix-1);

	for(XCnt = 0; XCnt < XPix; XCnt++)
	{
		for(YCnt = 0; YCnt < YPix; YCnt++)
		{
			SW(Colour, Dat);
		}
	}
}

void ClrScrn(void)
{
	SetScrn(BKGCol);
}

void WritePix(uint16_t X, uint16_t Y, Colours8 Colour)
{
	SetAddr(X, Y, X, Y);
	//PCol(Colour);
	SW(Colour, Dat);
}

void PCol(Colours8 Colour)
{

	switch(Colour){
	case Black:
		SW(0x0000, Dat);
		break;
	case Blue:
		SW(0x0010, Dat);
		break;
	case Red:
		SW(0x8000, Dat);
		break;
	case Magenta:
		SW(0x8010, Dat);
		break;
	case Green:
		SW(0x0400, Dat);
		break;
	case Cyan:
		SW(0x0410, Dat);
		break;
	case Yellow:
		SW(0x8400, Dat);
		break;
	case White:
		SW(0x8410, Dat);
		break;
	}

}

void SleepMode(uint8_t Mode)
{
	if (Mode == Sleep)

		SB(0x10, Reg);

	else
		SB(0x11, Reg);
	    Delay(120);
}

void InvMode(uint8_t Mode)
{
	if (Mode != 1)

		SB(0x20, Reg);

	else
		SB(0x21, Reg);

}

void RotateScreen(S_Rotation R, uint8_t Mirror)
{

	uint8_t RGB = 0;

#ifdef RGB_BACKWARD
	RGB = (1<<3);
#endif

	if(!Mirror)
		{
			switch(R)
				{
				case DEG0:
				SB(0x36, Reg); //Set Memory access mode
				SB(0x00|RGB, Dat);
				XPix = 128;
				YPix = 160;
				break;

				case DEG90:
				SB(0x36, Reg); //Set Memory access mode
				SB(0x60|RGB, Dat);
				XPix = 160;
				YPix = 128;
				break;

				case DEG180:
				SB(0x36, Reg); //Set Memory access mode
				SB(0xC0|RGB, Dat);
				XPix = 128;
				YPix = 160;
				break;

				case DEG270:
				SB(0x36, Reg); //Set Memory access mode
				SB(0xA0|RGB, Dat);
				XPix = 160;
				YPix = 128;
				break;
				}
		}

	else
		{
			switch(R)
				{
				case DEG0:
				SB(0x36, Reg); //Set Memory access mode
				SB(0x40|RGB, Dat);
				XPix = 128;
				YPix = 160;
				break;

				case DEG90:
				SB(0x36, Reg); //Set Memory access mode
				SB(0xF0|RGB, Dat);
				XPix = 160;
				YPix = 128;
				break;

				case DEG180:
				SB(0x36, Reg); //Set Memory access mode
				SB(0x80|RGB, Dat);
				XPix = 128;
				YPix = 160;
				break;

				case DEG270:
				SB(0x36, Reg); //Set Memory access mode
				SB(0x20|RGB, Dat);
				XPix = 160;
				YPix = 128;
				break;
				}
		}
}

