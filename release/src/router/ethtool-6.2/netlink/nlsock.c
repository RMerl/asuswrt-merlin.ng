/*
 * nlsock.c - netlink socket
 *
 * Data structure and code for netlink socket abstraction.
 */

#include <stdint.h>
#include <errno.h>

#include "../internal.h"
#include "nlsock.h"
#include "netlink.h"
#include "prettymsg.h"

#define NLSOCK_RECV_BUFFSIZE 65536

static void ctrl_msg_summary(const struct nlmsghdr *nlhdr)
{
	const struct nlmsgerr *nlerr;

	switch (nlhdr->nlmsg_type) {
	case NLMSG_NOOP:
		printf(" noop\n");
		break;
	case NLMSG_ERROR:
		printf(" error");
		if (nlhdr->nlmsg_len < NLMSG_HDRLEN + sizeof(*nlerr)) {
			printf(" malformed\n");
			break;
		}
		nlerr = mnl_nlmsg_get_payload(nlhdr);
		printf(" errno=%d\n", nlerr->error);
		break;
	case NLMSG_DONE:
		printf(" done\n");
		break;
	case NLMSG_OVERRUN:
		printf(" overrun\n");
		break;
	default:
		printf(" type %u\n", nlhdr->nlmsg_type);
		break;
	}
}

static void genl_msg_summary(const struct nlmsghdr *nlhdr, int ethnl_fam,
			     bool outgoing, bool pretty)
{
	if (nlhdr->nlmsg_type == ethnl_fam) {
		const struct pretty_nlmsg_desc *msg_desc;
		const struct genlmsghdr *ghdr;
		unsigned int n_desc;

		printf(" ethool");
		if (nlhdr->nlmsg_len < NLMSG_HDRLEN + GENL_HDRLEN) {
			printf(" malformed\n");
			return;
		}
		ghdr = mnl_nlmsg_get_payload(nlhdr);

		msg_desc = outgoing ? ethnl_umsg_desc : ethnl_kmsg_desc;
		n_desc = outgoing ? ethnl_umsg_n_desc : ethnl_kmsg_n_desc;
		if (ghdr->cmd < n_desc && msg_desc[ghdr->cmd].name)
			printf(" %s", msg_desc[ghdr->cmd].name);
		else
			printf(" cmd %u", ghdr->cmd);
		fputc('\n', stdout);

		if (pretty)
			pretty_print_genlmsg(nlhdr, msg_desc, n_desc, 0);
		return;
	}

	if (nlhdr->nlmsg_type == GENL_ID_CTRL) {
		printf(" genl-ctrl\n");
		if (pretty)
			pretty_print_genlmsg(nlhdr, genlctrl_msg_desc,
					     genlctrl_msg_n_desc, 0);
	} else {
		fputc('\n', stdout);
		if (pretty)
			pretty_print_genlmsg(nlhdr, NULL, 0, 0);
	}
}

static void rtnl_msg_summary(const struct nlmsghdr *nlhdr, bool pretty)
{
	unsigned int type = nlhdr->nlmsg_type;

	if (type < rtnl_msg_n_desc && rtnl_msg_desc[type].name)
		printf(" %s\n", rtnl_msg_desc[type].name);
	else
		printf(" type %u\n", type);

	if (pretty)
		pretty_print_rtnlmsg(nlhdr, 0);
}

static void debug_msg_summary(const struct nlmsghdr *nlhdr, int ethnl_fam,
			      int nl_fam, bool outgoing, bool pretty)
{
	printf("    msg length %u", nlhdr->nlmsg_len);

	if (nlhdr->nlmsg_type < NLMSG_MIN_TYPE) {
		ctrl_msg_summary(nlhdr);
		return;
	}

	switch(nl_fam) {
	case NETLINK_GENERIC:
		genl_msg_summary(nlhdr, ethnl_fam, outgoing, pretty);
		break;
	case NETLINK_ROUTE:
		rtnl_msg_summary(nlhdr, pretty);
		break;
	default:
		fputc('\n', stdout);
		break;
	}
}

static void debug_msg(struct nl_socket *nlsk, const void *msg, unsigned int len,
		      bool outgoing)
{
	const char *dirlabel = outgoing ? "sending" : "received";
	uint32_t debug = nlsk->nlctx->ctx->debug;
	const struct nlmsghdr *nlhdr = msg;
	bool summary, dump, pretty;
	const char *nl_fam_label;
	int left = len;

	summary = debug_on(debug, DEBUG_NL_MSGS);
	dump = debug_on(debug,
			outgoing ? DEBUG_NL_DUMP_SND : DEBUG_NL_DUMP_RCV);
	pretty = debug_on(debug, DEBUG_NL_PRETTY_MSG);
	if (!summary && !dump)
		return;
	switch(nlsk->nl_fam) {
	case NETLINK_GENERIC:
		nl_fam_label = "genetlink";
		break;
	case NETLINK_ROUTE:
		nl_fam_label = "rtnetlink";
		break;
	default:
		nl_fam_label = "netlink";
		break;
	}
	printf("%s %s packet (%u bytes):\n", dirlabel, nl_fam_label, len);

	while (nlhdr && left > 0 && mnl_nlmsg_ok(nlhdr, left)) {
		if (summary)
			debug_msg_summary(nlhdr, nlsk->nlctx->ethnl_fam,
					  nlsk->nl_fam, outgoing, pretty);
		if (dump)
			mnl_nlmsg_fprintf(stdout, nlhdr, nlhdr->nlmsg_len,
					  GENL_HDRLEN);

		nlhdr = mnl_nlmsg_next(nlhdr, &left);
	}
}

