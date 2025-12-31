################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/EKO_Drivers/ADC/Src/adc_driver.c 

OBJS += \
./Core/EKO_Drivers/ADC/Src/adc_driver.o 

C_DEPS += \
./Core/EKO_Drivers/ADC/Src/adc_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/EKO_Drivers/ADC/Src/%.o Core/EKO_Drivers/ADC/Src/%.su Core/EKO_Drivers/ADC/Src/%.cyclo: ../Core/EKO_Drivers/ADC/Src/%.c Core/EKO_Drivers/ADC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F105xC -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-EKO_Drivers-2f-ADC-2f-Src

clean-Core-2f-EKO_Drivers-2f-ADC-2f-Src:
	-$(RM) ./Core/EKO_Drivers/ADC/Src/adc_driver.cyclo ./Core/EKO_Drivers/ADC/Src/adc_driver.d ./Core/EKO_Drivers/ADC/Src/adc_driver.o ./Core/EKO_Drivers/ADC/Src/adc_driver.su

.PHONY: clean-Core-2f-EKO_Drivers-2f-ADC-2f-Src

