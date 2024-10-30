```c
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
       {"/hostname", hostname, ET_TEXT, TEXT},
       {"/", "src/index.html", ET_FILE, HTML},
       {"/test", test_function, ET_FUNC, HTML},
       {"/public", "src/public/", ET_DIRECTORY, HTML}};

   http_tree = build_http_tree(endpoints, sizeof(endpoints) / sizeof(endpoint_t));
   print_http_tree(http_tree, 0);
   parent_pid = getpid();
   printf("Starting server on process %d\n", parent_pid);
   start_server(port);

   return 0;
}
```

```bash
make http
./http { port }
```

```bash
# Careful not to forget to copy your served files in the Dockerfile if you have some.
make docker-image
```

## Data Structure

The data structure used to make the endpoints is a tree. It's children are also trees contained in a linked list. This tree structure allow to search and add endpoints efficiently and quickly.
Example:
url: /1/2/3
Here, there's is one endpoint and three trees whose paths are /1, /2 and /3. But the only resource served is at the /1/2/3 endpoint.

In the main example, the tree structure is:
```
├── / serving an http file with "Hello world" content
    └── /hostname serving machine hostname
    └── /test serving "<p>test</p>" (returned by a function taking the content of the request, so it can be set dynamically)
    └── /public serving src/public folder
```
