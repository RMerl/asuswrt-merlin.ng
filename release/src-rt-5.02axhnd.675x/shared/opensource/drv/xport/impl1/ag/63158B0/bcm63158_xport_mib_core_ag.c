/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "bcm63158_drivers_xport_ag.h"
#include "bcm63158_xport_mib_core_ag.h"
#include "xport_xlmac_indirect_access.h"
#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_xport_mib_core_grx64_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx64=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx64 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX64, COUNT40, reg_grx64, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX64, reg_grx64);

    return 0;
}

int ag_drv_xport_mib_core_grx64_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx64=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX64, reg_grx64);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX64, COUNT40, reg_grx64);

    return 0;
}

int ag_drv_xport_mib_core_grx127_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx127=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx127 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX127, COUNT40, reg_grx127, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX127, reg_grx127);

    return 0;
}

int ag_drv_xport_mib_core_grx127_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx127=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX127, reg_grx127);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX127, COUNT40, reg_grx127);

    return 0;
}

int ag_drv_xport_mib_core_grx255_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx255=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx255 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX255, COUNT40, reg_grx255, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX255, reg_grx255);

    return 0;
}

int ag_drv_xport_mib_core_grx255_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx255=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX255, reg_grx255);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX255, COUNT40, reg_grx255);

    return 0;
}

int ag_drv_xport_mib_core_grx511_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx511=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx511 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX511, COUNT40, reg_grx511, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX511, reg_grx511);

    return 0;
}

int ag_drv_xport_mib_core_grx511_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx511=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX511, reg_grx511);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX511, COUNT40, reg_grx511);

    return 0;
}

int ag_drv_xport_mib_core_grx1023_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx1023=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx1023 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX1023, COUNT40, reg_grx1023, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX1023, reg_grx1023);

    return 0;
}

int ag_drv_xport_mib_core_grx1023_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx1023=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX1023, reg_grx1023);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX1023, COUNT40, reg_grx1023);

    return 0;
}

int ag_drv_xport_mib_core_grx1518_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx1518=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx1518 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX1518, COUNT40, reg_grx1518, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX1518, reg_grx1518);

    return 0;
}

int ag_drv_xport_mib_core_grx1518_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx1518=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX1518, reg_grx1518);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX1518, COUNT40, reg_grx1518);

    return 0;
}

int ag_drv_xport_mib_core_grx1522_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx1522=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx1522 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX1522, COUNT40, reg_grx1522, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX1522, reg_grx1522);

    return 0;
}

int ag_drv_xport_mib_core_grx1522_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx1522=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX1522, reg_grx1522);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX1522, COUNT40, reg_grx1522);

    return 0;
}

int ag_drv_xport_mib_core_grx2047_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx2047=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx2047 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX2047, COUNT40, reg_grx2047, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX2047, reg_grx2047);

    return 0;
}

int ag_drv_xport_mib_core_grx2047_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx2047=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX2047, reg_grx2047);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX2047, COUNT40, reg_grx2047);

    return 0;
}

int ag_drv_xport_mib_core_grx4095_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx4095=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx4095 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX4095, COUNT40, reg_grx4095, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX4095, reg_grx4095);

    return 0;
}

int ag_drv_xport_mib_core_grx4095_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx4095=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX4095, reg_grx4095);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX4095, COUNT40, reg_grx4095);

    return 0;
}

int ag_drv_xport_mib_core_grx9216_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx9216=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx9216 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX9216, COUNT40, reg_grx9216, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX9216, reg_grx9216);

    return 0;
}

int ag_drv_xport_mib_core_grx9216_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx9216=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX9216, reg_grx9216);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX9216, COUNT40, reg_grx9216);

    return 0;
}

int ag_drv_xport_mib_core_grx16383_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grx16383=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grx16383 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRX16383, COUNT40, reg_grx16383, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRX16383, reg_grx16383);

    return 0;
}

int ag_drv_xport_mib_core_grx16383_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grx16383=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRX16383, reg_grx16383);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRX16383, COUNT40, reg_grx16383);

    return 0;
}

int ag_drv_xport_mib_core_grxpkt_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpkt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpkt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPKT, COUNT40, reg_grxpkt, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPKT, reg_grxpkt);

    return 0;
}

int ag_drv_xport_mib_core_grxpkt_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpkt=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPKT, reg_grxpkt);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPKT, COUNT40, reg_grxpkt);

    return 0;
}

int ag_drv_xport_mib_core_grxuca_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxuca=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxuca = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXUCA, COUNT40, reg_grxuca, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXUCA, reg_grxuca);

    return 0;
}

int ag_drv_xport_mib_core_grxuca_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxuca=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXUCA, reg_grxuca);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXUCA, COUNT40, reg_grxuca);

    return 0;
}

int ag_drv_xport_mib_core_grxmca_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxmca=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxmca = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXMCA, COUNT40, reg_grxmca, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXMCA, reg_grxmca);

    return 0;
}

