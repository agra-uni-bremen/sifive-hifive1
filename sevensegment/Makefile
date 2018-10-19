TARGET = sevensegment
C_SRCS += sevensegment.c
CFLAGS += -O2 -fno-builtin-printf -DVECT_IRQ -DNO_INIT

BSP_BASE = ../../bsp

ASM_SRCS += $(ENV_DIR)/ventry.S
C_SRCS += $(BSP_BASE)/drivers/plic/plic_driver.c

include $(BSP_BASE)/env/common.mk
