#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define FILE_PATH "/var/tmp/aesdsocketdata"
#define PORT "9000"
#define BACKLOG 10
#define BUFFER_SIZE 1024

#define ERROR_CODE -1

// Global variables
int server_socket = -1;
int client_socket = -1;
FILE* file_ptr = NULL;

void cleanup_and_exit(int signo) {
  // Log signal received
  const char* signal_name =
      (signo == SIGINT) ? "SIGINT" : (signo == SIGTERM) ? "SIGTERM" : "UNKNOWN";
  syslog(LOG_INFO, "Caught signal %s, exiting...", signal_name);

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
  int server_fd;
  // int client_fd;
  struct addrinfo hints;
  struct addrinfo* res;
  struct addrinfo* p;
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char client_ip[INET6_ADDRSTRLEN];

  // Register signal handlers for SIGINT and SIGTERM
  signal(SIGINT, cleanup_and_exit);
  signal(SIGTERM, cleanup_and_exit);

  // Open syslog for logging
  openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

  // Set up hints for getaddrinfo
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;        // use IPv4
  hints.ai_socktype = SOCK_STREAM;  // use TCP
  hints.ai_flags = AI_PASSIVE;

  // Get address info
  if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
    syslog(LOG_ERR, "getaddrinfo failed");
    return ERROR_CODE;
  }

  // Create a socket and bind it
  for (p = res; p != NULL; p = p->ai_next) {
    server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (ERROR_CODE == server_fd) {
      continue;
    }

    if (0 == bind(server_fd, p->ai_addr, p->ai_addrlen)) {
      break;
    }

    close(server_fd);
  }

  if (NULL == p) {
    syslog(LOG_ERR, "Failed to bind socket");
    freeaddrinfo(res);
    return ERROR_CODE;
  }

  freeaddrinfo(res);

  // Start listening for connections
  if (ERROR_CODE == listen(server_fd, BACKLOG)) {
    syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
    close(server_fd);
    return ERROR_CODE;
  }
  syslog(LOG_DEBUG, "Server is listening on port %s", PORT);

  // Accept a connection
  client_socket =
      accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
  if (ERROR_CODE == client_socket) {
    syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
    close(server_fd);
    return ERROR_CODE;
  }

  // Get the client IP address and log it
  if (inet_ntop(AF_INET, &((struct sockaddr_in*)&client_addr)->sin_addr,
                client_ip, sizeof(client_ip)) != NULL) {
    syslog(LOG_INFO, "Accepted connection from %s", client_ip);
  } else {
    syslog(LOG_ERR, "Failed to get client IP address");
  }

  // Close the server socket
  close(server_fd);
  syslog(LOG_DEBUG, "Server closed");

  // Close the syslog
  closelog();

  return 0;
}
