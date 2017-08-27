/*
 *  PCM - Direct Stream Mixing
 *  Copyright (c) 2003 by Jaroslav Kysela <perex@perex.cz>
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
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <grp.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/mman.h>
#include "pcm_direct.h"

/*
 *
 */
 
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux specific) */
};
 

int snd_pcm_direct_semaphore_create_or_connect(snd_pcm_direct_t *dmix)
{
	union semun s;
	struct semid_ds buf;
	int i;

	dmix->semid = semget(dmix->ipc_key, DIRECT_IPC_SEMS,
			     IPC_CREAT | dmix->ipc_perm);
	if (dmix->semid < 0)
		return -errno;
	if (dmix->ipc_gid < 0)
		return 0;
	for (i = 0; i < DIRECT_IPC_SEMS; i++) {
		s.buf = &buf;
		if (semctl(dmix->semid, i, IPC_STAT, s) < 0) {
			int err = -errno;
			snd_pcm_direct_semaphore_discard(dmix);
			return err;
		}
		buf.sem_perm.gid = dmix->ipc_gid;
		s.buf = &buf;
		semctl(dmix->semid, i, IPC_SET, s);
	}
	return 0;
}

#define SND_PCM_DIRECT_MAGIC	(0xa15ad300 + sizeof(snd_pcm_direct_share_t))

/*
 *  global shared memory area 
 */

int snd_pcm_direct_shm_create_or_connect(snd_pcm_direct_t *dmix)
{
	struct shmid_ds buf;
	int tmpid, err;
	
retryget:
	dmix->shmid = shmget(dmix->ipc_key, sizeof(snd_pcm_direct_share_t),
			     IPC_CREAT | dmix->ipc_perm);
	err = -errno;
	if (dmix->shmid < 0){
		if (errno == EINVAL)
		if ((tmpid = shmget(dmix->ipc_key, 0, dmix->ipc_perm)) != -1)
		if (!shmctl(tmpid, IPC_STAT, &buf))
		if (!buf.shm_nattch)
	    	/* no users so destroy the segment */
		if (!shmctl(tmpid, IPC_RMID, NULL))
		    goto retryget;
		return err;
	}
	dmix->shmptr = shmat(dmix->shmid, 0, 0);
	if (dmix->shmptr == (void *) -1) {
		err = -errno;
		snd_pcm_direct_shm_discard(dmix);
		return err;
	}
	mlock(dmix->shmptr, sizeof(snd_pcm_direct_share_t));
	if (shmctl(dmix->shmid, IPC_STAT, &buf) < 0) {
		err = -errno;
		snd_pcm_direct_shm_discard(dmix);
		return err;
	}
	if (buf.shm_nattch == 1) {	/* we're the first user, clear the segment */
		memset(dmix->shmptr, 0, sizeof(snd_pcm_direct_share_t));
		if (dmix->ipc_gid >= 0) {
			buf.shm_perm.gid = dmix->ipc_gid;
			shmctl(dmix->shmid, IPC_SET, &buf);
		}
		dmix->shmptr->magic = SND_PCM_DIRECT_MAGIC;
		return 1;
	} else {
		if (dmix->shmptr->magic != SND_PCM_DIRECT_MAGIC) {
			snd_pcm_direct_shm_discard(dmix);
			return -EINVAL;
		}
	}
	return 0;
}

/* discard shared memory */
/*
 * Define snd_* functions to be used in server.
 * Since objects referred in a plugin can be released dynamically, a forked
 * server should have statically linked functions.
 * (e.g. Novell bugzilla #105772)
 */
static int _snd_pcm_direct_shm_discard(snd_pcm_direct_t *dmix)
{
	struct shmid_ds buf;
	int ret = 0;

	if (dmix->shmid < 0)
		return -EINVAL;
	if (dmix->shmptr != (void *) -1 && shmdt(dmix->shmptr) < 0)
		return -errno;
	dmix->shmptr = (void *) -1;
	if (shmctl(dmix->shmid, IPC_STAT, &buf) < 0)
		return -errno;
	if (buf.shm_nattch == 0) {	/* we're the last user, destroy the segment */
		if (shmctl(dmix->shmid, IPC_RMID, NULL) < 0)
			return -errno;
		ret = 1;
	}
	dmix->shmid = -1;
	return ret;
}

/* ... and an exported version */
int snd_pcm_direct_shm_discard(snd_pcm_direct_t *dmix)
{
	return _snd_pcm_direct_shm_discard(dmix);
}

/*
 *  server side
 */

static int get_tmp_name(char *filename, size_t size)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	snprintf(filename, size, TMPDIR "/alsa-dmix-%i-%li-%li", (int)getpid(), (long)tv.tv_sec, (long)tv.tv_usec);
	filename[size-1] = '\0';
	return 0;
}

static int make_local_socket(const char *filename, int server, mode_t ipc_perm, int ipc_gid)
{
	size_t l = strlen(filename);
	size_t size = offsetof(struct sockaddr_un, sun_path) + l;
	struct sockaddr_un *addr = alloca(size);
	int sock;

	sock = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sock < 0) {
		int result = -errno;
		SYSERR("socket failed");
		return result;
	}

	if (server)
		unlink(filename);
	memset(addr, 0, size); /* make valgrind happy */
	addr->sun_family = AF_LOCAL;
	memcpy(addr->sun_path, filename, l);
	
	if (server) {
		if (bind(sock, (struct sockaddr *) addr, size) < 0) {
			int result = -errno;
			SYSERR("bind failed: %s", filename);
			close(sock);
			return result;
		} else {
			if (chmod(filename, ipc_perm) < 0) {
				int result = -errno;
				SYSERR("chmod failed: %s", filename);
				close(sock);
				unlink(filename);
				return result;
			}
			if (chown(filename, -1, ipc_gid) < 0) {
			}
		}
	} else {
		if (connect(sock, (struct sockaddr *) addr, size) < 0) {
			int result = -errno;
			SYSERR("connect failed: %s", filename);
			close(sock);
			return result;
		}
	}
	return sock;
}

#undef SERVER_JOB_DEBUG
#define server_printf(fmt, args...) /* nothing */

static snd_pcm_direct_t *server_job_dmix;

static void server_cleanup(snd_pcm_direct_t *dmix)
{
	close(dmix->server_fd);
	close(dmix->hw_fd);
	if (dmix->server_free)
		dmix->server_free(dmix);
	unlink(dmix->shmptr->socket_name);
	_snd_pcm_direct_shm_discard(dmix);
	snd_pcm_direct_semaphore_discard(dmix);
}

