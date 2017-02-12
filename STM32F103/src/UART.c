/**
  ******************************************************************************
  * @file    UART.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta as funções de controle do periférico UART
  ******************************************************************************
  */
#include "UART.h"
#include "stm32f10x_usart.h"


//Função:     configura e inicializa o componente UART1 com interrrupção
//Parâmetros: void
//Retorno:    void
void USART_Config()
{
    USART_InitTypeDef USART_InitStructure;
    USART_StructInit (& USART_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Init(USART1 ,& USART_InitStructure);

    USART_Cmd(USART1 , ENABLE);
}


//Função:     Rotina de interrupção da UART1 por recepção
//Parâmetros: void
//Retorno:    void
void USART1_IRQHandler()
{
    if (USART_GetITStatus(USART1 , USART_IT_RXNE) != RESET)
        {
        //GPIOC -> ODR ^=  ( 1 << 13 );
    	FlagInterruptUart = 1;
    	DadosBuffUart = USART1 -> DR;  // the character from the USART1 data register is saved in data
        }
}

