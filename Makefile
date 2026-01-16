# 1. Ορισμός φακέλων
BIN_DIR = bin
SRC_DIR = src

# 2. Compiler και Flags
CC = gcc
CFLAGS = -O2 -Wall -fopenmp

# 3. Ονόματα στόχων
TARGETS = q1 q2 q3

# 4. Πλήρη μονοπάτια των εκτελέσιμων (bin/q1, bin/q2, bin/q3)
EXECS = $(patsubst %,$(BIN_DIR)/%,$(TARGETS))

# Default rule
all: $(BIN_DIR) $(EXECS)

# Δημιουργία του φακέλου bin αν δεν υπάρχει
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Rules για το κάθε εκτελέσιμο
# Χρησιμοποιούμε το $@ για το όνομα του target (π.χ. bin/q1)
# Και το $< για το όνομα του source file (π.χ. src/q1.c)

$(BIN_DIR)/q1: $(SRC_DIR)/q1.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/q2: $(SRC_DIR)/q2.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/q3: $(SRC_DIR)/q3.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean rule: Διαγράφει όλο τον φάκελο bin
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean