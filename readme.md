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