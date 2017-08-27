/* libnetfilter_queue.c: generic library for access to nf_queue
 *
 * (C) 2005 by Harald Welte <laforge@gnumonks.org>
 * (C) 2005, 2008-2010 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 
 *  as published by the Free Software Foundation (or any later at your option)
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  2006-01-23 Andreas Florath <andreas@florath.net>
 *	Fix __set_verdict() that it can now handle payload.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <libnfnetlink/libnfnetlink.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

/**
 * \mainpage
 *
 * libnetfilter_queue is a userspace library providing an API to packets that
 * have been queued by the kernel packet filter. It is is part of a system that
 * deprecates the old ip_queue / libipq mechanism.
 *
 * libnetfilter_queue homepage is:
 * 	http://netfilter.org/projects/libnetfilter_queue/
 *
 * \section Dependencies
 * libnetfilter_queue requires libnfnetlink and a kernel that includes the
 * nfnetlink_queue subsystem (i.e. 2.6.14 or later).
 *
 * \section Main Features
 *  - receiving queued packets from the kernel nfnetlink_queue subsystem
 *  - issuing verdicts and/or reinjecting altered packets to the kernel
 *  nfnetlink_queue subsystem
 * 
 * \section Git Tree
 * The current development version of libnetfilter_queue can be accessed
 * at https://git.netfilter.org/cgi-bin/gitweb.cgi?p=libnetfilter_queue.git;a=summary.
 *
 * \section Privileges
 * You need the CAP_NET_ADMIN capability in order to allow your application
 * to receive from and to send packets to kernel-space.
 *
 * \section Using libnetfilter_queue
 * 
 * To write your own program using libnetfilter_queue, you should start by reading
 * the doxygen documentation (start by \link LibrarySetup \endlink page) and
 * nf-queue.c source file.
 *
 * \section errors ENOBUFS errors in recv()
 *
 * recv() may return -1 and errno is set to ENOBUFS in case that your
 * application is not fast enough to retrieve the packets from the kernel.
 * In that case, you can increase the socket buffer size by means of
 * nfnl_rcvbufsiz(). Although this delays the appearance of ENOBUFS errors,
 * you may hit it again sooner or later. The next section provides some hints
 * on how to obtain the best performance for your application.
 *
 * \section perf Performance
 * To improve your libnetfilter_queue application in terms of performance,
 * you may consider the following tweaks:
 *
 * - increase the default socket buffer size by means of nfnl_rcvbufsiz().
 * - set nice value of your process to -20 (maximum priority).
 * - set the CPU affinity of your process to a spare core that is not used
 * to handle NIC interruptions.
 * - set NETLINK_NO_ENOBUFS socket option to avoid receiving ENOBUFS errors
 * (requires Linux kernel >= 2.6.30).
 * - see --queue-balance option in NFQUEUE target for multi-threaded apps
 * (it requires Linux kernel >= 2.6.31).
 */

struct nfq_handle
{
	struct nfnl_handle *nfnlh;
	struct nfnl_subsys_handle *nfnlssh;
	struct nfq_q_handle *qh_list;
};

struct nfq_q_handle
{
	struct nfq_q_handle *next;
	struct nfq_handle *h;
	u_int16_t id;

	nfq_callback *cb;
	void *data;
};

struct nfq_data {
	struct nfattr **data;
};

int nfq_errno;

/***********************************************************************
 * low level stuff 
 ***********************************************************************/

static void del_qh(struct nfq_q_handle *qh)
{
	struct nfq_q_handle *cur_qh, *prev_qh = NULL;

	for (cur_qh = qh->h->qh_list; cur_qh; cur_qh = cur_qh->next) {
		if (cur_qh == qh) {
			if (prev_qh)
				prev_qh->next = qh->next;
			else
				qh->h->qh_list = qh->next;
			return;
		}
		prev_qh = cur_qh;
	}
}

static void add_qh(struct nfq_q_handle *qh)
{
	qh->next = qh->h->qh_list;
	qh->h->qh_list = qh;
}

static struct nfq_q_handle *find_qh(struct nfq_handle *h, u_int16_t id)
{
	struct nfq_q_handle *qh;

	for (qh = h->qh_list; qh; qh = qh->next) {
		if (qh->id == id)
			return qh;
	}
	return NULL;
}

/* build a NFQNL_MSG_CONFIG message */
	static int
__build_send_cfg_msg(struct nfq_handle *h, u_int8_t command,
		u_int16_t queuenum, u_int16_t pf)
{
	union {
		char buf[NFNL_HEADER_LEN
			+NFA_LENGTH(sizeof(struct nfqnl_msg_config_cmd))];
		struct nlmsghdr nmh;
	} u;
	struct nfqnl_msg_config_cmd cmd;

	nfnl_fill_hdr(h->nfnlssh, &u.nmh, 0, AF_UNSPEC, queuenum,
			NFQNL_MSG_CONFIG, NLM_F_REQUEST|NLM_F_ACK);

	cmd.command = command;
	cmd.pf = htons(pf);
	nfnl_addattr_l(&u.nmh, sizeof(u), NFQA_CFG_CMD, &cmd, sizeof(cmd));

	return nfnl_query(h->nfnlh, &u.nmh);
}

