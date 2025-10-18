#!/bin/bash
# Script to run all msgbuf tests

set -e

MODULE="msgbuf"
DEVICE="/dev/msgbuf"

echo "========================================"
echo "msgbuf Complete Test Suite"
echo "========================================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    echo "Usage: sudo $0"
    exit 1
fi

# Check if module is loaded
if ! lsmod | grep -q "^$MODULE "; then
    echo "Module not loaded. Loading now..."
    ./load_module.sh
    echo ""
fi

# Check if device exists
if [ ! -e "$DEVICE" ]; then
    echo "Error: Device $DEVICE not found"
    exit 1
fi

# Build userspace programs if needed
echo "Building test programs..."
cd ../userspace
make clean >/dev/null 2>&1
make
cd ../scripts
echo "✓ Test programs built"
echo ""

# Test 1: Basic functionality
echo "========================================"
echo "TEST 1: Basic Functionality"
echo "========================================"
../userspace/test_msgbuf
echo ""

# Show statistics after basic test
echo "Statistics after basic test:"
cat /proc/msgbuf_stats
echo ""

# Test 2: Concurrent access
echo "========================================"
echo "TEST 2: Concurrent Access"
echo "========================================"
../userspace/test_concurrent
echo ""

# Show statistics after concurrent test
echo "Statistics after concurrent test:"
cat /proc/msgbuf_stats
echo ""

# Test 3: Stress test
echo "========================================"
echo "TEST 3: Stress Test"
echo "========================================"
echo "WARNING: This may take 1-2 minutes..."
../userspace/test_stress
echo ""

# Final statistics
echo "========================================"
echo "Final Statistics"
echo "========================================"
cat /proc/msgbuf_stats
echo ""

# Check for kernel errors
echo "========================================"
echo "Checking for Kernel Errors"
echo "========================================"
ERRORS=$(dmesg | grep -E "(error|oops|bug|warning)" -i | tail -10)
if [ -z "$ERRORS" ]; then
    echo "✓ No kernel errors found"
else
    echo "⚠️  Found potential issues:"
    echo "$ERRORS"
fi
echo ""

# Summary
echo "========================================"
echo "Test Suite Complete!"
echo "========================================"
echo ""
echo "All tests passed successfully!"
echo ""
echo "To view full kernel log:"
echo "  dmesg | tail -50"
echo ""
echo "To unload module:"
echo "  sudo ./unload_module.sh"
echo ""