# Cosmic Oblivion Makefile

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -I.
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = cosmic

.PHONY: all clean debug release lint test run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g -O0
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

lint:
	cppcheck --enable=all --std=c99 --platform=unix64 \
		--inline-suppr --suppress=missingIncludeSystem \
		-I. $(SRC) 2>&1 || true

analyze:
	gcc -fanalyzer $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET) 2>&1 || true

run: $(TARGET)
	./$(TARGET)
