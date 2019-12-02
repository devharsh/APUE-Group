#include "network.h"

void
read_alarm_signal_handler(__attribute__((unused)) int signal) {
	fprintf(stderr, "Connection Timed out");
	(void) close(msgsock);
	exit(1);
}

void
handle_child_process(__attribute__((unused)) int signal) {
	(void) wait(NULL);
}

/**
 * open_connection opens up a connection and binds to the 
 * address and port on the machine and will listen to any
 * requests being sent to the socket.
 * 
 **/
int
open_connection(struct sockaddr *server, struct server_information server_info) {
	int sock, off;
	pid_t pid;
	socklen_t length;
	struct sockaddr client;
	char *client_address;
	int result;
	
	if (server_info.protocol == 4) {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		length = sizeof(struct sockaddr_in);
	} else {
		sock = socket(AF_INET6, SOCK_STREAM, 0);
		length = sizeof(struct sockaddr_in6);
	}
	
	if (sock < 0) {
		fprintf(stderr, "Could not open socket: %s \n", strerror(errno));
		return 1;
	}

	/*
	 * This condition is for us to determine if we need to listen on all ipv4 and ipv6 addresses
	 * 10 = combination of 4 and 6. This is not a protocol, just for logic purposes.
	 */
	if (server_info.protocol == 10) {
		off = 0;
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off)) != 0) {
			fprintf(stderr, "Could not change settings for socket: %s\n", strerror(errno));
			return 1;
		}
	}
	
	if (bind(sock, server, length) != 0) {
        fprintf(stderr, "Binding to socket failed: %s \n", strerror(errno));
		return 1;
	}

	/* Find out assigned port number and print it out */
	if (getsockname(sock, server, &length) != 0) {
		fprintf(stderr, "Error in getting socket name: %s \n", strerror(errno));
		return 1;
	}

	/* Start accepting connections */
	listen(sock, server_info.connections);

	if (signal(SIGCHLD, handle_child_process) == SIG_ERR) {
		fprintf(stderr, "Could not register signal: %s \n", strerror(errno));
		return 1;
	}

	do {
		length = sizeof(client);
		msgsock = accept(sock,	&client, &length);

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
			client_address = get_remote_host_ip(client);
			server_info.client_address = client_address;
			result = handle_child_request(server_info);
			return result;
		} else {
			(void) close(msgsock);
		}
	} while (TRUE);
	
	return 0;
}

char *
get_remote_host_ip(struct sockaddr client) {
	int protocol_family;
	char *ip_address;
	struct sockaddr_in *addr_in;
	struct sockaddr_in6 *addr_in6;

	protocol_family = client.sa_family;

	if (protocol_family == AF_INET6) {
		addr_in6 = (struct sockaddr_in6 *) &client;

		if ((ip_address = malloc(INET6_ADDRSTRLEN)) == NULL) {
			fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
			return NULL;
		}

		if (inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip_address, INET6_ADDRSTRLEN) == NULL) {
			fprintf(stderr, "Could not get address: %s\n", strerror(errno));
			return NULL;
		}
	} else {
		addr_in = (struct sockaddr_in *) &client;

		if ((ip_address = malloc(INET_ADDRSTRLEN)) == NULL) {
			fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
			return NULL;
		}

		if (inet_ntop(AF_INET, &(addr_in->sin_addr), ip_address, INET_ADDRSTRLEN) == NULL) {
			fprintf(stderr, "Could not get address: %s\n", strerror(errno));
			return NULL;
		}
	}

	return ip_address;
}

/**
 * All processing in the child process is available in this function
 * It basically reads from the socket until consecutive "\r\n" are encountered
 * 
 **/
