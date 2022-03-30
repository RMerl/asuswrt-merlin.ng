/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef PMC_USB_H
#define PMC_USB_H

enum{
    PMC_USB_ALL,
    PMC_USB_HOST_ALL,
    PMC_USB_HOST_20,
    PMC_USB_HOST_30,
    PMC_USB_DEVICE,
    PMC_USB_MAX
};

extern int pmc_usb_power_up(unsigned int usb_block);
extern int pmc_usb_power_down(unsigned int usb_block);

#endif //#ifndef PMC_USB_H
