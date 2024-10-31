#include "logger.h"

tm_t tm;

int write_log(char *message)
{
   time_t t = time(NULL);
   tm = *localtime(&t);
   stat_t st;
   if (stat("logs", &st) == -1)
   {
      mkdir("logs", 0700);
   }
   char time_str[64];
   sprintf(time_str, "logs/%d.%02d.%02d-%02d.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);

   FILE *log_file = fopen(time_str, "a+");
   int wrote_size = (int)fwrite(message, 1, strlen(message), log_file);
   fclose(log_file);
   if (wrote_size < (int)strlen(message))
   {
      return -1;
   }
   return 0;
}