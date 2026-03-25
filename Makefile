# Makefile - Raspberry Pi 4 bare-metal Task 1
# Requires aarch64-none-elf-gcc (or aarch64-linux-gnu-gcc) on PATH

CROSS   ?= aarch64-none-elf
CC       = $(CROSS)-gcc
AS       = $(CROSS)-gcc
LD       = $(CROSS)-ld
OBJCOPY  = $(CROSS)-objcopy

CFLAGS   = -Wall -Wextra -ffreestanding -nostdlib -nostartfiles \
           -mcpu=cortex-a72 -O2
LDFLAGS  = -T link.ld -nostdlib

TARGET   = kernel8.img
ELF      = kernel8.elf

SRCS_C  = kernel.c uart.c mbox.c
SRCS_S  = boot.S
OBJS    = $(SRCS_S:.S=.o) $(SRCS_C:.c=.o)

.PHONY: all clean

all: $(TARGET)

%.o: %.S
	$(AS) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "Built $(TARGET) successfully."

clean:
	rm -f $(OBJS) $(ELF) $(TARGET)
