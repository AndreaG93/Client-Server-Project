################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/network/client/client_manager.c 

OBJS += \
./src/network/client/client_manager.o 

C_DEPS += \
./src/network/client/client_manager.d 


# Each subdirectory must supply rules for building sources it contributes
src/network/client/%.o: ../src/network/client/%.c
	mkdir -p ./src/network/client
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


