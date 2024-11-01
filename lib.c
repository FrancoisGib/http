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
