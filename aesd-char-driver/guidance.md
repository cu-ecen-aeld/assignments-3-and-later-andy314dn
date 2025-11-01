# Assignment 8 Instruction

The following are guidances to complete Assignment 8 in this Assignment-3 repository.

> Step 4: Add source code, makefile and init scripts needed to install a `/dev/aesdchar` device.  
Ultimately this device should be installed in your qemu instance, however you may also wish to test on your host virtual machine during development.  
Your `/dev/aesdchar` device should use the circular buffer implementation you developed in assignment 7 plus new code you add to your driver to:  
> a. Allocate memory for each write command as it is received, supporting any length of write request
(up to the length of memory which can be allocated through `kmalloc`), and saving the write command content within allocated memory.  
> b. Return the content (or partial content) related to the **most recent 10 write commands**, in the order they were received, on any read attempt.  
> c. Perform appropriate locking to ensure safe multi-thread and multi-process access and ensure a full write file operation from a thread completes before accepting a new write file operation.  
> d. Your implementation should print the expected contents when running the [drivertest.sh](https://github.com/cu-ecen-aeld/assignment-autotest/blob/master/test/assignment8/drivertest.sh) script.
