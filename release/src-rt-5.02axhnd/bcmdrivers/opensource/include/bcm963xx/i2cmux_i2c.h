/*

 Copyright (c) 2016 Broadcom Corporation
* <:copyright-BRCM:2016:DUAL/GPL:standard
*
*    Copyright (c) 2016 Broadcom
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
/***********************************************************************/
/*                                                                     */
/*   MODULE   i2cmuc_i2c.h                                             */
/*   DATE:    08/23/16                                                 */
/*   PURPOSE: I2C MUx access API                                       */
/*                                                                     */
/***********************************************************************/
#ifndef __I2CMUX_I2C_H
#define __I2CMUX_I2C_H

#define MAX_TRANSACTION_SIZE  4


/****************************************************************************/
/* Read buffer: Reads buufer of size count                                  */
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/****************************************************************************/
ssize_t i2cmux_read(char *buf, size_t count);

/****************************************************************************/
/* Read Byte: Writes buffer of size count                                   */
/* Returns:                                                                 */
/*   value on success, negative value on failure.                           */
/****************************************************************************/
ssize_t i2cmux_write(char *buf, size_t count);

#endif
