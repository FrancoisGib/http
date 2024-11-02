#include "lib.h"

char *strdup(const char *str)
{
   char *dup = malloc(strlen(str) + 1);
   if (dup != NULL)
   {
      strcpy(dup, str);
   }
   return dup;
}

int read_file(char buffer[1024], char *file_path)
{
   stat_t st;
   stat(file_path, &st);
   if (!S_ISDIR(st.st_mode) && access(file_path, R_OK) == 0)
   {
      FILE *file = fopen(file_path, "r");
      int size = (int)fread(buffer, 1, 1024, file);
      fclose(file);
      return size;
   }
   return -1;
}