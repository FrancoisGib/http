#ifndef HTTP_H
#define HTTP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "http_tree.h"
#include "logger.h"

#define MAX_RESPONSE_SIZE 2048
#define MAX_REQUEST_SIZE 1024

#ifndef NB_PROCESSES
#define NB_PROCESSES 10
#endif

#define MIN(x, y) (x) < (y) ? (x) : (y)

typedef struct sockaddr_in sockaddr_in_t;

typedef struct
{
   int index;
   int sock;
   tree_t *endpoints;
} thread_params_t;

typedef struct
{
   char *name;
   char *value;
} header_t;

typedef char *(*resource_function)(char *);

typedef struct http_request_s
{
   char *method;
   char *path;
   char *http_version;
   ll_node_t *headers;
   int headers_length;
   char *body;
   int content_length;
} http_request_t;

void sigint_handler(int code);
char *strdup(const char *str);
int read_file(char buffer[1024], char *file_path);
void construct_response(int client_socket, http_request_t *http_request);
void send_error_response(int client_socket);
int http_request_parse_request_line(http_request_t *http_request, char **request_ptr);
int http_request_parse_headers(http_request_t *http_request, char **request_ptr);
int http_request_parse_body(http_request_t *http_request, char **request_ptr);
void *http_request_write_log_wrapper(void *http_request);
void accept_connection(void);
void start_server(int port);

#endif // HTTP_H
