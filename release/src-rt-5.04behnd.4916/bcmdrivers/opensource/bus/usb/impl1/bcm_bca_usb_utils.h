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

void usb_mdio_write(volatile uint32_t *mdio, uint32_t reg, uint32_t val, int mode);
uint32_t usb_mdio_read(volatile uint32_t *mdio, uint32_t reg, int mode);
uint32_t xhci_ecira_read(uint32_t *base, uint32_t reg);
void xhci_ecira_write(uint32_t *base, uint32_t reg, uint32_t value);
void usb3_ssc_enable(uint32_t *mdio);
void usb3_enable_pipe_reset(uint32_t *mdio);
void usb3_enable_sigdet(uint32_t *mdio);
void usb3_enable_skip_align(uint32_t *mdio);
void usb2_eye_fix(uint32_t *mdio);
