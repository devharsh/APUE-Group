#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

struct response {
    char *date;
    char *server;
    char *last_modified;
    char *content_type;
    int  content_length;
    char *data;
};

FILE* fp;

int msgsock;
int open_connection(struct sockaddr *server, int protocol);
int handle_child_request();
int add_line_to_request(char *request, char *line, unsigned int buffersize);

bool is_request_complete(char *line, int *repeat_return);
bool parse_first_line(char *line, struct request *req);
bool validate_additional_information(char *line, struct request *req);
bool validate_date(char* date_str, struct request *req);
bool validate_tm(struct tm *time_ptr);
