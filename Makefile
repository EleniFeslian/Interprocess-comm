CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = program

SRC = main.c fun.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: clean