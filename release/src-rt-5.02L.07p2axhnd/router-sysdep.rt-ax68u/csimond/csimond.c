/*
 * Broadcom CSI Monitor Sample Application
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id$
 */

/*
 * csimond.c : Channel State Information (CSI) Monitor daemon
 *
 * This userspace program demonstrates a sample application that reads CSI
 * records from kernel.
 *
 * This sample application uses netlink socket to read periodic CSI records
 * from kernel, specifically from DHD kernel module. It then prints the record
 * on the console. Each record is 2KB long. It consists of a CSI header that is
 * 64 bytes long and a CSI report that could be up to 1952 bytes long. This
 * sample application prints the first 64 + 512 bytes the CSI record. A
 * real-life application will use these periodic CSI records for further
 * analysis.
 *
 * This application receives the records in a multicast group. If for some
 * reason, the application restarts with a different process id, it can just
 * subscribe to the CSIMON multicast group and start getting the subsequent
 * records.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <linux/netlink.h>

#if !defined(NETLINK_CSIMON)
#define NETLINK_CSIMON 23	/* netlink subsystem for CSI Monitor */
#endif // endif
#define CSIMON_GRP_BIT 2	/* Multicast group bit for CSI record transfer */

/* MAX_PAYLOAD contains CSI record whihc is made up of CSI report (up to 1952B)
 * and CSI header (64B)
 */
#define MAX_PAYLOAD 2048

#define DEFAULT_REC_SIZE 2
#define DEFAULT_MSG_FREQ 100

/* Globals for this sample appliaction */
struct sockaddr_nl src_addr, nl_addr;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
unsigned int nl_socket_id; /* Netlink socket subsystem id */

static inline void /* Print first N bytes of a CSI record on console */
csimon_console_print(unsigned int rec_size, unsigned int *csi_rec)
{
	//unsigned int mem_len = 128 + 16; /* 512B for report and 64B for header */
	//unsigned int mem_len = 16 + 16; /* 64B for report and 64B for header */
	/* We are printing 4-byte words one by one with 8 such words on a line */
	unsigned int mem_len = rec_size*8; /* 64B for header, rest for report */
	unsigned int num_col = 8;
	int i, j;

	printf("CSI record:\n");
	/* Dump the CSI record to the console */
	for (i = 0; i < (mem_len / num_col); i++) {
		for (j = 0; j < num_col; j++) {
			printf("0x%08x\t", csi_rec[i * num_col + j]);
		}
		printf("\n");
	}
	return;
}

static int  /* Open a netlink socket for CSI record transfer */
csi_nl_sock_open()
{
	int sock_fd;

	/* Create a netlink socket */
	sock_fd = socket(AF_NETLINK, SOCK_RAW, nl_socket_id);
	if (sock_fd < 0) {
		printf("Netlink socket %d create failed\n", nl_socket_id);
		return (-1);
	}
	/* Associate a local address with the opened socket */
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	src_addr.nl_groups = CSIMON_GRP_BIT;  /* mcast group */
	if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
		printf("Netlink socket %d bind failed\n", nl_socket_id);
		close(sock_fd);
		return (-1);
	}
	return sock_fd;
} /* csi_nl_sock_open() */

