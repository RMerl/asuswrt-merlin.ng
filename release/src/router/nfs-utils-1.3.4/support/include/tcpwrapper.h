#ifndef TCP_WRAPPER_H
#define TCP_WRAPPER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int from_local(const struct sockaddr *sap);
extern int check_default(char *name, struct sockaddr *sap,
			const unsigned long program);

#endif /* TCP_WRAPPER_H */
