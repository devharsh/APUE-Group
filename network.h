#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

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

int msgsock;

struct sockaddr_in create_server_properties(char *address, int port);
int  open_connection(char *address, int port);
bool is_request_complete(char *line, int *repeat_return);
int handle_child_request();
int add_line_to_request(char *request, char *line, unsigned int buffersize);
bool parse_first_line(char *line, struct request *req);
bool validate_additional_information(char *line, struct request *req);
bool validate_date(char*, struct request *req);
int validate_weekday(char*, char*);
int validate_month(char*);