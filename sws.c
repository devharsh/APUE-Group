#include "sws.h"

int
main(int argc, char* argv[]) {	
	/* int fd = 0;*/
	int is_chdir;
	int opt;
	int port;
	/*int is_close = 1;*/ 

	char *address = NULL;
	
	opt = 0;
	port = 8080;
 	is_chdir = 1;
	
	server = malloc(sizeof(struct sockaddr));
	server_info.server_name = "SWS_HTTP/1.0";

	while ((opt = getopt(argc, argv,"c:dhi:l:p:")) != -1) {  
        	switch(opt) {
			case 'c':
				break;
			case 'd':
			case 'h':
				printf("usage: sws [-dh] [-c dir] [-i address] ");
			 	printf("[-l file] [-p port] dir\n");
				return 0;
			case 'i':
				address = optarg;
				break;
			case 'l':
				break;
			case 'p':
				port = atoi(optarg);

				if ((port < 1025) || (port > 65536)) {
					fprintf(stderr, "Invalid port: %s\n",
						optarg);
					exit(1);
				}

				break;	
			default:
				break;
		}
	}

	if ((server = validate_address(address, port)) == NULL) {
		fprintf(stderr, "Not a valid address!");
		return 1;
	}

	/*if (setsid() == -1)
		return (-1);*/

	if (is_chdir)
		(void) chdir("/");
	/* daemonize the process 
	if (is_close && (fd = open(_PATH_DEVNULL, O_RDWR, 0)) != -1) {
		(void)dup2(fd, STDIN_FILENO);
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			(void)close(fd);
	}*/
	
	if (open_connection(server, server_info) != 0) {
		return 1;
	}

	free(server);
	fclose(fp);

	return 0;
}


struct sockaddr*
validate_address(char *input_address, int port) {
	server_info.port = htons(port); 

	if (input_address == NULL) {
		server_info.protocol = 10;
		socket_address_ipv6.sin6_family = AF_INET6;
		socket_address_ipv6.sin6_addr = in6addr_any;
		socket_address_ipv6.sin6_port = htons(port);
		return (struct sockaddr*) &socket_address_ipv6;
	}

	if (inet_pton(AF_INET, input_address, &(socket_address_ipv4.sin_addr)) == 1) {
		server_info.protocol = 4;
		socket_address_ipv4.sin_family = AF_INET;
		socket_address_ipv4.sin_port = htons(port);
		return (struct sockaddr*) &socket_address_ipv4;
	}

	if (inet_pton(AF_INET6, input_address, &(socket_address_ipv6.sin6_addr)) == 1) {
		server_info.protocol = 6;
		socket_address_ipv6.sin6_family = AF_INET6;
		socket_address_ipv6.sin6_port = htons(port);
		return (struct sockaddr*) &socket_address_ipv6;
	}
	
	return NULL;
}