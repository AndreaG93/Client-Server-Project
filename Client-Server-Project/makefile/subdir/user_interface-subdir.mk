################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/user_interface/interface_functions.c \
../src/user_interface/interfaces.c 

OBJS += \
./src/user_interface/interface_functions.o \
./src/user_interface/interfaces.o 

C_DEPS += \
./src/user_interface/interface_functions.d \
./src/user_interface/interfaces.d 


# Each subdirectory must supply rules for building sources it contributes
src/user_interface/%.o: ../src/user_interface/%.c
	mkdir -p ./src/user_interface
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