static int __nfq_rcv_pkt(struct nlmsghdr *nlh, struct nfattr *nfa[],
		void *data)
{
	struct nfgenmsg *nfmsg = NLMSG_DATA(nlh);
	struct nfq_handle *h = data;
	u_int16_t queue_num = ntohs(nfmsg->res_id);
	struct nfq_q_handle *qh = find_qh(h, queue_num);
	struct nfq_data nfqa;

	if (!qh)
		return -ENODEV;

	if (!qh->cb)
		return -ENODEV;

	nfqa.data = nfa;

	return qh->cb(qh, nfmsg, &nfqa, qh->data);
}

static struct nfnl_callback pkt_cb = {
	.call		= &__nfq_rcv_pkt,
	.attr_count	= NFQA_MAX,
};

/* public interface */

struct nfnl_handle *nfq_nfnlh(struct nfq_handle *h)
{
	return h->nfnlh;
}

/**
 *
 * \defgroup Queue Queue handling [DEPRECATED]
 *
 * Once libnetfilter_queue library has been initialised (See 
 * \link LibrarySetup \endlink), it is possible to bind the program to a
 * specific queue. This can be done by using nfq_create_queue().
 *
 * The queue can then be tuned via nfq_set_mode() or nfq_set_queue_maxlen().
 * 
 * Here's a little code snippet that create queue numbered 0:
 * \verbatim
	printf("binding this socket to queue '0'\n");
	qh = nfq_create_queue(h,  0, &cb, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
	}

	printf("setting copy_packet mode\n");
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		exit(1);
	}
\endverbatim
 *
 * Next step is the handling of incoming packets which can be done via a loop:
 *
 * \verbatim
	fd = nfq_fd(h);

	while ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {
		printf("pkt received\n");
		nfq_handle_packet(h, buf, rv);
	}
\endverbatim
 * When the decision on a packet has been choosed, the verdict has to be given
 * by calling nfq_set_verdict() or nfq_set_verdict2(). The verdict
 * determines the destiny of the packet as follows:
 *
 *   - NF_DROP discarded the packet
 *   - NF_ACCEPT the packet passes, continue iterations
 *   - NF_QUEUE inject the packet into a different queue
 *     (the target queue number is in the high 16 bits of the verdict)
 *   - NF_REPEAT iterate the same cycle once more
 *   - NF_STOP accept, but don't continue iterations
 *
 * The verdict NF_STOLEN must not be used, as it has special meaning in the
 * kernel.
 * When using NF_REPEAT, one way to prevent re-queueing of the same packet
 * is to also set an nfmark using nfq_set_verdict2, and set up the nefilter
 * rules to only queue a packet when the mark is not (yet) set.
 *
 * Data and information about the packet can be fetch by using message parsing
 * functions (See \link Parsing \endlink).
 * @{
 */

/**
 * nfq_fd - get the file descriptor associated with the nfqueue handler
 * \param h Netfilter queue connection handle obtained via call to nfq_open()
 *
 * \return a file descriptor for the netlink connection associated with the
 * given queue connection handle. The file descriptor can then be used for
 * receiving the queued packets for processing.
 *
  * This function returns a file descriptor that can be used for communication
 * over the netlink connection associated with the given queue connection
 * handle.
 */
int nfq_fd(struct nfq_handle *h)
{
	return nfnl_fd(nfq_nfnlh(h));
}

/**
 * @}
 */

/**
 * \defgroup LibrarySetup Library setup [DEPRECATED]
 *
 * Library initialisation is made in two steps.
 *
 * First step is to call nfq_open() to open a NFQUEUE handler. 
 *
 * Second step is to tell the kernel that userspace queueing is handle by
 * NFQUEUE for the selected protocol. This is made by calling nfq_unbind_pf()
 * and nfq_bind_pf() with protocol information. The idea behind this is to
 * enable simultaneously loaded modules to be used for queuing.
 *
 * Here's a little code snippet that bind with AF_INET:
 * \verbatim
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "error during nfq_open()\n");
		exit(1);
	}

	printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
	if (nfq_unbind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
	}

	printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
	if (nfq_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}
\endverbatim
 * Once this is done, you can setup and use a \link Queue \endlink.
 * @{
 */

/**
 * nfq_open - open a nfqueue handler
 *
 * This function obtains a netfilter queue connection handle. When you are
 * finished with the handle returned by this function, you should destroy
 * it by calling nfq_close(). A new netlink connection is obtained internally
 * and associated with the queue connection handle returned.
 *
 * \return a pointer to a new queue handle or NULL on failure.
 */
struct nfq_handle *nfq_open(void)
{
	struct nfnl_handle *nfnlh = nfnl_open();
	struct nfq_handle *qh;

	if (!nfnlh)
		return NULL;

	/* unset netlink sequence tracking by default */
	nfnl_unset_sequence_tracking(nfnlh);

	qh = nfq_open_nfnl(nfnlh);
	if (!qh)
		nfnl_close(nfnlh);

	return qh;
}

/**
 * @}
 */

/**
 * nfq_open_nfnl - open a nfqueue handler from a existing nfnetlink handler
 * \param nfnlh Netfilter netlink connection handle obtained by calling nfnl_open()
 *
 * This function obtains a netfilter queue connection handle using an existing
 * netlink connection. This function is used internally to implement 
 * nfq_open(), and should typically not be called directly.
 *
 * \return a pointer to a new queue handle or NULL on failure.
 */
struct nfq_handle *nfq_open_nfnl(struct nfnl_handle *nfnlh)
{
	struct nfq_handle *h;
	int err;

	h = malloc(sizeof(*h));
	if (!h)
		return NULL;

	memset(h, 0, sizeof(*h));
	h->nfnlh = nfnlh;

