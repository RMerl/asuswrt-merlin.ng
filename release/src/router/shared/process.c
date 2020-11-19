/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

#include "shared.h"


//# cat /proc/1/stat
//1 (init) S 0 0 0 0 -1 256 287 10043 109 21377 7 110 473 1270 9 0 0 0 27 1810432 126 2147483647 4194304 4369680 2147450688 2147449688 717374852 0 0 0 514751 2147536844 0 0 0 0

char *psname(int pid, char *buffer, int maxlen)
{
	char buf[512];
	char path[64];
	char *p;
	int fn = 0;

	if (maxlen <= 0) return NULL;
	*buffer = 0;
	sprintf(path, "/proc/%d/stat", pid);
	if (((fn=f_read_string(path, buf, sizeof(buf))) > 4) && ((p = strrchr(buf, ')')) != NULL)) {
		*p = 0;
		if (((p = strchr(buf, '(')) != NULL) && (atoi(buf) == pid)) {
			strlcpy(buffer, p + 1, maxlen);
		}
	}
	return fn <= 0 ? "" : buffer;
}

/* There is a race condition when a brand new daemon starts up using the double-fork method.
 *   Example: dnsmasq
 * There are 2 windows of vulnerability.
 * 1) At the beginning of process startup, the new process has the wrong name, such as "init" because
 * init forks a child which execve()'s dnsmasq, but the execve() hasn't happened yet.
 * 2) At the end of process startup, the timing can be such that we don't see the long-lived process,
 * only the pid(s) of the short-lived process(es), but the psname fails because they've exited by then.
 *
 * The 1st can be covered by a retry after a slight delay.
 * The 2nd can be covered by a retry immediately.
 */
static int _pidof(const char *name, pid_t **pids)
{
	const char *p;
	char *e;
	DIR *dir;
	struct dirent *de;
	pid_t i;
	int count;
	char buf[256];

	count = 0;
	if (pids != NULL)
		*pids = NULL;
	if ((p = strrchr(name, '/')) != NULL) name = p + 1;
	if ((dir = opendir("/proc")) != NULL) {
		while ((de = readdir(dir)) != NULL) {
			i = strtol(de->d_name, &e, 10);
			if (*e != 0) continue;
			if (strcmp(name, psname(i, buf, sizeof(buf))) == 0) {
				if (pids == NULL) {
					count = i;
					break;
				}
				if ((*pids = realloc(*pids, sizeof(pid_t) * (count + 1))) == NULL) {
					return -1;
				}
				(*pids)[count++] = i;
			}
		}
	}
	closedir(dir);
	return count;
}

/* If we hit both windows, it will take three tries to discover the pid. */
int pidof(const char *name)
{
	pid_t p;

	p = _pidof(name, NULL);
	if (p < 1) {
		usleep(10 * 1000);
		p = _pidof(name, NULL);
		if (p < 1)
			p = _pidof(name, NULL);
	}
	if (p < 1)
		return -1;
	return p;
}

int ppid(int pid) {
	char buf[512];
	char path[64];
	int ppid = 0;

	buf[0] = 0;
	sprintf(path, "/proc/%d/stat", pid);
	if ((f_read_string(path, buf, sizeof(buf)) > 4))
		sscanf(buf, "%*d %*s %*c %d", &ppid);

	return ppid;
}

int killall(const char *name, int sig)
{
	pid_t *pids;
	int i;
	int r;

	if ((i = _pidof(name, &pids)) > 0) {
		r = 0;
		do {
			r |= kill(pids[--i], sig);
		} while (i > 0);
		free(pids);
		return r;
	}
	return -2;
}

void killall_tk_period_wait(const char *name, int wait)
{
	int n;

	if (killall(name, SIGTERM) == 0) {
		n = wait;
		while ((killall(name, 0) == 0) && (n-- > 0)) {
//			_dprintf("%s: waiting name=%s n=%d\n", __FUNCTION__, name, n);
			sleep(1);
		}
		if (n < 0) {
			n = wait;
			while ((killall(name, SIGKILL) == 0) && (n-- > 0)) {
//				_dprintf("%s: SIGKILL name=%s n=%d\n", __FUNCTION__, name, n);
				sleep(1);
			}
		}
	}
}

