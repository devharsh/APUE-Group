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

#define DATA "Half a league, half a league..."

int opt = 0;
int sock, port;

struct sockaddr_in server;
struct hostent *hp, *gethostbyname();
