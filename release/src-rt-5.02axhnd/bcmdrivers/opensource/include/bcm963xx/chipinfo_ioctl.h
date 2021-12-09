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

#ifndef CHIPINFO_IOCTL_H_DEFINED
#define CHIPINFO_IOCTL_H_DEFINED

#define CHIPINFO_IOCTL_MAGIC       'C'
#define CHIPINFO_DRV_MAJOR          3006

typedef struct chipinfoIoctlParms
{
    long long pString;
    int strLen;
    int action;
    int result;
} CHIPINFO_IOCTL_PARMS;


#define CHIPINFO_IOCTL_GET_CHIP_CAPABILITY         _IOWR(CHIPINFO_IOCTL_MAGIC, 0, CHIPINFO_IOCTL_PARMS)

/*

CHIPINFO_IOCTL_GET_CHIP_CAPABILITY is called and passsed the address of
a CHIPINFO_IOCTL_PARMS.  The CHIPINFO_IOCTL_PARMS contains one of the 
enumerated values of CHIPINFO_IOCTL_CHIP_CAPABILITY in its action member.

Upon successful return, the result member of CHIPINFO_IOCTL_PARMS is
-1 : cannot determine if capability exists on this device
0 : capability absent
1 : capability present

*/

typedef enum
{
    CAN_DECT,
    CAN_STBC  
} CHIPINFO_IOCTL_CHIP_CAPABILITY;

#define CHIPINFO_IOCTL_GET_CHIP_STRING             _IOWR(CHIPINFO_IOCTL_MAGIC, 1, CHIPINFO_IOCTL_PARMS)

#endif

