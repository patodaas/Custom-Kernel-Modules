/*
 * test_concurrent.c - Multi-process concurrent test for msgbuf device
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define DEVICE "/dev/msgbuf"
#define NUM_PROCESSES 10
#define ITERATIONS_PER_PROCESS 50

void child_process(int proc_id) {
    int fd;
    char buffer[128];
    int success_count = 0;
    int fail_count = 0;
    
    // Open device
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Process %d: Failed to open device: %s\n", 
                proc_id, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Seed random for this process
    srand(time(NULL) ^ (getpid() << 16));
    
    // Perform multiple read/write operations
    for (int i = 0; i < ITERATIONS_PER_PROCESS; i++) {
        // Write operation
        snprintf(buffer, sizeof(buffer), 
                 "Process %d, iteration %d, PID %d\n", 
                 proc_id, i, getpid());
        
        ssize_t written = write(fd, buffer, strlen(buffer));
        if (written > 0) {
            success_count++;
        } else {
            fail_count++;
        }
        
        // Small random delay (0-10ms)
        usleep(rand() % 10000);
        
        // Read operation (every 10 iterations)
        if (i % 10 == 0) {
            char read_buf[256];
            lseek(fd, 0, SEEK_SET);
            read(fd, read_buf, sizeof(read_buf) - 1);
        }
    }
    
    close(fd);
    
    printf("Process %d completed: %d successes, %d failures\n", 
           proc_id, success_count, fail_count);
    
    exit(EXIT_SUCCESS);
}

int main(void) {
    pid_t pids[NUM_PROCESSES];
    int status;
    int success_processes = 0;
    int failed_processes = 0;
    
    printf("========================================\n");
    printf("Concurrent Multi-Process Test\n");
    printf("========================================\n");
    printf("Spawning %d processes...\n", NUM_PROCESSES);
    printf("Each will perform %d iterations\n", ITERATIONS_PER_PROCESS);
    printf("\n");
    
    // Record start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Fork child processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            fprintf(stderr, "Failed to fork process %d: %s\n", 
                    i, strerror(errno));
            // Kill already spawned children
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGTERM);
            }
            return EXIT_FAILURE;
        }
        
        if (pid == 0) {
            // Child process
            child_process(i);
            // Never reaches here
        }
        
        // Parent process
        pids[i] = pid;
        printf("Spawned process %d (PID %d)\n", i, pid);
    }
    
    printf("\nWaiting for all processes to complete...\n\n");
    
    // Wait for all children
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = wait(&status);
        
        if (pid < 0) {
            fprintf(stderr, "Wait failed: %s\n", strerror(errno));
            continue;
        }
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            success_processes++;
        } else {
            failed_processes++;
            fprintf(stderr, "Process PID %d failed with status %d\n", 
                    pid, WEXITSTATUS(status));
        }
    }
    
    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Summary
    printf("\n========================================\n");
    printf("Concurrent Test Results\n");
    printf("========================================\n");
    printf("Total processes:     %d\n", NUM_PROCESSES);
    printf("Successful:          %d\n", success_processes);
    printf("Failed:              %d\n", failed_processes);
    printf("Total operations:    %d\n", NUM_PROCESSES * ITERATIONS_PER_PROCESS);
    printf("Elapsed time:        %.2f seconds\n", elapsed);
    printf("Operations/sec:      %.0f\n", 
           (NUM_PROCESSES * ITERATIONS_PER_PROCESS) / elapsed);
    
    if (failed_processes == 0) {
        printf("\n✓ All processes completed successfully!\n");
    } else {
        printf("\n✗ Some processes failed!\n");
    }
    
    printf("\nCheck statistics:\n");
    printf("  cat /proc/msgbuf_stats\n");
    printf("\nCheck for kernel errors:\n");
    printf("  dmesg | grep -i error\n");
    printf("  dmesg | grep -i oops\n");
    printf("\n");
    
    return (failed_processes == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}