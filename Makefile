CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -std=c11 -Iinclude -pthread
# Note: -pthread is used just for atomic operations in some environments, not for pthreads. 
# We explicitly avoid pthread features.

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(SRC_DIR)/sync/*.c) \
       $(wildcard $(SRC_DIR)/schedulers/*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Output library
LIB = $(LIB_DIR)/libuthread.a

# Test sources
UNIT_TEST_SRCS = $(wildcard tests/unit/*.c)
STRESS_TEST_SRCS = $(wildcard tests/stress/*.c)
BENCHMARK_SRCS = $(wildcard benchmarks/*.c)
EXAMPLE_SRCS = $(wildcard examples/*.c)

# Test binaries
UNIT_BINS = $(patsubst tests/unit/%.c, $(BIN_DIR)/%, $(UNIT_TEST_SRCS))
STRESS_BINS = $(patsubst tests/stress/%.c, $(BIN_DIR)/%, $(STRESS_TEST_SRCS))
BENCHMARK_BINS = $(patsubst benchmarks/%.c, $(BIN_DIR)/%, $(BENCHMARK_SRCS))
EXAMPLE_BINS = $(patsubst examples/%.c, $(BIN_DIR)/%, $(EXAMPLE_SRCS))

ALL_BINS = $(UNIT_BINS) $(STRESS_BINS) $(BENCHMARK_BINS) $(EXAMPLE_BINS)

.PHONY: all clean directories tests

all: directories $(LIB) tests

directories:
	@mkdir -p $(OBJ_DIR)/sync $(OBJ_DIR)/schedulers $(LIB_DIR) $(BIN_DIR)

# Build library
$(LIB): $(OBJS)
	ar rcs $@ $^

# Build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build all binaries
tests: $(ALL_BINS)

# Build tests and others
$(BIN_DIR)/%: tests/unit/%.c $(LIB)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -luthread

$(BIN_DIR)/%: tests/stress/%.c $(LIB)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -luthread

$(BIN_DIR)/%: benchmarks/%.c $(LIB)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -luthread

$(BIN_DIR)/%: examples/%.c $(LIB)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -luthread

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
