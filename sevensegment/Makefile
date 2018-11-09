TARGET = sevensegment
C_SRCS += sevensegment.c
CFLAGS += -O1 -fno-builtin-printf -DUSE_PLIC

BSP_BASE = ../../bsp

#ASM_SRCS += $(ENV_DIR)/ventry.S
C_SRCS += $(BSP_BASE)/drivers/plic/plic_driver.c

include $(BSP_BASE)/env/common.mk
