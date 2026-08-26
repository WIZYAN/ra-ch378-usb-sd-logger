################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CH378_USB/ch378_file_device.c \
../CH378_USB/ch378_hard.c \
../CH378_USB/ch378_storage.c 

C_DEPS += \
./CH378_USB/ch378_file_device.d \
./CH378_USB/ch378_hard.d \
./CH378_USB/ch378_storage.d 

OBJS += \
./CH378_USB/ch378_file_device.o \
./CH378_USB/ch378_hard.o \
./CH378_USB/ch378_storage.o 


# Each subdirectory must supply rules for building sources it contributes
CH378_USB/%.o: ../CH378_USB/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RA_CORE=CM33 -D_RENESAS_RA_ -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/src" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/HMI" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra/fsp/inc" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra/fsp/inc/api" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra/fsp/inc/instances" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra/arm/CMSIS_5/CMSIS/Core/Include" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra_gen" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra_cfg/fsp_cfg/bsp" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/ra_cfg/fsp_cfg" -I"C:/Users/CI/Desktop/usb_2/CH378/CH378/CH378_USB" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

