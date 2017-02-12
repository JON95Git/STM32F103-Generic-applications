/**
  ******************************************************************************
  * @file    LCD_I2C.c
  * @author  Adaptado de um autor russo desconhecido
  * @date    11/2016
  * @brief   O presente módulo apresenta as funções de controle do display
  * 		 lcd (HD44780) pelo controlador I2C PCF8574
  ******************************************************************************
  */
#include "LCD_I2C.h"
#include "stm32f10x_i2c.h"

uint8_t backlightState = 1;


//Função:     Vai até a linha-coluna desejada
//Parâmetros: linha, coluna
//Retorno:    void
void LCD_I2C_Goto(uint8_t row, uint8_t col)
{
    #ifdef LCD_1602
        switch (row)
            {
		    case 1:
			    LCD_I2C_Command(0x80 + col);
			break;

		    case 2:
			    LCD_I2C_Command(0x80 + col + 0x40);
			break;

		    case 3:
			    LCD_I2C_Command(0x80 + col + 0x14);
			break;

		    case 4:
			    LCD_I2C_Command(0x80 + col + 0x54);
			break;
	       }
    #endif
}


//Função:     Escreve uma string
//Parâmetros: ponteiro de string
//Retorno:    void
void LCD_I2C_PrintC(const uint8_t *str)
{
    uint8_t i;

    while (i = *str++)
 	    {
        LCD_I2C_Data(i);
 	    }
}


//Função:     Inicializa o LCD
//Parâmetros: void
//Retorno:    void
void LCD_I2C_Init(void)
{
    LCD_I2C_Command(0x33);
    LCD_I2C_pause;
    LCD_I2C_Command(0x32);
    LCD_I2C_Command(0x28);
    LCD_I2C_Command(0x08);
    LCD_I2C_Command(0x01);
    LCD_I2C_pause;
    LCD_I2C_Command(0x06);
    LCD_I2C_Command(0x0C);
}


//Função:     Start da comunicação I2C
//Parâmetros: byte de dados
//Retorno:    void
void LCD_I2C_Send(uint8_t data)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);

    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, ((/*0x20+*/LCD_ADDR <<  1)), I2C_Direction_Transmitter);

    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, data);

    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C1, ENABLE);
}


//Função:     Envia um comando
//Parâmetros: byte de dados
//Retorno:    void
void LCD_I2C_Command(uint8_t com)
{
    uint8_t data = 0;

    data |= (backlightState & 0x01) << BL;

    data |= (((com & 0x10) >> 4) << DB4);
    data |= (((com & 0x20) >> 5) << DB5);
    data |= (((com & 0x40) >> 6) << DB6);
    data |= (((com & 0x80) >> 7) << DB7);
    LCD_I2C_Send(data);

    data |= (1 << EN);
    LCD_I2C_Send(data);
    LCD_I2C_pause;

    data &= ~(1 << EN);
    LCD_I2C_Send(data);
    LCD_I2C_pause;

    data = 0;

    data |= (backlightState & 0x01) << BL;

    data |= (((com & 0x01) >> 0) << DB4);
    data |= (((com & 0x02) >> 1) << DB5);
    data |= (((com & 0x04) >> 2) << DB6);
    data |= (((com & 0x08) >> 3) << DB7);
    LCD_I2C_Send(data);

    data |= (1 << EN);
    LCD_I2C_Send(data);
    LCD_I2C_pause;

    data &= ~(1 << EN);
    LCD_I2C_Send(data);
    LCD_I2C_pause;
}


//Função:     Seta o backlight
//Parâmetros: byte de dados para definir o estado
//Retorno:    void
void LCD_I2C_Backlight(uint8_t state)
{
    backlightState = (state & 0x01) << BL;
    LCD_I2C_Send(backlightState);
}


//Função:     Envia um byte (caracter)
//Parâmetros: byte de dados
//Retorno:    void
void LCD_I2C_Data(uint8_t com)
{
    uint8_t data = 0;

    data |= (1 << EN);
    data |= (1 << RS);
    data |= (backlightState & 0x01) << BL;

    data |= (((com & 0x10) >> 4) << DB4);
    data |= (((com & 0x20) >> 5) << DB5);
    data |= (((com & 0x40) >> 6) << DB6);
    data |= (((com & 0x80) >> 7) << DB7);
    LCD_I2C_Send(data);
    LCD_I2C_pause;

    data &= ~(1 << EN);
    LCD_I2C_Send(data);
    LCD_I2C_pause;

    data = 0;

    data |= (1 << EN);
    data |= (1 << RS);
    data |= (backlightState & 0x01) << BL;

    data |= (((com & 0x01) >> 0) << DB4);
    data |= (((com & 0x02) >> 1) << DB5);
    data |= (((com & 0x04) >> 2) << DB6);
    data |= (((com & 0x08) >> 3) << DB7);
    LCD_I2C_Send(data);
    LCD_I2C_pause;

    data &= ~(1 << EN);
    LCD_I2C_Send(data);
    LCD_I2C_pause;
}


//Função:     Configura o periférico I2C1
//Parâmetros: void
//Retorno:    void
void I2C_Configuration()
{
    I2C_InitTypeDef I2C_InitStructure ;

    I2C_StructInit (&I2C_InitStructure);
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C ;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2 ;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Disable ;
    I2C_InitStructure.I2C_AcknowledgedAddress =I2C_AcknowledgedAddress_7bit ;
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_Init (I2C1 , &I2C_InitStructure );
    I2C_Cmd (I2C1 , ENABLE );
}


//Função:     Delay pós comando LCD_PAUSE
//Parâmetros: tempo de delay em ms (milisegundos)
//Retorno:    void
void delay(uint32_t t)
{
    uint32_t i = 0;

    for (; i < t; i++)
        {}
}


//Função:     Limpa a tela
//Parâmetros: void
//Retorno:    void
void LCD_I2C_Clear()
{
    LCD_I2C_Command(0x01);
    delay_clear(1);
}


//Função:     Delay exclusivo para limpeza de tela
//Parâmetros: tempo de delay em ms (milisegundos)
//Retorno:    void
void delay_clear(__IO uint32_t num)
{
    __IO uint32_t index = 0;

    /*default system clock is 72MHz */
    for(index = (72000 * num); index != 0; index--)
        {}
}
