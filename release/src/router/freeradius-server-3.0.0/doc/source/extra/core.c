/**
 * @cond skip
 * vim:syntax=doxygen
 * @endcond
 *
 *
@page server_doc

@section server_intro Introduction

FreeRADIUS uses a thread pool to serve requests. Each request is processed
synchronously, and processing passes through a series of stages, and a list
of modules in each stage.

The request is processed as follows

- The radius packet is received by a listener - see listen.c
- The radius packet is parsed and validated into a request - see ?
- The request is processed - see process.c
- The server passes through each authentication stage
  - authorize
  - if Proxy-To-Realm is set:
    - pre-proxy
    - send proxy request
    - post-proxy
  - else
    - authenticate
  - post-auth
- Authentication stages are lists of modules - see modcall.c

*/
