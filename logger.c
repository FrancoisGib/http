#include "logger.h"

int write_log(char *message)
{
   time_t t = time(NULL);
   tm_t tm = *localtime(&t);
   stat_t st;
   if (stat("logs", &st) == -1)
   {
      if (mkdir("logs", 0700)) /* drw------- */
      {
         perror("Error creating log file");
         return -1;
      }
   }
   char time_str[64];
   if (sprintf(time_str, "logs/%d.%02d.%02d-%02d.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour) == EOF)
   {
      return -1;
   }

   FILE *log_file = fopen(time_str, "a");
   if (log_file != NULL)
   {
      size_t message_length = (strlen(message));
      int wrote_size = (int)fwrite(message, 1, message_length, log_file);
      return fclose(log_file) == 0 && -(wrote_size != (int)message_length);
   }
   perror("Error opening log file");
   return -1;
}