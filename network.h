#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
<<<<<<< HEAD
#include <dirent.h>
#include <fts.h>
=======
#include <limits.h>
>>>>>>> f04e1aa48489313cb67f4d2bae176bc90828899a

#define TRUE 1
#define BUFFERSIZE 16384
#define TIMEOUT 60

struct request {
    char        *method;
    char        *protocol;
    char        *uri;
    char        *modified_since;
    struct tm   *time;
    time_t      timestamp;
};


/**
 * Status-Code    = "200"   ; OK
                      | "201"   ; Created
                      | "202"   ; Accepted
                      | "204"   ; No Content
                      | "301"   ; Moved Permanently
                      | "302"   ; Moved Temporarily
                      | "304"   ; Not Modified
                      | "400"   ; Bad Request
                      | "401"   ; Unauthorized
                      | "403"   ; Forbidden
                      | "404"   ; Not Found
                      | "500"   ; Internal Server Error
                      | "501"   ; Not Implemented
                      | "502"   ; Bad Gateway
                      | "503"   ; Service Unavailable
                      | extension-code
*/
struct response {
    char *date;
    char *server;
    char *last_modified;
    char *content_type;
    int  content_length;
    char *data;
};

struct server_information {
    int     protocol;
    int     port;
    char    *server_name;
    char    *ip_address;
};

FILE* fp;

int msgsock;
<<<<<<< HEAD
int open_connection(struct sockaddr *server, int protocol);
int (*compar) (const FTSENT **, const FTSENT **);
=======
int open_connection(struct sockaddr *server, struct server_information);
>>>>>>> f04e1aa48489313cb67f4d2bae176bc90828899a
int handle_child_request();
void handle_child_process(__attribute__((unused)) int signal);
int add_line_to_request(char *request, char *line, unsigned int buffersize);

bool is_request_complete(char *line, int *repeat_return);
bool parse_first_line(char *line, struct request *req);
bool validate_additional_information(char *line, struct request *req);
bool validate_date(char* date_str, struct request *req);
bool validate_tm(struct tm *time_ptr);
<<<<<<< HEAD
int traverse_files(struct request *req);
int sortLexographical(const FTSENT **fileEntryPointer, const FTSENT **fileEntryPointerTwo);
void generate_html(char* data);
=======
void set_env(char *key, char *value);
char* get_hostname();
>>>>>>> f04e1aa48489313cb67f4d2bae176bc90828899a