static void server_job_signal(int sig ATTRIBUTE_UNUSED)
{
	snd_pcm_direct_semaphore_down(server_job_dmix, DIRECT_IPC_SEM_CLIENT);
	server_cleanup(server_job_dmix);
	server_printf("DIRECT SERVER EXIT - SIGNAL\n");
	_exit(EXIT_SUCCESS);
}

/* This is a copy from ../socket.c, provided here only for a server job
 * (see the comment above)
 */
static int _snd_send_fd(int sock, void *data, size_t len, int fd)
{
	int ret;
	size_t cmsg_len = CMSG_LEN(sizeof(int));
	struct cmsghdr *cmsg = alloca(cmsg_len);
	int *fds = (int *) CMSG_DATA(cmsg);
	struct msghdr msghdr;
	struct iovec vec;

	vec.iov_base = (void *)&data;
	vec.iov_len = len;

	cmsg->cmsg_len = cmsg_len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*fds = fd;

	msghdr.msg_name = NULL;
	msghdr.msg_namelen = 0;
	msghdr.msg_iov = &vec;
 	msghdr.msg_iovlen = 1;
	msghdr.msg_control = cmsg;
	msghdr.msg_controllen = cmsg_len;
	msghdr.msg_flags = 0;

	ret = sendmsg(sock, &msghdr, 0 );
	if (ret < 0)
		return -errno;
	return ret;
}

static void server_job(snd_pcm_direct_t *dmix)
{
	int ret, sck, i;
	int max = 128, current = 0;
	struct pollfd pfds[max + 1];

	server_job_dmix = dmix;
	/* don't allow to be killed */
	signal(SIGHUP, server_job_signal);
	signal(SIGQUIT, server_job_signal);
	signal(SIGTERM, server_job_signal);
	signal(SIGKILL, server_job_signal);
	/* close all files to free resources */
	i = sysconf(_SC_OPEN_MAX);
#ifdef SERVER_JOB_DEBUG
	while (--i >= 3) {
#else
	while (--i >= 0) {
#endif
		if (i != dmix->server_fd && i != dmix->hw_fd)
			close(i);
	}
	
	/* detach from parent */
	setsid();

	pfds[0].fd = dmix->server_fd;
	pfds[0].events = POLLIN | POLLERR | POLLHUP;

	server_printf("DIRECT SERVER STARTED\n");
	while (1) {
		ret = poll(pfds, current + 1, 500);
		server_printf("DIRECT SERVER: poll ret = %i, revents[0] = 0x%x, errno = %i\n", ret, pfds[0].revents, errno);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			/* some error */
			break;
		}
		if (ret == 0 || (pfds[0].revents & (POLLERR | POLLHUP))) {	/* timeout or error? */
			struct shmid_ds buf;
			snd_pcm_direct_semaphore_down(dmix, DIRECT_IPC_SEM_CLIENT);
			if (shmctl(dmix->shmid, IPC_STAT, &buf) < 0) {
				_snd_pcm_direct_shm_discard(dmix);
				snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
				continue;
			}
			server_printf("DIRECT SERVER: nattch = %i\n", (int)buf.shm_nattch);
			if (buf.shm_nattch == 1)	/* server is the last user, exit */
				break;
			snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
			continue;
		}
		if (pfds[0].revents & POLLIN) {
			ret--;
			sck = accept(dmix->server_fd, 0, 0);
			if (sck >= 0) {
				server_printf("DIRECT SERVER: new connection %i\n", sck);
				if (current == max) {
					close(sck);
				} else {
					unsigned char buf = 'A';
					pfds[current+1].fd = sck;
					pfds[current+1].events = POLLIN | POLLERR | POLLHUP;
					_snd_send_fd(sck, &buf, 1, dmix->hw_fd);
					server_printf("DIRECT SERVER: fd sent ok\n");
					current++;
				}
			}
		}
		for (i = 0; i < current && ret > 0; i++) {
			struct pollfd *pfd = &pfds[i+1];
			unsigned char cmd;
			server_printf("client %i revents = 0x%x\n", pfd->fd, pfd->revents);
			if (pfd->revents & (POLLERR | POLLHUP)) {
				ret--;
				close(pfd->fd);
				pfd->fd = -1;
				continue;
			}
			if (!(pfd->revents & POLLIN))
				continue;
			ret--;
			if (read(pfd->fd, &cmd, 1) == 1)
				cmd = 0 /*process command */;
		}
		for (i = 0; i < current; i++) {
			if (pfds[i+1].fd < 0) {
				if (i + 1 != max)
					memcpy(&pfds[i+1], &pfds[i+2], sizeof(struct pollfd) * (max - i - 1));
				current--;
			}
		}
	}
	server_cleanup(dmix);
	server_printf("DIRECT SERVER EXIT\n");
#ifdef SERVER_JOB_DEBUG
	close(0); close(1); close(2);
#endif
	_exit(EXIT_SUCCESS);
}

int snd_pcm_direct_server_create(snd_pcm_direct_t *dmix)
{
	int ret;

	dmix->server_fd = -1;

	ret = get_tmp_name(dmix->shmptr->socket_name, sizeof(dmix->shmptr->socket_name));
	if (ret < 0)
		return ret;
	
	ret = make_local_socket(dmix->shmptr->socket_name, 1, dmix->ipc_perm, dmix->ipc_gid);
	if (ret < 0)
		return ret;
	dmix->server_fd = ret;

	ret = listen(dmix->server_fd, 4);
	if (ret < 0) {
		close(dmix->server_fd);
		return ret;
	}
	
	ret = fork();
	if (ret < 0) {
		close(dmix->server_fd);
		return ret;
	} else if (ret == 0) {
		ret = fork();
		if (ret == 0)
			server_job(dmix);
		_exit(EXIT_SUCCESS);
	} else {
		waitpid(ret, NULL, 0);
	}
	dmix->server_pid = ret;
	dmix->server = 1;
	return 0;
}

int snd_pcm_direct_server_discard(snd_pcm_direct_t *dmix)
{
	if (dmix->server) {
		//kill(dmix->server_pid, SIGTERM);
		//waitpid(dmix->server_pid, NULL, 0);
		dmix->server_pid = (pid_t)-1;
	}
	if (dmix->server_fd > 0) {
		close(dmix->server_fd);
		dmix->server_fd = -1;
	}
	dmix->server = 0;
	return 0;
}

/*
 *  client side
 */

int snd_pcm_direct_client_connect(snd_pcm_direct_t *dmix)
{
	int ret;
	unsigned char buf;

	ret = make_local_socket(dmix->shmptr->socket_name, 0, -1, -1);
	if (ret < 0)
		return ret;
	dmix->comm_fd = ret;

	ret = snd_receive_fd(dmix->comm_fd, &buf, 1, &dmix->hw_fd);
	if (ret < 1 || buf != 'A') {
		close(dmix->comm_fd);
		dmix->comm_fd = -1;
		return ret;
	}

	dmix->client = 1;
	return 0;
}

