/***********************************************************************
 *
 * Copyright (c) 2021  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2021:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************************/
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include "macsec_common.h"
#include "macsec_defs.h"
#include "macsec_dev.h"
#include "soc/mcm/memregs.h"
#include "bchp_regs_int.h"
#include "xflow_macsec_cfg_params.h"
#include "xflow_macsec_esw_defs.h"

#include "pmc_xport.h"
#include "pmc_ethtop.h"

#include "dt_access.h"

#define DRIVER_AUTHOR "Nikolai Iosifov <nikolai.iosifov@broadcom.com>"
#define DRIVER_DESC   "Broadcom MACSEC driver"

extern int macsec_proc_init(void);
/*
 * Setup the TDM data bandwidth calendar table.
 */
static int macsec_fl_setup_tdm(int unit)
{
#define SBUS_PORT 49
    int rv, x, i, index = 0;
    uint32_t rval, sbus_val = SBUS_PORT;
    uint32_t port_list[SOC_MAX_NUM_DEVICES] = {0, 2, 1, 3};

    for (x = 0; x < SOC_MAX_NUM_DEVICES; x++)
    {
        rv = soc_mem_write(unit, MACSEC_TDM_CALENDARm, MEM_BLOCK_ALL, index, (void *)&port_list[x]);

        index++;

        for (i = 0; i < 6; i++)
        {
            rv = soc_mem_write(unit, MACSEC_TDM_CALENDARm, MEM_BLOCK_ALL, index, (void *)&sbus_val);
            index++;
        }
    }
    soc_ubus_reg32_get(unit, MACSEC_TDM_WRAP_PTRreg, REG_PORT_ANY, &rval);
    soc_ubus_reg_field_set(unit, MACSEC_TDM_WRAP_PTRreg, &rval, ACTIVE_TDM_CAL_fld, 0);
//    soc_ubus_reg_field_set(unit, MACSEC_TDM_WRAP_PTRreg, &rval, TDM_WRAP_PTR_fld, index - 1); // CHANGED_68880 // Determines the number of entries that the TDM calendar will have for a particular SKU before rolling over. Reset value is 0x30.
    soc_ubus_reg_field_set(unit, MACSEC_TDM_WRAP_PTRreg, &rval, TDM_WRAP_PTR_fld, 0x1b);
    soc_ubus_reg32_set(unit, MACSEC_TDM_WRAP_PTRreg, REG_PORT_ANY, rval);

    return rv; 
}

