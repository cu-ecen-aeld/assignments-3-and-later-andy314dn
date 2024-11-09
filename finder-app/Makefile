# Compiler settings
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror

# Targets and files
TARGET := writer
SRC := writer.c
OBJ := $(SRC:.c=.o)

# Default target: build the "writer" application
all: $(TARGET)

# Build the "writer" executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile the source file into an object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove the "writer" executable and object files
clean:
	rm -f $(TARGET) $(OBJ)