int snd_pcm_direct_client_discard(snd_pcm_direct_t *dmix)
{
	if (dmix->client) {
		close(dmix->comm_fd);
		dmix->comm_fd = -1;
	}
	return 0;
}

/*
 *  plugin helpers
 */

int snd_pcm_direct_nonblock(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int nonblock ATTRIBUTE_UNUSED)
{
	/* value is cached for us in pcm->mode (SND_PCM_NONBLOCK flag) */
	return 0;
}

int snd_pcm_direct_async(snd_pcm_t *pcm, int sig, pid_t pid)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	return snd_timer_async(dmix->timer, sig, pid);
}

/* empty the timer read queue */
void snd_pcm_direct_clear_timer_queue(snd_pcm_direct_t *dmix)
{
	if (dmix->timer_need_poll) {
		while (poll(&dmix->timer_fd, 1, 0) > 0) {
			/* we don't need the value */
			if (dmix->tread) {
				snd_timer_tread_t rbuf[4];
				snd_timer_read(dmix->timer, rbuf, sizeof(rbuf));
			} else {
				snd_timer_read_t rbuf;
				snd_timer_read(dmix->timer, &rbuf, sizeof(rbuf));
			}
		}
	} else {
		if (dmix->tread) {
			snd_timer_tread_t rbuf[4];
			int len;
			while ((len = snd_timer_read(dmix->timer, rbuf,
						     sizeof(rbuf))) > 0 &&
			       len != sizeof(rbuf[0]))
				;
		} else {
			snd_timer_read_t rbuf;
			while (snd_timer_read(dmix->timer, &rbuf, sizeof(rbuf)) > 0)
				;
		}
	}
}

int snd_pcm_direct_timer_stop(snd_pcm_direct_t *dmix)
{
	snd_timer_stop(dmix->timer);
	return 0;
}

int snd_pcm_direct_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	unsigned short events;
	int empty = 0;

	assert(pfds && nfds == 1 && revents);
	events = pfds[0].revents;
	if (events & POLLIN) {
		snd_pcm_uframes_t avail;
		snd_pcm_avail_update(pcm);
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
			events |= POLLOUT;
			events &= ~POLLIN;
			avail = snd_pcm_mmap_playback_avail(pcm);
		} else {
			avail = snd_pcm_mmap_capture_avail(pcm);
		}
		empty = avail < pcm->avail_min;
	}
	switch (snd_pcm_state(dmix->spcm)) {
	case SND_PCM_STATE_XRUN:
	case SND_PCM_STATE_SUSPENDED:
	case SND_PCM_STATE_SETUP:
		events |= POLLERR;
		break;
	default:
		if (empty) {
			snd_pcm_direct_clear_timer_queue(dmix);
			events &= ~(POLLOUT|POLLIN);
			/* additional check */
			switch (snd_pcm_state(pcm)) {
			case SND_PCM_STATE_XRUN:
			case SND_PCM_STATE_SUSPENDED:
			case SND_PCM_STATE_SETUP:
				events |= POLLERR;
				break;
			default:
				break;
			}
		}
		break;
	}
	*revents = events;
	return 0;
}

int snd_pcm_direct_info(snd_pcm_t *pcm, snd_pcm_info_t * info)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	if (dmix->spcm && !dmix->shmptr->use_server)
		return snd_pcm_info(dmix->spcm, info);

	memset(info, 0, sizeof(*info));
	info->stream = pcm->stream;
	info->card = -1;
	if (pcm->name) {
		strncpy((char *)info->id, pcm->name, sizeof(info->id));
		strncpy((char *)info->name, pcm->name, sizeof(info->name));
		strncpy((char *)info->subname, pcm->name, sizeof(info->subname));
	}
	info->subdevices_count = 1;
	return 0;
}

static inline snd_mask_t *hw_param_mask(snd_pcm_hw_params_t *params,
					snd_pcm_hw_param_t var)
{
	return &params->masks[var - SND_PCM_HW_PARAM_FIRST_MASK];
}

static inline snd_interval_t *hw_param_interval(snd_pcm_hw_params_t *params,
						snd_pcm_hw_param_t var)
{
	return &params->intervals[var - SND_PCM_HW_PARAM_FIRST_INTERVAL];
}

static int hw_param_interval_refine_one(snd_pcm_hw_params_t *params,
					snd_pcm_hw_param_t var,
					snd_interval_t *src)
{
	snd_interval_t *i;

	if (!(params->rmask & (1<<var)))	/* nothing to do? */
		return 0;
	i = hw_param_interval(params, var);
	if (snd_interval_empty(i)) {
		SNDERR("dshare interval %i empty?", (int)var);
		return -EINVAL;
	}
	if (snd_interval_refine(i, src))
		params->cmask |= 1<<var;
	return 0;
}

static int hw_param_interval_refine_minmax(snd_pcm_hw_params_t *params,
					   snd_pcm_hw_param_t var,
					   unsigned int imin,
					   unsigned int imax)
{
	snd_interval_t t;

	memset(&t, 0, sizeof(t));
	snd_interval_set_minmax(&t, imin, imax);
	t.integer = 1;
	return hw_param_interval_refine_one(params, var, &t);
}

#undef REFINE_DEBUG

int snd_pcm_direct_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_direct_t *dshare = pcm->private_data;
	static const snd_mask_t access = { .bits = { 
					(1<<SNDRV_PCM_ACCESS_MMAP_INTERLEAVED) |
					(1<<SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED) |
					(1<<SNDRV_PCM_ACCESS_RW_INTERLEAVED) |
					(1<<SNDRV_PCM_ACCESS_RW_NONINTERLEAVED),
					0, 0, 0 } };
	int err;

#ifdef REFINE_DEBUG
	snd_output_t *log;
	snd_output_stdio_attach(&log, stderr, 0);
	snd_output_puts(log, "DMIX REFINE (begin):\n");
	snd_pcm_hw_params_dump(params, log);
