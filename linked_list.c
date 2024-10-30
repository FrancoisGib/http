#include "linked_list.h"

ll_node_t *insert_in_head(ll_node_t *root, void *element)
{
   ll_node_t *new_node = malloc(sizeof(ll_node_t));
   if (new_node == NULL)
   {
      return NULL;
   }
   if (root == NULL)
   {
      new_node->element = element;
      new_node->next = NULL;
      return new_node;
   }
   else
   {
      new_node->element = root->element;
      root->element = element;
      new_node->next = root->next;
      root->next = new_node;
      return root;
   }
}

ll_node_t *insert_in_tail(ll_node_t *root, void *element)
{
   ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
   if (new_node == NULL)
   {
      return NULL;
   }
   new_node->element = element;
   new_node->next = NULL;

   if (root == NULL)
   {
      return new_node;
   }

   ll_node_t *node = root;
   while (node->next != NULL)
   {
      node = node->next;
   }
   node->next = new_node;
   return root;
}

void map_linked_list(ll_node_t *root, ll_map_function_t map_function)
{
   ll_node_t *node;
   while ((node = root) != NULL)
   {
      root = root->next;
      if (map_function != NULL)
         map_function(node);
   }
}

void free_linked_list(ll_node_t *root, ll_map_function_t free_function)
{
   map_linked_list(root, (ll_map_function_t)free_function);
}

void free_with_arg(ll_node_t *node)
{
   free(node->element);
   free(node);
}

void print_node_string(ll_node_t *node)
{
   printf("%s ", (char *)node->element);
}

#ifdef LINKED_LIST_MAIN

int main()
{
   char salut[] = "salut";
   char salut2[] = "salut2";
   ll_node_t *root = insert_in_head(NULL, salut);
   insert_in_tail(root, salut2);
   map_linked_list(root, (ll_map_function_t)print_node_string);
   free_linked_list(root, (ll_map_function_t)free);
   return 0;
}

#endif