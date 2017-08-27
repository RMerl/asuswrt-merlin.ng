/*
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <linux/netlink.h>

#include <ctf_cfg.h>

#include "flow_linux.h"

int flow_cfg_request_send(ctf_cfg_request_t *buffer, int length)
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct msghdr msg;
	struct iovec iov;
	int    sock_fd, ret = SUCCESS;

	if ((buffer == NULL) || (length > CTFCFG_MAX_SIZE))
	{
		fprintf(stderr, "Invalid parameters %p %d\n", buffer, length);
		return FAILURE;
	}

	/* Create a netlink socket */
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_CTF);

	if (sock_fd < 0)
	{
		fprintf(stderr, "Netlink socket create failed\n");
		return FAILURE;
	}

	/* Associate a local address with the opened socket */
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	src_addr.nl_groups = 0;
	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

	/* Fill the destination address, pid of 0 indicates kernel */
	memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;

	/* Allocate memory for sending configuration request */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(CTFCFG_MAX_SIZE));

	if (nlh == NULL)
	{
		fprintf(stderr, "Out of memory allocating cfg buffer\n");
		ret = FAILURE;
		goto out;
	}

	/* Fill the netlink message header. The configuration request
	 * contains netlink header followed by data.
	 */
	nlh->nlmsg_len = NLMSG_SPACE(CTFCFG_MAX_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	/* Fill the data part */
	memcpy(NLMSG_DATA(nlh), buffer, length);
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Send request to kernel module */
	ret = sendmsg(sock_fd, &msg, 0);
	if (ret < 0)
	{
		perror("sendmsg:");
		ret = FAILURE;
		goto out;
	}

	/* Wait for the response */
	memset(nlh, 0, NLMSG_SPACE(CTFCFG_MAX_SIZE));
	ret = recvmsg(sock_fd, &msg, 0);
	if (ret < 0)
	{
		perror("recvmsg:");
		ret = FAILURE;
		goto out;
	}

	/* Copy data to user buffer */
	memcpy(buffer, NLMSG_DATA(nlh), length);

out:
	if (nlh) {
		free(nlh);
	}

	if (sock_fd >= 0) {
		close(sock_fd);
	}

	return ret;
}