static int  /* Read CSI records from kernel/DHD continuously */
csi_rec_continuous_read(int sock_fd, unsigned int rec_size, unsigned int msg_freq)
{
	int sz = 0;
	struct timeval tv;
	static unsigned int rec_cnt = 0;

	if (rec_size == 0 && msg_freq == 0) {
		printf("Both rec_size and msg_freq cannot be zero!\n");
		return (-1);
	}
	/* Allocate memory for receiving CSI record */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (nlh == NULL) {
		printf("Out of memory allocating record buffer\n");
		close(sock_fd);
		return (-1);
	}

	/* Fill in the netlink message header */
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	/* Fill in the socket msg structure */
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&nl_addr;
	msg.msg_namelen = sizeof(nl_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	//printf("%d: dest_addr nl_family %d nl_pid %d nl_groups %d \n", __LINE__,
		//   nl_addr.nl_family, nl_addr.nl_pid, nl_addr.nl_groups);

	/* Read the message from kernel */
	while (1) {
		rec_cnt++;

		/* The following gettimeofday() calls are used to check the approx
		   amount of time it takes to receive the CSI record
		 */
		gettimeofday(&tv, 0);
		//printf("%d: Waiting for msg from kernel %u.%u\n",
		//       __LINE__, tv.tv_sec, tv.tv_usec);
		sz = recvmsg(sock_fd, &msg, 0);
		gettimeofday(&tv, 0);
		if (sz <= 0) {
			printf("%d:recvmsg returned %d\n", __LINE__, sz);
			/* can possibly return: return(-1); */
		}
		/* This sample application outputs the first 574 bytes of the CSI
		 * record to the console. Actual application will use it for further
		 * analysis.
		 */
		if (rec_size != 0) {
			csimon_console_print(rec_size, (unsigned int *)NLMSG_DATA(nlh));
		} else if (rec_cnt % msg_freq == 0) {
			printf("Recd msg %u at %u.%u payld sz %d nl id %d msgflags %d \n",
			  rec_cnt, tv.tv_sec, tv.tv_usec, sz, nl_socket_id, msg.msg_flags);
		}

		/* This application sleeps for getting manageable console output */
		//sleep(1);
	}

	/* Clean up */
	free(nlh);
	close(sock_fd);

	return 0;
} /* csi_rec_continuous_read() */

int main(int argc, char **argv)
{
	int sock_fd, ret;
	unsigned int rec_size = 0, msg_freq = 0;
	char *endptr;

	/* Process the arguments: record size and output msg frequency */
	if (argc == 1) {
		rec_size = 0;
		msg_freq = DEFAULT_MSG_FREQ;
		nl_socket_id = NETLINK_CSIMON;
	} else if (argc == 2) {
		/* only nl_socket_id provided */
		nl_socket_id = strtoul(argv[1], &endptr, 0);
		if ((nl_socket_id > 511) || (*endptr != '\0')) {
			printf("%d:Invalid socket id; should be between 0 and 511: %s\n",
				   __LINE__, argv[1]);
			exit(-1);
		}
		rec_size = 0;
		msg_freq = DEFAULT_MSG_FREQ;
	} else if (argc == 3) {
		nl_socket_id = strtoul(argv[1], &endptr, 0);
		if ((nl_socket_id > 511) || (*endptr != '\0')) {
			printf("%d:Invalid socket id; should be between 0 and 511: %d\n",
				   __LINE__, argv[1]);
			exit(-1);
		}
		rec_size = strtoul(argv[2], &endptr, 0);
		if ((rec_size > 64) || (rec_size < 1) || (*endptr != '\0')) {
			printf("Invalid rec sz; Using dflt %d\n", DEFAULT_REC_SIZE);
			printf("Correct format: csimond [<nl id>][<record size: 0-64>] "
				   "[<message frequency: 1-100000 records>]\n");
			printf("If record size is 0, it will display the messages after "
				   "each <message frequency> records are read\n");
			rec_size = DEFAULT_REC_SIZE;
		}
	} else { // args is at least 4; ignore 5th onward if any
		nl_socket_id = strtoul(argv[1], &endptr, 0);
		if ((nl_socket_id > 511) || (*endptr != '\0')) {
			printf("%d:Invalid socket id; should be between 0 and 511: %d\n",
				   __LINE__, argv[1]);
			exit(-1);
		}
		rec_size = strtoul(argv[2], &endptr, 0);
		if (rec_size == 0) {
			/* Read the arg2 only if arg1 is 0 */
			msg_freq = strtoul(argv[3], &endptr, 0);
			if ((msg_freq < 1) || (msg_freq > 100000) || (*endptr != '\0')) {
				printf("Invalid msg freq; Using dflt %d\n", DEFAULT_MSG_FREQ);
				msg_freq = DEFAULT_MSG_FREQ;
			}
		} else if ((rec_size > 64) || (*endptr != '\0')) {
			printf("Invalid rec sz; Using dflt %d\n", DEFAULT_REC_SIZE);
			rec_size = DEFAULT_REC_SIZE;
		}
	}
	printf("CSIMOND application started with record size %u and message "
		   "frequency %u using nl subsystem %d\n", 32*rec_size, msg_freq,
		   nl_socket_id);

	/* Open netlink socket to recive CSI records */
	sock_fd = csi_nl_sock_open();
	if (sock_fd < 0) {
		printf("%d:csi_nl_sock_open() returned %d\n", __LINE__, sock_fd);
		exit(-1);
	}

	/* Keep reading CSI records from kernel/DHD */
	ret = csi_rec_continuous_read(sock_fd, rec_size, msg_freq);

	return ret;
} /* main() */