int ag_drv_xport_mib_core_grxmca_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxmca=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXMCA, reg_grxmca);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXMCA, COUNT40, reg_grxmca);

    return 0;
}

int ag_drv_xport_mib_core_grxbca_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxbca=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxbca = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXBCA, COUNT40, reg_grxbca, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXBCA, reg_grxbca);

    return 0;
}

int ag_drv_xport_mib_core_grxbca_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxbca=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXBCA, reg_grxbca);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXBCA, COUNT40, reg_grxbca);

    return 0;
}

int ag_drv_xport_mib_core_grxfcs_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxfcs=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxfcs = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXFCS, COUNT40, reg_grxfcs, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXFCS, reg_grxfcs);

    return 0;
}

int ag_drv_xport_mib_core_grxfcs_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxfcs=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXFCS, reg_grxfcs);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXFCS, COUNT40, reg_grxfcs);

    return 0;
}

int ag_drv_xport_mib_core_grxcf_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxcf=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxcf = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXCF, COUNT40, reg_grxcf, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXCF, reg_grxcf);

    return 0;
}

int ag_drv_xport_mib_core_grxcf_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxcf=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXCF, reg_grxcf);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXCF, COUNT40, reg_grxcf);

    return 0;
}

int ag_drv_xport_mib_core_grxpf_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpf=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpf = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPF, COUNT40, reg_grxpf, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPF, reg_grxpf);

    return 0;
}

int ag_drv_xport_mib_core_grxpf_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpf=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPF, reg_grxpf);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPF, COUNT40, reg_grxpf);

    return 0;
}

int ag_drv_xport_mib_core_grxpp_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpp=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpp = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPP, COUNT40, reg_grxpp, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPP, reg_grxpp);

    return 0;
}

int ag_drv_xport_mib_core_grxpp_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpp=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPP, reg_grxpp);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPP, COUNT40, reg_grxpp);

    return 0;
}

int ag_drv_xport_mib_core_grxuo_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxuo=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxuo = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXUO, COUNT40, reg_grxuo, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXUO, reg_grxuo);

    return 0;
}

int ag_drv_xport_mib_core_grxuo_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxuo=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXUO, reg_grxuo);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXUO, COUNT40, reg_grxuo);

    return 0;
}

int ag_drv_xport_mib_core_grxuda_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxuda=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxuda = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXUDA, COUNT40, reg_grxuda, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXUDA, reg_grxuda);

    return 0;
}

int ag_drv_xport_mib_core_grxuda_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxuda=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXUDA, reg_grxuda);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXUDA, COUNT40, reg_grxuda);

    return 0;
}

int ag_drv_xport_mib_core_grxwsa_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxwsa=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxwsa = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXWSA, COUNT40, reg_grxwsa, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXWSA, reg_grxwsa);

    return 0;
}

int ag_drv_xport_mib_core_grxwsa_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxwsa=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXWSA, reg_grxwsa);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXWSA, COUNT40, reg_grxwsa);

    return 0;
}

int ag_drv_xport_mib_core_grxaln_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxaln=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxaln = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXALN, COUNT40, reg_grxaln, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXALN, reg_grxaln);

    return 0;
}

int ag_drv_xport_mib_core_grxaln_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxaln=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXALN, reg_grxaln);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXALN, COUNT40, reg_grxaln);

    return 0;
}

int ag_drv_xport_mib_core_grxflr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxflr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxflr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXFLR, COUNT40, reg_grxflr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXFLR, reg_grxflr);

    return 0;
}

int ag_drv_xport_mib_core_grxflr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxflr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXFLR, reg_grxflr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXFLR, COUNT40, reg_grxflr);

    return 0;
}

int ag_drv_xport_mib_core_grxfrerr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxfrerr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxfrerr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXFRERR, COUNT40, reg_grxfrerr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXFRERR, reg_grxfrerr);

    return 0;
}

int ag_drv_xport_mib_core_grxfrerr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxfrerr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXFRERR, reg_grxfrerr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXFRERR, COUNT40, reg_grxfrerr);

    return 0;
}

int ag_drv_xport_mib_core_grxfcr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxfcr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxfcr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXFCR, COUNT40, reg_grxfcr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXFCR, reg_grxfcr);

    return 0;
}

int ag_drv_xport_mib_core_grxfcr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxfcr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXFCR, reg_grxfcr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXFCR, COUNT40, reg_grxfcr);

    return 0;
}

int ag_drv_xport_mib_core_grxovr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxovr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxovr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXOVR, COUNT40, reg_grxovr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXOVR, reg_grxovr);

    return 0;
}

int ag_drv_xport_mib_core_grxovr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxovr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXOVR, reg_grxovr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXOVR, COUNT40, reg_grxovr);

    return 0;
}

int ag_drv_xport_mib_core_grxjbr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxjbr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxjbr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXJBR, COUNT40, reg_grxjbr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXJBR, reg_grxjbr);

    return 0;
}

int ag_drv_xport_mib_core_grxjbr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxjbr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXJBR, reg_grxjbr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXJBR, COUNT40, reg_grxjbr);

    return 0;
}

