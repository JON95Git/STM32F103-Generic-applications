################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ADC.c \
../src/DMA.c \
../src/GFXC.c \
../src/GPIO.c \
../src/ILI9163.c \
../src/LCD_I2C.c \
../src/NVIC.c \
../src/RCC.c \
../src/UART.c \
../src/main.c \
../src/stm32f10x_it.c \
../src/syscalls.c \
../src/system_stm32f10x.c 

OBJS += \
./src/ADC.o \
./src/DMA.o \
./src/GFXC.o \
./src/GPIO.o \
./src/ILI9163.o \
./src/LCD_I2C.o \
./src/NVIC.o \
./src/RCC.o \
./src/UART.o \
./src/main.o \
./src/stm32f10x_it.o \
./src/syscalls.o \
./src/system_stm32f10x.o 

C_DEPS += \
./src/ADC.d \
./src/DMA.d \
./src/GFXC.d \
./src/GPIO.d \
./src/ILI9163.d \
./src/LCD_I2C.d \
./src/NVIC.d \
./src/RCC.d \
./src/UART.d \
./src/main.d \
./src/stm32f10x_it.d \
./src/syscalls.d \
./src/system_stm32f10x.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32F1 -DSTM32F103C8Tx -DSTM32 -DDEBUG -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -I"/home/jonathan/workspace/st_mini_stdperiph_lib" -I"/home/jonathan/workspace/testt/inc" -I"/home/jonathan/workspace/st_mini_stdperiph_lib/StdPeriph_Driver/inc" -I"/home/jonathan/workspace/st_mini_stdperiph_lib/CMSIS/device" -I"/home/jonathan/workspace/st_mini_stdperiph_lib/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


