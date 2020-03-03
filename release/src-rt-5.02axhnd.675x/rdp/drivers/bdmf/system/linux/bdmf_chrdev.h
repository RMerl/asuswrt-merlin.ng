/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