static int macsec_fl_init(int unit)
{
    int rv;
    uint32_t rval;
    uint64_t val64;

    /*
     * Before changing and MACSEC hardware settings, assert
     * the XLMAC soft reset
     */
//     xlmac_enable_core_soft_reset(mdev, mdev->macsec_port, 1);

    /* Enable MACSEC register accesses */
    soc_ubus_reg32_get(unit, MACSEC_CNTRLreg, REG_PORT_ANY, &rval);
    soc_ubus_reg_field_set(unit, MACSEC_CNTRLreg, &rval, egress_bypass_en_fld, 0);
    soc_ubus_reg_field_set(unit, MACSEC_CNTRLreg, &rval, ingress_bypass_en_fld, 0);
    soc_ubus_reg32_set(unit, MACSEC_CNTRLreg, REG_PORT_ANY, rval);

    /* Initialize SOC specific general MACSEC hardware, including MACSEC memories */
    rv = soc_macsec_hw_init(unit, 0, 0);
    if (rv != BCM_E_NONE)
    {
        PR_ERR("soc_macsec_hw_init() FAILED! rv = %d\n", rv);
        goto error_exit;
    }

    _bcm_xflow_macsec_init(unit);

    rv = macsec_fl_setup_tdm(unit);
    if (rv != BCM_E_NONE)
    {
        PR_ERR("macsec_fl_setup_tdm() FAILED! rv = %d\n", rv);
        goto error_exit;
    }

    /* Configure MGMT packet EtherType */
    rv = xflow_macsec_firelight_mac_addr_control_set(unit, 0, xflowMacsecMgmtDstMacEthertype0, NULL, MACSEC_CONFIG_EAPOL_MGMT_PKT_TYPE);
    if (rv != BCM_E_NONE)
    {
        PR_ERR("Configuration of the MGMT packet EtherType FAILED! rv = %d\n", rv);
        goto error_exit;
    }

    /* Disable Ingress SVTAG insertion */
    soc_ubus_reg32_get(unit, ISEC_SVTAG_CTRLreg, REG_PORT_ANY, &rval);
    soc_ubus_reg_field_set(unit, ISEC_SVTAG_CTRLreg, &rval, INS_EN_fld, 0);
    soc_ubus_reg_field_set(unit, ISEC_SVTAG_CTRLreg, &rval, TPID_fld, CMBB_SVTAG_TPID);
    soc_ubus_reg32_set(unit, ISEC_SVTAG_CTRLreg, REG_PORT_ANY, rval);

    /* Configure Egress SVTAG type */
    COMPILER_64_SET(val64, 0x0, CMBB_SVTAG_TPID);
    rv = xflow_macsec_firelight_control_set(unit, XFLOW_MACSEC_ENCRYPT, xflowMacsecControlSVTagTPIDEtype, val64);
    if (rv != BCM_E_NONE)
    {
        PR_ERR("Configuration of the Egress SVTAG type FAILED! rv = %d\n", rv);
        goto error_exit;
    }
    COMPILER_64_SET(val64, 0x0, 1);
    rv = xflow_macsec_firelight_control_set(unit, XFLOW_MACSEC_ENCRYPT, xflowMacsecControlSVTagEnable, val64);
    if (rv != BCM_E_NONE)
    {
        PR_ERR("xflow_macsec_firelight_control_set() (TAG_ENABLE) FAILED! rv = %d\n", rv);
        goto error_exit;
    }

error_exit:

    /*
     * After changing and MACSEC hardware settings, de-assert
     * the XLMAC soft reset
     */
//     xlmac_enable_core_soft_reset(mdev, mdev->macsec_port, 0);

    return rv;
}

static void macsec_fl_init_dev(macsec_dev_t *mdev, int unit, int port)
{
    mdev->macsec_unit = unit;
    mdev->macsec_port = port;
//    mdev->policy_id_decrypt = XFLOW_MACSEC_POLICY_ID_CREATE(xflowMacsecIdTypePolicy, mdev->macsec_port);
//    mdev->chan_id_decrypt = BCM_XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(BCM_XFLOW_MACSEC_DECRYPT, mdev->macsec_port);
	mdev->chan_id_encrypt = BCM_XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(BCM_XFLOW_MACSEC_ENCRYPT, mdev->macsec_port);
//    mdev->flow_id_decrypt = BCM_XFLOW_MACSEC_DECRYPT_FLOW_ID_CREATE(mdev->macsec_port);
//    mdev->assoc_id_decrypt = { 0, 0, 0, 0 };
//    mdev->assoc_id_encrypt = { 0, 0 };
//        BCM_XFLOW_MACSEC_SECURE_ASSOC_ID_CREATE(BCM_XFLOW_MACSEC_ENCRYPT, mdev->macsec_port);
    mdev->assoc_num = 0;
    mdev->include_sci = 1;
    COMPILER_64_SET(mdev->egress_sci, 0, 0);
    COMPILER_64_SET(mdev->egress_sci_mask, 0xffffffff, 0xffffffff);
    COMPILER_64_SET(mdev->ingress_sci, 0, 0);
    COMPILER_64_SET(mdev->ingress_sci_mask, 0xffffffff, 0xffffffff);

    printk("macsec_fl_init_dev macsec_unit=%d, macsec_port=%d\n", mdev->macsec_unit, mdev->macsec_port);

//    xflow_macsec_firelight_port_control_set(mdev->macsec_unit, BCM_XFLOW_MACSEC_DECRYPT, mdev->macsec_port,
//        xflowMacsecPortMgmtDefaultSubPort, mdev->macsec_port);
}

