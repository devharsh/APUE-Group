#include "sws.h"

int
main(int argc, char* argv[]) {
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8080);
	
	while ((opt = getopt(argc, argv,"c:dhi:l:p:")) != -1) {  
        	switch(opt) {
			case 'c':
				flags.c = 1;
				break;
			case 'd':
				flags.d = 1;
				is_chdir = 0;
				is_close = 0;
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
				flags.l = 1;
				fp = fopen(optarg, "w+");
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

	if (setsid() == -1)
		return (-1);

	if (is_chdir)
		(void)chdir("/");

	/* daemonize the process */
	if (is_close && (fd = open(_PATH_DEVNULL, O_RDWR, 0)) != -1) {
		(void)dup2(fd, STDIN_FILENO);
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			(void)close(fd);
	}

	if (open_connection(server) != 0) {
		return 1;
	}

	fclose(fp);

	return 0;
}
