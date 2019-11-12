#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>
#include <paths.h>

#include "network.h"

int fd;
int is_chdir;
int is_close;

char buf[BUFSIZ];

socklen_t length;

struct sockaddr_in server;
struct hostent *hp, *gethostbyname();
struct sockaddr_in create_server_properties(char *address, int port);