static int macsec_fl_port_get(int mac_port)
{
    int i = - 1;

    for (i = 0; i < CMBB_FL_MACSEC_MAX_PORT_NUM; i++)
    {
        if (msec_devs[FL_UNIT][i].mac_port == mac_port)
            return i;
    }
    return i;
}

int macsec_fl_port_enabled(int mac_port)
{
    int i;

    i = macsec_fl_port_get(mac_port);
    if (i != -1)
        return msec_devs[FL_UNIT][i].enabled ? i : -1;
    return -1;
}

int macsec_fl_port_init(int macsec_port)
{
    macsec_dev_t *mdev;
    uint32_t rval;

    macsec_port = macsec_fl_port_get(macsec_port);

    mdev = &msec_devs[FL_UNIT][macsec_port];
    /* Release soft reset for the specified MACSEC port */
    soc_ubus_reg32_get(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, &rval);
    soc_ubus_reg_field32_set(mdev->macsec_unit, MACSEC_CTRLreg, &rval, SOFT_RESET_fld, 0);
    if (msec_devs[FL_UNIT][macsec_port].enabled)
        soc_ubus_reg_field32_set(mdev->macsec_unit, MACSEC_CTRLreg, &rval, BYPASS_EN_fld, 0);
    
    soc_ubus_reg32_set(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, rval);

    udelay(100);
    return 0;
}

int macsec_fl_port_reset(int macsec_port)
{
    macsec_dev_t *mdev;
    uint32_t rval;

    macsec_port = macsec_fl_port_get(macsec_port);

    mdev = &msec_devs[FL_UNIT][macsec_port];
    /* Release soft reset for the specified MACSEC port */
    soc_ubus_reg32_get(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, &rval);
    soc_ubus_reg_field32_set(mdev->macsec_unit, MACSEC_CTRLreg, &rval, SOFT_RESET_fld, 1);
    if (msec_devs[FL_UNIT][macsec_port].enabled)
        soc_ubus_reg_field32_set(mdev->macsec_unit, MACSEC_CTRLreg, &rval, BYPASS_EN_fld, 1);
    soc_ubus_reg32_set(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, rval);

    udelay(100);
    return 0;
}

void bcm_macsec_port_set(int macsec_port, int mac_port)
{
    if (macsec_port >= 0 && macsec_port < CMBB_FL_MACSEC_MAX_PORT_NUM)
    {
        msec_devs[FL_UNIT][macsec_port].mac_port = mac_port;
    } 
}
EXPORT_SYMBOL(bcm_macsec_port_set);

static void macsec_fl_init_devs(int unit)
{
    int i;

    for (i = 0; i < CMBB_FL_MACSEC_MAX_PORT_NUM; i++)
        macsec_fl_init_dev(&msec_devs[unit][i], unit, i);
}

static int bcm_macsec_probe(dt_device_t *pdev)
{
    int ret;

    virt_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(virt_base))
    {
        ret = PTR_ERR(virt_base);
        virt_base = NULL;
        goto exit;
    }

    if (pmc_ethtop_power_up(ETHTOP_MDIO))
    {
        printk("Failed to power up mdio HW block!\n");
        goto exit;
    }

    pmc_xport_power_on(0);

    if ((ret = soc_attach(FL_UNIT)))
        goto exit;

    macsec_fl_init(FL_UNIT);
    macsec_fl_init_devs(FL_UNIT);
    macsec_proc_init();

    dev_info(&pdev->dev, "registered\n");

    return 0;

exit:
    printk("Failed to register MACSEC FL driver ret=%d\n", ret);
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,xflow-macsec-firelight" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-xflow-macsec-firelight",
        .of_match_table = of_platform_table,
    },
    .probe = bcm_macsec_probe,
};
module_platform_driver(of_platform_driver);



MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
