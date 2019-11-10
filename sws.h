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

#define TRUE 1
#define DATA "Half a league, half a league..."

int opt = 0;
int sock = 0;
int msgsock = 0;
int rval = 0;
int port = 8080;

char buf[BUFSIZ];

socklen_t length;

struct sockaddr_in server;
struct sockaddr_in client;
struct hostent *hp, *gethostbyname();
