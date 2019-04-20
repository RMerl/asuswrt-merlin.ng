#ifndef _UDP_H_
#define _UDP_H_

#include <stdint.h>
#include <netinet/in.h>
#include <sys/select.h>

struct udp_conf {
	int ipproto;
	int reuseaddr;
	int checksum;
	unsigned short port;
	union {
		struct {
			struct in_addr inet_addr;
		} ipv4;
		struct {
			struct in6_addr inet_addr6;
			int scope_id;
		} ipv6;
	} server;
	union {
		struct in_addr inet_addr;
		struct in6_addr inet_addr6;
	} client;
	int sndbuf;
	int rcvbuf;
};

struct udp_stats {
	uint64_t bytes;
	uint64_t messages;
	uint64_t error;
};

struct udp_sock {
	int fd;
	union {
		struct sockaddr_in ipv4;
		struct sockaddr_in6 ipv6;
	} addr;
	socklen_t sockaddr_len;
	struct udp_stats stats;
};

struct udp_sock *udp_server_create(struct udp_conf *conf);
void udp_server_destroy(struct udp_sock *m);

struct udp_sock *udp_client_create(struct udp_conf *conf);
void udp_client_destroy(struct udp_sock *m);

ssize_t udp_send(struct udp_sock *m, const void *data, int size);
ssize_t udp_recv(struct udp_sock *m, void *data, int size);

int udp_get_fd(struct udp_sock *m);
int udp_isset(struct udp_sock *m, fd_set *readfds);

int udp_snprintf_stats(char *buf, size_t buflen, char *ifname,
		       struct udp_stats *s, struct udp_stats *r);

int udp_snprintf_stats2(char *buf, size_t buflen, const char *ifname,
			const char *status, int active,
			struct udp_stats *s, struct udp_stats *r);

#endif