int ag_drv_xport_mib_core_grxmtue_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxmtue=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxmtue = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXMTUE, COUNT40, reg_grxmtue, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXMTUE, reg_grxmtue);

    return 0;
}

int ag_drv_xport_mib_core_grxmtue_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxmtue=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXMTUE, reg_grxmtue);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXMTUE, COUNT40, reg_grxmtue);

    return 0;
}

int ag_drv_xport_mib_core_grxmcrc_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxmcrc=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxmcrc = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXMCRC, COUNT40, reg_grxmcrc, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXMCRC, reg_grxmcrc);

    return 0;
}

int ag_drv_xport_mib_core_grxmcrc_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxmcrc=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXMCRC, reg_grxmcrc);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXMCRC, COUNT40, reg_grxmcrc);

    return 0;
}

int ag_drv_xport_mib_core_grxprm_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxprm=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxprm = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPRM, COUNT40, reg_grxprm, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPRM, reg_grxprm);

    return 0;
}

int ag_drv_xport_mib_core_grxprm_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxprm=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPRM, reg_grxprm);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPRM, COUNT40, reg_grxprm);

    return 0;
}

int ag_drv_xport_mib_core_grxvln_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxvln=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxvln = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXVLN, COUNT40, reg_grxvln, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXVLN, reg_grxvln);

    return 0;
}

int ag_drv_xport_mib_core_grxvln_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxvln=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXVLN, reg_grxvln);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXVLN, COUNT40, reg_grxvln);

    return 0;
}

int ag_drv_xport_mib_core_grxdvln_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxdvln=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxdvln = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXDVLN, COUNT40, reg_grxdvln, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXDVLN, reg_grxdvln);

    return 0;
}

int ag_drv_xport_mib_core_grxdvln_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxdvln=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXDVLN, reg_grxdvln);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXDVLN, COUNT40, reg_grxdvln);

    return 0;
}

int ag_drv_xport_mib_core_grxtrfu_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxtrfu=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxtrfu = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXTRFU, COUNT40, reg_grxtrfu, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXTRFU, reg_grxtrfu);

    return 0;
}

int ag_drv_xport_mib_core_grxtrfu_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxtrfu=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXTRFU, reg_grxtrfu);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXTRFU, COUNT40, reg_grxtrfu);

    return 0;
}

int ag_drv_xport_mib_core_grxpok_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpok=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpok = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPOK, COUNT40, reg_grxpok, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPOK, reg_grxpok);

    return 0;
}

int ag_drv_xport_mib_core_grxpok_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpok=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPOK, reg_grxpok);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPOK, COUNT40, reg_grxpok);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff0_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff0 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF0, COUNT40, reg_grxpfcoff0, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF0, reg_grxpfcoff0);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff0_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff0=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF0, reg_grxpfcoff0);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF0, COUNT40, reg_grxpfcoff0);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff1_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff1 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF1, COUNT40, reg_grxpfcoff1, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF1, reg_grxpfcoff1);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff1_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff1=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF1, reg_grxpfcoff1);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF1, COUNT40, reg_grxpfcoff1);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff2_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff2=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff2 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF2, COUNT40, reg_grxpfcoff2, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF2, reg_grxpfcoff2);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff2_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff2=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF2, reg_grxpfcoff2);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF2, COUNT40, reg_grxpfcoff2);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff3_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff3=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff3 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF3, COUNT40, reg_grxpfcoff3, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF3, reg_grxpfcoff3);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff3_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff3=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF3, reg_grxpfcoff3);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF3, COUNT40, reg_grxpfcoff3);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff4_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff4=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff4 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF4, COUNT40, reg_grxpfcoff4, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF4, reg_grxpfcoff4);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff4_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff4=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF4, reg_grxpfcoff4);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF4, COUNT40, reg_grxpfcoff4);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff5_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff5=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff5 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF5, COUNT40, reg_grxpfcoff5, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF5, reg_grxpfcoff5);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff5_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff5=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF5, reg_grxpfcoff5);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF5, COUNT40, reg_grxpfcoff5);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff6_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff6=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff6 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF6, COUNT40, reg_grxpfcoff6, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF6, reg_grxpfcoff6);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff6_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff6=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF6, reg_grxpfcoff6);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF6, COUNT40, reg_grxpfcoff6);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff7_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcoff7=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcoff7 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCOFF7, COUNT40, reg_grxpfcoff7, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCOFF7, reg_grxpfcoff7);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcoff7_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcoff7=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCOFF7, reg_grxpfcoff7);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCOFF7, COUNT40, reg_grxpfcoff7);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp0_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp0 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP0, COUNT40, reg_grxpfcp0, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP0, reg_grxpfcp0);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp0_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp0=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP0, reg_grxpfcp0);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP0, COUNT40, reg_grxpfcp0);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp1_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp1 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP1, COUNT40, reg_grxpfcp1, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP1, reg_grxpfcp1);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp1_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp1=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP1, reg_grxpfcp1);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP1, COUNT40, reg_grxpfcp1);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp2_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp2=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp2 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP2, COUNT40, reg_grxpfcp2, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP2, reg_grxpfcp2);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp2_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp2=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP2, reg_grxpfcp2);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP2, COUNT40, reg_grxpfcp2);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp3_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp3=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp3 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP3, COUNT40, reg_grxpfcp3, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP3, reg_grxpfcp3);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp3_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp3=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP3, reg_grxpfcp3);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP3, COUNT40, reg_grxpfcp3);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp4_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp4=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp4 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP4, COUNT40, reg_grxpfcp4, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP4, reg_grxpfcp4);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp4_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp4=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP4, reg_grxpfcp4);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP4, COUNT40, reg_grxpfcp4);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp5_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp5=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp5 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP5, COUNT40, reg_grxpfcp5, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP5, reg_grxpfcp5);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp5_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp5=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP5, reg_grxpfcp5);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP5, COUNT40, reg_grxpfcp5);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp6_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp6=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp6 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP6, COUNT40, reg_grxpfcp6, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP6, reg_grxpfcp6);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp6_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp6=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP6, reg_grxpfcp6);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP6, COUNT40, reg_grxpfcp6);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp7_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxpfcp7=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxpfcp7 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPFCP7, COUNT40, reg_grxpfcp7, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPFCP7, reg_grxpfcp7);

    return 0;
}

