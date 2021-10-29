#ifndef __BLOG_IOCTL_H_INCLUDED__
#define __BLOG_IOCTL_H_INCLUDED__
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *******************************************************************************
 * File Name : blog_ioctl.h
 *******************************************************************************
 */
/* Blog Character Device */
#define BLOG_DRV_MAJOR             3038
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
