#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <libgen.h>
#include <limits.h>
#include <netdb.h>
#include <paths.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "helper.h"

#define TRUE 1
#define BUFFERSIZE 1048576
#define TIMEOUT 60

/* 1 MB buffer size for copying */
#ifndef BUF_SIZE
#define BUF_SIZE 1048576 
#endif

/* limit for a file name */
#ifndef BUF_LIMIT
#define BUF_LIMIT 512
#endif

struct request {
    char        *method;
    char        *protocol;
    char        *uri;
    char        *modified_since;
    struct tm   *time;
    time_t      timestamp;
};

struct response {
    int status;
    char *date; 
    char *server; 
    char *last_modified; 
    const char *content_type; 
    int  content_length; 
    char *data; 
};

struct server_information {
    int     protocol;
    int     port;
    char    *server_name;
    char    *ip_address;
    char    *cgi_directory;
    int     connections;
};

FILE* fp;
int     	msgsock;

int     	open_connection(struct sockaddr *server, struct server_information);
int     	(*compar) (const FTSENT **, const FTSENT **);
int     	handle_child_request(struct server_information server_info);
void    	handle_child_process(__attribute__((unused)) int signal);
int     	add_line_to_request(char *request, char *line, unsigned int buffersize);
char*           generate_html(char* data);
int             append_char(char *string, char character);
int             generate_uri_information(char *uri);
char**          set_environment(struct request *req, struct response *res, struct server_information server_info, char **environment);
char*           get_env_string(char *key, char *value);
unsigned int    get_number_of_digits(int number);
int             convert_int_to_string(int number, char *str);
bool            is_request_complete(char *line, int *repeat_return);
bool            parse_first_line(char *line, struct request *req);
bool            validate_additional_information(char *line, struct request *req);
bool            validate_date(char* date_str, struct request *req);
bool            validate_tm(struct tm *time_ptr);
int             traverse_files(struct request *req, struct response *res, struct server_information info);
int             sortLexographical(const FTSENT **fileEntryPointer, const FTSENT **fileEntryPointerTwo);
char*           generate_error_contents(int e_no);
char*           prepare_listing_table(char* data);
void            prepare_response_directorylisting(struct response *res, char* html, int status, struct server_information info);
void            generate_error_response(struct response *res, struct server_information info, int status, char *error);
void            generate_response(struct response *res, struct server_information info, char *output, char *error);
int             cgi_request(struct request *req, struct response *res, struct server_information server_info);
void            write_response_to_socket(struct request *req, struct response *res);
void            write_to_socket(char *key, const char *value);
char*           get_user_directroy_ifexists(char* uri);
bool            is_leap_year(int year);
int             process_request(struct request *req, struct response *res, struct server_information info);
int             check_general_errors(struct response *res, struct server_information info);
char *          get_status_code_value(int status);
void            handle_child_exec_process(__attribute__((unused)) int signal);
void            send_request_error(struct request *req, struct response *res, struct server_information server_info, int status, char *message);
int		fileCopy(struct response *res, struct server_information server_info, char* source);
