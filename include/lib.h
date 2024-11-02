#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "vars.h"

#define MIN(x, y) (x) < (y) ? (x) : (y)

typedef struct stat stat_t;

char *strdup(const char *str);
int read_file(char buffer[MAX_FILE_READ_SIZE], char *file_path);
int read_file_block(char buffer[MAX_FILE_READ_SIZE], int fd, int block);
char *search_last_occurence(char *str, char c);

#endif // LIB_H