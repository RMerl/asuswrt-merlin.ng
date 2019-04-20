#ifndef _TCP_H_
#define _TCP_H_

#include <stdint.h>
#include <netinet/in.h>
#include <sys/select.h>

struct tcp_conf {
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

struct tcp_stats {
	uint64_t bytes;
	uint64_t messages;
	uint64_t error;
};

enum tcp_sock_state {
	TCP_SERVER_ACCEPTING,
	TCP_SERVER_CONNECTED,
	TCP_CLIENT_DISCONNECTED,
	TCP_CLIENT_CONNECTED
};

struct tcp_sock {
	int state;	/* enum tcp_sock_state */
	int fd;
	int client_fd;	/* only for the server side */
	union {
		struct sockaddr_in ipv4;
		struct sockaddr_in6 ipv6;
	} addr;
	socklen_t sockaddr_len;
	struct tcp_stats stats;
	struct tcp_conf *conf;
};

struct tcp_sock *tcp_server_create(struct tcp_conf *conf);
void tcp_server_destroy(struct tcp_sock *m);

struct tcp_sock *tcp_client_create(struct tcp_conf *conf);
void tcp_client_destroy(struct tcp_sock *m);

ssize_t tcp_send(struct tcp_sock *m, const void *data, int size);
ssize_t tcp_recv(struct tcp_sock *m, void *data, int size);
int tcp_accept(struct tcp_sock *m);

int tcp_get_fd(struct tcp_sock *m);
int tcp_isset(struct tcp_sock *m, fd_set *readfds);
int tcp_accept_isset(struct tcp_sock *m, fd_set *readfds);

int tcp_snprintf_stats(char *buf, size_t buflen, char *ifname,
		       struct tcp_sock *s, struct tcp_sock *r);

int tcp_snprintf_stats2(char *buf, size_t buflen, const char *ifname,
			const char *status, int active,
			struct tcp_stats *s, struct tcp_stats *r);

#endif
