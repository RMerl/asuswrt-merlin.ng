// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * This implements userspace semantics of "sticky sockets", modeled after
 * WireGuard's kernelspace implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/ipv6.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

struct magic_endpoint {
	union {
		struct sockaddr addr;
		struct sockaddr_in addr4;
		struct sockaddr_in6 addr6;
	};
	union {
		struct {
			struct in_addr src4;
			int src_if4; /* Essentially the same as addr6->scope_id */
		};
		struct in6_addr src6;
	};
};

ssize_t magic_send4(int sock, struct magic_endpoint *endpoint, void *buffer, size_t len)
{
	ssize_t ret;
	struct iovec iovec = {
		.iov_base = buffer,
		.iov_len = len
	};
	struct {
		struct cmsghdr cmsghdr;
		struct in_pktinfo pktinfo;
	} cmsg = {
		.cmsghdr.cmsg_level = IPPROTO_IP,
		.cmsghdr.cmsg_type = IP_PKTINFO,
		.cmsghdr.cmsg_len = CMSG_LEN(sizeof(cmsg.pktinfo)),
		.pktinfo.ipi_spec_dst = endpoint->src4,
		.pktinfo.ipi_ifindex = endpoint->src_if4
	};
	struct msghdr msghdr = {
		.msg_iov = &iovec,
		.msg_iovlen = 1,
		.msg_name = &endpoint->addr4,
		.msg_namelen = sizeof(endpoint->addr4),
		.msg_control = &cmsg,
		.msg_controllen = sizeof(cmsg)
	};
	ret = sendmsg(sock, &msghdr, 0);
	if (ret < 0 && errno == EINVAL) {
		memset(&cmsg.pktinfo, 0, sizeof(cmsg.pktinfo));
		endpoint->src4.s_addr = endpoint->src_if4 = 0;
		return sendmsg(sock, &msghdr, 0);
	}
	return ret;
}

ssize_t magic_send6(int sock, struct magic_endpoint *endpoint, void *buffer, size_t len)
{
	ssize_t ret;
	struct iovec iovec = {
		.iov_base = buffer,
		.iov_len = len
	};
	struct {
		struct cmsghdr cmsghdr;
		struct in6_pktinfo pktinfo;
	} cmsg = {
		.cmsghdr.cmsg_level = IPPROTO_IPV6,
		.cmsghdr.cmsg_type = IPV6_PKTINFO,
		.cmsghdr.cmsg_len = CMSG_LEN(sizeof(cmsg.pktinfo)),
		.pktinfo.ipi6_addr = endpoint->src6,
		.pktinfo.ipi6_ifindex = memcmp(&in6addr_any, &endpoint->src6, sizeof(endpoint->src6)) ? endpoint->addr6.sin6_scope_id : 0
	};
	struct msghdr msghdr = {
		.msg_iov = &iovec,
		.msg_iovlen = 1,
		.msg_name = &endpoint->addr6,
		.msg_namelen = sizeof(endpoint->addr6),
		.msg_control = &cmsg,
		.msg_controllen = sizeof(cmsg)
	};

	ret = sendmsg(sock, &msghdr, 0);
	if (ret < 0 && errno == EINVAL) {
		memset(&cmsg.pktinfo, 0, sizeof(cmsg.pktinfo));
		memset(&endpoint->src6, 0, sizeof(endpoint->src6));
		return sendmsg(sock, &msghdr, 0);
	}
	return ret;
}

ssize_t magic_receive4(int sock, struct magic_endpoint *endpoint, void *buffer, size_t len)
{
	ssize_t ret;
	struct iovec iovec = {
		.iov_base = buffer,
		.iov_len = len
	};
	struct {
		struct cmsghdr cmsghdr;
		struct in_pktinfo pktinfo;
	} cmsg;
	struct msghdr msghdr = {
		.msg_iov = &iovec,
		.msg_iovlen = 1,
		.msg_name = &endpoint->addr4,
		.msg_namelen = sizeof(endpoint->addr4),
		.msg_control = &cmsg,
		.msg_controllen = sizeof(cmsg)
	};

	ret = recvmsg(sock, &msghdr, 0);
	if (ret < 0)
		return ret;
	if (cmsg.cmsghdr.cmsg_level == IPPROTO_IP && cmsg.cmsghdr.cmsg_type == IP_PKTINFO && cmsg.cmsghdr.cmsg_len >= CMSG_LEN(sizeof(cmsg.pktinfo))) {
		endpoint->src4 = cmsg.pktinfo.ipi_spec_dst;
		endpoint->src_if4 = cmsg.pktinfo.ipi_ifindex;
	}
	return ret;
}

