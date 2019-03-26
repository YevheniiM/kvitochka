################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/I2C_LCD.c \
../Src/adc.c \
../Src/dma.c \
../Src/gpio.c \
../Src/i2c.c \
../Src/i2s.c \
../Src/iwdg.c \
../Src/main.c \
../Src/spi.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/tim.c \
../Src/usart.c 

OBJS += \
./Src/I2C_LCD.o \
./Src/adc.o \
./Src/dma.o \
./Src/gpio.o \
./Src/i2c.o \
./Src/i2s.o \
./Src/iwdg.o \
./Src/main.o \
./Src/spi.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/tim.o \
./Src/usart.o 

C_DEPS += \
./Src/I2C_LCD.d \
./Src/adc.d \
./Src/dma.d \
./Src/gpio.d \
./Src/i2c.d \
./Src/i2s.d \
./Src/iwdg.d \
./Src/main.d \
./Src/spi.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/tim.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F411xE -I"D:/Study/Programing/POC/___project/stm32f411/Inc" -I"D:/Study/Programing/POC/___project/stm32f411/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/Study/Programing/POC/___project/stm32f411/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"D:/Study/Programing/POC/___project/stm32f411/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/Study/Programing/POC/___project/stm32f411/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


