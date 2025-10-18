/*
 * msgbuf.c - Message Buffer Character Device Driver
 * 
 * A simple character device that acts as a message buffer.
 * Supports read, write, ioctl operations with synchronization.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#include "msgbuf.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Message Buffer Character Device");
MODULE_VERSION("1.0");

// Device structures
static int major_number;
static struct class *msgbuf_class = NULL;
static struct device *msgbuf_device = NULL;
static struct cdev msgbuf_cdev;

// Buffer
static char *device_buffer = NULL;
static size_t buffer_len = 0;

// Synchronization
static DEFINE_MUTEX(buffer_mutex);
static DEFINE_SPINLOCK(stats_lock);

// Statistics
static struct {
    unsigned long opens;
    unsigned long releases;
    unsigned long reads;
    unsigned long writes;
    unsigned long bytes_read;
    unsigned long bytes_written;
    unsigned long ioctl_calls;
} stats = {0};

// /proc entry
static struct proc_dir_entry *proc_entry = NULL;

// Function prototypes
static int msgbuf_open(struct inode *inode, struct file *file);
static int msgbuf_release(struct inode *inode, struct file *file);
static ssize_t msgbuf_read(struct file *file, char __user *user_buf,
                           size_t count, loff_t *offset);
static ssize_t msgbuf_write(struct file *file, const char __user *user_buf,
                            size_t count, loff_t *offset);
static long msgbuf_ioctl(struct file *file, unsigned int cmd, 
                        unsigned long arg);

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = msgbuf_open,
    .release = msgbuf_release,
    .read = msgbuf_read,
    .write = msgbuf_write,
    .unlocked_ioctl = msgbuf_ioctl,
    .llseek = default_llseek,
};

/*
 * Device open operation
 */
static int msgbuf_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "msgbuf: Device opened (PID: %d)\n", current->pid);
    
    spin_lock(&stats_lock);
    stats.opens++;
    spin_unlock(&stats_lock);
    
    return 0;
}

/*
 * Device release operation
 */
static int msgbuf_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "msgbuf: Device closed (PID: %d)\n", current->pid);
    
    spin_lock(&stats_lock);
    stats.releases++;
    spin_unlock(&stats_lock);
    
    return 0;
}

/*
 * Device read operation
 */
static ssize_t msgbuf_read(struct file *file, char __user *user_buf,
                           size_t count, loff_t *offset)
{
    ssize_t ret;
    size_t to_read;
    
    // Lock buffer for reading
    if (mutex_lock_interruptible(&buffer_mutex)) {
        return -ERESTARTSYS;
    }
    
    // Check for EOF
    if (*offset >= buffer_len) {
        ret = 0;
        goto out_unlock;
    }
    
    // Calculate how much to read
    to_read = min(count, buffer_len - (size_t)*offset);
    
    // Copy to user space
    if (copy_to_user(user_buf, device_buffer + *offset, to_read)) {
        ret = -EFAULT;
        goto out_unlock;
    }
    
    // Update offset
    *offset += to_read;
    ret = to_read;
    
    printk(KERN_DEBUG "msgbuf: Read %zd bytes at offset %lld\n", 
           to_read, *offset - to_read);

out_unlock:
    mutex_unlock(&buffer_mutex);
    
    // Update statistics
    if (ret > 0) {
        spin_lock(&stats_lock);
        stats.reads++;
        stats.bytes_read += ret;
        spin_unlock(&stats_lock);
    }
    
    return ret;
}

/*
 * Device write operation
 */
static ssize_t msgbuf_write(struct file *file, const char __user *user_buf,
                            size_t count, loff_t *offset)
{
    ssize_t ret;
    size_t to_write;
    
    // Lock buffer for writing
    if (mutex_lock_interruptible(&buffer_mutex)) {
        return -ERESTARTSYS;
    }
    
    // Truncate if exceeds buffer size
    to_write = min(count, (size_t)BUFFER_SIZE);
    
    // Copy from user space
    if (copy_from_user(device_buffer, user_buf, to_write)) {
        ret = -EFAULT;
        goto out_unlock;
    }
    
    // Update buffer length
    buffer_len = to_write;
    
    // Null-terminate for safety
    if (buffer_len < BUFFER_SIZE) {
        device_buffer[buffer_len] = '\0';
    }
    
    ret = to_write;
    
    printk(KERN_INFO "msgbuf: Wrote %zd bytes\n", to_write);

out_unlock:
    mutex_unlock(&buffer_mutex);
    
    // Update statistics
    if (ret > 0) {
        spin_lock(&stats_lock);
        stats.writes++;
        stats.bytes_written += ret;
        spin_unlock(&stats_lock);
    }
    
    return ret;
}

/*
 * Device ioctl operation
 */
static long msgbuf_ioctl(struct file *file, unsigned int cmd, 
                        unsigned long arg)
{
    long ret = 0;
    int size;
    
    printk(KERN_DEBUG "msgbuf: ioctl called with cmd=%u\n", cmd);
    
    // Update ioctl statistics
    spin_lock(&stats_lock);
    stats.ioctl_calls++;
    spin_unlock(&stats_lock);
    
    switch (cmd) {
    case MSGBUF_IOCTL_CLEAR:
        // Clear the buffer
        if (mutex_lock_interruptible(&buffer_mutex)) {
            return -ERESTARTSYS;
        }
        
        memset(device_buffer, 0, BUFFER_SIZE);
        buffer_len = 0;
        
        mutex_unlock(&buffer_mutex);
        
        printk(KERN_INFO "msgbuf: Buffer cleared\n");
        ret = 0;
        break;
        
    case MSGBUF_IOCTL_GETSIZE:
        // Get current buffer size
        if (mutex_lock_interruptible(&buffer_mutex)) {
            return -ERESTARTSYS;
        }
        
        size = buffer_len;
        
        mutex_unlock(&buffer_mutex);
        
        ret = size;
        break;
        
    case MSGBUF_IOCTL_RESIZE:
        // Not implemented for fixed-size buffer
        printk(KERN_WARNING "msgbuf: RESIZE not supported\n");
        ret = -EINVAL;
        break;
        
    default:
        printk(KERN_WARNING "msgbuf: Invalid ioctl command: %u\n", cmd);
        ret = -EINVAL;
        break;
    }
    
    return ret;
}

