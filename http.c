#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include "http_tree.h"

typedef struct sockaddr_in SOCKADDR_IN;

#define MAX_REQUEST_SIZE 2048
#define NB_THREADS 10

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

typedef char *(*resource_function)(void *);

char *test_function(void *content)
{
    return "<p>ça marche</p>";
}

void construct_response(tree_t *http_tree, int sock, char *method, char *path, char *content, char *content_type)
{
    endpoint_t *endpoint = get_endpoint(http_tree, path);
    if (endpoint != NULL)
    {
        // printf("endpoint %s %s %s %s\n", endpoint->path, print_endpoint_type(endpoint->type), endpoint->resource, print_content_type(endpoint->content_type));
        char response[1256] =
            "HTTP/1.1 200 OK\r\n";

        int content_length;
        char buffer[1024];
        memset(buffer, 0, 1024);
        if (endpoint->type == ET_FILE)
        {
            printf("resource %s\n", (char *)endpoint->resource);
            if (access(endpoint->resource, R_OK) == 0)
            {
                FILE *file = fopen(endpoint->resource, "r");
                fread(buffer, 1024, 1, file);
                content_length = strlen(buffer);
            }
            else
            {
                char message[] = "Error";
                strcpy(buffer, message);
                content_length = strlen(message);
            }
        }
        else if (endpoint->type == ET_TEXT)
        {
            strcpy(buffer, endpoint->resource);
            content_length = strlen(endpoint->resource);
        }
        else if (endpoint->type == ET_FUNC)
        {
            resource_function function = endpoint->resource;
            char *response_content = function((void *)content);
            content_length = strlen(response_content);
            strcpy(buffer, response_content);
        }

        char content_length_buffer[5];
        sprintf(content_length_buffer, "%d", content_length);

        char content_length_str[32] = "Content-Length: ";
        strcat(content_length_str, content_length_buffer);
        strcat(content_length_str, "\r\n");

        char content_type_str[64] = "Content-Type: ";
        strcat(content_type_str, print_content_type(endpoint->content_type));
        strcat(content_type_str, "\r\n\r\n");

        strcat(response, content_length_str);
        strcat(response, content_type_str);

        strcat(response, buffer);

        printf("-----------------\n%s\n-----------------\n", response);

        write(sock, response, strlen(response));
    }
    else
    {
        printf("NULL\n");
    }
}

void parse_http_request(char *request, tree_t *http_tree, int sock)
{
    char *rest = request;
    char *method = strtok_r(rest, " ", &rest);
    char *path = strtok_r(rest, " ", &rest);
    char *http_version = strtok_r(rest, "\n", &rest);
    strtok_r(rest, " ", &rest);
    char *host = strtok_r(rest, "\n", &rest);
    strtok_r(rest, " ", &rest);
    char *user_agent = strtok_r(rest, "\n", &rest);
    strtok_r(rest, " ", &rest);
    char *accept = strtok_r(rest, "\n", &rest);

    printf("METHOD: %s\n", method);
    printf("PATH: %s\n", path);
    printf("HTTP VERSION: %s\n", http_version);
    printf("HOST: %s\n", host);
    printf("USER AGENT: %s\n", user_agent);
    printf("ACCEPT: %s\n", accept);

    ll_node_t *headers = NULL;
    char *header = rest;
    while (*header != '\0')
    {
        char *header = strtok_r(rest, "\n", &rest);
        if (*header == '\r' || *(header + 1) == '\r')
            break;
        else
        {
            char *header_name = strtok_r(header, ":", &header); // header is now the header value when tok
            header++;
            char *header_value = header;
            header_t *new_header = malloc(sizeof(header_t));
            new_header->name = header_name;
            new_header->value = header_value;
            headers = insert_in_head(headers, (void *)new_header);
        }
    }

    printf("Headers: [\n");
    ll_node_t *current_header = headers;
    int content_length = -1;
    char *content_type = NULL;
    while (current_header != NULL)
    {
        header_t *new_header = (header_t *)current_header->element;
        printf(" %s: %s\n", (char *)new_header->name, (char *)new_header->value);
        if (strcmp("Content-Length", new_header->name) == 0)
        {
            content_length = atoi(new_header->value);
        }
        else if (strcmp("Content-Type", new_header->name) == 0)
        {
            content_type = new_header->value;
        }
        current_header = current_header->next;
    }
    printf("]\n");
    free_linked_list(headers, (ll_map_function_t)free_with_arg);

    if (content_length > 0)
    {
        char content[content_length + 1];
        content[content_length] = '\0';
        memcpy(content, rest, content_length);
        printf("Content: %s\n", content);
        construct_response(http_tree, sock, method, path, content, content_type);
    }
    else
    {
        construct_response(http_tree, sock, method, path, NULL, content_type);
    }
}

void *accept_connection(void *params)
{
    thread_params_t thread_params = *(thread_params_t *)params;
    if (listen(thread_params.sock, 1) != 0)
    {
        printf("Error listening on socket\n");
        return NULL;
    }
    printf("Thread %d listening for requests\n", thread_params.index);
    SOCKADDR_IN client_address;
    unsigned int client_address_len = sizeof(client_address);
    int client_fd;
    while (1)
    {
        if ((client_fd = accept(thread_params.sock, (struct sockaddr *)&client_address, &client_address_len)) == -1)
        {
            return NULL;
        }
        printf("Accepted request on thread %d\n\n", thread_params.index);
        char buffer[MAX_REQUEST_SIZE];
        int size;
        if ((size = read(client_fd, buffer, MAX_REQUEST_SIZE)) != -1)
        {
            // printf("%.*s", size, buffer);
            parse_http_request(buffer, thread_params.endpoints, client_fd);
            memset(buffer, 0, size);
        }
        close(client_fd);
    }
}

int start_server(tree_t *http_tree, int port, int nb_threads)
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error creating socket\n");
        return -1;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printf("Error setting socket options\n");
        close(sock);
        return -1;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) != 0)
    {
        printf("Error binding socket\n");
        close(sock);
        return -1;
    }

    pthread_t threads[nb_threads];
    for (int i = 0; i < nb_threads; i++)
    {
        thread_params_t params = {i, sock, http_tree};
        pthread_create(&threads[i], NULL, accept_connection, &params);
        usleep(100); // for thread index synch
    }
    for (int i = 0; i < nb_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    close(sock);
    return 0;
}

int main(int argc, char **argv)
{

    int port;
    int nb_threads = NB_THREADS;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    else
    {
        printf("Please enter a port number");
        exit(-1);
    }
    if (argc > 2)
    {
        nb_threads = atoi(argv[2]);
    }

    char hostname_resource[32] = "<p>";
    char hostname[_SC_HOST_NAME_MAX + 1];
    gethostname(hostname, _SC_HOST_NAME_MAX + 1);
    strcat(hostname_resource, hostname);
    strcat(hostname_resource, "</p>");

    endpoint_t endpoints[] = {
        {"/", hostname_resource, ET_TEXT, HTML},
        {"/home", "src/index.html", ET_FILE, HTML},
        {"/test", test_function, ET_FUNC, HTML}};

    tree_t *http_tree = build_http_tree(endpoints, sizeof(endpoints) / sizeof(endpoint_t));
    start_server(http_tree, port, nb_threads);
    return 0;
}