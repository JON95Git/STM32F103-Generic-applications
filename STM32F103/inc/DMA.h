/**
  ******************************************************************************
  * @file    DMA.h
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta a declaração das funções do DMA
  ******************************************************************************
  */


#ifndef  DMA.H
#define  DMA.H

extern uint16_t DataSPI;

void DMA_Configuration ();
void DMA1_Channel5_IRQHandler (void);

#endif



