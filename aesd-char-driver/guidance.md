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

Guidance for step 3:

### What You Need to Do in Step 3

1. **Implement the `llseek` Function**: This function is part of the file operations in your driver. It allows you to change the position of the file pointer based on the seek command you receive. You will need to define how the file pointer moves when a user requests to seek to a specific position.

2. **Support Different Seek Types**: Your implementation should handle three types of seeking:
   - **SEEK_SET**: This sets the file pointer to a specific position from the beginning of the file.
   - **SEEK_CUR**: This moves the file pointer relative to its current position.
   - **SEEK_END**: This sets the file pointer to a position relative to the end of the file.

3. **Update the File Position**: As you implement the `llseek` function, you need to ensure that the file position is updated correctly based on the type of seek requested. For example:
   - If a user wants to seek to position 0-5, they are looking at the first write ("Grass").
   - If they seek to position 6-13, they are looking at the second write ("Sentosa").
   - If they seek to position 14-23, they are looking at the third write ("Singapore").

### Example Logic

Here’s a simple way to think about how you might implement this:

```c
long my_llseek(struct file *file, long offset, int whence) {
    long new_position;

    switch (whence) {
        case SEEK_SET:
            new_position = offset; // Set to the absolute position
            break;
        case SEEK_CUR:
            new_position = file->f_pos + offset; // Move relative to current position
            break;
        case SEEK_END:
            new_position = total_length + offset; // Move relative to the end
            break;
        default:
            return -EINVAL; // Invalid argument
    }

    // Check if new_position is valid (e.g., within bounds)
    if (new_position < 0 || new_position > total_length) {
        return -EINVAL; // Out of bounds
    }

    file->f_pos = new_position; // Update the file position
    return new_position; // Return the new position
}
```

### Summary

- **Implement the `llseek` function** to handle seeking.
- **Support SEEK_SET, SEEK_CUR, and SEEK_END** to allow users to navigate through the data.
- **Update the file position** based on the seek request, ensuring it stays within valid bounds.

This implementation will allow your driver to effectively manage file positions, making it more versatile and user-friendly.

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

Guidance for Step 4:

Implementing the `ioctl` command support for `AESDCHAR_IOCSEEKTO` is a crucial step in enhancing your driver’s functionality. Let’s break down what you need to do in this step in a clear and straightforward manner.

### What You Need to Do in Step 4

1. **Define the `ioctl` Command**: You will be implementing the `AESDCHAR_IOCSEEKTO` command, which allows users to seek to a specific position in the command circular buffer. This command will be defined in the `aesd_ioctl.h` header file.

2. **Use `unlocked_ioctl`**: This function allows you to handle the `ioctl` command without locking the driver, which is important for performance and concurrency. You will need to implement your command handling logic within this function.

3. **Extract Values from the Buffer**: The `ioctl` command will receive a buffer containing two 4-byte values:
   - The **first value** indicates which command to seek into (zero-indexed).
   - The **second value** indicates the offset within that command.

4. **Translate to the Circular Buffer**: You will need to translate the first value into the appropriate command stored in your circular buffer. For example, if the first value is `0`, it refers to the command "Grass\n". 

5. **Calculate the Seek Location**: Using the second value (the offset), you will determine the exact byte position within the specified command. For instance, if the command is "Grass" and the offset is `2`, the seek location should point to the letter "a".

6. **Update the File Pointer**: After calculating the new position, you will update the file pointer in your driver to reflect this new location.

7. **Error Handling**: If either of the parameters (the command index or the offset) is out of range, you should return `-EINVAL` to indicate an invalid argument.

### Example Logic of Step 4

Here’s a simplified example of how you might implement this in your `ioctl` function:

