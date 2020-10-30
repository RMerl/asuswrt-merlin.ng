/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
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

#ifndef HAVE_STRUCT___OLD_KERNEL_STAT
#define	sys_oldfstat		printargs
#define	sys_oldstat		printargs
#endif

/* still unfinished */
#define	sys_add_key		printargs
#define	sys_fanotify_init	printargs
#define	sys_fanotify_mark	printargs
#define	sys_finit_module	printargs
#define	sys_ioperm		printargs
#define	sys_iopl		printargs
#define	sys_ioprio_get		printargs
#define	sys_ioprio_set		printargs
#define	sys_kcmp		printargs
#define	sys_kexec_load		printargs
#define	sys_keyctl		printargs
#define	sys_lookup_dcookie	printargs
#define	sys_name_to_handle_at	printargs
#define	sys_open_by_handle_at	printargs
#define	sys_request_key		printargs
#define	sys_sync_file_range	printargs
#define	sys_sync_file_range2	printargs
#define	sys_sysfs		printargs
#define	sys_vm86old		printargs
#define	sys_vm86		printargs

/* machine-specific */
#ifndef I386
#define	sys_modify_ldt		printargs
#ifndef M68K
#define	sys_get_thread_area	printargs
#define	sys_set_thread_area	printargs
#endif
#endif

/* like another call */
#define	sys_acct		sys_chdir
#define	sys_chroot		sys_chdir
#define	sys_clock_getres	sys_clock_gettime
#define	sys_delete_module	sys_open
#define	sys_dup			sys_close
#define	sys_fchdir		sys_close
#define	sys_fdatasync		sys_close
#define	sys_fsync		sys_close
#define	sys_getegid		sys_getuid
#define	sys_geteuid		sys_getuid
#define	sys_getgid		sys_getuid
#define	sys_getresgid		sys_getresuid
#define	sys_lstat		sys_stat
#define	sys_lstat64		sys_stat64
#define	sys_mlock		sys_munmap
#define	sys_mq_unlink		sys_chdir
#define	sys_munlock		sys_munmap
#define	sys_oldlstat		sys_oldstat
#define	sys_pivotroot		sys_link
#define	sys_rename		sys_link
#define	sys_rmdir		sys_chdir
#define	sys_sched_get_priority_max	sys_sched_get_priority_min
#define	sys_set_robust_list	sys_munmap
#define	sys_setfsgid		sys_setfsuid
#define	sys_setgid		sys_setuid
#define	sys_setns		sys_inotify_rm_watch
#define	sys_setregid		sys_setreuid
#define	sys_setresgid		sys_setresuid
#define	sys_swapoff		sys_chdir
#define	sys_symlink		sys_link
#define	sys_syncfs		sys_close
#define	sys_umount		sys_chdir
#define	sys_unlink		sys_chdir
#define	sys_uselib		sys_chdir

/* printargs does the right thing */
#define	sys_getpgid		printargs
#define	sys_getpid		printargs
#define	sys_getppid		printargs
#define	sys_gettid		printargs
#define	sys_idle		printargs
#define	sys_inotify_init	printargs
#define	sys_munlockall		printargs
#define	sys_pause		printargs
#define	sys_rt_sigreturn	printargs
#define	sys_sched_yield		printargs
#define	sys_setsid		printargs
#define	sys_set_tid_address	printargs
#define	sys_setup		printargs
#define	sys_sync		printargs
#define	sys_timer_delete	printargs
#define	sys_timer_getoverrun	printargs
#define	sys_vhangup		printargs

/* printargs_lu/ld does the right thing */
#define	sys_alarm		printargs_lu
#define	sys_getpgrp		printargs_lu
#define	sys_getsid		printargs_lu
#define	sys_nice		printargs_ld
#define	sys_setpgid		printargs_lu
#define	sys_setpgrp		printargs_lu

/* unimplemented */
#define	sys_afs_syscall		printargs
#define	sys_break		printargs
#define	sys_ftime		printargs
#define	sys_get_kernel_syms	printargs
#define	sys_gtty		printargs
#define	sys_lock		printargs
#define	sys_mpx			printargs
#define	sys_nfsservctl		printargs
#define	sys_phys		printargs
#define	sys_profil		printargs
#define	sys_prof		printargs
#define	sys_security		printargs
#define	sys_stty		printargs
#define	sys_tuxcall		printargs
#define	sys_ulimit		printargs
#define	sys_ustat		printargs
#define	sys_vserver		printargs

/* deprecated */
#define	sys_bdflush		printargs
#define	sys_oldolduname		printargs
#define	sys_olduname		printargs

/* no library support */
#ifndef HAVE_SENDMSG
#define	sys_recvmsg		printargs
#define	sys_sendmsg		printargs
#endif

/* Who has STREAMS syscalls?
 * Linux hasn't. Solaris has (had?).
 * Just in case I miss something, retain in for Sparc.
 * Note: SYS_get/putpmsg may be defined even though syscalls
 * return ENOSYS. Can't just check defined(SYS_getpmsg).
 */
#if (!defined(SPARC) && !defined(SPARC64)) || !defined(SYS_getpmsg)
#define	sys_getpmsg		printargs
#endif
#if (!defined(SPARC) && !defined(SPARC64)) || !defined(SYS_putpmsg)
#define	sys_putpmsg		printargs
#endif
