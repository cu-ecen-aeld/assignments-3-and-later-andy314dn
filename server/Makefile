# Compiler settings
# NOTE: comment CC out when running in x86
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror -pthread

# Targets and files
TARGET := aesdsocket
SRC := aesdsocket.c
OBJ := $(SRC:.c=.o)

# Default target: build the "aesdsocket" application
all: $(TARGET)

# Build the "aesdsocket" executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile the source file into an object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove the "aesdsocket" executable and object files
clean:
	rm -f $(TARGET) $(OBJ)
