/*
 *  ALSA client/server header file
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
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
  
#include <netdb.h>
#include "../src/pcm/pcm_local.h"
#include "../src/control/control_local.h"

int snd_receive_fd(int sock, void *data, size_t len, int *fd);
int snd_is_local(struct hostent *hent);

typedef enum _snd_dev_type {
	SND_DEV_TYPE_PCM,
	SND_DEV_TYPE_CONTROL,
	SND_DEV_TYPE_RAWMIDI,
	SND_DEV_TYPE_TIMER,
	SND_DEV_TYPE_HWDEP,
	SND_DEV_TYPE_SEQ,
} snd_dev_type_t;

typedef enum _snd_transport_type {
	SND_TRANSPORT_TYPE_SHM,
	SND_TRANSPORT_TYPE_TCP,
} snd_transport_type_t;

#define SND_PCM_IOCTL_HWSYNC		_IO ('A', 0x22)
#define SND_PCM_IOCTL_STATE		_IO ('A', 0xf1)
#define SND_PCM_IOCTL_MMAP		_IO ('A', 0xf2)
#define SND_PCM_IOCTL_MUNMAP		_IO ('A', 0xf3)
#define SND_PCM_IOCTL_MMAP_COMMIT	_IO ('A', 0xf4)
#define SND_PCM_IOCTL_AVAIL_UPDATE	_IO ('A', 0xf5)
#define SND_PCM_IOCTL_ASYNC		_IO ('A', 0xf6)
#define SND_PCM_IOCTL_CLOSE		_IO ('A', 0xf7)
#define SND_PCM_IOCTL_POLL_DESCRIPTOR	_IO ('A', 0xf8)
#define SND_PCM_IOCTL_HW_PTR_FD		_IO ('A', 0xf9)
#define SND_PCM_IOCTL_APPL_PTR_FD	_IO ('A', 0xfa)
#define SND_PCM_IOCTL_FORWARD		_IO ('A', 0xfb)

typedef struct {
	snd_pcm_uframes_t ptr;
	int use_mmap;
	off_t offset;		/* for mmap */
	int changed;
} snd_pcm_shm_rbptr_t;

typedef struct {
	long result;
	int cmd;
	snd_pcm_shm_rbptr_t hw;
	snd_pcm_shm_rbptr_t appl;
	union {
		struct {
			int sig;
			pid_t pid;
		} async;
		snd_pcm_info_t info;
		snd_pcm_hw_params_t hw_refine;
		snd_pcm_hw_params_t hw_params;
		snd_pcm_sw_params_t sw_params;
		snd_pcm_status_t status;
		struct {
			snd_pcm_uframes_t frames;
		} avail;
		struct {
			snd_pcm_sframes_t frames;
		} delay;
		struct {
			int enable;
		} pause;
		snd_pcm_channel_info_t channel_info;
		struct {
			snd_pcm_uframes_t frames;
		} rewind;
		struct {
			snd_pcm_uframes_t frames;
		} forward;
		struct {
			int fd;
		} link;
		struct {
			snd_pcm_uframes_t offset;
			snd_pcm_uframes_t frames;
		} mmap_commit;
		struct {
			char use_mmap;
			int shmid;
			off_t offset;
		} rbptr;
	} u;
	char data[0];
} snd_pcm_shm_ctrl_t;

#define PCM_SHM_SIZE sizeof(snd_pcm_shm_ctrl_t)
		
#define SND_CTL_IOCTL_READ		_IOR('U', 0xf1, snd_ctl_event_t)
#define SND_CTL_IOCTL_CLOSE		_IO ('U', 0xf2)
#define SND_CTL_IOCTL_POLL_DESCRIPTOR	_IO ('U', 0xf3)
#define SND_CTL_IOCTL_ASYNC		_IO ('U', 0xf4)

typedef struct {
	int result;
	int cmd;
	union {
		struct {
			int sig;
			pid_t pid;
		} async;
		int device;
		int subscribe_events;
		snd_ctl_card_info_t card_info;
		snd_ctl_elem_list_t element_list;
		snd_ctl_elem_info_t element_info;
		snd_ctl_elem_value_t element_read;
		snd_ctl_elem_value_t element_write;
		snd_ctl_elem_id_t element_lock;
		snd_ctl_elem_id_t element_unlock;
		snd_hwdep_info_t hwdep_info;
		snd_pcm_info_t pcm_info;
		int pcm_prefer_subdevice;
		snd_rawmidi_info_t rawmidi_info;
		int rawmidi_prefer_subdevice;
		unsigned int power_state;
		snd_ctl_event_t read;
	} u;
	char data[0];
} snd_ctl_shm_ctrl_t;

#define CTL_SHM_SIZE 65536
#define CTL_SHM_DATA_MAXLEN (CTL_SHM_SIZE - offsetof(snd_ctl_shm_ctrl_t, data))

typedef struct {
	unsigned char dev_type;
	unsigned char transport_type;
	unsigned char stream;
	unsigned char mode;
	unsigned char namelen;
	char name[0];
} snd_client_open_request_t;

typedef struct {
	long result;
	int cookie;
} snd_client_open_answer_t;
