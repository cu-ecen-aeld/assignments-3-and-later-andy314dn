#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FILE_PATH "/var/tmp/aesdsocketdata"

// Global variables
int server_socket = -1;
int client_socket = -1;

void cleanup_and_exit(int signo) {
  // Log signal received
  syslog(LOG_INFO, "Caught signal, exiting...");

  // Close client socket if open
  if (client_socket >= 0) {
    close(client_socket);
  }

  // Close server socket if open
  if (server_socket >= 0) {
    close(server_socket);
  }

  // Remove the file
  remove(FILE_PATH);

  // Close syslog
  closelog();

  exit(0);
}

int main(int argc, char* argv[]) {
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  ssize_t bytes_received;

  // Register signal handlers for SIGINT and SIGTERM
  signal(SIGINT, cleanup_and_exit);
  signal(SIGTERM, cleanup_and_exit);

  // Open syslog for logging
  openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

  // Create a socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == server_socket) {
    syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
    return -1;
  }

  // Configure server address

  // Bind the socket to the port

  // Start listening for connections

  // Infinite loop to accept and handle client connections
  while (1) {
    // Accept a new client connection

    // Log accepted connection

    // Open the file in append mode

    // Receive data from client

    // Close the file

    // Log closed connection

    // Close client connection
    
  }

  return 0;
}
