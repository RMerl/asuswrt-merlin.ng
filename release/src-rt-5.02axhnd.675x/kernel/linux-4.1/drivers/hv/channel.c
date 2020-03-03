/*
 * Copyright (c) 2009, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/hyperv.h>
#include <linux/uio.h>

#include "hyperv_vmbus.h"

#define NUM_PAGES_SPANNED(addr, len) \
((PAGE_ALIGN(addr + len) >> PAGE_SHIFT) - (addr >> PAGE_SHIFT))

/*
 * vmbus_setevent- Trigger an event notification on the specified
 * channel.
 */
static void vmbus_setevent(struct vmbus_channel *channel)
{
	struct hv_monitor_page *monitorpage;

	if (channel->offermsg.monitor_allocated) {
		/* Each u32 represents 32 channels */
		sync_set_bit(channel->offermsg.child_relid & 31,
			(unsigned long *) vmbus_connection.send_int_page +
			(channel->offermsg.child_relid >> 5));

		/* Get the child to parent monitor page */
		monitorpage = vmbus_connection.monitor_pages[1];

		sync_set_bit(channel->monitor_bit,
			(unsigned long *)&monitorpage->trigger_group
					[channel->monitor_grp].pending);

	} else {
		vmbus_set_event(channel);
	}
}

/*
 * vmbus_open - Open the specified channel.
 */
int vmbus_open(struct vmbus_channel *newchannel, u32 send_ringbuffer_size,
		     u32 recv_ringbuffer_size, void *userdata, u32 userdatalen,
		     void (*onchannelcallback)(void *context), void *context)
{
	struct vmbus_channel_open_channel *open_msg;
	struct vmbus_channel_msginfo *open_info = NULL;
	void *in, *out;
	unsigned long flags;
	int ret, err = 0;
	unsigned long t;

	spin_lock_irqsave(&newchannel->lock, flags);
	if (newchannel->state == CHANNEL_OPEN_STATE) {
		newchannel->state = CHANNEL_OPENING_STATE;
	} else {
		spin_unlock_irqrestore(&newchannel->lock, flags);
		return -EINVAL;
	}
	spin_unlock_irqrestore(&newchannel->lock, flags);

	newchannel->onchannel_callback = onchannelcallback;
	newchannel->channel_callback_context = context;

	/* Allocate the ring buffer */
	out = (void *)__get_free_pages(GFP_KERNEL|__GFP_ZERO,
		get_order(send_ringbuffer_size + recv_ringbuffer_size));

	if (!out) {
		err = -ENOMEM;
		goto error0;
	}

	in = (void *)((unsigned long)out + send_ringbuffer_size);

	newchannel->ringbuffer_pages = out;
	newchannel->ringbuffer_pagecount = (send_ringbuffer_size +
					   recv_ringbuffer_size) >> PAGE_SHIFT;

	ret = hv_ringbuffer_init(
		&newchannel->outbound, out, send_ringbuffer_size);

	if (ret != 0) {
		err = ret;
		goto error0;
	}

	ret = hv_ringbuffer_init(
		&newchannel->inbound, in, recv_ringbuffer_size);
	if (ret != 0) {
		err = ret;
		goto error0;
	}


	/* Establish the gpadl for the ring buffer */
	newchannel->ringbuffer_gpadlhandle = 0;

	ret = vmbus_establish_gpadl(newchannel,
					 newchannel->outbound.ring_buffer,
					 send_ringbuffer_size +
					 recv_ringbuffer_size,
					 &newchannel->ringbuffer_gpadlhandle);

	if (ret != 0) {
		err = ret;
		goto error0;
	}

	/* Create and init the channel open message */
	open_info = kmalloc(sizeof(*open_info) +
			   sizeof(struct vmbus_channel_open_channel),
			   GFP_KERNEL);
	if (!open_info) {
		err = -ENOMEM;
		goto error_gpadl;
	}

	init_completion(&open_info->waitevent);

	open_msg = (struct vmbus_channel_open_channel *)open_info->msg;
	open_msg->header.msgtype = CHANNELMSG_OPENCHANNEL;
	open_msg->openid = newchannel->offermsg.child_relid;
	open_msg->child_relid = newchannel->offermsg.child_relid;
	open_msg->ringbuffer_gpadlhandle = newchannel->ringbuffer_gpadlhandle;
	open_msg->downstream_ringbuffer_pageoffset = send_ringbuffer_size >>
						  PAGE_SHIFT;
	open_msg->target_vp = newchannel->target_vp;

	if (userdatalen > MAX_USER_DEFINED_BYTES) {
		err = -EINVAL;
		goto error_gpadl;
	}

	if (userdatalen)
		memcpy(open_msg->userdata, userdata, userdatalen);

	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_add_tail(&open_info->msglistentry,
		      &vmbus_connection.chn_msg_list);
	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);

	ret = vmbus_post_msg(open_msg,
			       sizeof(struct vmbus_channel_open_channel));

	if (ret != 0) {
		err = ret;
		goto error1;
	}

	t = wait_for_completion_timeout(&open_info->waitevent, 5*HZ);
	if (t == 0) {
		err = -ETIMEDOUT;
		goto error1;
	}


	if (open_info->response.open_result.status)
		err = open_info->response.open_result.status;

	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_del(&open_info->msglistentry);
	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);

	if (err == 0)
		newchannel->state = CHANNEL_OPENED_STATE;

	kfree(open_info);
	return err;

