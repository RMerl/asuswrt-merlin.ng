/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
/*
 * Copyright (C) 2012 achelos GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "hook.h"

/* this hook is currently only supported on Linux (systems like FreeBSD don't
 * actually provide an interface to change the sequence numbers of SAs) */
#ifdef __linux__

#include <linux/xfrm.h>
#include <unistd.h>
#include <errno.h>

#include <processing/jobs/callback_job.h>
#include <plugins/kernel_netlink/kernel_netlink_shared.h>

#define XFRM_RTA(nlh, x) ((struct rtattr*)(NLMSG_DATA(nlh) + NLMSG_ALIGN(sizeof(x))))

typedef struct private_reset_seq_t private_reset_seq_t;

/**
 * Private data of an reset_seq_t object.
 */
struct private_reset_seq_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;

	/**
	 * Delay for reset
	 */
	int delay;

	/**
	 * Sequence number to set for outgoing packages
	 */
	int oseq;
};

typedef struct reset_cb_data_t reset_cb_data_t;

/**
 * Data needed for the callback job
 */
struct reset_cb_data_t {

	/**
	 * The SA to modify
	 */
	struct xfrm_usersa_id usersa;

	/**
	 * Sequence number to set for outgoing packages
	 */
	int oseq;
};

/**
 * Callback job
 */
static job_requeue_t reset_cb(struct reset_cb_data_t *data)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct xfrm_aevent_id *id;
	struct rtattr *rthdr;
	struct xfrm_replay_state *rpstate;
	struct sockaddr_nl addr;
	int s, len;

	DBG1(DBG_CFG, "setting sequence number of SPI 0x%x to %d",
		 htonl(data->usersa.spi), data->oseq);

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_REPLACE;
	hdr->nlmsg_seq = 201;
	hdr->nlmsg_pid = getpid();
	hdr->nlmsg_type = XFRM_MSG_NEWAE;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_aevent_id));

	id = (struct xfrm_aevent_id*)NLMSG_DATA(hdr);
	id->sa_id = data->usersa;

	rthdr = XFRM_RTA(hdr, struct xfrm_aevent_id);
	rthdr->rta_type = XFRMA_REPLAY_VAL;
	rthdr->rta_len = RTA_LENGTH(sizeof(struct xfrm_replay_state));
	hdr->nlmsg_len += rthdr->rta_len;

	/* xfrm_replay_state is the structure the kernel uses for
	 * replay detection, and the oseq element contains the
	 * sequence number for outgoing packets. Currently, this
	 * function sets the other elements seq (records the number of
	 * incoming packets) and bitmask to zero, but they could be
	 * adjusted in the same way as oseq if required. */
	rpstate = (struct xfrm_replay_state*)RTA_DATA(rthdr);
	rpstate->oseq = data->oseq;

	s = socket(AF_NETLINK, SOCK_RAW, NETLINK_XFRM);
	if (s == -1)
	{
		DBG1(DBG_CFG, "opening XFRM socket failed: %s", strerror(errno));
		return JOB_REQUEUE_NONE;
	}
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	len = sendto(s, hdr, hdr->nlmsg_len, 0,
				 (struct sockaddr*)&addr, sizeof(addr));
	if (len != hdr->nlmsg_len)
	{
		DBG1(DBG_CFG, "sending XFRM aevent failed: %s", strerror(errno));
	}
	close(s);
	return JOB_REQUEUE_NONE;
}

/**
 * Schedule sequence number reset job
 */
static void schedule_reset_job(private_reset_seq_t *this, host_t *dst,
							   uint32_t spi)
{
	struct reset_cb_data_t *data;
	chunk_t chunk;

	INIT(data,
		.usersa = {
			.spi = spi,
			.family = dst->get_family(dst),
			.proto = IPPROTO_ESP,
		},
		.oseq = this->oseq,
	);

	chunk = dst->get_address(dst);
	memcpy(&data->usersa.daddr, chunk.ptr,
		   min(chunk.len, sizeof(xfrm_address_t)));

	lib->scheduler->schedule_job(lib->scheduler,
								 (job_t*)callback_job_create(
									(void*)reset_cb, data, (void*)free, NULL),
								 this->delay);
}

METHOD(listener_t, child_updown, bool,
	private_reset_seq_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	bool up)
{
	if (up)
	{
		schedule_reset_job(this, ike_sa->get_other_host(ike_sa),
						   child_sa->get_spi(child_sa, FALSE));
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_reset_seq_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *reset_seq_hook_create(char *name)
{
	private_reset_seq_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.child_updown = _child_updown,
			},
			.destroy = _destroy,
		},
		.delay = conftest->test->get_int(conftest->test,
										"hooks.%s.delay", 10, name),
		.oseq = conftest->test->get_int(conftest->test,
										"hooks.%s.oseq", 0, name),
	);

	return &this->hook;
}

#endif /* __linux__ */
