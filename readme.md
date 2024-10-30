```c
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
```bash
├── / serving machine hostname
    └── /home serving an http file with "Hello world" content
    └── /test serving "<p>ça marche</p>" (returned by a function taking the content of the request, so it can be set dynamically)
```