	h->nfnlssh = nfnl_subsys_open(h->nfnlh, NFNL_SUBSYS_QUEUE, 
				      NFQNL_MSG_MAX, 0);
	if (!h->nfnlssh) {
		/* FIXME: nfq_errno */
		goto out_free;
	}

	pkt_cb.data = h;
	err = nfnl_callback_register(h->nfnlssh, NFQNL_MSG_PACKET, &pkt_cb);
	if (err < 0) {
		nfq_errno = err;
		goto out_close;
	}

	return h;
out_close:
	nfnl_subsys_close(h->nfnlssh);
out_free:
	free(h);
	return NULL;
}

/**
 * \addtogroup LibrarySetup
 *
 * When the program has finished with libnetfilter_queue, it has to call
 * the nfq_close() function to free all associated resources.
 *
 * @{
 */

/**
 * nfq_close - close a nfqueue handler
 * \param h Netfilter queue connection handle obtained via call to nfq_open()
 *
 * This function closes the nfqueue handler and free associated resources.
 *
 * \return 0 on success, non-zero on failure. 
 */
int nfq_close(struct nfq_handle *h)
{
	int ret;
	
	ret = nfnl_close(h->nfnlh);
	if (ret == 0)
		free(h);
	return ret;
}

/**
 * nfq_bind_pf - bind a nfqueue handler to a given protocol family
 * \param h Netfilter queue connection handle obtained via call to nfq_open()
 * \param pf protocol family to bind to nfqueue handler obtained from nfq_open()
 *
 * Binds the given queue connection handle to process packets belonging to 
 * the given protocol family (ie. PF_INET, PF_INET6, etc).
 *
 * \return integer inferior to 0 in case of failure
 */
int nfq_bind_pf(struct nfq_handle *h, u_int16_t pf)
{
	return __build_send_cfg_msg(h, NFQNL_CFG_CMD_PF_BIND, 0, pf);
}

/**
 * nfq_unbind_pf - unbind nfqueue handler from a protocol family
 * \param h Netfilter queue connection handle obtained via call to nfq_open()
 * \param pf protocol family to unbind family from
 *
 * Unbinds the given queue connection handle from processing packets belonging
 * to the given protocol family.
 */
int nfq_unbind_pf(struct nfq_handle *h, u_int16_t pf)
{
	return __build_send_cfg_msg(h, NFQNL_CFG_CMD_PF_UNBIND, 0, pf);
}



/**
 * @}
 */

/**
 * \addtogroup Queue
 * @{
 */

/**
 * nfq_create_queue - create a new queue handle and return it.
 *
 * \param h Netfilter queue connection handle obtained via call to nfq_open()
 * \param num the number of the queue to bind to
 * \param cb callback function to call for each queued packet
 * \param data custom data to pass to the callback function
 *
 * \return a nfq_q_handle pointing to the newly created queue
 *
 * Creates a new queue handle, and returns it.  The new queue is identified by
 * #num, and the callback specified by #cb will be called for each enqueued
 * packet.  The #data argument will be passed unchanged to the callback.  If
 * a queue entry with id #num already exists, this function will return failure
 * and the existing entry is unchanged.
 *
 * The nfq_callback type is defined in libnetfilter_queue.h as:
 * \verbatim
typedef int nfq_callback(struct nfq_q_handle *qh,
		    	 struct nfgenmsg *nfmsg,
			 struct nfq_data *nfad, void *data);
\endverbatim
 *
 * Parameters:
 *  - qh The queue handle returned by nfq_create_queue
 *  - nfmsg message objetc that contains the packet
 *  - nfad Netlink packet data handle
 *  - data the value passed to the data parameter of nfq_create_queue
 *
 * The callback should return < 0 to stop processing.
 */

struct nfq_q_handle *nfq_create_queue(struct nfq_handle *h, 
		u_int16_t num,
		nfq_callback *cb,
		void *data)
{
	int ret;
	struct nfq_q_handle *qh;

	if (find_qh(h, num))
		return NULL;

	qh = malloc(sizeof(*qh));

	memset(qh, 0, sizeof(*qh));
	qh->h = h;
	qh->id = num;
	qh->cb = cb;
	qh->data = data;

	ret = __build_send_cfg_msg(h, NFQNL_CFG_CMD_BIND, num, 0);
	if (ret < 0) {
		nfq_errno = ret;
		free(qh);
		return NULL;
	}

	add_qh(qh);
	return qh;
}

/**
 * @}
 */

/**
 * \addtogroup Queue
 * @{
 */

/**
 * nfq_destroy_queue - destroy a queue handle
 * \param qh queue handle that we want to destroy created via nfq_create_queue
 *
 * Removes the binding for the specified queue handle. This call also unbind
 * from the nfqueue handler, so you don't have to call nfq_unbind_pf.
 */
int nfq_destroy_queue(struct nfq_q_handle *qh)
{
	int ret = __build_send_cfg_msg(qh->h, NFQNL_CFG_CMD_UNBIND, qh->id, 0);
	if (ret == 0) {
		del_qh(qh);
		free(qh);
	}

	return ret;
}

/**
 * nfq_handle_packet - handle a packet received from the nfqueue subsystem
 * \param h Netfilter queue connection handle obtained via call to nfq_open()
 * \param buf data to pass to the callback
 * \param len length of packet data in buffer
 *
 * Triggers an associated callback for the given packet received from the
 * queue. Packets can be read from the queue using nfq_fd() and recv(). See
 * example code for nfq_fd().
 *
 * \return 0 on success, non-zero on failure.
 */
