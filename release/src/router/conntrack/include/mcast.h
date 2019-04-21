#ifndef _MCAST_H_
#define _MCAST_H_

#include <stdint.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/select.h>

struct mcast_conf {
	int ipproto;
	int reuseaddr;
	int checksum;
	unsigned short port;
	union {
		struct in_addr inet_addr;
		struct in6_addr inet_addr6;
	} in;
	union {
		struct in_addr interface_addr;
		unsigned int interface_index6;
	} ifa;
	int sndbuf;
	int rcvbuf;
};

struct mcast_stats {
	uint64_t bytes;
	uint64_t messages;
	uint64_t error;
};

struct mcast_sock {
	int fd;
	union {
		struct sockaddr_in ipv4;
		struct sockaddr_in6 ipv6;
	} addr;
	socklen_t sockaddr_len;
	struct mcast_stats stats;
};

struct mcast_sock *mcast_server_create(struct mcast_conf *conf);
void mcast_server_destroy(struct mcast_sock *m);

struct mcast_sock *mcast_client_create(struct mcast_conf *conf);
void mcast_client_destroy(struct mcast_sock *m);

ssize_t mcast_send(struct mcast_sock *m, const void *data, int size);
ssize_t mcast_recv(struct mcast_sock *m, void *data, int size);

int mcast_get_fd(struct mcast_sock *m);
int mcast_isset(struct mcast_sock *m, fd_set *readfds);

int mcast_snprintf_stats(char *buf, size_t buflen, char *ifname,
			 struct mcast_stats *s, struct mcast_stats *r);

int mcast_snprintf_stats2(char *buf, size_t buflen, const char *ifname,
			  const char *status, int active,
			  struct mcast_stats *s, struct mcast_stats *r);

#endif
