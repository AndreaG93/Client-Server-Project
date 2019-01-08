################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c \
../src/project_property.c 

OBJS += \
./src/main.o \
./src/project_property.o 

C_DEPS += \
./src/main.d \
./src/project_property.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	mkdir -p ./src
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


