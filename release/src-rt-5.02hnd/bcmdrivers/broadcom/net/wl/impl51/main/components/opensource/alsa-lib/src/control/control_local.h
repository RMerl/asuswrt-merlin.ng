/*
 *  Control Interface - local header file
 *  Copyright (c) 2000 by Jaroslav Kysela <perex@perex.cz>
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

#include "local.h"

typedef struct _snd_ctl_ops {
	int (*close)(snd_ctl_t *handle);
	int (*nonblock)(snd_ctl_t *handle, int nonblock);
	int (*async)(snd_ctl_t *handle, int sig, pid_t pid);
	int (*subscribe_events)(snd_ctl_t *handle, int subscribe);
	int (*card_info)(snd_ctl_t *handle, snd_ctl_card_info_t *info);
	int (*element_list)(snd_ctl_t *handle, snd_ctl_elem_list_t *list);
	int (*element_info)(snd_ctl_t *handle, snd_ctl_elem_info_t *info);
	int (*element_add)(snd_ctl_t *handle, snd_ctl_elem_info_t *info);
	int (*element_replace)(snd_ctl_t *handle, snd_ctl_elem_info_t *info);
	int (*element_remove)(snd_ctl_t *handle, snd_ctl_elem_id_t *id);
	int (*element_read)(snd_ctl_t *handle, snd_ctl_elem_value_t *control);
	int (*element_write)(snd_ctl_t *handle, snd_ctl_elem_value_t *control);
	int (*element_lock)(snd_ctl_t *handle, snd_ctl_elem_id_t *lock);
	int (*element_unlock)(snd_ctl_t *handle, snd_ctl_elem_id_t *unlock);
	int (*element_tlv)(snd_ctl_t *handle, int op_flag, unsigned int numid,
			   unsigned int *tlv, unsigned int tlv_size);
	int (*hwdep_next_device)(snd_ctl_t *handle, int *device);
	int (*hwdep_info)(snd_ctl_t *handle, snd_hwdep_info_t * info);
	int (*pcm_next_device)(snd_ctl_t *handle, int *device);
	int (*pcm_info)(snd_ctl_t *handle, snd_pcm_info_t * info);
	int (*pcm_prefer_subdevice)(snd_ctl_t *handle, int subdev);
	int (*rawmidi_next_device)(snd_ctl_t *handle, int *device);
	int (*rawmidi_info)(snd_ctl_t *handle, snd_rawmidi_info_t * info);
	int (*rawmidi_prefer_subdevice)(snd_ctl_t *handle, int subdev);
	int (*set_power_state)(snd_ctl_t *handle, unsigned int state);
	int (*get_power_state)(snd_ctl_t *handle, unsigned int *state);
	int (*read)(snd_ctl_t *handle, snd_ctl_event_t *event);
	int (*poll_descriptors_count)(snd_ctl_t *handle);
	int (*poll_descriptors)(snd_ctl_t *handle, struct pollfd *pfds, unsigned int space);
	int (*poll_revents)(snd_ctl_t *handle, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
} snd_ctl_ops_t;


struct _snd_ctl {
	void *dl_handle;
	char *name;
	snd_ctl_type_t type;
	const snd_ctl_ops_t *ops;
	void *private_data;
	int nonblock;
	int poll_fd;
	struct list_head async_handlers;
};

struct _snd_hctl_elem {
	snd_ctl_elem_id_t id; 		/* must be always on top */
	struct list_head list;		/* links for list of all helems */
	int compare_weight;		/* compare weight (reversed) */
	/* event callback */
	snd_hctl_elem_callback_t callback;
	void *callback_private;
	/* links */
	snd_hctl_t *hctl;		/* associated handle */
};

struct _snd_hctl {
	snd_ctl_t *ctl;
	struct list_head elems;		/* list of all controls */
	unsigned int alloc;	
	unsigned int count;
	snd_hctl_elem_t **pelems;
	snd_hctl_compare_t compare;
	snd_hctl_callback_t callback;
	void *callback_private;
};


/* make local functions really local */
#define snd_ctl_new	snd1_ctl_new

int snd_ctl_new(snd_ctl_t **ctlp, snd_ctl_type_t type, const char *name);
int _snd_ctl_poll_descriptor(snd_ctl_t *ctl);
#define _snd_ctl_async_descriptor _snd_ctl_poll_descriptor
int snd_ctl_hw_open(snd_ctl_t **handle, const char *name, int card, int mode);
int snd_ctl_shm_open(snd_ctl_t **handlep, const char *name, const char *sockname, const char *sname, int mode);
int snd_ctl_async(snd_ctl_t *ctl, int sig, pid_t pid);
