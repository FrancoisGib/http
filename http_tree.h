#include "tree.h"

typedef enum
{
   ET_FILE,
   ET_DIRECTORY,
   ET_TEXT,
   ET_PATH,
   ET_FUNC
} endpoint_type_t;

typedef enum
{
   JSON,
   TEXT,
   HTML,
   NULL_CONTENT
} content_type_t;

typedef struct
{
   char *path;
   void *resource;
   endpoint_type_t type;
   content_type_t content_type;
} endpoint_t;

char *print_endpoint_type(endpoint_type_t type);
char *print_content_type(content_type_t content_type);
endpoint_t *get_endpoint(tree_t *tree, char *path);
void add_endpoint(tree_t *tree, char *path, void *resource, endpoint_type_t type, content_type_t content_type);
void print_http_tree(tree_t *tree, int depth);
void free_http_tree(tree_t *tree);
tree_t *init_http_tree(void *resource, endpoint_type_t type, content_type_t content_type);
tree_t *build_http_tree(endpoint_t endpoints[], int n);