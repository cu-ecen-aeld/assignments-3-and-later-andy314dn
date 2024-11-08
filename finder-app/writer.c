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

int main() {
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_ERR, "For error: \n\r");
    syslog(LOG_DEBUG, "Debug message\r\n");
}
