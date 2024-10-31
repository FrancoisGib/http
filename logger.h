#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

typedef struct tm tm_t;
typedef struct stat stat_t;

int write_log(char *message);