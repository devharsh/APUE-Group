#include "sws.h"

int
main(int argc, char* argv[]) {
	while((opt = getopt(argc, argv,"cdhilp")) != -1) {  
        	switch(opt) {
			case 'c':
				break;
			case 'd':
				break;
			case 'h':
				break;
			case 'i':
				break;
			case 'l':
				break;
			case 'p':
				break;	
			default:
				break;
		}
	}

	if (argc != 3) {
		perror("usage: a.out <hostname> <portnumber>");
		exit(1);
	}

	port = atoi(argv[2]);
	if ((port < 1) || (port > 65536)) {
		fprintf(stderr, "Invalid port: %s\n", argv[2]);
		exit(1);
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("opening stream socket");
		exit(1);
	}

	server.sin_family = AF_INET;
	if ((hp = gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "%s: unknown host\n", argv[1]);
		exit(2);
	}

	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(port);
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("connecting stream socket");
		exit(1);
	}

	if (write(sock, DATA, sizeof(DATA)) < 0)
		perror("writing on stream socket");
	close(sock);

	return 0;
}
