CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
SRC_DIR = src
BUILD_DIR = build

all: $(BUILD_DIR)/emulator $(BUILD_DIR)/test

$(BUILD_DIR)/emulator: $(BUILD_DIR)/main.o $(BUILD_DIR)/emulator.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/emulator $(BUILD_DIR)/main.o $(BUILD_DIR)/emulator.o

$(BUILD_DIR)/test: $(BUILD_DIR)/test.o $(BUILD_DIR)/emulator.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test $(BUILD_DIR)/test.o $(BUILD_DIR)/emulator.o 

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/emulator.h $(SRC_DIR)/state.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c main.c -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/emulator.o: $(SRC_DIR)/emulator.c $(SRC_DIR)/emulator.h $(SRC_DIR)/state.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/emulator.c -o $(BUILD_DIR)/emulator.o

$(BUILD_DIR)/test.o: $(SRC_DIR)/test.c $(SRC_DIR)/emulator.h $(SRC_DIR)/state.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/test.c -o $(BUILD_DIR)/test.o

run: $(BUILD_DIR)/emulator
	./$(BUILD_DIR)/emulator

test: $(BUILD_DIR)/test
	./$(BUILD_DIR)/test

clean:
	rm -rf $(BUILD_DIR)