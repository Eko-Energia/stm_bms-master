################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EKO_Drivers/CAN/Src/can_driver.c 

OBJS += \
./EKO_Drivers/CAN/Src/can_driver.o 

C_DEPS += \
./EKO_Drivers/CAN/Src/can_driver.d 


# Each subdirectory must supply rules for building sources it contributes
EKO_Drivers/CAN/Src/%.o EKO_Drivers/CAN/Src/%.su EKO_Drivers/CAN/Src/%.cyclo: ../EKO_Drivers/CAN/Src/%.c EKO_Drivers/CAN/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F105xC -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../EKO_Drivers/CAN/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../EKO_Drivers/ADC/Inc -I../BMS_Driver/BMS/Inc -I../BMS_Driver/CAN/Inc -I../BMS_Driver/ADC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-EKO_Drivers-2f-CAN-2f-Src

clean-EKO_Drivers-2f-CAN-2f-Src:
	-$(RM) ./EKO_Drivers/CAN/Src/can_driver.cyclo ./EKO_Drivers/CAN/Src/can_driver.d ./EKO_Drivers/CAN/Src/can_driver.o ./EKO_Drivers/CAN/Src/can_driver.su

.PHONY: clean-EKO_Drivers-2f-CAN-2f-Src

