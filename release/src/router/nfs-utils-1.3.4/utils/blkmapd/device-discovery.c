/*
 * device-discovery.c: main function, discovering device and processing
 * pipe request from kernel.
 *
 * Copyright (c) 2010 EMC Corporation, Haiying Tang <Tang_Haiying@emc.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <linux/kdev_t.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/sg.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <libdevmapper.h>

#include "device-discovery.h"
#include "xcommon.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUFSIZE (1024 * EVENT_SIZE)

#define BL_PIPE_FILE	"/var/lib/nfs/rpc_pipefs/nfs/blocklayout"
#define NFSPIPE_DIR	"/var/lib/nfs/rpc_pipefs/nfs"
#define RPCPIPE_DIR	"/var/lib/nfs/rpc_pipefs"
#define PID_FILE	"/var/run/blkmapd.pid"

struct bl_disk *visible_disk_list;
int    bl_watch_fd, bl_pipe_fd, nfs_pipedir_wfd, rpc_pipedir_wfd;
int    pidfd = -1;

struct bl_disk_path *bl_get_path(const char *filepath,
				 struct bl_disk_path *paths)
{
	struct bl_disk_path *tmp = paths;

	while (tmp) {
		if (!strcmp(tmp->full_path, filepath))
			break;
		tmp = tmp->next;
	}
	return tmp;
}

/*
 * For multipath devices, devices state could be PASSIVE/ACTIVE/PSEUDO,
 * where PSEUDO > ACTIVE > PASSIVE. Device with highest state is used to
 * create pseudo device. So if state is higher, the device path needs to
 * be updated.
 * If device-mapper multipath support is a must, pseudo devices should
 * exist for each multipath device. If not, active device path will be
 * chosen for device creation.
 */
int bl_update_path(enum bl_path_state_e state, struct bl_disk *disk)
{
	struct bl_disk_path *valid_path = disk->valid_path;

	if (valid_path && valid_path->state >= state)
		return 0;
	return 1;
}

void bl_release_disk(void)
{
	struct bl_disk *disk;
	struct bl_disk_path *path = NULL;

	while (visible_disk_list) {
		disk = visible_disk_list;
		path = disk->paths;
		while (path) {
			disk->paths = path->next;
			free(path->full_path);
			free(path);
			path = disk->paths;
		}
		if (disk->serial)
			free(disk->serial);
		visible_disk_list = disk->next;
		free(disk);
	}
}

void bl_add_disk(char *filepath)
{
	struct bl_disk *disk = NULL;
	int fd = 0;
	struct stat sb;
	off_t size = 0;
	struct bl_serial *serial = NULL;
	enum bl_path_state_e ap_state;
	struct bl_disk_path *diskpath = NULL, *path = NULL;
	dev_t dev;

	fd = open(filepath, O_RDONLY | O_LARGEFILE);
	if (fd < 0)
		return;

	if (fstat(fd, &sb)) {
		close(fd);
		return;
	}

	if (!sb.st_size)
		ioctl(fd, BLKGETSIZE, &size);
	else
		size = sb.st_size;

	if (!size) {
		close(fd);
		return;
	}

	dev = sb.st_rdev;
	serial = bldev_read_serial(fd, filepath);
	if (!serial) {
		BL_LOG_ERR("%s: no serial found for %s\n",
				 __func__, filepath);
		ap_state = BL_PATH_STATE_PASSIVE;
	} else if (dm_is_dm_major(major(dev)))
		ap_state = BL_PATH_STATE_PSEUDO;
	else
		ap_state = bldev_read_ap_state(fd);
	close(fd);

	for (disk = visible_disk_list; disk != NULL; disk = disk->next) {
		/* Already scanned or a partition?
		 * XXX: if released each time, maybe not need to compare
		 */
		if ((serial->len == disk->serial->len) &&
		    !memcmp(serial->data, disk->serial->data, serial->len)) {
			diskpath = bl_get_path(filepath, disk->paths);
			break;
		}
	}

	if (disk && diskpath)
		return;

	/* add path */
	path = malloc(sizeof(struct bl_disk_path));
	if (!path) {
		BL_LOG_ERR("%s: Out of memory!\n", __func__);
		goto out_err;
	}
	path->next = NULL;
	path->state = ap_state;
	path->full_path = strdup(filepath);
	if (!path->full_path)
		goto out_err;

	if (!disk) {		/* add disk */
		disk = malloc(sizeof(struct bl_disk));
		if (!disk) {
			BL_LOG_ERR("%s: Out of memory!\n", __func__);
			goto out_err;
		}
		disk->next = visible_disk_list;
		disk->dev = dev;
		disk->size = size;
		disk->serial = serial;
		disk->valid_path = path;
		disk->paths = path;
		visible_disk_list = disk;
	} else {
		path->next = disk->paths;
		disk->paths = path;
		/* check whether we need to update disk info */
		if (bl_update_path(path->state, disk)) {
			disk->dev = dev;
			disk->size = size;
			disk->valid_path = path;
		}
	}
	return;

 out_err:
	if (path) {
		if (path->full_path)
			free(path->full_path);
		free(path);
	}
	return;
}

