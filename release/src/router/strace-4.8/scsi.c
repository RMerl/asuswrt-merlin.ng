/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007 Dmitry V. Levin <ldv@altlinux.org>
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
#include <sys/ioctl.h>
#include <scsi/sg.h>

static const struct xlat sg_io_dxfer_direction[] = {
	{SG_DXFER_NONE,        "SG_DXFER_NONE"},
	{SG_DXFER_TO_DEV,      "SG_DXFER_TO_DEV"},
	{SG_DXFER_FROM_DEV,    "SG_DXFER_FROM_DEV"},
	{SG_DXFER_TO_FROM_DEV, "SG_DXFER_TO_FROM_DEV"},
	{0, NULL}
};

static void
print_sg_io_buffer(struct tcb *tcp, unsigned char *addr, int len)
{
	unsigned char *buf = NULL;
	int     allocated, i;

	if (len == 0)
		return;
	allocated = (len > max_strlen) ? max_strlen : len;
	if (len < 0 ||
	    (buf = malloc(allocated)) == NULL ||
	    umoven(tcp, (unsigned long) addr, allocated, (char *) buf) < 0) {
		tprintf("%p", addr);
		free(buf);
		return;
	}
	tprintf("%02x", buf[0]);
	for (i = 1; i < allocated; ++i)
		tprintf(", %02x", buf[i]);
	free(buf);
	if (allocated != len)
		tprints(", ...");
}

static void
print_sg_io_req(struct tcb *tcp, struct sg_io_hdr *sg_io)
{
	tprintf("{'%c', ", sg_io->interface_id);
	printxval(sg_io_dxfer_direction, sg_io->dxfer_direction,
		  "SG_DXFER_???");
	tprintf(", cmd[%u]=[", sg_io->cmd_len);
	print_sg_io_buffer(tcp, sg_io->cmdp, sg_io->cmd_len);
	tprintf("], mx_sb_len=%d, ", sg_io->mx_sb_len);
	tprintf("iovec_count=%d, ", sg_io->iovec_count);
	tprintf("dxfer_len=%u, ", sg_io->dxfer_len);
	tprintf("timeout=%u, ", sg_io->timeout);
	tprintf("flags=%#x", sg_io->flags);

	if (sg_io->dxfer_direction == SG_DXFER_TO_DEV ||
	    sg_io->dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		tprintf(", data[%u]=[", sg_io->dxfer_len);
		printstr(tcp, (unsigned long) sg_io->dxferp,
			 sg_io->dxfer_len);
		tprints("]");
	}
}

static void
print_sg_io_res(struct tcb *tcp, struct sg_io_hdr *sg_io)
{
	if (sg_io->dxfer_direction == SG_DXFER_FROM_DEV ||
	    sg_io->dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		tprintf(", data[%u]=[", sg_io->dxfer_len);
		printstr(tcp, (unsigned long) sg_io->dxferp,
			 sg_io->dxfer_len);
		tprints("]");
	}
	tprintf(", status=%02x, ", sg_io->status);
	tprintf("masked_status=%02x, ", sg_io->masked_status);
	tprintf("sb[%u]=[", sg_io->sb_len_wr);
	print_sg_io_buffer(tcp, sg_io->sbp, sg_io->sb_len_wr);
	tprintf("], host_status=%#x, ", sg_io->host_status);
	tprintf("driver_status=%#x, ", sg_io->driver_status);
	tprintf("resid=%d, ", sg_io->resid);
	tprintf("duration=%d, ", sg_io->duration);
	tprintf("info=%#x}", sg_io->info);
}

int
scsi_ioctl(struct tcb *tcp, long code, long arg)
{
	switch (code) {
	case SG_IO:
		if (entering(tcp)) {
			struct sg_io_hdr sg_io;

			if (umove(tcp, arg, &sg_io) < 0)
				tprintf(", %#lx", arg);
			else {
				tprints(", ");
				print_sg_io_req(tcp, &sg_io);
			}
		}
		if (exiting(tcp)) {
			struct sg_io_hdr sg_io;

			if (!syserror(tcp) && umove(tcp, arg, &sg_io) >= 0)
				print_sg_io_res(tcp, &sg_io);
			else
				tprints("}");
		}
		break;
	default:
		if (entering(tcp))
			tprintf(", %#lx", arg);
		break;
	}
	return 1;
}
