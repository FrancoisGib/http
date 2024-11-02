#ifndef HTTP_TREE_H
#define HTTP_TREE_H

#include "tree.h"
#include "lib.h"
#include "http.h"
#include "http_status.h"
#include "content_type.h"

typedef union
{
   char *(*function)(void *);
   char *content;
} resource_t;

typedef enum
{
   ET_FILE,
   ET_DIRECTORY,
   ET_TEXT,
   ET_PATH,
   ET_FUNC
} endpoint_type_e;

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
content_type_e get_content_type_with_file_extension(char *path);
endpoint_t *get_endpoint(tree_t *tree, char *path);
void add_endpoint(tree_t *tree, const char *path, response_t response);
void print_http_tree(tree_t *tree, int depth);
void free_http_tree(tree_t *tree);
tree_t *init_http_tree(void);
tree_t *build_http_tree(const endpoint_t endpoints[], int n);

#endif // HTTP_TREE_H