int ag_drv_xport_mib_core_grxpfcp7_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxpfcp7=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPFCP7, reg_grxpfcp7);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPFCP7, COUNT40, reg_grxpfcp7);

    return 0;
}

int ag_drv_xport_mib_core_grxschcrc_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxschcrc=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxschcrc = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXSCHCRC, COUNT40, reg_grxschcrc, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXSCHCRC, reg_grxschcrc);

    return 0;
}

int ag_drv_xport_mib_core_grxschcrc_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxschcrc=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXSCHCRC, reg_grxschcrc);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXSCHCRC, COUNT40, reg_grxschcrc);

    return 0;
}

int ag_drv_xport_mib_core_grxbyt_set(uint8_t port_id, uint64_t count48)
{
    uint64_t reg_grxbyt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count48 >= _48BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxbyt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXBYT, COUNT48, reg_grxbyt, (uint64_t)count48);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXBYT, reg_grxbyt);

    return 0;
}

int ag_drv_xport_mib_core_grxbyt_get(uint8_t port_id, uint64_t *count48)
{
    uint64_t reg_grxbyt=0;

#ifdef VALIDATE_PARMS
    if(!count48)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXBYT, reg_grxbyt);

    *count48 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXBYT, COUNT48, reg_grxbyt);

    return 0;
}

int ag_drv_xport_mib_core_grxrpkt_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxrpkt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxrpkt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXRPKT, COUNT40, reg_grxrpkt, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXRPKT, reg_grxrpkt);

    return 0;
}

int ag_drv_xport_mib_core_grxrpkt_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxrpkt=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXRPKT, reg_grxrpkt);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXRPKT, COUNT40, reg_grxrpkt);

    return 0;
}

int ag_drv_xport_mib_core_grxund_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxund=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxund = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXUND, COUNT40, reg_grxund, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXUND, reg_grxund);

    return 0;
}

int ag_drv_xport_mib_core_grxund_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxund=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXUND, reg_grxund);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXUND, COUNT40, reg_grxund);

    return 0;
}

int ag_drv_xport_mib_core_grxfrg_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxfrg=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxfrg = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXFRG, COUNT40, reg_grxfrg, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXFRG, reg_grxfrg);

    return 0;
}

int ag_drv_xport_mib_core_grxfrg_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxfrg=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXFRG, reg_grxfrg);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXFRG, COUNT40, reg_grxfrg);

    return 0;
}

int ag_drv_xport_mib_core_grxrbyt_set(uint8_t port_id, uint64_t count48)
{
    uint64_t reg_grxrbyt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count48 >= _48BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxrbyt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXRBYT, COUNT48, reg_grxrbyt, (uint64_t)count48);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXRBYT, reg_grxrbyt);

    return 0;
}

int ag_drv_xport_mib_core_grxrbyt_get(uint8_t port_id, uint64_t *count48)
{
    uint64_t reg_grxrbyt=0;

#ifdef VALIDATE_PARMS
    if(!count48)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXRBYT, reg_grxrbyt);

    *count48 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXRBYT, COUNT48, reg_grxrbyt);

    return 0;
}

int ag_drv_xport_mib_core_gtx64_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx64=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx64 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX64, COUNT40, reg_gtx64, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX64, reg_gtx64);

    return 0;
}

int ag_drv_xport_mib_core_gtx64_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx64=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX64, reg_gtx64);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX64, COUNT40, reg_gtx64);

    return 0;
}

int ag_drv_xport_mib_core_gtx127_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx127=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx127 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX127, COUNT40, reg_gtx127, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX127, reg_gtx127);

    return 0;
}

