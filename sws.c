#include "sws.h"

int
main(int argc, char* argv[]) {
	int opt = 0;
	int port = 8080;
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8080);
	
	while ((opt = getopt(argc, argv,"c:dhi:l:p:")) != -1) {  
        	switch(opt) {
			case 'c':
				break;
			case 'd':
				break;
			case 'h':
				printf("usage: sws [-dh] [-c dir] [-i address]\
					 [-l file] [-p port] dir\n");
				return 0;
			case 'i':
				if ((hp = gethostbyname(optarg)) == NULL) {
					fprintf(stderr, "%s: unknown host\n",
						optarg);
					exit(2);
				}
				bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
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

				server.sin_port = htons(port);
				break;	
			default:
				break;
		}
	}

	if (open_connection(server) != 0) {
		return 1;
	}

	return 0;
}