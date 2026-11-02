# ============================================================
# Toda OS — Build System (Makefile)
# Compiles the entire OS for ARM64
#
# Requirements:
#   - aarch64-linux-gnu-gcc  (ARM64 cross compiler)
#   - qemu-system-aarch64    (ARM64 emulator)
#
# Targets:
#   make          → Build Toda OS
#   make run      → Run with graphical screen emulator window
#   make run-cli  → Run in terminal console
#   make clean    → Clean build files
# ============================================================

# ── Toolchain ────────────────────────────────────────────────
CROSS   := aarch64-linux-gnu-
CC      := $(CROSS)gcc
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

# ── Build Output ──────────────────────────────────────────────
BUILD   := build
TARGET  := $(BUILD)/toda-os

# ── Compiler Flags ────────────────────────────────────────────
CFLAGS  := -march=armv8-a          \
           -mtune=cortex-a72       \
           -mstrict-align          \
           -ffreestanding          \
           -fno-common             \
           -fno-builtin            \
           -fno-stack-protector    \
           -fno-pie                \
           -nostdlib               \
           -nostdinc               \
           -std=gnu11              \
           -O2                     \
           -Wall                   \
           -Wextra                 \
           -Ikernel/core           \
           -Ikernel/drivers/uart   \
           -Ikernel/drivers/timer  \
           -Ikernel/drivers/display \
           -Ikernel/ui             \
           -Ikernel/mm             \
           -Ikernel/security       \
           -Ikernel/proc           \
           -Ikernel/arch/arm64

ASFLAGS := -march=armv8-a

LDFLAGS := -nostdlib               \
           -T tools/linker.ld      \
           -Map=$(BUILD)/toda-os.map

# ── Source Files ──────────────────────────────────────────────
BOOT_SRC := bootloader/boot.S
ASM_SRCS := kernel/arch/arm64/vectors.S

C_SRCS   := kernel/core/main.c             \
            kernel/core/shell.c            \
            kernel/drivers/uart/uart.c     \
            kernel/drivers/timer/timer.c   \
            kernel/drivers/display/fb.c    \
            kernel/ui/gui.c                \
            kernel/mm/memory.c             \
            kernel/security/security.c     \
            kernel/proc/process.c          \
            kernel/arch/arm64/interrupts.c

# ── Object Files ──────────────────────────────────────────────
BOOT_OBJ := $(BUILD)/bootloader/boot.o
ASM_OBJS := $(patsubst %.S, $(BUILD)/%.o, $(ASM_SRCS))
C_OBJS   := $(patsubst %.c, $(BUILD)/%.o, $(C_SRCS))
ALL_OBJS := $(BOOT_OBJ) $(ASM_OBJS) $(C_OBJS)

# ── QEMU Settings ─────────────────────────────────────────────
QEMU     := qemu-system-aarch64

# Graphical Mobile Screen Mode (Opens Display Window on your desktop)
QEMU_GUI_FLAGS := -machine virt \
                  -cpu cortex-a72 \
                  -m 128M \
                  -kernel $(TARGET).img \
                  -device ramfb \
                  -serial mon:stdio

# Terminal Mode
QEMU_CLI_FLAGS := -machine virt \
                  -cpu cortex-a72 \
                  -m 128M \
                  -kernel $(TARGET).img \
                  -nographic \
                  -serial mon:stdio

# ============================================================
# Build Targets
# ============================================================

.PHONY: all run run-cli clean dirs

all: dirs $(TARGET).img
	@echo ""
	@echo "=================================================="
	@echo "  Toda OS Mobile Built Successfully!"
	@echo "  Image: $(TARGET).img"
	@echo "  Run:   make run"
	@echo "=================================================="

# Create build directory structure
dirs:
	@mkdir -p $(BUILD)/bootloader
	@mkdir -p $(BUILD)/kernel/core
	@mkdir -p $(BUILD)/kernel/drivers/uart
	@mkdir -p $(BUILD)/kernel/drivers/timer
	@mkdir -p $(BUILD)/kernel/drivers/display
	@mkdir -p $(BUILD)/kernel/ui
	@mkdir -p $(BUILD)/kernel/mm
	@mkdir -p $(BUILD)/kernel/security
	@mkdir -p $(BUILD)/kernel/proc
	@mkdir -p $(BUILD)/kernel/arch/arm64

# Link everything into final ELF binary
$(TARGET).elf: $(ALL_OBJS)
	@echo "[LD]  Linking Toda OS..."
	@$(LD) $(LDFLAGS) -o $@ $^

# Convert ELF to raw binary image
$(TARGET).img: $(TARGET).elf
	@echo "[IMG] Creating boot image..."
	@$(OBJCOPY) -O binary $< $@
	@echo "[OK]  toda-os.img created"

# Compile bootloader (Assembly)
$(BOOT_OBJ): $(BOOT_SRC)
	@echo "[AS]  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile other assembly source files
$(BUILD)/%.o: %.S
	@echo "[AS]  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile C source files
$(BUILD)/%.o: %.c
	@echo "[CC]  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ── Run Targets ───────────────────────────────────────────────
run: all
	@echo ""
	@echo "Starting Toda OS in QEMU (Opening Mobile Window)..."
	@echo "Press Ctrl+A then X to exit QEMU"
	@echo ""
	$(QEMU) $(QEMU_GUI_FLAGS)

run-cli: all
	@echo ""
	@echo "Starting Toda OS in QEMU (CLI Mode)..."
	@echo ""
	$(QEMU) $(QEMU_CLI_FLAGS)

# ── Clean ─────────────────────────────────────────────────────
clean:
	@echo "Cleaning build..."
	@rm -rf $(BUILD)
	@echo "Clean done."
