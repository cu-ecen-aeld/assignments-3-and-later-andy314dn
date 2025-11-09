# Implementation Suggestion

In the current work, we will implement `llseek` and modidy `filp->f_pos` in an `ioctl`.

Explanation of `whence` argument - 3rd argument of `lseek()` in user space:

```txt
NAME
       lseek - reposition read/write file offset

SYNOPSIS
       #include <sys/types.h>
       #include <unistd.h>

       off_t lseek(int fd, off_t offset, int whence);

DESCRIPTION
       lseek() repositions the file offset of the open file description associated with the file descriptor fd
       to the argument offset according to the directive whence as follows:

       SEEK_SET
              The file offset is set to offset bytes.

       SEEK_CUR
              The file offset is set to its current location plus offset bytes.

       SEEK_END
              The file offset is set to the size of the file plus offset bytes.
```

## `llseek` driver implementation

Snippet from kernel code:

```c
struct file_operations {
        // ...
        loff_t (*llseek) (struct file *, loff_t, int);
        ssize_t (*read) (struct file *, char *, size_t, loff_t *);
        ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
        // ...
    };
```

- “If the `llseek` method is missing from the device’s operations, the default implementation in the kernel performs seek by modifying `filp->fpos`.”
- Several wrappers around generic `llseek` which seek for you which you can call from your `llseek` method
  - `fixed_size_llseek()` uses a size you provide
- What should we use for size?
  - The total size of all content of the circular buffer

=> this is what we should do:  Add your own `llseek` function, **with locking and logging**, but use `fixed_size_llseek` for logic.

- “For the `lseek` system call to work correctly, the read and write methods must cooperate by using and updating the offset item they receive as an argument.”
  - `read` function:
    - Must set `*f_pos` to `*f_pos + retcount` where `retcount` is the number of bytes read
  - `write` function:
    - Must set `*f_pos` to `*f_pos + retcount` where `retcount` is the number of bytes written

## `ioctl` implementation

Use the provided [aesd_ioctl.h](./aesd_ioctl.h) file you can share with your `aesdsocket` implementation.

```c
// ioctl prototype
#include <sys/ioctl.h>

int ioctl(int fd, unsigned long request, ...);
```

- In user space, `#include` the `aesd_ioctl.h` file
- Use the `ioctl` command with `fd` representing the driver
- Pass the filled in structure to the driver via `ioctl`, see code suggestion below

```c
// code suggestion
#include "aesd_ioctl.h"

struct aesd_seekto seekto;
seekto.write_cmd = write_cmd;
seekto.write_cmd_offset = offset;
// pass the filled in structure to the driver via ioctl
int result_ret = ioctl(fd, AESDCHAR_IOCSEEKTO, &seekto);
```

- My `aesdsocket` uses a `FILE *` to access my driver.  How do I get the `fd` used for `ioctl`?
  - Use `fileno()`

## `ioctl` implementation - driver

- In kernel space, `#include` the `aesd_ioctl.h` file
- Use `copy_from_user` to obtain the value from userspace. See below code suggestion

```c
// code suggestion
case AESDCHAR_IOCSEEKTO: {
    struct aesd_seekto seekto;
    if (copy_from_user(&seekto, (const void __user *)arg, sizeof(seekto)) != 0) {
        retval = EFAULT;
    } else {
        retval = aesd_adjust_file_offset(filp, seekto.write_cmd, seekto.write_cmd_offset);
    }
    break;
}
```

- Adjusting the file offset from `ioctl` with `aesd_adjust_file_offset()`, see code suggestion below:

```c
/**
 * Adjust the file offset (f_pos) parameter of @param filp based on the location specified by
 * @param write_cmd (the zero referenced command to locate)
 * and @param write_cmd_offset (the zero referenced offset into the command)
 *
 * @return 0 if successful, negative if error occured:
 *        -ERESTARTSYS if mutex could not be obtained
 *        -EINVAL if write command or write_cmd_offset was out of range
 */ 
 static long aesd_adjust_file_offset(struct file *filp, unsigned int write_cmd,
                                     unsigned int write_cmd_offset);
```

- Here are what we should consider with the function implementation:
  - Check for valid `write_cmd` and `write_cmd_offset` values
    - When would values be invalid?
      - specify a command that we haven’t written yet
      - out of range cmd (since we only buffer 10 commands, so 11 and above are out of range)
      - `write_cmd_offset` is ≥ size of command
  - Calculate the start offset to `write_cmd`
    - Add length of each write between the output pointer and `write_cmd` 
  - Add `write_cmd_offset` (to give you the total final position inside the circular buffer)
    - look at the circular buffer as a long string of bytes for every single element and you're going to figure out what's the byte offset for write command
    - add in the additional offset for the right command offset
  - Save as `filp->f_pos`
    - then update that byte count into the file pointer f_pos as a part of your ioctl manipulation.