int ag_drv_xport_mib_core_gtx127_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx127=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX127, reg_gtx127);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX127, COUNT40, reg_gtx127);

    return 0;
}

int ag_drv_xport_mib_core_gtx255_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx255=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx255 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX255, COUNT40, reg_gtx255, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX255, reg_gtx255);

    return 0;
}

int ag_drv_xport_mib_core_gtx255_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx255=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX255, reg_gtx255);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX255, COUNT40, reg_gtx255);

    return 0;
}

int ag_drv_xport_mib_core_gtx511_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx511=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx511 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX511, COUNT40, reg_gtx511, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX511, reg_gtx511);

    return 0;
}

int ag_drv_xport_mib_core_gtx511_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx511=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX511, reg_gtx511);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX511, COUNT40, reg_gtx511);

    return 0;
}

int ag_drv_xport_mib_core_gtx1023_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx1023=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx1023 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX1023, COUNT40, reg_gtx1023, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX1023, reg_gtx1023);

    return 0;
}

int ag_drv_xport_mib_core_gtx1023_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx1023=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX1023, reg_gtx1023);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX1023, COUNT40, reg_gtx1023);

    return 0;
}

int ag_drv_xport_mib_core_gtx1518_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx1518=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx1518 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX1518, COUNT40, reg_gtx1518, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX1518, reg_gtx1518);

    return 0;
}

int ag_drv_xport_mib_core_gtx1518_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx1518=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX1518, reg_gtx1518);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX1518, COUNT40, reg_gtx1518);

    return 0;
}

int ag_drv_xport_mib_core_gtx1522_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx1522=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx1522 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX1522, COUNT40, reg_gtx1522, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX1522, reg_gtx1522);

    return 0;
}

int ag_drv_xport_mib_core_gtx1522_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx1522=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX1522, reg_gtx1522);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX1522, COUNT40, reg_gtx1522);

    return 0;
}

int ag_drv_xport_mib_core_gtx2047_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx2047=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx2047 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX2047, COUNT40, reg_gtx2047, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX2047, reg_gtx2047);

    return 0;
}

int ag_drv_xport_mib_core_gtx2047_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx2047=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX2047, reg_gtx2047);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX2047, COUNT40, reg_gtx2047);

    return 0;
}

int ag_drv_xport_mib_core_gtx4095_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx4095=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx4095 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX4095, COUNT40, reg_gtx4095, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX4095, reg_gtx4095);

    return 0;
}

int ag_drv_xport_mib_core_gtx4095_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx4095=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX4095, reg_gtx4095);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX4095, COUNT40, reg_gtx4095);

    return 0;
}

int ag_drv_xport_mib_core_gtx9216_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx9216=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx9216 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX9216, COUNT40, reg_gtx9216, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX9216, reg_gtx9216);

    return 0;
}

int ag_drv_xport_mib_core_gtx9216_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx9216=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX9216, reg_gtx9216);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX9216, COUNT40, reg_gtx9216);

    return 0;
}

int ag_drv_xport_mib_core_gtx16383_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtx16383=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtx16383 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTX16383, COUNT40, reg_gtx16383, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTX16383, reg_gtx16383);

    return 0;
}

int ag_drv_xport_mib_core_gtx16383_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtx16383=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTX16383, reg_gtx16383);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTX16383, COUNT40, reg_gtx16383);

    return 0;
}

int ag_drv_xport_mib_core_gtxpok_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpok=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpok = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPOK, COUNT40, reg_gtxpok, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPOK, reg_gtxpok);

    return 0;
}

int ag_drv_xport_mib_core_gtxpok_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpok=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPOK, reg_gtxpok);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPOK, COUNT40, reg_gtxpok);

    return 0;
}

int ag_drv_xport_mib_core_gtxpkt_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpkt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpkt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPKT, COUNT40, reg_gtxpkt, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPKT, reg_gtxpkt);

    return 0;
}

int ag_drv_xport_mib_core_gtxpkt_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpkt=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPKT, reg_gtxpkt);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPKT, COUNT40, reg_gtxpkt);

    return 0;
}

int ag_drv_xport_mib_core_gtxuca_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxuca=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxuca = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXUCA, COUNT40, reg_gtxuca, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXUCA, reg_gtxuca);

    return 0;
}

int ag_drv_xport_mib_core_gtxuca_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxuca=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXUCA, reg_gtxuca);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXUCA, COUNT40, reg_gtxuca);

    return 0;
}

int ag_drv_xport_mib_core_gtxmca_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxmca=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxmca = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXMCA, COUNT40, reg_gtxmca, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXMCA, reg_gtxmca);

    return 0;
}

int ag_drv_xport_mib_core_gtxmca_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxmca=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXMCA, reg_gtxmca);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXMCA, COUNT40, reg_gtxmca);

    return 0;
}

int ag_drv_xport_mib_core_gtxbca_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxbca=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxbca = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXBCA, COUNT40, reg_gtxbca, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXBCA, reg_gtxbca);

    return 0;
}