error1:
	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_del(&open_info->msglistentry);
	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);

error_gpadl:
	vmbus_teardown_gpadl(newchannel, newchannel->ringbuffer_gpadlhandle);

error0:
	free_pages((unsigned long)out,
		get_order(send_ringbuffer_size + recv_ringbuffer_size));
	kfree(open_info);
	newchannel->state = CHANNEL_OPEN_STATE;
	return err;
}
EXPORT_SYMBOL_GPL(vmbus_open);

/*
 * create_gpadl_header - Creates a gpadl for the specified buffer
 */
static int create_gpadl_header(void *kbuffer, u32 size,
					 struct vmbus_channel_msginfo **msginfo,
					 u32 *messagecount)
{
	int i;
	int pagecount;
	struct vmbus_channel_gpadl_header *gpadl_header;
	struct vmbus_channel_gpadl_body *gpadl_body;
	struct vmbus_channel_msginfo *msgheader;
	struct vmbus_channel_msginfo *msgbody = NULL;
	u32 msgsize;

	int pfnsum, pfncount, pfnleft, pfncurr, pfnsize;

	pagecount = size >> PAGE_SHIFT;

	/* do we need a gpadl body msg */
	pfnsize = MAX_SIZE_CHANNEL_MESSAGE -
		  sizeof(struct vmbus_channel_gpadl_header) -
		  sizeof(struct gpa_range);
	pfncount = pfnsize / sizeof(u64);

	if (pagecount > pfncount) {
		/* we need a gpadl body */
		/* fill in the header */
		msgsize = sizeof(struct vmbus_channel_msginfo) +
			  sizeof(struct vmbus_channel_gpadl_header) +
			  sizeof(struct gpa_range) + pfncount * sizeof(u64);
		msgheader =  kzalloc(msgsize, GFP_KERNEL);
		if (!msgheader)
			goto nomem;

		INIT_LIST_HEAD(&msgheader->submsglist);
		msgheader->msgsize = msgsize;

		gpadl_header = (struct vmbus_channel_gpadl_header *)
			msgheader->msg;
		gpadl_header->rangecount = 1;
		gpadl_header->range_buflen = sizeof(struct gpa_range) +
					 pagecount * sizeof(u64);
		gpadl_header->range[0].byte_offset = 0;
		gpadl_header->range[0].byte_count = size;
		for (i = 0; i < pfncount; i++)
			gpadl_header->range[0].pfn_array[i] = slow_virt_to_phys(
				kbuffer + PAGE_SIZE * i) >> PAGE_SHIFT;
		*msginfo = msgheader;
		*messagecount = 1;

		pfnsum = pfncount;
		pfnleft = pagecount - pfncount;

		/* how many pfns can we fit */
		pfnsize = MAX_SIZE_CHANNEL_MESSAGE -
			  sizeof(struct vmbus_channel_gpadl_body);
		pfncount = pfnsize / sizeof(u64);

		/* fill in the body */
		while (pfnleft) {
			if (pfnleft > pfncount)
				pfncurr = pfncount;
			else
				pfncurr = pfnleft;

			msgsize = sizeof(struct vmbus_channel_msginfo) +
				  sizeof(struct vmbus_channel_gpadl_body) +
				  pfncurr * sizeof(u64);
			msgbody = kzalloc(msgsize, GFP_KERNEL);

			if (!msgbody) {
				struct vmbus_channel_msginfo *pos = NULL;
				struct vmbus_channel_msginfo *tmp = NULL;
				/*
				 * Free up all the allocated messages.
				 */
				list_for_each_entry_safe(pos, tmp,
					&msgheader->submsglist,
					msglistentry) {

					list_del(&pos->msglistentry);
					kfree(pos);
				}

				goto nomem;
			}

			msgbody->msgsize = msgsize;
			(*messagecount)++;
			gpadl_body =
				(struct vmbus_channel_gpadl_body *)msgbody->msg;

			/*
			 * Gpadl is u32 and we are using a pointer which could
			 * be 64-bit
			 * This is governed by the guest/host protocol and
			 * so the hypervisor gurantees that this is ok.
			 */
			for (i = 0; i < pfncurr; i++)
				gpadl_body->pfn[i] = slow_virt_to_phys(
					kbuffer + PAGE_SIZE * (pfnsum + i)) >>
					PAGE_SHIFT;

			/* add to msg header */
			list_add_tail(&msgbody->msglistentry,
				      &msgheader->submsglist);
			pfnsum += pfncurr;
			pfnleft -= pfncurr;
		}
	} else {
		/* everything fits in a header */
		msgsize = sizeof(struct vmbus_channel_msginfo) +
			  sizeof(struct vmbus_channel_gpadl_header) +
			  sizeof(struct gpa_range) + pagecount * sizeof(u64);
		msgheader = kzalloc(msgsize, GFP_KERNEL);
		if (msgheader == NULL)
			goto nomem;
		msgheader->msgsize = msgsize;

		gpadl_header = (struct vmbus_channel_gpadl_header *)
			msgheader->msg;
		gpadl_header->rangecount = 1;
		gpadl_header->range_buflen = sizeof(struct gpa_range) +
					 pagecount * sizeof(u64);
		gpadl_header->range[0].byte_offset = 0;
		gpadl_header->range[0].byte_count = size;
		for (i = 0; i < pagecount; i++)
			gpadl_header->range[0].pfn_array[i] = slow_virt_to_phys(
				kbuffer + PAGE_SIZE * i) >> PAGE_SHIFT;

		*msginfo = msgheader;
		*messagecount = 1;
	}

	return 0;
nomem:
	kfree(msgheader);
	kfree(msgbody);
	return -ENOMEM;
}

