TARGET = sws 
CC = cc
CFLAGS  = -ansi -g -Wall -Werror -Wextra -Wformat=2 -Wno-format-y2k -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow
RM = rm -f

default: $(TARGET)
all: default

$(TARGET): $(TARGET).o helper.o network.o bbcp.o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o helper.o network.o bbcp.o

$(TARGET).o: $(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -c $(TARGET).c

helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c

network.o: network.c network.h
	$(CC) $(CFLAGS) -c network.c

bbcp.o: bbcp.c bbcp.h
	$(CC) $(CFLAGS) -c bbcp.c

clean:
	$(RM) $(TARGET) *.o 
