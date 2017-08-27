/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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
 *
 *	$Id$
 */
#include "dummy.h"

int	sys_restart_syscall();
int	sys_nosys();
int	sys_nullsys();
int	sys_errsys();

/* 1.1 processes and protection */
int	sys_gethostid(),sys_sethostname(),sys_gethostname(),sys_getpid();
int	sys_setdomainname(),sys_getdomainname();
int	sys_fork(),sys_clone(),sys_exit(),sys_execv(),sys_execve(),sys_wait4(),sys_waitpid();
int	sys_setuid(),sys_setgid(),sys_getuid(),sys_setreuid(),sys_getgid(),sys_getgroups(),sys_setregid(),sys_setgroups();
int	sys_getpgrp(),sys_setpgrp();
int	sys_setsid(), sys_setpgid();
int	sys_uname(), sys_sysinfo();

/* 1.2 memory management */
int	sys_brk(),sys_sbrk(),sys_sstk();
int	sys_getpagesize(),sys_mmap(),sys_mctl(),sys_munmap(),sys_mprotect(),sys_mincore(), sys_mremap();
int	sys_omsync(),sys_omadvise(), sys_madvise(),sys_mlockall();

/* 1.3 signals */
int	sys_sigvec(),sys_sigblock(),sys_sigsetmask(),sys_sigpause(),sys_sigstack(),sys_sigcleanup(), sys_sigreturn();
int	sys_kill(), sys_killpg(), sys_sigpending(), sys_signal(), sys_sigaction(), sys_sigsuspend(), sys_sigprocmask();

/* 1.4 timing and statistics */
int	sys_gettimeofday(),sys_settimeofday();
int	sys_adjtime(), sys_adjtimex();
int	sys_getitimer(),sys_setitimer();

/* 1.5 descriptors */
int	sys_getdtablesize(),sys_dup(),sys_dup2(),sys_close();
int	sys_oldselect(),sys_select(),sys_getdopt(),sys_setdopt(),sys_fcntl(),sys_flock();
int	sys_epoll_create(), sys_epoll_ctl(), sys_epoll_wait();

/* 1.6 resource controls */
int	sys_getpriority(),sys_setpriority(),sys_getrusage(),sys_getrlimit(),sys_setrlimit();
int	sys_oldquota(), sys_quotactl();
int	sys_rtschedule(), sys_personality();

/* 1.7 system operation support */
int	sys_mount(),sys_unmount(),sys_swapon(),sys_pivotroot();
int	sys_sync(),sys_reboot();
int	sys_sysacct();
int	sys_auditsys();

/* 2.1 generic operations */
int	sys_read(),sys_write(),sys_readv(),sys_writev(),sys_ioctl();

/* 2.1.1 asynch operations */
int	sys_aioread(), sys_aiowrite(), sys_aiowait(), sys_aiocancel();

/* 2.2 file system */
int	sys_chdir(),sys_chroot();
int	sys_fchdir(),sys_fchroot();
int	sys_mkdir(),sys_rmdir(),sys_getdirentries();
int	sys_getdents(), sys_getdents64(), sys_readdir();
int	sys_creat(),sys_open(),sys_mknod(),sys_unlink(),sys_stat(),sys_fstat(),sys_lstat();
int	sys_chown(),sys_fchown(),sys_chmod(),sys_fchmod(),sys_utimes();
int	sys_link(),sys_symlink(),sys_readlink(),sys_rename();
int	sys_lseek(), sys_llseek();
int	sys_truncate(),sys_ftruncate(),sys_access(),sys_fsync(),sys_sysctl();
int	sys_statfs(),sys_fstatfs(),sys_msync();
int sys_stat64(), sys_lstat64(), sys_fstat64();
int sys_truncate64(), sys_ftruncate64();
int sys_semtimedop();

/* 2.3 communications */
int	sys_socket(),sys_bind(),sys_listen(),sys_accept(),sys_connect();
int	sys_socketpair(),sys_sendto(),sys_send(),sys_recvfrom(),sys_recv();
int	sys_sendmsg(),sys_recvmsg(),sys_shutdown(),sys_setsockopt(),sys_getsockopt();
int	sys_getsockname(),sys_getpeername(),sys_pipe(),sys_accept4();
int	sys_recvmmsg();

