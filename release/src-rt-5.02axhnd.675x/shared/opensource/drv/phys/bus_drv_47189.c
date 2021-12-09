/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
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


/*
 * Bus driver for BCM47189. Implements all c22 and c45 operations over MDIO
 * (GMAC).
 * This includes two classes:
 *   - bus_47189_gmac0_drv: for MDIO access through GMAC0
 *   - bus_47189_gmac0_drv: for MDIO access through GMAC1
 *
 * They are both identical except for the target GMAC.
 *
 * NOTE: Possible future improvement
 * An additional private data field in the bus_drv_t class could encode the GMAC
 * core number so that one single class definition can be used for both cores.
 */

#include "bus_drv.h"
#include "mdio_drv_impl3.h"

static int bus_47189_gmac0_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(0, addr, reg, val);
}

static int bus_47189_gmac0_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(0, addr, reg, val);
}

static int bus_47189_gmac0_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(0, addr, dev, reg, val);
}

static int bus_47189_gmac0_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(0, addr, dev, reg, val);
}

bus_drv_t bus_47189_gmac0_drv =
{
    .c22_read = bus_47189_gmac0_c22_read,
    .c22_write = bus_47189_gmac0_c22_write,
    .c45_read = bus_47189_gmac0_c45_read,
    .c45_write = bus_47189_gmac0_c45_write,
};


static int bus_47189_gmac1_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(1, addr, reg, val);
}

static int bus_47189_gmac1_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(1, addr, reg, val);
}

static int bus_47189_gmac1_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(1, addr, dev, reg, val);
}

static int bus_47189_gmac1_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(1, addr, dev, reg, val);
}

bus_drv_t bus_47189_gmac1_drv =
{
    .c22_read = bus_47189_gmac1_c22_read,
    .c22_write = bus_47189_gmac1_c22_write,
    .c45_read = bus_47189_gmac1_c45_read,
    .c45_write = bus_47189_gmac1_c45_write,
};
