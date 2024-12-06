/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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

void usb_mdio_write(volatile uint32_t *mdio, uint32_t reg, uint32_t val, int mode);
uint32_t usb_mdio_read(volatile uint32_t *mdio, uint32_t reg, int mode);
uint32_t xhci_ecira_read(uint32_t *base, uint32_t reg);
void xhci_ecira_write(uint32_t *base, uint32_t reg, uint32_t value);
void usb3_ssc_enable(uint32_t *mdio);
void usb3_enable_pipe_reset(uint32_t *mdio);
void usb3_enable_sigdet(uint32_t *mdio);
void usb3_enable_skip_align(uint32_t *mdio);
void usb2_eye_fix(uint32_t *mdio);
