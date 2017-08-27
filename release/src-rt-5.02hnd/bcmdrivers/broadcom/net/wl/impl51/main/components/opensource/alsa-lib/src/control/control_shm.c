/*
 *  Control - SHM Client
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
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
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netdb.h>
#include "aserver.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_control_shm = "";
#endif

#ifndef DOC_HIDDEN
typedef struct {
	int socket;
	volatile snd_ctl_shm_ctrl_t *ctrl;
} snd_ctl_shm_t;
#endif

static int snd_ctl_shm_action(snd_ctl_t *ctl)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	int err;
	char buf[1];
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	err = write(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	err = read(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	if (ctrl->cmd) {
		SNDERR("Server has not done the cmd");
		return -EBADFD;
	}
	return ctrl->result;
}

static int snd_ctl_shm_action_fd(snd_ctl_t *ctl, int *fd)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	int err;
	char buf[1];
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	err = write(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	err = snd_receive_fd(shm->socket, buf, 1, fd);
	if (err != 1)
		return -EBADFD;
	if (ctrl->cmd) {
		SNDERR("Server has not done the cmd");
		return -EBADFD;
	}
	return ctrl->result;
}

static int snd_ctl_shm_close(snd_ctl_t *ctl)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int result;
	ctrl->cmd = SND_CTL_IOCTL_CLOSE;
	result = snd_ctl_shm_action(ctl);
	shmdt((void *)ctrl);
	close(shm->socket);
	free(shm);
	return result;
}

static int snd_ctl_shm_nonblock(snd_ctl_t *handle ATTRIBUTE_UNUSED, int nonblock ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_ctl_shm_async(snd_ctl_t *ctl, int sig, pid_t pid)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SND_CTL_IOCTL_ASYNC;
	ctrl->u.async.sig = sig;
	if (pid == 0)
		pid = getpid();
	ctrl->u.async.pid = pid;
	return snd_ctl_shm_action(ctl);
}

static int snd_ctl_shm_poll_descriptor(snd_ctl_t *ctl)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int fd, err;
	ctrl->cmd = SND_CTL_IOCTL_POLL_DESCRIPTOR;
	err = snd_ctl_shm_action_fd(ctl, &fd);
	if (err < 0)
		return err;
	return fd;
}

static int snd_ctl_shm_subscribe_events(snd_ctl_t *ctl, int subscribe)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS;
	ctrl->u.subscribe_events = subscribe;
	return snd_ctl_shm_action(ctl);
}

static int snd_ctl_shm_card_info(snd_ctl_t *ctl, snd_ctl_card_info_t *info)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
//	ctrl->u.card_info = *info;
	ctrl->cmd = SNDRV_CTL_IOCTL_CARD_INFO;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*info = ctrl->u.card_info;
	return err;
}

static int snd_ctl_shm_elem_list(snd_ctl_t *ctl, snd_ctl_elem_list_t *list)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	size_t maxsize = CTL_SHM_DATA_MAXLEN;
	size_t bytes = list->space * sizeof(*list->pids);
	int err;
	snd_ctl_elem_id_t *pids = list->pids;
	if (bytes > maxsize)
		return -EINVAL;
	ctrl->u.element_list = *list;
	ctrl->cmd = SNDRV_CTL_IOCTL_ELEM_LIST;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*list = ctrl->u.element_list;
	list->pids = pids;
	bytes = list->used * sizeof(*list->pids);
	memcpy(pids, (void *)ctrl->data, bytes);
	return err;
}

static int snd_ctl_shm_elem_info(snd_ctl_t *ctl, snd_ctl_elem_info_t *info)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.element_info = *info;
	ctrl->cmd = SNDRV_CTL_IOCTL_ELEM_INFO;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*info = ctrl->u.element_info;
	return err;
}

static int snd_ctl_shm_elem_read(snd_ctl_t *ctl, snd_ctl_elem_value_t *control)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.element_read = *control;
	ctrl->cmd = SNDRV_CTL_IOCTL_ELEM_READ;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*control = ctrl->u.element_read;
	return err;
}

static int snd_ctl_shm_elem_write(snd_ctl_t *ctl, snd_ctl_elem_value_t *control)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.element_write = *control;
	ctrl->cmd = SNDRV_CTL_IOCTL_ELEM_WRITE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*control = ctrl->u.element_write;
	return err;
}

static int snd_ctl_shm_elem_lock(snd_ctl_t *ctl, snd_ctl_elem_id_t *id)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.element_lock = *id;
	ctrl->cmd = SNDRV_CTL_IOCTL_ELEM_LOCK;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*id = ctrl->u.element_lock;
	return err;
}

static int snd_ctl_shm_elem_unlock(snd_ctl_t *ctl, snd_ctl_elem_id_t *id)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.element_unlock = *id;
	ctrl->cmd = SNDRV_CTL_IOCTL_ELEM_UNLOCK;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*id = ctrl->u.element_unlock;
	return err;
}

static int snd_ctl_shm_hwdep_next_device(snd_ctl_t *ctl, int * device)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.device = *device;
	ctrl->cmd = SNDRV_CTL_IOCTL_HWDEP_NEXT_DEVICE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*device = ctrl->u.device;
	return err;
}

static int snd_ctl_shm_hwdep_info(snd_ctl_t *ctl, snd_hwdep_info_t * info)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.hwdep_info = *info;
	ctrl->cmd = SNDRV_CTL_IOCTL_HWDEP_INFO;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*info = ctrl->u.hwdep_info;
	return err;
}

static int snd_ctl_shm_pcm_next_device(snd_ctl_t *ctl, int * device)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.device = *device;
	ctrl->cmd = SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*device = ctrl->u.device;
	return err;
}

static int snd_ctl_shm_pcm_info(snd_ctl_t *ctl, snd_pcm_info_t * info)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.pcm_info = *info;
	ctrl->cmd = SNDRV_CTL_IOCTL_PCM_INFO;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*info = ctrl->u.pcm_info;
	return err;
}

static int snd_ctl_shm_pcm_prefer_subdevice(snd_ctl_t *ctl, int subdev)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.pcm_prefer_subdevice = subdev;
	ctrl->cmd = SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	return err;
}

static int snd_ctl_shm_rawmidi_next_device(snd_ctl_t *ctl, int * device)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.device = *device;
	ctrl->cmd = SNDRV_CTL_IOCTL_RAWMIDI_NEXT_DEVICE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*device = ctrl->u.device;
	return err;
}

static int snd_ctl_shm_rawmidi_info(snd_ctl_t *ctl, snd_rawmidi_info_t * info)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.rawmidi_info = *info;
	ctrl->cmd = SNDRV_CTL_IOCTL_RAWMIDI_INFO;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*info = ctrl->u.rawmidi_info;
	return err;
}

static int snd_ctl_shm_rawmidi_prefer_subdevice(snd_ctl_t *ctl, int subdev)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.rawmidi_prefer_subdevice = subdev;
	ctrl->cmd = SNDRV_CTL_IOCTL_RAWMIDI_PREFER_SUBDEVICE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	return err;
}

static int snd_ctl_shm_set_power_state(snd_ctl_t *ctl, unsigned int state)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.power_state = state;
	ctrl->cmd = SNDRV_CTL_IOCTL_POWER;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	return err;
}

static int snd_ctl_shm_get_power_state(snd_ctl_t *ctl, unsigned int *state)
{
	snd_ctl_shm_t *shm = ctl->private_data;
	volatile snd_ctl_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->cmd = SNDRV_CTL_IOCTL_POWER_STATE;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*state = ctrl->u.power_state;
	return err;
}

static int snd_ctl_shm_read(snd_ctl_t *ctl, snd_ctl_event_t *event)
{
	snd_ctl_shm_t *shm;
	volatile snd_ctl_shm_ctrl_t *ctrl;
	int err;
	err = snd_ctl_wait(ctl, -1);
	if (err < 0)
		return 0;
	shm = ctl->private_data;
	ctrl = shm->ctrl;
	ctrl->u.read = *event;
	ctrl->cmd = SND_CTL_IOCTL_READ;
	err = snd_ctl_shm_action(ctl);
	if (err < 0)
		return err;
	*event = ctrl->u.read;
	return err;
}

static const snd_ctl_ops_t snd_ctl_shm_ops = {
	.close = snd_ctl_shm_close,
	.nonblock = snd_ctl_shm_nonblock,
	.async = snd_ctl_shm_async,
	.subscribe_events = snd_ctl_shm_subscribe_events,
	.card_info = snd_ctl_shm_card_info,
	.element_list = snd_ctl_shm_elem_list,
	.element_info = snd_ctl_shm_elem_info,
	.element_read = snd_ctl_shm_elem_read,
	.element_write = snd_ctl_shm_elem_write,
	.element_lock = snd_ctl_shm_elem_lock,
	.element_unlock = snd_ctl_shm_elem_unlock,
	.hwdep_next_device = snd_ctl_shm_hwdep_next_device,
	.hwdep_info = snd_ctl_shm_hwdep_info,
	.pcm_next_device = snd_ctl_shm_pcm_next_device,
	.pcm_info = snd_ctl_shm_pcm_info,
	.pcm_prefer_subdevice = snd_ctl_shm_pcm_prefer_subdevice,
	.rawmidi_next_device = snd_ctl_shm_rawmidi_next_device,
	.rawmidi_info = snd_ctl_shm_rawmidi_info,
	.rawmidi_prefer_subdevice = snd_ctl_shm_rawmidi_prefer_subdevice,
	.set_power_state = snd_ctl_shm_set_power_state,
	.get_power_state = snd_ctl_shm_get_power_state,
	.read = snd_ctl_shm_read,
};

static int make_local_socket(const char *filename)
{
	size_t l = strlen(filename);
	size_t size = offsetof(struct sockaddr_un, sun_path) + l;
	struct sockaddr_un *addr = alloca(size);
	int sock;

	sock = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sock < 0)
		return -errno;
	
	addr->sun_family = AF_LOCAL;
	memcpy(addr->sun_path, filename, l);

	if (connect(sock, (struct sockaddr *) addr, size) < 0)
		return -errno;
	return sock;
}


int snd_ctl_shm_open(snd_ctl_t **handlep, const char *name, const char *sockname, const char *sname, int mode)
{
	snd_ctl_t *ctl;
	snd_ctl_shm_t *shm = NULL;
	snd_client_open_request_t *req;
	snd_client_open_answer_t ans;
	size_t snamelen, reqlen;
	int err;
	int result;
	int sock = -1;
	snd_ctl_shm_ctrl_t *ctrl = NULL;
	snamelen = strlen(sname);
	if (snamelen > 255)
		return -EINVAL;

	result = make_local_socket(sockname);
	if (result < 0) {
		SNDERR("server for socket %s is not running", sockname);
		goto _err;
	}
	sock = result;

	reqlen = sizeof(*req) + snamelen;
	req = alloca(reqlen);
	memcpy(req->name, sname, snamelen);
	req->dev_type = SND_DEV_TYPE_CONTROL;
	req->transport_type = SND_TRANSPORT_TYPE_SHM;
	req->stream = 0;
	req->mode = mode;
	req->namelen = snamelen;
	err = write(sock, req, reqlen);
	if (err < 0) {
		SNDERR("write error");
		result = -errno;
		goto _err;
	}
	if ((size_t) err != reqlen) {
		SNDERR("write size error");
		result = -EINVAL;
		goto _err;
	}
	err = read(sock, &ans, sizeof(ans));
	if (err < 0) {
		SNDERR("read error");
		result = -errno;
		goto _err;
	}
	if (err != sizeof(ans)) {
		SNDERR("read size error");
		result = -EINVAL;
		goto _err;
	}
	result = ans.result;
	if (result < 0)
		goto _err;

	ctrl = shmat(ans.cookie, 0, 0);
	if (!ctrl) {
		result = -errno;
		goto _err;
	}
		
	shm = calloc(1, sizeof(snd_ctl_shm_t));
	if (!shm) {
		result = -ENOMEM;
		goto _err;
	}

	shm->socket = sock;
	shm->ctrl = ctrl;

	err = snd_ctl_new(&ctl, SND_CTL_TYPE_SHM, name);
	if (err < 0) {
		result = err;
		goto _err;
	}
	ctl->ops = &snd_ctl_shm_ops;
	ctl->private_data = shm;
	err = snd_ctl_shm_poll_descriptor(ctl);
	if (err < 0) {
		snd_ctl_close(ctl);
		return err;
	}
	ctl->poll_fd = err;
	*handlep = ctl;
	return 0;

 _err:
	close(sock);
	if (ctrl)
		shmdt(ctrl);
	free(shm);
	return result;
}

int _snd_ctl_shm_open(snd_ctl_t **handlep, char *name, snd_config_t *root, snd_config_t *conf, int mode)
{
	snd_config_iterator_t i, next;
	const char *server = NULL;
	const char *ctl_name = NULL;
	snd_config_t *sconfig;
	const char *host = NULL;
	const char *sockname = NULL;
	long port = -1;
	int err;
	int local;
	struct hostent *h;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "type") == 0)
			continue;
		if (strcmp(id, "server") == 0) {
			err = snd_config_get_string(n, &server);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "ctl") == 0) {
			err = snd_config_get_string(n, &ctl_name);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!ctl_name) {
		SNDERR("ctl is not defined");
		return -EINVAL;
	}
	if (!server) {
		SNDERR("server is not defined");
		return -EINVAL;
	}
	err = snd_config_search_definition(root, "server", server, &sconfig);
	if (err < 0) {
		SNDERR("Unknown server %s", server);
		return -EINVAL;
	}
	if (snd_config_get_type(sconfig) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("Invalid type for server %s definition", server);
		err = -EINVAL;
		goto _err;
	}
	snd_config_for_each(i, next, sconfig) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "host") == 0) {
			err = snd_config_get_string(n, &host);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "socket") == 0) {
			err = snd_config_get_string(n, &sockname);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "port") == 0) {
			err = snd_config_get_integer(n, &port);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto _err;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		err = -EINVAL;
		goto _err;
	}

	if (!host) {
		SNDERR("host is not defined");
		goto _err;
	}
	if (!sockname) {
		SNDERR("socket is not defined");
		goto _err;
	}
	h = gethostbyname(host);
	if (!h) {
		SNDERR("Cannot resolve %s", host);
		goto _err;
	}
	local = snd_is_local(h);
	if (!local) {
		SNDERR("%s is not the local host", host);
		goto _err;
	}
	err = snd_ctl_shm_open(handlep, name, sockname, ctl_name, mode);
       _err:
	snd_config_delete(sconfig);
	return err;
}
SND_DLSYM_BUILD_VERSION(_snd_ctl_shm_open, SND_CONTROL_DLSYM_VERSION);
