#include "ssl.h"

void initialize_ssl(void)
{
   SSL_load_error_strings();
   SSL_library_init();
   OpenSSL_add_all_algorithms();
}

SSL_CTX *create_ssl_context(void)
{
   SSL_CTX *ctx;
   const SSL_METHOD *method = SSLv23_server_method();
   ctx = SSL_CTX_new(method);
   if (!ctx)
   {
      perror("Unable to create SSL context");
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
   }

   // Charger le certificat et la clé
   SSL_CTX_use_certificate_file(ctx, "cert/cert.pem", SSL_FILETYPE_PEM);
   SSL_CTX_use_PrivateKey_file(ctx, "cert/key.pem", SSL_FILETYPE_PEM);

   return ctx;
}
