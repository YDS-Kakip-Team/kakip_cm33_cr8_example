# common.mk - Shared build rules for Kakip CM33/CR8 firmware projects
#
# Usage: In each project's Makefile, set PROJECT_NAME and CORE, then include this file.
#   PROJECT_NAME = can_fd_kakip_cr8_0_ep
#   CORE         = CR8_0        # CR8_0, CR8_1, or CM33
#   include ../../../common.mk
#
# Optional variables:
#   EXTRA_SRC   - Additional .c source files
#   EXTRA_ASM   - Additional .asm source files
#   EXTRA_INC   - Additional -I include paths
#   INSTALL_DIR - Destination for 'make install' (default: /mnt)
#
# Prerequisites:
#   1. Run ./setup.sh (one-time toolchain install)
#   2. export PATH=/opt/arm-gnu-toolchain/bin:$PATH

# Toolchain
CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size

# Check toolchain exists
ifeq ($(shell which $(CC) 2>/dev/null),)
  $(error $(CC) not found. Run: export PATH=/opt/arm-gnu-toolchain/bin:$$PATH)
endif

# ---------- Core-specific configuration ----------

ifeq ($(CORE),CR8_0)
  CPU_FLAGS     = -mcpu=cortex-r8 -mthumb -mfloat-abi=hard -mfpu=vfpv3-d16
  DEFINES       = -D_RENESAS_RZV_ -D_RZV_CORE=CR8_0 -D_RZV_ORDINAL=2
  LINKER_SCRIPT = script/rzv2h_evk_cr.ld
  CORE_DIR      = cr
  OUTPUT_MODE   = split
else ifeq ($(CORE),CR8_1)
  CPU_FLAGS     = -mcpu=cortex-r8 -mthumb -mfloat-abi=hard -mfpu=vfpv3-d16
  DEFINES       = -D_RENESAS_RZV_ -D_RZV_CORE=CR8_1 -D_RZV_ORDINAL=3
  LINKER_SCRIPT = script/rzv2h_evk_cr.ld
  CORE_DIR      = cr
  OUTPUT_MODE   = split
else ifeq ($(CORE),CM33)
  CPU_FLAGS     = -mcpu=cortex-m33+nodsp -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -mcmse
  DEFINES       = -D_RENESAS_RZV_ -D_RZV_CORE=CM33_0
  LINKER_SCRIPT = script/rzv2h_evk_cm.ld
  CORE_DIR      = cm
  OUTPUT_MODE   = single
else
  $(error CORE must be CR8_0, CR8_1, or CM33)
endif

# ---------- Compiler flags ----------

CFLAGS = $(CPU_FLAGS) $(DEFINES) -std=c99 -Og -g \
         --param=min-pagesize=0 \
         -Wno-format-truncation -Wno-stringop-overflow \
         -ffunction-sections -fdata-sections \
         -fno-strict-aliasing \
         -fsigned-char -fmessage-length=0

ASFLAGS = $(CPU_FLAGS)

# ---------- Include paths ----------

INCLUDES = -I rzv_cfg/fsp_cfg/bsp \
           -I rzv_cfg/fsp_cfg \
           -I rzv/fsp/inc/api \
           -I rzv/fsp/inc/instances \
           -I rzv/fsp/inc \
           -I rzv/fsp/src/bsp/mcu/rzv2h \
           -I rzv_gen \
           -I src

# SEGGER_RTT (present in most projects, ignored if dir doesn't exist)
ifneq ($(wildcard src/SEGGER_RTT),)
  INCLUDES += -I src/SEGGER_RTT/Config -I src/SEGGER_RTT/RTT
endif

# CMSIS (required for CM33)
ifneq ($(wildcard rzv/arm/CMSIS_6),)
  INCLUDES += -I rzv/arm/CMSIS_6/CMSIS/Core/Include
endif

# Driver config (CM33 projects have extra driver configs)
ifneq ($(wildcard rzv_cfg/driver),)
  INCLUDES += -I rzv_cfg/driver
endif

# ---------- Source files (auto-scan) ----------

