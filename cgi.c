#include "network.h"

char *path_info;
char *path;
char *query_string;

int
cgi_request(struct request *req,__attribute__((unused)) struct response *res, struct server_information server_info) {
    int output[2], error[2];
    char *output_buffer, *error_buffer;
    char *output_store, *error_store;
    int content;

    pid_t child;

    if ((output_buffer = malloc(BUFFERSIZE)) == NULL || 
        (error_buffer = malloc(BUFFERSIZE))) {
        fprintf(stdout, "Could not allocate memory: %s\n", strerror(errno));
        exit(1);
    }

    if (pipe(output) < 0 || pipe(error) < 0) {
        fprintf(stdout, "Could not generate output: %s \n", strerror(errno));
        exit(1);
    }

    if ((child = fork()) < 0) {
        fprintf(stderr, "Could not fork a new process: %s \n", strerror(errno));
        exit(1);
    } else if (child == 0) {
        (void) close(output[0]);
        (void) close(error[0]);

        if (output[1] != STDOUT_FILENO) {
            if (dup2(output[1], STDOUT_FILENO) != STDOUT_FILENO) {
                fprintf(stderr, "Could not duplicate fd: %s \n", strerror(errno));
                exit(1);
            }
        }

        if (error[1] != STDERR_FILENO) {
            if (dup2(error[1], STDERR_FILENO) != STDERR_FILENO) {
                fprintf(stderr, "Could not duplicate fd: %s \n", strerror(errno));
                exit(1);
            }
        }

        (void) set_environment(req, server_info);

        (void) execl(req->uri, (char *) 0);
        return 1;

    } else {
        (void) close(output[1]);
        (void) close(error[1]);

        if ((output_buffer = malloc(BUFFERSIZE)) == NULL ||
            (error_buffer = malloc(BUFFERSIZE)) == NULL  ||
            (output_store = malloc(BUFFERSIZE)) == NULL  ||
            (error_store = malloc(BUFFERSIZE)) == NULL) {
            fprintf(stderr, "Could not allocate space: %s \n", strerror(errno));
            exit(1);
        }

        while ((content = read(output[0], output_store, BUFFERSIZE)) > 0) {
            if (strlcat(output_buffer, output_store, BUFFERSIZE) > BUFFERSIZE) {
                fprintf(stderr, "Could not allocate space: %s \n", strerror(errno));
                exit(1);
            }
        }

        while ((content = read(error[0], error_store, BUFFERSIZE)) > 0) {
           if (strlcat(output_buffer, output_store, BUFFERSIZE) > BUFFERSIZE) {
                fprintf(stderr, "Could not allocate space: %s \n", strerror(errno));
                exit(1);
           }
        }

        (void) close(output[0]);
        (void) close(error[0]);
    }

    (void) free(output_buffer);
    (void) free(error_buffer);
    (void) free(output_store);
    (void) free(error_store);
 
    return 0;
}

int
is_valid_uri(char *uri) {
    if (strstr(uri, "/../") != NULL) {
        return false;
    }
    return true;
}

int
set_environment(struct request *req, struct server_information server_info) {
    char *hostname, *port;

    /*
    * PATH_TRANSLATED - request
    * SCRIPT_NAME - request
    */
    hostname = get_hostname();
    port = convert_int_to_string(server_info.port);

    (void) generate_uri_information(req->uri);

    (void) set_env("AUTH_TYPE", "Basic");
    (void) set_env("GATEWAY_INTERFACE", "CGI/1.1");
    (void) set_env("REQUEST_METHOD", req->method);
    (void) set_env("SERVER_PROTOCOL", "HTTP/1.0");
    (void) set_env("SERVER_SOFTWARE", server_info.server_name);
    (void) set_env("SERVER_PORT", port);
    (void) set_env("SERVER_NAME", hostname);
    (void) set_env("PATH_INFO", path_info);
    (void) set_env("QUERY_STRING", query_string);
    (void) set_env("SCRIPT_NAME", path);

    return 1;
}

