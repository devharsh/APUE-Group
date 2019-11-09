#include "sws.h"

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		perror("usage error!");
		exit(1);
	}

	/* Create a socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}

	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8080);

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
		perror("binding stream socket");
		exit(1);
	}

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

	/* Find out assigned port number and print it out */
	length = sizeof(server);

	if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
		perror("getting socket name");
		exit(1);
	}

	printf("Socket has port #%d\n", ntohs(server.sin_port));

	/* Start accepting connections */
	listen(sock, 5);

	do {
		length = sizeof(client);
		msgsock = accept(sock, (struct sockaddr *)&client, &length);

		if(msgsock == -1) {
			perror("socket accept error");
			exit(1);
		} else do {
			bzero(buf, sizeof(buf));

			if ((rval = read(msgsock, buf, BUFSIZ)) < 0) {
				perror("reading stream message");
				exit(1);
			}

			if (rval == 0) 
				printf("\nEnding connection\n");
			else
				printf("Client (%s) sent: %s", 
					inet_ntoa(client.sin_addr), buf);
		} while (rval != 0);
		
		close(msgsock);
	} while (TRUE);

	close(sock);

	return 0;
}
