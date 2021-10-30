################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/fatfs.c \
../Src/fingerprint.c \
../Src/jdata_conf.c \
../Src/libjpeg.c \
../Src/main.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/usb_host.c \
../Src/usbh_conf.c \
../Src/usbh_platform.c 

OBJS += \
./Src/fatfs.o \
./Src/fingerprint.o \
./Src/jdata_conf.o \
./Src/libjpeg.o \
./Src/main.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/usb_host.o \
./Src/usbh_conf.o \
./Src/usbh_platform.o 

C_DEPS += \
./Src/fatfs.d \
./Src/fingerprint.d \
./Src/jdata_conf.d \
./Src/libjpeg.d \
./Src/main.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/usb_host.d \
./Src/usbh_conf.d \
./Src/usbh_platform.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F429xx -I"C:/stm_workspace/student_attendance_marker/Inc" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/Common" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/ili9341" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/l3gd20" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/stmpe811" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/STM32F429I-Discovery" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Utilities/CPU" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Utilities/Fonts" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Utilities/Log" -I"C:/stm_workspace/student_attendance_marker/Middlewares/Third_Party/FatFs/src/drivers" -I"C:/stm_workspace/student_attendance_marker/Middlewares/Third_Party/LibJPEG/include" -I"C:/stm_workspace/student_attendance_marker/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"C:/stm_workspace/student_attendance_marker/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc" -I"C:/stm_workspace/student_attendance_marker/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/stm_workspace/student_attendance_marker/Middlewares/Third_Party/FatFs/src" -I"C:/stm_workspace/student_attendance_marker/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


