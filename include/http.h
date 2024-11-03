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
#include "parsing.h"

#ifdef USE_SSL
#include "ssl.h"
#endif

typedef struct
{
   http_request_t *request;
   http_response_t *response;
} http_req_res_t;

typedef struct sockaddr_in sockaddr_in_t;

void sigint_handler(int code);
void construct_response(int client_socket, http_req_res_t *http_req_res);
void *http_req_res_write_log_and_free(void *arg);
void free_http_request(http_request_t *http_request);
void free_http_response(http_response_t *http_response);
void accept_connection(void);
void start_server(int port);

#endif // HTTP_H