#endif
	if (params->rmask & (1<<SND_PCM_HW_PARAM_ACCESS)) {
		if (snd_mask_empty(hw_param_mask(params, SND_PCM_HW_PARAM_ACCESS))) {
			SNDERR("dshare access mask empty?");
			return -EINVAL;
		}
		if (snd_mask_refine(hw_param_mask(params, SND_PCM_HW_PARAM_ACCESS), &access))
			params->cmask |= 1<<SND_PCM_HW_PARAM_ACCESS;
	}
	if (params->rmask & (1<<SND_PCM_HW_PARAM_FORMAT)) {
		if (snd_mask_empty(hw_param_mask(params, SND_PCM_HW_PARAM_FORMAT))) {
			SNDERR("dshare format mask empty?");
			return -EINVAL;
		}
		if (snd_mask_refine_set(hw_param_mask(params, SND_PCM_HW_PARAM_FORMAT),
					dshare->shmptr->hw.format))
			params->cmask |= 1<<SND_PCM_HW_PARAM_FORMAT;
	}
	//snd_mask_none(hw_param_mask(params, SND_PCM_HW_PARAM_SUBFORMAT));
	if (params->rmask & (1<<SND_PCM_HW_PARAM_CHANNELS)) {
		if (snd_interval_empty(hw_param_interval(params, SND_PCM_HW_PARAM_CHANNELS))) {
			SNDERR("dshare channels mask empty?");
			return -EINVAL;
		}
		err = snd_interval_refine_set(hw_param_interval(params, SND_PCM_HW_PARAM_CHANNELS), dshare->channels);
		if (err < 0)
			return err;
	}
	err = hw_param_interval_refine_one(params, SND_PCM_HW_PARAM_RATE,
					   &dshare->shmptr->hw.rate);
	if (err < 0)
		return err;
	err = hw_param_interval_refine_one(params, SND_PCM_HW_PARAM_PERIOD_SIZE,
					   &dshare->shmptr->hw.period_size);
	if (err < 0)
		return err;
	err = hw_param_interval_refine_one(params, SND_PCM_HW_PARAM_PERIOD_TIME,
					   &dshare->shmptr->hw.period_time);
	if (err < 0)
		return err;
	if (dshare->max_periods < 0) {
		err = hw_param_interval_refine_one(params, SND_PCM_HW_PARAM_BUFFER_SIZE,
						   &dshare->shmptr->hw.buffer_size);
		if (err < 0)
			return err;
		err = hw_param_interval_refine_one(params, SND_PCM_HW_PARAM_BUFFER_TIME,
						   &dshare->shmptr->hw.buffer_time);
		if (err < 0)
			return err;
	} else if (params->rmask & ((1<<SND_PCM_HW_PARAM_PERIODS)|
				    (1<<SND_PCM_HW_PARAM_BUFFER_BYTES)|
				    (1<<SND_PCM_HW_PARAM_BUFFER_SIZE)|
				    (1<<SND_PCM_HW_PARAM_BUFFER_TIME))) {
		int changed;
		unsigned int max_periods = dshare->max_periods;
		if (max_periods < 2)
			max_periods = dshare->slave_buffer_size / dshare->slave_period_size;
		do {
			changed = 0;
			err = hw_param_interval_refine_minmax(params, SND_PCM_HW_PARAM_PERIODS,
							      2, max_periods);
			if (err < 0)
				return err;
			changed |= err;
			err = snd_pcm_hw_refine_soft(pcm, params);
			if (err < 0)
				return err;
			changed |= err;
		} while (changed);
	}
	params->info = dshare->shmptr->s.info;
#ifdef REFINE_DEBUG
	snd_output_puts(log, "DMIX REFINE (end):\n");
	snd_pcm_hw_params_dump(params, log);
	snd_output_close(log);
#endif
	return 0;
}

int snd_pcm_direct_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	params->info = dmix->shmptr->s.info;
	params->rate_num = dmix->shmptr->s.rate;
	params->rate_den = 1;
	params->fifo_size = 0;
	params->msbits = dmix->shmptr->s.msbits;
	return 0;
}

int snd_pcm_direct_hw_free(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	/* values are cached in the pcm structure */
	return 0;
}

int snd_pcm_direct_sw_params(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_sw_params_t * params ATTRIBUTE_UNUSED)
{
	/* values are cached in the pcm structure */
	return 0;
}

int snd_pcm_direct_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t * info)
{
        return snd_pcm_channel_info_shm(pcm, info, -1);
}

int snd_pcm_direct_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}
        
int snd_pcm_direct_munmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_pcm_direct_resume(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	int err;
	
	snd_pcm_direct_semaphore_down(dmix, DIRECT_IPC_SEM_CLIENT);
	err = snd_pcm_resume(dmix->spcm);
	if (err == -ENOSYS) {
		snd_pcm_prepare(dmix->spcm);
		snd_pcm_start(dmix->spcm);
		err = 0;
	}
	snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
	return err;
}

#define COPY_SLAVE(field) (dmix->shmptr->s.field = spcm->field)

/* copy the slave setting */
static void save_slave_setting(snd_pcm_direct_t *dmix, snd_pcm_t *spcm)
{
	spcm->info &= ~SND_PCM_INFO_PAUSE;

	COPY_SLAVE(access);
	COPY_SLAVE(format);
	COPY_SLAVE(subformat);
	COPY_SLAVE(channels);
	COPY_SLAVE(rate);
	COPY_SLAVE(period_size);
	COPY_SLAVE(period_time);
	COPY_SLAVE(periods);
	COPY_SLAVE(tstamp_mode);
	COPY_SLAVE(period_step);
	COPY_SLAVE(avail_min);
	COPY_SLAVE(start_threshold);
	COPY_SLAVE(stop_threshold);
	COPY_SLAVE(silence_threshold);
	COPY_SLAVE(silence_size);
	COPY_SLAVE(boundary);
	COPY_SLAVE(info);
	COPY_SLAVE(msbits);
	COPY_SLAVE(rate_num);
	COPY_SLAVE(rate_den);
	COPY_SLAVE(hw_flags);
	COPY_SLAVE(fifo_size);
	COPY_SLAVE(buffer_size);
	COPY_SLAVE(buffer_time);
	COPY_SLAVE(sample_bits);
	COPY_SLAVE(frame_bits);
}

#undef COPY_SLAVE

/*
 * this function initializes hardware and starts playback operation with
 * no stop threshold (it operates all time without xrun checking)
 * also, the driver silences the unused ring buffer areas for us
 */