/**
 * nlsock_process_ack() - process NLMSG_ERROR message from kernel
 * @nlhdr:          pointer to netlink header
 * @len:            length of received data (from nlhdr to end of buffer)
 * @suppress_nlerr: 0 show all errors, 1 silence -EOPNOTSUPP, 2 silence all
 *
 * Return: error code extracted from the message
 */
static int nlsock_process_ack(struct nlmsghdr *nlhdr, unsigned long len,
			      unsigned int suppress_nlerr, bool pretty)
{
	const struct nlattr *tb[NLMSGERR_ATTR_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	unsigned int err_offset = 0;
	unsigned int tlv_offset;
	struct nlmsgerr *nlerr;
	bool silent;

	if ((len < NLMSG_HDRLEN + sizeof(*nlerr)) || (len < nlhdr->nlmsg_len))
		return -EFAULT;
	nlerr = mnl_nlmsg_get_payload(nlhdr);
	silent = suppress_nlerr >= 2 ||
		(suppress_nlerr && nlerr->error == -EOPNOTSUPP);
	if (silent || !(nlhdr->nlmsg_flags & NLM_F_ACK_TLVS))
		goto tlv_done;

	tlv_offset = sizeof(*nlerr);
	if (!(nlhdr->nlmsg_flags & NLM_F_CAPPED))
		tlv_offset += MNL_ALIGN(mnl_nlmsg_get_payload_len(&nlerr->msg));
	if (mnl_attr_parse(nlhdr, tlv_offset, attr_cb, &tb_info) < 0)
		goto tlv_done;

	if (tb[NLMSGERR_ATTR_MSG]) {
		const char *msg = mnl_attr_get_str(tb[NLMSGERR_ATTR_MSG]);

		fprintf(stderr, "netlink %s: %s",
			nlerr->error ? "error" : "warning", msg);
		if (!pretty && tb[NLMSGERR_ATTR_OFFS])
			fprintf(stderr, " (offset %u)",
				mnl_attr_get_u32(tb[NLMSGERR_ATTR_OFFS]));
		fputc('\n', stderr);
	}
	if (tb[NLMSGERR_ATTR_OFFS])
		err_offset = mnl_attr_get_u32(tb[NLMSGERR_ATTR_OFFS]);

tlv_done:
	if (nlerr->error && !silent) {
		errno = -nlerr->error;
		perror("netlink error");
	}
	if (pretty && !(nlhdr->nlmsg_flags & NLM_F_CAPPED) &&
	    nlhdr->nlmsg_len >= NLMSG_HDRLEN + nlerr->msg.nlmsg_len) {
		fprintf(stderr, "offending message%s:\n",
			err_offset ? " and attribute" : "");
		pretty_print_genlmsg(&nlerr->msg, ethnl_umsg_desc,
				     ethnl_umsg_n_desc, err_offset);
	}
	return nlerr->error;
}

/**
 * nlsock_process_reply() - process reply packet(s) from kernel
 * @nlsk:     netlink socket to read from
 * @reply_cb: callback to process each message
 * @data:     pointer passed as argument to @reply_cb callback
 *
 * Read packets from kernel and pass reply messages to @reply_cb callback
 * until an error is encountered or NLMSG_ERR message is received. In the
 * latter case, return value is the error code extracted from it.
 *
 * Return: 0 on success or negative error code
 */
int nlsock_process_reply(struct nl_socket *nlsk, mnl_cb_t reply_cb, void *data)
{
	struct nl_msg_buff *msgbuff = &nlsk->msgbuff;
	struct nlmsghdr *nlhdr;
	ssize_t len;
	char *buff;
	int ret;

	ret = msgbuff_realloc(msgbuff, NLSOCK_RECV_BUFFSIZE);
	if (ret < 0)
		return ret;
	buff = msgbuff->buff;

	do {
		len = mnl_socket_recvfrom(nlsk->sk, buff, msgbuff->size);
		if (len <= 0)
			return (len ? -EFAULT : 0);
		debug_msg(nlsk, buff, len, false);
		if (len < NLMSG_HDRLEN)
			return -EFAULT;

		nlhdr = (struct nlmsghdr *)buff;
		if (nlhdr->nlmsg_type == NLMSG_ERROR) {
			unsigned int suppress = nlsk->nlctx->suppress_nlerr;
			bool pretty;

			pretty = debug_on(nlsk->nlctx->ctx->debug,
					  DEBUG_NL_PRETTY_MSG);
			return nlsock_process_ack(nlhdr, len, suppress, pretty);
		}

		msgbuff->nlhdr = nlhdr;
		msgbuff->genlhdr = mnl_nlmsg_get_payload(nlhdr);
		msgbuff->payload =
			mnl_nlmsg_get_payload_offset(nlhdr, GENL_HDRLEN);
		ret = mnl_cb_run(buff, len, nlsk->seq, nlsk->port, reply_cb,
				 data);
	} while (ret > 0);

	return ret;
}

int nlsock_prep_get_request(struct nl_socket *nlsk, unsigned int nlcmd,
			    uint16_t hdr_attrtype, u32 flags)
{
	unsigned int nlm_flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nl_context *nlctx = nlsk->nlctx;
	const char *devname = nlctx->ctx->devname;
	int ret;

	if (devname && !strcmp(devname, WILDCARD_DEVNAME)) {
		devname = NULL;
		nlm_flags |= NLM_F_DUMP;
	}
	nlctx->is_dump = !devname;

	ret = msg_init(nlctx, &nlsk->msgbuff, nlcmd, nlm_flags);
	if (ret < 0)
		return ret;
	if (ethnla_fill_header(&nlsk->msgbuff, hdr_attrtype, devname, flags))
		return -EMSGSIZE;

	return 0;
}

#ifndef TEST_ETHTOOL
/**
 * nlsock_sendmsg() - send a netlink message to kernel
 * @nlsk:    netlink socket
 * @altbuff: alternative message buffer; if null, use buffer embedded in @nlsk
 *
 * Return: sent size or negative error code
 */
ssize_t nlsock_sendmsg(struct nl_socket *nlsk, struct nl_msg_buff *altbuff)
{
	struct nl_msg_buff *msgbuff = altbuff ?: &nlsk->msgbuff;
	struct nlmsghdr *nlhdr = msgbuff->nlhdr;

	nlhdr->nlmsg_seq = ++nlsk->seq;
	debug_msg(nlsk, msgbuff->buff, nlhdr->nlmsg_len, true);
	return mnl_socket_sendto(nlsk->sk, nlhdr, nlhdr->nlmsg_len);
}
#endif

/**
 * nlsock_send_get_request() - send request and process reply
 * @nlsk: netlink socket
 * @cb:   callback to process reply message(s)
 *
 * This simple helper only handles the most common case when the embedded
 * message buffer is sent and @cb takes netlink context (struct nl_context)
 * as last argument.
 */
int nlsock_send_get_request(struct nl_socket *nlsk, mnl_cb_t cb)
{
	int ret;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		goto err;
	ret = nlsock_process_reply(nlsk, cb, nlsk->nlctx);
	if (ret == 0)
		return 0;
err:
	return nlsk->nlctx->exit_code ?: 1;
}

/**
 * nlsock_init() - allocate and initialize netlink socket
 * @nlctx:  netlink context
 * @__nlsk: store pointer to the allocated socket here
 * @nl_fam: netlink family (e.g. NETLINK_GENERIC or NETLINK_ROUTE)
 *
 * Allocate and initialize netlink socket and its embedded message buffer.
 * Cleans up on error, caller is responsible for destroying the socket with
 * nlsock_done() on success.
 *
 * Return: 0 on success or negative error code
 */
int nlsock_init(struct nl_context *nlctx, struct nl_socket **__nlsk, int nl_fam)
{
	struct nl_socket *nlsk;
	int val;
	int ret;

	nlsk = calloc(1, sizeof(*nlsk));
	if (!nlsk)
		return -ENOMEM;
	nlsk->nlctx = nlctx;
	msgbuff_init(&nlsk->msgbuff);

	ret = -ECONNREFUSED;
	nlsk->sk = mnl_socket_open(nl_fam);
	if (!nlsk->sk)
		goto out_msgbuff;
	val = 1;
	mnl_socket_setsockopt(nlsk->sk, NETLINK_EXT_ACK, &val, sizeof(val));
	ret = mnl_socket_bind(nlsk->sk, 0, MNL_SOCKET_AUTOPID);
	if (ret < 0)
		goto out_close;
	nlsk->port = mnl_socket_get_portid(nlsk->sk);
	nlsk->nl_fam = nl_fam;

	*__nlsk = nlsk;
	return 0;

out_close:
	if (nlsk->sk)
		mnl_socket_close(nlsk->sk);
out_msgbuff:
	msgbuff_done(&nlsk->msgbuff);
	free(nlsk);
	return ret;
}

/**
 * nlsock_done() - destroy a netlink socket
 * @nlsk: netlink socket
 *
 * Close the socket and free the structure and related data.
 */
void nlsock_done(struct nl_socket *nlsk)
{
	if (!nlsk)
		return;
	if (nlsk->sk)
		mnl_socket_close(nlsk->sk);
	msgbuff_done(&nlsk->msgbuff);
	memset(nlsk, '\0', sizeof(*nlsk));
	free(nlsk);
}
