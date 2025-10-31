/* captab.h --
 * Copyright 2009,2011-14,2020 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING.LIB. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

_S(CAP_CHOWN,			"chown"			)
_S(CAP_DAC_OVERRIDE,		"dac_override"		)
_S(CAP_DAC_READ_SEARCH,		"dac_read_search"	)
_S(CAP_FOWNER,			"fowner"		)
_S(CAP_FSETID,			"fsetid"		)
_S(CAP_KILL,			"kill"			)
_S(CAP_SETGID,			"setgid"		)
_S(CAP_SETUID,			"setuid"		)
_S(CAP_SETPCAP,			"setpcap"		)
_S(CAP_LINUX_IMMUTABLE,		"linux_immutable"	)
_S(CAP_NET_BIND_SERVICE,	"net_bind_service"	)
_S(CAP_NET_BROADCAST,		"net_broadcast"		)
_S(CAP_NET_ADMIN,		"net_admin"		)
_S(CAP_NET_RAW,			"net_raw"		)
_S(CAP_IPC_LOCK,		"ipc_lock"		)
_S(CAP_IPC_OWNER,		"ipc_owner"		)
_S(CAP_SYS_MODULE,		"sys_module"		)
_S(CAP_SYS_RAWIO,		"sys_rawio"		)
_S(CAP_SYS_CHROOT,		"sys_chroot"		)
_S(CAP_SYS_PTRACE,		"sys_ptrace"		)
_S(CAP_SYS_PACCT,		"sys_pacct"		)
_S(CAP_SYS_ADMIN,		"sys_admin"		)
_S(CAP_SYS_BOOT,		"sys_boot"		)
_S(CAP_SYS_NICE,		"sys_nice"		)
_S(CAP_SYS_RESOURCE,		"sys_resource"		)
_S(CAP_SYS_TIME,		"sys_time"		)
_S(CAP_SYS_TTY_CONFIG,		"sys_tty_config"	)
_S(CAP_MKNOD,			"mknod"			)
_S(CAP_LEASE,			"lease"			)
_S(CAP_AUDIT_WRITE,		"audit_write"		)
_S(CAP_AUDIT_CONTROL,		"audit_control"		)
#ifdef CAP_SETFCAP
_S(CAP_SETFCAP,			"setfcap"		)
#endif
#ifdef CAP_MAC_OVERRIDE
_S(CAP_MAC_OVERRIDE,		"mac_override"		)
#endif
#ifdef CAP_MAC_ADMIN
_S(CAP_MAC_ADMIN,		"mac_admin"		)
#endif
#ifdef CAP_SYSLOG
_S(CAP_SYSLOG,			"syslog"		)
#endif
#if defined(CAP_EPOLLWAKEUP) && defined(CAP_BLOCK_SUSPEND)
#error "Both CAP_EPOLLWAKEUP and CAP_BLOCK_SUSPEND are defined"
#endif
#ifdef CAP_EPOLLWAKEUP
_S(CAP_EPOLLWAKEUP,		"epollwakeup"		)
#endif
#ifdef CAP_WAKE_ALARM
_S(CAP_WAKE_ALARM,              "wake_alarm"            )
#endif
#ifdef CAP_BLOCK_SUSPEND
_S(CAP_BLOCK_SUSPEND,		"block_suspend"		)
#endif
#ifdef CAP_AUDIT_READ
_S(CAP_AUDIT_READ,		"audit_read"		)
#endif
#ifdef CAP_PERFMON
_S(CAP_PERFMON,                 "perfmon"               )
#endif
#ifdef CAP_BPF
_S(CAP_BPF,                     "bpf"                   )
#endif
#ifdef CAP_CHECKPOINT_RESTORE
_S(CAP_CHECKPOINT_RESTORE,	"checkpoint_restore")
#endif
