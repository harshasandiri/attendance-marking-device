################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/FatFs/src/option/syscall.c 

OBJS += \
./Middlewares/Third_Party/FatFs/src/option/syscall.o 

C_DEPS += \
./Middlewares/Third_Party/FatFs/src/option/syscall.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/FatFs/src/option/%.o: ../Middlewares/Third_Party/FatFs/src/option/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F429xx -I"C:/stm_workspace/student_attendance_marker/Inc" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/Common" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/ili9341" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/l3gd20" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/Components/stmpe811" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/BSP/STM32F429I-Discovery" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Utilities/CPU" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Utilities/Fonts" -I"C:/stm32cubef4/STM32Cube_FW_F4_V1.16.0/Utilities/Log" -I"C:/stm_workspace/student_attendance_marker/Middlewares/Third_Party/FatFs/src/drivers" -I"C:/stm_workspace/student_attendance_marker/Middlewares/Third_Party/LibJPEG/include" -I"C:/stm_workspace/student_attendance_marker/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"C:/stm_workspace/student_attendance_marker/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc" -I"C:/stm_workspace/student_attendance_marker/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/stm_workspace/student_attendance_marker/Middlewares/Third_Party/FatFs/src" -I"C:/stm_workspace/student_attendance_marker/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


