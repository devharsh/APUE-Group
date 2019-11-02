TARGET = sws 
CC = cc
CFLAGS  = -ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow
RM = rm -f

default: $(TARGET)
all: default

$(TARGET): $(TARGET).o helper.o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o helper.o

$(TARGET).o: $(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -c $(TARGET).c

helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c

clean:
	$(RM) $(TARGET) *.o 
