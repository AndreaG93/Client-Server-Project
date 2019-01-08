################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/network/server/server_manager.c 

OBJS += \
./src/network/server/server_manager.o 

C_DEPS += \
./src/network/server/server_manager.d 


# Each subdirectory must supply rules for building sources it contributes
src/network/server/%.o: ../src/network/server/%.c
	mkdir -p ./src/network/server
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


