################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/LEDs.c \
../src/_initialize_hardware.c \
../src/_write.c \
../src/accelerometer.c \
../src/adcs.c \
../src/animations.c \
../src/lidar.c \
../src/main.c \
../src/motors.c \
../src/serial.c \
../src/stm32f4xx_hal_msp.c \
../src/stm32f4xx_it.c \
../src/system_stm32f4xx.c \
../src/time.c \
../src/topLoop.c \
../src/watchdog.c 

OBJS += \
./src/LEDs.o \
./src/_initialize_hardware.o \
./src/_write.o \
./src/accelerometer.o \
./src/adcs.o \
./src/animations.o \
./src/lidar.o \
./src/main.o \
./src/motors.o \
./src/serial.o \
./src/stm32f4xx_hal_msp.o \
./src/stm32f4xx_it.o \
./src/system_stm32f4xx.o \
./src/time.o \
./src/topLoop.o \
./src/watchdog.o 

C_DEPS += \
./src/LEDs.d \
./src/_initialize_hardware.d \
./src/_write.d \
./src/accelerometer.d \
./src/adcs.d \
./src/animations.d \
./src/lidar.d \
./src/main.d \
./src/motors.d \
./src/serial.d \
./src/stm32f4xx_hal_msp.d \
./src/stm32f4xx_it.d \
./src/system_stm32f4xx.d \
./src/time.d \
./src/topLoop.d \
./src/watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F446xx -DUSE_HAL_DRIVER -DHSI_VALUE=16000000 -DHSE_VALUE=0 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f4-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


