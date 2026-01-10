# Compiler
CC = gcc

# Compiler flags
CFLAGS = -O2 -Wall -fopenmp

# Target executable
TARGET = mergesort

# Source file
SRC = mergesort.c

# Default rule
all: $(TARGET)

# Build executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean rule
clean:
	rm -f $(TARGET)
