/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include <unistd.h>
#include "host_adapt.h"
#include "sys_switch_ioctl.h"

#define SMDIO_ADDR 0x10

#define FILELOCK_REPLACE_PTHREAD_MUTEX_LOCK 1

static void __usleep(unsigned long usec)
{
	/* TO be replaced with OS dependent implementation */
	usleep(usec);
}

#ifdef FILELOCK_REPLACE_PTHREAD_MUTEX_LOCK
#include <fcntl.h>
#include <syslog.h>

/* Serialize using fcntl() calls
 */

/* when lock file has been re-opened by the same process,
 * it can't be closed, because it release original lock,
 * that have been set earlier. this results in file
 * descriptors leak.
 * one way to avoid it - check if the process has set the
 * lock already via /proc/locks, but it seems overkill
 * with al of related file ops and text searching. there's
 * no kernel API for that.
 * maybe need different lock kind? */
#define LET_FD_LEAK

int _file_lock2(const char *dir, const char *tag)
{
	struct flock lock;
	char path[64];
	int lockfd;
	pid_t pid, err;
#ifdef LET_FD_LEAK
	pid_t lockpid;
#else
	struct stat st;
#endif

	snprintf(path, sizeof(path), "%s/%s.lock", dir, tag);

#ifndef LET_FD_LEAK
	pid = getpid();

	/* check if we already hold a lock */
	if (stat(path, &st) == 0 && !S_ISDIR(st.st_mode) && st.st_size > 0) {
		FILE *fp;
		char line[100], *ptr, *value;
		char id[sizeof("XX:XX:4294967295")];

		if ((fp = fopen("/proc/locks", "r")) == NULL)
			goto error;

		snprintf(id, sizeof(id), "%02x:%02x:%ld",
			 major(st.st_dev), minor(st.st_dev), st.st_ino);
		while ((value = fgets(line, sizeof(line), fp)) != NULL) {
			strtok_r(line, " ", &ptr);
			if ((value = strtok_r(NULL, " ", &ptr)) && strcmp(value, "POSIX") == 0 &&
			    (value = strtok_r(NULL, " ", &ptr)) && /* strcmp(value, "ADVISORY") == 0 && */
			    (value = strtok_r(NULL, " ", &ptr)) && strcmp(value, "WRITE") == 0 &&
			    (value = strtok_r(NULL, " ", &ptr)) && atoi(value) == pid &&
			    (value = strtok_r(NULL, " ", &ptr)) && strcmp(value, id) == 0)
				break;
		}
		fclose(fp);

		if (value != NULL) {
			syslog(LOG_DEBUG, "Error locking %s: %d %s", path, 0, "Already locked");
			return -1;
		}
	}
#endif

	if ((lockfd = open(path, O_CREAT | O_RDWR, 0666)) < 0)
		goto error;

#ifdef LET_FD_LEAK
	pid = getpid();

	/* check if we already hold a lock */
	if (read(lockfd, &lockpid, sizeof(pid_t)) == sizeof(pid_t) &&
	    lockpid == pid) {
		/* don't close the file here as that will release all locks */
		syslog(LOG_DEBUG, "Error locking %s: %d %s", path, 0, "Already locked");
		return -1;
	}
#endif

	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_pid = pid;
	while (fcntl(lockfd, F_SETLKW, &lock) < 0) {
		if (errno != EINTR)
			goto close;
	}

	if (lseek(lockfd, 0, SEEK_SET) < 0 ||
	    write(lockfd, &pid, sizeof(pid_t)) < 0)
		goto close;

	return lockfd;

close:
	err = errno;
	close(lockfd);
	errno = err;
error:
	syslog(LOG_DEBUG, "Error locking %s: %d %s", path, errno, strerror(errno));
	return -1;
}

int file_lock2(const char *tag)
{
	return _file_lock2("/var/lock", tag);
}

int _file_unlock2(int lockfd)
{
	if (lockfd < 0) {
		errno = EBADF;
		return -1;
	}

	ftruncate(lockfd, 0);
	return close(lockfd);
}

void file_unlock2(int lockfd)
{
	if (_file_unlock2(lockfd) < 0)
		syslog(LOG_DEBUG, "Error unlocking %d: %d %s", lockfd, errno, strerror(errno));
}

static int lock2;
#else
static pthread_mutex_t lock;
#endif

static void __lock(void *lock_data)
{
#ifdef FILELOCK_REPLACE_PTHREAD_MUTEX_LOCK
	lock2 = file_lock2("hostapi");
#else
	pthread_mutex_lock(lock_data);
#endif
}

static void __unlock(void *lock_data)
{
#ifdef FILELOCK_REPLACE_PTHREAD_MUTEX_LOCK
	file_unlock2(lock2);
#else
	pthread_mutex_unlock(lock_data);
#endif
}

static int mdiobus_read(void *mdiobus_data, uint8_t phyaddr, uint8_t mmd,
			uint16_t reg)
{
	uint16_t val=0;
	int ret;

	if (/*!mdiobus_data || */phyaddr > 31 || reg > GSW_MMD_REG_DATA_LAST)
		return -EINVAL;

	if (mmd == GSW_MMD_DEV)
		ret = sys_cl45_mdio_read(phyaddr, mmd, reg, &val);
	else if (mmd == GSW_MMD_SMDIO_DEV)
		ret = sys_cl22_mdio_read(phyaddr, reg, &val);
	else
		return -EINVAL;

	return ret < 0 ? ret : (int)val;
}

static int mdiobus_write(void *mdiobus_data, uint8_t phyaddr, uint8_t mmd,
			 uint16_t reg, uint16_t val)
{
	if (/*!mdiobus_data || */phyaddr > 31 || reg > GSW_MMD_REG_DATA_LAST)
		return -EINVAL;

	if (mmd == GSW_MMD_DEV)
		return sys_cl45_mdio_write(phyaddr, mmd, reg, val);
	else if (mmd == GSW_MMD_SMDIO_DEV)
		return sys_cl22_mdio_write(phyaddr, reg, val);
	else
		return -EINVAL;
}

static GSW_Device_t gsw_dev = {0};


/* TO be adapted  with target dependent implementation */
int gsw_adapt_init(void)
{
	gsw_dev.usleep = __usleep;

	gsw_dev.lock = __lock;
	gsw_dev.unlock = __unlock;
#ifdef FILELOCK_REPLACE_PTHREAD_MUTEX_LOCK
	gsw_dev.lock_data = &lock2;
#else
	gsw_dev.lock_data = &lock;
#endif

	gsw_dev.mdiobus_read = mdiobus_read;
	gsw_dev.mdiobus_write = mdiobus_write;
	gsw_dev.mdiobus_data = NULL;

	gsw_dev.phy_addr = SMDIO_ADDR;
	gsw_dev.smdio_phy_addr = SMDIO_ADDR;

	return 0;
}

int32_t api_gsw_get_links(char *lib)
{
	gsw_adapt_init();
	return 0;
}

GSW_Device_t* gsw_get_struc(uint8_t lif_id,uint8_t phy_id)
{
   return &gsw_dev;
}

int gsw_read(const GSW_Device_t *dev, uint32_t regaddr)
{
	return dev->mdiobus_read(dev->mdiobus_data, dev->phy_addr, GSW_MMD_DEV,
				 regaddr);
}

int gsw_write(const GSW_Device_t *dev, uint32_t regaddr, uint16_t data)
{
	return dev->mdiobus_write(dev->mdiobus_data, dev->phy_addr, GSW_MMD_DEV,
				  regaddr, data);
}
