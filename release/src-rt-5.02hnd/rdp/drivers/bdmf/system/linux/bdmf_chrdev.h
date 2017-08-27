/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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


#ifndef BDMF_CHRDEV_H_
#define BDMF_CHRDEV_H_

#include <linux/ioctl.h>

/* Use 'S' as magic number */
#define BDMF_CHRDEV_IOC_MAGIC  'S'

#define BDMF_CHRDEV_SESSION_INIT  _IOW(BDMF_CHRDEV_IOC_MAGIC, 1, int)
#define BDMF_CHRDEV_SESSION_SEND  _IOW(BDMF_CHRDEV_IOC_MAGIC, 2, int)
#define BDMF_CHRDEV_SESSION_CLOSE _IOW(BDMF_CHRDEV_IOC_MAGIC, 3, int)

#define BDMF_CHRDEV_MAX_NR            3
#define BDMF_CHRDEV_MAX_CMD_LENGTH    2048

struct io_param
{
    int     session_id;                                 /* session id */
    int     rc;                                         /* return code of the executed command */
    char    command[BDMF_CHRDEV_MAX_CMD_LENGTH];          /* Input command as one string */
};

#endif /* BDMF_CHRDEV_H_ */
