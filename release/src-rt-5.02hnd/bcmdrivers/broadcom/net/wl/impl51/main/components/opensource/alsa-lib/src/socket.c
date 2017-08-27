/**
 * \file socket.c
 * \brief Socket helper routines
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2003
 */
/*
 *  Socket helper routines
 *  Copyright (c) 2003 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
  
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include "local.h"

#ifndef DOC_HIDDEN
int snd_send_fd(int sock, void *data, size_t len, int fd)
{
	int ret;
	size_t cmsg_len = CMSG_LEN(sizeof(int));
	struct cmsghdr *cmsg = alloca(cmsg_len);
	int *fds = (int *) CMSG_DATA(cmsg);
	struct msghdr msghdr;
	struct iovec vec;

	vec.iov_base = (void *)&data;
	vec.iov_len = len;

	cmsg->cmsg_len = cmsg_len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*fds = fd;

	msghdr.msg_name = NULL;
	msghdr.msg_namelen = 0;
	msghdr.msg_iov = &vec;
 	msghdr.msg_iovlen = 1;
	msghdr.msg_control = cmsg;
	msghdr.msg_controllen = cmsg_len;
	msghdr.msg_flags = 0;

	ret = sendmsg(sock, &msghdr, 0 );
	if (ret < 0) {
		SYSERR("sendmsg failed");
		return -errno;
	}
	return ret;
}

int snd_receive_fd(int sock, void *data, size_t len, int *fd)
{
	int ret;
	size_t cmsg_len = CMSG_LEN(sizeof(int));
	struct cmsghdr *cmsg = alloca(cmsg_len);
	int *fds = (int *) CMSG_DATA(cmsg);
	struct msghdr msghdr;
	struct iovec vec;

	vec.iov_base = (void *)&data;
	vec.iov_len = len;

	cmsg->cmsg_len = cmsg_len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*fds = -1;

	msghdr.msg_name = NULL;
	msghdr.msg_namelen = 0;
	msghdr.msg_iov = &vec;
	msghdr.msg_iovlen = 1;
	msghdr.msg_control = cmsg;
	msghdr.msg_controllen = cmsg_len;
	msghdr.msg_flags = 0;

	ret = recvmsg(sock, &msghdr, 0);
	if (ret < 0) {
		SYSERR("recvmsg failed");
		return -errno;
	}
	*fd = *fds;
	return ret;
}

int snd_is_local(struct hostent *hent)
{
	int s;
	int err;
	struct ifconf conf;
	size_t numreqs = 10;
	size_t i;
	struct in_addr *haddr = (struct in_addr*) hent->h_addr_list[0];
	
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		SYSERR("socket failed");
		return -errno;
	}
	
	conf.ifc_len = numreqs * sizeof(struct ifreq);
	conf.ifc_buf = malloc((unsigned int) conf.ifc_len);
	if (! conf.ifc_buf)
		return -ENOMEM;
	while (1) {
		err = ioctl(s, SIOCGIFCONF, &conf);
		if (err < 0) {
			SYSERR("SIOCGIFCONF failed");
			return -errno;
		}
		if ((size_t)conf.ifc_len < numreqs * sizeof(struct ifreq))
			break;
		numreqs *= 2;
		conf.ifc_len = numreqs * sizeof(struct ifreq);
		conf.ifc_buf = realloc(conf.ifc_buf, (unsigned int) conf.ifc_len);
		if (! conf.ifc_buf)
			return -ENOMEM;
	}
	numreqs = conf.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < numreqs; ++i) {
		struct ifreq *req = &conf.ifc_req[i];
		struct sockaddr_in *s_in = (struct sockaddr_in *)&req->ifr_addr;
		s_in->sin_family = AF_INET;
		err = ioctl(s, SIOCGIFADDR, req);
		if (err < 0)
			continue;
		if (haddr->s_addr == s_in->sin_addr.s_addr)
			break;
	}
	close(s);
	free(conf.ifc_buf);
	return i < numreqs;
}
#endif