void
set_env(char *key, char *value) {
    if (setenv(key, value, 1) == -1) {
        fprintf(stderr, "Could not get allocate memory: %s \n", strerror(errno));
        /*todo send response*/
    }
}

char *
get_hostname() {
    char *hostname, *hostname_assignment;

    if ((hostname = malloc(_POSIX_HOST_NAME_MAX)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
        return NULL;
    }

    if (gethostname(hostname, _POSIX_HOST_NAME_MAX) != 0) {
        fprintf(stderr, "Could not get hostname from server: %s \n", strerror(errno));
        return NULL;
    }

    if ((hostname_assignment = strdup(hostname)) == NULL) {
        fprintf(stderr, "Could not get allocate memory: %s \n", strerror(errno));
        return NULL;
    }

    (void) free(hostname);

    return hostname_assignment;
}

void
generate_uri_information(char *uri) {
    char *current_path, *last, *uri_dup, *ptr;
    int  index, retrived_file_name, count, length;
    struct stat *sb;

    if ((current_path = malloc(PATH_MAX)) == NULL) {
        exit(1);
    }

    if ((sb = malloc(sizeof (struct stat))) == NULL) {
        exit(1);
    }

    if ((uri_dup = strdup(uri)) == NULL) {
        exit(1);
    }

    current_path[0] = '\0'; 
    retrived_file_name = 0;
    count = 1;

    ptr = strtok_r(uri_dup, "?", &last);

    while (ptr != NULL) {
        if (count > 1) {
            if (strcat(current_path, ptr) == NULL) {
                fprintf(stderr, "error: %s\n", strerror(errno));
                exit(1);
            }
        } else {
            length = strlen(ptr);
            for (index = 0; index < length; index++) {
                if (ptr[index] == '/' && retrived_file_name == 0) {
                    if (strlen(current_path) > 0) {
                        if (stat(current_path, sb) != 0) {
                            fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                            exit(1);
                        }

                        if (S_ISREG(sb->st_mode)) {
                            retrived_file_name = 1;
                            
                            if ((path = strdup(current_path)) == NULL) {
                                fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                                exit(1);
                            }

                            (void) bzero(current_path, strlen(current_path));
                        } 
                    } 
                }
                append_char(current_path, ptr[index]);
            }

            if (strlen(current_path) > 0) {
                if (retrived_file_name == 0) {
                    if (stat(current_path, sb) != 0) {
                        fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                        exit(1);
                    }

                    if (S_ISREG(sb->st_mode)) {
                        if ((path = strdup(current_path)) == NULL) {
                            fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                            exit(1);
                        }
                    }
                } else {
                    if ((path_info = strdup(current_path)) == NULL) {
                        fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                        exit(1);
                    }
                }

                (void) bzero(current_path, strlen(current_path));
            }
        }
        count++;
        ptr = strtok_r(NULL, "&", &last);
    }

    if (count > 1 && strlen(current_path) > 0) {
        if ((query_string = strdup(current_path)) == NULL) {
            fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
            exit(1);
        }
    }

    (void) free(current_path);
    (void) free(sb);
}

void
append_char(char *string, char character) {
    char *temp;
    if ((temp = malloc(2)) == NULL) {
         exit(1);
    }
                
    temp[0] = character;
    temp[1] = '\0';

    if (strcat(string, temp) == NULL) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    (void) free(temp);
}

char *
convert_int_to_string(int number) {
    int length;
    char *str;

    length = get_number_of_digits(number);

    if ((str = malloc(length + 1)) == NULL) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    if (sprintf(str, "%d", number) < 0) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    /*TODO using strdup*/
    return str;
}

unsigned int
get_number_of_digits(int number) {
    unsigned int count;
    
    if(number == 0) {
        return 1;
    }

    count = 0;

    while(number != 0) {
        number = number / 10;
        count++;
    }

    return count;
}