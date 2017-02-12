/**
  ******************************************************************************
  * @file    NVIC.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta as funções de controle do NVIC
  ******************************************************************************
  */
#include "NVIC.h"
#include "stm32f10x.h"


//Função:     configura e inicializa o componente NVIC para as interrupções
//Parâmetros: void
//Retorno:    void
void NVIC_Configuration()
{
	/*
	* NVIC_PriorityGroup_0 0 bits priority , 4 bits subgroup
	* NVIC_PriorityGroup_1 1 bits priority , 3 bits subgroup
	* NVIC_PriorityGroup_2 2 bits priority , 2 bits subgroup
	* NVIC_PriorityGroup_3 3 bits priority , 1 bits subgroup
	* NVIC_PriorityGroup_4 4 bits priority , 0 bits subgroup
	*/

    NVIC_InitTypeDef NVIC_InitStructure;

    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    //DMA 1 Channel 5 -> SPI2 ILI9163
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // Enable the USARTx Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    //Enable ADC Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