int snd_pcm_direct_initialize_slave(snd_pcm_direct_t *dmix, snd_pcm_t *spcm, struct slave_params *params)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	int ret, buffer_is_not_initialized;
	snd_pcm_uframes_t boundary;
	struct pollfd fd;
	int loops = 10;

	snd_pcm_hw_params_alloca(&hw_params);
	snd_pcm_sw_params_alloca(&sw_params);

      __again:
      	if (loops-- <= 0) {
      		SNDERR("unable to find a valid configuration for slave");
      		return -EINVAL;
      	}
	ret = snd_pcm_hw_params_any(spcm, hw_params);
	if (ret < 0) {
		SNDERR("snd_pcm_hw_params_any failed");
		return ret;
	}
	ret = snd_pcm_hw_params_set_access(spcm, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	if (ret < 0) {
		ret = snd_pcm_hw_params_set_access(spcm, hw_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		if (ret < 0) {
			SNDERR("slave plugin does not support mmap interleaved or mmap noninterleaved access");
			return ret;
		}
	}
	if (params->format == SND_PCM_FORMAT_UNKNOWN)
		ret = -EINVAL;
	else
		ret = snd_pcm_hw_params_set_format(spcm, hw_params,
						   params->format);
	if (ret < 0) {
		static const snd_pcm_format_t dmix_formats[] = {
			SND_PCM_FORMAT_S32,
			SND_PCM_FORMAT_S32 ^ SND_PCM_FORMAT_S32_LE ^ SND_PCM_FORMAT_S32_BE,
			SND_PCM_FORMAT_S16,
			SND_PCM_FORMAT_S16 ^ SND_PCM_FORMAT_S16_LE ^ SND_PCM_FORMAT_S16_BE,
			SND_PCM_FORMAT_S24_LE,
			SND_PCM_FORMAT_S24_3LE,
			SND_PCM_FORMAT_U8,
		};
		snd_pcm_format_t format;
		unsigned int i;

		for (i = 0; i < sizeof dmix_formats / sizeof dmix_formats[0]; ++i) {
			format = dmix_formats[i];
			ret = snd_pcm_hw_params_set_format(spcm, hw_params, format);
			if (ret >= 0)
				break;
		}
		if (ret < 0 && dmix->type != SND_PCM_TYPE_DMIX) {
			/* TODO: try to choose a good format */
			ret = INTERNAL(snd_pcm_hw_params_set_format_first)(spcm, hw_params, &format);
		}
		if (ret < 0) {
			SNDERR("requested or auto-format is not available");
			return ret;
		}
		params->format = format;
	}
	ret = INTERNAL(snd_pcm_hw_params_set_channels_near)(spcm, hw_params, (unsigned int *)&params->channels);
	if (ret < 0) {
		SNDERR("requested count of channels is not available");
		return ret;
	}
	ret = INTERNAL(snd_pcm_hw_params_set_rate_near)(spcm, hw_params, (unsigned int *)&params->rate, 0);
	if (ret < 0) {
		SNDERR("requested rate is not available");
		return ret;
	}

	buffer_is_not_initialized = 0;
	if (params->buffer_time > 0) {
		ret = INTERNAL(snd_pcm_hw_params_set_buffer_time_near)(spcm, hw_params, (unsigned int *)&params->buffer_time, 0);
		if (ret < 0) {
			SNDERR("unable to set buffer time");
			return ret;
		}
	} else if (params->buffer_size > 0) {
		ret = INTERNAL(snd_pcm_hw_params_set_buffer_size_near)(spcm, hw_params, (snd_pcm_uframes_t *)&params->buffer_size);
		if (ret < 0) {
			SNDERR("unable to set buffer size");
			return ret;
		}
	} else {
		buffer_is_not_initialized = 1;
	}

	if (params->period_time > 0) {
		ret = INTERNAL(snd_pcm_hw_params_set_period_time_near)(spcm, hw_params, (unsigned int *)&params->period_time, 0);
		if (ret < 0) {
			SNDERR("unable to set period_time");
			return ret;
		}
	} else if (params->period_size > 0) {
		ret = INTERNAL(snd_pcm_hw_params_set_period_size_near)(spcm, hw_params, (snd_pcm_uframes_t *)&params->period_size, 0);
		if (ret < 0) {
			SNDERR("unable to set period_size");
			return ret;
		}
	}		
	
	if (buffer_is_not_initialized && params->periods > 0) {
		unsigned int periods = params->periods;
		ret = INTERNAL(snd_pcm_hw_params_set_periods_near)(spcm, hw_params, &params->periods, 0);
		if (ret < 0) {
			SNDERR("unable to set requested periods");
			return ret;
		}
		if (params->periods == 1) {
			params->periods = periods;
			if (params->period_time > 0) {
				params->period_time /= 2;
				goto __again;
			} else if (params->period_size > 0) {
				params->period_size /= 2;
				goto __again;
			}
			SNDERR("unable to use stream with periods == 1");
			return ret;
		}
	}
	
	ret = snd_pcm_hw_params(spcm, hw_params);
	if (ret < 0) {
		SNDERR("unable to install hw params");
		return ret;
	}

	/* store some hw_params values to shared info */
	dmix->shmptr->hw.format = snd_mask_value(hw_param_mask(hw_params, SND_PCM_HW_PARAM_FORMAT));
	dmix->shmptr->hw.rate = *hw_param_interval(hw_params, SND_PCM_HW_PARAM_RATE);
	dmix->shmptr->hw.buffer_size = *hw_param_interval(hw_params, SND_PCM_HW_PARAM_BUFFER_SIZE);
	dmix->shmptr->hw.buffer_time = *hw_param_interval(hw_params, SND_PCM_HW_PARAM_BUFFER_TIME);
	dmix->shmptr->hw.period_size = *hw_param_interval(hw_params, SND_PCM_HW_PARAM_PERIOD_SIZE);
	dmix->shmptr->hw.period_time = *hw_param_interval(hw_params, SND_PCM_HW_PARAM_PERIOD_TIME);
	dmix->shmptr->hw.periods = *hw_param_interval(hw_params, SND_PCM_HW_PARAM_PERIODS);


	ret = snd_pcm_sw_params_current(spcm, sw_params);
	if (ret < 0) {
		SNDERR("unable to get current sw_params");
		return ret;
	}

	ret = snd_pcm_sw_params_get_boundary(sw_params, &boundary);
	if (ret < 0) {
		SNDERR("unable to get boundary");
		return ret;
	}
	ret = snd_pcm_sw_params_set_stop_threshold(spcm, sw_params, boundary);
	if (ret < 0) {
		SNDERR("unable to set stop threshold");
		return ret;
	}

	/* set timestamp mode to MMAP
	 * the slave timestamp is copied appropriately in dsnoop/dmix/dshare
	 * based on the tstamp_mode of each client
	 */
	ret = snd_pcm_sw_params_set_tstamp_mode(spcm, sw_params,
						SND_PCM_TSTAMP_ENABLE);
	if (ret < 0) {
		SNDERR("unable to tstamp mode MMAP");
		return ret;
	}

	if (dmix->type != SND_PCM_TYPE_DMIX)
		goto __skip_silencing;

	ret = snd_pcm_sw_params_set_silence_threshold(spcm, sw_params, 0);
	if (ret < 0) {
		SNDERR("unable to set silence threshold");
		return ret;
	}
	ret = snd_pcm_sw_params_set_silence_size(spcm, sw_params, boundary);
	if (ret < 0) {
		SNDERR("unable to set silence threshold (please upgrade to 0.9.0rc8+ driver)");
		return ret;
	}

      __skip_silencing:

	ret = snd_pcm_sw_params(spcm, sw_params);
	if (ret < 0) {
		SNDERR("unable to install sw params (please upgrade to 0.9.0rc8+ driver)");
		return ret;
	}

	if (dmix->type == SND_PCM_TYPE_DSHARE) {
		const snd_pcm_channel_area_t *dst_areas;
		dst_areas = snd_pcm_mmap_areas(spcm);
		snd_pcm_areas_silence(dst_areas, 0, spcm->channels, spcm->buffer_size, spcm->format);
	}
	
	ret = snd_pcm_start(spcm);
	if (ret < 0) {
		SNDERR("unable to start PCM stream");
		return ret;
	}

	if (snd_pcm_poll_descriptors_count(spcm) != 1) {
		SNDERR("unable to use hardware pcm with fd more than one!!!");
		return ret;
	}
	snd_pcm_poll_descriptors(spcm, &fd, 1);
	dmix->hw_fd = fd.fd;
	
	save_slave_setting(dmix, spcm);

	/* Currently, we assume that each dmix client has the same
	 * hw_params setting.
	 * If the arbitrary hw_parmas is supported in future,
	 * boundary has to be taken from the slave config but
	 * recalculated for the native boundary size (for 32bit
	 * emulation on 64bit arch).
	 */
	dmix->slave_buffer_size = spcm->buffer_size;
	dmix->slave_period_size = spcm->period_size;
	dmix->slave_boundary = spcm->boundary;

	spcm->donot_close = 1;

	{
		int ver = 0;
		ioctl(spcm->poll_fd, SNDRV_PCM_IOCTL_PVERSION, &ver);
		if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 8))
			dmix->shmptr->use_server = 1;
	}

	return 0;
}

