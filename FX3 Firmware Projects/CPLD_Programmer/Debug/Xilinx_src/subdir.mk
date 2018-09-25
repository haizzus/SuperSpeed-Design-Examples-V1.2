################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Xilinx_src/lenval.c \
../Xilinx_src/micro.c 

OBJS += \
./Xilinx_src/lenval.o \
./Xilinx_src/micro.o 

C_DEPS += \
./Xilinx_src/lenval.d \
./Xilinx_src/micro.d 


# Each subdirectory must supply rules for building sources it contributes
Xilinx_src/%.o: ../Xilinx_src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -D__CYU3P_TX__=1 -I"C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\/firmware/u3p_firmware/inc" -I.. -O0 -ffunction-sections -fdata-sections -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm926ej-s -mthumb-interwork -g -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


