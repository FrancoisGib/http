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
#include "http_structs.h"

#ifdef USE_SSL
#include "ssl.h"
#endif

typedef struct sockaddr_in sockaddr_in_t;

void sigint_handler(int code);
void construct_response(int client_socket, http_request_t *http_request);
int http_request_parse_request_line(http_request_t *http_request, char **request_ptr);
int http_request_parse_headers(http_request_t *http_request, char **request_ptr);
int http_request_parse_body(http_request_t *http_request, char **request_ptr);
void *http_request_write_log_wrapper(void *http_request);
void accept_connection(void);
void start_server(int port);

#endif // HTTP_H
