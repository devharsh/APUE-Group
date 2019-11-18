#include "network.h"

int
cgi_request(struct request *req, struct response *res) {
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

    if (pipe(output) < 0 || pipe(error_buffer) < 0) {
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

        (void) execv(req->uri, (char *) 0);
        return 1;

    } else {
        (void) close(output[1]);
        (void) close(error[1]);

        if ((output_buffer = malloc(BUFFERSIZE)) == NULL ||
            (error_buffer = malloc(BUFFERSIZE) == NULL)  ||
            (output_store = malloc(BUFFERSIZE) == NULL)  ||
            (error_store = malloc(BUFFERSIZE) == NULL)) {
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
set_environment() {
    /*
    * AUTH_TYPE Basic
    * CONTENT_LENGTH - doubt
    * CONTENT_TYPE  - doubt
    * GATEWAY_INTERFACE - CGI/1.1
    * PATH_INFO - request
    * PATH_TRANSLATED - request
    * QUERY_STRING - request
    * REQUEST_METHOD - request
    * SCRIPT_NAME - request
    * SERVER_NAME - get from hostname
    * SERVER_PORT - constant
    * SERVER_PROTOCOL - HTTP/1.0
    * SERVER_SOFTWARE - constant
    * 
    */

   if (setenv("AUTH_TYPE", "Basic", 1) != 0) {
       fprintf(stderr, "Could not set environment: %s\n", strerror(errno));
       return 1;
   }
   
}