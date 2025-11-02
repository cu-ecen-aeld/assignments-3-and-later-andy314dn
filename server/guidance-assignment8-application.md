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
