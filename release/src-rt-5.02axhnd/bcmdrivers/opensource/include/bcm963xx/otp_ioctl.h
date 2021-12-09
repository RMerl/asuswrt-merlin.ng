/* 
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