int sys_setresuid(), sys_setresgid(), sys_getresuid(), sys_getresgid(), sys_pread();
int sys_pwrite(), sys_getcwd();
int sys_sigaltstack(), sys_rt_sigprocmask(), sys_rt_sigaction();
int sys_rt_sigpending(), sys_rt_sigsuspend(), sys_rt_sigqueueinfo();
int sys_rt_sigtimedwait(), sys_prctl(), sys_poll();
int sys_sendfile(), sys_query_module(), sys_capget(), sys_capset();
int sys_create_module(), sys_init_module();
int sys_setgroups32(), sys_getgroups32();

int	sys_umask();		/* XXX */

int sys_sched_setparam(), sys_sched_getparam();
int sys_sched_setscheduler(), sys_sched_getscheduler(), sys_sched_yield();
int sys_sched_get_priority_max(), sys_sched_get_priority_min();

/* 2.3.1 SystemV-compatible IPC */
int	sys_semsys(), sys_semctl(), sys_semget();
#define SYS_semsys_subcall	200
#define SYS_semsys_nsubcalls	3
#define SYS_semctl		(SYS_semsys_subcall + 0)
#define SYS_semget		(SYS_semsys_subcall + 1)
#define SYS_semop		(SYS_semsys_subcall + 2)
int	sys_msgsys(), sys_msgget(), sys_msgctl(), sys_msgrcv(), sys_msgsnd();
#define SYS_msgsys_subcall	203
#define SYS_msgsys_nsubcalls	4
#define SYS_msgget		(SYS_msgsys_subcall + 0)
#define SYS_msgctl		(SYS_msgsys_subcall + 1)
#define SYS_msgrcv		(SYS_msgsys_subcall + 2)
#define SYS_msgsnd		(SYS_msgsys_subcall + 3)
int	sys_shmsys(), sys_shmat(), sys_shmctl(), sys_shmdt(), sys_shmget();
#define SYS_shmsys_subcall	207
#define SYS_shmsys_nsubcalls	4
#define	SYS_shmat		(SYS_shmsys_subcall + 0)
#define SYS_shmctl		(SYS_shmsys_subcall + 1)
#define SYS_shmdt		(SYS_shmsys_subcall + 2)
#define SYS_shmget		(SYS_shmsys_subcall + 3)

/* 2.4 processes */
int	sys_ptrace();

/* 2.5 terminals */

/* emulations for backwards compatibility */
int	sys_otime();		/* now use gettimeofday */
int	sys_ostime();		/* now use settimeofday */
int	sys_oalarm();		/* now use setitimer */
int	sys_outime();		/* now use utimes */
int	sys_opause();		/* now use sigpause */
int	sys_onice();		/* now use setpriority,getpriority */
int	sys_oftime();		/* now use gettimeofday */
int	sys_osetpgrp();		/* ??? */
int	sys_otimes();		/* now use getrusage */
int	sys_ossig();		/* now use sigvec, etc */
int	sys_ovlimit();		/* now use setrlimit,getrlimit */
int	sys_ovtimes();		/* now use getrusage */
int	sys_osetuid();		/* now use setreuid */
int	sys_osetgid();		/* now use setregid */
int	sys_ostat();		/* now use stat */
int	sys_ofstat();		/* now use fstat */

/* BEGIN JUNK */
int	sys_profil();		/* 'cuz sys calls are interruptible */
int	sys_vhangup();		/* should just do in sys_exit() */
int	sys_vfork();		/* XXX - was awaiting fork w/ copy on write */
int	sys_ovadvise();		/* awaiting new madvise */
int	sys_indir();		/* indirect system call */
int	sys_ustat();		/* System V compatibility */
int	sys_owait();		/* should use wait4 interface */
int	sys_owait3();		/* should use wait4 interface */
int	sys_umount();		/* still more Sys V (and 4.2?) compatibility */
int	sys_umount2();
int	sys_pathconf();		/* posix */
int	sys_fpathconf();		/* posix */
int	sys_sysconf();		/* posix */
int     sys_delete_module();
int sys_debug();
/* END JUNK */

int	sys_vtrace();		/* kernel event tracing */

/* nfs */
int	sys_async_daemon();		/* client async daemon */
int	sys_nfs_svc();		/* run nfs server */
int	sys_nfs_getfh();		/* get file handle */
int	sys_exportfs();		/* export file systems */

int  	sys_rfssys();		/* RFS-related calls */

int	sys_getmsg();
int	sys_putmsg();
int	sys_poll();

int	sys_vpixsys();		/* VP/ix system calls */

