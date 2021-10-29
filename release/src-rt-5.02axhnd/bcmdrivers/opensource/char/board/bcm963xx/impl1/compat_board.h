/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
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
* :> 
*/

/***********************************************************************/
/*                                                                     */
/*   MODULE:  compat_board.h                                           */
/*   PURPOSE: compat layer for board driver.                           */
/*                                                                     */
/***********************************************************************/
#ifndef _COMPAT_BOARD_H
#define _COMPAT_BOARD_H

long compat_board_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif /* _COMPAT_BOARD_H */
