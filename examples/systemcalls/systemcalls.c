#include "systemcalls.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

    // Prevent NULL pointer dereference
    if (NULL == cmd)
        return false;

    int ret = system(cmd);
    // check if system() failed
    if (-1 == ret)
        return false;
    
    // check the command's exit status
    if (WIFEXITED(ret) && 0 == WEXITSTATUS(ret))
        return true;
    return false;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    // at least one argument is required
    if (count < 1) {
        // log an error if no command is provided
        syslog(LOG_ERR, "No command provided");
        return false;
    }

    va_list args;
    va_start(args, count);

    char * command[count+1];
    for (int i=0; i<count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    va_end(args);

    pid_t pid = fork();
    if (pid < 0) {
        // fork failed
        syslog(LOG_ERR, "Fork failed: %m");
        return false;
    }

    if (0 == pid) {
        // child process
        execv(command[0], command); // execute command
        syslog(LOG_ERR, "execv failed for command %s: %m", command[0]);
        _exit(1);
    }

    // parent process
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        syslog(LOG_ERR, "waitpid failed: %m");
        return false;
    }

    // Check if the child process terminated normally
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    syslog(LOG_ERR, "Command %s failed with exit status %d", command[0], WEXITSTATUS(status));
    return false; // Command failed or returned a non-zero exit status
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    // at least one argument is required
    if (count < 1) {
        // log an error if no command is provided
        syslog(LOG_ERR, "No command provided");
        return false;
    }

    va_list args;
    va_start(args, count);

    char * command[count+1];
    for (int i=0; i<count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    va_end(args);

    pid_t pid = fork();
    if (pid < 0) {
        // fork failed
        syslog(LOG_ERR, "Fork failed: %m");
        return false;
    }

    if (0 == pid) {
        // child process
        // open output file for writing
        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            syslog(LOG_ERR, "Failed to open output file %s: %m", outputfile);
            _exit(1);
        }
        
        // redirect stdout and stderr to the output file
        if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
            syslog(LOG_ERR, "Failed to redirect output to %s: %m", outputfile);
            close(fd);
            _exit(1);
        }
        close(fd); // close the original file descriptor

        // execute command
        execv(command[0], command);
        syslog(LOG_ERR, "execv failed for command %s: %m", command[0]);
        _exit(1);
    }

    // parent process
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        syslog(LOG_ERR, "waitpid failed: %m");
        return false;
    }

    // check if the child process terminated normally
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    syslog(LOG_ERR, "Command %s failed with exit status %d", command[0], WEXITSTATUS(status));
    return false;
}
