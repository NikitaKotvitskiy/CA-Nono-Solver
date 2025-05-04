# Compiler and flags
CC = gcc
CFLAGS = -O2 -std=c17

# Output binaries
BIN1 = method1
BIN2 = method2

# Directories
DIR1 = v1
DIR2 = v2

# Source files
SRC1 = $(wildcard $(DIR1)/*.c)
SRC2 = $(wildcard $(DIR2)/*.c)

# Object files
OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)

# Default target
all: $(BIN1) $(BIN2)

$(BIN1): $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN2): $(OBJ2)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Pattern rule to compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(DIR1)/*.o $(DIR2)/*.o $(BIN1) $(BIN2)

.PHONY: all clean
