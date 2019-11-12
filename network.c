#include "network.h"

void
read_alarm_signal_handler(int signal) {
	if (signal == SIGALRM) {
		fprintf(stderr, "Read timeout for connection");
		(void) close(msgsock);
		exit(1);
	}
}

/**
 * open_connection opens up a connection and binds to the 
 * address and port on the machine and will listen to any
 * requests being sent to the socket.
 * 
 **/
int
open_connection(struct sockaddr_in server) {
	int sock;
	pid_t pid;
	socklen_t length;
	struct sockaddr_in client;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		fprintf(stderr, "Could not open socket: %s \n", strerror(errno));
		return 1;
	}
	
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
			int result = handle_child_request();
			return result;
		} else {
			(void) close(msgsock);
		}
	} while (TRUE);
	
	return 0;
}

/**
 * All processing in the child process is available in this function
 * It basically reads from the socket until consecutive "\r\n" are encountered
 * 
**/
int
handle_child_request() {
	int  			init = false;
	int  			is_first_line = 1;
	int  			result = 0;
	char 			read_buf[BUFSIZ];
	int  			rval;
	int  			*repeat_return = &init;
	char 			*raw_request = malloc(BUFFERSIZE);
	struct request 	*req = malloc(sizeof(struct request));
	char*           line_dup;
	raw_request[0] = '\0';

	if (raw_request == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		return 1;
	}

	if (signal(SIGALRM, read_alarm_signal_handler) == SIG_ERR) {
		fprintf(stderr, "Could not register signal: %s \n", strerror(errno));
		return 1;
	}

	if (alarm(TIMEOUT) == ((unsigned int) -1)) {
		fprintf(stderr, "Could not set alarm for timeout: %s \n", strerror(errno));
	}

	do {
		bzero(read_buf, sizeof(read_buf));
		if ((rval = read(msgsock, read_buf, BUFSIZ)) < 0) {
			fprintf(stderr, "Could not read message from socket: %s\n", strerror(errno));
			return 1;
		} else if (rval > 0) {
			if (is_first_line) {
				line_dup = strdup(read_buf);
				bool valid_first_line = parse_first_line(line_dup, req);
				if (!valid_first_line) {
					(void) close(msgsock);
					return 1;
				} 
				is_first_line = false;
			} else {
				bool is_valid_header = validate_additional_information(read_buf, req);

				if (!is_valid_header) {
					(void) close(msgsock);
					return 1;
				}
			}

			result = add_line_to_request(raw_request, read_buf, BUFFERSIZE);
			if (result != 0) {
				free(raw_request);
				return 1;
			}
		}
	} while (!is_request_complete(read_buf, repeat_return));

	printf("input %s \n", raw_request);

	(void) alarm(0);
	(void) free(raw_request);
	(void) free(req);
	(void) close(msgsock);

	return 0;
}

int
add_line_to_request(char *request, char *line, unsigned int buffersize) {
	if (strlcat(request, line, buffersize) > buffersize) {
		fprintf(stderr, "Could not copy line to request: %s \n",
				strerror(errno));
		return 1;
	}
	return 0;
}

bool
is_request_complete(char *line, int *repeat_return) {
	if (strstr(line, "\015\012\015\012") != NULL) {
		return true;
	} else if (strstr(line, "\015\012") != NULL) {
		if (*repeat_return && (strcmp(line, "\015\012") == 0)) {
			return true;
		} else {
			*repeat_return = true;
		}
	} else {
		*repeat_return = false;
	}
	return false;
}

/**
 * Function to parse the first line from the buffer and validate
 * */
