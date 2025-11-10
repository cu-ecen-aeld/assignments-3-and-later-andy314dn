/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#include "aesd-circular-buffer.h"
#include <linux/mutex.h>

struct aesd_dev
{
    struct cdev cdev;     /* Char device structure */
    struct aesd_circular_buffer buffer; /* Circular buffer for storing data */
    struct mutex lock;    /* Mutex for synchronizing access */
    char *partial_write;  /* Buffer for incomplete write operations */
    size_t partial_write_size; /* Size of the incomplete write buffer */
};

static long aesd_adjust_file_offset(struct file *filp, unsigned int write_cmd, unsigned int write_cmd_offset);

#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */
