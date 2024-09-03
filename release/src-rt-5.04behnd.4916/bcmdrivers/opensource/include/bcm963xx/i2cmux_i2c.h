/*

 Copyright (c) 2016 Broadcom Corporation
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
