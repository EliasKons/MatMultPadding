CC = gcc
CFLAGS = -g -Wall -Wextra -pthread

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
TARGET = main
ARGS = 8 8000000 4 4 output.txt

.PHONY: all clean run valgrind

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET) $(ARGS)