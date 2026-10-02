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

extern int nfq_set_verdict(struct nfq_q_handle *qh,
			     u_int32_t id,
			     u_int32_t verdict,
			     u_int32_t data_len,
			     unsigned char *buf);

extern int nfq_set_verdict_mark(struct nfq_q_handle *qh, 
				  u_int32_t id,
			   	  u_int32_t verdict, 
				  u_int32_t mark,
			   	  u_int32_t datalen,
				  unsigned char *buf);

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
extern int nfq_get_payload(struct nfq_data *nfad, char **data);



#endif	/* __LIBNFQNETLINK_H */