int nfq_handle_packet(struct nfq_handle *h, char *buf, int len)
{
	return nfnl_handle_packet(h->nfnlh, buf, len);
}

/**
 * nfq_set_mode - set the amount of packet data that nfqueue copies to userspace
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param mode the part of the packet that we are interested in
 * \param range size of the packet that we want to get
 *
 * Sets the amount of data to be copied to userspace for each packet queued
 * to the given queue.
 *
 * - NFQNL_COPY_NONE - noop, do not use it
 * - NFQNL_COPY_META - copy only packet metadata
 * - NFQNL_COPY_PACKET - copy entire packet
 *
 * \return -1 on error; >=0 otherwise.
 */
int nfq_set_mode(struct nfq_q_handle *qh,
		u_int8_t mode, u_int32_t range)
{
	union {
		char buf[NFNL_HEADER_LEN
			+NFA_LENGTH(sizeof(struct nfqnl_msg_config_params))];
		struct nlmsghdr nmh;
	} u;
	struct nfqnl_msg_config_params params;

	nfnl_fill_hdr(qh->h->nfnlssh, &u.nmh, 0, AF_UNSPEC, qh->id,
			NFQNL_MSG_CONFIG, NLM_F_REQUEST|NLM_F_ACK);

	params.copy_range = htonl(range);
	params.copy_mode = mode;
	nfnl_addattr_l(&u.nmh, sizeof(u), NFQA_CFG_PARAMS, &params,
			sizeof(params));

	return nfnl_query(qh->h->nfnlh, &u.nmh);
}

/**
 * nfq_set_queue_flags - set flags (options) for the kernel queue
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param mask specifies which flag bits to modify
 * \param flag bitmask of flags
 *
 * Here's a little code snippet to show how to use this API:
 * \verbatim
	uint32_t flags = NFQA_CFG_F_FAIL_OPEN;
	uint32_t mask = NFQA_CFG_F_FAIL_OPEN;

	printf("Enabling fail-open on this q\n");
	err = nfq_set_queue_flags(qh, mask, flags);

	printf("Disabling fail-open on this q\n");
	flags &= ~NFQA_CFG_F_FAIL_OPEN;
	err = nfq_set_queue_flags(qh, mask, flags);
\endverbatim
 * \return -1 on error with errno set appropriately; =0 otherwise.
 */
int nfq_set_queue_flags(struct nfq_q_handle *qh,
			uint32_t mask, uint32_t flags)
{
	union {
		char buf[NFNL_HEADER_LEN
			+NFA_LENGTH(sizeof(mask)
			+NFA_LENGTH(sizeof(flags)))];
		struct nlmsghdr nmh;
	} u;

	mask = htonl(mask);
	flags = htonl(flags);

	nfnl_fill_hdr(qh->h->nfnlssh, &u.nmh, 0, AF_UNSPEC, qh->id,
		      NFQNL_MSG_CONFIG, NLM_F_REQUEST|NLM_F_ACK);

	nfnl_addattr32(&u.nmh, sizeof(u), NFQA_CFG_FLAGS, flags);
	nfnl_addattr32(&u.nmh, sizeof(u), NFQA_CFG_MASK, mask);

	return nfnl_query(qh->h->nfnlh, &u.nmh);
}

/**
 * nfq_set_queue_maxlen - Set kernel queue maximum length parameter
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param queuelen the length of the queue
 *
 * Sets the size of the queue in kernel. This fixes the maximum number
 * of packets the kernel will store before internally before dropping
 * upcoming packets.
 *
 * \return -1 on error; >=0 otherwise.
 */
int nfq_set_queue_maxlen(struct nfq_q_handle *qh,
				u_int32_t queuelen)
{
	union {
		char buf[NFNL_HEADER_LEN
			+NFA_LENGTH(sizeof(struct nfqnl_msg_config_params))];
		struct nlmsghdr nmh;
	} u;
	u_int32_t queue_maxlen = htonl(queuelen);

	nfnl_fill_hdr(qh->h->nfnlssh, &u.nmh, 0, AF_UNSPEC, qh->id,
			NFQNL_MSG_CONFIG, NLM_F_REQUEST|NLM_F_ACK);

	nfnl_addattr_l(&u.nmh, sizeof(u), NFQA_CFG_QUEUE_MAXLEN, &queue_maxlen,
			sizeof(queue_maxlen));

	return nfnl_query(qh->h->nfnlh, &u.nmh);
}

/**
 * @}
 */

static int __set_verdict(struct nfq_q_handle *qh, u_int32_t id,
		u_int32_t verdict, u_int32_t mark, int set_mark,
		u_int32_t data_len, const unsigned char *data,
		enum nfqnl_msg_types type)
{
	struct nfqnl_msg_verdict_hdr vh;
	union {
		char buf[NFNL_HEADER_LEN
			+NFA_LENGTH(sizeof(mark))
			+NFA_LENGTH(sizeof(vh))];
		struct nlmsghdr nmh;
	} u;

	struct iovec iov[3];
	int nvecs;

	/* This must be declared here (and not inside the data
	 * handling block) because the iovec points to this. */
	struct nfattr data_attr;

	memset(iov, 0, sizeof(iov));

	vh.verdict = htonl(verdict);
	vh.id = htonl(id);

