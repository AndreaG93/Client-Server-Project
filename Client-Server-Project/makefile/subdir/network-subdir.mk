################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/network/NetworkStatistics.c \
../src/network/data_transmission.c \
../src/network/time_management.c 

OBJS += \
./src/network/NetworkStatistics.o \
./src/network/data_transmission.o \
./src/network/time_management.o 

C_DEPS += \
./src/network/NetworkStatistics.d \
./src/network/data_transmission.d \
./src/network/time_management.d 


# Each subdirectory must supply rules for building sources it contributes
src/network/%.o: ../src/network/%.c
	mkdir -p ./src/network
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


