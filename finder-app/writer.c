#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>

/**
 * One difference from the write.sh instructions in Assignment 1:  
 *   You do not need to make your "writer" utility create directories which do not exist.  
 *   You can assume the directory is created by the caller.
 * Setup syslog logging for your utility using the LOG_USER facility.
 * Use the syslog capability to write a message “Writing <string> to <file>” 
 *   where <string> is the text string written to file (second argument) and 
 *   <file> is the file created by the script.  
 *   This should be written with LOG_DEBUG level.
 * Use the syslog capability to log any unexpected errors with LOG_ERR level.
 */

int main(int argc, char *argv[]) {
    // open connection to syslog
    openlog("AELD-Assignment1", LOG_PID | LOG_CONS, LOG_USER);

    // check the number of arguments
    if (argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments. Usage: %s <writefle> <writestr>", argv[0]);
        fprintf(stderr, "Error: Invalid number of arguments.\n");
        closelog();
        return 1;
    }

    char *writefile = argv[1];
    char *writestr = argv[2];

    // open file for writing
    // (create if it does not exist, truncate if it does)
    int fd = open(writefile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (-1 == fd) {
        syslog(LOG_ERR, "Failed to open file %s: %s", writefile, strerror(errno));
        fprintf(stderr, "Error: Could not open or create file!\n");
        closelog();
        return 1;
    }

    // write the string to file
    ssize_t num_bytes_written = write(fd, writestr, strlen(writestr));
    if (-1 == num_bytes_written) {
        syslog(LOG_ERR, "Failed to write to file %s: %s", writefile, strerror(errno));
        fprintf(stderr, "Error: Could not write to file!\n");
        close(fd);
        closelog();
        return 1;
    }

    // log a debug message indicating what was written
    syslog(LOG_DEBUG, "Writing %s to %s.", writestr, writefile);

    // close opened files
    close(fd);
    closelog();
}
