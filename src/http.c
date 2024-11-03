#include "http.h"

int sock = -1;
int nb_processes = NB_PROCESSES;
tree_t *http_tree;
endpoint_t *error_endpoint;
pid_t parent_pid;

#ifdef USE_SSL
#define SOCK ssl

SSL_CTX *ctx = NULL;
SSL *ssl = NULL;

int ssl_read(SSL *s, char buffer[MAX_REQUEST_SIZE], int buf_size)
{
   if (s == NULL)
   {
      return -1;
   }

   if (SSL_accept(s) <= 0)
   {
      ERR_print_errors_fp(stderr);
      return -1;
   }
   else
   {
      int size = 0;
      if ((size = SSL_read(s, buffer, buf_size)) <= 0)
      {
         ERR_print_errors_fp(stderr);
         return -1;
      };
      return size;
   }
}

#define READ ssl_read
#define WRITE SSL_write

#else
#define SOCK client_socket

#define READ read
#define WRITE write
#endif

void sigint_handler(int code)
{
   printf("Process %d ended\n", getpid());
#ifdef USE_SSL
   SSL_CTX_free(ctx);
#endif
   if (sock != -1)
   {
      close(sock);
   }
   if (http_tree != NULL)
   {
      free_http_tree(http_tree);
   }
   if (getpid() == parent_pid)
   {
#ifdef USE_SSL
      printf("\nFreed SSL context");
#endif
      printf("\nClosed socket");
      printf("\nFreed http tree\n");
   }
   exit(0);
}

int http_response_build_type_et_file(http_response_t *http_response, endpoint_t *endpoint)
{
   stat_t st;
   if (stat(endpoint->response.resource.content, &st) == -1)
   {
      return -1;
   }
   else
   {
      http_response->resource.file_path = endpoint->response.resource.content;
      http_response->content_type = get_content_type_with_file_extension(http_response->resource.file_path);
      http_response->content_length = (int)st.st_size;
      return 0;
   }
}

void http_response_build_type_et_text(http_response_t *http_response, endpoint_t *endpoint, char response_buffer[MAX_RESPONSE_SIZE])
{
   strcpy(response_buffer, endpoint->response.resource.content);
   http_response->content_length = strlen(endpoint->response.resource.content);
}

void http_response_build_type_et_func(http_response_t *http_response, http_request_t *http_request, endpoint_t *endpoint, char response_buffer[MAX_RESPONSE_SIZE])
{
   resource_function function = endpoint->response.resource.function;
   char *response_content = function(http_request);
   http_response->content_length = strlen(response_content);
   strcpy(response_buffer, response_content);
   free(response_content);
}

int http_response_build_type_et_directory(http_response_t *http_response, http_request_t *http_request, endpoint_t *endpoint)
{
   char *path_ptr = http_request->path;
   int size = strlen(path_ptr);
   while (size > 0 && strcmp(path_ptr, endpoint->path) != 0)
   {
      path_ptr++;
      size--;
   }
   path_ptr += strlen(endpoint->path) + 1;
   if (*path_ptr != '\0')
   {
      char *file_path = malloc(strlen(path_ptr) + strlen(endpoint->response.resource.content) + 2); // pass the \0 made by get_endpoint
                                                                                                    // add 1 more if there's no / at the end of the directory path
      strcpy(file_path, endpoint->response.resource.content);
      int file_path_size = (int)strlen(endpoint->response.resource.content);
      file_path[file_path_size] = '/'; // always / at the end
      file_path[file_path_size + 1] = '\0';
      strcat(file_path, path_ptr);
      http_response->resource.file_path = file_path;
      http_response->content_type = get_content_type_with_file_extension(http_response->resource.file_path);
      stat_t st;
      stat(file_path, &st);
      if (stat(file_path, &st) != -1)
      {
         http_response->content_length = (int)st.st_size;
         return 0;
      }
      free(file_path);
   }
   return -1;
}

