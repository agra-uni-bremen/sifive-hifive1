TARGET = snake
C_SRCS += snake.cpp helpers.c display.c font.c
CFLAGS += -O1 -fno-builtin-printf -DUSE_PLIC -DUSE_M_TIME

BSP_BASE = ../../bsp

C_SRCS += $(BSP_BASE)/drivers/plic/plic_driver.c

include $(BSP_BASE)/env/common.mk
