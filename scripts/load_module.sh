#!/bin/bash
# Script to load msgbuf kernel module and create device node

set -e

MODULE="msgbuf"
DEVICE="/dev/msgbuf"
MODULE_PATH="../module/msgbuf.ko"

echo "========================================="
echo "Loading msgbuf Kernel Module"
echo "========================================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    echo "Usage: sudo $0"
    exit 1
fi

# Check if module file exists
if [ ! -f "$MODULE_PATH" ]; then
    echo "Error: Module file not found: $MODULE_PATH"
    echo "Please compile the module first:"
    echo "  cd ../module && make"
    exit 1
fi

# Unload if already loaded
if lsmod | grep -q "^$MODULE "; then
    echo "Module already loaded, unloading first..."
    rmmod $MODULE 2>/dev/null || true
    sleep 1
fi

# Remove old device node if exists
if [ -e "$DEVICE" ]; then
    echo "Removing old device node..."
    rm -f $DEVICE
fi

# Load the module
echo "Loading module..."
insmod $MODULE_PATH

# Wait a moment for device creation
sleep 1

# Check if module loaded successfully
if ! lsmod | grep -q "^$MODULE "; then
    echo "Error: Module failed to load"
    echo "Check dmesg for errors:"
    dmesg | tail -20
    exit 1
fi

echo "✓ Module loaded successfully"

# Check if device was created automatically
if [ -e "$DEVICE" ]; then
    echo "✓ Device node created automatically: $DEVICE"
else
    echo "Creating device node manually..."
    
    # Get major number
    MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)
    
    if [ -z "$MAJOR" ]; then
        echo "Error: Could not find major number for $MODULE"
        echo "Registered devices:"
        cat /proc/devices
        exit 1
    fi
    
    # Create device node
    mknod $DEVICE c $MAJOR 0
    echo "✓ Device node created: $DEVICE (major: $MAJOR)"
fi

# Set permissions
chmod 666 $DEVICE
echo "✓ Permissions set to 666"

# Show module info
echo ""
echo "Module Information:"
echo "-------------------"
lsmod | grep $MODULE
echo ""
echo "Device Information:"
echo "-------------------"
ls -l $DEVICE
echo ""

# Show /proc entry
if [ -f /proc/msgbuf_stats ]; then
    echo "Statistics available at: /proc/msgbuf_stats"
    echo ""
fi

echo "========================================="
echo "Module loaded successfully!"
echo "========================================="
echo ""
echo "To test the device:"
echo "  cd ../userspace && make && ./test_msgbuf"
echo ""
echo "To check kernel logs:"
echo "  dmesg | tail -20"
echo ""
echo "To unload:"
echo "  sudo ./unload_module.sh"
echo ""