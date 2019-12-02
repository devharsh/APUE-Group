#include "network.h"

struct sockaddr_in socket_address_ipv4;
struct sockaddr_in6 socket_address_ipv6;
struct server_information server_info;

socklen_t length;

struct sockaddr *server;
struct hostent *hp, *gethostbyname();
struct sockaddr_in create_server_properties(char *address, int port);
struct sockaddr* validate_address(char *input_address, int port);
int	get_logging_file_descriptor(char *path);
void check_cgi_file(char *path);

struct sws_flags {
	int c;
	int d;
	int l;
} flags;