/*
 * vmbus_establish_gpadl - Estabish a GPADL for the specified buffer
 *
 * @channel: a channel
 * @kbuffer: from kmalloc or vmalloc
 * @size: page-size multiple
 * @gpadl_handle: some funky thing
 */
int vmbus_establish_gpadl(struct vmbus_channel *channel, void *kbuffer,
			       u32 size, u32 *gpadl_handle)
{
	struct vmbus_channel_gpadl_header *gpadlmsg;
	struct vmbus_channel_gpadl_body *gpadl_body;
	struct vmbus_channel_msginfo *msginfo = NULL;
	struct vmbus_channel_msginfo *submsginfo, *tmp;
	u32 msgcount;
	struct list_head *curr;
	u32 next_gpadl_handle;
	unsigned long flags;
	int ret = 0;

	next_gpadl_handle =
		(atomic_inc_return(&vmbus_connection.next_gpadl_handle) - 1);

	ret = create_gpadl_header(kbuffer, size, &msginfo, &msgcount);
	if (ret)
		return ret;

	init_completion(&msginfo->waitevent);

	gpadlmsg = (struct vmbus_channel_gpadl_header *)msginfo->msg;
	gpadlmsg->header.msgtype = CHANNELMSG_GPADL_HEADER;
	gpadlmsg->child_relid = channel->offermsg.child_relid;
	gpadlmsg->gpadl = next_gpadl_handle;


	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_add_tail(&msginfo->msglistentry,
		      &vmbus_connection.chn_msg_list);

	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);

	ret = vmbus_post_msg(gpadlmsg, msginfo->msgsize -
			       sizeof(*msginfo));
	if (ret != 0)
		goto cleanup;

	if (msgcount > 1) {
		list_for_each(curr, &msginfo->submsglist) {

			submsginfo = (struct vmbus_channel_msginfo *)curr;
			gpadl_body =
			     (struct vmbus_channel_gpadl_body *)submsginfo->msg;

			gpadl_body->header.msgtype =
				CHANNELMSG_GPADL_BODY;
			gpadl_body->gpadl = next_gpadl_handle;

			ret = vmbus_post_msg(gpadl_body,
					       submsginfo->msgsize -
					       sizeof(*submsginfo));
			if (ret != 0)
				goto cleanup;

		}
	}
	wait_for_completion(&msginfo->waitevent);

	/* At this point, we received the gpadl created msg */
	*gpadl_handle = gpadlmsg->gpadl;

