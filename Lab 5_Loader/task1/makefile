# Makefile for static loader

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -m32 -Wall -Werror

# Target executable
TARGET = loader

# Source files
SRC = task1.c

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean up
clean:
	rm -f $(TARGET)
