################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Module/Flash/Src/Flash.c 

OBJS += \
./Module/Flash/Src/Flash.o 

C_DEPS += \
./Module/Flash/Src/Flash.d 


# Each subdirectory must supply rules for building sources it contributes
Module/Flash/Src/%.o Module/Flash/Src/%.su Module/Flash/Src/%.cyclo: ../Module/Flash/Src/%.c Module/Flash/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L452xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/E220-900T22D/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Queue/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/EEA_protocol/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Sensor/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Flash/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Module-2f-Flash-2f-Src

clean-Module-2f-Flash-2f-Src:
	-$(RM) ./Module/Flash/Src/Flash.cyclo ./Module/Flash/Src/Flash.d ./Module/Flash/Src/Flash.o ./Module/Flash/Src/Flash.su

.PHONY: clean-Module-2f-Flash-2f-Src

