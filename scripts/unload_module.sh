#!/bin/bash
# Script to unload msgbuf kernel module

set -e

MODULE="msgbuf"
DEVICE="/dev/msgbuf"

echo "========================================="
echo "Unloading msgbuf Kernel Module"
echo "========================================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    echo "Usage: sudo $0"
    exit 1
fi

# Check if module is loaded
if ! lsmod | grep -q "^$MODULE "; then
    echo "Module is not loaded"
    exit 0
fi

echo "Module currently loaded:"
lsmod | grep $MODULE

# Show final statistics before unloading
if [ -f /proc/msgbuf_stats ]; then
    echo ""
    echo "Final Statistics:"
    echo "-----------------"
    cat /proc/msgbuf_stats
    echo ""
fi

# Remove device node (if exists and was manually created)
if [ -e "$DEVICE" ]; then
    echo "Removing device node..."
    rm -f $DEVICE
    echo "✓ Device node removed"
fi

# Unload module
echo "Unloading module..."
rmmod $MODULE

# Wait a moment
sleep 1

# Verify module unloaded
if lsmod | grep -q "^$MODULE "; then
    echo "✗ Error: Module still loaded"
    echo "Check for processes using the device:"
    lsof | grep msgbuf || true
    exit 1
fi

echo "✓ Module unloaded successfully"

# Show kernel log
echo ""
echo "Recent kernel messages:"
echo "-----------------------"
dmesg | tail -10

echo ""
echo "========================================="
echo "Module unloaded successfully!"
echo "========================================="
echo ""