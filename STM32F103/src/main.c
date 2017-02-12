/**
  ******************************************************************************
  * @file    main.c
  * @author  Jonathan Gonzaga
  * @date    11/2016
  * @MCU:    STM32F103C8T6
  * @brief   O presente módulo tem como objetivo apresentar aplicações genéricas para
  *	         os componentes de hardware do microcontrolador de forma didática.
  *
  *           O projeto foi feito na IDE AC6 SW4STM32 (Eclipse) configurada para uma 
  *           custom board chinesa.
  *           Para utilizar alguma outra placa é necessário configurar o projeto.
  *
  *
  *	         Estão exemplificadas as seguintes aplicações:
  *	         - LCD I2C com biblioteca adaptada de terceiro (infelizmente desconhecido);
  *	         - Display TFT (ILI 9163) com biblioteca de terceiro;
  *	         - ADC com interrupção por EOC (end of convertion);
  *	         - UART com interupção por recepção e módulo Bluetooth;
  *	         - Cartão SD por SPI, usando FAT FS e código disponibilizado pelo Domen Puncer.
  *               - Algumas funções estão comentadas, "descomente" os includes para usá-las.
  *
  ******************************************************************************
  */

//___________________________________________________________________
//_____________________MACROS/INCLUDES/DEFINES_______________________

#include "stm32f10x.h"
#include "GFXC.h"
#include "ILI9163.h"
//#include "DMA.h"
//#include "FAT_FS.h"

//#include "xprintf.h"
//#include "ff.h"
//#include "SD.h"
//#include "diskio.h"
//#include "integer.h"

//FATFS FatFs;
//FIL Fil;
//DIR Dir;
//FRESULT fr;    /* FatFs return code */
//uint8_t buff[160][16];

//___________________________________________________________________
//_____________________VARIÁVEIS GLOBAIS_____________________________

volatile int FlagInterruptUart = 0;
volatile char DadosBuffUart = 0;
char DadoADCConvertido[5];
volatile uint16_t DadosBuffADC = 0, FlagInterruptADC = 0;
extern volatile int FlagInterruptSystick;
volatile uint32_t MSec = 0;


//___________________________________________________________________
//_______________________PROTÓTIPOS DE FUNÇÕES_______________________

void Systick_Configuration();

//___________________________________________________________________
//______________________FUNÇÕES ADICIONAIS___________________________

//Função: configura o timer do sistema Systick para 1 ms
//Parâmetros: void
//Retorno: void
void Systick_Configuration()
{
    // Update SystemCoreClock value
    SystemCoreClockUpdate();

    // Configure the SysTick timer to overflow every 1 us
    SysTick_Config(SystemCoreClock / 1000);

    NVIC_SetPriority(SysTick_IRQn, 0x1);
}

//___________________________________________________________________
//______________________FUNÇÃO MAIN__________________________________

int main()
{
    //-----Initialization MCU modules--------
    RCC_Configuration();   			//Deve vir ANTES do GPIO Configurations
    GPIO_Configuration(); 				//Deve vir DEPOIS do RCC Configurations
    USART_Config();
    I2C_Configuration();
    ADC1_Init();
    ADC1_Calibration();
    Systick_Configuration();
    NVIC_Configuration();
    //DMA_Configuration();

    //----------------------------------------
    USART_SendData(USART1, 53);

    delay_clear(100);

    //-----Initialization interfaces--------
    LCD_I2C_Init();
    LCD_I2C_PrintC(" STM32 PCF85744");

    ILI9163Init();

    SetScrn(Black);
    WriteString("STM32", Black, Green,  10, 80, 0, 1);
    delay_clear(100);
    //--------------------------------------


while (1)
    {

    if (FlagInterruptUart != 0)
        {
        LCD_I2C_Clear();
        LCD_I2C_PrintC("RX: ");
        LCD_I2C_Data(DadosBuffUart);

        SetScrn(Black);
        WriteString("RX: ", Black, Green,  10, 80, 0, 1);
        WriteString(DadosBuffUart, Black, Green,  50, 80,0 , 2);

        delay_clear(100);
        SetScrn(Black);
        WriteString("STM32", Black, Green,  30, 80, 0, 1);
        LCD_I2C_Clear();
        LCD_I2C_PrintC(" STM32 LCD_I2C ");
        FlagInterruptUart = 0;
        }

    if (FlagInterruptADC != 0)
        {
        LCD_I2C_Clear();
        LCD_I2C_PrintC("Analog input");
        LCD_I2C_Goto(2,1);

        sprintf(DadoADCConvertido, "%i", DadosBuffADC);
        LCD_I2C_PrintC(DadoADCConvertido);

        SetScrn(Black);
        WriteString("Analog input", Black, Green,  10, 80, 0, 1);
        WriteString(DadoADCConvertido, Black, Green,  10, 100, 0, 1);

        delay_clear(20);
        FlagInterruptADC = 0;
        }

    if (FlagInterruptSystick >= 10000)
        {
        FlagInterruptSystick = 0;
        ADC1_StartConvertion();
        }


    }

}