int
handle_child_request(struct server_information server_info) {
	int  			init = false;
	int  			is_first_line = 1;
	int  			result = 0;
	char 			read_buf[BUFSIZ];
	int  			rval;
	int  			*repeat_return = &init;
	char 			*raw_request;
	struct request 	*req;
	char*           line_dup;
	struct response *res;


	if ((raw_request = malloc(BUFFERSIZE)) == NULL ||
		(req = malloc(sizeof(struct request))) == NULL ||
		(res = malloc(sizeof(struct response))) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		generate_error_response(res, server_info, 500, "Internal Server Error");
		req->method = "GET";
		write_response_to_socket(req, res);
		return 1;
	}

	raw_request[0] = '\0';

	if (signal(SIGALRM, read_alarm_signal_handler) == SIG_ERR) {
		fprintf(stderr, "Could not register signal: %s \n", strerror(errno));
		generate_error_response(res, server_info, 500, "Internal Server Error");
		req->method = "GET";
		write_response_to_socket(req, res);
		return 1;
	}

	if (alarm(TIMEOUT) == ((unsigned int) -1)) {
		fprintf(stderr, "Could not set alarm for timeout: %s \n", strerror(errno));
	}

	do {
		bzero(read_buf, sizeof(read_buf));
		if ((rval = read(msgsock, read_buf, BUFSIZ)) < 0) {
			send_request_error(req, res, server_info, 500, "Internal Server Error");
			write_response_to_socket(req, res);
			return 1;
		} else if (rval > 0) {
			if (is_first_line) {
				if ((line_dup = strdup(read_buf)) == NULL) {
					fprintf(stderr, "Could not duplicate String: %s \n", strerror(errno));
					send_request_error(req, res, server_info, 500, "Internal Server Error");
					(void) close(msgsock);
					return 1; 
				}

				bool valid_first_line = parse_first_line(line_dup, req);
				if (!valid_first_line) {
					send_request_error(req, res, server_info, 400, "Invalid request");
					(void) close(msgsock);
					return 1;
				} 
				is_first_line = false;
			} else {
				bool is_valid_header = validate_additional_information(read_buf, req);

				if (!is_valid_header) {
					send_request_error(req, res, server_info, 400, "Invalid request");
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
	
	process_request(req, res, server_info);
	write_response_to_socket(req, res);

	(void) alarm(0);

	if (server_info.log_file != NULL) {
		(void) log_request(req, res, server_info);
	}

	(void) free(raw_request);
	(void) free(req);
	(void) close(msgsock);

	return 0;
}

void
send_request_error(struct request *req, struct response *res, 
				   struct server_information server_info, int status, char *message) {
	generate_error_response(res, server_info, status, message);
	req->method = "GET";
	write_response_to_socket(req, res);
}

int
process_request(struct request *req, struct response *res, struct server_information info) {
	char *uri, *ptr, *last, *final_path;
	char path[PATH_MAX];
	int index;
	struct stat *sb;

	if ((sb = malloc(sizeof (struct stat))) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
		generate_error_response(res, info, 500, "Internal Server Error");
		return 1;
    }

	if ((uri = strdup(req->uri)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
		generate_error_response(res, info, 500, "Internal Server Error");
		return 1;
	}

	if ((final_path = malloc(PATH_MAX)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
		generate_error_response(res, info, 500, "Internal Server Error");
		return 1;
	}

	final_path[0] = '\0'; 

	(void) realpath(uri, path);

	if (strncmp(path, "/cgi-bin", 8) == 0) {
		ptr = strtok_r(uri, "/", &last);
		index = 0;

		while (ptr != NULL) {
			if (index == 0) {
				if (strcat(final_path, info.cgi_directory) == NULL) {
					fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
					generate_error_response(res, info, 500, "Internal Server Error");
					return 1;
				}
			} else {
				if (strcat(final_path, "/") == NULL) {
					fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
					generate_error_response(res, info, 500, "Internal Server Error");
					return 1;
				}

				if (strcat(final_path, ptr) == NULL) {
					fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
					generate_error_response(res, info, 500, "Internal Server Error");
					return 1;
				}
			}

			index++;
			ptr = strtok_r(NULL, "/", &last);
		}

		if ((req->uri = strdup(final_path)) == NULL) {
			fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
			generate_error_response(res, info, 500, "Internal Server Error");
			return 1;
		}

		(void) cgi_request(req, res, info);

	} else {
		ptr = strtok_r(uri, "?", &last);
		errno = 0;
		if (realpath(ptr, final_path) == NULL) {
			if (check_general_errors(res, info) == 0) {
				fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
				generate_error_response(res, info, 500, "Internal Server Error");
			}
			return 1;
		}

		if (errno != 0) {
			if (check_general_errors(res, info) == 0) {
				fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
				generate_error_response(res, info, 500, "Internal Server Error");
			}
			return 1;
		}

		if (stat(final_path, sb) != 0) {
			if (check_general_errors(res, info) == 0) {
				fprintf(stderr, "Something went wrong: %s\n", strerror(errno));
				generate_error_response(res, info, 500, "Internal Server Error");
			}
			return 1;
		}

		req->uri = final_path;
		
		if (sb->st_mtimespec.tv_sec > req->timestamp) {
			if (S_ISREG(sb->st_mode)) {
				(void) fileCopy(res, info, final_path);
			} else if (S_ISDIR(sb->st_mode)) {
				(void) traverse_files(req, res, info);
			} else {

			}
		} else {
			res->status = 304;
			res->server = info.server_name;
			res->content_length = 0;
		}
	}
	
	return 0;
}

int
check_general_errors(struct response *res, struct server_information info) {
	if (errno == EACCES) {
		fprintf(stderr, "Permission Denied: %s\n", strerror(errno));
		generate_error_response(res, info, 403, "Unauthorized Access");
		return 1;
	} else if (errno == ENOENT) {
		fprintf(stderr, "No such file or directory: %s\n", strerror(errno));
		generate_error_response(res, info, 404, "No such file or directory");
		return 1;
	}
	return 0;
}

void
write_response_to_socket(struct request *req, struct response *res) {
	time_t current_time;
  	struct tm * current_time_struct;
	char time_str[50];
	char status[3];
	char *content_length;
	int length, status_length;
	char *status_value, *full_status_string;

	length = get_number_of_digits(res->content_length);

	if ((content_length = malloc(length + 1)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
		exit(1);
	}

	if (sprintf(status, "%d", res->status) < 0) {
        fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
    }

	if (sprintf(content_length, "%d", res->content_length) < 0) {
    	fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
    }

	status_value = get_status_code_value(res->status);

	status_length = strlen(status_value) + strlen(status) + 1;

	if ((full_status_string = malloc(status_length)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
		exit(1);
	}

	if (sprintf(full_status_string, "%s %s", status, status_value) < 0) {
    	fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
    }
	
	(void) write_to_socket("HTTP/1.0 ", full_status_string);
	(void) write_to_socket("Content-Length: ", content_length);

	if (res->content_type != NULL) {
		(void) write_to_socket("Content-Type: ", res->content_type);
	}

	/* use helper function time_now? */	
  	(void) time(&current_time);
  	current_time_struct = gmtime(&current_time);

	/*Tue, 26 Nov 2019 22:51:25 GMT*/
	(void) strftime(time_str, sizeof(time_str), "%a, %d %b %Y %H:%M:%S %Z", current_time_struct);
	
	if (res->date != NULL) {
		(void) write_to_socket("Date: ", res->date);
	} else {
		(void) write_to_socket("Date: ", time_str);
	}
	
	if (res->last_modified != NULL) {
		(void) write_to_socket("Last-Modified: " , res->last_modified);
	}
	
	(void) write_to_socket("Server: ", res->server);
	if (res->status != 200 || strncmp(req->method, "GET", 3) == 0) {
		if (res->data != NULL) {
			(void) write(msgsock, "\n", 1);
			(void) write_to_socket(NULL, res->data);
		}
	}

	(void) free(content_length);
}

char *
decode_url(char *encoded_url) {
	char *decoded_url, *result;
    char hex_value[2];
    int  length, index;
    char append_character;
    
    length = strlen(encoded_url);

    if ((decoded_url = malloc(length + 1)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
        return NULL;
    }

    decoded_url[0] = '\0'; 

    for (index = 0; index < length; index++) {
        append_character = encoded_url[index];
        
        if (encoded_url[index] == '%') {
            if ((length - index) < 2) {
                append_char(decoded_url, append_character);
                continue;
            }

            hex_value[0] = encoded_url[index + 1];
            hex_value[1] = encoded_url[index + 2];

            if (isxdigit(hex_value[0]) && isxdigit(hex_value[1])) {
                append_character = (char) strtol(hex_value, NULL, 16);
                index += 2;
            }
        }

        append_char(decoded_url, append_character);
    }

	if ((result = strdup(decoded_url)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
        return NULL;
	}
    
	(void) free(decoded_url);

    return result;
}

void
log_request(struct request *req, struct response *res, struct server_information info) {
	int length, status_length, content_length;
	char *log;
	int left, transmitted;

	if (info.client_address != NULL) {
		status_length = get_number_of_digits(res->status);
		content_length = get_number_of_digits(res->content_length);
		length = strlen(info.client_address) + strlen(req->current_time) 
				 + strlen(req->method) + strlen(req->raw_request) + strlen(req->protocol)
				 + status_length + content_length + 9;
		if ((log = malloc(length)) == NULL) {
			fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
			exit(1);
		}

		if (sprintf(log, "%s %s \"%s %s %s\" %i %i\n", info.client_address, req->current_time,
					req->method, req->raw_request, req->protocol, res->status, res->content_length) < 0) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			exit(1);
		}

		left = strlen(log);

		while (left > 0) {
			if ((transmitted = write(info.log_file_descriptor, log, left)) < 0) {
				exit(1);
			}
			log += transmitted;
			left -= transmitted;
		}
	}

	(void) free(log);
}


void
generate_error_response(struct response *res, struct server_information info, int status, char *error) {
	char *error_content;

	if((error_content = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

	if (sprintf(error_content, "<h1>%s</h1><p>%s</p>", get_status_code_value(status) , error) < 0) {
		fprintf(stderr, "read error");
		exit(1);
	}

    res->status = status;
    res->data = generate_html(error_content);
    res->content_type = "text/html";
    res->content_length = strlen(res->data);
    res->server = info.server_name;
	
	(void) free(error_content);
}

char *
get_status_code_value(int status) {
	char *status_string;

	switch (status) {
		case 200:
			status_string = "OK";
			break;
		case 201:
			status_string = "Created";
			break;
		case 202:
			status_string = "Accepted";
			break;
		case 301:
			status_string = "Moved Permanently";
			break;
		case 302:
			status_string = "Moved Temporarily";
			break;
		case 304:
			status_string = "Not Modified";
			break;
		case 400:
			status_string = "Bad Request";
			break;
		case 401:
			status_string = "Unauthorized";
			break;
		case 403:
			status_string = "Forbidden";
			break;
		case 404:
			status_string = "Not Found";
			break;
		case 500:
			status_string = "Internal Server Error";
			break;
		case 501:
			status_string = "Not Implemented";
			break;
		case 502:
			status_string = "Bad Gateway";
			break;
		case 503:
			status_string = "Service Unavailable";
			break;
		default:
			break;
	}

	return status_string;
}

void
write_to_socket(char *key, const char *value) {
	int left, transmitted;
	int malloc_size;
	char *final_value;

	malloc_size = strlen(value) + 2;

	if (key != NULL) {
		malloc_size += strlen(key);
	}

	if ((final_value = malloc(malloc_size)) == NULL) {
		exit(1);
	}

	final_value[0] = '\0'; 

	if (key != NULL) {
		if (strcat(final_value, key) == NULL) {
			exit(1);
		}
	}

	if (strcat(final_value, value) == NULL) {
		exit(1);
	}

	if (strcat(final_value, "\n") == NULL) {
		exit(1);
	}

	left = strlen(final_value);

	while (left > 0) {
		if ((transmitted = write(msgsock, final_value, left)) < 0) {
			exit(1);
		}
		final_value += transmitted;
		left -= transmitted;
	}
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
	int  validate_req = 0, validate_protocol = 0, line_number = 0, n = 0;
	char *ptr, *last, *method, *uri, *protocol, *line_pointer_dup, *decoded_url;
	char *line_ptr = strtok_r(line, "\r\n", &last);
	time_t current_time;
  	struct tm * current_time_struct;
	char time_str[20];

	while (line_ptr != NULL) {
		line_number++;
		if (line_number > 1) {
			bool is_valid_header = validate_additional_information(line_ptr, req);
			if (is_valid_header) {
				validate_req = 1;
			} else {
				validate_req = 0;
			}
		} else {
			if ((line_pointer_dup = strdup(line_ptr)) == NULL) {
				fprintf(stderr, "Could not duplicate String: %s \n", strerror(errno));
				exit(1);
			}

			ptr = strtok(line_pointer_dup, " ");
			while (ptr != NULL) {
				switch (n) {
					case 0:
						if (strcmp(ptr, "GET") == 0 || strcmp(ptr, "HEAD") == 0) {
							method = ptr;
							validate_req = 1;
						}
						break;
					case 1:
						if (line_number == 1) {
							req->raw_request = ptr;
							uri = get_user_directroy_ifexists(ptr);
						}
						break;
					case 2:
						if (strcmp(ptr, "HTTP/1.0") == 0) {
							validate_protocol = 1;
							protocol = ptr;
						}
						break;
					default:
						validate_protocol = 0;
						validate_req = 0;
						break;
				}
				n++;
				ptr = strtok(NULL, " ");
			}
		}
		
		line_ptr = strtok_r(NULL, "\r\n", &last);
	}

	if (validate_req == 0 || validate_protocol == 0) {
		return false;
	} else {
		req->method = method;

		decoded_url = decode_url(uri);

		req->uri = decoded_url;
		req->protocol = protocol;

		(void) time(&current_time);
  		current_time_struct = gmtime(&current_time);
		(void) strftime(time_str, sizeof(time_str), "%Y-%b-%dT%H:%M:%SZ", current_time_struct);
		
		if ((req->current_time = strdup(time_str)) == NULL) {
			fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
			exit(1);
		}

		return true;
	}
}

bool
validate_additional_information(char *line, struct request *req) {
	char *line_pointer;
	char *date_string;
	bool valid = false;

	if ((date_string = malloc(strlen(line) + 1)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
	}

	if ((line_pointer = strdup(line)) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
	}

	if (strncasecmp(line, "If-Modified-Since:", 18) == 0) {
		date_string = strchr(line, ':') + 1;
		valid = validate_date(date_string, req);
	} else {
		valid = true;
	}

	/* (void) free(date_string); */
	return valid;
}

bool
validate_date(char* date_str, struct request *req) {
	char* date;
	struct tm *timeptr;
	bool valid = false;

	if ((timeptr = malloc(sizeof(struct tm))) == NULL) {
		fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
	}

	/*Skipping initial  whitespaces*/
	for (date = date_str; *date == ' ' || *date == '\t'; ++date)
		continue;

	/* wdy, DD mth YY HH:MM:SS GMT */
	if (strptime(date, "%a, %d %b %Y %T %z", timeptr) != NULL) {
			if(validate_tm(timeptr)) {
				req->time = timeptr;
				req->timestamp = timegm(timeptr);
				valid = true;
			}
    } 
	
	/* wdy, DD-mth-YY HH:MM:SS GMT */
	else if (strptime(date, "%a, %d-%b-%y %T %z", timeptr) != NULL) {
		if(validate_tm(timeptr)) {
			req->time = timeptr;
			req->timestamp = timegm(timeptr);
			valid = true;
		}
	}

	/* wdy mth DD HH:MM:SS YY */
	else if (strptime(date, "%a %b  %d %T %Y", timeptr) != NULL) {
		if(validate_tm(timeptr)) {
			req->time = timeptr;
			req->timestamp = timegm(timeptr);
			valid = true;
		}
	} else {
        valid = false;
    }

	(void) free(timeptr);
	return valid;
}

/**
 * If the request begins with a "˜", then the following string up to the 
 * first slash is translated into that user’s sws directory (ie /home/<user>/sws/)
 * */
char*           
get_user_directroy_ifexists(char* uri) {
    char            *loc;
    int             index;
    char            *user;
    char            *uri_path;
	char			*p;
	char 			*r_uri;
    struct passwd   *passwd;

	if((p = strchr(uri, '~')) == NULL ) {
		return uri;
    }

	p++; /* skipping `~` from the string */

    if((user = malloc(MAXNAMLEN)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    if((uri_path = malloc(PATH_MAX)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }
    
    if( (loc = strchr(p, '/')) != NULL) {
        index = (int) (loc - p);
        strncpy(user, p, index);

        if((passwd = getpwnam(user)) == NULL) {
            fprintf(stderr, "getpwnam() error: %s \n", strerror(errno));
			exit(1);
        }

        strcat(uri_path, passwd->pw_dir);
        strcat(uri_path, loc);

    } else {
        /* hanlde if slash(/) does not exist */
    }

	if ((r_uri = strdup(uri_path)) == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }

    (void) free(user);
    (void) free(uri_path);

	return r_uri;
}

/*
* TODO: add date validation
*/

bool            
validate_tm(struct tm *time_ptr) {

	int year = 1900 + time_ptr->tm_year;

	/* validating tm_mday - day of the month */
	if (!(time_ptr->tm_mday >= 1 && time_ptr->tm_mday <=31))  {
		return false;
	}

	/* validating tm_mday based on the month */
	switch (time_ptr->tm_mon)
	{
		case 0:
		case 2:
		case 4:
		case 6:
		case 7:
		case 9:
		case 11:
			if( !(time_ptr->tm_mday >= 1 && time_ptr->tm_mday <=31))  {
				return false;
			}
			break;
		case 3:
		case 5:
		case 8:
		case 10:
			if (!(time_ptr->tm_mday >= 1 && time_ptr->tm_mday <=  30))  {
				return false;
			}
			break;
		case 1:
				if (is_leap_year(year)) {
					if(!(time_ptr->tm_mday >= 1 && time_ptr->tm_mday <= 29))  {
						return false;
					}
				} else {
					if (!(time_ptr->tm_mday >= 1 && time_ptr->tm_mday <= 28))  {
						return false;
					}
				}
			break;
		default:
			return false;
			break;
	}

	return true;
}

/**
 * Funtion to check for leap year returns boolean true if 'yes'
 * or boolean false if 'no'
 * */
bool            
is_leap_year(int year) {
	if (year % 4 == 0) {
        if ( year % 100 == 0) {
            if ( year % 400 == 0)
                return true;
            else
                return false;
        }
        else
            return true;
    }
    else {
        return false;
	}
}


/**
 * Function to generate HTML content
 * */
char* 
generate_html(char* data) {
    char *html;
    char *r_html;

    if((html = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    if (sprintf(html, "<html><body>%s</body></html>", data) < 0) {
        fprintf(stderr, "read error %s\n", data);
    }

    if ((r_html = strdup(html)) == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(EXIT_FAILURE);
    }
    
    (void) free(html);
    
    return r_html;
}


unsigned int
get_number_of_digits(int number) {
    unsigned int count;
    
    if (number == 0) {
        return 1;
    }

    count = 0;

    while (number != 0) {
        number = number / 10;
        count++;
    }

    return count;
}

int
append_char(char *string, char character) {
    char *temp;
    if ((temp = malloc(2)) == NULL) {
         return 1;
    }
                
    temp[0] = character;
    temp[1] = '\0';

    if (strcat(string, temp) == NULL) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return 1;
    }

    (void) free(temp);

    return 0;
}