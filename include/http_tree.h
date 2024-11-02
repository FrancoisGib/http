#ifndef HTTP_TREE_H
#define HTTP_TREE_H

#include "tree.h"

typedef enum
{
   ET_FILE,
   ET_DIRECTORY,
   ET_TEXT,
   ET_PATH,
   ET_FUNC
} endpoint_type_e;

typedef enum
{
   HTTP_STATUS_OK = 200,
   HTTP_STATUS_CREATED = 201,
   HTTP_STATUS_NO_CONTENT = 204,
   HTTP_STATUS_BAD_REQUEST = 400,
   HTTP_STATUS_UNAUTHORIZED = 401,
   HTTP_STATUS_FORBIDDEN = 403,
   HTTP_STATUS_NOT_FOUND = 404
} http_status_e;

typedef enum
{
   JSON,
   JAVASCRIPT,
   TEXT,
   HTML,
   NULL_CONTENT
} content_type_e;

typedef union
{
   char *(*function)(char *);
   char *content;
} resource_t;

typedef struct
{
   resource_t resource;
   endpoint_type_e type;
   content_type_e content_type;
   http_status_e status;
} response_t;

typedef struct
{
   const char *path;
   response_t response;
} endpoint_t;

const char *get_endpoint_type(endpoint_type_e type);
const char *get_content_type(content_type_e content_type);
const char *get_content_type_with_file_extension(char *path);
endpoint_t *get_endpoint(tree_t *tree, char *path);
void add_endpoint(tree_t *tree, const char *path, response_t response);
void print_http_tree(tree_t *tree, int depth);
void free_http_tree(tree_t *tree);
tree_t *init_http_tree(void);
tree_t *build_http_tree(const endpoint_t endpoints[], int n);

#endif // HTTP_TREE_H