#include "sws.h"

int
main(int argc, char* argv[]) {	
	/* int fd = 0;
	int is_chdir;*/
	int opt;
	int port;
	int daemonize;
	/*int is_close = 1;*/ 

	char *address = NULL;
	
	opt = 0;
	port = 8080;
 	is_chdir = 1;
	daemonize = false;
	
	server = malloc(sizeof(struct sockaddr));
	server_info.server_name = "SWS_HTTP/1.0";
	server_info.cgi_directory = "/cgi-bin"; 
	server_info.connections = 5;

	while ((opt = getopt(argc, argv,"c:dhi:l:p:")) != -1) {  
        	switch(opt) {
			case 'c':
				server_info.cgi_directory = optarg;
				break;
			case 'd':
				server_info.connections = 1;
				daemonize = false;
				break;
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
			case '?':
				fprintf(stderr, "usage: sws [-dh] [-c dir] [-i address] ");
			 	fprintf(stderr, "[-l file] [-p port] dir\n");
				return 1;
			default:
				break;
		}
	}

	if ((server = validate_address(address, port)) == NULL) {
		fprintf(stderr, "Not a valid address!");
		return 1;
	}

	if (daemonize) {
		if (daemon(0, 0) != 0) {
			fprintf(stderr, "Could not daemonize process: %s\n", strerror(errno));
			return 1;
		}
	}

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
