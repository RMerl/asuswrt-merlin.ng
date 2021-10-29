/* 
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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

#ifndef OTP_IOCTL_H_DEFINED
#define OTP_IOCTL_H_DEFINED

#define OTP_IOCTL_MAGIC       'C'
#define OTP_DRV_MAJOR          3010

typedef struct otpIoctlParms
{
    int action;
    int id;
    int result;
} OTP_IOCTL_PARMS;


#define OTP_IOCTL_GET              _IOWR(OTP_IOCTL_MAGIC, 0, OTP_IOCTL_PARMS)
#define OTP_IOCTL_SET              _IOWR(OTP_IOCTL_MAGIC, 1, OTP_IOCTL_PARMS)

/*

OTP_IOCTL_GET/SET is called and passed the address of OTP_IOCTL_PARMS,
which contains one of the enumerated values of OTP_IOCTL_ENUM in its action member.
If a market identifier is passed in, it is in the id member.

Upon successful return, the result member of OTP_IOCTL_PARMS is populated with the
value of the OTP bit(s)

*/

typedef enum
{
    OTP_BTRM_ENABLE_BIT,
    OTP_OPERATOR_ENABLE_BIT,
    OTP_MID_BITS,
    OTP_OID_BITS
} OTP_IOCTL_ENUM;

#endif
