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

