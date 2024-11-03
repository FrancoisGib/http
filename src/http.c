#include "http.h"

int sock = -1;
int nb_processes = NB_PROCESSES;
tree_t *http_tree;
http_response_t error_response;
pid_t parent_pid;

#ifdef USE_SSL
#define SOCK ssl

SSL_CTX *ctx = NULL;
SSL *ssl = NULL;

int ssl_read(SSL *_ssl, char buffer[MAX_REQUEST_SIZE], int buf_size)
{
   printf("read\n");
   if (SSL_accept(ssl) <= 0)
   {
      printf("error\n");
      // ERR_print_errors_fp(stderr);
      return -1;
   }
   else
   {
      int size = 0;
      if ((size = SSL_read(ssl, buffer, buf_size)) <= 0)
      {
         printf("error2\n");
         // ERR_print_errors_fp(stderr);
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

/*
void construct_response2(int client_socket, http_request_t *http_request)
{
   endpoint_t *endpoint = get_endpoint(http_tree, http_request->path);
   if (endpoint == NULL)
   {
      printf("No endpoint for path: %s \n", http_request->path);
      return;
   }

   int content_length = 0;
   char buffer[MAX_RESPONSE_SIZE];
   memset(buffer, 0, MAX_RESPONSE_SIZE);
   if (endpoint->response.type == ET_FILE)
   {
      stat_t st;
      if (stat(endpoint->response.resource.content, &st) == -1)
      {
         content_length = -1;
      }
      else
      {
         http_request->path = endpoint->response.resource.content;
         content_length = (int)st.st_size;
      }
   }
   else if (endpoint->response.type == ET_TEXT)
   {
      strcpy(buffer, endpoint->response.resource.content);
      content_length = strlen(endpoint->response.resource.content);
   }
   else if (endpoint->response.type == ET_FUNC)
   {
      resource_function function = endpoint->response.resource.function;
      char *response_content = function(http_request->body);
      content_length = (int)strlen(response_content);
      strcpy(buffer, response_content);
      free(response_content);
   }
   else if (endpoint->response.type == ET_DIRECTORY)
   {
      char *path_ptr = http_request->path;
      int size = strlen(path_ptr);
      while (size > 0 && strcmp(path_ptr, endpoint->path) != 0)
      {
         path_ptr++;
         size--;
      }
      path_ptr += strlen(endpoint->path) + 1;
      char *file_path = malloc(strlen(path_ptr) + strlen(endpoint->response.resource.content) + 2); # pass the \0 made by get_endpoint
// add 1 more if there's no / at the end of the directory path
strcpy(file_path, endpoint->response.resource.content);
int file_path_size = (int)strlen(endpoint->response.resource.content);
file_path[file_path_size] = '/'; // always / at the end
file_path[file_path_size + 1] = '\0';
if (*path_ptr != '\0')
{
   strcat(file_path, path_ptr);
   free(http_request->path);
   http_request->path = file_path;
   stat_t st;
   stat(file_path, &st);
   if (stat(file_path, &st) == -1)
   {
      content_length = -1;
   }
   else
   {
      content_length = (int)st.st_size;
   }
}
else
{
   content_length = -1;
}
}

char content_type_str[64] = "Content-Type: ";
if (content_length == -1)
{
   char message[] = "Error";
   strcpy(buffer, message);
   content_length = strlen(message);
   strcat(content_type_str, get_content_type(error_response.content_type));
}
else
{
   if (endpoint->response.content_type == NULL_CONTENT)
   {
      strcat(content_type_str, get_content_type_with_file_extension(http_request->path));
   }
   else
   {
      strcat(content_type_str, get_content_type(endpoint->response.content_type));
   }
}
strcat(content_type_str, "\r\n\r\n");

char content_length_buffer[7];
sprintf(content_length_buffer, "%d", content_length);

char content_length_str[32] = "Content-Length: ";
strcat(content_length_str, content_length_buffer);
strcat(content_length_str, "\r\n");

char response[MAX_RESPONSE_SIZE] = "HTTP/1.1 ";

char status_code[6];
sprintf(status_code, "%d\r\n", endpoint->response.status);
strcat(response, status_code);

strcat(response, content_length_str);
strcat(response, content_type_str);

strcat(response, buffer);

printf("-----------------\n%s\n-----------------\n", response);

int write_size = -1;
if (tls)
{
   if (content_length > 0 && (endpoint->response.type == ET_FILE || endpoint->response.type == ET_DIRECTORY))
   {
      if (strcmp(content_type_str, "Content-Type: image/x-icon\r\n\r\n") == 0 || strcmp(content_type_str, "Content-Type: image/png\r\n\r\n") == 0)
      {
         char buf[content_length + strlen(response) + 1];
         memset(buf, 0, content_length + strlen(response) + 1);
         strcpy(buf, response);
         if (read_file(buf, http_request->path) != -1)
         {
            strcat(response, buf);
            write_size = (int)SSL_write(ssl, buf, strlen(response) + content_length);
         }
         else
         {
            printf("Error reading file\n");
         }
      }
      else
      {
         int response_length = strlen(response);
         if (response_length + content_length < MAX_RESPONSE_SIZE && content_length <= MAX_FILE_READ_SIZE)
         {
            char buf[content_length + 1];
            memset(buf, 0, content_length);

            if (read_file(buf, http_request->path) != -1)
            {
               strcat(response, buf);
               write_size = (int)SSL_write(ssl, response, response_length + content_length);
            }
            else
            {
               printf("Error reading file\n");
            }
         }
         else
         {
            write_size = (int)SSL_write(ssl, response, response_length);
            char buf[MAX_FILE_READ_SIZE];
            memset(buf, 0, MAX_FILE_READ_SIZE);
            int read_size;
            int fd = open(http_request->path, O_RDONLY);
            if (fd != -1)
            {
               int block = 0;
               while ((read_size = read_file_block(buf, fd, block)) > 0)
               {
                  write_size = (int)SSL_write(ssl, buf, strlen(buf));
                  block++;
                  memset(buf, 0, MAX_FILE_READ_SIZE);
               }
               close(fd);
            }
         }
      }
   }
   else
   {
      write_size = (int)SSL_write(ssl, response, strlen(response));
   }
}
else
{
   write_size = (int)write(client_socket, response, strlen(response));
}
if (write_size < (int)strlen(response))
{
   printf("Error while sending response\n");
   write_log("Error while sending response\n");
}
}
*/

void http_response_build_type_et_file(http_response_t *http_response, endpoint_t *endpoint)
{
   stat_t st;
   if (stat(endpoint->response.resource.content, &st) == -1)
   {
      http_response->content_length = -1;
   }
   else
   {
      http_response->resource.file_path = endpoint->response.resource.content;
      http_response->content_type = get_content_type_with_file_extension(http_response->resource.file_path);
      http_response->content_length = (int)st.st_size;
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

void http_response_build_type_et_directory(http_response_t *http_response, http_request_t *http_request, endpoint_t *endpoint)
{
   char *path_ptr = http_request->path;
   int size = strlen(path_ptr);
   while (size > 0 && strcmp(path_ptr, endpoint->path) != 0)
   {
      path_ptr++;
      size--;
   }
   path_ptr += strlen(endpoint->path) + 1;
   char *file_path = malloc(strlen(path_ptr) + strlen(endpoint->response.resource.content) + 2);
   // pass the \0 made by get_endpoint
   // add 1 more if there's no / at the end of the directory path
   strcpy(file_path, endpoint->response.resource.content);
   int file_path_size = (int)strlen(endpoint->response.resource.content);
   file_path[file_path_size] = '/'; // always / at the end
   file_path[file_path_size + 1] = '\0';
   if (*path_ptr != '\0')
   {
      strcat(file_path, path_ptr);
      http_response->resource.file_path = file_path;
      http_response->content_type = get_content_type_with_file_extension(http_response->resource.file_path);
      stat_t st;
      stat(file_path, &st);
      if (stat(file_path, &st) == -1)
      {
         http_response->content_length = -1;
      }
      else
      {
         http_response->content_length = (int)st.st_size;
      }
   }
   else
   {
      http_response->content_length = -1;
   }
}

void http_response_build_error(http_response_t *http_response)
{
   // http_response->content_length = error_response.content_length;
   // http_response->content_type = error_response.content_type;
   // http_response->headers = error_response.headers;
   // http_response->resource = error_response.resource;
   // http_response->status = error_response.status;
   // http_response->type = error_response.type;
   memcpy(http_response, &error_response, sizeof(http_response_t));
}

void construct_response(int client_socket, http_request_t *http_request)
{
   endpoint_t *endpoint = get_endpoint(http_tree, http_request->path);
   if (endpoint == NULL)
   {
      printf("No endpoint for path: %s \n", http_request->path);
      return;
   }

   http_response_t http_response;
   http_response.headers = NULL;
   http_response.status = endpoint->response.status;
   http_response.content_type = endpoint->response.content_type;

   char response_content_buffer[MAX_RESPONSE_SIZE];
   memset(response_content_buffer, 0, MAX_RESPONSE_SIZE);

   switch (endpoint->response.type)
   {
   case ET_FILE:
      http_response_build_type_et_file(&http_response, endpoint);
      break;

   case ET_TEXT:
      http_response_build_type_et_text(&http_response, endpoint, response_content_buffer);
      break;

   case ET_FUNC:
      http_response_build_type_et_func(&http_response, http_request, endpoint, response_content_buffer);
      break;

   case ET_DIRECTORY:
      http_response_build_type_et_directory(&http_response, http_request, endpoint);
      break;

   default:
      http_response_build_error(&http_response);
      break;
   }

   char response_buffer[MAX_RESPONSE_SIZE] = "HTTP/1.1 ";

   char status_code[6];
   sprintf(status_code, "%d\r\n", http_response.status);
   strcat(response_buffer, status_code);

   ll_node_t *header_node = http_response.headers;
   while (header_node != NULL)
   {
      header_t *header = header_node->element;
      strcat(response_buffer, header->name);
      strcat(response_buffer, ": ");
      strcat(response_buffer, header->value);
      strcat(response_buffer, "\r\n");
      header_node = header_node->next;
   }

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
   switch (endpoint->response.type)
   {
   case ET_FILE:
      http_response_build_type_et_file(&http_response, endpoint);
      break;

   case ET_TEXT:
      strcat(response_buffer, response_content_buffer);
      written_size = WRITE(SOCK, response_buffer, strlen(response_buffer));
      break;

   case ET_FUNC:
      strcat(response_buffer, response_content_buffer);
      written_size = WRITE(SOCK, response_buffer, strlen(response_buffer));
      break;

   case ET_DIRECTORY:
      break;

   default:
      break;
   }

   // int write_size = -1;
   // if (tls)
   // {
   //    if (http_response.content_length > 0 && (endpoint->response.type == ET_FILE || endpoint->response.type == ET_DIRECTORY))
   //    {
   //       if (strcmp(content_type_str, "Content-Type: image/x-icon\r\n\r\n") == 0 || strcmp(content_type_str, "Content-Type: image/png\r\n\r\n") == 0)
   //       {
   //          char buf[http_response.content_length + strlen(response) + 1];
   //          memset(buf, 0, http_response.content_length + strlen(response) + 1);
   //          strcpy(buf, response);
   //          if (read_file(buf, http_request->path) != -1)
   //          {
   //             strcat(response, buf);
   //             write_size = (int)SSL_write(ssl, buf, strlen(response) + http_response.content_length);
   //          }
   //          else
   //          {
   //             printf("Error reading file\n");
   //          }
   //       }
   //       else
   //       {
   //          int response_length = strlen(response);
   //          if (response_length + http_response.content_length < MAX_RESPONSE_SIZE && http_response.content_length <= MAX_FILE_READ_SIZE)
   //          {
   //             char buf[http_response.content_length + 1];
   //             memset(buf, 0, http_response.content_length);

   //             if (read_file(buf, http_request->path) != -1)
   //             {
   //                strcat(response, buf);
   //                write_size = (int)SSL_write(ssl, response, response_length + http_response.content_length);
   //             }
   //             else
   //             {
   //                printf("Error reading file\n");
   //             }
   //          }
   //          else
   //          {
   //             write_size = (int)SSL_write(ssl, response, response_length);
   //             char buf[MAX_FILE_READ_SIZE];
   //             memset(buf, 0, MAX_FILE_READ_SIZE);
   //             int read_size;
   //             int fd = open(http_request->path, O_RDONLY);
   //             if (fd != -1)
   //             {
   //                int block = 0;
   //                while ((read_size = read_file_block(buf, fd, block)) > 0)
   //                {
   //                   write_size = (int)SSL_write(ssl, buf, strlen(buf));
   //                   block++;
   //                   memset(buf, 0, MAX_FILE_READ_SIZE);
   //                }
   //                close(fd);
   //             }
   //          }
   //       }
   //    }
   //    else
   //    {
   //       write_size = (int)SSL_write(ssl, response, strlen(response));
   //    }
   // }
   // else
   // {
   //    write_size = (int)write(client_socket, response, strlen(response));
   // }
   // if (write_size < (int)strlen(response))
   // {
   //    printf("Error while sending response\n");
   //    write_log("Error while sending response\n");
   // }
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
         if (referee_path != NULL)
         {
            int diff_size = referer_directory_end_pointer - referee_path;
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

void *http_request_write_log_wrapper(void *http_request)
{
   http_request_write_log((http_request_t *)http_request);
   return NULL;
}

// int ssl_read(int client_socket, char buffer[MAX_REQUEST_SIZE])
// {
//    if (SSL_accept(ssl) <= 0)
//    {
//       // ERR_print_errors_fp(stderr);
//       return -1;
//    }
//    else
//    {
//       int size = 0;
//       if ((size = SSL_read(ssl, buffer, MAX_REQUEST_SIZE)) <= 0)
//       {
//          // ERR_print_errors_fp(stderr);
//          return -1;
//       };
//       return size;
//    }
// }

void free_http_request_args(http_request_t *request)
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
      if (ssl == NULL)
      {
         printf("Error\n");
         continue;
      }
      if (SSL_set_fd(ssl, client_socket) == 0)
      {
         printf("Error2\n");
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
#ifdef USE_SSL
         printf("ctx %d\n", ssl == NULL);
#endif
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
               continue;
            }
            is_first_request = 0;
         }

         if (!header_parsed && http_request_parse_headers(&http_request, request_ptr) == 1)
         {
            header_parsed = 1;
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
      if (size == -1)
      {
         printf("Error with the request\n");
      }
      else
      {
         construct_response(client_socket, &http_request);
         http_request_write_log(&http_request);
      }
      free_http_request_args(&http_request);
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
#ifdef USE_SSL
   printf("init\n");
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
