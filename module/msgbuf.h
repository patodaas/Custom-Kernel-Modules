#ifndef MSGBUF_H
#define MSGBUF_H

#include <linux/ioctl.h>

// Device name
#define DEVICE_NAME "msgbuf"

// Buffer configuration
#define BUFFER_SIZE 4096

// ioctl commands
#define MSGBUF_IOCTL_CLEAR   _IO('M', 1)
#define MSGBUF_IOCTL_GETSIZE _IOR('M', 2, int)
#define MSGBUF_IOCTL_RESIZE  _IOW('M', 3, int)

#endif // MSGBUF_H