bool
parse_first_line(char *line, struct request *req) {
	int   validate_req = 0, validate_protocol = 0, line_number = 0;
	char* method;
	char* uri;
	char* protocol;
	char *ptr;
	int  n = 0;
	char *line_pointer_dup;
	char *line_ptr = strtok(line, "\r\n");

	while (line_ptr != NULL) {
		line_number++;
		line_pointer_dup = strdup(line_ptr);
		ptr = strtok(line_pointer_dup, " ");
		
		while (ptr != NULL) {
			switch (n) {
				case 0:
					if ((strcmp(ptr, "GET") == 0 || strcmp(ptr, "HEAD") == 0) && (line_number == 1)) {
						method = ptr;
						validate_req = 1;
					}
					break;
				case 1:
					if (line_number == 1) {
						uri = ptr;
					}
					break;
				case 2:
					if (strcmp(ptr, "HTTP/1.0") == 0 && line_number == 1) {
						validate_protocol = 1;
						protocol = ptr;
					}
					break;
				default:
					if (line_number == 1) {
						validate_protocol = 0;
						validate_req = 0;
					} else {
						bool is_valid_header = validate_additional_information(ptr, req);

						if (is_valid_header) {
							validate_req = 1;
						} else {
							validate_req = 0;
						}
					}
					break;
			}
			n++;
			ptr = strtok(NULL, " ");
		}

		line_ptr = strtok(NULL, "\r\n");
	}

	if (validate_req == 0 || validate_protocol == 0) {
		return false;
	} else {
		req->method = method;
		req->uri = uri;
		req->protocol = protocol;
		fprintf(fp, "method=%s,\tprotocol=%s,\turi=%s", method, protocol, uri);
		return true;
	}
}

bool
validate_additional_information(char *line, struct request *req) {
	char *line_pointer = strdup(line);
	char *date_string = malloc(strlen(line) + 1);
	bool valid = false;

	if (line_pointer == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
	}

	if (strncasecmp(line, "If-Modified-Since:", 18) == 0) {
		date_string = strchr(line, ':') + 1;
		valid = validate_date(date_string, req);
	} else {
		valid = true;
	}

	(void) free(date_string);
	return valid;
}

bool
validate_date(char* date_str, struct request *req) {
	char* date;
	struct tm *timeptr = malloc(sizeof(struct tm));
	bool valid = false;

	/*Skipping initial  whitespaces*/
	for (date = date_str; *date == ' ' || *date == '\t'; ++date)
	continue;

	/* wdy, DD mth YY HH:MM:SS GMT */
	if (strptime(date, "%a, %d %b %Y %T %z", timeptr) != NULL) {
			req->time = timeptr;
			req->timestamp = mktime(timeptr);
			valid = true;
    } 
	
	/* wdy, DD-mth-YY HH:MM:SS GMT */
	else if (strptime(date, "%a, %d-%b-%y %T %z", timeptr) != NULL) {
			req->time = timeptr;
			req->timestamp = mktime(timeptr);
			valid = true;
	}

	/* wdy mth DD HH:MM:SS YY */
	else if (strptime(date, "%a %b  %d %T %Y", timeptr) != NULL) {
			req->time = timeptr;
			req->timestamp = mktime(timeptr);
			valid = true;
	} else {
        valid = false;
    }

	(void) free(timeptr);
	return valid;
}

bool
validate_tm(struct tm *time_ptr) {
	char *time_buffer = malloc(11);	
	char *strf_buffer = malloc(100);
	bool valid = false;

	time_buffer[0] = '\0';
	strf_buffer[0] = '\0';

	if (time_buffer == NULL || strf_buffer == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
	}

	if (sprintf(time_buffer, "%i/%i/%i", time_ptr->tm_mon + 1, time_ptr->tm_mday, time_ptr->tm_year) < 0) {
		fprintf(stderr, "Could not create string from time: %s \n", strerror(errno));
		exit(1);
	}

	strftime(strf_buffer, sizeof(strf_buffer), "%x", time_ptr);

	if (strcmp(time_buffer, strf_buffer) == 0) {
		valid = true;
	}

	(void) free(time_buffer);
	(void) free(strf_buffer);

	return valid;
}
