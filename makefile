# Makefile for bootloader

CC = clang
CFLAGS = -Wall -O2
TARGET = bootloader
SRC = bootloader.c

.PHONY: all clean

all: $(TARGET)
	@echo "Appending payload..."
	@bash payload.sh
	@echo "Build complete!"

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

