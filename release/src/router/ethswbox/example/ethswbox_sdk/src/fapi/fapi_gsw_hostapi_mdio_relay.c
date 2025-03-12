/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

#include <os_types.h>
#include <os_linux.h>
#include <os_linux.h>
#include <gsw_device.h>
#include <host_adapt.h>
#include <mdio_relay.h>
#include <gsw_cli_common.h>

#define lif_id 0
#define NUM_TC 16
#define MAX_NUM_OF_DISPLAY_PORTS 2

/* read internal GPHY MDIO/MMD registers */
int fapi_int_gphy_read(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    int ret;
    struct mdio_relay_data param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "phy", sizeof(param.phy), &param.phy);
    if (rret < 1)
    {
        printf("parameter not Found: phy\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mmd", sizeof(param.mmd), &param.mmd);
    if (rret < 1)
    {
        printf("parameter not Found: mmd\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "reg", sizeof(param.reg), &param.reg);
    if (rret < 1)
    {
        printf("parameter not Found: reg\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = int_gphy_read(gsw_dev, &param);

    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_int_gphy_read failed with ret code", ret);
    else
        printf("fapi_int_gphy_read:\tphy=0x%x mmd=0x%x reg=0x%x ret=0x%x\n", param.reg, param.mmd, param.reg, param.data);
    return 0;
}

/* write internal GPHY MDIO/MMD registers */
int fapi_int_gphy_write(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    int ret;
    struct mdio_relay_data param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "phy", sizeof(param.phy), &param.phy);
    if (rret < 1)
    {
        printf("parameter not Found: phy\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mmd", sizeof(param.mmd), &param.mmd);
    if (rret < 1)
    {
        printf("parameter not Found: mmd\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "reg", sizeof(param.reg), &param.reg);
    if (rret < 1)
    {
        printf("parameter not Found: reg\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "data", sizeof(param.data), &param.data);
    if (rret < 1)
    {
        printf("parameter not Found: data\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = int_gphy_write(gsw_dev, &param);

    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_int_gphy_write failed with ret code", ret);
    else
        printf("fapi_int_gphy_write:\tphy=0x%x mmd=0x%x reg=0x%x data=0x%x\n", param.reg, param.mmd, param.reg, param.data);
    return 0;
}

// /* modify internal GPHY MDIO/MMD registers */
int fapi_int_gphy_mod(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    int ret;
    struct mdio_relay_mod_data param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "phy", sizeof(param.phy), &param.phy);
    if (rret < 1)
    {
        printf("parameter not Found: phy\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mmd", sizeof(param.mmd), &param.mmd);
    if (rret < 1)
    {
        printf("parameter not Found: mmd\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "reg", sizeof(param.reg), &param.reg);
    if (rret < 1)
    {
        printf("parameter not Found: reg\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "data", sizeof(param.data), &param.data);
    if (rret < 1)
    {
        printf("parameter not Found: data\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mask", sizeof(param.mask), &param.mask);
    if (rret < 1)
    {
        printf("parameter not Found: mask\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = int_gphy_mod(gsw_dev, &param);

    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_int_gphy_mod failed with ret code", ret);
    else
        printf("fapi_int_gphy_mod:\tphy=0x%x mmd=0x%x reg=0x%x data=0x%x mask=0x%x\n", param.reg, param.mmd, param.reg, param.data, param.mask);
    return 0;
}

/* read external GPHY MDIO/MMD registers via MDIO bus */
int fapi_ext_mdio_read(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    int ret;
    struct mdio_relay_data param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "phy", sizeof(param.phy), &param.phy);
    if (rret < 1)
    {
        printf("parameter not Found: phy\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mmd", sizeof(param.mmd), &param.mmd);
    if (rret < 1)
    {
        printf("parameter not Found: mmd\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "reg", sizeof(param.reg), &param.reg);
    if (rret < 1)
    {
        printf("parameter not Found: reg\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = ext_mdio_read(gsw_dev, &param);

    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_ext_mdio_read failed with ret code", ret);
    else
        printf("fapi_ext_mdio_read:\tphy=0x%x mmd=0x%x reg=0x%x ret=0x%x\n", param.reg, param.mmd, param.reg, param.data);
    return 0;
}
/* write external GPHY MDIO/MMD registers via MDIO bus */
int fapi_ext_mdio_write(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    int ret;
    struct mdio_relay_data param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "phy", sizeof(param.phy), &param.phy);
    if (rret < 1)
    {
        printf("parameter not Found: phy\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mmd", sizeof(param.mmd), &param.mmd);
    if (rret < 1)
    {
        printf("parameter not Found: mmd\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "reg", sizeof(param.reg), &param.reg);
    if (rret < 1)
    {
        printf("parameter not Found: reg\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "data", sizeof(param.data), &param.data);
    if (rret < 1)
    {
        printf("parameter not Found: data\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = ext_mdio_write(gsw_dev, &param);

    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_ext_mdio_write failed with ret code", ret);
    else
        printf("fapi_ext_mdio_write:\tphy=0x%x mmd=0x%x reg=0x%x data=0x%x\n", param.reg, param.mmd, param.reg, param.data);
    return 0;
}
/* modify external GPHY MDIO/MMD registers via MDIO bus */
int fapi_ext_mdio_mod(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    int ret;
    struct mdio_relay_mod_data param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "phy", sizeof(param.phy), &param.phy);
    if (rret < 1)
    {
        printf("parameter not Found: phy\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mmd", sizeof(param.mmd), &param.mmd);
    if (rret < 1)
    {
        printf("parameter not Found: mmd\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "reg", sizeof(param.reg), &param.reg);
    if (rret < 1)
    {
        printf("parameter not Found: reg\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "data", sizeof(param.data), &param.data);
    if (rret < 1)
    {
        printf("parameter not Found: data\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mask", sizeof(param.mask), &param.mask);
    if (rret < 1)
    {
        printf("parameter not Found: mask\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = ext_mdio_mod(gsw_dev, &param);

    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_ext_mdio_mod failed with ret code", ret);
    else
        printf("fapi_ext_mdio_mod:\tphy=0x%x mmd=0x%x reg=0x%x data=0x%x mask=0x%x\n", param.reg, param.mmd, param.reg, param.data, param.mask);
    return 0;
}