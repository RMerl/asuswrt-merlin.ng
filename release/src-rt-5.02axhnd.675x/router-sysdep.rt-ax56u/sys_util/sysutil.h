/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
************************************************************************/

#ifndef __SYSUTIL_H__
#define __SYSUTIL_H__

/*!\file sysutil.h
 * \brief Header file for various system utility functions. 
 *
 */

/** Get the thread id of the calling process.  For single threaded processes,
 *  the thread id is the same as process id (pid).
 *
 * @return the thread id.
 */
int sysUtl_getThreadId();

/** Same as sysUtl_getThreadId().
 */
int sysUtl_gettid();


#endif /* __SYSUTIL_H__ */
