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
#include <linux/types.h>
#include <linux/delay.h>

void usb_mdio_write(volatile uint32_t *mdio, uint32_t reg, uint32_t val, int mode)
{
    uint32_t data;
    data = (reg << 16) | val | mode;
    *mdio = data;
    data |= (1 << 25);
    *mdio = data;
    mdelay(1);
    data &= ~(1 << 25);
    *mdio = data;
}

uint32_t usb_mdio_read(volatile uint32_t *mdio, uint32_t reg, int mode)
{
    uint32_t data;

    data = (reg << 16) | mode;
    mdio[0] = data;
    data |= (1 << 24);
    mdio[0] = data;
    mdelay(1);
    data &= ~(1 << 24);
    mdelay(1);

    return (mdio[1] & 0xffff);
}

uint32_t xhci_ecira_read(uint32_t *base, uint32_t reg)
{
    volatile uint32_t *addr;
    uint32_t value;

    addr = &base[2];
    *addr = reg;

    addr = &base[3];
    value = *addr; 

    return value;
}

void xhci_ecira_write(uint32_t *base, uint32_t reg, uint32_t value)
{

    volatile uint32_t *addr;

    addr = &base[2];
    *addr = reg;

    addr = &base[3];
    *addr = value; 
}

#define MDIO_USB2   0
#define MDIO_USB3   (1 << 31)

void usb3_ssc_enable(uint32_t *mdio)
{
    uint32_t val;

    /* Enable USB 3.0 TX spread spectrum */
    usb_mdio_write(mdio, 0x1f, 0x8040, MDIO_USB3);
    val = usb_mdio_read(mdio, 0x01, MDIO_USB3) | 0x0f;
    usb_mdio_write(mdio, 0x01, val, MDIO_USB3);

    usb_mdio_write(mdio, 0x1f, 0x9040, MDIO_USB3);
    val = usb_mdio_read(mdio, 0x01, MDIO_USB3) | 0x0f;
    usb_mdio_write(mdio, 0x01, val, MDIO_USB3);
}

void usb3_enable_pipe_reset(uint32_t *mdio)
{
    uint32_t val;

    /* Re-enable USB 3.0 pipe reset */
    usb_mdio_write(mdio, 0x1f, 0x8000, MDIO_USB3);
    val = usb_mdio_read(mdio, 0x0f, MDIO_USB3) | 0x200;
    usb_mdio_write(mdio, 0x0f, val, MDIO_USB3);
}

void usb3_enable_sigdet(uint32_t *mdio)
{
    uint32_t val, ofs;
    int ii;

    ofs = 0;
    for (ii = 0; ii < 2; ++ii)
    {
        /* Set correct default for sigdet */
        usb_mdio_write(mdio, 0x1f, (0x8080 + ofs), MDIO_USB3);
        val = usb_mdio_read(mdio, 0x05, MDIO_USB3);
        val = (val & ~0x800f) | 0x800d;
        usb_mdio_write(mdio, 0x05, val, MDIO_USB3);
        ofs = 0x1000;
    }
}

void usb3_enable_skip_align(uint32_t *mdio)
{
    uint32_t val, ofs;
    int ii;

    ofs = 0;
    for (ii = 0; ii < 2; ++ii)
    {
        /* Set correct default for SKIP align */
        usb_mdio_write(mdio, 0x1f, (0x8060 + ofs), MDIO_USB3);
        val = usb_mdio_read(mdio, 0x01, MDIO_USB3) | 0x200;
        usb_mdio_write(mdio, 0x01, val, MDIO_USB3);
        ofs = 0x1000;
    }
}

void usb2_eye_fix(uint32_t *mdio)
{
    /* Updating USB 2.0 PHY registers */
    usb_mdio_write(mdio, 0x1f, 0x80a0, MDIO_USB2);
    usb_mdio_write(mdio, 0x0a, 0xc6a0, MDIO_USB2);
}

