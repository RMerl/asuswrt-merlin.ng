/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2001 Wichert Akkerman <wichert@cistron.nl>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

#include "defs.h"
#include <asm/ioctl.h>

static int
compare(const void *a, const void *b)
{
	unsigned long code1 = (long) a;
	unsigned long code2 = ((struct_ioctlent *) b)->code;
	return (code1 > code2) ? 1 : (code1 < code2) ? -1 : 0;
}

const struct_ioctlent *
ioctl_lookup(long code)
{
	struct_ioctlent *iop;

	code &= (_IOC_NRMASK<<_IOC_NRSHIFT) | (_IOC_TYPEMASK<<_IOC_TYPESHIFT);
	iop = bsearch((void*)code, ioctlent,
			nioctlents, sizeof(ioctlent[0]), compare);
	while (iop > ioctlent) {
		iop--;
		if (iop->code != code) {
			iop++;
			break;
		}
	}
	return iop;
}

const struct_ioctlent *
ioctl_next_match(const struct_ioctlent *iop)
{
	long code;

	code = iop->code;
	iop++;
	if (iop < ioctlent + nioctlents && iop->code == code)
		return iop;
	return NULL;
}

int
ioctl_decode(struct tcb *tcp, long code, long arg)
{
	switch ((code >> 8) & 0xff) {
#if defined(ALPHA) || defined(POWERPC)
	case 'f': case 't': case 'T':
#else /* !ALPHA */
	case 0x54:
#endif /* !ALPHA */
		return term_ioctl(tcp, code, arg);
	case 0x89:
		return sock_ioctl(tcp, code, arg);
	case 'p':
		return rtc_ioctl(tcp, code, arg);
	case 0x03:
	case 0x12:
		return block_ioctl(tcp, code, arg);
	case 0x22:
		return scsi_ioctl(tcp, code, arg);
	case 'L':
		return loop_ioctl(tcp, code, arg);
	case 'M':
		return mtd_ioctl(tcp, code, arg);
	case 'o':
	case 'O':
		return ubi_ioctl(tcp, code, arg);
	default:
		break;
	}
	return 0;
}

/*
 * Registry of ioctl characters, culled from
 *	@(#)ioccom.h 1.7 89/06/16 SMI; from UCB ioctl.h 7.1 6/4/86
 *
 * char	file where defined		notes
 * ----	------------------		-----
 *   F	sun/fbio.h
 *   G	sun/gpio.h
 *   H	vaxif/if_hy.h
 *   M	sundev/mcpcmd.h			*overlap*
 *   M	sys/modem.h			*overlap*
 *   S	sys/stropts.h
 *   T	sys/termio.h			-no overlap-
 *   T	sys/termios.h			-no overlap-
 *   V	sundev/mdreg.h
 *   a	vaxuba/adreg.h
 *   d	sun/dkio.h			-no overlap with sys/des.h-
 *   d	sys/des.h			(possible overlap)
 *   d	vax/dkio.h			(possible overlap)
 *   d	vaxuba/rxreg.h			(possible overlap)
 *   f	sys/filio.h
 *   g	sunwindow/win_ioctl.h		-no overlap-
 *   g	sunwindowdev/winioctl.c		!no manifest constant! -no overlap-
 *   h	sundev/hrc_common.h
 *   i	sys/sockio.h			*overlap*
 *   i	vaxuba/ikreg.h			*overlap*
 *   k	sundev/kbio.h
 *   m	sundev/msio.h			(possible overlap)
 *   m	sundev/msreg.h			(possible overlap)
 *   m	sys/mtio.h			(possible overlap)
 *   n	sun/ndio.h
 *   p	net/nit_buf.h			(possible overlap)
 *   p	net/nit_if.h			(possible overlap)
 *   p	net/nit_pf.h			(possible overlap)
 *   p	sundev/fpareg.h			(possible overlap)
 *   p	sys/sockio.h			(possible overlap)
 *   p	vaxuba/psreg.h			(possible overlap)
 *   q	sun/sqz.h
 *   r	sys/sockio.h
 *   s	sys/sockio.h
 *   t	sys/ttold.h			(possible overlap)
 *   t	sys/ttycom.h			(possible overlap)
 *   v	sundev/vuid_event.h		*overlap*
 *   v	sys/vcmd.h			*overlap*
 *
 * End of Registry
 */
