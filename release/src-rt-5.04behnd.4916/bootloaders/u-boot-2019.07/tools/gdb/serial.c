// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@csiro.au>
 */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include "serial.h"

#if defined(__sun__)	 || \
    defined(__OpenBSD__) || \
    defined(__FreeBSD__) || \
    defined(__NetBSD__)	 || \
    defined(__APPLE__)
static struct termios tios = { BRKINT, 0, B115200|CS8|CREAD, 0, { 0 } };
#else
static struct termios tios = { BRKINT, 0, B115200|CS8|CREAD, 0,   0   };
#endif

static struct speedmap {
    char *str;
    speed_t val;
} speedmap[] = {
    { "50", B50 },		{ "75", B75 },		{ "110", B110 },
    { "134", B134 },		{ "150", B150 },	{ "200", B200 },
    { "300", B300 },		{ "600", B600 },	{ "1200", B1200 },
    { "1800", B1800 },		{ "2400", B2400 },	{ "4800", B4800 },
    { "9600", B9600 },		{ "19200", B19200 },	{ "38400", B38400 },
    { "57600", B57600 },
#ifdef	B76800
    { "76800", B76800 },
#endif
    { "115200", B115200 },
#ifdef	B153600
    { "153600", B153600 },
#endif
    { "230400", B230400 },
#ifdef	B307200
    { "307200", B307200 },
#endif
#ifdef B460800
    { "460800", B460800 }
#endif
};
static int nspeeds = sizeof speedmap / sizeof speedmap[0];

speed_t
cvtspeed(char *str)
{
    struct speedmap *smp = speedmap, *esmp = &speedmap[nspeeds];

    while (smp < esmp) {
	if (strcmp(str, smp->str) == 0)
	    return (smp->val);
	smp++;
    }
    return B0;
}

int
serialopen(char *device, speed_t speed)
{
    int fd;

    if (cfsetospeed(&tios, speed) < 0)
	return -1;

    if ((fd = open(device, O_RDWR)) < 0)
	return -1;

    if (tcsetattr(fd, TCSAFLUSH, &tios) < 0) {
	(void)close(fd);
	return -1;
    }

    return fd;
}

int
serialreadchar(int fd, int timeout)
{
    fd_set fds;
    struct timeval tv;
    int n;
    char ch;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    /* this is a fucking horrible quick hack - fix this */

    if ((n = select(fd + 1, &fds, 0, 0, &tv)) < 0)
	return SERIAL_ERROR;

    if (n == 0)
	return SERIAL_TIMEOUT;

    if ((n = read(fd, &ch, 1)) < 0)
	return SERIAL_ERROR;

    if (n == 0)
	return SERIAL_EOF;

    return ch;
}

int
serialwrite(int fd, char *buf, int len)
{
    int n;

    do {
	n = write(fd, buf, len);
	if (n < 0)
	    return 1;
	len -= n;
	buf += n;
    } while (len > 0);
    return 0;
}

int
serialclose(int fd)
{
    return close(fd);
}
