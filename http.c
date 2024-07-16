#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct sockaddr_in SOCKADDR_IN;

#define str(x) #x

#define PORT 3000
#define MAX_REQUEST_SIZE 2048
#define NB_THREADS 10

typedef enum {GET, POST, PUT, PATCH, DELETE} METHOD;

typedef struct {
    char* name;
    char* value;
} HEADER;

typedef struct {
    METHOD method;
    HEADER* headers;
    char* body;
} request_t;

void* accept_connection(void* sock) {
    if (listen(*(int*)sock, 1) != 0) {
        printf("Error listening");
        return NULL;
    }
    SOCKADDR_IN client_address;
    unsigned int client_address_len = sizeof(client_address);
    int client_fd;
    while (1) {
        if ((client_fd = accept(*(int*)sock, (struct sockaddr*)&client_address, &client_address_len)) == -1) {
            return NULL;
        }
        char buffer[MAX_REQUEST_SIZE];
        int size;
        if ((size = read(client_fd, buffer, MAX_REQUEST_SIZE)) != -1) {
            printf("%.*s\n", size, buffer);
            memset(buffer, 0, size);
        }
        const char *response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Set-Cookie: ok=value\r\n"
                "Content-Length: 2\r\n"
                "\r\n"
                "ok";
        write(client_fd, response, strlen(response));
        close(client_fd);
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Error creating socket");
        exit(-1);
    }
    SOCKADDR_IN sin;
    sin.sin_family= AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) != 0) {
        printf("Error binding socket");
        exit(-1);
    }

    pthread_t threads[NB_THREADS];
    int i;
    for (i = 0; i < NB_THREADS; i++){
        pthread_create(&threads[i], NULL, accept_connection, &sock);
        i++;
    }
    for (i = 0; i < NB_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    close(sock);
}