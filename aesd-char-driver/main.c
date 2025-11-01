/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h> // for kmalloc and kfree
#include <linux/mutex.h> // for mutex
#include "aesdchar.h"
#include "aesd-circular-buffer.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("andy314dn");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

/**
 * @brief Associate the device structure with the file pointer for future operations.
 * 
 * @param inode: Represents the device file in the filesystem.
 * @param filp: File pointer for the opened file, used to store private data.
 * 
 * @return 0 on success.
 */
int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    // Sets filp->private_data to point to the aesd_dev structure, retrieved via container_of from inode->i_cdev.
    filp->private_data = container_of(inode->i_cdev, struct aesd_dev, cdev);
    return 0;
}

/**
 * @brief Release the device when the file is closed.
 * 
 * @param inode: Represents the device file.
 * @param filp: File pointer for the file being closed.
 * 
 * @return 0 to indicate successful release.
 */
int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    return 0;
}

/**
 * @brief Reads data from the circular buffer and copies it to user space.
 * 
 * @param filp: File pointer containing the device structure in private_data.
 * @param buf: User-space buffer to copy data into.
 * @param count: Number of bytes requested to read.
 * @param f_pos: File position offset for reading.
 * 
 * @return Number of bytes read, 0 for EOF, or -EFAULT on copy failure.
 */
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *entry;
    size_t entry_offset;
    size_t bytes_to_copy;

    PDEBUG("read %zu bytes with offset %lld", count, *f_pos);

    // Locks the device mutex to ensure thread-safe access to the circular buffer.
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Finds the buffer entry and offset for the current file position using aesd_circular_buffer_find_entry_offset_for_fpos.
    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer, *f_pos, &entry_offset);
    if (!entry || !entry->buffptr || entry->size == 0) {
        retval = 0; // EOF or invalid entry
        goto out;
    }

    // Validates the entry's buffptr and size to prevent invalid memory access.
    bytes_to_copy = min(count, entry->size - entry_offset);
    if (bytes_to_copy == 0) {
        retval = 0; // No more data in this entry
        goto out;
    }

    // Copies the minimum of requested bytes or remaining entry bytes to user space.
    if (copy_to_user(buf, entry->buffptr + entry_offset, bytes_to_copy)) {
        retval = -EFAULT;
        goto out;
    }

    // Updates file position and returns the number of bytes read, or 0 for EOF, or -EFAULT on copy failure.
    *f_pos += bytes_to_copy;
    retval = bytes_to_copy;

out:
    mutex_unlock(&dev->lock);
    return retval;
}

/**
 * @brief Writes data from user space to the device, handling partial writes and newline-terminated entries.
 * 
 * @param filp: File pointer containing the device structure in private_data.
 * @param buf: User-space buffer containing data to write.
 * @param count: Number of bytes to write.
 * @param f_pos: File position offset (not modified in this implementation).
 * 
 * @return Number of bytes written, or an error code (-ENOMEM, -EFAULT, or -ERESTARTSYS).
 */
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    struct aesd_dev *dev = filp->private_data;
    char *new_data = NULL;
    struct aesd_buffer_entry entry;
    size_t total_size = 0;
    size_t i;

    PDEBUG("write %zu bytes with offset %lld", count, *f_pos);

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Allocate memory for new data plus existing partial write
    total_size = count + dev->partial_write_size;
    new_data = kmalloc(total_size, GFP_KERNEL);
    if (!new_data)
        goto out;

    // Copy existing partial write (if any) to new buffer
    if (dev->partial_write) {
        memcpy(new_data, dev->partial_write, dev->partial_write_size);
        kfree(dev->partial_write);
        dev->partial_write = NULL;
        dev->partial_write_size = 0;
    }

    // Copy user data into kernel space
    if (copy_from_user(new_data + dev->partial_write_size, buf, count)) {
        kfree(new_data);
        retval = -EFAULT;
        goto out;
    }

    // Check for a single newline
    for (i = 0; i < total_size; i++) {
        if (new_data[i] == '\n') {
            // Create entry with all data up to and including newline
            entry.size = i + 1;
            entry.buffptr = kmalloc(entry.size, GFP_KERNEL);
            if (!entry.buffptr) {
                kfree(new_data);
                retval = -ENOMEM;
                goto out;
            }
            memcpy((char *)entry.buffptr, new_data, entry.size);
            // Add entry to circular buffer
            if (dev->buffer.full) {
                kfree((void *)dev->buffer.entry[dev->buffer.out_offs].buffptr);
            }
            aesd_circular_buffer_add_entry(&dev->buffer, &entry);
            // Store remaining data (if any) as partial write
            if (i + 1 < total_size) {
                dev->partial_write_size = total_size - (i + 1);
                dev->partial_write = kmalloc(dev->partial_write_size, GFP_KERNEL);
                if (!dev->partial_write) {
                    kfree(new_data);
                    retval = -ENOMEM;
                    goto out;
                }
                memcpy(dev->partial_write, new_data + i + 1, dev->partial_write_size);
            }
            kfree(new_data);
            retval = count;
            goto out;
        }
    }

    // No newline found, store all data as partial write
    dev->partial_write_size = total_size;
    dev->partial_write = kmalloc(total_size, GFP_KERNEL);
    if (!dev->partial_write) {
        kfree(new_data);
        retval = -ENOMEM;
        goto out;
    }
    memcpy(dev->partial_write, new_data, total_size);
    kfree(new_data);
    retval = count;

out:
    mutex_unlock(&dev->lock);
    return retval;
}

/**
 * @brief File operations structure for the AESD character device.
 */
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

/**
 * @brief Initializes and registers the character device with the kernel.
 * 
 * @param dev: Pointer to the aesd_dev structure containing the character device (cdev).
 * 
 * @return 0 on success or a negative error code if cdev_add fails.
 */
static int aesd_setup_cdev(struct aesd_dev *dev)
{
    // Creates a device number using the major and minor numbers.
    int err, devno = MKDEV(aesd_major, aesd_minor);

    // Initializes the cdev structure with the file operations (aesd_fops) and module owner.
    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    // Adds the character device to the system with cdev_add.
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}

/**
 * @brief Initializes the AESD character driver module during loading.
 * 
 * @return 0 on success or a negative error code on failure.
 */
int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    // Allocates a dynamic major number for the device using alloc_chrdev_region.
    result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    // Initializes the aesd_device structure, including the circular buffer and mutex.
    memset(&aesd_device, 0, sizeof(struct aesd_dev));

    aesd_circular_buffer_init(&aesd_device.buffer);
    mutex_init(&aesd_device.lock);

    // Sets up the character device with aesd_setup_cdev
    result = aesd_setup_cdev(&aesd_device);

    // If setup fails, unregisters the device region and returns the error.
    if (result) {
        unregister_chrdev_region(dev, 1);
    }
    return result;
}

/**
 * @brief Cleans up resources when the module is unloaded.
 * 
 */
void aesd_cleanup_module(void)
{
    // Creates the device number from major and minor numbers.
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    struct aesd_buffer_entry *entry;
    uint8_t index;

    // Removes the character device from the system with cdev_del.
    cdev_del(&aesd_device.cdev);

    // Frees all buffer entries in the circular buffer using AESD_CIRCULAR_BUFFER_FOREACH.
    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.buffer, index) {
        if (entry->buffptr)
            kfree(entry->buffptr);
    }

    // Frees the partial write buffer to prevent memory leaks.
    if (aesd_device.partial_write)
        kfree(aesd_device.partial_write);

    // Destroys the mutex to release its resources.
    mutex_destroy(&aesd_device.lock);
    // Unregisters the device region to free the major number.
    unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
