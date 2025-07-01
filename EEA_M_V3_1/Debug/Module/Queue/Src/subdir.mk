################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Module/Queue/Src/NNFQueue.c 

OBJS += \
./Module/Queue/Src/NNFQueue.o 

C_DEPS += \
./Module/Queue/Src/NNFQueue.d 


# Each subdirectory must supply rules for building sources it contributes
Module/Queue/Src/%.o Module/Queue/Src/%.su Module/Queue/Src/%.cyclo: ../Module/Queue/Src/%.c Module/Queue/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L452xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/E220-900T22D/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Queue/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/EEA_protocol/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Sensor/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Flash/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Module-2f-Queue-2f-Src

clean-Module-2f-Queue-2f-Src:
	-$(RM) ./Module/Queue/Src/NNFQueue.cyclo ./Module/Queue/Src/NNFQueue.d ./Module/Queue/Src/NNFQueue.o ./Module/Queue/Src/NNFQueue.su

.PHONY: clean-Module-2f-Queue-2f-Src

