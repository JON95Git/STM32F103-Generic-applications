/**
  ******************************************************************************
  * @file    ADC.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @brief   O presente módulo apresenta a implementação das funções de controle
  *          do periférico ADC
  ******************************************************************************
  */
#include "stm32f10x_adc.h"
#include "ADC.h"


//Função:     configura e inicializa o periférico ADC1
//Parâmetros: void
//Retorno:    void
void ADC1_Init()
{
    ADC_InitTypeDef ADC_InitStructure;

    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_Init(ADC1, &ADC_InitStructure);

    //Enable End Of Converstion Interrupt
    ADC_ITConfig (ADC1 , ADC_IT_EOC , ENABLE );

    // Select input channel for ADC1 ADC1 channel 0 (PA0)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); // Enable ADC1 AWD interrupt

    ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);
    ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x6A0, 0x000);
    ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_1);

    ADC_Init(ADC1, &ADC_InitStructure);

    //Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
}


//Função:     executa a calibração necessário para o ADC1
//Parâmetros: void
//Retorno:    void
void ADC1_Calibration()
{

    ADC_ResetCalibration(ADC1);

    // Check the end of ADC1 reset calibration register
    while ( ADC_GetResetCalibrationStatus (ADC1));

    // Start ADC1 calibration
    ADC_StartCalibration (ADC1);

    // Check the end of ADC1 calibration
    while ( ADC_GetCalibrationStatus (ADC1));


}


//Função:     Rotina de interrupção do ADC1 por EOD (end of convertion)
//Parâmetros: void
//Retorno:    void
void ADC1_2_IRQHandler()
{

    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
		{
	    // Read ADC DR and set LED accordingly
	    ADC_ClearITPendingBit (ADC1 , ADC_IT_EOC ); //End of convertion
	    ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);  //Analog watch dog
	    //GPIOC -> ODR ^=  ( 1 << 13 );
	    DadosBuffADC = ADC_GetConversionValue(ADC1);
	    FlagInterruptADC = 1;
		}
}


//Função:     Inicia a conversão do ADC1
//Parâmetros: void
//Retorno:    void
void ADC1_StartConvertion()
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


//Função:     Realiza a leitura do ADC1 por pooling
//Parâmetros: void
//Retorno:    void
uint16_t ADC1_Read()
{
    // Start ADC conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Wait until ADC conversion finished
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    return ADC_GetConversionValue(ADC1);
}
