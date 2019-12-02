
#include <ctype.h>
#include <fcntl.h>
#include <paths.h>

#include "network.h"

int protocol;
int fd = 0;
int is_chdir = 1;
int is_close = 1;
char buf[BUFSIZ];

struct sockaddr_in socket_address_ipv4;
struct sockaddr_in6 socket_address_ipv6;
struct server_information server_info;

socklen_t length;

struct sockaddr *server;
struct hostent *hp, *gethostbyname();
struct sockaddr_in create_server_properties(char *address, int port);
struct sockaddr* validate_address(char *input_address, int port);
int	get_logging_file_descriptor(char *path);

struct sws_flags {
	int c;
	int d;
	int l;
} flags;
