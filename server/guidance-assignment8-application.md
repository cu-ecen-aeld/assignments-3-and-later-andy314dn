# Assignment 8 Instruction for Application part

The following contains guidances to complete Assignment 8 in Assignment-3 repository for Application part
(to distinguish it with Driver part).

> **Step 5**: Modify your socket server application developed in assignments 5 and 6 to support
and use a build switch `USE_AESD_CHAR_DEVICE`, set to 1 by default, which:
>
> a. Redirects reads and writes to `/dev/aesdchar` instead of `/var/tmp/aesdsocketdata`  
> b. Removes timestamp printing.  
> c. Ensure you do not remove the `/dev/aesdchar` endpoint after exiting the `aesdsocket` application.  
>
> In addition, please ensure you are not opening the file descriptor for your driver endpoint from
the `aesdsocket` until it is accessed.
>
> **Step 6**: Test with [sockettest.sh](https://github.com/cu-ecen-aeld/assignment-autotest/blob/master/test/assignment5/sockettest.sh)
script used with assignment 5 (which doesn’t validate timestamps).

## Spec from Driver's Write and Read functionlities

Write Return Value Requirements:

* **Positive Return Value**: If the write operation is successful and the number of bytes written equals the count specified, return the count.
* **Partial Write**: If the write operation is successful but fewer bytes than requested are written, return the number of bytes actually written (a positive value less than count).
* **Zero Return Value**: If no data is written (e.g., due to an error), return zero.
* **Negative Return Value**: If an error occurs (e.g., memory allocation failure), return a negative error code (e.g., `ENOMEM` for out of memory, `EFAULT` for a bad address).

Read Return Value Requirements:

* **Positive Return Value**: If the read operation is successful and the number of bytes read equals the count specified, return the count.
* **Partial Read**: If the read operation is successful but fewer bytes than requested are read, return the number of bytes actually read (a positive value less than count).
* **Zero Return Value**: If there is no data remaining to read (end of file), return zero.
* **Negative Return Value**: If an error occurs during the read operation, return a negative error code (e.g., `EINTR` for interrupted system calls, `EFAULT` for a bad address).