	nfnl_fill_hdr(qh->h->nfnlssh, &u.nmh, 0, AF_UNSPEC, qh->id,
				type, NLM_F_REQUEST);

	/* add verdict header */
	nfnl_addattr_l(&u.nmh, sizeof(u), NFQA_VERDICT_HDR, &vh, sizeof(vh));

	if (set_mark)
		nfnl_addattr32(&u.nmh, sizeof(u), NFQA_MARK, mark);

	iov[0].iov_base = &u.nmh;
	iov[0].iov_len = NLMSG_TAIL(&u.nmh) - (void *)&u.nmh;
	nvecs = 1;

	if (data_len) {
		/* The typecast here is to cast away data's const-ness: */
		nfnl_build_nfa_iovec(&iov[1], &data_attr, NFQA_PAYLOAD,
				data_len, (unsigned char *) data);
		nvecs += 2;
		/* Add the length of the appended data to the message
		 * header.  The size of the attribute is given in the
		 * nfa_len field and is set in the nfnl_build_nfa_iovec()
		 * function. */
		u.nmh.nlmsg_len += data_attr.nfa_len;
	}

	return nfnl_sendiov(qh->h->nfnlh, iov, nvecs, 0);
}

/**
 * \addtogroup Queue
 * @{
 */

/**
 * nfq_set_verdict - issue a verdict on a packet 
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param id	ID assigned to packet by netfilter.
 * \param verdict verdict to return to netfilter (NF_ACCEPT, NF_DROP)
 * \param data_len number of bytes of data pointed to by #buf
 * \param buf the buffer that contains the packet data
 *
 * Can be obtained by: 
 * \verbatim
	int id;
	struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(tb);
	if (ph)
 		id = ntohl(ph->packet_id);
\endverbatim
 *
 * Notifies netfilter of the userspace verdict for the given packet.  Every
 * queued packet _must_ have a verdict specified by userspace, either by
 * calling this function, the nfq_set_verdict2() function, or the _batch
 * versions of these functions.
 *
 * \return -1 on error; >= 0 otherwise.
 */
int nfq_set_verdict(struct nfq_q_handle *qh, u_int32_t id,
		u_int32_t verdict, u_int32_t data_len, 
		const unsigned char *buf)
{
	return __set_verdict(qh, id, verdict, 0, 0, data_len, buf,
						NFQNL_MSG_VERDICT);
}	

/**
 * nfq_set_verdict2 - like nfq_set_verdict, but you can set the mark.
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param id	ID assigned to packet by netfilter.
 * \param verdict verdict to return to netfilter (NF_ACCEPT, NF_DROP)
 * \param mark mark to put on packet
 * \param data_len number of bytes of data pointed to by #buf
 * \param buf the buffer that contains the packet data
 */
int nfq_set_verdict2(struct nfq_q_handle *qh, u_int32_t id,
		     u_int32_t verdict, u_int32_t mark,
		     u_int32_t data_len, const unsigned char *buf)
{
	return __set_verdict(qh, id, verdict, htonl(mark), 1, data_len,
						buf, NFQNL_MSG_VERDICT);
}

/**
 * nfq_set_verdict_batch - issue verdicts on several packets at once
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param id maximum ID of the packets that the verdict should be applied to.
 * \param verdict verdict to return to netfilter (NF_ACCEPT, NF_DROP)
 *
 * Unlike nfq_set_verdict, the verdict is applied to all queued packets
 * whose packet id is smaller or equal to #id.
 *
 * batch support was added in Linux 3.1.
 * These functions will fail silently on older kernels.
 */
int nfq_set_verdict_batch(struct nfq_q_handle *qh, u_int32_t id,
					  u_int32_t verdict)
{
	return __set_verdict(qh, id, verdict, 0, 0, 0, NULL,
					NFQNL_MSG_VERDICT_BATCH);
}

/**
 * nfq_set_verdict_batch2 - like nfq_set_verdict_batch, but you can set a mark.
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param id maximum ID of the packets that the verdict should be applied to.
 * \param verdict verdict to return to netfilter (NF_ACCEPT, NF_DROP)
 * \param mark mark to put on packet
 */
int nfq_set_verdict_batch2(struct nfq_q_handle *qh, u_int32_t id,
		     u_int32_t verdict, u_int32_t mark)
{
	return __set_verdict(qh, id, verdict, htonl(mark), 1, 0,
				NULL, NFQNL_MSG_VERDICT_BATCH);
}

/**
 * nfq_set_verdict_mark - like nfq_set_verdict, but you can set the mark.
 * \param qh Netfilter queue handle obtained by call to nfq_create_queue().
 * \param id	ID assigned to packet by netfilter.
 * \param verdict verdict to return to netfilter (NF_ACCEPT, NF_DROP)
 * \param mark the mark to put on the packet, in network byte order.
 * \param data_len number of bytes of data pointed to by #buf
 * \param buf the buffer that contains the packet data
 *
 * \return -1 on error; >= 0 otherwise.
 *
 * This function is deprecated since it is broken, its use is highly
 * discouraged. Please, use nfq_set_verdict2 instead.
 */
int nfq_set_verdict_mark(struct nfq_q_handle *qh, u_int32_t id,
		u_int32_t verdict, u_int32_t mark,
		u_int32_t data_len, const unsigned char *buf)
{
	return __set_verdict(qh, id, verdict, mark, 1, data_len, buf,
						NFQNL_MSG_VERDICT);
}

/**
 * @}
 */



/*************************************************************
 * Message parsing functions 
 *************************************************************/

