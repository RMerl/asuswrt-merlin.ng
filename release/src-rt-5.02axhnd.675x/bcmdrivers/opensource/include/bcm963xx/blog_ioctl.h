#ifndef __BLOG_IOCTL_H_INCLUDED__
#define __BLOG_IOCTL_H_INCLUDED__
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 * File Name : blog_ioctl.h
 *******************************************************************************
 */
/* Blog Character Device */
#define BLOG_DRV_MAJOR             338
#define BLOG_NAME                  "blog"
#define BLOG_DRV_NAME              BLOG_NAME
#define BLOG_DRV_DEVICE_NAME       "/dev/" BLOG_DRV_NAME

/* Functional interface return status */
#define BLOG_ERROR                (-1)    /* Functional interface error     */
#define BLOG_SUCCESS              0       /* Functional interface success   */

#undef BLOG_DECL
#define BLOG_DECL(x)      x,  /* for enum declaration in H file */

/*
 *------------------------------------------------------------------------------
 *              Blog character device driver IOCTL enums
 * A character device and the associated userspace utility for design debug.
 *------------------------------------------------------------------------------
 */
typedef enum blog_ioctl
{
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    BLOG_IOCTL_DUMMY=99,
    BLOG_DECL(BLOG_IOCTL_GET_STATS)
    BLOG_DECL(BLOG_IOCTL_RESET_STATS)
    BLOG_DECL(BLOG_IOCTL_DUMP_BLOG)
    BLOG_DECL(BLOG_IOCTL_INVALID)
} blog_ioctl_t;

#endif  /* defined(__BLOG_IOCTL_H_INCLUDED__) */
