################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/file/FilePropertyList_struct.c \
../src/file/Settings.c \
../src/file/directory_manager.c 

OBJS += \
./src/file/FilePropertyList_struct.o \
./src/file/Settings.o \
./src/file/directory_manager.o 

C_DEPS += \
./src/file/FilePropertyList_struct.d \
./src/file/Settings.d \
./src/file/directory_manager.d 


# Each subdirectory must supply rules for building sources it contributes
src/file/%.o: ../src/file/%.c
	mkdir -p ./src/file
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


