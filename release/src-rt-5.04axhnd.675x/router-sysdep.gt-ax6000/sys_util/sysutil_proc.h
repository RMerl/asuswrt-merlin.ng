/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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

#ifndef __SYSUTIL_PROC_H__
#define __SYSUTIL_PROC_H__

/*!\file sysutil_proc.h
 * \brief Header file for various utility functions dealing with /proc.
 *
 */

#define PROC_THREAD_NAME_LEN  64

/** Structure to hold info about a thread or process, filled in and returned
 *  by sysUtl_getInfoFromProc().  More fields can be added as needed.
 */
typedef struct {
   char name[PROC_THREAD_NAME_LEN];
   char status;  /**< a single letter: R, S, Z, etc */
   int  totalMemKB;  /**< total size of the app in KB, includes code and heap */
} ProcThreadInfo;


/** Evaluates to true if ProcThreadInfo struct indicates Zombie. */
#define IS_PROC_THREAD_INFO_ZOMBIE(i) ((i)->status == 'Z')


/** Get info about the thread id or process id.
 *
 * @param tid  (IN)  the thread or process id.
 * @param info (OUT) will be filled in by this function.
 *
 * @return 0 on success, -1 for not found, -2 for all other errors.
 */
int sysUtl_getThreadInfoFromProc(int tid, ProcThreadInfo *info);



#endif /* __SYSUTIL_PROC_H__ */
