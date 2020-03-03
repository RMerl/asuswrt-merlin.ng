/*************************************************************************
 *
 * ivi_ioctl.h :
 * 
 * This file is the header file for the 'ivi_ioctl.c' file,
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 *   Guoliang Han <bupthgl@gmail.com>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/


#ifndef IVI_IOCTL_H
#define IVI_IOCTL_H

#include "ivi_config.h"

#define IVI_DEVNAME	"ivi"

typedef enum IviIoctl
{
    IVI_IOC_DUMMY = 999,
    IVI_IOC_V4DEV,
    IVI_IOC_V6DEV,
    IVI_IOC_START,
    IVI_IOC_STOP,
    IVI_IOC_V4NET,
    IVI_IOC_V4MASK,
    IVI_IOC_V6NET,
    IVI_IOC_V6MASK,
    IVI_IOC_V4PUB,
    IVI_IOC_V4PUBMASK,
    IVI_IOC_NAT,
    IVI_IOC_NONAT,
    IVI_IOC_HGW_MAPX,
    IVI_IOC_MAPT,
    IVI_IOC_MSS_LIMIT,
    IVI_IOC_ADJACENT,
    IVI_IOC_ADD_RULE,
    IVI_IOC_TRANSPT,
    IVI_IOC_ADD_PORTMAP,
    IVI_IOC_DEL_PORTMAP,
} IviIoctl_t;

#define IVI_IOCTL_LEN	32

#ifdef __KERNEL__

extern int ivi_ioctl_init(void);
extern void ivi_ioctl_exit(void);

#endif

#endif /* IVI_IOCTL_H */