cleanup:
	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_del(&msginfo->msglistentry);
	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);

	if (msgcount > 1) {
		list_for_each_entry_safe(submsginfo, tmp, &msginfo->submsglist,
			 msglistentry) {
			kfree(submsginfo);
		}
	}

	kfree(msginfo);
	return ret;
}
EXPORT_SYMBOL_GPL(vmbus_establish_gpadl);

/*
 * vmbus_teardown_gpadl -Teardown the specified GPADL handle
 */
int vmbus_teardown_gpadl(struct vmbus_channel *channel, u32 gpadl_handle)
{
	struct vmbus_channel_gpadl_teardown *msg;
	struct vmbus_channel_msginfo *info;
	unsigned long flags;
	int ret;

	info = kmalloc(sizeof(*info) +
		       sizeof(struct vmbus_channel_gpadl_teardown), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	init_completion(&info->waitevent);

	msg = (struct vmbus_channel_gpadl_teardown *)info->msg;

	msg->header.msgtype = CHANNELMSG_GPADL_TEARDOWN;
	msg->child_relid = channel->offermsg.child_relid;
	msg->gpadl = gpadl_handle;

	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_add_tail(&info->msglistentry,
		      &vmbus_connection.chn_msg_list);
	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);
	ret = vmbus_post_msg(msg,
			       sizeof(struct vmbus_channel_gpadl_teardown));

	if (ret)
		goto post_msg_err;

	wait_for_completion(&info->waitevent);

post_msg_err:
	spin_lock_irqsave(&vmbus_connection.channelmsg_lock, flags);
	list_del(&info->msglistentry);
	spin_unlock_irqrestore(&vmbus_connection.channelmsg_lock, flags);

	kfree(info);
	return ret;
}
EXPORT_SYMBOL_GPL(vmbus_teardown_gpadl);

static void reset_channel_cb(void *arg)
{
	struct vmbus_channel *channel = arg;

	channel->onchannel_callback = NULL;
}

