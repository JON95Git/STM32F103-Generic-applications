/**
  ******************************************************************************
  * @file    GPIO.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta as funções de controle do periférico GPIO
  ******************************************************************************
  */
#include "GPIO.h"
#include "stm32f10x_gpio.h"
#include "ILI9163.h"

//Função:     configura o componente GPIO
//Parâmetros: void
//Retorno:    void
void GPIO_Configuration()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //Initialize led pin B13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //Initialize I2C1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD ;
    GPIO_Init (GPIOB , &GPIO_InitStructure );

    // Initialize USART1_Tx
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA , &GPIO_InitStructure);
    // Initialize USART1_RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA , &GPIO_InitStructure);

    //Initialize GPIOA 0 as analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