/**
 * \defgroup Parsing Message parsing functions [DEPRECATED]
 * @{
 */

/**
 * nfqnl_msg_packet_hdr - return the metaheader that wraps the packet
 * \param nfad Netlink packet data handle passed to callback function
 *
 * \return the netfilter queue netlink packet header for the given
 * nfq_data argument.  Typically, the nfq_data value is passed as the 3rd
 * parameter to the callback function set by a call to nfq_create_queue().
  *
 * The nfqnl_msg_packet_hdr structure is defined in libnetfilter_queue.h as:
 *
 * \verbatim
	struct nfqnl_msg_packet_hdr {
		u_int32_t	packet_id;	// unique ID of packet in queue
		u_int16_t	hw_protocol;	// hw protocol (network order)
		u_int8_t	hook;		// netfilter hook
	} __attribute__ ((packed));
\endverbatim
 */
struct nfqnl_msg_packet_hdr *nfq_get_msg_packet_hdr(struct nfq_data *nfad)
{
	return nfnl_get_pointer_to_data(nfad->data, NFQA_PACKET_HDR,
					struct nfqnl_msg_packet_hdr);
}

/**
 * nfq_get_nfmark - get the packet mark
 * \param nfad Netlink packet data handle passed to callback function
 *
 * \return the netfilter mark currently assigned to the given queued packet.
 */
uint32_t nfq_get_nfmark(struct nfq_data *nfad)
{
	return ntohl(nfnl_get_data(nfad->data, NFQA_MARK, u_int32_t));
}

/**
 * nfq_get_timestamp - get the packet timestamp
 * \param nfad Netlink packet data handle passed to callback function
 * \param tv structure to fill with timestamp info
 *
 * Retrieves the received timestamp when the given queued packet.
 *
 * \return 0 on success, non-zero on failure.
 */
int nfq_get_timestamp(struct nfq_data *nfad, struct timeval *tv)
{
	struct nfqnl_msg_packet_timestamp *qpt;
	qpt = nfnl_get_pointer_to_data(nfad->data, NFQA_TIMESTAMP,
					struct nfqnl_msg_packet_timestamp);
	if (!qpt)
		return -1;

	tv->tv_sec = __be64_to_cpu(qpt->sec);
	tv->tv_usec = __be64_to_cpu(qpt->usec);

	return 0;
}

/**
 * nfq_get_indev - get the interface that the packet was received through
 * \param nfad Netlink packet data handle passed to callback function
 *
 * \return The index of the device the queued packet was received via.  If the
 * returned index is 0, the packet was locally generated or the input
 * interface is not known (ie. POSTROUTING?).
 *
 * \warning all nfq_get_dev() functions return 0 if not set, since linux
 * only allows ifindex >= 1, see net/core/dev.c:2600  (in 2.6.13.1)
 */
u_int32_t nfq_get_indev(struct nfq_data *nfad)
{
	return ntohl(nfnl_get_data(nfad->data, NFQA_IFINDEX_INDEV, u_int32_t));
}

/**
 * nfq_get_physindev - get the physical interface that the packet was received
 * \param nfad Netlink packet data handle passed to callback function
 *
 * \return The index of the physical device the queued packet was received via.
 * If the returned index is 0, the packet was locally generated or the
 * physical input interface is no longer known (ie. POSTROUTING?).
 */
u_int32_t nfq_get_physindev(struct nfq_data *nfad)
{
	return ntohl(nfnl_get_data(nfad->data, NFQA_IFINDEX_PHYSINDEV, u_int32_t));
}

/**
 * nfq_get_outdev - gets the interface that the packet will be routed out
 * \param nfad Netlink packet data handle passed to callback function
 *
 * \return The index of the device the queued packet will be sent out.  If the
 * returned index is 0, the packet is destined for localhost or the output
 * interface is not yet known (ie. PREROUTING?).
 */
u_int32_t nfq_get_outdev(struct nfq_data *nfad)
{
	return ntohl(nfnl_get_data(nfad->data, NFQA_IFINDEX_OUTDEV, u_int32_t));
}

/**
 * nfq_get_physoutdev - get the physical interface that the packet output
 * \param nfad Netlink packet data handle passed to callback function
 *
 * The index of the physical device the queued packet will be sent out.
 * If the returned index is 0, the packet is destined for localhost or the
 * physical output interface is not yet known (ie. PREROUTING?).
 * 
 * \return The index of physical interface that the packet output will be routed out.
 */
u_int32_t nfq_get_physoutdev(struct nfq_data *nfad)
{
	return ntohl(nfnl_get_data(nfad->data, NFQA_IFINDEX_PHYSOUTDEV, u_int32_t));
}

/**
 * nfq_get_indev_name - get the name of the interface the packet
 * was received through
 * \param nlif_handle pointer to a nlif interface resolving handle
 * \param nfad Netlink packet data handle passed to callback function
 * \param name pointer to the buffer to receive the interface name;
 *  not more than \c IFNAMSIZ bytes will be copied to it.
 * \return -1 in case of error, >0 if it succeed. 
 *
 * To use a nlif_handle, You need first to call nlif_open() and to open
 * an handler. Don't forget to store the result as it will be used 
 * during all your program life:
 * \verbatim
	h = nlif_open();
 	if (h == NULL) {
 		perror("nlif_open");
 		exit(EXIT_FAILURE);
 	}
\endverbatim
 * Once the handler is open, you need to fetch the interface table at a
 * whole via a call to nlif_query.
 * \verbatim
  	nlif_query(h);
\endverbatim
 * libnfnetlink is able to update the interface mapping when a new interface
 * appears. To do so, you need to call nlif_catch() on the handler after each
 * interface related event. The simplest way to get and treat event is to run
 * a select() or poll() against the nlif file descriptor. To get this file 
 * descriptor, you need to use nlif_fd:
 * \verbatim
 	if_fd = nlif_fd(h);
\endverbatim
 * Don't forget to close the handler when you don't need the feature anymore:
 * \verbatim
 	nlif_close(h);
\endverbatim
 *
 */
