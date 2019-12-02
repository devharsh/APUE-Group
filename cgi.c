#include "network.h"

char *path_info;
char *path;
char *query_string;

void
handle_child_exec_process(__attribute__((unused)) int signal) {
    printf("handle_child_exec_process\n");
	(void) waitid(P_ALL, 0, NULL ,WNOHANG);
}

int
cgi_request(struct request *req, struct response *res, struct server_information server_info) {
    int output[2], error[2];
    char *output_buffer, *error_buffer;
    char *output_store, *error_store;
    int content;
    char *environment[12]; 
    char *arg[1];

    pid_t child;

    res->status = 200;

    if ((output_buffer = malloc(BUFFERSIZE)) == NULL || 
        (error_buffer = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stdout, "Could not allocate memory 1: %s\n", strerror(errno));
        generate_error_response(res, server_info, 500, "Could not allocate memory");
        return 1;
    }

    if (pipe(output) < 0 || pipe(error) < 0) {
        fprintf(stdout, "Could not generate output: %s \n", strerror(errno));
        generate_error_response(res, server_info, 500, "Could not allocate memory");
        return 1;
    }

    if ((set_environment(req, res, server_info, environment)) == NULL) {
        if (res->status == 200) {
            generate_error_response(res, server_info, 500, "Internal Server Error");
        }
        return 1;
    }

    if ((child = fork()) < 0) {
        fprintf(stderr, "Could not fork a new process: %s \n", strerror(errno));
        generate_error_response(res, server_info, 500, "Internal Server Error");
        return 1;
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

        arg[0] = '\0';
        
        (void) execvpe(path, arg, environment);
        
        exit(1);
    } else {
        (void) close(output[1]);
        (void) close(error[1]);

        if ((output_buffer = malloc(BUFFERSIZE)) == NULL ||
            (error_buffer = malloc(BUFFERSIZE)) == NULL  ||
            (output_store = malloc(BUFFERSIZE)) == NULL  ||
            (error_store = malloc(BUFFERSIZE)) == NULL) {
            fprintf(stderr, "Could not allocate space: %s \n", strerror(errno));
            generate_error_response(res, server_info, 500, "Internal Server Error");
            return 1;
        }

        while ((content = read(output[0], output_store, BUFFERSIZE)) > 0) {
            if (strlcat(output_buffer, output_store, BUFFERSIZE) > BUFFERSIZE) {
                fprintf(stderr, "Could not allocate space: %s \n", strerror(errno));
                generate_error_response(res, server_info, 500, "Internal Server Error");
                return 1;
            }
        }

        while ((content = read(error[0], error_store, BUFFERSIZE)) > 0) {
           if (strlcat(output_buffer, output_store, BUFFERSIZE) > BUFFERSIZE) {
                fprintf(stderr, "Could not allocate space: %s \n", strerror(errno));
                generate_error_response(res, server_info, 500, "Internal Server Error");
                return 1;
           }
        }

        generate_response(res, server_info, output_buffer, error_buffer);

        (void) close(output[0]);
        (void) close(error[0]);
    }

    (void) free(output_buffer);
    (void) free(error_buffer);
    (void) free(output_store);
    (void) free(error_store);
 
    return 0;
}

void
generate_response(struct response *res, struct server_information info, char *output, char *error) {
    char *output_dup, *error_dup;
    
    if (strlen(output) > 0) {
        if (strncmp(output, "Content-Type:", 13) != 0 ) {
            generate_error_response(res, info, 500, "Incorrect Output from script");
            return;
        } else {
            res->status = 200;
        }

        res->content_length = strlen(output);
        
        if ((output_dup = strdup(output)) == NULL) {
            generate_error_response(res, info, 500, "Could not allocate memory");
            return;
        } else {
            res->data = output_dup;
        }

        res->server = info.server_name;
    } else {
        if (strlen(error) > 0) {
            if ((error_dup = strdup(error)) == NULL) {
                generate_error_response(res, info, 500, "Could not allocate memory");
                return;
            }
            generate_error_response(res, info, 500, error_dup);
        } else {
            generate_error_response(res, info, 500, "Internal Server Error 1");
        }
        return;
    }    
}

int
is_valid_uri(char *uri) {
    if (strstr(uri, "/../") != NULL) {
        return false;
    }
    return true;
}

