/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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

struct bcm_bca_usb_ctrl {
    void __iomem *usb_ctrl;
    struct platform_device *pdev;
    bool xhci_enable;
    bool port0_enable;
    bool port1_enable;
    bool pwrflt_p_high;
    bool pwron_p_high;
};

int hw_init(struct bcm_bca_usb_ctrl *bca_usb);
void hw_uninit(struct bcm_bca_usb_ctrl *bca_usb);

