# Makefile for msgbuf kernel module

obj-m += msgbuf.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Default target
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@echo "✓ Module compiled successfully"
	@echo "To load: sudo insmod msgbuf.ko"

# Clean build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.* *.symvers *.order .*.cmd
	@echo "✓ Cleaned module build artifacts"

# Install module (optional)
install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a
	@echo "✓ Module installed"

# Show module info
info:
	@if [ -f msgbuf.ko ]; then \
		modinfo msgbuf.ko; \
	else \
		echo "Module not compiled. Run 'make' first."; \
	fi

# Help
help:
	@echo "Available targets:"
	@echo "  make          - Build the kernel module"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make install  - Install module system-wide"
	@echo "  make info     - Show module information"
	@echo "  make help     - Show this help"

.PHONY: all clean install info help