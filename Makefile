CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
BUILD_DIR = build

all: $(BUILD_DIR)/emulator $(BUILD_DIR)/test

$(BUILD_DIR)/emulator: $(BUILD_DIR)/main.o $(BUILD_DIR)/emulator.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/emulator $(BUILD_DIR)/main.o $(BUILD_DIR)/emulator.o

$(BUILD_DIR)/test: $(BUILD_DIR)/test.o $(BUILD_DIR)/emulator.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test $(BUILD_DIR)/test.o $(BUILD_DIR)/emulator.o 

$(BUILD_DIR)/main.o: main.c emulator.h state.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c main.c -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/emulator.o: emulator.c emulator.h state.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c emulator.c -o $(BUILD_DIR)/emulator.o

$(BUILD_DIR)/test.o: test.c emulator.h state.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c test.c -o $(BUILD_DIR)/test.o

run: $(BUILD_DIR)/emulator
	./$(BUILD_DIR)/emulator

test: $(BUILD_DIR)/test
	./$(BUILD_DIR)/test

clean:
	rm -rf $(BUILD_DIR)