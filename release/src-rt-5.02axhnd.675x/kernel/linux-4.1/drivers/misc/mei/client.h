/*
 *
 * Intel Management Engine Interface (Intel MEI) Linux driver
 * Copyright (c) 2003-2012, Intel Corporation.
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
 */

#ifndef _MEI_CLIENT_H_
#define _MEI_CLIENT_H_

#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/poll.h>
#include <linux/mei.h>

#include "mei_dev.h"

/*
 * reference counting base function
 */
void mei_me_cl_init(struct mei_me_client *me_cl);
void mei_me_cl_put(struct mei_me_client *me_cl);
struct mei_me_client *mei_me_cl_get(struct mei_me_client *me_cl);

void mei_me_cl_add(struct mei_device *dev, struct mei_me_client *me_cl);
void mei_me_cl_del(struct mei_device *dev, struct mei_me_client *me_cl);

struct mei_me_client *mei_me_cl_by_uuid(struct mei_device *dev,
					const uuid_le *uuid);
struct mei_me_client *mei_me_cl_by_id(struct mei_device *dev, u8 client_id);
struct mei_me_client *mei_me_cl_by_uuid_id(struct mei_device *dev,
					   const uuid_le *uuid, u8 client_id);
void mei_me_cl_rm_by_uuid(struct mei_device *dev, const uuid_le *uuid);
void mei_me_cl_rm_by_uuid_id(struct mei_device *dev,
			     const uuid_le *uuid, u8 id);
void mei_me_cl_rm_all(struct mei_device *dev);

/*
 * MEI IO Functions
 */
struct mei_cl_cb *mei_io_cb_init(struct mei_cl *cl, enum mei_cb_file_ops type,
				 struct file *fp);
void mei_io_cb_free(struct mei_cl_cb *priv_cb);
int mei_io_cb_alloc_buf(struct mei_cl_cb *cb, size_t length);


/**
 * mei_io_list_init - Sets up a queue list.
 *
 * @list: An instance cl callback structure
 */
static inline void mei_io_list_init(struct mei_cl_cb *list)
{
	INIT_LIST_HEAD(&list->list);
}
void mei_io_list_flush(struct mei_cl_cb *list, struct mei_cl *cl);

/*
 * MEI Host Client Functions
 */

struct mei_cl *mei_cl_allocate(struct mei_device *dev);
void mei_cl_init(struct mei_cl *cl, struct mei_device *dev);


int mei_cl_link(struct mei_cl *cl, int id);
int mei_cl_unlink(struct mei_cl *cl);

struct mei_cl *mei_cl_alloc_linked(struct mei_device *dev, int id);

struct mei_cl_cb *mei_cl_read_cb(const struct mei_cl *cl,
				 const struct file *fp);
void mei_cl_read_cb_flush(const struct mei_cl *cl, const struct file *fp);
struct mei_cl_cb *mei_cl_alloc_cb(struct mei_cl *cl, size_t length,
				  enum mei_cb_file_ops type, struct file *fp);
int mei_cl_flush_queues(struct mei_cl *cl, const struct file *fp);

int mei_cl_flow_ctrl_creds(struct mei_cl *cl);

int mei_cl_flow_ctrl_reduce(struct mei_cl *cl);
/*
 *  MEI input output function prototype
 */

/**
 * mei_cl_is_connected - host client is connected
 *
 * @cl: host clinet
 *
 * Return: true if the host clinet is connected
 */
static inline bool mei_cl_is_connected(struct mei_cl *cl)
{
	return  cl->state == MEI_FILE_CONNECTED;
}

bool mei_cl_is_other_connecting(struct mei_cl *cl);
int mei_cl_disconnect(struct mei_cl *cl);
int mei_cl_connect(struct mei_cl *cl, struct file *file);
int mei_cl_read_start(struct mei_cl *cl, size_t length, struct file *fp);
int mei_cl_irq_read_msg(struct mei_cl *cl, struct mei_msg_hdr *hdr,
			struct mei_cl_cb *cmpl_list);
int mei_cl_write(struct mei_cl *cl, struct mei_cl_cb *cb, bool blocking);
int mei_cl_irq_write(struct mei_cl *cl, struct mei_cl_cb *cb,
		     struct mei_cl_cb *cmpl_list);

void mei_cl_complete(struct mei_cl *cl, struct mei_cl_cb *cb);

void mei_host_client_init(struct work_struct *work);



void mei_cl_all_disconnect(struct mei_device *dev);
void mei_cl_all_wakeup(struct mei_device *dev);
void mei_cl_all_write_clear(struct mei_device *dev);

#define MEI_CL_FMT "cl:host=%02d me=%02d "
#define MEI_CL_PRM(cl) (cl)->host_client_id, (cl)->me_client_id

#define cl_dbg(dev, cl, format, arg...) \
	dev_dbg((dev)->dev, MEI_CL_FMT format, MEI_CL_PRM(cl), ##arg)

#define cl_err(dev, cl, format, arg...) \
	dev_err((dev)->dev, MEI_CL_FMT format, MEI_CL_PRM(cl), ##arg)

#endif /* _MEI_CLIENT_H_ */
