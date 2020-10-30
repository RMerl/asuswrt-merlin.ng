	{ 0,	0,	printargs,		"spill"		}, /* 0 */
	{ 0,	0,	printargs,		"xtensa"	}, /* 1 */
	[2 ... 7] = { },
	{ 3,	TD|TF,	sys_open,		"open"		}, /* 8 */
	{ 1,	TD,	sys_close,		"close"		}, /* 9 */
	{ 1,	TD,	sys_dup,		"dup"		}, /* 10 */
	{ 2,	TD,	sys_dup2,		"dup2"		}, /* 11 */
	{ 3,	TD,	sys_read,		"read"		}, /* 12 */
	{ 3,	TD,	sys_write,		"write"		}, /* 13 */
	{ 5,	TD,	sys_select,		"select"	}, /* 14 */
	{ 3,	TD,	sys_lseek,		"lseek"		}, /* 15 */
	{ 3,	TD,	sys_poll,		"poll"		}, /* 16 */
	{ 5,	TD,	sys_llseek,		"_llseek"	}, /* 17 */
	{ 4,	TD,	sys_epoll_wait,		"epoll_wait"	}, /* 18 */
	{ 4,	TD,	sys_epoll_ctl,		"epoll_ctl"	}, /* 19 */
	{ 1,	TD,	sys_epoll_create,	"epoll_create"	}, /* 20 */
	{ 2,	TD|TF,	sys_creat,		"creat"		}, /* 21 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 22 */
	{ 2,	TD,	sys_ftruncate,		"ftruncate"	}, /* 23 */
	{ 3,	TD,	sys_readv,		"readv"		}, /* 24 */
	{ 3,	TD,	sys_writev,		"writev"	}, /* 25 */
	{ 1,	TD,	sys_fsync,		"fsync"		}, /* 26 */
	{ 1,	TD,	sys_fdatasync,		"fdatasync"	}, /* 27 */
	{ 4,	TF,	sys_truncate64,		"truncate64"	}, /* 28 */
	{ 4,	TD,	sys_ftruncate64,	"ftruncate64"	}, /* 29 */
	{ 6,	TD,	sys_pread,		"pread64"	}, /* 30 */
	{ 6,	TD,	sys_pwrite,		"pwrite64"	}, /* 31 */
	{ 2,	TF,	sys_link,		"link"		}, /* 32 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 33 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 34 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 35 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 36 */
	{ 1,	TD,	sys_pipe,		"pipe"		}, /* 37 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 38 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 39 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 40 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 41 */
	{ 1,	TD,	sys_fchdir,		"fchdir"	}, /* 42 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 43 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 44 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 45 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 46 */
	{ 2,	TF,	sys_stat64,		"stat64"	}, /* 47 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 48 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 49 */
	{ 2,	TF,	sys_lstat64,		"lstat64"	}, /* 50 */
	[51] = { },
	{ 2,	TD,	sys_fchmod,		"fchmod"	}, /* 52 */
	{ 3,	TD,	sys_fchown,		"fchown"	}, /* 53 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 54 */
	{ 2,	TD,	sys_fstat64,		"fstat64"	}, /* 55 */
	{ 2,	TD,	sys_flock,		"flock"		}, /* 56 */
	{ 2,	TF,	sys_access,		"access"	}, /* 57 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 58 */
	{ 3,	TD,	sys_getdents,		"getdents"	}, /* 59 */
	{ 3,	TD,	sys_getdents64,		"getdents64"	}, /* 60 */
	{ 3,	TD,	sys_fcntl,		"fcntl64"	}, /* 61 */
	{ 6,	TD,	sys_fallocate,		"fallocate"	}, /* 62 */
	{ 6,	TD,	sys_fadvise64_64,	"fadvise64_64"	}, /* 63 */
	{ 2,	TF,	sys_utime,		"utime"		}, /* 64 */
	{ 2,	TF,	sys_utimes,		"utimes"	}, /* 65 */
	{ 3,	TD,	sys_ioctl,		"ioctl"		}, /* 66 */
	{ 3,	TD,	sys_fcntl,		"fcntl"		}, /* 67 */
	{ 5,	TF,	sys_setxattr,		"setxattr"	}, /* 68 */
	{ 4,	TF,	sys_getxattr,		"getxattr"	}, /* 69 */
	{ 3,	TF,	sys_listxattr,		"listxattr"	}, /* 70 */
	{ 2,	TF,	sys_removexattr,	"removexattr"	}, /* 71 */
	{ 5,	TF,	sys_setxattr,		"lsetxattr"	}, /* 72 */
	{ 4,	TF,	sys_getxattr,		"lgetxattr"	}, /* 73 */
	{ 3,	TF,	sys_listxattr,		"llistxattr"	}, /* 74 */
	{ 2,	TF,	sys_removexattr,	"lremovexattr"	}, /* 75 */
	{ 5,	TD,	sys_fsetxattr,		"fsetxattr"	}, /* 76 */
	{ 4,	TD,	sys_fgetxattr,		"fgetxattr"	}, /* 77 */
	{ 3,	TD,	sys_flistxattr,		"flistxattr"	}, /* 78 */
	{ 2,	TD,	sys_fremovexattr,	"fremovexattr"	}, /* 79 */
	{ 6,	TD|TM,	sys_mmap_pgoff,		"mmap2"		}, /* 80 */
	{ 2,	TM,	sys_munmap,		"munmap"	}, /* 81 */
	{ 3,	TM,	sys_mprotect,		"mprotect"	}, /* 82 */
	{ 1,	TM,	sys_brk,		"brk"		}, /* 83 */
	{ 2,	TM,	sys_mlock,		"mlock"		}, /* 84 */
	{ 2,	TM,	sys_munlock,		"munlock"	}, /* 85 */
	{ 1,	TM,	sys_mlockall,		"mlockall"	}, /* 86 */
	{ 0,	TM,	sys_munlockall,		"munlockall"	}, /* 87 */
	{ 4,	TM,	sys_mremap,		"mremap"	}, /* 88 */
	{ 3,	TM,	sys_msync,		"msync"		}, /* 89 */
	{ 3,	TM,	sys_mincore,		"mincore"	}, /* 90 */
	{ 3,	TM,	sys_madvise,		"madvise"	}, /* 91 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 92 */
	{ 4,	TI,	sys_shmat,		"shmat"		}, /* 93 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 94 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 95 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 96 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 97 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 98 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 99 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 100 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 101 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 102 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 103 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 104 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 105 */
	{ 3,	TN,	sys_sendmsg,		"sendmsg"	}, /* 106 */
	{ 3,	TN,	sys_recvmsg,		"recvmsg"	}, /* 107 */
	{ 4,	TN,	sys_send,		"send"		}, /* 108 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 109 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 110 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 111 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 112 */
	{ 4,	TD|TN,	sys_sendfile,		"sendfile"	}, /* 113 */
	{ 4,	TD|TN,	sys_sendfile64,		"sendfile64"	}, /* 114 */
	{ 4,	TN,	sys_sendmsg,		"sendmsg"	}, /* 115 */
	{ 5,	TP,	sys_clone,		"clone"		}, /* 116 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 117 */
	{ 1,	TP,	sys_exit,		"exit"		}, /* 118 */
	{ 1,	TP,	sys_exit,		"exit_group"	}, /* 119 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 120 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 121 */
	{ 5,	TP,	sys_waitid,		"waitid"	}, /* 122 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 123 */
	{ 2,	TS,	sys_kill,		"tkill"		}, /* 124 */
	{ 3,	TS,	sys_tgkill,		"tgkill"	}, /* 125 */
	{ 1,	0,	printargs,		"set_tid_address"}, /* 126 */
	{ 0,	0,	printargs,		"gettid"	}, /* 127 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 128 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 129 */
	{ 5,	0,	sys_prctl,		"prctl"		}, /* 130 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 131 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 132 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 133 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 134 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 135 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 136 */
	{ 0,	NF,	sys_getuid,		"getuid"	}, /* 137 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 138 */
	{ 0,	NF,	sys_getgid,		"getgid"	}, /* 139 */
	{ 0,	NF,	sys_geteuid,		"geteuid"	}, /* 140 */
	{ 0,	NF,	sys_getegid,		"getegid"	}, /* 141 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 142 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 143 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 144 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 145 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 146 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 147 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 148 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 149 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 150 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 151 */
	[152 ... 153] = { },
	{ 1,	0,	sys_times,		"times"		}, /* 154 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 155 */
	{ 3,	0,	sys_sched_setaffinity,	"sched_setaffinity"}, /* 156 */
	{ 3,	0,	sys_sched_getaffinity,	"sched_getaffinity"}, /* 157 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 158 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 159 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 160 */
	{ 5,	TI,	sys_semtimedop,		"semtimedop"	}, /* 161 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 162 */
	{ 4,	TI,	sys_semop,		"semop"		}, /* 163 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 164 */
	[165] = { },
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 166 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 167 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 168 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 169 */
	[170] = { },
	{ 2,	TF,	sys_umount2,		"umount2"	}, /* 171 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 172 */
	{ 2,	TF,	sys_swapon,		"swapon"	}, /* 173 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 174 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 175 */
	{ 2,	TF,	sys_umount,		"umount"	}, /* 176 */
	{ 1,	TF,	sys_swapoff,		"swapoff"	}, /* 177 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 178 */
	[179] = { },
	{ 1,	NF,	sys_setfsuid,		"setfsuid"	}, /* 180 */
	{ 1,	NF,	sys_setfsgid,		"setfsgid"	}, /* 181 */
	{ 3,	0,	sys_sysfs,		"sysfs"		}, /* 182 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 183 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 184 */
	{ 2,	TD,	sys_fstatfs,		"fstatfs"	}, /* 185 */
	{ 3,	TF,	sys_statfs64,		"statfs64"	}, /* 186 */
	{ 3,	TD,	sys_fstatfs64,		"fstatfs64"	}, /* 187 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 188 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 189 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 190 */
	{ 5,	0,	sys_futex,		"futex"		}, /* 191 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 192 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 193 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 194 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 195 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 196 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 197 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 198 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 199 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 200 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 201 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 202 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 203 */
	{ 4,	TF,	sys_quotactl,		"quotactl"	}, /* 204 */
	{ 0,	0,	printargs,		"nfsservctl"	}, /* 205 */
	{ 1,	0,	sys_sysctl,		"_sysctl"	}, /* 206 */
	{ 2,	0,	sys_bdflush,		"bdflush"	}, /* 207 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 208 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 209 */
	{ 2,	0,	sys_init_module,	"init_module"	}, /* 210 */
	{ 1,	0,	sys_delete_module,	"delete_module"	}, /* 211 */
	{ 2,	0,	sys_sched_setparam,	"sched_setparam"}, /* 212 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 213 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 214 */
	{ 1,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 215 */
	{ 1,	0,	sys_sched_get_priority_max, "sched_get_priority_max"}, /* 216 */
	{ 1,	0,	sys_sched_get_priority_min, "sched_get_priority_min"}, /* 217 */
	{ 2,	0,	sys_sched_rr_get_interval, "sched_rr_get_interval"}, /* 218 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"	}, /* 219 */
	[220 ... 222] = { },
	{ 0,	0,	printargs,		"restart_syscall"}, /* 223 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 224 */
	{ 0,	TS,	sys_rt_sigreturn,	"rt_sigreturn"	}, /* 225 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"	}, /* 226 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 227 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 228 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 229 */
	{ 3,	TS,	sys_rt_sigqueueinfo,	"rt_sigqueueinfo"}, /* 230 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 231 */
	{ 4,	0,	sys_mq_open,		"mq_open"	}, /* 232 */
	{ 1,	0,	sys_mq_unlink,		"mq_unlink"	}, /* 233 */
	{ 5,	0,	sys_mq_timedsend,	"mq_timedsend"	}, /* 234 */
	{ 5,	0,	sys_mq_timedreceive,	"mq_timedreceive"}, /* 235 */
	{ 2,	0,	sys_mq_notify,		"mq_notify"	}, /* 236 */
	{ 3,	0,	sys_mq_getsetattr,	"mq_getsetattr"	}, /* 237 */
	[238] = { },
	{ 2,	0,	printargs,		"io_setup"	}, /* 239 */
	{ 1,	0,	printargs,		"io_destroy"	}, /* 240 */
	{ 3,	0,	printargs,		"io_submit"	}, /* 241 */
	{ 5,	0,	printargs,		"io_getevents"	}, /* 242 */
	{ 3,	0,	printargs,		"io_cancel"	}, /* 243 */
	{ 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 244 */
	{ 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 245 */
	{ 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 246 */
	{ 4,	0,	sys_clock_nanosleep,	"clock_nanosleep"}, /* 247 */
	{ 3,	0,	sys_timer_create,	"timer_create"	}, /* 248 */
	{ 1,	0,	sys_timer_delete,	"timer_delete"	}, /* 249 */
	{ 4,	0,	sys_timer_settime,	"timer_settime"	}, /* 250 */
	{ 2,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 251 */
	{ 1,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 252 */
	[253] = { },
	{ 4,	0,	printargs,		"lookup_dcookie"}, /* 254 */
	[255] = { },
	{ 5,	0,	printargs,		"add_key"	}, /* 256 */
	{ 5,	0,	printargs,		"request_key"	}, /* 257 */
	{ 5,	0,	printargs,		"keyctl"	}, /* 258 */
	[259] = { },
	{ 5,	TD,	sys_readahead,		"readahead"	}, /* 260 */
	{ 5,	TM,	sys_remap_file_pages,	"remap_file_pages"}, /* 261 */
	{ 5,	TM,	sys_migrate_pages,	"migrate_pages"	}, /* 262 */
	{ 0,	TM,	sys_mbind,		"mbind"		}, /* 263 */
	{ 6,	TM,	sys_get_mempolicy,	"get_mempolicy"	}, /* 264 */
	{ 5,	TM,	sys_set_mempolicy,	"set_mempolicy"	}, /* 265 */
	{ 3,	TP,	sys_unshare,		"unshare"	}, /* 266 */
	{ 1,	TM,	sys_move_pages,		"move_pages"	}, /* 267 */
	{ 0,	TD,	sys_splice,		"splice"	}, /* 268 */
	{ 0,	TD,	sys_tee,		"tee"		}, /* 269 */
	{ 0,	TD,	sys_vmsplice,		"vmsplice"	}, /* 270 */
	{ 0,	0,	printargs,		"SYS_271"	}, /* 271 */
	{ 0,	TD,	sys_pselect6,		"pselect6"	}, /* 272 */
	{ 0,	TD,	sys_ppoll,		"ppoll"		}, /* 273 */
	{ 0,	TD,	sys_epoll_pwait,	"epoll_pwait"	}, /* 274 */
	{ 1,	TD,	sys_epoll_create1,	"epoll_create1"	}, /* 275 */
	{ 0,	TD,	sys_inotify_init,	"inotify_init"	}, /* 276 */
	{ 3,	TD,	sys_inotify_add_watch,	"inotify_add_watch"}, /* 277 */
	{ 2,	TD,	sys_inotify_rm_watch,	"inotify_rm_watch"}, /* 278 */
	{ 1,	TD,	sys_inotify_init1,	"inotify_init1"	}, /* 279 */
	{ 0,	0,	sys_getcpu,		"getcpu"	}, /* 280 */
	{ 4,	0,	sys_kexec_load,		"kexec_load"	}, /* 281 */
	{ 2,	0,	sys_ioprio_set,		"ioprio_set"	}, /* 282 */
	{ 3,	0,	sys_ioprio_get,		"ioprio_get"	}, /* 283 */
	{ 3,	0,	sys_set_robust_list,	"set_robust_list"}, /* 284 */
	{ 3,	0,	sys_get_robust_list,	"get_robust_list"}, /* 285 */
	{ 0,	0,	printargs,		"SYS_286"	}, /* 286 */
	{ 0,	0,	printargs,		"SYS_287"	}, /* 287 */
	{ 4,	TD|TF,	sys_openat,		"openat"	}, /* 288 */
	{ 3,	TD|TF,	sys_mkdirat,		"mkdirat"	}, /* 289 */
	{ 4,	TD|TF,	sys_mknodat,		"mknodat"	}, /* 290 */
	{ 3,	TD|TF,	sys_unlinkat,		"unlinkat"	}, /* 291 */
	{ 4,	TD|TF,	sys_renameat,		"renameat"	}, /* 292 */
	{ 5,	TD|TF,	sys_linkat,		"linkat"	}, /* 293 */
	{ 3,	TD|TF,	sys_symlinkat,		"symlinkat"	}, /* 294 */
	{ 4,	TD|TF,	sys_readlinkat,		"readlinkat"	}, /* 295 */
	{ 0,	TD|TF,	sys_utimensat,		"utimensat"	}, /* 296 */
	{ 5,	TD|TF,	sys_fchownat,		"fchownat"	}, /* 297 */
	{ 4,	TD|TF,	sys_futimesat,		"futimesat"	}, /* 298 */
	{ 0,	TD|TF,	sys_newfstatat,		"fstatat64"	}, /* 299 */
	{ 4,	TD|TF,	sys_fchmodat,		"fchmodat"	}, /* 300 */
	{ 4,	TD|TF,	sys_faccessat,		"faccessat"	}, /* 301 */
	{ 0,	0,	printargs,		"SYS_302"	}, /* 302 */
	{ 0,	0,	printargs,		"SYS_303"	}, /* 303 */
	{ 3,	TD|TS,	sys_signalfd,		"signalfd"	}, /* 304 */
	{ 0,	0,	printargs,		"SYS_305"	}, /* 305 */
	{ 1,	TD,	sys_eventfd,		"eventfd"	}, /* 306 */
	{ 5,	TN,	sys_recvmmsg,		"recvmmsg"	}, /* 307 */
	{ 2,	TD,	sys_setns,		"setns"		}, /* 308 */
	{ 4,	TD|TS,	sys_signalfd4,		"signalfd4"	}, /* 309 */
	{ 3,	TD,	sys_dup3,		"dup3"		}, /* 310 */
	{ 2,	TD,	sys_pipe2,		"pipe2"		}, /* 311 */
	{ 2,	TD,	sys_timerfd_create,	"timerfd_create"}, /* 312 */
	{ 4,	TD,	sys_timerfd_settime,	"timerfd_settime"}, /* 313 */
	{ 2,	TD,	sys_timerfd_gettime,	"timerfd_gettime"}, /* 314 */
	{ 0,	0,	printargs,		"SYS_315"	}, /* 315 */
	{ 2,	TD,	sys_eventfd2,		"eventfd2"	}, /* 316 */
	{ 6,	TD,	sys_preadv,		"preadv"	}, /* 317 */
	{ 6,	TD,	sys_pwritev,		"pwritev"	}, /* 318 */
	[319] = { },
	{ 2,	TD,	sys_fanotify_init,	"fanotify_init"	}, /* 320 */
	{ 6,	TD|TF,	sys_fanotify_mark,	"fanotify_mark"	}, /* 321 */
	{ 6,	0,	sys_process_vm_readv,	"process_vm_readv"}, /* 322 */
	{ 6,	0,	sys_process_vm_writev,	"process_vm_writev"}, /* 323 */
	{ 5,	TD|TF,	sys_name_to_handle_at,	"name_to_handle_at"}, /* 324 */
	{ 3,	TD,	sys_open_by_handle_at,	"open_by_handle_at"}, /* 325 */
	{ 6,	TD,	sys_sync_file_range2,	"sync_file_range2"}, /* 326 */
	{ 5,	TD,	sys_perf_event_open,	"perf_event_open"}, /* 327 */
	{ 4,	TP|TS,	sys_rt_tgsigqueueinfo,	"rt_tgsigqueueinfo"}, /* 328 */
	{ 2,	0,	sys_clock_adjtime,	"clock_adjtime"	}, /* 329 */
	{ 4,	0,	sys_prlimit64,		"prlimit64"	}, /* 330 */
	{ 5,	0,	sys_kcmp,		"kcmp"		}, /* 331 */
	{ 3,	TD,	sys_finit_module,	"finit_module"	}, /* 332 */
	{ 4,	TN,	sys_accept4,		"accept4"	}, /* 333 */
