################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Module/Sensor/Src/HC-SR501PirSensor.c \
../Module/Sensor/Src/LedSensor.c \
../Module/Sensor/Src/SZH-SSBH-011CdsSensor.c \
../Module/Sensor/Src/Sensor.c 

OBJS += \
./Module/Sensor/Src/HC-SR501PirSensor.o \
./Module/Sensor/Src/LedSensor.o \
./Module/Sensor/Src/SZH-SSBH-011CdsSensor.o \
./Module/Sensor/Src/Sensor.o 

C_DEPS += \
./Module/Sensor/Src/HC-SR501PirSensor.d \
./Module/Sensor/Src/LedSensor.d \
./Module/Sensor/Src/SZH-SSBH-011CdsSensor.d \
./Module/Sensor/Src/Sensor.d 


# Each subdirectory must supply rules for building sources it contributes
Module/Sensor/Src/%.o Module/Sensor/Src/%.su Module/Sensor/Src/%.cyclo: ../Module/Sensor/Src/%.c Module/Sensor/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L452xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/E220-900T22D/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Queue/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/EEA_protocol/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Sensor/Inc" -I"C:/Users/dwegw/2023_EEA/project/EEA_M_V3_1/Module/Flash/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Module-2f-Sensor-2f-Src

clean-Module-2f-Sensor-2f-Src:
	-$(RM) ./Module/Sensor/Src/HC-SR501PirSensor.cyclo ./Module/Sensor/Src/HC-SR501PirSensor.d ./Module/Sensor/Src/HC-SR501PirSensor.o ./Module/Sensor/Src/HC-SR501PirSensor.su ./Module/Sensor/Src/LedSensor.cyclo ./Module/Sensor/Src/LedSensor.d ./Module/Sensor/Src/LedSensor.o ./Module/Sensor/Src/LedSensor.su ./Module/Sensor/Src/SZH-SSBH-011CdsSensor.cyclo ./Module/Sensor/Src/SZH-SSBH-011CdsSensor.d ./Module/Sensor/Src/SZH-SSBH-011CdsSensor.o ./Module/Sensor/Src/SZH-SSBH-011CdsSensor.su ./Module/Sensor/Src/Sensor.cyclo ./Module/Sensor/Src/Sensor.d ./Module/Sensor/Src/Sensor.o ./Module/Sensor/Src/Sensor.su

.PHONY: clean-Module-2f-Sensor-2f-Src

