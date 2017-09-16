#ifndef _NSF_EXPORT_H
#define _NSF_EXPORT_H

/*
 * Important limits for the exports stuff.
 */
#define NFSCLNT_IDMAX		1024
#define NFSCLNT_ADDRMAX		16
#define NFSCLNT_KEYMAX		32

/*
 * Export flags.
 */
#define NFSEXP_READONLY		0x0001
#define NFSEXP_INSECURE_PORT	0x0002
#define NFSEXP_ROOTSQUASH	0x0004
#define NFSEXP_ALLSQUASH	0x0008
#define NFSEXP_ASYNC		0x0010
#define NFSEXP_GATHERED_WRITES	0x0020
#define NFSEXP_NOREADDIRPLUS	0x0040
/* 80, 100 unused */
#define NFSEXP_NOHIDE		0x0200
#define NFSEXP_NOSUBTREECHECK	0x0400
#define NFSEXP_NOAUTHNLM	0x0800
#define NFSEXP_FSID		0x2000
#define	NFSEXP_CROSSMOUNT	0x4000
#define NFSEXP_NOACL		0x8000 /* reserved for possible ACL related use */
#define NFSEXP_V4ROOT		0x10000
#define NFSEXP_PNFS            0x20000
/*
 * All flags supported by the kernel before addition of the
 * export_features interface:
 */
#define NFSEXP_OLDFLAGS		0x7E3F
/*
 * Flags that can vary per flavor, for kernels before addition of the
 * export_features interface:
 */
#define NFSEXP_OLD_SECINFO_FLAGS (NFSEXP_READONLY | NFSEXP_ROOTSQUASH \
					| NFSEXP_ALLSQUASH)

#endif /* _NSF_EXPORT_H */
