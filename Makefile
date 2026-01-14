# Compiler
CC = gcc

# Compiler flags
# -O2: Optimization level 2
# -Wall: Enable all warnings
# -fopenmp: Enable OpenMP library support
CFLAGS = -O2 -Wall -fopenmp

# Target executables
TARGET1 = q1
TARGET2 = q2
TARGET3 = q3

# Source files
SRC1 = src/q1.c
SRC2 = src/q2.c
SRC3 = src/q3.c

# Default rule: Compile all targets
all: $(TARGET1) $(TARGET2) $(TARGET3)

# Rules for building each executable
$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(SRC1)

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRC2)

$(TARGET3): $(SRC3)
	$(CC) $(CFLAGS) -o $(TARGET3) $(SRC3)

# Clean rule: Remove executables
clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET3)