int ag_drv_xport_mib_core_gtxbca_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxbca=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXBCA, reg_gtxbca);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXBCA, COUNT40, reg_gtxbca);

    return 0;
}

int ag_drv_xport_mib_core_gtxpf_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpf=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpf = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPF, COUNT40, reg_gtxpf, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPF, reg_gtxpf);

    return 0;
}

int ag_drv_xport_mib_core_gtxpf_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpf=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPF, reg_gtxpf);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPF, COUNT40, reg_gtxpf);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfc_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfc=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfc = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFC, COUNT40, reg_gtxpfc, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFC, reg_gtxpfc);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfc_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfc=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFC, reg_gtxpfc);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFC, COUNT40, reg_gtxpfc);

    return 0;
}

int ag_drv_xport_mib_core_gtxjbr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxjbr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxjbr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXJBR, COUNT40, reg_gtxjbr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXJBR, reg_gtxjbr);

    return 0;
}

int ag_drv_xport_mib_core_gtxjbr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxjbr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXJBR, reg_gtxjbr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXJBR, COUNT40, reg_gtxjbr);

    return 0;
}

int ag_drv_xport_mib_core_gtxfcs_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxfcs=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxfcs = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXFCS, COUNT40, reg_gtxfcs, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXFCS, reg_gtxfcs);

    return 0;
}

int ag_drv_xport_mib_core_gtxfcs_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxfcs=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXFCS, reg_gtxfcs);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXFCS, COUNT40, reg_gtxfcs);

    return 0;
}

int ag_drv_xport_mib_core_gtxcf_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxcf=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxcf = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXCF, COUNT40, reg_gtxcf, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXCF, reg_gtxcf);

    return 0;
}

int ag_drv_xport_mib_core_gtxcf_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxcf=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXCF, reg_gtxcf);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXCF, COUNT40, reg_gtxcf);

    return 0;
}

int ag_drv_xport_mib_core_gtxovr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxovr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxovr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXOVR, COUNT40, reg_gtxovr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXOVR, reg_gtxovr);

    return 0;
}

int ag_drv_xport_mib_core_gtxovr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxovr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXOVR, reg_gtxovr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXOVR, COUNT40, reg_gtxovr);

    return 0;
}

int ag_drv_xport_mib_core_gtxdfr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxdfr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxdfr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXDFR, COUNT40, reg_gtxdfr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXDFR, reg_gtxdfr);

    return 0;
}

int ag_drv_xport_mib_core_gtxdfr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxdfr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXDFR, reg_gtxdfr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXDFR, COUNT40, reg_gtxdfr);

    return 0;
}

int ag_drv_xport_mib_core_gtxedf_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxedf=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxedf = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXEDF, COUNT40, reg_gtxedf, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXEDF, reg_gtxedf);

    return 0;
}

int ag_drv_xport_mib_core_gtxedf_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxedf=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXEDF, reg_gtxedf);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXEDF, COUNT40, reg_gtxedf);

    return 0;
}

int ag_drv_xport_mib_core_gtxscl_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxscl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxscl = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXSCL, COUNT40, reg_gtxscl, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXSCL, reg_gtxscl);

    return 0;
}

int ag_drv_xport_mib_core_gtxscl_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxscl=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXSCL, reg_gtxscl);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXSCL, COUNT40, reg_gtxscl);

    return 0;
}

int ag_drv_xport_mib_core_gtxmcl_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxmcl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxmcl = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXMCL, COUNT40, reg_gtxmcl, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXMCL, reg_gtxmcl);

    return 0;
}

int ag_drv_xport_mib_core_gtxmcl_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxmcl=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXMCL, reg_gtxmcl);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXMCL, COUNT40, reg_gtxmcl);

    return 0;
}

int ag_drv_xport_mib_core_gtxlcl_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxlcl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxlcl = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXLCL, COUNT40, reg_gtxlcl, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXLCL, reg_gtxlcl);

    return 0;
}

int ag_drv_xport_mib_core_gtxlcl_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxlcl=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXLCL, reg_gtxlcl);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXLCL, COUNT40, reg_gtxlcl);

    return 0;
}

int ag_drv_xport_mib_core_gtxxcl_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxxcl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxxcl = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXXCL, COUNT40, reg_gtxxcl, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXXCL, reg_gtxxcl);

    return 0;
}

int ag_drv_xport_mib_core_gtxxcl_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxxcl=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXXCL, reg_gtxxcl);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXXCL, COUNT40, reg_gtxxcl);

    return 0;
}

int ag_drv_xport_mib_core_gtxfrg_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxfrg=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxfrg = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXFRG, COUNT40, reg_gtxfrg, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXFRG, reg_gtxfrg);

    return 0;
}

int ag_drv_xport_mib_core_gtxfrg_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxfrg=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXFRG, reg_gtxfrg);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXFRG, COUNT40, reg_gtxfrg);

    return 0;
}

int ag_drv_xport_mib_core_gtxerr_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxerr=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxerr = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXERR, COUNT40, reg_gtxerr, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXERR, reg_gtxerr);

    return 0;
}

