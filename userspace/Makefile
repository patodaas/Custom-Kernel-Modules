# Makefile for msgbuf userspace test programs

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O2
LDFLAGS = -lrt

# Targets
TARGETS = test_msgbuf test_concurrent test_stress

.PHONY: all clean test

# Default target
all: $(TARGETS)
	@echo "✓ All test programs compiled"

# Build test_msgbuf
test_msgbuf: test_msgbuf.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build test_concurrent
test_concurrent: test_concurrent.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build test_stress
test_stress: test_stress.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean
clean:
	rm -f $(TARGETS) *.o
	@echo "✓ Cleaned userspace programs"

# Run basic test
test: test_msgbuf
	@echo "Running basic test..."
	@./test_msgbuf

# Help
help:
	@echo "Available targets:"
	@echo "  make              - Build all test programs"
	@echo "  make clean        - Remove binaries"
	@echo "  make test         - Run basic test"
	@echo "  make help         - Show this help"