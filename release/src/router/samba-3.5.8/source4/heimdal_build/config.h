/*
  this is a replacement config.h for building the heimdal parts of the
  Samba source tree
*/

#ifndef HAVE_HEIMDAL_CONFIG_H
#define HAVE_HEIMDAL_CONFIG_H

#include "include/config.h"
#include "../replace/replace.h"

#if !defined(HAVE_DIRFD) && !defined(HAVE_DIRFD_DECL) && !defined(dirfd)
#define dirfd(d) (-1)
#endif

#define RCSID(msg) struct __rcsid { int __rcsdi; }
#define KRB5

/* This needs to be defined for roken too */
#ifdef VOID_RETSIGTYPE
#define SIGRETURN(x) return
#else
#define SIGRETURN(x) return (RETSIGTYPE)(x)
#endif

#define HDB_DB_DIR ""

#undef HAVE_KRB5_ENCRYPT_BLOCK

#if defined(UID_WRAPPER)
#if !defined(UID_WRAPPER_REPLACE) && !defined(UID_WRAPPER_NOT_REPLACE)
#define UID_WRAPPER_REPLACE
#include "../uid_wrapper/uid_wrapper.h"
#endif
#else
#define uwrap_enabled() 0
#endif

#endif
