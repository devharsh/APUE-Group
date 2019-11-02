TARGET = ls
CC = cc
CFLAGS  = -ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow
RM = rm -f

default: $(TARGET)
all: default

$(TARGET): ls.o helper.o cmp.o 
	$(CC) $(CFLAGS) -o $(TARGET) ls.o helper.o cmp.o /usr/lib/libmagic.so 

ls.o: ls.c ls.h
	$(CC) $(CFLAGS) -c ls.c

helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c

cmp.o: cmp.c cmp.h
	$(CC) $(CFLAGS) -c cmp.c

clean:
	$(RM) $(TARGET) *.o 
