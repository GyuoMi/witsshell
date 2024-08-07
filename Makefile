CC = gcc
CFLAGS = -Wall -Wextra -pedantic
SRC_DIR = src

TARGET = witsshell

all: $(TARGET)

$(TARGET): $(SRC_DIR)/witsshell.c
	$(CC) $(SRC_DIR)/witsshell.c $(CFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