static int vmbus_close_internal(struct vmbus_channel *channel)
{
	struct vmbus_channel_close_channel *msg;
	int ret;

	channel->state = CHANNEL_OPEN_STATE;
	channel->sc_creation_callback = NULL;
	/* Stop callback and cancel the timer asap */
	if (channel->target_cpu != get_cpu()) {
		put_cpu();
		smp_call_function_single(channel->target_cpu, reset_channel_cb,
					 channel, true);
	} else {
		reset_channel_cb(channel);
		put_cpu();
	}

	/* Send a closing message */

	msg = &channel->close_msg.msg;

	msg->header.msgtype = CHANNELMSG_CLOSECHANNEL;
	msg->child_relid = channel->offermsg.child_relid;

	ret = vmbus_post_msg(msg, sizeof(struct vmbus_channel_close_channel));

	if (ret) {
		pr_err("Close failed: close post msg return is %d\n", ret);
		/*
		 * If we failed to post the close msg,
		 * it is perhaps better to leak memory.
		 */
		return ret;
	}

	/* Tear down the gpadl for the channel's ring buffer */
	if (channel->ringbuffer_gpadlhandle) {
		ret = vmbus_teardown_gpadl(channel,
					   channel->ringbuffer_gpadlhandle);
		if (ret) {
			pr_err("Close failed: teardown gpadl return %d\n", ret);
			/*
			 * If we failed to teardown gpadl,
			 * it is perhaps better to leak memory.
			 */
			return ret;
		}
	}

	/* Cleanup the ring buffers for this channel */
	hv_ringbuffer_cleanup(&channel->outbound);
	hv_ringbuffer_cleanup(&channel->inbound);

	free_pages((unsigned long)channel->ringbuffer_pages,
		get_order(channel->ringbuffer_pagecount * PAGE_SIZE));

	/*
	 * If the channel has been rescinded; process device removal.
	 */
	if (channel->rescind)
		hv_process_channel_removal(channel,
					   channel->offermsg.child_relid);
	return ret;
}

/*
 * vmbus_close - Close the specified channel
 */
void vmbus_close(struct vmbus_channel *channel)
{
	struct list_head *cur, *tmp;
	struct vmbus_channel *cur_channel;

	if (channel->primary_channel != NULL) {
		/*
		 * We will only close sub-channels when
		 * the primary is closed.
		 */
		return;
	}
	/*
	 * Close all the sub-channels first and then close the
	 * primary channel.
	 */
	list_for_each_safe(cur, tmp, &channel->sc_list) {
		cur_channel = list_entry(cur, struct vmbus_channel, sc_list);
		if (cur_channel->state != CHANNEL_OPENED_STATE)
			continue;
		vmbus_close_internal(cur_channel);
	}
	/*
	 * Now close the primary.
	 */
	vmbus_close_internal(channel);
}
EXPORT_SYMBOL_GPL(vmbus_close);

int vmbus_sendpacket_ctl(struct vmbus_channel *channel, void *buffer,
			   u32 bufferlen, u64 requestid,
			   enum vmbus_packet_type type, u32 flags, bool kick_q)
{
	struct vmpacket_descriptor desc;
	u32 packetlen = sizeof(struct vmpacket_descriptor) + bufferlen;
	u32 packetlen_aligned = ALIGN(packetlen, sizeof(u64));
	struct kvec bufferlist[3];
	u64 aligned_data = 0;
	int ret;
	bool signal = false;


	/* Setup the descriptor */
	desc.type = type; /* VmbusPacketTypeDataInBand; */
	desc.flags = flags; /* VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED; */
	/* in 8-bytes granularity */
	desc.offset8 = sizeof(struct vmpacket_descriptor) >> 3;
	desc.len8 = (u16)(packetlen_aligned >> 3);
	desc.trans_id = requestid;

	bufferlist[0].iov_base = &desc;
	bufferlist[0].iov_len = sizeof(struct vmpacket_descriptor);
	bufferlist[1].iov_base = buffer;
	bufferlist[1].iov_len = bufferlen;
	bufferlist[2].iov_base = &aligned_data;
	bufferlist[2].iov_len = (packetlen_aligned - packetlen);

	ret = hv_ringbuffer_write(&channel->outbound, bufferlist, 3, &signal);

	/*
	 * Signalling the host is conditional on many factors:
	 * 1. The ring state changed from being empty to non-empty.
	 *    This is tracked by the variable "signal".
	 * 2. The variable kick_q tracks if more data will be placed
	 *    on the ring. We will not signal if more data is
	 *    to be placed.
	 *
	 * If we cannot write to the ring-buffer; signal the host
	 * even if we may not have written anything. This is a rare
	 * enough condition that it should not matter.
	 */
	if (((ret == 0) && kick_q && signal) || (ret))
		vmbus_setevent(channel);