/*
 * the trick is used here; we cannot use effectively the hardware handle because
 * we cannot drive multiple accesses to appl_ptr; so we use slave timer of given
 * PCM hardware handle; it's not this easy and cheap?
 */
int snd_pcm_direct_initialize_poll_fd(snd_pcm_direct_t *dmix)
{
	int ret;
	snd_pcm_info_t *info;
	char name[128];
	int capture = dmix->type == SND_PCM_TYPE_DSNOOP ? 1 : 0;

	dmix->tread = 1;
	dmix->timer_need_poll = 0;
	snd_pcm_info_alloca(&info);
	ret = snd_pcm_info(dmix->spcm, info);
	if (ret < 0) {
		SNDERR("unable to info for slave pcm");
		return ret;
	}
	sprintf(name, "hw:CLASS=%i,SCLASS=0,CARD=%i,DEV=%i,SUBDEV=%i",
				(int)SND_TIMER_CLASS_PCM, 
				snd_pcm_info_get_card(info),
				snd_pcm_info_get_device(info),
				snd_pcm_info_get_subdevice(info) * 2 + capture);
	ret = snd_timer_open(&dmix->timer, name, SND_TIMER_OPEN_NONBLOCK | SND_TIMER_OPEN_TREAD);
	if (ret < 0) {
		dmix->tread = 0;
		ret = snd_timer_open(&dmix->timer, name, SND_TIMER_OPEN_NONBLOCK);
		if (ret < 0) {
			SNDERR("unable to open timer '%s'", name);
			return ret;
		}
	}

	if (snd_timer_poll_descriptors_count(dmix->timer) != 1) {
		SNDERR("unable to use timer '%s' with more than one fd!", name);
		return ret;
	}
	snd_timer_poll_descriptors(dmix->timer, &dmix->timer_fd, 1);
	dmix->poll_fd = dmix->timer_fd.fd;

	dmix->timer_events = (1<<SND_TIMER_EVENT_MSUSPEND) |
			     (1<<SND_TIMER_EVENT_MRESUME) |
			     (1<<SND_TIMER_EVENT_STOP);

	/*
	 * Some hacks for older kernel drivers
	 */
	{
		int ver = 0;
		ioctl(dmix->poll_fd, SNDRV_TIMER_IOCTL_PVERSION, &ver);
		/* In older versions, check via poll before read() is needed
		 * because of the confliction between TIMER_START and
		 * FIONBIO ioctls.
		 */
		if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 4))
			dmix->timer_need_poll = 1;
		/*
		 * In older versions, timer uses pause events instead
		 * suspend/resume events.
		 */
		if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 5)) {
			dmix->timer_events &= ~((1<<SND_TIMER_EVENT_MSUSPEND) |
						(1<<SND_TIMER_EVENT_MRESUME));
			dmix->timer_events |= (1<<SND_TIMER_EVENT_MPAUSE) |
					      (1<<SND_TIMER_EVENT_MCONTINUE);
		}
		/* In older versions, use SND_TIMER_EVENT_START too.
		 */
		if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 6))
			dmix->timer_events |= 1<<SND_TIMER_EVENT_START;
	}
	return 0;
}

static snd_pcm_uframes_t recalc_boundary_size(unsigned long long bsize, snd_pcm_uframes_t buffer_size)
{
	if (bsize > LONG_MAX) {
		bsize = buffer_size;
		while (bsize * 2 <= LONG_MAX - buffer_size)
			bsize *= 2;
	}
	return (snd_pcm_uframes_t)bsize;
}

#define COPY_SLAVE(field) (spcm->field = dmix->shmptr->s.field)

/* copy the slave setting */
static void copy_slave_setting(snd_pcm_direct_t *dmix, snd_pcm_t *spcm)
{
	COPY_SLAVE(access);
	COPY_SLAVE(format);
	COPY_SLAVE(subformat);
	COPY_SLAVE(channels);
	COPY_SLAVE(rate);
	COPY_SLAVE(period_size);
	COPY_SLAVE(period_time);
	COPY_SLAVE(periods);
	COPY_SLAVE(tstamp_mode);
	COPY_SLAVE(period_step);
	COPY_SLAVE(avail_min);
	COPY_SLAVE(start_threshold);
	COPY_SLAVE(stop_threshold);
	COPY_SLAVE(silence_threshold);
	COPY_SLAVE(silence_size);
	COPY_SLAVE(boundary);
	COPY_SLAVE(info);
	COPY_SLAVE(msbits);
	COPY_SLAVE(rate_num);
	COPY_SLAVE(rate_den);
	COPY_SLAVE(hw_flags);
	COPY_SLAVE(fifo_size);
	COPY_SLAVE(buffer_size);
	COPY_SLAVE(buffer_time);
	COPY_SLAVE(sample_bits);
	COPY_SLAVE(frame_bits);

	spcm->info &= ~SND_PCM_INFO_PAUSE;
	spcm->boundary = recalc_boundary_size(dmix->shmptr->s.boundary, spcm->buffer_size);
}

