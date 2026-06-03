PROJECT_NAME = freertos_kakip_cr8_1_ep
CORE         = CR8_1

EXTRA_INC = -I rzv/aws/FreeRTOS/FreeRTOS/Source/include \
            -I rzv_cfg/aws \
            -I rzv/fsp/src/rm_freertos_port/cr

EXTRA_SRC = $(wildcard rzv/aws/FreeRTOS/FreeRTOS/Source/*.c) \
            rzv/aws/FreeRTOS/FreeRTOS/Source/portable/MemMang/heap_4.c \
            $(wildcard rzv/fsp/src/rm_freertos_port/cr/*.c)

EXTRA_ASM = $(wildcard rzv/fsp/src/rm_freertos_port/cr/*.asm)

include ../../../common.mk