	return ret;
}
EXPORT_SYMBOL(vmbus_sendpacket_ctl);

/**
 * vmbus_sendpacket() - Send the specified buffer on the given channel
 * @channel: Pointer to vmbus_channel structure.
 * @buffer: Pointer to the buffer you want to receive the data into.
 * @bufferlen: Maximum size of what the the buffer will hold
 * @requestid: Identifier of the request
 * @type: Type of packet that is being send e.g. negotiate, time
 * packet etc.
 *
 * Sends data in @buffer directly to hyper-v via the vmbus
 * This will send the data unparsed to hyper-v.
 *
 * Mainly used by Hyper-V drivers.
 */
int vmbus_sendpacket(struct vmbus_channel *channel, void *buffer,
			   u32 bufferlen, u64 requestid,
			   enum vmbus_packet_type type, u32 flags)
{
	return vmbus_sendpacket_ctl(channel, buffer, bufferlen, requestid,
				    type, flags, true);
}
EXPORT_SYMBOL(vmbus_sendpacket);

/*
 * vmbus_sendpacket_pagebuffer_ctl - Send a range of single-page buffer
 * packets using a GPADL Direct packet type. This interface allows you
 * to control notifying the host. This will be useful for sending
 * batched data. Also the sender can control the send flags
 * explicitly.
 */
int vmbus_sendpacket_pagebuffer_ctl(struct vmbus_channel *channel,
				     struct hv_page_buffer pagebuffers[],
				     u32 pagecount, void *buffer, u32 bufferlen,
				     u64 requestid,
				     u32 flags,
				     bool kick_q)
{
	int ret;
	int i;
	struct vmbus_channel_packet_page_buffer desc;
	u32 descsize;
	u32 packetlen;
	u32 packetlen_aligned;
	struct kvec bufferlist[3];
	u64 aligned_data = 0;
	bool signal = false;

	if (pagecount > MAX_PAGE_BUFFER_COUNT)
		return -EINVAL;


	/*
	 * Adjust the size down since vmbus_channel_packet_page_buffer is the
	 * largest size we support
	 */
	descsize = sizeof(struct vmbus_channel_packet_page_buffer) -
			  ((MAX_PAGE_BUFFER_COUNT - pagecount) *
			  sizeof(struct hv_page_buffer));
	packetlen = descsize + bufferlen;
	packetlen_aligned = ALIGN(packetlen, sizeof(u64));

	/* Setup the descriptor */
	desc.type = VM_PKT_DATA_USING_GPA_DIRECT;
	desc.flags = flags;
	desc.dataoffset8 = descsize >> 3; /* in 8-bytes grandularity */
	desc.length8 = (u16)(packetlen_aligned >> 3);
	desc.transactionid = requestid;
	desc.rangecount = pagecount;

	for (i = 0; i < pagecount; i++) {
		desc.range[i].len = pagebuffers[i].len;
		desc.range[i].offset = pagebuffers[i].offset;
		desc.range[i].pfn	 = pagebuffers[i].pfn;
	}

	bufferlist[0].iov_base = &desc;
	bufferlist[0].iov_len = descsize;
	bufferlist[1].iov_base = buffer;
	bufferlist[1].iov_len = bufferlen;
	bufferlist[2].iov_base = &aligned_data;
	bufferlist[2].iov_len = (packetlen_aligned - packetlen);

	ret = hv_ringbuffer_write(&channel->outbound, bufferlist, 3, &signal);

	/*
	 * Signalling the host is conditional on many factors:
	 * 1. The ring state changed from being empty to non-empty.
	 *    This is tracked by the variable "signal".
	 * 2. The variable kick_q tracks if more data will be placed
	 *    on the ring. We will not signal if more data is
	 *    to be placed.
	 *
	 * If we cannot write to the ring-buffer; signal the host
	 * even if we may not have written anything. This is a rare
	 * enough condition that it should not matter.
	 */
	if (((ret == 0) && kick_q && signal) || (ret))
		vmbus_setevent(channel);

	return ret;
}
EXPORT_SYMBOL_GPL(vmbus_sendpacket_pagebuffer_ctl);