#undef COPY_SLAVE


/*
 * open a slave PCM as secondary client (dup'ed fd)
 */
int snd_pcm_direct_open_secondary_client(snd_pcm_t **spcmp, snd_pcm_direct_t *dmix, const char *client_name)
{
	int ret;
	snd_pcm_t *spcm;

	ret = snd_pcm_hw_open_fd(spcmp, client_name, dmix->hw_fd, 0, 0);
	if (ret < 0) {
		SNDERR("unable to open hardware");
		return ret;
	}
		
	spcm = *spcmp;
	spcm->donot_close = 1;
	spcm->setup = 1;

	copy_slave_setting(dmix, spcm);

	/* Use the slave setting as SPCM, so far */
	dmix->slave_buffer_size = spcm->buffer_size;
	dmix->slave_period_size = dmix->shmptr->s.period_size;
	dmix->slave_boundary = spcm->boundary;

	ret = snd_pcm_mmap(spcm);
	if (ret < 0) {
		SNDERR("unable to mmap channels");
		return ret;
	}
	return 0;
}

/*
 * open a slave PCM as secondary client (dup'ed fd)
 */
int snd_pcm_direct_initialize_secondary_slave(snd_pcm_direct_t *dmix,
					      snd_pcm_t *spcm,
					      struct slave_params *params ATTRIBUTE_UNUSED)
{
	int ret;

	spcm->donot_close = 1;
	spcm->setup = 1;

	copy_slave_setting(dmix, spcm);

	/* Use the slave setting as SPCM, so far */
	dmix->slave_buffer_size = spcm->buffer_size;
	dmix->slave_period_size = dmix->shmptr->s.period_size;
	dmix->slave_boundary = spcm->boundary;

	ret = snd_pcm_mmap(spcm);
	if (ret < 0) {
		SNDERR("unable to mmap channels");
		return ret;
	}
	return 0;
}

int snd_pcm_direct_set_timer_params(snd_pcm_direct_t *dmix)
{
	snd_timer_params_t *params;
	unsigned int filter;
	int ret;

	snd_timer_params_alloca(&params);
	snd_timer_params_set_auto_start(params, 1);
	if (dmix->type != SND_PCM_TYPE_DSNOOP)
		snd_timer_params_set_early_event(params, 1);
	snd_timer_params_set_ticks(params, 1);
	if (dmix->tread) {
		filter = (1<<SND_TIMER_EVENT_TICK) |
			 dmix->timer_events;
		snd_timer_params_set_filter(params, filter);
	}
	ret = snd_timer_params(dmix->timer, params);
	if (ret < 0) {
		SNDERR("unable to set timer parameters");
                return ret;
	}
	return 0;
}

/*
 *  ring buffer operation
 */
int snd_pcm_direct_check_interleave(snd_pcm_direct_t *dmix, snd_pcm_t *pcm)
{
	unsigned int chn, channels;
	int bits, interleaved = 1;
	const snd_pcm_channel_area_t *dst_areas;
	const snd_pcm_channel_area_t *src_areas;

	bits = snd_pcm_format_physical_width(pcm->format);
	if ((bits % 8) != 0)
		interleaved = 0;
	channels = dmix->channels;
	dst_areas = snd_pcm_mmap_areas(dmix->spcm);
	src_areas = snd_pcm_mmap_areas(pcm);
	for (chn = 1; chn < channels; chn++) {
		if (dst_areas[chn-1].addr != dst_areas[chn].addr) {
			interleaved = 0;
			break;
		}
		if (src_areas[chn-1].addr != src_areas[chn].addr) {
			interleaved = 0;
			break;
		}
	}
	for (chn = 0; chn < channels; chn++) {
		if (dmix->bindings && dmix->bindings[chn] != chn) {
			interleaved = 0;
			break;
		}
		if (dst_areas[chn].first != chn * bits ||
		    dst_areas[chn].step != channels * bits) {
			interleaved = 0;
			break;
		}
		if (src_areas[chn].first != chn * bits ||
		    src_areas[chn].step != channels * bits) {
			interleaved = 0;
			break;
		}
	}
	return dmix->interleaved = interleaved;
}

/*
 * parse the channel map
 * id == client channel
 * value == slave's channel
 */
int snd_pcm_direct_parse_bindings(snd_pcm_direct_t *dmix,
				  struct slave_params *params,
				  snd_config_t *cfg)
{
	snd_config_iterator_t i, next;
	unsigned int chn, chn1, count = 0;
	unsigned int *bindings;
	int err;

	dmix->channels = UINT_MAX;
	if (cfg == NULL)
		return 0;
	if (snd_config_get_type(cfg) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("invalid type for bindings");
		return -EINVAL;
	}
	snd_config_for_each(i, next, cfg) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		long cchannel;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		err = safe_strtol(id, &cchannel);
		if (err < 0 || cchannel < 0) {
			SNDERR("invalid client channel in binding: %s\n", id);
			return -EINVAL;
		}
		if ((unsigned)cchannel >= count)
			count = cchannel + 1;
	}
	if (count == 0)
		return 0;
	if (count > 1024) {
		SNDERR("client channel out of range");
		return -EINVAL;
	}
	bindings = malloc(count * sizeof(unsigned int));
	if (bindings == NULL)
		return -ENOMEM;
	for (chn = 0; chn < count; chn++)
		bindings[chn] = UINT_MAX;		/* don't route */
	snd_config_for_each(i, next, cfg) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		long cchannel, schannel;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		safe_strtol(id, &cchannel);
		if (snd_config_get_integer(n, &schannel) < 0) {
			SNDERR("unable to get slave channel (should be integer type) in binding: %s\n", id);
			free(bindings);
			return -EINVAL;
		}
		if (schannel < 0 || schannel >= params->channels) {
			SNDERR("invalid slave channel number %ld in binding to %ld",
			       schannel, cchannel);
			free(bindings);
			return -EINVAL;
		}
		bindings[cchannel] = schannel;
	}
	if (dmix->type == SND_PCM_TYPE_DSNOOP ||
	    ! dmix->bindings)
		goto __skip_same_dst;
	for (chn = 0; chn < count; chn++) {
		for (chn1 = 0; chn1 < count; chn1++) {
			if (chn == chn1)
				continue;
			if (bindings[chn] == dmix->bindings[chn1]) {
				SNDERR("unable to route channels %d,%d to same destination %d", chn, chn1, bindings[chn]);
				free(bindings);
				return -EINVAL;
			}
		}
	}
      __skip_same_dst:
	dmix->bindings = bindings;
	dmix->channels = count;
	return 0;
}

