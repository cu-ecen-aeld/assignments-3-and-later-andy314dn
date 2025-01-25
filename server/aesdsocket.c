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
  syslog(LOG_INFO, "Caught signal, exiting");

  // Close client socket if open
  if (client_socket >= 0) {
    shutdown(client_socket, SHUT_RDWR);  // Disable further send/receive
    close(client_socket);
  }

  // Close server socket if open
  if (server_socket >= 0) {
    shutdown(server_socket, SHUT_RDWR);  // Disable further send/receive
    close(server_socket);
  }

  // Remove the file
  if (file_ptr != NULL) {
    fclose(file_ptr);
    file_ptr = NULL;
  }
  remove(FILE_PATH);

  // Close syslog
  closelog();

  exit(0);
}

void handle_client_connection() {
  char buffer[BUFFER_SIZE];
  char* data = NULL;  // Pointer for dynamically allocated memory
  size_t total_data_size = 0;
  ssize_t bytes_received;

  // Open the file for appending
  file_ptr = fopen(FILE_PATH, "a+");
  if (NULL == file_ptr) {
    syslog(LOG_ERR, "Failed to open file: %s", strerror(errno));
    return;
  }

  // Receive data from the client
  while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) >
         0) {
    buffer[bytes_received] = '\0';  // Null-terminate the received data

    // Allocate/reallocate memory for the data
    char* new_data = realloc(data, total_data_size + bytes_received + 1);
    if (!new_data) {
      syslog(LOG_ERR, "Failed to allocate memory: %s", strerror(errno));
      free(data);
      fclose(file_ptr);
      return;
    }
    data = new_data;

    // Copy the received data into the allocated memory
    memcpy(data + total_data_size, buffer, bytes_received);
    total_data_size += bytes_received;
    data[total_data_size] = '\0';  // Null-terminate the combined data

    // If a newline is detected in the buffer, process the accumulated data
    if (strchr(buffer, '\n') != NULL) {
      // Write the accumulated data to the file
      if (fwrite(data, sizeof(char), total_data_size, file_ptr) !=
          total_data_size) {
        syslog(LOG_ERR, "Failed to write to file: %s", strerror(errno));
        break;
      }
      fflush(file_ptr);  // Ensure data is flushed to disk

      // Send the full file content back to the client
      fseek(file_ptr, 0, SEEK_SET);
      char file_buffer[BUFFER_SIZE];
      size_t bytes_read;
      while ((bytes_read = fread(file_buffer, sizeof(char), BUFFER_SIZE,
                                 file_ptr)) > 0) {
        if (send(client_socket, file_buffer, bytes_read, 0) < 0) {
          syslog(LOG_ERR, "Failed to send data to client: %s", strerror(errno));
          break;
        }
      }
      fseek(file_ptr, 0, SEEK_END);  // Reset file pointer to append mode

      // Free the dynamically allocated memory and reset for the next message
      free(data);
      data = NULL;
      total_data_size = 0;
    }
  }

  if (bytes_received < 0) {
    syslog(LOG_ERR, "Error receiving data from client: %s", strerror(errno));
  }

  // Cleanup
  free(data);
  fclose(file_ptr);
  file_ptr = NULL;
}

int main(int argc, char* argv[]) {
  int server_fd;
  struct addrinfo hints;
  struct addrinfo* res;
  struct addrinfo* p;
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char client_ip[INET6_ADDRSTRLEN];

  // Register signal handlers for SIGINT and SIGTERM
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = cleanup_and_exit;  // Set the handler function
  sigemptyset(&sa.sa_mask);          // No additional signals are blocked
  sa.sa_flags = 0;                   // No special flags

  // Register handlers for SIGINT and SIGTERM
  if (sigaction(SIGINT, &sa, NULL) < 0) {
    syslog(LOG_ERR, "Failed to set signal handler for SIGINT: %s",
           strerror(errno));
    exit(ERROR_CODE);
  }
  if (sigaction(SIGTERM, &sa, NULL) < 0) {
    syslog(LOG_ERR, "Failed to set signal handler for SIGTERM: %s",
           strerror(errno));
    exit(ERROR_CODE);
  }

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

    // Set SO_REUSEADDR option to allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
      syslog(LOG_ERR, "Failed to set SO_REUSEADDR: %s", strerror(errno));
      close(server_fd);
      continue;
    }

    if (0 == bind(server_fd, p->ai_addr, p->ai_addrlen)) {
      break;
    }

    close(server_fd);
  }

  if (NULL == p) {
    syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
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

  while (1) {
    // Accept a connection
    client_socket =
        accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (ERROR_CODE == client_socket) {
      syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
      // close(server_fd);
      return ERROR_CODE;
    }

    // Get the client IP address and log it
    if (inet_ntop(AF_INET, &((struct sockaddr_in*)&client_addr)->sin_addr,
                  client_ip, sizeof(client_ip)) != NULL) {
      syslog(LOG_INFO, "Accepted connection from %s", client_ip);
    } else {
      syslog(LOG_ERR, "Failed to get client IP address");
    }

    handle_client_connection();

    // Log message when closing the connection
    if (client_socket >= 0) {
      close(client_socket);
      syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }
  }

  syslog(LOG_DEBUG, "Server closed");
  cleanup_and_exit(0);

  return 0;
}