char **
set_environment(struct request *req, struct response *res, struct server_information server_info, char **environment) {
    char *hostname, *port, *env_path;
    int length, uri_status;
    int env_index = 0;
    
    /*
    * PATH_TRANSLATED - request
    */
   if ((hostname = malloc(_POSIX_HOST_NAME_MAX)) == NULL) {
        fprintf(stderr, "Could not allocate memory 7: %s\n", strerror(errno));
        return NULL;
    }

    length = get_number_of_digits(server_info.port);

    if ((port = malloc(length + 1)) == NULL) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return NULL;
    }

    if (gethostname(hostname, _POSIX_HOST_NAME_MAX) != 0) {
        fprintf(stderr, "Could not get hostname from server: %s \n", strerror(errno));
        return NULL;
    }

    if (convert_int_to_string(server_info.port, port) != 0) {
        return NULL;
    }

    if((uri_status = generate_uri_information(req->uri)) > 0) {
        return NULL;
    } else if (uri_status < 0) {
        generate_error_response(res, server_info, 404, "Resource not found as mentioned");
        return NULL;
    }

    environment[env_index++] = "AUTH_TYPE=Basic";
    environment[env_index++] = "GATEWAY_INTERFACE=CGI/1.1";
    environment[env_index++] = "SERVER_PROTOCOL=HTTP/1.0";

    if ((environment[env_index++] = 
            get_env_string("SERVER_SOFTWARE=", server_info.server_name)) == NULL) {
        return NULL;
    }

    if ((environment[env_index++] = 
            get_env_string("SERVER_PORT=", port)) == NULL) {
        return NULL;
    }

    if ((environment[env_index++] = 
            get_env_string("SERVER_NAME=", hostname)) == NULL) {
        return NULL;
    }
    
    if (path_info != NULL) {
        if ((environment[env_index++] = 
                get_env_string("PATH_INFO=", path_info)) == NULL) {
            return NULL;
        }
    }

    if (query_string != NULL) {
        if ((environment[env_index++] = 
                get_env_string("QUERY_STRING=", query_string)) == NULL) {
            return NULL;
        }
    }

    if ((env_path = getenv("PATH")) != NULL) {
        if ((environment[env_index++] = 
                get_env_string("PATH=", env_path)) == NULL) {
            return NULL;
        }
    }

    if ((environment[env_index++] = 
            get_env_string("SCRIPT_NAME=", path)) == NULL) {
        return NULL;
    }

    environment[env_index++] = '\0';
    
    return environment;
}

char *
get_env_string(char *key, char *value) {
    char *buffer, *result;
    size_t size;

    size = strlen(key) + strlen(value) + 1;

    if ((buffer = malloc(size)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
        return NULL;
    }

    if (snprintf(buffer, size, "%s%s", key, value) < 0) {
        fprintf(stderr, "Could not copy string: %s\n", strerror(errno));
        return NULL;
    }

    if ((result = strdup(buffer)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
        return NULL;
    }

    return result;
}

int
generate_uri_information(char *uri) {
    char *current_path, *last, *uri_dup, *ptr;
    int  index, retrived_file_name, count, length;
    struct stat *sb;

    if ((current_path = malloc(PATH_MAX)) == NULL) {
        return 1;
    }

    if ((sb = malloc(sizeof (struct stat))) == NULL) {
        return 1;
    }

    if ((uri_dup = strdup(uri)) == NULL) {
        return 1;
    }

    current_path[0] = '\0'; 
    retrived_file_name = 0;
    count = 1;

    ptr = strtok_r(uri_dup, "?", &last);

    while (ptr != NULL) {
        if (count > 1) {
            if (strcat(current_path, ptr) == NULL) {
                fprintf(stderr, "error: %s\n", strerror(errno));
                return 1;
            }
        } else {
            length = strlen(ptr);
            for (index = 0; index < length; index++) {
                if (ptr[index] == '/' && retrived_file_name == 0) {
                    if (strlen(current_path) > 0) {
                        if (stat(current_path, sb) != 0) {
                            fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                            return -1;
                        }

                        if (S_ISREG(sb->st_mode)) {
                            retrived_file_name = 1;
                            
                            if ((path = strdup(current_path)) == NULL) {
                                fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                                return 1;
                            }

                            (void) bzero(current_path, strlen(current_path));
                        } 
                    } 
                }
                if (append_char(current_path, ptr[index]) != 0) {
                    return 1;
                }
            }

            if (strlen(current_path) > 0) {
                if (retrived_file_name == 0) {
                    if (stat(current_path, sb) != 0) {
                        fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                        return -1; 
                    }

                    if (S_ISREG(sb->st_mode)) {
                        if ((path = strdup(current_path)) == NULL) {
                            fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                            return 1;
                        }
                    } else {
                        return -1;
                    }
                } else {
                    if ((path_info = strdup(current_path)) == NULL) {
                        fprintf(stderr, "error: %s %s\n", strerror(errno), current_path);
                        return 1;
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
            return 1;
        }
    }

    (void) free(current_path);
    (void) free(sb);

    return 0;
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

int
convert_int_to_string(int number, char *str) {
    if (sprintf(str, "%d", number) < 0) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return 1;
    }
    return 0;
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