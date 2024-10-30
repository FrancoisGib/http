#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "http_tree.h"

typedef struct sockaddr_in SOCKADDR_IN;

#define MAX_RESPONSE_SIZE 2048
#define MAX_REQUEST_SIZE 2048
#define NB_PROCESSES 10

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

typedef const char *(*resource_function)(void *);