#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {GET, POST, PUT, PATCH, DELETE, HEAD, OPTIONS} METHOD;

typedef struct {
    METHOD method;
    char* path;
} endpoint;

struct routes_node {
    char* path;
    int children_nb;
    struct routes_node* children;
    endpoint* endpoints;
};

typedef struct routes_node routes_node_t;

routes_node_t* init_routes_node(char* path) {
    routes_node_t* node = (routes_node_t*)malloc(sizeof(routes_node_t));
    node->children_nb = 0;
    unsigned int path_length = strlen(path);
    node->path = malloc(path_length);
    memcpy(node->path, path, path_length);
    return node;
}

void free_routes_node(routes_node_t* routes_node) {
    for (int i = 0; i < routes_node->children_nb; i++) {
        free_routes_node(&routes_node->children[i]);
    }
    free(routes_node->children);
    free(routes_node->endpoints);
    free(routes_node->path);
    free(routes_node);
}

void add_route(routes_node_t* parent, routes_node_t* added_route) {
    parent->children = (routes_node_t*)realloc(parent->children, ++parent->children_nb);

}

int main() {

    return 0;
}