int nfq_get_indev_name(struct nlif_handle *nlif_handle,
			struct nfq_data *nfad, char *name)
{
	u_int32_t ifindex = nfq_get_indev(nfad);
	return nlif_index2name(nlif_handle, ifindex, name);
}

/**
 * nfq_get_physindev_name - get the name of the physical interface the
 * packet was received through
 * \param nlif_handle pointer to a nlif interface resolving handle
 * \param nfad Netlink packet data handle passed to callback function
 * \param name pointer to the buffer to receive the interface name;
 *  not more than \c IFNAMSIZ bytes will be copied to it.
 *
 * See nfq_get_indev_name() documentation for nlif_handle usage.
 *
 * \return  -1 in case of error, > 0 if it succeed. 
 */
int nfq_get_physindev_name(struct nlif_handle *nlif_handle,
			   struct nfq_data *nfad, char *name)
{
	u_int32_t ifindex = nfq_get_physindev(nfad);
	return nlif_index2name(nlif_handle, ifindex, name);
}

/**
 * nfq_get_outdev_name - get the name of the physical interface the
 * packet will be sent to
 * \param nlif_handle pointer to a nlif interface resolving handle
 * \param nfad Netlink packet data handle passed to callback function
 * \param name pointer to the buffer to receive the interface name;
 *  not more than \c IFNAMSIZ bytes will be copied to it.
 *
 * See nfq_get_indev_name() documentation for nlif_handle usage.
 *
 * \return  -1 in case of error, > 0 if it succeed. 
 */
int nfq_get_outdev_name(struct nlif_handle *nlif_handle,
			struct nfq_data *nfad, char *name)
{
	u_int32_t ifindex = nfq_get_outdev(nfad);
	return nlif_index2name(nlif_handle, ifindex, name);
}

/**
 * nfq_get_physoutdev_name - get the name of the interface the
 * packet will be sent to
 * \param nlif_handle pointer to a nlif interface resolving handle
 * \param nfad Netlink packet data handle passed to callback function
 * \param name pointer to the buffer to receive the interface name;
 *  not more than \c IFNAMSIZ bytes will be copied to it.
 *
 * See nfq_get_indev_name() documentation for nlif_handle usage.
 *
 * \return  -1 in case of error, > 0 if it succeed. 
 */

int nfq_get_physoutdev_name(struct nlif_handle *nlif_handle,
			    struct nfq_data *nfad, char *name)
{
	u_int32_t ifindex = nfq_get_physoutdev(nfad);
	return nlif_index2name(nlif_handle, ifindex, name);
}

/**
 * nfq_get_packet_hw
 *
 * get hardware address 
 *
 * \param nfad Netlink packet data handle passed to callback function
 *
 * Retrieves the hardware address associated with the given queued packet.
 * For ethernet packets, the hardware address returned (if any) will be the
 * MAC address of the packet source host.  The destination MAC address is not
 * known until after POSTROUTING and a successful ARP request, so cannot
 * currently be retrieved.
 *
 * The nfqnl_msg_packet_hw structure is defined in libnetfilter_queue.h as:
 * \verbatim
	struct nfqnl_msg_packet_hw {
		u_int16_t	hw_addrlen;
		u_int16_t	_pad;
		u_int8_t	hw_addr[8];
	} __attribute__ ((packed));
\endverbatim
 */
struct nfqnl_msg_packet_hw *nfq_get_packet_hw(struct nfq_data *nfad)
{
	return nfnl_get_pointer_to_data(nfad->data, NFQA_HWADDR,
					struct nfqnl_msg_packet_hw);
}

/**
 * nfq_get_payload - get payload 
 * \param nfad Netlink packet data handle passed to callback function
 * \param data Pointer of pointer that will be pointed to the payload
 *
 * Retrieve the payload for a queued packet. The actual amount and type of
 * data retrieved by this function will depend on the mode set with the
 * nfq_set_mode() function.
 *
 * \return -1 on error, otherwise > 0.
 */
int nfq_get_payload(struct nfq_data *nfad, unsigned char **data)
{
	*data = (unsigned char *)
		nfnl_get_pointer_to_data(nfad->data, NFQA_PAYLOAD, char);
	if (*data)
		return NFA_PAYLOAD(nfad->data[NFQA_PAYLOAD-1]);

	return -1;
}

/**
 * @}
 */

#define SNPRINTF_FAILURE(ret, rem, offset, len)			\
do {								\
	if (ret < 0)						\
		return ret;					\
	len += ret;						\
	if (ret > rem)						\
		ret = rem;					\
	offset += ret;						\
	rem -= ret;						\
} while (0)

/**
 * \defgroup Printing Printing [DEPRECATED]
 * @{
 */

