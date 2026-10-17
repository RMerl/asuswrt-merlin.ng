/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * termios fuctions to support arbitrary baudrates (on Linux)
 *
 * Copyright (c) 2021 Pali Rohár <pali@kernel.org>
 * Copyright (c) 2021 Marek Behún <kabel@kernel.org>
 */

#ifndef _TERMIOS_LINUX_H_
#define _TERMIOS_LINUX_H_

/*
 * We need to use raw TCGETS2/TCSETS2 or TCGETS/TCSETS ioctls with the BOTHER
 * flag in struct termios2/termios, defined in Linux headers <asm/ioctls.h>
 * (included by <sys/ioctl.h>) and <asm/termbits.h>. Since these headers
 * conflict with glibc's header file <termios.h>, it is not possible to use
 * libc's termios functions and we need to reimplement them via ioctl() calls.
 *
 * An arbitrary baudrate is supported when the macro BOTHER is defined. The
 * baudrate value itself is then stored into the c_ospeed and c_ispeed members.
 * If ioctls TCGETS2/TCSETS2 are defined and supported then these fields are
 * present in struct termios2, otherwise these fields are present in struct
 * termios.
 *
 * Note that the Bnnn constants from <termios.h> need not be compatible with Bnnn
 * constants from <asm/termbits.h>.
 */

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <asm/ioctls.h>
#include <asm/termbits.h>

#if defined(BOTHER) && defined(TCGETS2)
#define termios termios2
#endif

static inline int tcgetattr(int fd, struct termios *t)
{
#if defined(BOTHER) && defined(TCGETS2)
	return ioctl(fd, TCGETS2, t);
#else
	return ioctl(fd, TCGETS, t);
#endif
}

static inline int tcsetattr(int fd, int a, const struct termios *t)
{
	int cmd;

	switch (a) {
#if defined(BOTHER) && defined(TCGETS2)
	case TCSANOW:
		cmd = TCSETS2;
		break;
	case TCSADRAIN:
		cmd = TCSETSW2;
		break;
	case TCSAFLUSH:
		cmd = TCSETSF2;
		break;
#else
	case TCSANOW:
		cmd = TCSETS;
		break;
	case TCSADRAIN:
		cmd = TCSETSW;
		break;
	case TCSAFLUSH:
		cmd = TCSETSF;
		break;
#endif
	default:
		errno = EINVAL;
		return -1;
	}

	return ioctl(fd, cmd, t);
}

static inline int tcdrain(int fd)
{
	return ioctl(fd, TCSBRK, 1);
}

static inline int tcflush(int fd, int q)
{
	return ioctl(fd, TCFLSH, q);
}

static inline int tcsendbreak(int fd, int d)
{
#ifdef TCSBRKP
	return ioctl(fd, TCSBRKP, d);
#else
	return ioctl(fd, TCSBRK, 0);
#endif
}

static inline int tcflow(int fd, int a)
{
	return ioctl(fd, TCXONC, a);
}

static inline pid_t tcgetsid(int fd)
{
	pid_t sid;

	if (ioctl(fd, TIOCGSID, &sid) < 0)
		return (pid_t)-1;

	return sid;
}

static inline speed_t cfgetospeed(const struct termios *t)
{
	return t->c_cflag & CBAUD;
}

static inline int cfsetospeed(struct termios *t, speed_t s)
{
	if (s & ~CBAUD) {
		errno = EINVAL;
		return -1;
	}

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= s;

	return 0;
}

#ifdef IBSHIFT
static inline speed_t cfgetispeed(const struct termios *t)
{
	speed_t s = (t->c_cflag >> IBSHIFT) & CBAUD;

	if (s == B0)
		return cfgetospeed(t);
	else
		return s;
}

static inline int cfsetispeed(struct termios *t, speed_t s)
{
	if (s == 0)
		s = B0;

	if (s & ~CBAUD) {
		errno = EINVAL;
		return -1;
	}

	t->c_cflag &= ~(CBAUD << IBSHIFT);
	t->c_cflag |= s << IBSHIFT;

	return 0;
}
#else /* !IBSHIFT */
static inline speed_t cfgetispeed(const struct termios *t)
{
	return cfgetospeed(t);
}

static inline int cfsetispeed(struct termios *t, speed_t s)
{
	return cfsetospeed(t, s);
}
#endif /* !IBSHIFT */

static inline int cfsetspeed(struct termios *t, speed_t s)
{
	if (cfsetospeed(t, s))
		return -1;
#ifdef IBSHIFT
	if (cfsetispeed(t, s))
		return -1;
#endif

	return 0;
}

static void cfmakeraw(struct termios *t)
{
	t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
			ICRNL | IXON);
	t->c_oflag &= ~OPOST;
	t->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	t->c_cflag &= ~(CSIZE | PARENB);
	t->c_cflag |= CS8;
}

#endif /* _TERMIOS_LINUX_H_ */
