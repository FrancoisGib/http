#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


#define MIN(x, y) (x) < (y) ? (x) : (y)

typedef struct stat stat_t;

char *strdup(const char *str);
int read_file(char buffer[1024], char *file_path);

#endif // LIB_H