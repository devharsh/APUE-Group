#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define TRUE 1

struct sockaddr_in create_server_properties(struct sockaddr_in server, char *address, int port);
int  open_connection(char *address, int port);