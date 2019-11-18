#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>
#include <paths.h>

#include "network.h"

int fd = 0;
int is_chdir = 1;
int is_close = 1;
int opt = 0;
int port = 8080;

char buf[BUFSIZ];

socklen_t length;

struct sockaddr_in6 server;
struct hostent *hp, *gethostbyname();
struct sockaddr_in create_server_properties(char *address, int port);

struct sws_flags {
	int c;
	int d;
	int l;
} flags;