void http_response_send_et_file(http_response_t *http_response, char response_buffer[MAX_RESPONSE_SIZE], int client_socket)
{
   int response_length = strlen(response_buffer);
   if (http_response->content_type == IMAGE_X_ICON || http_response->content_type == IMAGE_PNG)
   {
      char buf[http_response->content_length + 1];
      memset(buf, 0, http_response->content_length);
      int read_size;
      if (*http_response->resource.file_path == '\0')
      {
         printf("Resource path error\n");
         return;
      }
      if ((read_size = read_file(buf, http_response->resource.file_path)) != -1)
      {
         strncat(response_buffer, buf, read_size);
         int written_size = WRITE(SOCK, buf, response_length + read_size);
         if (written_size != response_length + read_size)
         {
            printf("Error sending file\n");
         }
      }
      else
      {
         printf("Error reading file\n");
      }
   }
   else
   {
      if (response_length + http_response->content_length < MAX_RESPONSE_SIZE && http_response->content_length <= MAX_FILE_READ_SIZE)
      {
         if (*http_response->resource.file_path == '\0')
         {
            printf("Resource path error\n");
            return;
         }
         char buf[http_response->content_length];
         memset(buf, 0, http_response->content_length);
         int read_size;
         if ((read_size = read_file(buf, http_response->resource.file_path)) != -1)
         {
            strncat(response_buffer, buf, read_size);
            int written_size = WRITE(SOCK, response_buffer, response_length + read_size);
            if (written_size != response_length + read_size)
            {
               printf("Error sending file\n");
            }
         }
         else
         {
            printf("Error reading file\n");
         }
      }
      else
      {
         int written_size = WRITE(SOCK, response_buffer, response_length);
         if (written_size != response_length)
         {
            printf("Error sending file\n");
         }
         char buf[MAX_FILE_READ_SIZE];
         memset(buf, 0, MAX_FILE_READ_SIZE);
         int read_size;
         int fd = open(http_response->resource.file_path, O_RDONLY);
         if (fd != -1)
         {
            int block = 0;
            while ((read_size = read_file_block(buf, fd, block)) > 0)
            {
               written_size += WRITE(SOCK, buf, read_size);
               block++;
               memset(buf, 0, read_size);
            }
            close(fd);
            if (written_size != http_response->content_length + response_length)
            {
               printf("Error splitting response data into chunks\n");
            }
         }
      }
   }
}

void http_response_build_by_type(http_response_t *http_response, endpoint_t *endpoint, char response_content_buffer[MAX_RESPONSE_SIZE], http_request_t *http_request)
{
   http_response->status = endpoint->response.status;
   http_response->content_type = endpoint->response.content_type;
   http_response->endpoint_type = endpoint->response.type;
   int res = 0;
   switch (endpoint->response.type)
   {
   case ET_FILE:
      res = http_response_build_type_et_file(http_response, endpoint);
      break;

   case ET_TEXT:
      http_response_build_type_et_text(http_response, endpoint, response_content_buffer);
      break;

   case ET_FUNC:
      http_response_build_type_et_func(http_response, http_request, endpoint, response_content_buffer);
      break;

   case ET_DIRECTORY:
      res = http_response_build_type_et_directory(http_response, http_request, endpoint);
      break;

   case ET_PATH:
      res = -1;
   }

   if (res == -1)
   {
      http_response_build_by_type(http_response, error_endpoint, response_content_buffer, http_request);
   }
}

void construct_response(int client_socket, http_request_t *http_request)
{
   endpoint_t *endpoint = get_endpoint(http_tree, http_request->path);
   if (endpoint == NULL)
   {
      endpoint = error_endpoint;
   }

   http_response_t http_response;
   http_response.headers = NULL;
   char response_content_buffer[MAX_RESPONSE_SIZE];
   memset(response_content_buffer, 0, MAX_RESPONSE_SIZE);
   http_response_build_by_type(&http_response, endpoint, response_content_buffer, http_request);

   char response_buffer[MAX_RESPONSE_SIZE] = "HTTP/1.1 ";

   char status_code[6];
   sprintf(status_code, "%d\r\n", http_response.status);
   strcat(response_buffer, status_code);

   // Uncomment to copy all the request headers into the response
   // ll_node_t *header_node = http_request->headers;
   // while (header_node != NULL)
   // {
   //    header_t *header = header_node->element;
   //    strcat(response_buffer, header->name);
   //    strcat(response_buffer, ": ");
   //    strcat(response_buffer, header->value);
   //    strcat(response_buffer, "\r\n");
   //    header_node = header_node->next;
   // }

   if (http_response.content_length > 0)
   {
      char *content_type = get_content_type(http_response.content_type);
      strcat(response_buffer, "Content-Type: ");
      strcat(response_buffer, content_type);
      strcat(response_buffer, "\r\n");

      char content_length_buffer[11];
      sprintf(content_length_buffer, "%d", http_response.content_length);
      strcat(response_buffer, "Content-Length: ");
      strcat(response_buffer, content_length_buffer);
      strcat(response_buffer, "\r\n");
   }

   strcat(response_buffer, "\r\n");

   printf("-----------------\n%s\n-----------------\n", response_buffer);

   int written_size;
   int response_length = strlen(response_buffer);
   switch (http_response.endpoint_type)
   {
   case ET_FILE:
      http_response_send_et_file(&http_response, response_buffer, client_socket);
      break;

   case ET_DIRECTORY:
      http_response_send_et_file(&http_response, response_buffer, client_socket);
      free(http_response.resource.file_path);
      break;

   case ET_TEXT:
      strcat(response_buffer, response_content_buffer);
      written_size = WRITE(SOCK, response_buffer, response_length + http_response.content_length);
      if (written_size != response_length)
      {
         printf("Error sending file\n");
      }
      break;

   case ET_FUNC:
      strcat(response_buffer, response_content_buffer);
      written_size = WRITE(SOCK, response_buffer, response_length + http_response.content_length);
      if (written_size != response_length)
      {
         printf("Error sending content\n");
      }
      break;

   default:
      break;
   }
   free_http_response(&http_response);
}

