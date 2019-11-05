#include "network.h"

void
read_alarm_signal_handler(int signal) {
	fprintf(stderr, "Read timeout for connection");
	(void) close(msgsock);
	exit(1);
}

/**
 * open_connection opens up a connection and binds to the 
 * address and port on the machine and will listen to any
 * requests being sent to the socket.
 * 
 **/
int
open_connection(char *address, int port) {
    int sock, rval;
	pid_t pid;
	socklen_t length;
	struct sockaddr_in server;
	char buf[BUFSIZ];
	struct sockaddr_in client;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		fprintf(stderr, "Could not open socket: %s \n", strerror(errno));
		return 1;
	}
	
    server = create_server_properties(server, address, port);
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
        fprintf(stderr, "Binding to socket failed: %s \n", strerror(errno));
		return 1;
	}

	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
		fprintf(stderr, "Error in getting socket name: %s \n", strerror(errno));
		return 1;
	}
	fprintf(stdout, "Socket port #%d\n", ntohs(server.sin_port));

	/* Start accepting connections */
	listen(sock, 5);

	do {
		length = sizeof(client);
		msgsock = accept(sock, (struct sockaddr *)&client, &length);

		if (msgsock == -1) {
			fprintf(stderr, "Could not accept socket connection: %s \n", strerror(errno));
			return 1;
		}
		/*
		* fork a new process to handle the request, since we have a queue with a backlog,
		* if we do not do this then the other requests are blocked until we complete the 
		* current request.
		*/
		if ((pid = fork()) < 0) {
			fprintf(stderr, "Unable to fork process: %s \n", strerror(errno));
			return 1;
		} else if (pid == 0) {
			if (signal(SIGALRM, read_alarm_signal_handler) == SIG_ERR) {
				fprintf(stderr, "Could not register signal: %s \n", strerror(errno));
				return 1;
			}

			if (alarm(TIMEOUT) < 0) {
				fprintf(stderr, "Could not set alarm for timeout: %s \n", strerror(errno));
			}

			do {
				bzero(buf, sizeof(buf));
				if ((rval = read(msgsock, buf, BUFSIZ)) < 0) {
					fprintf(stderr, "Could not read message from socket: %s\n", strerror(errno));
					return 1;
				}
			} while (rval != 0);

			(void) alarm(0);
		} else {
			(void) close(msgsock);
		}
	} while (TRUE);
	
	return 0;
}

/**
 * Create server properties based on the input provided by the user
 * The properties include ip and port to be used. If no ip and port is provided
 * then we listen on all the ip addresses available to us and we listen to a
 * random port assigned to us.
 * 
 **/
struct sockaddr_in 
create_server_properties(struct sockaddr_in server, char *address, int port) {
    server.sin_family = AF_INET;
    
    if (address == NULL) {
        server.sin_addr.s_addr = INADDR_ANY;
    } else {
        in_addr_t inet_address = inet_addr(address);
        server.sin_addr.s_addr = inet_address;
    }

    if (port > 0) {
        server.sin_port = htons(port); 
    } else {
        server.sin_port = 0;
    }

    return server;
}
