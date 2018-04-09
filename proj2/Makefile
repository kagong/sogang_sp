CC = gcc
OBJS = 20151575.o functions.o hash.o assemble.o
TARGET = 20151575.out

.SUFFIXES: .c .o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -Wall -o $@ $(OBJS)

clean :
	rm -f $(OBJS) $(TARGET)
