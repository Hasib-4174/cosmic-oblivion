# Cosmic Oblivion Makefile

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -I.
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

BUILD_DIR = build
BIN_DIR = out

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRC))
DEP = $(OBJ:.o=.d)

TARGET = $(BIN_DIR)/cosmic

.PHONY: all clean debug release lint test run

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

debug: CFLAGS += -g -O0
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

lint:
	cppcheck --enable=all --std=c99 --platform=unix64 \
		--inline-suppr --suppress=missingIncludeSystem \
		-I. $(SRC) 2>&1 || true

analyze:
	gcc -fanalyzer $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET) 2>&1 || true

run: $(TARGET)
	./$(TARGET)

-include $(DEP)
