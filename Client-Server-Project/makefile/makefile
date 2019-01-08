################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include ./subdir/user_interface-subdir.mk
-include ./subdir/thread-subdir.mk
-include ./subdir/network-server-subdir.mk
-include ./subdir/network-client-subdir.mk
-include ./subdir/network-subdir.mk
-include ./subdir/file-subdir.mk
-include ./subdir/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ClientServerProject

# Tool invocations
ClientServerProject: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc  -o "ClientServerProject" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) ClientServerProject
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