/*
 * vmbus_sendpacket_pagebuffer - Send a range of single-page buffer
 * packets using a GPADL Direct packet type.
 */
int vmbus_sendpacket_pagebuffer(struct vmbus_channel *channel,
				     struct hv_page_buffer pagebuffers[],
				     u32 pagecount, void *buffer, u32 bufferlen,
				     u64 requestid)
{
	u32 flags = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
	return vmbus_sendpacket_pagebuffer_ctl(channel, pagebuffers, pagecount,
					       buffer, bufferlen, requestid,
					       flags, true);

}
EXPORT_SYMBOL_GPL(vmbus_sendpacket_pagebuffer);

/*
 * vmbus_sendpacket_multipagebuffer - Send a multi-page buffer packet
 * using a GPADL Direct packet type.
 * The buffer includes the vmbus descriptor.
 */
int vmbus_sendpacket_mpb_desc(struct vmbus_channel *channel,
			      struct vmbus_packet_mpb_array *desc,
			      u32 desc_size,
			      void *buffer, u32 bufferlen, u64 requestid)
{
	int ret;
	u32 packetlen;
	u32 packetlen_aligned;
	struct kvec bufferlist[3];
	u64 aligned_data = 0;
	bool signal = false;

	packetlen = desc_size + bufferlen;
	packetlen_aligned = ALIGN(packetlen, sizeof(u64));

	/* Setup the descriptor */
	desc->type = VM_PKT_DATA_USING_GPA_DIRECT;
	desc->flags = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
	desc->dataoffset8 = desc_size >> 3; /* in 8-bytes grandularity */
	desc->length8 = (u16)(packetlen_aligned >> 3);
	desc->transactionid = requestid;
	desc->rangecount = 1;

	bufferlist[0].iov_base = desc;
	bufferlist[0].iov_len = desc_size;
	bufferlist[1].iov_base = buffer;
	bufferlist[1].iov_len = bufferlen;
	bufferlist[2].iov_base = &aligned_data;
	bufferlist[2].iov_len = (packetlen_aligned - packetlen);

	ret = hv_ringbuffer_write(&channel->outbound, bufferlist, 3, &signal);

	if (ret == 0 && signal)
		vmbus_setevent(channel);

	return ret;
}
EXPORT_SYMBOL_GPL(vmbus_sendpacket_mpb_desc);

/*
 * vmbus_sendpacket_multipagebuffer - Send a multi-page buffer packet
 * using a GPADL Direct packet type.
 */
int vmbus_sendpacket_multipagebuffer(struct vmbus_channel *channel,
				struct hv_multipage_buffer *multi_pagebuffer,
				void *buffer, u32 bufferlen, u64 requestid)
{
	int ret;
	struct vmbus_channel_packet_multipage_buffer desc;
	u32 descsize;
	u32 packetlen;
	u32 packetlen_aligned;
	struct kvec bufferlist[3];
	u64 aligned_data = 0;
	bool signal = false;
	u32 pfncount = NUM_PAGES_SPANNED(multi_pagebuffer->offset,
					 multi_pagebuffer->len);

	if (pfncount > MAX_MULTIPAGE_BUFFER_COUNT)
		return -EINVAL;

	/*
	 * Adjust the size down since vmbus_channel_packet_multipage_buffer is
	 * the largest size we support
	 */
	descsize = sizeof(struct vmbus_channel_packet_multipage_buffer) -
			  ((MAX_MULTIPAGE_BUFFER_COUNT - pfncount) *
			  sizeof(u64));
	packetlen = descsize + bufferlen;
	packetlen_aligned = ALIGN(packetlen, sizeof(u64));


	/* Setup the descriptor */
	desc.type = VM_PKT_DATA_USING_GPA_DIRECT;
	desc.flags = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
	desc.dataoffset8 = descsize >> 3; /* in 8-bytes grandularity */
	desc.length8 = (u16)(packetlen_aligned >> 3);
	desc.transactionid = requestid;
	desc.rangecount = 1;

	desc.range.len = multi_pagebuffer->len;
	desc.range.offset = multi_pagebuffer->offset;

	memcpy(desc.range.pfn_array, multi_pagebuffer->pfn_array,
	       pfncount * sizeof(u64));

	bufferlist[0].iov_base = &desc;
	bufferlist[0].iov_len = descsize;
	bufferlist[1].iov_base = buffer;
	bufferlist[1].iov_len = bufferlen;
	bufferlist[2].iov_base = &aligned_data;
	bufferlist[2].iov_len = (packetlen_aligned - packetlen);

	ret = hv_ringbuffer_write(&channel->outbound, bufferlist, 3, &signal);

	if (ret == 0 && signal)
		vmbus_setevent(channel);

	return ret;
}
EXPORT_SYMBOL_GPL(vmbus_sendpacket_multipagebuffer);

