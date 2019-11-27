#ifndef _REQUEST_H_
#define _REQUEST_H_
#include "first.h"

#include "server.h"

typedef enum {
  HTTP_PARSEOPT_HEADER_STRICT  = 1
 ,HTTP_PARSEOPT_HOST_STRICT    = 2
 ,HTTP_PARSEOPT_HOST_NORMALIZE = 4
} http_parseopts_e;

int http_request_parse(server *srv, connection *con);
int http_request_header_finished(server *srv, connection *con);
int http_request_host_normalize(buffer *b);

#endif
