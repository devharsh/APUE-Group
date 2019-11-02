#include "network.h"

/**
 * open_connection opens up a connection and binds to the 
 * address and port on the machine and will listen to any
 * requests being sent to the socket.
 * 
 **/
int
open_connection(char *address, int port) {
    int sock, msgsock, rval;
	socklen_t length;
	struct sockaddr_in server;
	char buf[BUFSIZ];
	struct sockaddr_in client;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		fprintf(stderr, "Could not open socket \n", strerror(errno));
		return 1;
	}
	
    server = create_server_properties(server, address, port);
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
        fprintf(stderr, "Binding to socket failed \n", strerror(errno));
		return 1;
	}

	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
		fprintf(stderr, "Error in getting socket name \n", strerror(errno));
		return 1;
	}
	fprintf(stdout, "Socket port #%d\n", ntohs(server.sin_port));

	/* Start accepting connections */
	listen(sock, 5);
	do {
		length = sizeof(client);
		msgsock = accept(sock, (struct sockaddr *)&client, &length);
		if (msgsock == -1)
			perror("accept");
		else do {
			bzero(buf, sizeof(buf));
			if ((rval = read(msgsock, buf, BUFSIZ)) < 0)
				perror("reading stream message");
			if (rval == 0)
				printf("\nEnding connection\n");
			else {
				// printf("Client (%s) sent: %s", inet_ntoa(client.sin_addr), buf);
				printf("Client (%s) ", inet_ntoa(client.sin_addr));
				printf("sent: %s", buf);

            }
		} while (rval != 0);
		close(msgsock);
	} while (TRUE);
	
	return 0;
}

struct sockaddr_in 
create_server_properties(struct sockaddr_in server, char *address, int port) {
    server.sin_family = AF_INET;
    
    if (address == NULL) {
        server.sin_addr.s_addr = INADDR_ANY;
    } else {
        in_addr_t inet_address = inet_addr(address);
        printf("inet %i \n", inet_address);
        server.sin_addr.s_addr = inet_address;
    }

    if (port > 0) {
        server.sin_port = htons(port); 
    } else {
        server.sin_port = 0;
    }

    return server;
}