/*
 * parse slave config and calculate the ipc_key offset
 */

static int _snd_pcm_direct_get_slave_ipc_offset(snd_config_t *root,
						snd_config_t *sconf,
						int direction,
						int hop)
{
	snd_config_iterator_t i, next;
	snd_config_t *pcm_conf;
	int err;
	long card = 0, device = 0, subdevice = 0;
	const char *str;

	if (snd_config_get_string(sconf, &str) >= 0) {
		if (hop > SND_CONF_MAX_HOPS) {
			SNDERR("Too many definition levels (looped?)");
			return -EINVAL;
		}
		err = snd_config_search_definition(root, "pcm", str, &pcm_conf);
		if (err < 0) {
			SNDERR("Unknown slave PCM %s", str);
			return err;
		}
		err = _snd_pcm_direct_get_slave_ipc_offset(root, pcm_conf,
							   direction,
							   hop + 1);
		snd_config_delete(pcm_conf);
		return err;
	}


	if (snd_config_search(sconf, "slave", &pcm_conf) >= 0 &&
	    (snd_config_search(pcm_conf, "pcm", &pcm_conf) >= 0 ||
	    (snd_config_get_string(pcm_conf, &str) >= 0 &&
	    snd_config_search_definition(root, "pcm_slave", str, &pcm_conf) >= 0 &&
	    snd_config_search(pcm_conf, "pcm", &pcm_conf) >= 0)))
		return _snd_pcm_direct_get_slave_ipc_offset(root, pcm_conf,
							    direction,
							    hop + 1);

	snd_config_for_each(i, next, sconf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id, *str;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "type") == 0) {
			err = snd_config_get_string(n, &str);
			if (err < 0) {
				SNDERR("Invalid value for PCM type definition\n");
				return -EINVAL;
			}
			if (strcmp(str, "hw")) {
				SNDERR("Invalid type '%s' for slave PCM\n", str);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "card") == 0) {
			err = snd_config_get_integer(n, &card);
			if (err < 0) {
				err = snd_config_get_string(n, &str);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					return -EINVAL;
				}
				card = snd_card_get_index(str);
				if (card < 0) {
					SNDERR("Invalid value for %s", id);
					return card;
				}
			}
			continue;
		}
		if (strcmp(id, "device") == 0) {
			err = snd_config_get_integer(n, &device);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			continue;
		}
		if (strcmp(id, "subdevice") == 0) {
			err = snd_config_get_integer(n, &subdevice);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			continue;
		}
	}
	if (card < 0)
		card = 0;
	if (device < 0)
		device = 0;
	if (subdevice < 0)
		subdevice = 0;
	return (direction << 1) + (device << 2) + (subdevice << 8) + (card << 12);
}

static int snd_pcm_direct_get_slave_ipc_offset(snd_config_t *root,
					snd_config_t *sconf,
					int direction)
{
	return _snd_pcm_direct_get_slave_ipc_offset(root, sconf, direction, 0);
}

int snd_pcm_direct_parse_open_conf(snd_config_t *root, snd_config_t *conf,
				   int stream, struct snd_pcm_direct_open_conf *rec)
{
	snd_config_iterator_t i, next;
	int ipc_key_add_uid = 0;
	snd_config_t *n;
	int err;

	rec->slave = NULL;
	rec->bindings = NULL;
	rec->ipc_key = 0;
	rec->ipc_perm = 0600;
	rec->ipc_gid = -1;
	rec->slowptr = 1;
	rec->max_periods = 0;

	/* read defaults */
	if (snd_config_search(root, "defaults.pcm.dmix_max_periods", &n) >= 0) {
		long val;
		err = snd_config_get_integer(n, &val);
		if (err >= 0)
			rec->max_periods = val;
	}

	snd_config_for_each(i, next, conf) {
		const char *id;
		n = snd_config_iterator_entry(i);
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "ipc_key") == 0) {
			long key;
			err = snd_config_get_integer(n, &key);
			if (err < 0) {
				SNDERR("The field ipc_key must be an integer type");

				return err;
			}
			rec->ipc_key = key;
			continue;
		}
		if (strcmp(id, "ipc_perm") == 0) {
			long perm;
			err = snd_config_get_integer(n, &perm);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			if ((perm & ~0777) != 0) {
				SNDERR("The field ipc_perm must be a valid file permission");
				return -EINVAL;
			}
			rec->ipc_perm = perm;
			continue;
		}
		if (strcmp(id, "ipc_gid") == 0) {
			char *group;
			char *endp;
			err = snd_config_get_ascii(n, &group);
			if (err < 0) {
				SNDERR("The field ipc_gid must be a valid group");
				return err;
			}
			if (! *group) {
				rec->ipc_gid = -1;
				free(group);
				continue;
			}
			if (isdigit(*group) == 0) {
				struct group *grp = getgrnam(group);
				if (grp == NULL) {
					SNDERR("The field ipc_gid must be a valid group (create group %s)", group);
					free(group);
					return -EINVAL;
				}
				rec->ipc_gid = grp->gr_gid;
			} else {
				rec->ipc_gid = strtol(group, &endp, 10);
			}
			free(group);
			continue;
		}
		if (strcmp(id, "ipc_key_add_uid") == 0) {
			if ((err = snd_config_get_bool(n)) < 0) {
				SNDERR("The field ipc_key_add_uid must be a boolean type");
				return err;
			}
			ipc_key_add_uid = err;
			continue;
		}
		if (strcmp(id, "slave") == 0) {
			rec->slave = n;
			continue;
		}
		if (strcmp(id, "bindings") == 0) {
			rec->bindings = n;
			continue;
		}
		if (strcmp(id, "slowptr") == 0) {
			err = snd_config_get_bool(n);
			if (err < 0)
				return err;
			rec->slowptr = err;
			continue;
		}
		if (strcmp(id, "max_periods") == 0) {
			long val;
			err = snd_config_get_integer(n, &val);
			if (err < 0)
				return err;
			rec->max_periods = val;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!rec->slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	if (!rec->ipc_key) {
		SNDERR("Unique IPC key is not defined");
		return -EINVAL;
	}
	if (ipc_key_add_uid)
		rec->ipc_key += getuid();
	err = snd_pcm_direct_get_slave_ipc_offset(root, conf, stream);
	if (err < 0)
		return err;
	rec->ipc_key += err;

	return 0;
}