int process_exists(pid_t pid)
{
	return (kill(pid, 0) == 0 || errno != ESRCH);
}

int module_loaded(const char *module)
{
	char sys_path[128];
	char *p = sys_path;

	snprintf(sys_path, sizeof(sys_path), "/sys/module/%s", module);
	while ((p = strchr(p, '-'))) {
		*p = '_';
	}
	return d_exists(sys_path);
}

#if defined(RTCONFIG_PTHSAFE_POPEN)
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/un.h>
#include <signal.h>
#if defined(__GLIBC__) || defined(__UCLIBC__) /* not musl */
#include <wait.h>
#else
#include <sys/wait.h>
#endif

static int un_tcpsock_connect(char *path, int nodelay)
{
	int sock;
	struct sockaddr_un uaddr;
	const int on = 1;

	memset ((char *)&uaddr, 0, sizeof(uaddr));

	sock = socket(AF_LOCAL, SOCK_STREAM, 0);
	if ( sock == -1 ) {
		_dprintf("%s: socket: %s\n", __func__, strerror(errno));
		return -1;
	}

	uaddr.sun_family = AF_LOCAL;
	strcpy(uaddr.sun_path, path);

	if ( connect(sock, (struct sockaddr*)(&uaddr), sizeof(uaddr)) == -1 ) {
		close(sock);
		_dprintf("%s: connect[%s]: %s\n", __func__, path, strerror(errno));
		close(sock);
		return -1;
	}

	if (nodelay)
		setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
	return sock;
}

#if defined(RTCONFIG_QCA) || defined(CONFIG_BCMWL5)
static int not_in_thread(void)
{
	struct stat task_stat;

	if (stat("/proc/self/task", &task_stat))
		return 1;

	if (task_stat.st_nlink <= 3) return 1;
	else return 0;
}
#else
#warning WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
#warning WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
#warning WWWWWWW implement your own pthread detection mechanism WWWWWWW
#warning WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
#warning WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
static int not_in_thread(void)
{
	return 0; // assume always in pthread
}
#endif

#ifdef popen
#undef popen
#endif
#ifdef pclose
#undef pclose
#endif
#define SELECT_TIMEOUT	2
FILE *PS_popen(const char *cmd, const char *mode)
{
	int sfd, ret, len;
	fd_set rmask;
	struct timeval select_timeout;

	if (not_in_thread())
		return popen(cmd, mode);
#if 0 /* debug message */
	_dprintf("%s: PID[%d], CMD[%s]\n", __func__, getpid(), cmd);
#endif

	if (!mode || strcmp(mode, "r")!=0) {
		_dprintf("%s: EEEEEError!!!! [%s][%s]\n", __func__, cmd, mode);
		return NULL;
	}

	sfd = un_tcpsock_connect(PS_SOCK, 1);
	if (sfd==-1) {
		_dprintf("%s: tcpsock_connect!\n", __func__);
		return NULL;
	}

	len = strlen(cmd);
	write(sfd, cmd, len);
	FD_ZERO(&rmask);
	FD_SET(sfd, &rmask);
	select_timeout.tv_sec = SELECT_TIMEOUT;
	select_timeout.tv_usec = 0;

	ret = select(sfd+1, &rmask, NULL, NULL, &select_timeout);
	if (ret > 0) {
		FILE *fp;
		fp = fdopen(sfd, "r");
		if (fp) {
			return fp;
		}
	}
	close(sfd);
	return NULL;
}

int PS_pclose(FILE *fp)
{
	if (fp) {
		if (not_in_thread())
			return pclose(fp);
		fclose(fp);
	}
	return 0;
}
#endif
