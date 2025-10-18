/*
 * test_msgbuf.c - Basic test program for msgbuf device
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE "/dev/msgbuf"
#define MSGBUF_IOCTL_CLEAR   _IO('M', 1)
#define MSGBUF_IOCTL_GETSIZE _IOR('M', 2, int)
#define MSGBUF_IOCTL_RESIZE  _IOW('M', 3, int)

void print_test_header(const char *test_name) {
    printf("\n=== %s ===\n", test_name);
}

void print_success(const char *msg) {
    printf("✓ %s\n", msg);
}

void print_error(const char *msg) {
    fprintf(stderr, "✗ %s: %s\n", msg, strerror(errno));
}

int main(void) {
    int fd;
    ssize_t ret;
    char buffer[256];
    int size;
    
    printf("========================================\n");
    printf("Message Buffer Device - Basic Test\n");
    printf("========================================\n");
    
    // Test 1: Open device
    print_test_header("Test 1: Opening Device");
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        print_error("Failed to open device");
        printf("\nMake sure:\n");
        printf("  1. Module is loaded: lsmod | grep msgbuf\n");
        printf("  2. Device exists: ls -l /dev/msgbuf\n");
        printf("  3. You have permissions: sudo chmod 666 /dev/msgbuf\n");
        return EXIT_FAILURE;
    }
    print_success("Device opened successfully");
    
    // Test 2: Clear buffer
    print_test_header("Test 2: Clearing Buffer");
    ret = ioctl(fd, MSGBUF_IOCTL_CLEAR);
    if (ret < 0) {
        print_error("Failed to clear buffer");
        close(fd);
        return EXIT_FAILURE;
    }
    print_success("Buffer cleared");
    
    // Test 3: Write to device
    print_test_header("Test 3: Writing to Device");
    const char *test_msg = "Hello from user space! This is a test message.";
    ret = write(fd, test_msg, strlen(test_msg));
    if (ret < 0) {
        print_error("Failed to write");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("✓ Wrote %zd bytes: \"%s\"\n", ret, test_msg);
    
    // Test 4: Get buffer size
    print_test_header("Test 4: Getting Buffer Size");
    size = ioctl(fd, MSGBUF_IOCTL_GETSIZE);
    if (size < 0) {
        print_error("Failed to get buffer size");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("✓ Current buffer size: %d bytes\n", size);
    
    // Test 5: Read from device
    print_test_header("Test 5: Reading from Device");
    lseek(fd, 0, SEEK_SET);  // Reset to beginning
    memset(buffer, 0, sizeof(buffer));
    ret = read(fd, buffer, sizeof(buffer) - 1);
    if (ret < 0) {
        print_error("Failed to read");
        close(fd);
        return EXIT_FAILURE;
    }
    buffer[ret] = '\0';
    printf("✓ Read %zd bytes: \"%s\"\n", ret, buffer);
    
    // Test 6: Verify data matches
    print_test_header("Test 6: Verifying Data");
    if (strcmp(buffer, test_msg) == 0) {
        print_success("Data matches! Read/Write working correctly");
    } else {
        printf("✗ Data mismatch!\n");
        printf("  Expected: %s\n", test_msg);
        printf("  Got:      %s\n", buffer);
    }
    
    // Test 7: Partial read
    print_test_header("Test 7: Partial Read");
    lseek(fd, 0, SEEK_SET);
    memset(buffer, 0, sizeof(buffer));
    ret = read(fd, buffer, 10);  // Read only 10 bytes
    if (ret < 0) {
        print_error("Failed partial read");
    } else {
        buffer[ret] = '\0';
        printf("✓ Partial read (10 bytes): \"%s\"\n", buffer);
    }
    
    // Test 8: Multiple writes (overwrites)
    print_test_header("Test 8: Overwriting Buffer");
    const char *new_msg = "New message overwrites old one!";
    ret = write(fd, new_msg, strlen(new_msg));
    if (ret < 0) {
        print_error("Failed to overwrite");
    } else {
        printf("✓ Overwrote with %zd bytes\n", ret);
        
        lseek(fd, 0, SEEK_SET);
        memset(buffer, 0, sizeof(buffer));
        read(fd, buffer, sizeof(buffer) - 1);
        printf("  Buffer now contains: \"%s\"\n", buffer);
    }
    
    // Test 9: Clear and verify
    print_test_header("Test 9: Clear and Verify Empty");
    ioctl(fd, MSGBUF_IOCTL_CLEAR);
    size = ioctl(fd, MSGBUF_IOCTL_GETSIZE);
    printf("✓ After clear, buffer size: %d bytes\n", size);
    
    // Test 10: Close device
    print_test_header("Test 10: Closing Device");
    close(fd);
    print_success("Device closed successfully");
    
    // Summary
    printf("\n========================================\n");
    printf("All Basic Tests Completed!\n");
    printf("========================================\n");
    printf("\nCheck statistics:\n");
    printf("  cat /proc/msgbuf_stats\n");
    printf("\nCheck kernel logs:\n");
    printf("  dmesg | tail -20\n");
    printf("\n");
    
    return EXIT_SUCCESS;
}