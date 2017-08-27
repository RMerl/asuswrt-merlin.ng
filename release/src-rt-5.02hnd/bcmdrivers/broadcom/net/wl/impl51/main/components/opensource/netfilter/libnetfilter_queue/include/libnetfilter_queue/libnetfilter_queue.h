/* libnfqnetlink.h: Header file for the Netfilter Queue library.
 *
 * (C) 2005 by Harald Welte <laforge@gnumonks.org>
 *
 *
 * Changelog : 
 * 	(2005/08/11)  added  parsing function (Eric Leblond <regit@inl.fr>)
 * 
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef __LIBCTNETLINK_H
#define __LIBCTNETLINK_H

#include <libnfnetlink/libnfnetlink.h>
// #include <libnfnetlink/liunx_nfnetlink.h>

#include <libnetfilter_queue/linux_nfnetlink_queue.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nfq_handle;
struct nfq_q_handle;
struct nfq_data;

extern int nfq_errno;

extern struct nfnl_handle *nfq_nfnlh(struct nfq_handle *h);
extern int nfq_fd(struct nfq_handle *h);

typedef int  nfq_callback(struct nfq_q_handle *gh, struct nfgenmsg *nfmsg,
		       struct nfq_data *nfad, void *data);


extern struct nfq_handle *nfq_open(void);
extern struct nfq_handle *nfq_open_nfnl(struct nfnl_handle *nfnlh);
extern int nfq_close(struct nfq_handle *h);

extern int nfq_bind_pf(struct nfq_handle *h, u_int16_t pf);
extern int nfq_unbind_pf(struct nfq_handle *h, u_int16_t pf);

extern struct nfq_q_handle *nfq_create_queue(struct nfq_handle *h,
			      			 u_int16_t num,
						 nfq_callback *cb,
						 void *data);
extern int nfq_destroy_queue(struct nfq_q_handle *qh);

extern int nfq_handle_packet(struct nfq_handle *h, char *buf, int len);

extern int nfq_set_mode(struct nfq_q_handle *qh,
			  u_int8_t mode, unsigned int len);

int nfq_set_queue_maxlen(struct nfq_q_handle *qh,
			u_int32_t queuelen);

extern int nfq_set_queue_flags(struct nfq_q_handle *qh,
			       uint32_t mask, uint32_t flags);

extern int nfq_set_verdict(struct nfq_q_handle *qh,
			     u_int32_t id,
			     u_int32_t verdict,
			     u_int32_t data_len,
			     const unsigned char *buf);

extern int nfq_set_verdict2(struct nfq_q_handle *qh,
			    u_int32_t id,
			    u_int32_t verdict, 
			    u_int32_t mark,
			    u_int32_t datalen,
			    const unsigned char *buf);

extern int nfq_set_verdict_batch(struct nfq_q_handle *qh,
			    u_int32_t id,
			    u_int32_t verdict);

extern int nfq_set_verdict_batch2(struct nfq_q_handle *qh,
			    u_int32_t id,
			    u_int32_t verdict,
			    u_int32_t mark);

extern __attribute__((deprecated))
int nfq_set_verdict_mark(struct nfq_q_handle *qh, 
			 u_int32_t id,
			 u_int32_t verdict, 
			 u_int32_t mark,
			 u_int32_t datalen,
			 const unsigned char *buf);

/* message parsing function */

extern struct nfqnl_msg_packet_hdr *
				nfq_get_msg_packet_hdr(struct nfq_data *nfad);

extern u_int32_t nfq_get_nfmark(struct nfq_data *nfad);

extern int nfq_get_timestamp(struct nfq_data *nfad, struct timeval *tv);

/* return 0 if not set */
extern u_int32_t nfq_get_indev(struct nfq_data *nfad);
extern u_int32_t nfq_get_physindev(struct nfq_data *nfad);
extern u_int32_t nfq_get_outdev(struct nfq_data *nfad);
extern u_int32_t nfq_get_physoutdev(struct nfq_data *nfad);

extern int nfq_get_indev_name(struct nlif_handle *nlif_handle,
			      struct nfq_data *nfad, char *name);
extern int nfq_get_physindev_name(struct nlif_handle *nlif_handle,
			          struct nfq_data *nfad, char *name);
extern int nfq_get_outdev_name(struct nlif_handle *nlif_handle,
			       struct nfq_data *nfad, char *name);
extern int nfq_get_physoutdev_name(struct nlif_handle *nlif_handle,
				   struct nfq_data *nfad, char *name);

extern struct nfqnl_msg_packet_hw *nfq_get_packet_hw(struct nfq_data *nfad);

/* return -1 if problem, length otherwise */
extern int nfq_get_payload(struct nfq_data *nfad, unsigned char **data);

enum {
	NFQ_XML_HW	= (1 << 0),
	NFQ_XML_MARK	= (1 << 1),
	NFQ_XML_DEV	= (1 << 2),
	NFQ_XML_PHYSDEV	= (1 << 3),
	NFQ_XML_PAYLOAD	= (1 << 4),
	NFQ_XML_TIME	= (1 << 5),
	NFQ_XML_ALL	= ~0U,
};

extern int nfq_snprintf_xml(char *buf, size_t len, struct nfq_data *tb, int flags);

/*
 * New API based on libmnl
 */

void nfq_nlmsg_cfg_put_cmd(struct nlmsghdr *nlh, uint16_t pf, uint8_t cmd);
void nfq_nlmsg_cfg_put_params(struct nlmsghdr *nlh, uint8_t mode, int range);
void nfq_nlmsg_cfg_put_qmaxlen(struct nlmsghdr *nlh, uint32_t qmaxlen);

void nfq_nlmsg_verdict_put(struct nlmsghdr *nlh, int id, int verdict);
void nfq_nlmsg_verdict_put_mark(struct nlmsghdr *nlh, uint32_t mark);
void nfq_nlmsg_verdict_put_pkt(struct nlmsghdr *nlh, const void *pkt, uint32_t pktlen);

int nfq_nlmsg_parse(const struct nlmsghdr *nlh, struct nlattr **pkt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* __LIBNFQNETLINK_H */
