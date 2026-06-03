PROJECT_NAME = rzv2h_cm33_rpmsg_linux-rtos_demo
CORE         = CM33

EXTRA_INC = -I rzv/aws/FreeRTOS/FreeRTOS/Source/include \
            -I rzv_cfg/aws \
            -I rzv_cfg/driver \
            -I rzv/fsp/src/rm_freertos_port/cm \
            -I rzv/linaro/libmetal/include \
            -I rzv/linaro/open-amp/lib/include \
            -I rzv/arm/CMSIS_6/CMSIS/Core/Include

EXTRA_SRC = $(wildcard rzv/aws/FreeRTOS/FreeRTOS/Source/*.c) \
            rzv/aws/FreeRTOS/FreeRTOS/Source/portable/MemMang/heap_4.c \
            $(wildcard rzv/fsp/src/rm_freertos_port/cm/*.c) \
            $(wildcard rzv/linaro/libmetal/lib/*.c) \
            $(wildcard rzv/linaro/libmetal/lib/system/freertos/*.c) \
            $(wildcard rzv/linaro/libmetal/lib/system/freertos/rzv2/*.c) \
            $(wildcard rzv/linaro/open-amp/lib/remoteproc/*.c) \
            $(wildcard rzv/linaro/open-amp/lib/rpmsg/*.c) \
            $(wildcard rzv/linaro/open-amp/lib/virtio/*.c)

include ../../common.mk