int ag_drv_xport_mib_core_gtxerr_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxerr=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXERR, reg_gtxerr);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXERR, COUNT40, reg_gtxerr);

    return 0;
}

int ag_drv_xport_mib_core_gtxvln_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxvln=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxvln = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXVLN, COUNT40, reg_gtxvln, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXVLN, reg_gtxvln);

    return 0;
}

int ag_drv_xport_mib_core_gtxvln_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxvln=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXVLN, reg_gtxvln);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXVLN, COUNT40, reg_gtxvln);

    return 0;
}

int ag_drv_xport_mib_core_gtxdvln_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxdvln=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxdvln = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXDVLN, COUNT40, reg_gtxdvln, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXDVLN, reg_gtxdvln);

    return 0;
}

int ag_drv_xport_mib_core_gtxdvln_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxdvln=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXDVLN, reg_gtxdvln);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXDVLN, COUNT40, reg_gtxdvln);

    return 0;
}

int ag_drv_xport_mib_core_gtxrpkt_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxrpkt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxrpkt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXRPKT, COUNT40, reg_gtxrpkt, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXRPKT, reg_gtxrpkt);

    return 0;
}

int ag_drv_xport_mib_core_gtxrpkt_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxrpkt=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXRPKT, reg_gtxrpkt);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXRPKT, COUNT40, reg_gtxrpkt);

    return 0;
}

int ag_drv_xport_mib_core_gtxufl_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxufl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxufl = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXUFL, COUNT40, reg_gtxufl, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXUFL, reg_gtxufl);

    return 0;
}

int ag_drv_xport_mib_core_gtxufl_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxufl=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXUFL, reg_gtxufl);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXUFL, COUNT40, reg_gtxufl);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp0_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp0 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP0, COUNT40, reg_gtxpfcp0, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP0, reg_gtxpfcp0);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp0_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp0=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP0, reg_gtxpfcp0);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP0, COUNT40, reg_gtxpfcp0);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp1_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp1 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP1, COUNT40, reg_gtxpfcp1, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP1, reg_gtxpfcp1);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp1_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp1=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP1, reg_gtxpfcp1);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP1, COUNT40, reg_gtxpfcp1);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp2_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp2=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp2 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP2, COUNT40, reg_gtxpfcp2, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP2, reg_gtxpfcp2);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp2_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp2=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP2, reg_gtxpfcp2);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP2, COUNT40, reg_gtxpfcp2);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp3_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp3=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp3 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP3, COUNT40, reg_gtxpfcp3, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP3, reg_gtxpfcp3);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp3_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp3=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP3, reg_gtxpfcp3);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP3, COUNT40, reg_gtxpfcp3);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp4_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp4=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp4 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP4, COUNT40, reg_gtxpfcp4, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP4, reg_gtxpfcp4);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp4_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp4=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP4, reg_gtxpfcp4);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP4, COUNT40, reg_gtxpfcp4);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp5_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp5=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp5 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP5, COUNT40, reg_gtxpfcp5, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP5, reg_gtxpfcp5);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp5_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp5=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP5, reg_gtxpfcp5);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP5, COUNT40, reg_gtxpfcp5);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp6_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp6=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp6 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP6, COUNT40, reg_gtxpfcp6, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP6, reg_gtxpfcp6);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp6_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp6=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP6, reg_gtxpfcp6);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP6, COUNT40, reg_gtxpfcp6);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp7_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxpfcp7=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxpfcp7 = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXPFCP7, COUNT40, reg_gtxpfcp7, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXPFCP7, reg_gtxpfcp7);

    return 0;
}

int ag_drv_xport_mib_core_gtxpfcp7_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxpfcp7=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXPFCP7, reg_gtxpfcp7);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXPFCP7, COUNT40, reg_gtxpfcp7);

    return 0;
}

int ag_drv_xport_mib_core_gtxncl_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxncl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxncl = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXNCL, COUNT40, reg_gtxncl, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXNCL, reg_gtxncl);

    return 0;
}

int ag_drv_xport_mib_core_gtxncl_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxncl=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXNCL, reg_gtxncl);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXNCL, COUNT40, reg_gtxncl);

    return 0;
}

int ag_drv_xport_mib_core_gtxbyt_set(uint8_t port_id, uint64_t count48)
{
    uint64_t reg_gtxbyt=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count48 >= _48BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxbyt = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXBYT, COUNT48, reg_gtxbyt, (uint64_t)count48);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXBYT, reg_gtxbyt);

    return 0;
}

int ag_drv_xport_mib_core_gtxbyt_get(uint8_t port_id, uint64_t *count48)
{
    uint64_t reg_gtxbyt=0;

#ifdef VALIDATE_PARMS
    if(!count48)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXBYT, reg_gtxbyt);

    *count48 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXBYT, COUNT48, reg_gtxbyt);

    return 0;
}

