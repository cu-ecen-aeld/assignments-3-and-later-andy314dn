#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define FILE_PATH "/var/tmp/aesdsocketdata"
#define PORT 9000
#define BUFFER_SIZE 1024

// Global variables
int server_socket = -1;
int client_socket = -1;
FILE* file_ptr = NULL;

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
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind the socket to the port
  if (-1 == bind(server_socket, (struct sockaddr*)&server_addr,
                 sizeof(server_addr))) {
    syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
    close(server_socket);
    return -1;
  }

  // Start listening for connections
  if (-1 == listen(server_socket, 10)) {
    syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
    close(server_socket);
    return -1;
  }

  // Infinite loop to accept and handle client connections
  while (1) {
    // Accept a new client connection
    client_socket =
        accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (-1 == client_socket) {
      syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
      continue;
    }

    // Log accepted connection
    syslog(LOG_INFO, "Accepted connection from %s",
           inet_ntoa(client_addr.sin_addr));

    // Open the file in append mode
    file_ptr = fopen(FILE_PATH, "a+");
    if (!file_ptr) {
      syslog(LOG_ERR, "Failed to open file: %s", strerror(errno));
      close(client_socket);
      continue;
    }

    // Receive data from client

    // Log error when receiving data
    if (-1 == bytes_received) {
      syslog(LOG_ERR, "Error receiving data: %s", strerror(errno));
    }

    // Close the file
    fclose(file_ptr);
    file_ptr = NULL;

    // Log closed connection
    syslog(LOG_INFO, "Closed connection from %s",
           inet_ntoa(client_addr.sin_addr));

    // Close client connection
    close(client_socket);
    client_socket = -1;
  }

  return 0;
}
