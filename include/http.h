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
#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "lib.h"
#include "http_tree.h"
#include "logger.h"
#include "vars.h"
#include "http_status.h"
#include "content_type.h"

// #ifdef SSL
#include "ssl.h"
// #endif

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

typedef union
{
   char *file_path;
   char *content;
} response_resource_u;

typedef struct http_response_s
{
   response_resource_u resource;
   ll_node_t *headers;
   int content_length;
   content_type_e content_type;
   http_status_e status;
} http_response_t;

typedef char *(*resource_function)(http_request_t *);

void sigint_handler(int code);
void construct_response(int client_socket, http_request_t *http_request);
int http_request_parse_request_line(http_request_t *http_request, char **request_ptr);
int http_request_parse_headers(http_request_t *http_request, char **request_ptr);
int http_request_parse_body(http_request_t *http_request, char **request_ptr);
void *http_request_write_log_wrapper(void *http_request);
void accept_connection(void);
void start_server(int port);

#endif // HTTP_H
