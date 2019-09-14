/*
    bdpoll.c (based on addon-storage.c from hal-0.5.10)

    Poll storage devices for media changes

    Copyright (C) 2007 Andreas Oberritter
    Copyright (C) 2004 David Zeuthen, <david@fubar.dk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License 2.0 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <mntent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mount.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include <sys/stat.h>
#include <sys/types.h>

enum {
	MEDIA_STATUS_UNKNOWN = 0,
	MEDIA_STATUS_GOT_MEDIA = 1,
	MEDIA_STATUS_NO_MEDIA = 2,
};

static int media_status = MEDIA_STATUS_NO_MEDIA;
static const int interval_in_seconds = 2;
static char *pidfile = NULL;

static void bdpoll_notify(const char devname[])
{
#if 1
	char buf[128];
	char *argv[] = { "asus_sr",
		(char *) devname,
		media_status == MEDIA_STATUS_GOT_MEDIA ? "add" : "remove",
		NULL
	};
	pid_t pid;
	int fd;

	pid = fork();
	if (pid == 0) {
		/* child */
		setsid();

		chdir("/");

		fd = open("/dev/null", O_RDWR);
		if (fd >= 0) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > STDERR_FILENO)
				close(fd);
		}

		snprintf(buf, sizeof(buf), "/block/%s", devname);
		setenv("DEVPATH", buf, 1);
		snprintf(buf, sizeof(buf), "/block/%s/device", devname);
		setenv("PHYSDEVPATH", buf, 1);

		setenv("BDPOLL_DEVICE", devname, 1);
		setenv("BDPOLL_STATUS", media_status == MEDIA_STATUS_GOT_MEDIA ? "1" : "0", 1);

		execvp(argv[0], argv);

		_exit(1);
	} else if (pid < 0)
		warn("%s: %s", "fork", strerror(errno));
#else
	char buf[1024];
	if (media_status == MEDIA_STATUS_GOT_MEDIA)
	{
		snprintf(buf, sizeof(buf), "/autofs/%s", devname);
		mkdir(buf, 0777);
		// workaround for crashing mount -t auto on audio CD
		// try DVD first (because CDFS also works but yields an ISO)
		snprintf(buf, sizeof(buf), "/bin/mount -t udf /dev/%s /autofs/%s", devname, devname);
		if (system(buf) != 0)
		{
			// udf fails, try cdfs
			snprintf(buf, sizeof(buf), "/bin/mount -t cdfs /dev/%s /autofs/%s", devname, devname);
			if (system(buf) != 0)
			{
				// cdfs failed too. Does that even make sense?
				snprintf(buf, sizeof(buf), "/bin/mount /dev/%s /autofs/%s", devname, devname);
				system(buf);
			}
		}
	}
	else
	{
		snprintf(buf, sizeof(buf), "/bin/umount /dev/%s", devname);
		system(buf);
		snprintf(buf, sizeof(buf), "/autofs/%s", devname);
		unlink(buf);
	}
	snprintf(buf, sizeof(buf), "/block/%s", devname);
	setenv("DEVPATH", buf, 1);
	snprintf(buf, sizeof(buf), "/block/%s/device", devname);
	setenv("PHYSDEVPATH", buf, 1);
	setenv("X_E2_MEDIA_STATUS", (media_status == MEDIA_STATUS_GOT_MEDIA) ? "1" : "0", 1);
	system("/usr/bin/hotplug_e2_helper");
#endif
}

static bool is_mounted(const char device_file[])
{
	FILE *f;
	bool rc;
	struct mntent mnt;
	struct mntent *mnte;
	char buf[512];

	rc = false;

	if ((f = setmntent("/etc/mtab", "r")) == NULL)
		return rc;

	while ((mnte = getmntent_r(f, &mnt, buf, sizeof(buf))) != NULL) {
		if (strcmp(device_file, mnt.mnt_fsname) == 0) {
			rc = true;
			break;
		}
	}

	endmntent(f);
	return rc;
}

