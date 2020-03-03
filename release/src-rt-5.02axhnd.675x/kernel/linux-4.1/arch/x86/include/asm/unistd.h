#ifndef _ASM_X86_UNISTD_H
#define _ASM_X86_UNISTD_H 1

#include <uapi/asm/unistd.h>


# ifdef CONFIG_X86_X32_ABI
#  define __SYSCALL_MASK (~(__X32_SYSCALL_BIT))
# else
#  define __SYSCALL_MASK (~0)
# endif

# ifdef CONFIG_X86_32

#  include <asm/unistd_32.h>
#  define __ARCH_WANT_STAT64
#  define __ARCH_WANT_SYS_IPC
#  define __ARCH_WANT_SYS_OLD_MMAP
#  define __ARCH_WANT_SYS_OLD_SELECT

# else

#  include <asm/unistd_64.h>
#  include <asm/unistd_64_x32.h>
#  define __ARCH_WANT_COMPAT_SYS_TIME
#  define __ARCH_WANT_COMPAT_SYS_GETDENTS64
#  define __ARCH_WANT_COMPAT_SYS_PREADV64
#  define __ARCH_WANT_COMPAT_SYS_PWRITEV64

# endif

# define __ARCH_WANT_OLD_READDIR
# define __ARCH_WANT_OLD_STAT
# define __ARCH_WANT_SYS_ALARM
# define __ARCH_WANT_SYS_FADVISE64
# define __ARCH_WANT_SYS_GETHOSTNAME
# define __ARCH_WANT_SYS_GETPGRP
# define __ARCH_WANT_SYS_LLSEEK
# define __ARCH_WANT_SYS_NICE
# define __ARCH_WANT_SYS_OLDUMOUNT
# define __ARCH_WANT_SYS_OLD_GETRLIMIT
# define __ARCH_WANT_SYS_OLD_UNAME
# define __ARCH_WANT_SYS_PAUSE
# define __ARCH_WANT_SYS_SIGNAL
# define __ARCH_WANT_SYS_SIGPENDING
# define __ARCH_WANT_SYS_SIGPROCMASK
# define __ARCH_WANT_SYS_SOCKETCALL
# define __ARCH_WANT_SYS_TIME
# define __ARCH_WANT_SYS_UTIME
# define __ARCH_WANT_SYS_WAITPID
# define __ARCH_WANT_SYS_FORK
# define __ARCH_WANT_SYS_VFORK
# define __ARCH_WANT_SYS_CLONE

#endif /* _ASM_X86_UNISTD_H */
