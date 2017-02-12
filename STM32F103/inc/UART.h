/**
  ******************************************************************************
  * @file    UART.h
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta a declaração das funções de controle
  *          do periférico UART
  ******************************************************************************
  */
#include "stm32f10x.h"

#ifndef  UART.H
#define  UART.H

extern volatile int FlagInterruptUart;
extern volatile char DadosBuffUart;



void USART_Config();
void USART1_IRQHandler();


#endif