```c
long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == AESDCHAR_IOCSEEKTO) {
        // Buffer to hold the two 4-byte values
        uint32_t seek_params[2];
        
        // Copy data from user space to kernel space
        if (copy_from_user(&seek_params, (void __user *)arg, sizeof(seek_params))) {
            return -EFAULT; // Handle copy error
        }

        // Extract the command index and offset
        int command_index = seek_params[0];
        int offset = seek_params[1];

        // Validate the command index
        if (command_index < 0 || command_index >= total_commands) {
            return -EINVAL; // Invalid command index
        }

        // Get the command string from the circular buffer
        char *command = get_command_from_buffer(command_index);
        int command_length = strlen(command);

        // Validate the offset
        if (offset < 0 || offset >= command_length) {
            return -EINVAL; // Invalid offset
        }

        // Calculate the new file position
        file->f_pos = calculate_new_position(command_index, offset);
        return 0; // Success
    }
    return -ENOTTY; // Command not recognized
}
```

### Summary of Step 4

- **Implement the `ioctl` command** using `unlocked_ioctl`.
- **Extract the two 4-byte values** from the user buffer.
- **Translate the command index** to the corresponding command in your circular buffer.
- **Calculate the new seek position** based on the offset.
- **Update the file pointer** accordingly and handle any errors by returning `-EINVAL` for invalid parameters.

This implementation will allow your driver to effectively handle seek requests from user space, enhancing its functionality.

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

Guidance for Step 5:

In Step 5, you will be enhancing your `aesdsocket` server application to handle special socket write commands that allow users to seek within the data stored in your driver.

### What You Need to Do in Step 5

1. **Identify the Command Format**: Your application needs to recognize when a string sent over the socket matches the format `AESDCHAR_IOCSEEKTO:X,Y`, where `X` and `Y` are unsigned decimal integers. Here, `X` represents the command index in your circular buffer, and `Y` represents the offset within that command.

2. **Extract Values from the Command**: When you receive a string that matches this format, you will need to parse the string to extract the values of `X` and `Y`. This can be done using string manipulation functions to split the string at the colon and comma.

3. **Send Values to the Driver**: Once you have the values of `X` and `Y`, you will use the `ioctl` command you implemented earlier (`AESDCHAR_IOCSEEKTO`) to send these values to your driver. This will allow the driver to update the file pointer accordingly.

4. **Avoid Writing the Command to the Device**: Unlike other strings sent to the socket, you should **not** write the `AESDCHAR_IOCSEEKTO:X,Y` command string into the `aesdchar` device. This command is meant solely for seeking, not for storage.

5. **Read and Send Content Back**: After performing the `ioctl` command, you will read the content from the `aesdchar` device. This content should be sent back over the socket to the client, just like you would with any other string received through the socket interface.

6. **Maintain the File Descriptor**: Ensure that the read operation uses the same file descriptor that was used to send the `ioctl` command. This is crucial because it ensures that the file offset is honored, meaning that the read will start from the correct position based on the seek operation.

### Example Logic of Step 5

Here’s a simplified example of how you might implement this in your `aesdsocket` server application:

```c
void handle_socket_command(int socket_fd, const char *command) {
    if (strncmp(command, "AESDCHAR_IOCSEEKTO:", 19) == 0) {
        // Extract X and Y from the command
        int x, y;
        sscanf(command + 19, "%d,%d", &x, &y);

        // Send the values to the driver using ioctl
        uint32_t seek_params[2] = {x, y};
        if (ioctl(driver_fd, AESDCHAR_IOCSEEKTO, seek_params) < 0) {
            perror("ioctl failed");
            return;
        }

        // Read the content from the aesdchar device
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(driver_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            // Send the content back over the socket
            send(socket_fd, buffer, bytes_read, 0);
        }
    } else {
        // Handle other string commands as usual
        write_to_aesdchar_device(command);
    }
}
```

### Summary of Step 5

- **Recognize the command format** `AESDCHAR_IOCSEEKTO:X,Y` and extract `X` and `Y`.
- **Send these values to your driver** using the `ioctl` command.
- **Do not write the command string** into the `aesdchar` device.
- **Read the content from the device** and send it back over the socket.
- **Use the same file descriptor** for reading to ensure the correct file offset is honored.

By following these steps, you will successfully implement the special handling for socket write commands, allowing users to seek within the data stored in your driver.
