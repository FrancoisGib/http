#ifndef HTTP_STATUS_H
#define HTTP_STATUS_H

typedef enum
{
   HTTP_STATUS_OK = 200,
   HTTP_STATUS_CREATED = 201,
   HTTP_STATUS_NO_CONTENT = 204,
   HTTP_STATUS_BAD_REQUEST = 400,
   HTTP_STATUS_UNAUTHORIZED = 401,
   HTTP_STATUS_FORBIDDEN = 403,
   HTTP_STATUS_NOT_FOUND = 404
} http_status_e;

#endif