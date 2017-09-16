/*
 * support/nfs/export.c
 *
 * Add or delete an NFS export in knfsd.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "nfslib.h"
#include "misc.h"
#include "xcommon.h"

	/* if /proc/net/rpc/... exists, then 
	 * write to it, as that interface is more stable.
	 * Write:
	 *  client fsidtype fsid path
	 * to /proc/net/rpc/nfsd.fh/channel
	 * and
	 *  client path expiry flags anonuid anongid fsid
	 * to /proc/net/rpc/nfsd.export/channel
	 */

static int
exp_unexp(struct nfsctl_export *exp, int export)
{
	char buf[RPC_CHAN_BUF_SIZE], *bp;
	struct stat stb;
	__u32 fsid;
	char fsidstr[8];
	__u16 dev;
	__u32 inode;
	int err = 0, f, blen;

	f = open("/proc/net/rpc/nfsd.export/channel", O_WRONLY);
	if (f < 0) return -1;

	bp = buf; blen = sizeof(buf);
	qword_add(&bp, &blen, exp->ex_client);
	qword_add(&bp, &blen, exp->ex_path);
	if (export) {
		qword_addint(&bp, &blen, 0x7fffffff);
		qword_addint(&bp, &blen, exp->ex_flags);
		qword_addint(&bp, &blen, exp->ex_anon_uid);
		qword_addint(&bp, &blen, exp->ex_anon_gid);
		qword_addint(&bp, &blen, exp->ex_dev);
	} else
		qword_addint(&bp, &blen, 1);
	qword_addeol(&bp, &blen);
	if (blen <= 0 || write(f, buf, bp - buf) != bp - buf)
		err = -1;
	close(f);

	if (stat(exp->ex_path, &stb) != 0)
		return -1;

	f = open("/proc/net/rpc/nfsd.fh/channel", O_WRONLY);
	if (f < 0) return -1;
	if (exp->ex_flags & NFSEXP_FSID) {
		bp = buf; blen = sizeof(buf);
		qword_add(&bp, &blen, exp->ex_client);
		qword_addint(&bp, &blen, 1);
		fsid = exp->ex_dev;
		qword_addhex(&bp, &blen, (char*)&fsid, 4);
		if (export) {
			qword_addint(&bp, &blen, 0x7fffffff);
			qword_add(&bp, &blen, exp->ex_path);
		} else
			qword_addint(&bp, &blen, 1);
		qword_addeol(&bp, &blen);
		if (blen <= 0 || write(f, buf, bp - buf) != bp - buf)
			err = -1;
	}

	bp = buf; blen = sizeof(buf);
	qword_add(&bp, &blen, exp->ex_client);
	qword_addint(&bp, &blen, 0);
	dev = htons(major(stb.st_dev)); memcpy(fsidstr, &dev, 2);
	dev = htons(minor(stb.st_dev)); memcpy(fsidstr+2, &dev, 2);
	inode = stb.st_ino; memcpy(fsidstr+4, &inode, 4);
	
	qword_addhex(&bp, &blen, fsidstr, 8);
	if (export) {
		qword_addint(&bp, &blen, 0x7fffffff);
		qword_add(&bp, &blen, exp->ex_path);
	} else
		qword_addint(&bp, &blen, 1);
	qword_addeol(&bp, &blen);
	if (blen <= 0 || write(f, buf, bp - buf) != bp - buf)
		err = -1;
	close(f);

	return err;
}

int
nfsexport(struct nfsctl_export *exp)
{
	struct nfsctl_arg	arg;
	int fd;
	if ((fd=open("/proc/net/rpc/nfsd.fh/channel", O_WRONLY))>= 0) {
		close(fd);
		return exp_unexp(exp, 1);
	}
	arg.ca_version = NFSCTL_VERSION;
	memcpy(&arg.ca_export, exp, sizeof(arg.ca_export));
	return nfsctl(NFSCTL_EXPORT, &arg, NULL);
}

int
nfsunexport(struct nfsctl_export *exp)
{
	struct nfsctl_arg	arg;

	int fd;
	if ((fd=open("/proc/net/rpc/nfsd.fh/channel", O_WRONLY))>= 0) {
		close(fd);
		return exp_unexp(exp, 0);
	}

	arg.ca_version = NFSCTL_VERSION;
	memcpy(&arg.ca_export, exp, sizeof(arg.ca_export));
	return nfsctl(NFSCTL_UNEXPORT, &arg, NULL);
}
