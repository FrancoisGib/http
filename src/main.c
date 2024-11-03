#include "http.h"

extern int sock;
extern int nb_processes;
extern tree_t *http_tree;
extern endpoint_t *error_endpoint;

char *test_function(http_request_t *http_request)
{
   char *str = malloc(strlen("<p>test</p>") + 1);
   strcpy(str, "<p>test</p>");
   return str;
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
       {"/test", {{.function = test_function}, ET_FUNC, HTML, HTTP_STATUS_CREATED}},
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