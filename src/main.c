#include "http.h"

extern int sock;
extern int nb_processes;
extern tree_t *http_tree;
extern endpoint_t *error_endpoint;

void test_function(http_req_res_t *http_req_res)
{
   char *content = "<p>Response from server !</p>";
   int content_length = strlen(content);
   http_req_res->response->resource.content = malloc(content_length + 1);
   strcpy(http_req_res->response->resource.content, content);
   http_req_res->response->content_length = content_length;
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
      return 0;
   }
   if (argc > 2)
   {
      nb_processes = atoi(argv[2]);
   }

   char hostname[_SC_HOST_NAME_MAX + 1];
   gethostname(hostname, _SC_HOST_NAME_MAX + 1);

   const endpoint_t endpoints[] = {
       {"/hostname", {{.content = hostname}, ET_TEXT, TEXT, HTTP_STATUS_OK}},
       {"/", {{.content = "examples/index.html"}, ET_FILE, HTML, HTTP_STATUS_OK}},
       {"/func", {{.function = test_function}, ET_FUNC, HTML, HTTP_STATUS_CREATED}},
       {"/public", {{.content = "examples/public"}, ET_DIRECTORY, NULL_CONTENT, HTTP_STATUS_OK}},
       {"/build", {{.content = "examples/app/build"}, ET_DIRECTORY, NULL_CONTENT, HTTP_STATUS_OK}},
   };

   endpoint_t error = (endpoint_t){"", {{.content = "Error"}, ET_TEXT, TEXT, HTTP_STATUS_OK}};
   error_endpoint = &error;

   http_tree = build_http_tree(endpoints, sizeof(endpoints) / sizeof(endpoint_t));
   print_http_tree(http_tree, 0);
   printf("Starting server on port %d\n", port);
   start_server(port);
   return 0;
}