/**
 * nfq_snprintf_xml - print the enqueued packet in XML format into a buffer
 * \param buf The buffer that you want to use to print the logged packet
 * \param rem The size of the buffer that you have passed
 * \param tb Netlink packet data handle passed to callback function
 * \param flags The flag that tell what to print into the buffer
 *
 * This function supports the following flags:
 *
 *	- NFQ_XML_HW: include the hardware link layer address
 *	- NFQ_XML_MARK: include the packet mark
 *	- NFQ_XML_DEV: include the device information
 *	- NFQ_XML_PHYSDEV: include the physical device information
 *	- NFQ_XML_PAYLOAD: include the payload (in hexadecimal)
 *	- NFQ_XML_TIME: include the timestamp
 *	- NFQ_XML_ALL: include all the logging information (all flags set)
 *
 * You can combine this flags with an binary OR.
 *
 * \return -1 in case of failure, otherwise the length of the string that
 * would have been printed into the buffer (in case that there is enough
 * room in it). See snprintf() return value for more information.
 */
int nfq_snprintf_xml(char *buf, size_t rem, struct nfq_data *tb, int flags)
{
	struct nfqnl_msg_packet_hdr *ph;
	struct nfqnl_msg_packet_hw *hwph;
	u_int32_t mark, ifi;
	int size, offset = 0, len = 0, ret;
	unsigned char *data;

	size = snprintf(buf + offset, rem, "<pkt>");
	SNPRINTF_FAILURE(size, rem, offset, len);

	if (flags & NFQ_XML_TIME) {
		time_t t;
		struct tm tm;

		t = time(NULL);
		if (localtime_r(&t, &tm) == NULL)
			return -1;

		size = snprintf(buf + offset, rem, "<when>");
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset, rem,
				"<hour>%d</hour>", tm.tm_hour);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset,
				rem, "<min>%02d</min>", tm.tm_min);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset,
				rem, "<sec>%02d</sec>", tm.tm_sec);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset, rem, "<wday>%d</wday>",
				tm.tm_wday + 1);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset, rem, "<day>%d</day>", tm.tm_mday);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset, rem, "<month>%d</month>",
				tm.tm_mon + 1);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset, rem, "<year>%d</year>",
				1900 + tm.tm_year);
		SNPRINTF_FAILURE(size, rem, offset, len);

		size = snprintf(buf + offset, rem, "</when>");
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	ph = nfq_get_msg_packet_hdr(tb);
	if (ph) {
		size = snprintf(buf + offset, rem,
				"<hook>%u</hook><id>%u</id>",
				ph->hook, ntohl(ph->packet_id));
		SNPRINTF_FAILURE(size, rem, offset, len);

		hwph = nfq_get_packet_hw(tb);
		if (hwph && (flags & NFQ_XML_HW)) {
			int i, hlen = ntohs(hwph->hw_addrlen);

			size = snprintf(buf + offset, rem, "<hw><proto>%04x"
							   "</proto>",
					ntohs(ph->hw_protocol));
			SNPRINTF_FAILURE(size, rem, offset, len);

			size = snprintf(buf + offset, rem, "<src>");
			SNPRINTF_FAILURE(size, rem, offset, len);

			for (i=0; i<hlen; i++) {
				size = snprintf(buf + offset, rem, "%02x",
						hwph->hw_addr[i]);
				SNPRINTF_FAILURE(size, rem, offset, len);
			}

			size = snprintf(buf + offset, rem, "</src></hw>");
			SNPRINTF_FAILURE(size, rem, offset, len);
		} else if (flags & NFQ_XML_HW) {
			size = snprintf(buf + offset, rem, "<hw><proto>%04x"
						    "</proto></hw>",
				 ntohs(ph->hw_protocol));
			SNPRINTF_FAILURE(size, rem, offset, len);
		}
	}

	mark = nfq_get_nfmark(tb);
	if (mark && (flags & NFQ_XML_MARK)) {
		size = snprintf(buf + offset, rem, "<mark>%u</mark>", mark);
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	ifi = nfq_get_indev(tb);
	if (ifi && (flags & NFQ_XML_DEV)) {
		size = snprintf(buf + offset, rem, "<indev>%u</indev>", ifi);
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	ifi = nfq_get_outdev(tb);
	if (ifi && (flags & NFQ_XML_DEV)) {
		size = snprintf(buf + offset, rem, "<outdev>%u</outdev>", ifi);
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	ifi = nfq_get_physindev(tb);
	if (ifi && (flags & NFQ_XML_PHYSDEV)) {
		size = snprintf(buf + offset, rem,
				"<physindev>%u</physindev>", ifi);
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	ifi = nfq_get_physoutdev(tb);
	if (ifi && (flags & NFQ_XML_PHYSDEV)) {
		size = snprintf(buf + offset, rem,
				"<physoutdev>%u</physoutdev>", ifi);
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	ret = nfq_get_payload(tb, &data);
	if (ret >= 0 && (flags & NFQ_XML_PAYLOAD)) {
		int i;

		size = snprintf(buf + offset, rem, "<payload>");
		SNPRINTF_FAILURE(size, rem, offset, len);

		for (i=0; i<ret; i++) {
			size = snprintf(buf + offset, rem, "%02x",
					data[i] & 0xff);
			SNPRINTF_FAILURE(size, rem, offset, len);
		}

		size = snprintf(buf + offset, rem, "</payload>");
		SNPRINTF_FAILURE(size, rem, offset, len);
	}

	size = snprintf(buf + offset, rem, "</pkt>");
	SNPRINTF_FAILURE(size, rem, offset, len);

	return len;
}

/**
 * @}
 */
