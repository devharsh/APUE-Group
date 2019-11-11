#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "network.h"

#define TRUE 1
#define DATA "Half a league, half a league..."

char buf[BUFSIZ];

socklen_t length;

struct sockaddr_in server;

struct hostent *hp, *gethostbyname();
struct sockaddr_in create_server_properties(char *address, int port);
