################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Displays/ST7735_Canvas/ST7735Canvas.c \
../Displays/ST7735_Canvas/fonts.c 

OBJS += \
./Displays/ST7735_Canvas/ST7735Canvas.o \
./Displays/ST7735_Canvas/fonts.o 

C_DEPS += \
./Displays/ST7735_Canvas/ST7735Canvas.d \
./Displays/ST7735_Canvas/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Displays/ST7735_Canvas/%.o Displays/ST7735_Canvas/%.su Displays/ST7735_Canvas/%.cyclo: ../Displays/ST7735_Canvas/%.c Displays/ST7735_Canvas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/apps/http -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I"C:/Users/alex/source/repos/FacCon/HVAC_RemCon_MB/Displays/Fonts" -I"C:/Users/alex/source/repos/FacCon/HVAC_RemCon_MB/Displays/ST7735_Canvas" -I"C:/Users/alex/source/repos/FacCon/HVAC_RemCon_MB/Core/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Displays-2f-ST7735_Canvas

clean-Displays-2f-ST7735_Canvas:
	-$(RM) ./Displays/ST7735_Canvas/ST7735Canvas.cyclo ./Displays/ST7735_Canvas/ST7735Canvas.d ./Displays/ST7735_Canvas/ST7735Canvas.o ./Displays/ST7735_Canvas/ST7735Canvas.su ./Displays/ST7735_Canvas/fonts.cyclo ./Displays/ST7735_Canvas/fonts.d ./Displays/ST7735_Canvas/fonts.o ./Displays/ST7735_Canvas/fonts.su

.PHONY: clean-Displays-2f-ST7735_Canvas

