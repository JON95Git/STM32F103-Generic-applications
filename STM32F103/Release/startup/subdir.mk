################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32f10x_md.s 

OBJS += \
./startup/startup_stm32f10x_md.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -I"/home/jonathan/workspace/st_mini_stdperiph_lib" -I"/home/jonathan/workspace/testt/inc" -I"/home/jonathan/workspace/st_mini_stdperiph_lib/StdPeriph_Driver/inc" -I"/home/jonathan/workspace/st_mini_stdperiph_lib/CMSIS/device" -I"/home/jonathan/workspace/st_mini_stdperiph_lib/CMSIS/core" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


