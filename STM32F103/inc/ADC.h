/**
  ******************************************************************************
  * @file    ADC.h
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta a declaração das funções de controle
  *          do periférico ADC
  ******************************************************************************
  */


#ifndef  ADC.H
#define  ADC.H

extern volatile uint16_t DadosBuffADC, FlagInterruptADC;

void ADC1_Init();
uint16_t ADC1_Read();
void ADC1_2_IRQHandler();
void ADC1_Calibration();
void ADC1_StartConvertion();

#endif


