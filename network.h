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
#include <signal.h>
#include <stdbool.h>

#define TRUE 1
#define TIMEOUT 60

struct request {
    char *method;
    char *protocol;
    char *uri;
};

int msgsock;

struct sockaddr_in create_server_properties(char *address, int port);
int  open_connection(char *address, int port);
bool is_request_complete(char *line, int *repeat_return);