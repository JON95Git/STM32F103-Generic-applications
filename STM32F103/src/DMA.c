/**
  ******************************************************************************
  * @file    DMA.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta as funções de controle do DMA
  ******************************************************************************
  */
#include "stm32f10x_dma.h"
#include "DMA.h"

//Função: configura e inicializa o DMA 1 no canal 5 para controle do SPI 2 (display TFT)
//Parâmetros: void
//Retorno: void
void DMA_Configuration()
{

    DMA_DeInit(DMA1_Channel5);
    DMA_InitTypeDef DMA_InitStructure;

    DMA_InitStructure.DMA_PeripheralBaseAddr =  (uint32_t) &(SPI2 -> DR);// (uint32_t) & (SPI2 -> DR);
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable ;
    DMA_InitStructure.DMA_BufferSize = 50;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable ;

    DMA_InitStructure.DMA_MemoryBaseAddr =  (uint32_t) DataSPI;//(uint32_t) &DataSPI;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //Destiny DST or Source SRC

    DMA_Init (DMA1_Channel5 , &DMA_InitStructure);

 //DMA_ITConfig(DMA1_Channel5, DMA_IT_TC  , ENABLE);

 //DMA_Cmd(DMA1_Channel5, ENABLE);

 //DMA_ITConfig(DMA1_Channel5, DMA1_IT_TC5 , ENABLE);


}


//Função: Rotina de interrupção do DMA 1 no canal 5 por TC (transfer complete)
//Parâmetros: void
//Retorno: void
void DMA1_Channel5_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET )
        {
        GPIOC -> ODR ^=  ( 1 << 13 );

        LCD_I2C_Clear();
        LCD_I2C_PrintC("Funcionou essa");
        LCD_I2C_Goto (2,1);
        LCD_I2C_PrintC ("porra de DMA");

        while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);

       // DMA_Cmd(DMA1_Channel5, DISABLE);
 		//SPI_I2S_DMACmd (SPI2, SPI_I2S_DMAReq_Tx, DISABLE );

        DMA_ClearFlag(DMA_IT_TC);
        DMA_ClearITPendingBit(DMA1_IT_GL5);
        }

 }