int	sys_sendfile64(), sys_futex(), sys_gettid(), sys_sched_setaffinity();
int	sys_sched_getaffinity(), sys_setxattr(), sys_lsetxattr();
int	sys_fsetxattr(), sys_getxattr(), sys_lgetxattr(), sys_fgetxattr();
int	sys_listxattr(), sys_llistxattr(), sys_flistxattr();
int	sys_removexattr(), sys_lremovexattr(), sys_fremovexattr();
int	sys_remap_file_pages(), sys_readahead(), sys_tgkill(), sys_statfs64();
int	sys_fstatfs64(), sys_clock_settime(), sys_clock_gettime();
int	sys_clock_getres(), sys_clock_nanosleep(), sys_nanosleep();
int	sys_timer_create(), sys_timer_settime(), sys_timer_gettime();

int	sys_io_setup(), sys_io_destroy(), sys_io_submit(), sys_io_cancel(), sys_io_getevents();
int	sys_mq_open(), sys_mq_unlink(), sys_mq_timedsend(), sys_mq_timedreceive(), sys_mq_notify(), sys_mq_getsetattr();
int	sys_waitid();
int sys_mbind(), sys_get_mempolicy(), sys_set_mempolicy();
int sys_utimensat();
int sys_fallocate(), sys_timerfd_create(), sys_timerfd_settime(), sys_timerfd_gettime();
int	sys_openat(), sys_mkdirat(), sys_mknodat(), sys_fchownat(), sys_futimesat(), sys_newfstatat(), sys_unlinkat(), sys_renameat(), sys_linkat(), sys_symlinkat(), sys_readlinkat(), sys_fchmodat(),	sys_faccessat();
int	sys_pselect6(), sys_ppoll();
int	sys_unshare();
int	sys_move_pages(), sys_getcpu();
int	sys_epoll_pwait();
int	sys_signalfd(), sys_timerfd(), sys_eventfd();
int sys_signalfd4(), sys_eventfd2(), sys_epoll_create1(), sys_dup3(), sys_pipe2();
int sys_inotify_init1();

#  define SYS_socket_subcall	353
#define SYS_sub_socket		(SYS_socket_subcall + 1)
#define SYS_sub_bind		(SYS_socket_subcall + 2)
#define SYS_sub_connect		(SYS_socket_subcall + 3)
#define SYS_sub_listen		(SYS_socket_subcall + 4)
#define SYS_sub_accept		(SYS_socket_subcall + 5)
#define SYS_sub_getsockname	(SYS_socket_subcall + 6)
#define SYS_sub_getpeername	(SYS_socket_subcall + 7)
#define SYS_sub_socketpair	(SYS_socket_subcall + 8)
#define SYS_sub_send		(SYS_socket_subcall + 9)
#define SYS_sub_recv		(SYS_socket_subcall + 10)
#define SYS_sub_sendto		(SYS_socket_subcall + 11)
#define SYS_sub_recvfrom	(SYS_socket_subcall + 12)
#define SYS_sub_shutdown	(SYS_socket_subcall + 13)
#define SYS_sub_setsockopt	(SYS_socket_subcall + 14)
#define SYS_sub_getsockopt	(SYS_socket_subcall + 15)
#define SYS_sub_sendmsg		(SYS_socket_subcall + 16)
#define SYS_sub_recvmsg		(SYS_socket_subcall + 17)
#define SYS_sub_accept4		(SYS_socket_subcall + 18)
#define SYS_sub_recvmmsg	(SYS_socket_subcall + 19)

#define SYS_socket_nsubcalls	20

#define SYS_ipc_subcall		((SYS_socket_subcall)+(SYS_socket_nsubcalls))
#define SYS_sub_semop		(SYS_ipc_subcall + 1)
#define SYS_sub_semget		(SYS_ipc_subcall + 2)
#define SYS_sub_semctl		(SYS_ipc_subcall + 3)
#define SYS_sub_semtimedop	(SYS_ipc_subcall + 4)
#define SYS_sub_msgsnd		(SYS_ipc_subcall + 11)
#define SYS_sub_msgrcv		(SYS_ipc_subcall + 12)
#define SYS_sub_msgget		(SYS_ipc_subcall + 13)
#define SYS_sub_msgctl		(SYS_ipc_subcall + 14)
#define SYS_sub_shmat		(SYS_ipc_subcall + 21)
#define SYS_sub_shmdt		(SYS_ipc_subcall + 22)
#define SYS_sub_shmget		(SYS_ipc_subcall + 23)
#define SYS_sub_shmctl		(SYS_ipc_subcall + 24)

#define SYS_ipc_nsubcalls	25

#include "syscall1.h"
