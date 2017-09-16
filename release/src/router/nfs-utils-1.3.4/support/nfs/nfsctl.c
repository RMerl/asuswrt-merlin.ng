/*
 * support/nfs/nfsctl.c
 *
 * Central syscall to the nfsd kernel module.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <asm/unistd.h>
#include "nfslib.h"

/* compatibility hack... */
#if !defined(__NR_nfsctl) && defined(__NR_nfsservctl)
#define __NR_nfsctl	__NR_nfsservctl
#endif

int
nfsctl (int cmd, struct nfsctl_arg * argp, union nfsctl_res * resp)
{
#ifdef __NR_nfsctl
  return syscall (__NR_nfsctl, cmd, argp, resp);
#else
  errno = ENOSYS;
  return -1;
#endif
}
