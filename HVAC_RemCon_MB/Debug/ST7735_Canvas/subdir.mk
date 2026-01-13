################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ST7735_Canvas/ST7735Canvas.c \
../ST7735_Canvas/fonts.c 

OBJS += \
./ST7735_Canvas/ST7735Canvas.o \
./ST7735_Canvas/fonts.o 

C_DEPS += \
./ST7735_Canvas/ST7735Canvas.d \
./ST7735_Canvas/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
ST7735_Canvas/%.o ST7735_Canvas/%.su ST7735_Canvas/%.cyclo: ../ST7735_Canvas/%.c ST7735_Canvas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/apps/http -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I"C:/Users/alex/source/repos/FacCon/HVAC_RemCon_MB/ST7735_Canvas" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ST7735_Canvas

clean-ST7735_Canvas:
	-$(RM) ./ST7735_Canvas/ST7735Canvas.cyclo ./ST7735_Canvas/ST7735Canvas.d ./ST7735_Canvas/ST7735Canvas.o ./ST7735_Canvas/ST7735Canvas.su ./ST7735_Canvas/fonts.cyclo ./ST7735_Canvas/fonts.d ./ST7735_Canvas/fonts.o ./ST7735_Canvas/fonts.su

.PHONY: clean-ST7735_Canvas