/*
 * /proc file show function
 */
static int msgbuf_proc_show(struct seq_file *m, void *v)
{
    unsigned long opens, releases, reads, writes;
    unsigned long bytes_read, bytes_written, ioctl_calls;
    size_t current_len;
    
    // Read statistics atomically
    spin_lock(&stats_lock);
    opens = stats.opens;
    releases = stats.releases;
    reads = stats.reads;
    writes = stats.writes;
    bytes_read = stats.bytes_read;
    bytes_written = stats.bytes_written;
    ioctl_calls = stats.ioctl_calls;
    spin_unlock(&stats_lock);
    
    // Read buffer length
    mutex_lock(&buffer_mutex);
    current_len = buffer_len;
    mutex_unlock(&buffer_mutex);
    
    seq_printf(m, "Message Buffer Statistics\n");
    seq_printf(m, "=========================\n");
    seq_printf(m, "Opens:          %lu\n", opens);
    seq_printf(m, "Releases:       %lu\n", releases);
    seq_printf(m, "Reads:          %lu\n", reads);
    seq_printf(m, "Writes:         %lu\n", writes);
    seq_printf(m, "IOCTL calls:    %lu\n", ioctl_calls);
    seq_printf(m, "Bytes Read:     %lu\n", bytes_read);
    seq_printf(m, "Bytes Written:  %lu\n", bytes_written);
    seq_printf(m, "Buffer Size:    %d\n", BUFFER_SIZE);
    seq_printf(m, "Current Length: %zu\n", current_len);
    
    return 0;
}

/*
 * /proc file open function
 */
static int msgbuf_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, msgbuf_proc_show, NULL);
}

/*
 * /proc file operations
 */
static const struct proc_ops msgbuf_proc_ops = {
    .proc_open = msgbuf_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

/*
 * Module initialization
 */
static int __init msgbuf_init(void)
{
    int ret;
    dev_t dev;
    
    printk(KERN_INFO "msgbuf: Initializing module\n");
    
    // Allocate buffer
    device_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer) {
        printk(KERN_ERR "msgbuf: Failed to allocate buffer\n");
        return -ENOMEM;
    }
    
    // Allocate character device region
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "msgbuf: Failed to allocate device region\n");
        goto fail_alloc;
    }
    
    major_number = MAJOR(dev);
    printk(KERN_INFO "msgbuf: Registered with major number %d\n", major_number);
    
    // Initialize cdev structure
    cdev_init(&msgbuf_cdev, &fops);
    msgbuf_cdev.owner = THIS_MODULE;
    
    // Add character device
    ret = cdev_add(&msgbuf_cdev, dev, 1);
    if (ret < 0) {
        printk(KERN_ERR "msgbuf: Failed to add cdev\n");
        goto fail_cdev;
    }
    
    // Create device class
    msgbuf_class = class_create(DEVICE_NAME);
    if (IS_ERR(msgbuf_class)) {
        printk(KERN_ERR "msgbuf: Failed to create class\n");
        ret = PTR_ERR(msgbuf_class);
        goto fail_class;
    }
    
    // Create device
    msgbuf_device = device_create(msgbuf_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(msgbuf_device)) {
        printk(KERN_ERR "msgbuf: Failed to create device\n");
        ret = PTR_ERR(msgbuf_device);
        goto fail_device;
    }
    
    // Create /proc entry
    proc_entry = proc_create("msgbuf_stats", 0444, NULL, &msgbuf_proc_ops);
    if (!proc_entry) {
        printk(KERN_WARNING "msgbuf: Failed to create /proc entry\n");
        // Not fatal, continue
    }
    
    printk(KERN_INFO "msgbuf: Module loaded successfully\n");
    printk(KERN_INFO "msgbuf: Device created at /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "msgbuf: Statistics available at /proc/msgbuf_stats\n");
    
    return 0;

fail_device:
    class_destroy(msgbuf_class);
fail_class:
    cdev_del(&msgbuf_cdev);
fail_cdev:
    unregister_chrdev_region(dev, 1);
fail_alloc:
    kfree(device_buffer);
    return ret;
}

/*
 * Module cleanup
 */
static void __exit msgbuf_exit(void)
{
    dev_t dev = MKDEV(major_number, 0);
    
    printk(KERN_INFO "msgbuf: Unloading module\n");
    
    // Remove /proc entry
    if (proc_entry) {
        proc_remove(proc_entry);
    }
    
    // Destroy device
    if (msgbuf_device) {
        device_destroy(msgbuf_class, dev);
    }
    
    // Destroy class
    if (msgbuf_class) {
        class_destroy(msgbuf_class);
    }
    
    // Remove character device
    cdev_del(&msgbuf_cdev);
    
    // Unregister device region
    unregister_chrdev_region(dev, 1);
    
    // Free buffer
    kfree(device_buffer);
    
    printk(KERN_INFO "msgbuf: Module unloaded\n");
}

module_init(msgbuf_init);
module_exit(msgbuf_exit);
