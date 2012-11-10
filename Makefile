CC = gcc
CFLAGS = -g -Wall -pthread
TARGET = webserver
OBJS = main.o network.o linkedlist.o
DEPS = network.h linkedlist.h
.PHONY : clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) *~

