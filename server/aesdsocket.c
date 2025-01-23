#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

  return 0;
}
