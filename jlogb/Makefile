
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread

TARGET = vm  #executavel
SRCS = main.c memory.c pagetable.c tlb.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
