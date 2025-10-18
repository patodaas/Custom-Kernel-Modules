/*
 * test_stress.c - Stress test for msgbuf device
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
#include <time.h>

#define DEVICE "/dev/msgbuf"
#define MSGBUF_IOCTL_CLEAR   _IO('M', 1)
#define MSGBUF_IOCTL_GETSIZE _IOR('M', 2, int)

#define BUFFER_SIZE 4096
#define NUM_ITERATIONS 1000

void test_rapid_open_close(void) {
    printf("\n=== Test 1: Rapid Open/Close ===\n");
    
    int success = 0;
    int failures = 0;
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int fd = open(DEVICE, O_RDWR);
        if (fd < 0) {
            failures++;
            continue;
        }
        close(fd);
        success++;
        
        if ((i + 1) % 100 == 0) {
            printf("  %d iterations completed\r", i + 1);
            fflush(stdout);
        }
    }
    
    printf("\n✓ Completed: %d successes, %d failures\n", success, failures);
}

void test_large_writes(void) {
    printf("\n=== Test 2: Large Writes (Buffer Overflow) ===\n");
    
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device\n");
        return;
    }
    
    // Try to write more than buffer size
    char *large_buffer = malloc(BUFFER_SIZE * 2);
    if (!large_buffer) {
        fprintf(stderr, "Failed to allocate memory\n");
        close(fd);
        return;
    }
    
    memset(large_buffer, 'A', BUFFER_SIZE * 2);
    
    ssize_t written = write(fd, large_buffer, BUFFER_SIZE * 2);
    printf("Attempted to write %d bytes\n", BUFFER_SIZE * 2);
    printf("Actually written: %zd bytes\n", written);
    
    int size = ioctl(fd, MSGBUF_IOCTL_GETSIZE);
    printf("Buffer size after write: %d bytes\n", size);
    
    if (size <= BUFFER_SIZE) {
        printf("✓ Buffer overflow prevented correctly\n");
    } else {
        printf("✗ WARNING: Buffer overflow occurred!\n");
    }
    
    free(large_buffer);
    close(fd);
}

void test_rapid_read_write(void) {
    printf("\n=== Test 3: Rapid Read/Write ===\n");
    
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device\n");
        return;
    }
    
    char write_buf[64];
    char read_buf[64];
    int errors = 0;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Write
        snprintf(write_buf, sizeof(write_buf), "Iteration %d", i);
        ssize_t w = write(fd, write_buf, strlen(write_buf));
        
        // Read back
        lseek(fd, 0, SEEK_SET);
        ssize_t r = read(fd, read_buf, sizeof(read_buf) - 1);
        
        if (w < 0 || r < 0) {
            errors++;
        }
        
        if ((i + 1) % 100 == 0) {
            printf("  %d iterations completed\r", i + 1);
            fflush(stdout);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("\n✓ Completed %d iterations in %.2f seconds\n", 
           NUM_ITERATIONS, elapsed);
    printf("  Operations/sec: %.0f\n", (NUM_ITERATIONS * 2) / elapsed);
    printf("  Errors: %d\n", errors);
    
    close(fd);
}

void test_ioctl_stress(void) {
    printf("\n=== Test 4: IOCTL Stress ===\n");
    
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device\n");
        return;
    }
    
    int errors = 0;
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Write some data
        char buf[32];
        snprintf(buf, sizeof(buf), "Data %d", i);
        write(fd, buf, strlen(buf));
        
        // Get size
        int size = ioctl(fd, MSGBUF_IOCTL_GETSIZE);
        if (size < 0) {
            errors++;
        }
        
        // Clear
        if (i % 10 == 0) {
            if (ioctl(fd, MSGBUF_IOCTL_CLEAR) < 0) {
                errors++;
            }
        }
        
        if ((i + 1) % 100 == 0) {
            printf("  %d iterations completed\r", i + 1);
            fflush(stdout);
        }
    }
    
    printf("\n✓ Completed %d IOCTL operations\n", NUM_ITERATIONS * 2);
    printf("  Errors: %d\n", errors);
    
    close(fd);
}

void test_seek_operations(void) {
    printf("\n=== Test 5: Seek Operations ===\n");
    
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device\n");
        return;
    }
    
    // Write test data
    const char *data = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    write(fd, data, strlen(data));
    
    char buf[2];
    int errors = 0;
    
    // Test various seek positions
    for (int i = 0; i < (int)strlen(data); i++) {
        lseek(fd, i, SEEK_SET);
        ssize_t r = read(fd, buf, 1);
        buf[1] = '\0';
        
        if (r != 1 || buf[0] != data[i]) {
            errors++;
        }
    }
    
    printf("✓ Tested %zu seek positions\n", strlen(data));
    printf("  Errors: %d\n", errors);
    
    close(fd);
}

int main(void) {
    printf("========================================\n");
    printf("Message Buffer Device - Stress Test\n");
    printf("========================================\n");
    printf("WARNING: This test performs %d iterations\n", NUM_ITERATIONS);
    printf("         and may take a minute to complete.\n");
    printf("\n");
    
    // Check device exists
    if (access(DEVICE, F_OK) != 0) {
        fprintf(stderr, "Device %s not found!\n", DEVICE);
        fprintf(stderr, "Make sure the module is loaded.\n");
        return EXIT_FAILURE;
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Run stress tests
    test_rapid_open_close();
    test_large_writes();
    test_rapid_read_write();
    test_ioctl_stress();
    test_seek_operations();
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Summary
    printf("\n========================================\n");
    printf("Stress Test Summary\n");
    printf("========================================\n");
    printf("Total time: %.2f seconds\n", elapsed);
    printf("\n✓ All stress tests completed!\n");
    printf("\nCheck statistics:\n");
    printf("  cat /proc/msgbuf_stats\n");
    printf("\nCheck for errors:\n");
    printf("  dmesg | grep -E '(error|oops|bug)' -i\n");
    printf("\n");
    
    return EXIT_SUCCESS;
}