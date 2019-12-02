TARGET = sws 
CC = cc
CFLAGS  = -ansi -g -Wall -Werror -Wextra -Wformat=2 -Wno-format-y2k -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow -lmagic
RM = rm -f

default: $(TARGET)
all: default

$(TARGET): $(TARGET).o helper.o network.o bbcp.o cgi.o ls.o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o helper.o network.o bbcp.o cgi.o ls.o

$(TARGET).o: $(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -c $(TARGET).c

helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c

network.o: network.c network.h
	$(CC) $(CFLAGS) -c network.c

ls.o: ls.c network.h
	$(CC) $(CFLAGS) -c ls.c

bbcp.o: bbcp.c bbcp.h
	$(CC) $(CFLAGS) -c bbcp.c

cgi.o: cgi.c network.h
	$(CC) $(CFLAGS) -c cgi.c

debug:
	$(CC) -o $(TARGET) $(TARGET).c helper.c network.c bbcp.c cgi.c ls.c -lmagic

clean:
	$(RM) $(TARGET) *.o 