int http_request_parse_request_line(http_request_t *http_request, char **request_ptr)
{
   char *request = *request_ptr;
   http_request->method = strtok_r(request, " ", &request);
   if (http_request->method == NULL)
   {
      printf("Invalid request received");
      return -1;
   }
   else
   {
      http_request->method = mstrdup(http_request->method);
   }

   http_request->path = strtok_r(request, " ", &request);
   if (http_request->path == NULL)
   {
      printf("Invalid request received\n");
      return -1;
   }
   else
   {
      http_request->path = mstrdup(http_request->path);
   }

   http_request->http_version = strtok_r(request, "\n", &request);
   if (http_request->http_version == NULL)
   {
      printf("Invalid request received\n");
      return -1;
   }
   else
   {
      http_request->http_version = mstrdup(http_request->http_version);
   }
   *request_ptr = request;
   return 0;
}

int http_request_parse_headers(http_request_t *http_request, char **request_ptr)
{
   char *request = *request_ptr;
   int request_size = strlen(request);

   if (request_size == 0)
   {
      printf("Empty request\n");
      return 1;
   }

   char *header_end = strstr(request, "\r\n\r\n");
   if (!header_end)
   {
      printf("Invalid request\n");
      return -1;
   }

   *header_end = '\0';
   char *body = header_end + 4; // Body pointer after "\r\n\r\n"

   char *header_str;
   while ((header_str = strtok_r(request, "\r\n", &request)) != NULL)
   {
      header_t *header = malloc(sizeof(header_t));
      char *header_name = strtok_r(header_str, ":", &header_str);
      if (header_name && *header_str == ' ')
      {
         header_str++;
      }
      header->name = mstrdup(header_name);
      header->value = mstrdup(header_str);
      if (strcmp(header_name, "Content-Length") == 0)
      {
         http_request->content_length = (int)strtol(header->value, NULL, 10);
      }
      else if (strcmp(header_name, "Referer") == 0)
      {
         char *referee_path = strchr(header->value, '/');
         referee_path += 2;
         referee_path = strchr(referee_path, '/');
         referee_path++;
         char *referer_directory_end_pointer = search_last_occurence(referee_path, '/');
         int diff_size = referer_directory_end_pointer - referee_path;
         if (referee_path != NULL && diff_size > 0)
         {
            char *new_path = malloc(diff_size + strlen(http_request->path) + 1);
            memset(new_path, 0, diff_size + strlen(http_request->path) + 1);
            strncpy(new_path, referee_path, diff_size);
            new_path[diff_size] = '\0';
            if (strncmp(new_path, http_request->path + 1, strlen(new_path)) == 0)
            {
               strcat(new_path, http_request->path + strlen(new_path) + 1);
            }
            else
            {
               strcat(new_path, http_request->path);
            }
            free(http_request->path);
            http_request->path = new_path;
         }
      }
      http_request->headers = insert_in_head(http_request->headers, header);
      http_request->headers_length += strlen(header->name) + strlen(header->value) + 4; // +1 to add \n\t and ": " later
   }
   *request_ptr = body;
   return *body != '\0'; // 1 if this is not the end of the headers part, 0 if this is the body or empty.
}

int http_request_parse_body(http_request_t *http_request, char **request_ptr)
{
   char *request = *request_ptr;
   int request_size = strlen(request);
   int added_body_length = MIN(request_size, http_request->content_length);
   if (http_request->body == NULL)
   {
      http_request->body = malloc(http_request->content_length + 1);
      strncpy(http_request->body, request, added_body_length);
      http_request->body[added_body_length] = '\0';
   }
   else
   {
      int body_size = strlen(http_request->body);
      int remaining_body_size = http_request->content_length - body_size;
      strncat(&http_request->body[body_size], request, remaining_body_size);
   }
   return added_body_length == http_request->content_length;
}

void free_http_request(http_request_t *request)
{
   if (request->content_length > 0)
   {
      free(request->body);
   }
   ll_node_t *header_node;
   while ((header_node = request->headers) != NULL)
   {
      request->headers = request->headers->next;
      header_t *header = (header_t *)header_node->element;
      free(header->name);
      free(header->value);
      free(header);
      free(header_node);
   }
   free(request->path);
   free(request->method);
   free(request->http_version);
}

