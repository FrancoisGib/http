#include "http_tree.h"

const char *print_endpoint_type(endpoint_type_t type)
{
   switch (type)
   {
   case ET_FILE:
      return "FILE";
   case ET_DIRECTORY:
      return "DIRECTORY";
   case ET_PATH:
      return "PATH";
   case ET_TEXT:
      return "TEXT";
   case ET_FUNC:
      return "FUNC";
   default:
      return "";
   }
}

const char *print_content_type(content_type_t content_type)
{
   switch (content_type)
   {
   case JSON:
      return "application/json";
   case HTML:
      return "text/html";
   case TEXT:
      return "text/plain";
   case NULL_CONTENT:
      return "";
   default:
      return "";
   }
}

endpoint_t *get_endpoint(tree_t *tree, char *path)
{
   if (*path == '/') /* delete the / at the beginning */
   {
      path++;
   }
   endpoint_t *tree_endpoint = (endpoint_t *)tree->element;
   if (strlen(path) == 0 || strcmp("/", path) == 0)
   {
      return tree_endpoint;
   }

   char *rest = path;
   char *first_part = strtok_r(rest, "/", &rest);
   if (strcmp(first_part, tree_endpoint->path) == 0 && *rest == '\0')
   {
      return tree_endpoint;
   }

   ll_node_t *children = tree->children;
   tree_t *child;
   while (children != NULL)
   {
      child = children->element;
      endpoint_t *child_endpoint = (endpoint_t *)child->element;
      if (strncmp(child_endpoint->path, path, strlen(child_endpoint->path)) == 0 && child_endpoint->type == ET_DIRECTORY)
      {
         return child_endpoint;
      }
      if (strcmp(child_endpoint->path, first_part) == 0)
      {
         if (*rest == '\0')
         {
            return child_endpoint;
         }
         return get_endpoint(child, rest);
      }
      children = children->next;
   }
   return NULL;
}

void add_endpoint(tree_t *tree, const char *path, const void *resource, endpoint_type_t type, content_type_t content_type)
{
   endpoint_t *tree_endpoint = (endpoint_t *)tree->element;
   if (*path == '/') /* delete the / at the beginning */
   {
      return add_endpoint(tree, &path[1], resource, type, content_type);
   }
   if ((strlen(path) == 0 || strcmp("/", path) == 0) && (strcmp("/", tree_endpoint->path) == 0 || strcmp("", tree_endpoint->path) == 0)) // for initialization (if overriding / path)
   {
      tree_endpoint->resource = resource;
      tree_endpoint->type = type;
      tree_endpoint->content_type = content_type;
      return;
   }

   char *rest = path;
   char *first_part = strtok_r(rest, "/", &rest);

   if (strcmp(first_part, tree_endpoint->path) == 0 && *rest == '\0')
   {
      tree_endpoint->content_type = content_type;
      tree_endpoint->type = type;
      tree_endpoint->resource = resource;
   }

   ll_node_t *children = tree->children;
   tree_t *child;
   int found = 0;
   while (children != NULL && !found)
   {
      child = children->element;
      endpoint_t *child_endpoint = (endpoint_t *)child->element;
      if (strcmp(child_endpoint->path, first_part) == 0)
      {
         found = 1;
      }
      else
      {
         children = children->next;
      }
   }

   if (first_part != NULL)
   {
      if (found)
      {
         if (*rest != '\0')
         {
            add_endpoint(child, rest, resource, type, content_type);
         }
      }
      else
      {
         endpoint_t *endpoint = malloc(sizeof(endpoint_t));
         endpoint->path = first_part;
         child = add_child(tree, (void *)endpoint);
         if (*rest != '\0')
         {
            endpoint->type = ET_PATH;
            endpoint->resource = "";
            endpoint->content_type = NULL_CONTENT;
            add_endpoint(child, rest, resource, type, content_type);
         }
         else
         {
            endpoint->type = type;
            endpoint->content_type = content_type;
            endpoint->resource = resource;
         }
      }
   }
}

void print_http_tree(tree_t *tree, int depth)
{
   endpoint_t *tree_endpoint = (endpoint_t *)tree->element;
   for (int i = 0; i < depth * 2; i++)
   {
      printf(" ");
   }
   printf("/%s %s %s\n", tree_endpoint->path, print_endpoint_type(tree_endpoint->type), print_content_type(tree_endpoint->content_type));
   ll_node_t *children = tree->children;
   while (children != NULL)
   {
      print_http_tree((tree_t *)children->element, depth + 1);
      children = children->next;
   }
}

void free_http_tree(tree_t *tree)
{
   if (tree != NULL)
   {
      ll_node_t *child_node = tree->children;
      while (child_node != NULL)
      {
         ll_node_t *next_child = child_node->next;
         tree_t *child_tree = (tree_t *)child_node->element;
         free_http_tree(child_tree);
         free(child_node);
         child_node = next_child;
      }
      endpoint_t *endpoint = (endpoint_t *)tree->element;
      free(endpoint);
      free(tree);
   }
}

tree_t *init_http_tree(void *resource, endpoint_type_t type, content_type_t content_type)
{
   endpoint_t *endpoint = malloc(sizeof(endpoint_t));
   endpoint->path = "";
   endpoint->type = type;
   endpoint->resource = resource;
   endpoint->content_type = content_type;
   return init_tree((void *)endpoint);
}

tree_t *build_http_tree(const endpoint_t endpoints[], int n)
{
   tree_t *tree = init_http_tree(NULL, ET_PATH, NULL_CONTENT);
   for (int i = 0; i < n; i++)
   {
      endpoint_t endpoint = endpoints[i];
      add_endpoint(tree, endpoint.path, endpoint.resource, endpoint.type, endpoint.content_type);
   }
   return tree;
}