CC = gcc
CFLAGS = -Wall -Wextra -pedantic
SRC_DIR = src

TARGET = witsshell

all: $(TARGET)

$(TARGET): witsshell.c
	$(CC) witsshell.c $(CFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