ssize_t magic_receive6(int sock, struct magic_endpoint *endpoint, void *buffer, size_t len)
{
	ssize_t ret;
	struct iovec iovec = {
		.iov_base = buffer,
		.iov_len = len
	};
	struct {
		struct cmsghdr cmsghdr;
		struct in6_pktinfo pktinfo;
	} cmsg;
	struct msghdr msghdr = {
		.msg_iov = &iovec,
		.msg_iovlen = 1,
		.msg_name = &endpoint->addr6,
		.msg_namelen = sizeof(endpoint->addr6),
		.msg_control = &cmsg,
		.msg_controllen = sizeof(cmsg)
	};

	ret = recvmsg(sock, &msghdr, 0);
	if (ret < 0)
		return ret;
	if (cmsg.cmsghdr.cmsg_level == IPPROTO_IPV6 && cmsg.cmsghdr.cmsg_type == IPV6_PKTINFO && cmsg.cmsghdr.cmsg_len >= CMSG_LEN(sizeof(cmsg.pktinfo))) {
		endpoint->src6 = cmsg.pktinfo.ipi6_addr;
		endpoint->addr6.sin6_scope_id = cmsg.pktinfo.ipi6_ifindex;
	}
	return ret;
}

void magic_endpoint_clearsrc(struct magic_endpoint *endpoint)
{
	if (endpoint->addr.sa_family == AF_INET)
		endpoint->src4.s_addr = endpoint->src_if4 = 0;
	else if (endpoint->addr.sa_family == AF_INET6)
		memset(&endpoint->src6, 0, sizeof(endpoint->src6));
	else
		memset(endpoint, 0, sizeof(*endpoint));
}

void magic_endpoint_set(struct magic_endpoint *endpoint, const struct sockaddr *addr)
{
	if (addr->sa_family == AF_INET)
		endpoint->addr4 = *(struct sockaddr_in *)addr;
	else if (addr->sa_family == AF_INET6)
		endpoint->addr6 = *(struct sockaddr_in6 *)addr;
	magic_endpoint_clearsrc(endpoint);
}

int magic_create_sock4(uint16_t listen_port)
{
	static const int on = 1;
	struct sockaddr_in listen_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(listen_port),
		.sin_addr = INADDR_ANY
	};
	int fd, ret;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return fd;
	
	ret = setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on));
	if (ret < 0)
		goto err;
	
	ret = bind(fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
	if (ret < 0)
		goto err;
	
	return fd;

err:
	close(fd);
	return ret;
}

int magic_create_sock6(uint16_t listen_port)
{
	static const int on = 1;
	struct sockaddr_in6 listen_addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(listen_port),
		.sin6_addr = IN6ADDR_ANY_INIT
	};
	int fd, ret;
	
	fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (fd < 0)
		return fd;
	
	ret = setsockopt(fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on));
	if (ret < 0)
		goto err;

	ret = setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
	if (ret < 0)
		goto err;
	
	ret = bind(fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
	if (ret < 0)
		goto err;
	
	return fd;

err:
	close(fd);
	return ret;
}

int main(int argc, char *argv[])
{
	struct magic_endpoint endpoint = { 0 };
	int sock;
	ssize_t ret;
	uint8_t buffer[1024] = { 0 };
	char srcaddr[40], dstaddr[40];

	if (argc == 2 && !strcmp(argv[1], "-4"))
		goto v4;
	if (argc == 2 && !strcmp(argv[1], "-6"))
		goto v6;
	return 1;

v6:
	sock = magic_create_sock6(51820);
	if (sock < 0) {
		perror("magic_create_sock6");
		return 1;
	}

	ret = magic_receive6(sock, &endpoint, buffer, sizeof(buffer));
	if (ret < 0) {
		perror("magic_receive6");
		return 1;
	}

	if (!inet_ntop(AF_INET6, &endpoint.src6, srcaddr, sizeof(srcaddr))) {
		perror("inet_ntop");
		return 1;
	}

	if (!inet_ntop(AF_INET6, &endpoint.addr6.sin6_addr, dstaddr, sizeof(dstaddr))) {
		perror("inet_ntop");
		return 1;
	}

	printf("if:%d src:%s dst:%s\n", endpoint.addr6.sin6_scope_id, srcaddr, dstaddr);
	printf("Received a packet. Sleeping for 10 seconds before replying, so you have time to mess with your networking setup.\n");
	sleep(10);

	ret = magic_send6(sock, &endpoint, buffer, sizeof(buffer));
	if (ret < 0) {
		perror("magic_send6");
		return 1;
	}

	close(sock);
	return 0;

v4:
	sock = magic_create_sock4(51820);
	if (sock < 0) {
		perror("magic_create_sock4");
		return 1;
	}

	ret = magic_receive4(sock, &endpoint, buffer, sizeof(buffer));
	if (ret < 0) {
		perror("magic_receive4");
		return 1;
	}

	if (!inet_ntop(AF_INET, &endpoint.src4, srcaddr, sizeof(srcaddr))) {
		perror("inet_ntop");
		return 1;
	}

	if (!inet_ntop(AF_INET, &endpoint.addr4.sin_addr, dstaddr, sizeof(dstaddr))) {
		perror("inet_ntop");
		return 1;
	}

	printf("if:%d src:%s dst:%s\n", endpoint.src_if4, srcaddr, dstaddr);
	printf("Received a packet. Sleeping for 10 seconds before replying, so you have time to mess with your networking setup.\n");
	sleep(10);

	ret = magic_send4(sock, &endpoint, buffer, sizeof(buffer));
	if (ret < 0) {
		perror("magic_send4");
		return 1;
	}
	
	close(sock);
	return 0;
}
