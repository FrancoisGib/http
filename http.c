#include "http.h"

int sock = -1;
int nb_processes = NB_PROCESSES;
tree_t *http_tree;
pid_t parent_pid;

void sigint_handler(int code)
{
   printf("Process %d ended\n", getpid());
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
      printf("\nClosed socket");
      printf("\nFreed http tree\n");
   }
   exit(0);
}

const char *test_function(char *content)
{
   return "<p>test</p>";
}

int read_file(char buffer[1024], char *file_path)
{
   if (access(file_path, R_OK) == 0)
   {
      FILE *file = fopen(file_path, "r");
      return (int)fread(buffer, 1, 1024, file);
      fclose(file);
   }
   else
   {
      return -1;
   }
}

void construct_response(int client_socket, char *method, char *path, char *content, char *content_type)
{
   endpoint_t *endpoint = get_endpoint(http_tree, path);
   if (endpoint == NULL)
   {
      printf("No endpoint for path: %s \n", path);
      return;
   }
   char response[2048] = "HTTP/1.1 200 OK\r\n";

   int content_length = 0;
   char buffer[1024];
   memset(buffer, 0, 1024);
   if (endpoint->type == ET_FILE)
   {
      content_length = read_file(buffer, endpoint->resource.content);
   }
   else if (endpoint->type == ET_TEXT)
   {
      strcpy(buffer, endpoint->resource.content);
      content_length = strlen(endpoint->resource.content);
   }
   else if (endpoint->type == ET_FUNC)
   {
      resource_function function = endpoint->resource.function;
      const char *response_content = function(content);
      content_length = (int)strlen(response_content);
      strcpy(buffer, response_content);
   }
   else if (endpoint->type == ET_DIRECTORY)
   {
      while (strncmp(path, endpoint->path, strlen(endpoint->path)) != 0)
      {
         path++;
      }
      path += strlen(endpoint->path) + 1; /* pass the \0 made by get_endpoint */
      char file_path[strlen(path) + strlen(endpoint->resource.content) + 1];
      strcpy(file_path, endpoint->resource.content);
      strcat(file_path, path);
      content_length = read_file(buffer, file_path);
   }

   char content_type_str[64] = "Content-Type: ";
   if (content_length == -1)
   {
      char message[] = "Error";
      strcpy(buffer, message);
      content_length = strlen(message);
      strcat(content_type_str, print_content_type(TEXT));
   }
   else
   {
      if (endpoint->content_type == NULL_CONTENT)
      {
         strcat(content_type_str, get_content_type_with_file_extension(path));
      }
      else
      {
         strcat(content_type_str, print_content_type(endpoint->content_type));
      }
   }
   strcat(content_type_str, "\r\n\r\n");

   char content_length_buffer[5];
   sprintf(content_length_buffer, "%d", content_length);

   char content_length_str[32] = "Content-Length: ";
   strcat(content_length_str, content_length_buffer);
   strcat(content_length_str, "\r\n");

   strcat(response, content_length_str);
   strcat(response, content_type_str);

   strcat(response, buffer);

   printf("-----------------\n%s\n-----------------\n", response);

   int write_size = (int)write(client_socket, response, strlen(response));
   if (write_size < (int)strlen(response))
   {
      printf("Error while sending response\n");
      write_log("Error while sending response\n");
   }
   else
   {
      write_log("---\n\n");
      write_log(response);
      write_log("\n\n-----------------\n\n");
   }
}

void parse_http_request(char *request, int client_socket)
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
      header = strtok_r(rest, "\n", &rest);
      if (*header == '\r' || *(header + 1) == '\r')
      {
         break;
      }
      else
      {
         char *header_name = strtok_r(header, ":", &header);
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
      construct_response(client_socket, method, path, content, content_type);
   }
   else
   {
      construct_response(client_socket, method, path, NULL, content_type);
   }
}

void accept_connection(void)
{
   SOCKADDR_IN client_address;
   unsigned int client_address_len = sizeof(client_address);
   int client_socket;

   while (1)
   {
      client_socket = accept(sock, (struct sockaddr *)&client_address, &client_address_len);
      if (client_socket == -1)
      {
         perror("Error accepting connection");
         exit(EXIT_FAILURE);
      }

      char buffer[MAX_REQUEST_SIZE];
      int size = read(client_socket, buffer, MAX_REQUEST_SIZE);
      if (size > 0)
      {
         write_log(buffer);
         parse_http_request(buffer, client_socket);
         memset(buffer, 0, size);
      }
      close(client_socket);
   }
}

void start_server(int port)
{
   signal(SIGINT, sigint_handler);

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
      ; /* Wait for all child processes to finish */
}

int main(int argc, char **argv)
{
   int port;
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
      nb_processes = atoi(argv[2]);
   }

   char hostname[_SC_HOST_NAME_MAX + 1];
   gethostname(hostname, _SC_HOST_NAME_MAX + 1);

   const endpoint_t endpoints[] = {
       {"/hostname", {.content = hostname}, ET_TEXT, TEXT},
       {"/", {.content = "src/index.html"}, ET_FILE, HTML},
       {"/test", {.function = test_function}, ET_FUNC, HTML},
       {"/public", {.content = "src/public/"}, ET_DIRECTORY, NULL_CONTENT}};

   http_tree = build_http_tree(endpoints, sizeof(endpoints) / sizeof(endpoint_t));
   print_http_tree(http_tree, 0);
   parent_pid = getpid();
   printf("Starting server on process %d\n", parent_pid);
   start_server(port);

   return 0;
}