void free_http_response(http_response_t *http_response)
{
   ll_node_t *header_node;
   while ((header_node = http_response->headers) != NULL)
   {
      http_response->headers = http_response->headers->next;
      header_t *header = (header_t *)header_node->element;
      free(header->name);
      free(header->value);
      free(header);
      free(header_node);
   }
}

void accept_connection(void)
{
   sockaddr_in_t client_address;
   unsigned int client_address_len = sizeof(sockaddr_in_t);
   int client_socket;

   while (1)
   {
      client_socket = accept(sock, (struct sockaddr *)&client_address, &client_address_len);
      if (client_socket == -1)
      {
         perror("Error accepting connection");
         exit(EXIT_FAILURE);
      }
#ifdef USE_SSL
      ssl = SSL_new(ctx);
      if (ssl == NULL || SSL_set_fd(ssl, client_socket) == 0)
      {
         ERR_print_errors_fp(stderr);
         close(client_socket);
         continue; // Move to the next connection if SSL setup fails
      }

      if (SSL_accept(ssl) <= 0)
      {
         ERR_print_errors_fp(stderr); // Output SSL errors
         SSL_free(ssl);
         ssl = NULL;
         close(client_socket);
         continue;
      }
#endif
      char buffer[MAX_REQUEST_SIZE];
      http_request_t http_request;
      int is_first_request = 1;
      int is_incorrect_request = 0;
      int header_parsed = 0;
      int done = 0;
      int size;
      while (!done && !is_incorrect_request && (size = READ(SOCK, buffer, MAX_REQUEST_SIZE)) > 0)
      {
         buffer[size] = '\0';
         char *buffer_ptr = buffer;
         http_request.path = NULL;
         http_request.method = NULL;
         http_request.http_version = NULL;
         http_request.body = NULL;
         http_request.headers = NULL;
         http_request.content_length = 0;
         http_request.headers_length = 0;
         char **request_ptr = &buffer_ptr;
         printf("%s\n", buffer);
         if (is_first_request)
         {
            if (http_request_parse_request_line(&http_request, request_ptr) == -1)
            {
               is_incorrect_request = 1;
               break;
            }
            is_first_request = 0;
         }

         int header_parsing_status = http_request_parse_headers(&http_request, request_ptr);
         if (!header_parsed && header_parsing_status == 1)
         {
            header_parsed = 1;
         }
         else if (header_parsing_status == -1)
         {
            is_incorrect_request = 1;
            break;
         }

         if (http_request.content_length > 0)
         {
            done = http_request_parse_body(&http_request, request_ptr);
         }
         else
         {
            done = 1;
         }
         memset(buffer, 0, size);
      }
      // pthread_t log_thread;
      // pthread_create(&log_thread, NULL, http_request_write_log_wrapper, (void *)&http_request);
      // pthread_detach(log_thread);
      if (size == -1 || is_incorrect_request)
      {
         printf("Error with the request\n");
      }
      else
      {
         construct_response(client_socket, &http_request);
         http_request_write_log(&http_request);
      }
      free_http_request(&http_request);
#ifdef USE_SSL
      SSL_shutdown(ssl);
      SSL_free(ssl);
      ssl = NULL;
#endif
      close(client_socket);
   }
}

void start_server(int port)
{
   signal(SIGINT, sigint_handler);
   signal(SIGPIPE, SIG_IGN);
#ifdef USE_SSL
   initialize_ssl();
   ctx = create_ssl_context();
#endif

   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("Error creating socket");
      exit(EXIT_FAILURE);
   }

   int opt = 1;
   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
   {
      perror("Error setting socket options");
      close(sock);
      exit(EXIT_FAILURE);
   }

   struct sockaddr_in sin;
   sin.sin_family = AF_INET;
   sin.sin_port = htons(port);
   sin.sin_addr.s_addr = INADDR_ANY;

   if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) != 0)
   {
      perror("Error binding socket");
      close(sock);
      exit(EXIT_FAILURE);
   }

   if (listen(sock, nb_processes) != 0)
   {
      perror("Error listening on socket");
      exit(EXIT_FAILURE);
   }

   parent_pid = getpid();
   for (int i = 0; i < nb_processes; i++)
   {
      pid_t pid = fork();
      if (pid == 0)
      {
         printf("Worker process %d listening for requests\n", getpid());
         accept_connection();
         exit(0);
      }
      else if (pid < 0)
      {
         perror("Error forking process");
         exit(EXIT_FAILURE);
      }
   }

   while (wait(NULL) > 0)
      ;
#ifdef USE_SSL
   SSL_CTX_free(ctx);
#endif
   close(sock);
}
