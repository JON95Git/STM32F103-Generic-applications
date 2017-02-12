/**
  ******************************************************************************
  * @file    RCC.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta as funções de controle do componente RCC
  ******************************************************************************
  */
#include "stm32f10x_rcc.h"
#include "RCC.h"


//Função:     configura e inicializa o componente RCC para funcionamento dos periféricos
//Parâmetros: void
//Retorno:    void
void RCC_Configuration()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 |RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

/*								Tabela dos periféricos RCC
 			APB1 Devices												APB2 Devices

RCC_APB1Periph_BKP														RCC_APB2Periph_ADC1
RCC_APB1Periph_CEC														RCC_APB2Periph_AFIO
RCC_APB1Periph_DAC														RCC_APB2Periph_GPIOA
RCC_APB1Periph_I2C1														RCC_APB2Periph_GPIOB
RCC_APB1Periph_I2C2														RCC_APB2Periph_GPIOD
RCC_APB1Periph_PWR														RCC_APB2Periph_GPIOE
RCC_APB1Periph_SPI2														RCC_APB2Periph_SPI1
RCC_APB1Periph_TIM2														RCC_APB2Periph_TIM1
RCC_APB1Periph_TIM3														RCC_APB2Periph_TIM15
RCC_APB1Periph_TIM4													    RCC_APB2Periph_TIM16
RCC_APB1Periph_TIM5														RCC_APB2Periph_TIM17
RCC_APB1Periph_TIM6														RCC_APB2Periph_USART1
RCC_APB1Periph_TIM7
RCC_APB1Periph_USART2
RCC_APB1Periph_USART3
RCC_APB1Periph_WWDG


										   AHB Devices
										RCC_AHBPeriph_CRC
										RCC_AHBPeriph_DMA

 * */