int ag_drv_xport_mib_core_grxlpi_set(uint8_t port_id, uint32_t count32)
{
    uint64_t reg_grxlpi=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxlpi = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXLPI, COUNT32, reg_grxlpi, (uint64_t)count32);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXLPI, reg_grxlpi);

    return 0;
}

int ag_drv_xport_mib_core_grxlpi_get(uint8_t port_id, uint32_t *count32)
{
    uint64_t reg_grxlpi=0;

#ifdef VALIDATE_PARMS
    if(!count32)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXLPI, reg_grxlpi);

    *count32 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXLPI, COUNT32, reg_grxlpi);

    return 0;
}

int ag_drv_xport_mib_core_grxdlpi_set(uint8_t port_id, uint32_t count32)
{
    uint64_t reg_grxdlpi=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxdlpi = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXDLPI, COUNT32, reg_grxdlpi, (uint64_t)count32);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXDLPI, reg_grxdlpi);

    return 0;
}

int ag_drv_xport_mib_core_grxdlpi_get(uint8_t port_id, uint32_t *count32)
{
    uint64_t reg_grxdlpi=0;

#ifdef VALIDATE_PARMS
    if(!count32)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXDLPI, reg_grxdlpi);

    *count32 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXDLPI, COUNT32, reg_grxdlpi);

    return 0;
}

int ag_drv_xport_mib_core_gtxlpi_set(uint8_t port_id, uint32_t count32)
{
    uint64_t reg_gtxlpi=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxlpi = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXLPI, COUNT32, reg_gtxlpi, (uint64_t)count32);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXLPI, reg_gtxlpi);

    return 0;
}

int ag_drv_xport_mib_core_gtxlpi_get(uint8_t port_id, uint32_t *count32)
{
    uint64_t reg_gtxlpi=0;

#ifdef VALIDATE_PARMS
    if(!count32)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXLPI, reg_gtxlpi);

    *count32 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXLPI, COUNT32, reg_gtxlpi);

    return 0;
}

int ag_drv_xport_mib_core_gtxdlpi_set(uint8_t port_id, uint32_t count32)
{
    uint64_t reg_gtxdlpi=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxdlpi = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXDLPI, COUNT32, reg_gtxdlpi, (uint64_t)count32);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXDLPI, reg_gtxdlpi);

    return 0;
}

int ag_drv_xport_mib_core_gtxdlpi_get(uint8_t port_id, uint32_t *count32)
{
    uint64_t reg_gtxdlpi=0;

#ifdef VALIDATE_PARMS
    if(!count32)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXDLPI, reg_gtxdlpi);

    *count32 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXDLPI, COUNT32, reg_gtxdlpi);

    return 0;
}

int ag_drv_xport_mib_core_grxptllfc_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxptllfc=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxptllfc = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXPTLLFC, COUNT40, reg_grxptllfc, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXPTLLFC, reg_grxptllfc);

    return 0;
}

int ag_drv_xport_mib_core_grxptllfc_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxptllfc=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXPTLLFC, reg_grxptllfc);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXPTLLFC, COUNT40, reg_grxptllfc);

    return 0;
}

int ag_drv_xport_mib_core_grxltllfc_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxltllfc=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxltllfc = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXLTLLFC, COUNT40, reg_grxltllfc, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXLTLLFC, reg_grxltllfc);

    return 0;
}

int ag_drv_xport_mib_core_grxltllfc_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxltllfc=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXLTLLFC, reg_grxltllfc);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXLTLLFC, COUNT40, reg_grxltllfc);

    return 0;
}

int ag_drv_xport_mib_core_grxllfcfcs_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_grxllfcfcs=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_grxllfcfcs = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GRXLLFCFCS, COUNT40, reg_grxllfcfcs, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GRXLLFCFCS, reg_grxllfcfcs);

    return 0;
}

int ag_drv_xport_mib_core_grxllfcfcs_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_grxllfcfcs=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GRXLLFCFCS, reg_grxllfcfcs);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GRXLLFCFCS, COUNT40, reg_grxllfcfcs);

    return 0;
}

int ag_drv_xport_mib_core_gtxltllfc_set(uint8_t port_id, uint64_t count40)
{
    uint64_t reg_gtxltllfc=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (count40 >= _40BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gtxltllfc = RU_FIELD_SET(port_id, XPORT_MIB_CORE, GTXLTLLFC, COUNT40, reg_gtxltllfc, (uint64_t)count40);

    XPORT_MIB_INDIRECT_WRITE(port_id, XPORT_MIB_CORE, GTXLTLLFC, reg_gtxltllfc);

    return 0;
}

int ag_drv_xport_mib_core_gtxltllfc_get(uint8_t port_id, uint64_t *count40)
{
    uint64_t reg_gtxltllfc=0;

#ifdef VALIDATE_PARMS
    if(!count40)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_MIB_INDIRECT_READ(port_id, XPORT_MIB_CORE, GTXLTLLFC, reg_gtxltllfc);

    *count40 = RU_FIELD_GET(port_id, XPORT_MIB_CORE, GTXLTLLFC, COUNT40, reg_gtxltllfc);

    return 0;
}