static bool poll_for_media(const char device_file[], bool is_cdrom, bool support_media_changed)
{
	int fd;
	bool got_media = false;
	bool ret = false;

	if (is_cdrom) {
		int drive;

		fd = open(device_file, O_RDONLY | O_NONBLOCK | O_EXCL);
		if (fd < 0 && errno == EBUSY) {
			/* this means the disc is mounted or some other app,
			 * like a cd burner, has already opened O_EXCL */

			/* HOWEVER, when starting hald, a disc may be
			 * mounted; so check /etc/mtab to see if it
			 * actually is mounted. If it is we retry to open
			 * without O_EXCL
			 */
			if (!is_mounted(device_file))
				return false;
			fd = open(device_file, O_RDONLY | O_NONBLOCK);
		}
		if (fd < 0) {
			err(1, "%s: %s", device_file, strerror(errno));
			return false;
		}

		/* Check if a disc is in the drive
		 *
		 * @todo Use MMC-2 API if applicable
		 */
		drive = ioctl(fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
		switch (drive) {
		case CDS_NO_INFO:
		case CDS_NO_DISC:
		case CDS_TRAY_OPEN:
		case CDS_DRIVE_NOT_READY:
			break;

		case CDS_DISC_OK:
			/* some CD-ROMs report CDS_DISK_OK even with an open
			 * tray; if media check has the same value two times in
			 * a row then this seems to be the case and we must not
			 * report that there is a media in it. */
			if (support_media_changed &&
			    ioctl(fd, CDROM_MEDIA_CHANGED, CDSL_CURRENT) &&
			    ioctl(fd, CDROM_MEDIA_CHANGED, CDSL_CURRENT)) {
			} else {
				got_media = true;
				/*
				 * this is a bit of a hack; because we mount the cdrom, the eject button
				 * would not work, so we would never detect 'medium removed', and
				 * never umount the cdrom.
				 * So we unlock the door
				 */
				ioctl(fd, CDROM_LOCKDOOR, 0);
			}
			break;

		case -1:
			err(1, "%s: CDROM_DRIVE_STATUS: %s", device_file, strerror(errno));
			break;
		}

		close(fd);
	} else {
		fd = open(device_file, O_RDONLY);
		if ((fd < 0) && (errno == ENOMEDIUM)) {
			got_media = false;
		} else if (fd >= 0) {
			got_media = true;
		} else {
			err(1, "%s: %s", device_file, strerror(errno));
			return false;
		}
	}

	switch (media_status) {
	case MEDIA_STATUS_GOT_MEDIA:
		if (!got_media) {
			printf("Media removal detected on %s\n", device_file);
			ret = true;
			/* have to this to trigger appropriate hotplug events */
			fd = open(device_file, O_RDONLY | O_NONBLOCK);
			if (fd >= 0) {
				ioctl(fd, BLKRRPART);
				close(fd);
			}
		}
		break;

	case MEDIA_STATUS_NO_MEDIA:
		if (got_media) {
			printf("Media insertion detected on %s\n", device_file);
			ret = true;
		}
		break;
	}

	/* update our current status */
	if (got_media)
		media_status = MEDIA_STATUS_GOT_MEDIA;
	else
		media_status = MEDIA_STATUS_NO_MEDIA;

	return ret;
}

static void usage(const char argv0[])
{
	fprintf(stderr, "usage: %s <devname> [-c][-m][-d][-p pidfile]\n", argv0);
}

static void remove_pidfile(void)
{
	if (pidfile) {
		unlink(pidfile);
		pidfile = NULL;
	}
}

static void sig_term(int sig)
{
	remove_pidfile();
	_exit(1);
}

int bdpoll(int argc, char *argv[], char *envp[])
{
	const char *devname;
	char devnode[1024], pid[32];
	bool is_cdrom = false;
	bool support_media_changed = false;
	bool daemonize = true;
	int opt, fd = -1;

	while ((opt = getopt(argc, argv, "cmdp:")) != -1) {
		switch (opt) {
		case 'c':
			is_cdrom = true;
			break;
		case 'm':
			support_media_changed = true;
			break;
		case 'p':
			pidfile = optarg;
			break;
		case 'd':
			daemonize = false;
			break;
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (optind > argc) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	devname = argv[optind];
	snprintf(devnode, sizeof(devnode), "/dev/%s", devname);

	if (daemonize && pidfile) {
		fd = open(pidfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (fd < 0)
			err(1, "%s: %s", pidfile, strerror(errno));
	}

	if (daemonize) {
		daemon(0, 0);

		if (pidfile && fd >= 0) {
			snprintf(pid, sizeof(pid), "%d\n", getpid());
			write(fd, pid, strlen(pid));
			close(fd);

			atexit(remove_pidfile);
			signal(SIGTERM, sig_term);
		}
	}

	signal(SIGCHLD, SIG_IGN);

	for (;;) {
		if (poll_for_media(devnode, is_cdrom, support_media_changed))
			bdpoll_notify(devname);
		sleep(interval_in_seconds);
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[], char *envp[])
{
	return bdpoll(argc, argv, envp);
}