# BSP core-specific sources
BSP_CORE_SRC = $(wildcard rzv/fsp/src/bsp/cmsis/Device/RENESAS/Source/$(CORE_DIR)/*.c)
BSP_CORE_ASM = $(wildcard rzv/fsp/src/bsp/cmsis/Device/RENESAS/Source/$(CORE_DIR)/*.asm)
BSP_MCU_SRC  = $(wildcard rzv/fsp/src/bsp/mcu/all/*.c)
BSP_MCU_CORE_SRC = $(wildcard rzv/fsp/src/bsp/mcu/all/$(CORE_DIR)/*.c)
BSP_MCU_CORE_ASM = $(wildcard rzv/fsp/src/bsp/mcu/all/$(CORE_DIR)/*.asm)
BSP_BOARD_SRC = $(wildcard rzv/board/rzv2h_evk/*.c)

# Generated sources
GEN_SRC = $(wildcard rzv_gen/*.c)

# FSP driver sources (r_canfd, r_ioport, r_sci_b_uart, etc.)
DRV_SRC = $(wildcard rzv/fsp/src/r_*/*.c)

# User application sources
USR_SRC = $(wildcard src/*.c)
ifneq ($(wildcard src/SEGGER_RTT/RTT),)
  USR_SRC += $(wildcard src/SEGGER_RTT/RTT/*.c)
endif

# Combine all sources
SRCS = $(BSP_CORE_SRC) $(BSP_MCU_SRC) $(BSP_MCU_CORE_SRC) $(BSP_BOARD_SRC) \
       $(GEN_SRC) $(DRV_SRC) $(USR_SRC) $(EXTRA_SRC)
ASMS = $(BSP_CORE_ASM) $(BSP_MCU_CORE_ASM) $(EXTRA_ASM)

# ---------- Build directory and objects ----------

BUILD_DIR = build
OBJS  = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
OBJS += $(patsubst %.asm,$(BUILD_DIR)/%.o,$(ASMS))

# ---------- Output files ----------

ELF = $(BUILD_DIR)/$(PROJECT_NAME).elf

ifeq ($(OUTPUT_MODE),split)
  # CR8: three separate binaries (matching e2 studio output)
  ITCM_BIN   = $(PROJECT_NAME)_itcm.bin
  SRAM_BIN   = $(PROJECT_NAME)_sram.bin
  HEADER_BIN = $(PROJECT_NAME)_header.bin
  BINS = $(ITCM_BIN) $(SRAM_BIN) $(HEADER_BIN)
else
  # CM33: single binary
  BIN1 = $(PROJECT_NAME).bin
  BINS = $(BIN1)
endif

# ---------- Rules ----------

all: $(BINS)
	@$(SIZE) $(ELF)
	@echo "Build complete: $(BINS)"

ifeq ($(OUTPUT_MODE),split)
$(ITCM_BIN): $(ELF)
	$(OBJCOPY) -O binary \
		-j .itcm_load_section0 \
		-j .itcm_load_section1 \
		$< $@

$(SRAM_BIN): $(ELF)
	$(OBJCOPY) -O binary \
		-j .sram_load_section0 \
		-j .sram_load_section1 \
		$< $@

$(HEADER_BIN): $(ELF)
	$(OBJCOPY) -O binary \
		-j .header \
		-j .header_reserved \
		$< $@
else
$(BIN1): $(ELF)
	$(OBJCOPY) -O binary $< $@
endif

$(ELF): $(OBJS)
	@mkdir -p $(dir $@)
	cd $(BUILD_DIR) && $(LD) $(CPU_FLAGS) \
		-T ../$(LINKER_SCRIPT) -L ../Release \
		-Wl,--gc-sections \
		-nostartfiles --specs=rdimon.specs \
		-Wl,--start-group $(patsubst $(BUILD_DIR)/%,./%,$(OBJS)) -lm -lc -lgcc -Wl,--end-group \
		-o $(notdir $@)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $(EXTRA_INC) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -x assembler-with-cpp -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BINS)

# ---------- Install ----------

INSTALL_DIR ?= /mnt

install: all
	@echo "Installing to $(INSTALL_DIR)/"
	install -m 644 $(BINS) $(INSTALL_DIR)/

.PHONY: all clean install
