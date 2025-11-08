# Assignment Guidance

## Assignment 8 Instruction

The following are guidances to complete Assignment 8 in this Assignment-3 repository.

> Step 4: Add source code, makefile and init scripts needed to install a `/dev/aesdchar` device.  
Ultimately this device should be installed in your `QEMU` instance, however you may also wish to test on your host virtual machine during development.  
Your `/dev/aesdchar` device should use the circular buffer implementation you developed in assignment 7 plus new code you add to your driver to:  
>
> a. Allocate memory for each write command as it is received, supporting any length of write request
(up to the length of memory which can be allocated through `kmalloc`), and saving the write command content within allocated memory.  
>> i. The write command will be terminated with a `\n` character as was done with the aesdsocket application.  
>>> Write operations which do not include a \n character should be saved and appended by future write operations and should not be written to the circular buffer until terminated.  
>>
>> ii. The content for the **most recent 10 write commands** should be saved.  
>> iii. Memory associated with write commands more than 10 writes ago should be freed.  
>> iv. Write position and write file offsets can be ignored on this assignment, each write will just write to the most recent entry in the history buffer or append to any unterminated command.  
>> v. For the purpose of this assignment you can use `kmalloc` for all allocations regardless of size, and assume writes will be small enough to work with kmalloc.
>
> b. Return the content (or partial content) related to the **most recent 10 write commands**, in the order they were received, on any read attempt.  
>> 1(i). You should use the position specified in the read to determine the location and number of bytes to return.  
>> 2(ii). You should honor the count argument by sending only up to the first “count” bytes back of the available bytes remaining.
>
> c. Perform appropriate locking to ensure safe multi-thread and multi-process access and ensure a full write file operation from a thread completes before accepting a new write file operation.  
>
> d. Your implementation should print the expected contents when running the [drivertest.sh](https://github.com/cu-ecen-aeld/assignment-autotest/blob/master/test/assignment8/drivertest.sh) script.

## Assignment 9 Instruction

> **Step 3**: Update your `aesd-char-driver` implementation in your `assignment 3 and later` repository to add the following features:
>
> a. Add custom seek support to your driver using the `llseek` `file_operations` function.  Your seek implementation should:
>
>> i. Support all positional types (`SEEK_SET`, `SEEK_CUR`, and `SEEK_END`) and update the file position accordingly.
>>
>>> 1. For instance, if your driver is storing writes of 5, 7, and 9 bytes each,
>>> a file position of 0-5 would represent a byte within the first write,
>>> a file position of 6-13 would represent a byte within the second write,
>>> and a file position  of 14 through 23 would a byte within represent the 3rd write.
>
> For instance: Consider the content in the table below as handled by your driver, where each string ends with a newline character.
> An offset of 15 would set the pointer to the second byte of the word “Singapore”, byte "i"

```txt
    Grass\n
    Sentosa\n
    Singapore\n
```

![Assignment 9 table](./assignment-9-table.png "Table of Assignment 9")

> **Step 4**: Add `ioctl` command support for `AESDCHAR_IOCSEEKTO` as defined in
> [aesd_ioctl.h](https://github.com/cu-ecen-aeld/aesd-assignments/blob/assignment9/aesd-char-driver/aesd_ioctl.h)
> ( [relative link](./aesd_ioctl.h) ) which is passed a buffer from user space containing two 4 byte values.
> Use `unlocked_ioctl` to implement your ioctl command.
>
> a. The first value represents the command to seek into,
> based on a zero referenced number of commands currently stored by the driver in the command circular buffer.
> The command reference is relative to the number of commands currently stored in your circular buffer.
> You will need to translate into the appropriate offset within your circular buffer based on the current location of the output index.
>
>> i. For instance, in the example above, a value of 0 refers to the command “Grass\n”.
>
> b. The second value represents the zero referenced offset within this command to seek into.
> All offsets are specified relative to the start of the request.
>
>> i. For instance, if the offset was 2 in the command “Grass”, the seek location should be the letter “a”.
>
> c. The two parameters above will seek to the appropriate place and update the file pointer
> as described in the seek support details within the previous step.
>
> d. If the two parameters are out of range of the number of write commands/command length, the `ioctl` cmd should return `-EINVAL`.

Here is the last step:

> **Step 5**: Add special handling for socket write commands to your `aesdsocket` server application.
>
> a. When the string sent over the socket equals `AESDCHAR_IOCSEEKTO:X,Y` where `X` and `Y` are unsigned decimal integer values,
> the `X` should be considered the write command to seek into
> and the `Y` should be considered the offset within the write command.
>
> b. These values should be sent to your driver using the `AESDCHAR_IOCSEEKTO` `ioctl` described earlier.
> The `ioctl` command should be performed before any additional writes to your `aesdchar` device.
>
> c. Do not write this string command into the `aesdchar` device as you do with other strings sent to the socket.
>
> d. Send the content of the `aesdchar` device back over the socket, as you do with any other string received over the socket interface.
> We will use this to test the seek command.
>
> e. Ensure the read of the file and return over the socket uses the same (not closed and re-opened)
> file descriptor used to send the ioctl, to ensure your file offset is honored for the read command.