/**
 * vmbus_recvpacket() - Retrieve the user packet on the specified channel
 * @channel: Pointer to vmbus_channel structure.
 * @buffer: Pointer to the buffer you want to receive the data into.
 * @bufferlen: Maximum size of what the the buffer will hold
 * @buffer_actual_len: The actual size of the data after it was received
 * @requestid: Identifier of the request
 *
 * Receives directly from the hyper-v vmbus and puts the data it received
 * into Buffer. This will receive the data unparsed from hyper-v.
 *
 * Mainly used by Hyper-V drivers.
 */
int vmbus_recvpacket(struct vmbus_channel *channel, void *buffer,
			u32 bufferlen, u32 *buffer_actual_len, u64 *requestid)
{
	struct vmpacket_descriptor desc;
	u32 packetlen;
	u32 userlen;
	int ret;
	bool signal = false;

	*buffer_actual_len = 0;
	*requestid = 0;


	ret = hv_ringbuffer_peek(&channel->inbound, &desc,
			     sizeof(struct vmpacket_descriptor));
	if (ret != 0)
		return 0;

	packetlen = desc.len8 << 3;
	userlen = packetlen - (desc.offset8 << 3);

	*buffer_actual_len = userlen;

	if (userlen > bufferlen) {

		pr_err("Buffer too small - got %d needs %d\n",
			   bufferlen, userlen);
		return -ETOOSMALL;
	}

	*requestid = desc.trans_id;

	/* Copy over the packet to the user buffer */
	ret = hv_ringbuffer_read(&channel->inbound, buffer, userlen,
			     (desc.offset8 << 3), &signal);

	if (signal)
		vmbus_setevent(channel);

	return 0;
}
EXPORT_SYMBOL(vmbus_recvpacket);

/*
 * vmbus_recvpacket_raw - Retrieve the raw packet on the specified channel
 */
int vmbus_recvpacket_raw(struct vmbus_channel *channel, void *buffer,
			      u32 bufferlen, u32 *buffer_actual_len,
			      u64 *requestid)
{
	struct vmpacket_descriptor desc;
	u32 packetlen;
	int ret;
	bool signal = false;

	*buffer_actual_len = 0;
	*requestid = 0;


	ret = hv_ringbuffer_peek(&channel->inbound, &desc,
			     sizeof(struct vmpacket_descriptor));
	if (ret != 0)
		return 0;


	packetlen = desc.len8 << 3;

	*buffer_actual_len = packetlen;

	if (packetlen > bufferlen)
		return -ENOBUFS;

	*requestid = desc.trans_id;

	/* Copy over the entire packet to the user buffer */
	ret = hv_ringbuffer_read(&channel->inbound, buffer, packetlen, 0,
				 &signal);

	if (signal)
		vmbus_setevent(channel);

	return ret;
}
EXPORT_SYMBOL_GPL(vmbus_recvpacket_raw);
