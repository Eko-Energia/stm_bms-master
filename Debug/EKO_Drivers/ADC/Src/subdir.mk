################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EKO_Drivers/ADC/Src/adc_driver.c 

OBJS += \
./EKO_Drivers/ADC/Src/adc_driver.o 

C_DEPS += \
./EKO_Drivers/ADC/Src/adc_driver.d 


# Each subdirectory must supply rules for building sources it contributes
EKO_Drivers/ADC/Src/%.o EKO_Drivers/ADC/Src/%.su EKO_Drivers/ADC/Src/%.cyclo: ../EKO_Drivers/ADC/Src/%.c EKO_Drivers/ADC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F105xC -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../EKO_Drivers/CAN/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../EKO_Drivers/ADC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-EKO_Drivers-2f-ADC-2f-Src

clean-EKO_Drivers-2f-ADC-2f-Src:
	-$(RM) ./EKO_Drivers/ADC/Src/adc_driver.cyclo ./EKO_Drivers/ADC/Src/adc_driver.d ./EKO_Drivers/ADC/Src/adc_driver.o ./EKO_Drivers/ADC/Src/adc_driver.su

.PHONY: clean-EKO_Drivers-2f-ADC-2f-Src