int bl_discover_devices(void)
{
	FILE *f;
	int n;
	char buf[PATH_MAX], devname[PATH_MAX], fulldevname[PATH_MAX];

	/* release previous list */
	bl_release_disk();

	/* scan all block devices */
	f = fopen("/proc/partitions", "r");
	if (f == NULL)
		return 0;

	while (1) {
		if (fgets(buf, sizeof buf, f) == NULL)
			break;
		n = sscanf(buf, "%*d %*d %*d %31s", devname);
		if (n != 1)
			continue;
		snprintf(fulldevname, sizeof fulldevname, "/sys/block/%s",
			 devname);
		if (access(fulldevname, F_OK) < 0)
			continue;
		snprintf(fulldevname, sizeof fulldevname, "/dev/%s", devname);
		bl_add_disk(fulldevname);
	}

	fclose(f);

	return 0;
}

/* process kernel request
 * return 0: request processed, and no more request waiting;
 * return 1: request processed, and more requests waiting;
 * return < 0: error
 */
static int bl_disk_inquiry_process(int fd)
{
	int ret = 0;
	struct bl_pipemsg_hdr head;
	char *buf = NULL;
	uint32_t major, minor;
	uint16_t buflen;
	struct bl_dev_msg reply;

	/* read request */
	if (atomicio(read, fd, &head, sizeof(head)) != sizeof(head)) {
		/* Note that an error in this or the next read is pretty
		 * catastrophic, as there is no good way to resync into
		 * the pipe's stream.
		 */
		BL_LOG_ERR("Read pipefs head error!\n");
		ret = -EIO;
		goto out;
	}

	buflen = head.totallen;
	buf = malloc(buflen);
	if (!buf) {
		BL_LOG_ERR("%s: Out of memory!\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	if (atomicio(read, fd, buf, buflen) != buflen) {
		BL_LOG_ERR("Read pipefs content error!\n");
		ret = -EIO;
		goto out;
	}

	reply.status = BL_DEVICE_REQUEST_PROC;

	switch (head.type) {
	case BL_DEVICE_MOUNT:
		/*
		 * It shouldn't be necessary to discover devices here, since
		 * process_deviceinfo() will re-discover if it can't find
		 * the devices it needs.  But in the case of multipath
		 * devices (ones that appear more than once, for example an
		 * active and a standby LUN), this will re-order them in the
		 * correct priority.
		 */
		bl_discover_devices();
		if (!process_deviceinfo(buf, buflen, &major, &minor)) {
			reply.status = BL_DEVICE_REQUEST_ERR;
			break;
		}
		reply.major = major;
		reply.minor = minor;
		break;
	case BL_DEVICE_UMOUNT:
		if (!dm_device_remove_all((uint64_t *) buf))
			reply.status = BL_DEVICE_REQUEST_ERR;
		break;
	default:
		reply.status = BL_DEVICE_REQUEST_ERR;
		break;
	}

	/* write to pipefs */
	if (atomicio((void *)write, fd, &reply, sizeof(reply))
	    != sizeof(reply)) {
		BL_LOG_ERR("Write pipefs error!\n");
		ret = -EIO;
	}

 out:
	if (buf)
		free(buf);
	return ret;
}

static void bl_watch_dir(const char* dir, int *wd)
{
	*wd = inotify_add_watch(bl_watch_fd, dir, IN_CREATE|IN_DELETE);
	if (*wd < 0)
		BL_LOG_ERR("failed to watch %s: %s\n", dir, strerror(errno));
}

static void bl_rpcpipe_cb(void)
{
	int rc, curr_byte = 0;
	char eventArr[EVENT_BUFSIZE];
	struct inotify_event *event;

	rc = read(bl_watch_fd, &eventArr, EVENT_BUFSIZE);
	if (rc < 0)
		BL_LOG_ERR("read event fail: %s", strerror(errno));

	while (rc > curr_byte) {
		event = (struct inotify_event *)&eventArr[curr_byte];
		curr_byte += EVENT_SIZE + event->len;
		if (event->wd == rpc_pipedir_wfd) {
			if (strncmp(event->name, "nfs", 3))
				continue;
			if (event->mask & IN_CREATE) {
				BL_LOG_WARNING("nfs pipe dir created\n");
				bl_watch_dir(NFSPIPE_DIR, &nfs_pipedir_wfd);
				bl_pipe_fd = open(BL_PIPE_FILE, O_RDWR);
			} else if (event->mask & IN_DELETE) {
				BL_LOG_WARNING("nfs pipe dir deleted\n");
				inotify_rm_watch(bl_watch_fd, nfs_pipedir_wfd);
				close(bl_pipe_fd);
				nfs_pipedir_wfd = -1;
				bl_pipe_fd = -1;
			}
		} else if (event->wd == nfs_pipedir_wfd) {
			if (strncmp(event->name, "blocklayout", 11))
				continue;
			if (event->mask & IN_CREATE) {
				BL_LOG_WARNING("blocklayout pipe file created\n");
				bl_pipe_fd = open(BL_PIPE_FILE, O_RDWR);
				if (bl_pipe_fd < 0)
					BL_LOG_ERR("open %s failed: %s\n",
						event->name, strerror(errno));
			} else if (event->mask & IN_DELETE) {
				BL_LOG_WARNING("blocklayout pipe file deleted\n");
				close(bl_pipe_fd);
				bl_pipe_fd = -1;
			}
		}
	}
}

static int bl_event_helper(void)
{
	fd_set rset;
	int ret = 0, maxfd;

	for (;;) {
		FD_ZERO(&rset);
		FD_SET(bl_watch_fd, &rset);
		if (bl_pipe_fd > 0)
			FD_SET(bl_pipe_fd, &rset);
		maxfd = (bl_watch_fd>bl_pipe_fd)?bl_watch_fd:bl_pipe_fd;
		switch (select(maxfd + 1, &rset, NULL, NULL, NULL)) {
		case -1:
			if (errno == EINTR)
				continue;
			else {
				ret = -errno;
				goto out;
			}
		case 0:
			goto out;
		default:
			if (FD_ISSET(bl_watch_fd, &rset))
				bl_rpcpipe_cb();
			else if (bl_pipe_fd > 0 && FD_ISSET(bl_pipe_fd, &rset))
				ret = bl_disk_inquiry_process(bl_pipe_fd);
			if (ret)
				goto out;
		}
	}
 out:
	return ret;
}

void sig_die(int signal)
{
	if (pidfd >= 0) {
		close(pidfd);
		unlink(PID_FILE);
	}
	BL_LOG_ERR("exit on signal(%d)\n", signal);
	exit(1);
}
static void usage(void)
{
	fprintf(stderr, "Usage: blkmapd [-hdf]\n" );
}
/* Daemon */
int main(int argc, char **argv)
{
	int opt, dflag = 0, fg = 0, ret = 1;
	char pidbuf[64];

	while ((opt = getopt(argc, argv, "hdf")) != -1) {
		switch (opt) {
		case 'd':
			dflag = 1;
			break;
		case 'f':
			fg = 1;
			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(1);
			
		}
	}

	if (fg) {
		openlog("blkmapd", LOG_PERROR, 0);
	} else {
		if (daemon(0, 0) != 0) {
			fprintf(stderr, "Daemonize failed\n");
			exit(1);
		}

		openlog("blkmapd", LOG_PID, 0);
		pidfd = open(PID_FILE, O_WRONLY | O_CREAT, 0644);
		if (pidfd < 0) {
			BL_LOG_ERR("Create pid file %s failed\n", PID_FILE);
			exit(1);
		}

		if (lockf(pidfd, F_TLOCK, 0) < 0) {
			BL_LOG_ERR("Already running; Exiting!");
			close(pidfd);
			exit(1);
		}
		ftruncate(pidfd, 0);
		sprintf(pidbuf, "%d\n", getpid());
		write(pidfd, pidbuf, strlen(pidbuf));
	}

	signal(SIGINT, sig_die);
	signal(SIGTERM, sig_die);
	signal(SIGHUP, SIG_IGN);

	if (dflag) {
		ret = bl_discover_devices();
		goto out;
	}

	if ((bl_watch_fd = inotify_init()) < 0) {
		BL_LOG_ERR("init inotify failed %s\n", strerror(errno));
		goto out;
	}

	/* open pipe file */
	bl_watch_dir(RPCPIPE_DIR, &rpc_pipedir_wfd);
	bl_watch_dir(NFSPIPE_DIR, &nfs_pipedir_wfd);

	bl_pipe_fd = open(BL_PIPE_FILE, O_RDWR);
	if (bl_pipe_fd < 0)
		BL_LOG_ERR("open pipe file %s failed: %s\n", BL_PIPE_FILE, strerror(errno));

	while (1) {
		/* discover device when needed */
		bl_discover_devices();

		ret = bl_event_helper();
		if (ret < 0) {
			/* what should we do with process error? */
			BL_LOG_ERR("inquiry process return %d\n", ret);
		}
	}
out:
	if (pidfd >= 0) {
		close(pidfd);
		unlink(PID_FILE);
	}

	exit(ret);
}
