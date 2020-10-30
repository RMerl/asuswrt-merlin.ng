/*
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
 */

	{ 6,	0,	solaris_syscall,	"syscall"	}, /* 0 */
	{ 6,	TP,	solaris_exit,		"_exit"		}, /* 1 */
	{ 6,	TP,	solaris_fork,		"fork"		}, /* 2 */
	{ 6,	0,	solaris_read,		"read"		}, /* 3 */
	{ 6,	0,	solaris_write,		"write"		}, /* 4 */
	{ 6,	TF,	solaris_open,		"open"		}, /* 5 */
	{ 6,	0,	solaris_close,		"close"		}, /* 6 */
	{ 6,	TP,	solaris_wait,		"wait"		}, /* 7 */
	{ 6,	TF,	solaris_creat,		"creat"		}, /* 8 */
	{ 6,	TF,	solaris_link,		"link"		}, /* 9 */
	{ 6,	TF,	solaris_unlink,		"unlink"	}, /* 10 */
	{ 6,	TF|TP,	solaris_exec,		"exec"		}, /* 11 */
	{ 6,	TF,	solaris_chdir,		"chdir"		}, /* 12 */
	{ 6,	0,	solaris_time,		"time"		}, /* 13 */
	{ 6,	TF,	solaris_mknod,		"mknod"		}, /* 14 */
	{ 6,	TF,	solaris_chmod,		"chmod"		}, /* 15 */
	{ 6,	TF,	solaris_chown,		"chown"		}, /* 16 */
	{ 6,	0,	solaris_brk,		"brk"		}, /* 17 */
	{ 6,	TF,	solaris_stat,		"stat"		}, /* 18 */
	{ 6,	0,	solaris_lseek,		"lseek"		}, /* 19 */
	{ 6,	0,	solaris_getpid,		"getpid"	}, /* 20 */
	{ 6,	TF,	solaris_mount,		"mount"		}, /* 21 */
	{ 6,	TF,	solaris_umount,		"umount"	}, /* 22 */
	{ 6,	0,	solaris_setuid,		"setuid"	}, /* 23 */
	{ 6,	0,	solaris_getuid,		"getuid"	}, /* 24 */
	{ 6,	0,	solaris_stime,		"stime"		}, /* 25 */
	{ 6,	0,	solaris_ptrace,		"ptrace"	}, /* 26 */
	{ 6,	0,	solaris_alarm,		"alarm"		}, /* 27 */
	{ 6,	0,	solaris_fstat,		"fstat"		}, /* 28 */
	{ 6,	TS,	solaris_pause,		"pause"		}, /* 29 */
	{ 6,	TF,	solaris_utime,		"utime"		}, /* 30 */
	{ 6,	0,	solaris_stty,		"stty"		}, /* 31 */
	{ 6,	0,	solaris_gtty,		"gtty"		}, /* 32 */
	{ 6,	TF,	solaris_access,		"access"	}, /* 33 */
	{ 6,	0,	solaris_nice,		"nice"		}, /* 34 */
	{ 6,	TF,	solaris_statfs,		"statfs"	}, /* 35 */
	{ 6,	0,	solaris_sync,		"sync"		}, /* 36 */
	{ 6,	TS,	solaris_kill,		"kill"		}, /* 37 */
	{ 6,	0,	solaris_fstatfs,	"fstatfs"	}, /* 38 */
	{ 6,	0,	solaris_pgrpsys,	"pgrpsys"	}, /* 39 */
	{ 6,	0,	solaris_xenix,		"xenix"		}, /* 40 */
	{ 6,	0,	solaris_dup,		"dup"		}, /* 41 */
	{ 6,	0,	solaris_pipe,		"pipe"		}, /* 42 */
	{ 6,	0,	solaris_times,		"times"		}, /* 43 */
	{ 6,	0,	solaris_profil,		"profil"	}, /* 44 */
	{ 6,	0,	solaris_plock,		"plock"		}, /* 45 */
	{ 6,	0,	solaris_setgid,		"setgid"	}, /* 46 */
	{ 6,	0,	solaris_getgid,		"getgid"	}, /* 47 */
	{ 6,	0,	solaris_sigcall,	"sigcall"	}, /* 48 */
	{ 6,	TI,	solaris_msgsys,		"msgsys"	}, /* 49 */
	{ 6,	0,	solaris_syssun,		"syssun"	}, /* 50 */
	{ 6,	TF,	solaris_acct,		"acct"		}, /* 51 */
	{ 6,	TI,	solaris_shmsys,		"shmsys"	}, /* 52 */
	{ 6,	TI,	solaris_semsys,		"semsys"	}, /* 53 */
	{ 6,	0,	solaris_ioctl,		"ioctl"		}, /* 54 */
	{ 6,	0,	solaris_uadmin,		"uadmin"	}, /* 55 */
	{ 6,	0,	solaris_sysmp,		"sysmp"		}, /* 56 */
	{ 6,	0,	solaris_utssys,		"utssys"	}, /* 57 */
	{ 6,	0,	solaris_fdsync,		"fdsync"	}, /* 58 */
	{ 6,	TF|TP,	solaris_execve,		"execve"	}, /* 59 */
	{ 6,	0,	solaris_umask,		"umask"		}, /* 60 */
	{ 6,	TF,	solaris_chroot,		"chroot"	}, /* 61 */
	{ 6,	0,	solaris_fcntl,		"fcntl"		}, /* 62 */
	{ 6,	0,	solaris_ulimit,		"ulimit"	}, /* 63 */
	{ 6,	0,	NULL,			NULL		}, /* 64 */
	{ 6,	0,	NULL,			NULL		}, /* 65 */
	{ 6,	0,	NULL,			NULL		}, /* 66 */
	{ 6,	0,	NULL,			NULL		}, /* 67 */
	{ 6,	0,	NULL,			NULL		}, /* 68 */
	{ 6,	0,	NULL,			NULL		}, /* 69 */
	{ 6,	0,	NULL,			NULL		}, /* 70 */
	{ 6,	0,	NULL,			NULL		}, /* 71 */
	{ 6,	0,	NULL,			NULL		}, /* 72 */
	{ 6,	0,	NULL,			NULL		}, /* 73 */
	{ 6,	0,	NULL,			NULL		}, /* 74 */
	{ 6,	0,	NULL,			NULL		}, /* 75 */
	{ 6,	0,	NULL,			NULL		}, /* 76 */
	{ 6,	0,	NULL,			NULL		}, /* 77 */
	{ 6,	0,	NULL,			NULL		}, /* 78 */
	{ 6,	TF,	solaris_rmdir,		"rmdir"		}, /* 79 */
	{ 6,	TF,	solaris_mkdir,		"mkdir"		}, /* 80 */
	{ 6,	0,	solaris_getdents,	"getdents"	}, /* 81 */
	{ 6,	0,	solaris_sginap,		"sginap"	}, /* 82 */
	{ 6,	0,	solaris_sgikopt,	"sgikopt"	}, /* 83 */
	{ 6,	0,	solaris_sysfs,		"sysfs"		}, /* 84 */
	{ 6,	TN,	sys_getmsg,		"getmsg"	}, /* 85 */
	{ 6,	TN,	sys_putmsg,		"putmsg"	}, /* 86 */
	{ 6,	TN,	solaris_poll,		"poll"		}, /* 87 */
	{ 6,	TF,	solaris_lstat,		"lstat"		}, /* 88 */
	{ 6,	TF,	solaris_symlink,	"symlink"	}, /* 89 */
	{ 6,	TF,	solaris_readlink,	"readlink"	}, /* 90 */
	{ 6,	0,	solaris_setgroups,	"setgroups"	}, /* 91 */
	{ 6,	0,	solaris_getgroups,	"getgroups"	}, /* 92 */
	{ 6,	0,	solaris_fchmod,		"fchmod"	}, /* 93 */
	{ 6,	0,	solaris_fchown,		"fchown"	}, /* 94 */
	{ 6,	TS,	solaris_sigprocmask,	"sigprocmask"	}, /* 95 */
	{ 6,	TS,	solaris_sigsuspend,	"sigsuspend"	}, /* 96 */
	{ 6,	TS,	solaris_sigaltstack,	"sigaltstack"	}, /* 97 */
	{ 6,	TS,	solaris_sigaction,	"sigaction"	}, /* 98 */
	{ 6,	0,	solaris_spcall,		"spcall"	}, /* 99 */
	{ 6,	0,	solaris_context,	"context"	}, /* 100 */
	{ 6,	0,	solaris_evsys,		"evsys"		}, /* 101 */
	{ 6,	0,	solaris_evtrapret,	"evtrapret"	}, /* 102 */
	{ 6,	TF,	solaris_statvfs,	"statvfs"	}, /* 103 */
	{ 6,	0,	solaris_fstatvfs,	"fstatvfs"	}, /* 104 */
	{ 6,	0,	NULL,			NULL		}, /* 105 */
	{ 6,	0,	solaris_nfssys,		"nfssys"	}, /* 106 */
	{ 6,	TP,	solaris_waitid,		"waitid"	}, /* 107 */
	{ 6,	0,	solaris_sigsendsys,	"sigsendsys"	}, /* 108 */
	{ 6,	0,	solaris_hrtsys,		"hrtsys"	}, /* 109 */
	{ 6,	0,	solaris_acancel,	"acancel"	}, /* 110 */
	{ 6,	0,	solaris_async,		"async"		}, /* 111 */
	{ 6,	0,	solaris_priocntlsys,	"priocntlsys"	}, /* 112 */
	{ 6,	TF,	solaris_pathconf,	"pathconf"	}, /* 113 */
	{ 6,	0,	solaris_mincore,	"mincore"	}, /* 114 */
	{ 6,	TD|TM,	solaris_mmap,		"mmap"		}, /* 115 */
	{ 6,	0,	solaris_mprotect,	"mprotect"	}, /* 116 */
	{ 6,	0,	solaris_munmap,		"munmap"	}, /* 117 */
	{ 6,	0,	solaris_fpathconf,	"fpathconf"	}, /* 118 */
	{ 6,	TP,	solaris_vfork,		"vfork"		}, /* 119 */
	{ 6,	0,	solaris_fchdir,		"fchdir"	}, /* 120 */
	{ 6,	0,	solaris_readv,		"readv"		}, /* 121 */
	{ 6,	0,	solaris_writev,		"writev"	}, /* 122 */
	{ 6,	TF,	solaris_xstat,		"xstat"		}, /* 123 */
	{ 6,	TF,	solaris_lxstat,		"lxstat"	}, /* 124 */
	{ 6,	0,	solaris_fxstat,		"fxstat"	}, /* 125 */
	{ 6,	TF,	solaris_xmknod,		"xmknod"	}, /* 126 */
	{ 6,	0,	solaris_clocal,		"clocal"	}, /* 127 */
	{ 6,	0,	solaris_setrlimit,	"setrlimit"	}, /* 128 */
	{ 6,	0,	solaris_getrlimit,	"getrlimit"	}, /* 129 */
	{ 6,	TF,	solaris_lchown,		"lchown"	}, /* 130 */
	{ 6,	0,	solaris_memcntl,	"memcntl"	}, /* 131 */
	{ 6,	TN,	solaris_getpmsg,	"getpmsg"	}, /* 132 */
	{ 6,	TN,	solaris_putpmsg,	"putpmsg"	}, /* 133 */
	{ 6,	TF,	solaris_rename,		"rename"	}, /* 134 */
	{ 6,	0,	solaris_uname,		"uname"		}, /* 135 */
	{ 6,	0,	solaris_setegid,	"setegid"	}, /* 136 */
	{ 6,	0,	solaris_sysconfig,	"sysconfig"	}, /* 137 */
	{ 6,	0,	solaris_adjtime,	"adjtime"	}, /* 138 */
	{ 6,	0,	solaris_sysinfo,	"sysinfo"	}, /* 139 */
	{ 6,	0,	NULL,			NULL		}, /* 140 */
	{ 6,	0,	solaris_seteuid,	"seteuid"	}, /* 141 */
	{ 6,	0,	solaris_vtrace,		"vtrace"	}, /* 142 */
	{ 6,	TP,	solaris_fork1,		"fork1"		}, /* 143 */
	{ 6,	TS,	solaris_sigtimedwait,	"sigtimedwait"	}, /* 144 */
	{ 6,	0,	solaris_lwp_info,	"lwp_info"	}, /* 145 */
	{ 6,	0,	solaris_yield,		"yield"		}, /* 146 */
	{ 6,	0,	solaris_lwp_sema_wait,	"lwp_sema_wait"	}, /* 147 */
	{ 6,	0,	solaris_lwp_sema_post,	"lwp_sema_post"	}, /* 148 */
	{ 6,	0,	NULL,			NULL		}, /* 149 */
	{ 6,	0,	NULL,			NULL		}, /* 150 */
	{ 6,	0,	NULL,			NULL		}, /* 151 */
	{ 6,	0,	solaris_modctl,		"modctl"	}, /* 152 */
	{ 6,	0,	solaris_fchroot,	"fchroot"	}, /* 153 */
	{ 6,	TF,	solaris_utimes,		"utimes"	}, /* 154 */
	{ 6,	0,	solaris_vhangup,	"vhangup"	}, /* 155 */
	{ 6,	0,	solaris_gettimeofday,	"gettimeofday"	}, /* 156 */
	{ 6,	0,	solaris_getitimer,	"getitimer"	}, /* 157 */
	{ 6,	0,	solaris_setitimer,	"setitimer"	}, /* 158 */
	{ 6,	0,	solaris_lwp_create,	"lwp_create"	}, /* 159 */
	{ 6,	0,	solaris_lwp_exit,	"lwp_exit"	}, /* 160 */
	{ 6,	0,	solaris_lwp_suspend,	"lwp_suspend"	}, /* 161 */
	{ 6,	0,	solaris_lwp_continue,	"lwp_continue"	}, /* 162 */
	{ 6,	0,	solaris_lwp_kill,	"lwp_kill"	}, /* 163 */
	{ 6,	0,	solaris_lwp_self,	"lwp_self"	}, /* 164 */
	{ 6,	0,	solaris_lwp_setprivate,	"lwp_setprivate"}, /* 165 */
	{ 6,	0,	solaris_lwp_getprivate,	"lwp_getprivate"}, /* 166 */
	{ 6,	0,	solaris_lwp_wait,	"lwp_wait"	}, /* 167 */
	{ 6,	0,	solaris_lwp_mutex_unlock,"lwp_mutex_unlock"}, /* 168 */
	{ 6,	0,	solaris_lwp_mutex_lock,	"lwp_mutex_lock"}, /* 169 */
	{ 6,	0,	solaris_lwp_cond_wait,	"lwp_cond_wait"}, /* 170 */
	{ 6,	0,	solaris_lwp_cond_signal,"lwp_cond_signal"}, /* 171 */
	{ 6,	0,	solaris_lwp_cond_broadcast,"lwp_cond_broadcast"}, /* 172 */
	{ 6,	0,	solaris_pread,		"pread"		}, /* 173 */
	{ 6,	0,	solaris_pwrite,		"pwrite"	}, /* 174 */
	{ 6,	0,	solaris_llseek,		"llseek"	}, /* 175 */
	{ 6,	0,	solaris_inst_sync,	"inst_sync"	}, /* 176 */
	{ 6,	0,	NULL,			NULL		}, /* 177 */
	{ 6,	0,	NULL,			NULL		}, /* 178 */
	{ 6,	0,	NULL,			NULL		}, /* 179 */
	{ 6,	0,	NULL,			NULL		}, /* 180 */
	{ 6,	0,	NULL,			NULL		}, /* 181 */
	{ 6,	0,	NULL,			NULL		}, /* 182 */
	{ 6,	0,	NULL,			NULL		}, /* 183 */
	{ 6,	0,	NULL,			NULL		}, /* 184 */
	{ 6,	0,	NULL,			NULL		}, /* 185 */
	{ 6,	0,	solaris_auditsys,	"auditsys"	}, /* 186 */
	{ 6,	0,	solaris_processor_bind,	"processor_bind"}, /* 187 */
	{ 6,	0,	solaris_processor_info,	"processor_info"}, /* 188 */
	{ 6,	0,	solaris_p_online,	"p_online"	}, /* 189 */
	{ 6,	0,	solaris_sigqueue,	"sigqueue"	}, /* 190 */
	{ 6,	0,	solaris_clock_gettime,	"clock_gettime"	}, /* 191 */
	{ 6,	0,	solaris_clock_settime,	"clock_settime"	}, /* 192 */
	{ 6,	0,	solaris_clock_getres,	"clock_getres"	}, /* 193 */
	{ 6,	0,	solaris_timer_create,	"timer_create"	}, /* 194 */
	{ 6,	0,	solaris_timer_delete,	"timer_delete"	}, /* 195 */
	{ 6,	0,	solaris_timer_settime,	"timer_settime"	}, /* 196 */
	{ 6,	0,	solaris_timer_gettime,	"timer_gettime"	}, /* 197 */
	{ 6,	0,	solaris_timer_getoverrun,"timer_getoverrun"}, /* 198 */
	{ 6,	0,	solaris_nanosleep,	"nanosleep"	}, /* 199 */
	{ 6,	0,	NULL,			NULL		}, /* 200 */
	{ 6,	0,	NULL,			NULL		}, /* 201 */
	{ 6,	0,	NULL,			NULL		}, /* 202 */
	{ 6,	0,	NULL,			NULL		}, /* 203 */
	{ 6,	0,	NULL,			NULL		}, /* 204 */
	{ 6,	0,	NULL,			NULL		}, /* 205 */
	{ 6,	0,	NULL,			NULL		}, /* 206 */
	{ 6,	0,	NULL,			NULL		}, /* 207 */
	{ 6,	0,	NULL,			NULL		}, /* 208 */
	{ 6,	0,	NULL,			NULL		}, /* 209 */
	{ 6,	0,	NULL,			NULL		}, /* 210 */
	{ 6,	0,	NULL,			NULL		}, /* 211 */
	{ 6,	0,	NULL,			NULL		}, /* 212 */
	{ 6,	0,	NULL,			NULL		}, /* 213 */
	{ 6,	0,	NULL,			NULL		}, /* 214 */
	{ 6,	0,	NULL,			NULL		}, /* 215 */
	{ 6,	0,	NULL,			NULL		}, /* 216 */
	{ 6,	0,	NULL,			NULL		}, /* 217 */
	{ 6,	0,	NULL,			NULL		}, /* 218 */
	{ 6,	0,	NULL,			NULL		}, /* 219 */
	{ 6,	0,	NULL,			NULL		}, /* 220 */
	{ 6,	0,	NULL,			NULL		}, /* 221 */
	{ 6,	0,	NULL,			NULL		}, /* 222 */
	{ 6,	0,	NULL,			NULL		}, /* 223 */
	{ 6,	0,	NULL,			NULL		}, /* 224 */
	{ 6,	0,	NULL,			NULL		}, /* 225 */
	{ 6,	0,	NULL,			NULL		}, /* 226 */
	{ 6,	0,	NULL,			NULL		}, /* 227 */
	{ 6,	0,	NULL,			NULL		}, /* 228 */
	{ 6,	0,	NULL,			NULL		}, /* 229 */
	{ 6,	0,	NULL,			NULL		}, /* 230 */
	{ 6,	0,	NULL,			NULL		}, /* 231 */
	{ 6,	0,	NULL,			NULL		}, /* 232 */
	{ 6,	0,	NULL,			NULL		}, /* 233 */
	{ 6,	0,	NULL,			NULL		}, /* 234 */
	{ 6,	0,	NULL,			NULL		}, /* 235 */
	{ 6,	0,	NULL,			NULL		}, /* 236 */
	{ 6,	0,	NULL,			NULL		}, /* 237 */
	{ 6,	0,	NULL,			NULL		}, /* 238 */
	{ 6,	0,	NULL,			NULL		}, /* 239 */
	{ 6,	0,	NULL,			NULL		}, /* 240 */
	{ 6,	0,	NULL,			NULL		}, /* 241 */
	{ 6,	0,	NULL,			NULL		}, /* 242 */
	{ 6,	0,	NULL,			NULL		}, /* 243 */
	{ 6,	0,	NULL,			NULL		}, /* 244 */
	{ 6,	0,	NULL,			NULL		}, /* 245 */
	{ 6,	0,	NULL,			NULL		}, /* 246 */
	{ 6,	0,	NULL,			NULL		}, /* 247 */
	{ 6,	0,	NULL,			NULL		}, /* 248 */
	{ 6,	0,	NULL,			NULL		}, /* 249 */
	{ 6,	0,	NULL,			NULL		}, /* 250 */
	{ 6,	0,	NULL,			NULL		}, /* 251 */
	{ 6,	0,	NULL,			NULL		}, /* 252 */
	{ 6,	0,	NULL,			NULL		}, /* 253 */
	{ 6,	0,	NULL,			NULL		}, /* 254 */
	{ 6,	0,	NULL,			NULL		}, /* 255 */
	{ 6,	0,	NULL,			NULL		}, /* 256 */
	{ 6,	0,	NULL,			NULL		}, /* 257 */
	{ 6,	0,	NULL,			NULL		}, /* 258 */
	{ 6,	0,	NULL,			NULL		}, /* 259 */
	{ 6,	0,	NULL,			NULL		}, /* 260 */
	{ 6,	0,	NULL,			NULL		}, /* 261 */
	{ 6,	0,	NULL,			NULL		}, /* 262 */
	{ 6,	0,	NULL,			NULL		}, /* 263 */
	{ 6,	0,	NULL,			NULL		}, /* 264 */
	{ 6,	0,	NULL,			NULL		}, /* 265 */
	{ 6,	0,	NULL,			NULL		}, /* 266 */
	{ 6,	0,	NULL,			NULL		}, /* 267 */
	{ 6,	0,	NULL,			NULL		}, /* 268 */
	{ 6,	0,	NULL,			NULL		}, /* 269 */
	{ 6,	0,	NULL,			NULL		}, /* 270 */
	{ 6,	0,	NULL,			NULL		}, /* 271 */
	{ 6,	0,	NULL,			NULL		}, /* 272 */
	{ 6,	0,	NULL,			NULL		}, /* 273 */
	{ 6,	0,	NULL,			NULL		}, /* 274 */
	{ 6,	0,	NULL,			NULL		}, /* 275 */
	{ 6,	0,	NULL,			NULL		}, /* 276 */
	{ 6,	0,	NULL,			NULL		}, /* 277 */
	{ 6,	0,	NULL,			NULL		}, /* 278 */
	{ 6,	0,	NULL,			NULL		}, /* 279 */
	{ 6,	0,	NULL,			NULL		}, /* 280 */
	{ 6,	0,	NULL,			NULL		}, /* 281 */
	{ 6,	0,	NULL,			NULL		}, /* 282 */
	{ 6,	0,	NULL,			NULL		}, /* 283 */
	{ 6,	0,	NULL,			NULL		}, /* 284 */
	{ 6,	0,	NULL,			NULL		}, /* 285 */
	{ 6,	0,	NULL,			NULL		}, /* 286 */
	{ 6,	0,	NULL,			NULL		}, /* 287 */
	{ 6,	0,	NULL,			NULL		}, /* 288 */
	{ 6,	0,	NULL,			NULL		}, /* 289 */
	{ 6,	0,	NULL,			NULL		}, /* 290 */
	{ 6,	0,	NULL,			NULL		}, /* 291 */
	{ 6,	0,	NULL,			NULL		}, /* 292 */
	{ 6,	0,	NULL,			NULL		}, /* 293 */
	{ 6,	0,	NULL,			NULL		}, /* 294 */
	{ 6,	0,	NULL,			NULL		}, /* 295 */
	{ 6,	0,	NULL,			NULL		}, /* 296 */
	{ 6,	0,	NULL,			NULL		}, /* 297 */
	{ 6,	0,	NULL,			NULL		}, /* 298 */
	{ 6,	0,	NULL,			NULL		}, /* 299 */

	{ 6,	0,	solaris_getpgrp,	"getpgrp"	}, /* 300 */
	{ 6,	0,	solaris_setpgrp,	"setpgrp"	}, /* 301 */
	{ 6,	0,	solaris_getsid,		"getsid"	}, /* 302 */
	{ 6,	0,	solaris_setsid,		"setsid"	}, /* 303 */
	{ 6,	0,	solaris_getpgid,	"getpgid"	}, /* 304 */
	{ 6,	0,	solaris_setpgid,	"setpgid"	}, /* 305 */
	{ 6,	0,	NULL,			NULL		}, /* 306 */
	{ 6,	0,	NULL,			NULL		}, /* 307 */
	{ 6,	0,	NULL,			NULL		}, /* 308 */
	{ 6,	0,	NULL,			NULL		}, /* 309 */

	{ 6,	TS,	solaris_signal,		"signal"	}, /* 310 */
	{ 6,	TS,	solaris_sigset,		"sigset"	}, /* 311 */
	{ 6,	TS,	solaris_sighold,	"sighold"	}, /* 312 */
	{ 6,	TS,	solaris_sigrelse,	"sigrelse"	}, /* 313 */
	{ 6,	TS,	solaris_sigignore,	"sigignore"	}, /* 314 */
	{ 6,	TS,	solaris_sigpause,	"sigpause"	}, /* 315 */
	{ 6,	0,	NULL,			NULL		}, /* 316 */
	{ 6,	0,	NULL,			NULL		}, /* 317 */
	{ 6,	0,	NULL,			NULL		}, /* 318 */
	{ 6,	0,	NULL,			NULL		}, /* 319 */

	{ 6,	TI,	solaris_msgget,		"msgget"	}, /* 320 */
	{ 6,	TI,	solaris_msgctl,		"msgctl"	}, /* 321 */
	{ 6,	TI,	solaris_msgrcv,		"msgrcv"	}, /* 322 */
	{ 6,	TI,	solaris_msgsnd,		"msgsnd"	}, /* 323 */
	{ 6,	0,	NULL,			NULL		}, /* 324 */
	{ 6,	0,	NULL,			NULL		}, /* 325 */
	{ 6,	0,	NULL,			NULL		}, /* 326 */
	{ 6,	0,	NULL,			NULL		}, /* 327 */
	{ 6,	0,	NULL,			NULL		}, /* 328 */
	{ 6,	0,	NULL,			NULL		}, /* 329 */

	{ 6,	TI,	solaris_shmat,		"shmat"		}, /* 330 */
	{ 6,	TI,	solaris_shmctl,		"shmctl"	}, /* 331 */
	{ 6,	TI,	solaris_shmdt,		"shmdt"		}, /* 332 */
	{ 6,	TI,	solaris_shmget,		"shmget"	}, /* 333 */
	{ 6,	0,	NULL,			NULL		}, /* 334 */
	{ 6,	0,	NULL,			NULL		}, /* 335 */
	{ 6,	0,	NULL,			NULL		}, /* 336 */
	{ 6,	0,	NULL,			NULL		}, /* 337 */
	{ 6,	0,	NULL,			NULL		}, /* 338 */
	{ 6,	0,	NULL,			NULL		}, /* 339 */

	{ 6,	TI,	solaris_semctl,		"semctl"	}, /* 340 */
	{ 6,	TI,	solaris_semget,		"semget"	}, /* 341 */
	{ 6,	TI,	solaris_semop,		"semop"		}, /* 342 */
	{ 6,	0,	NULL,			NULL		}, /* 343 */
	{ 6,	0,	NULL,			NULL		}, /* 344 */
	{ 6,	0,	NULL,			NULL		}, /* 345 */
	{ 6,	0,	NULL,			NULL		}, /* 346 */
	{ 6,	0,	NULL,			NULL		}, /* 347 */
	{ 6,	0,	NULL,			NULL		}, /* 348 */
	{ 6,	0,	NULL,			NULL		}, /* 349 */

	{ 6,	0,	solaris_olduname,	"olduname"	}, /* 350 */
	{ 6,	0,	printargs,		"utssys1"	}, /* 351 */
	{ 6,	0,	solaris_ustat,		"ustat"		}, /* 352 */
	{ 6,	0,	solaris_fusers,		"fusers"	}, /* 353 */
	{ 6,	0,	NULL,			NULL		}, /* 354 */
	{ 6,	0,	NULL,			NULL		}, /* 355 */
	{ 6,	0,	NULL,			NULL		}, /* 356 */
	{ 6,	0,	NULL,			NULL		}, /* 357 */
	{ 6,	0,	NULL,			NULL		}, /* 358 */
	{ 6,	0,	NULL,			NULL		}, /* 359 */

	{ 6,	0,	printargs,		"sysfs0"	}, /* 360 */
	{ 6,	0,	solaris_sysfs1,		"sysfs1"	}, /* 361 */
	{ 6,	0,	solaris_sysfs2,		"sysfs2"	}, /* 362 */
	{ 6,	0,	solaris_sysfs3,		"sysfs3"	}, /* 363 */
	{ 6,	0,	NULL,			NULL		}, /* 364 */
	{ 6,	0,	NULL,			NULL		}, /* 365 */
	{ 6,	0,	NULL,			NULL		}, /* 366 */
	{ 6,	0,	NULL,			NULL		}, /* 367 */
	{ 6,	0,	NULL,			NULL		}, /* 368 */
	{ 6,	0,	NULL,			NULL		}, /* 369 */

	{ 6,	0,	printargs,		"spcall0"	}, /* 370 */
	{ 6,	TS,	solaris_sigpending,	"sigpending"	}, /* 371 */
	{ 6,	TS,	solaris_sigfillset,	"sigfillset"	}, /* 372 */
	{ 6,	0,	NULL,			NULL		}, /* 373 */
	{ 6,	0,	NULL,			NULL		}, /* 374 */
	{ 6,	0,	NULL,			NULL		}, /* 375 */
	{ 6,	0,	NULL,			NULL		}, /* 376 */
	{ 6,	0,	NULL,			NULL		}, /* 377 */
	{ 6,	0,	NULL,			NULL		}, /* 378 */
	{ 6,	0,	NULL,			NULL		}, /* 379 */

	{ 6,	0,	solaris_getcontext,	"getcontext"	}, /* 380 */
	{ 6,	0,	solaris_setcontext,	"setcontext"	}, /* 381 */
	{ 6,	0,	NULL,			NULL		}, /* 382 */
	{ 6,	0,	NULL,			NULL		}, /* 383 */
	{ 6,	0,	NULL,			NULL		}, /* 384 */
	{ 6,	0,	NULL,			NULL		}, /* 385 */
	{ 6,	0,	NULL,			NULL		}, /* 386 */
	{ 6,	0,	NULL,			NULL		}, /* 387 */
	{ 6,	0,	NULL,			NULL		}, /* 388 */
	{ 6,	0,	NULL,			NULL		}, /* 389 */

	{ 6,	0,	NULL,			NULL		}, /* 390 */
	{ 6,	0,	NULL,			NULL		}, /* 391 */
	{ 6,	0,	NULL,			NULL		}, /* 392 */
	{ 6,	0,	NULL,			NULL		}, /* 393 */
	{ 6,	0,	NULL,			NULL		}, /* 394 */
	{ 6,	0,	NULL,			NULL		}, /* 395 */
	{ 6,	0,	NULL,			NULL		}, /* 396 */
	{ 6,	0,	NULL,			NULL		}, /* 397 */
	{ 6,	0,	NULL,			NULL		}, /* 398 */
	{ 6,	0,	NULL,			NULL		}, /* 399 */
