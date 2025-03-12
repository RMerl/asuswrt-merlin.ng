/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**
   \file cmds_fapi.c
    Implements CLI commands for lif mdio

*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "os_types.h"
#include "os_linux.h"

#include "cmds.h"
#include "cmds_fapi.h"
#include "host_adapt.h"
#include "fapi_gsw_hostapi.h"
#include "fapi_gsw_hostapi_mdio_relay.h"

#ifdef ETHSWBOX
#define slif_lib "bcm2835"
#else
#define slif_lib ""
#endif

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
static void cmds_fapi_help(void);

OS_boolean_t cmds_fapi(CmdArgs_t *pArgs, int *err)
{
    OS_boolean_t api_executed;
    int32_t ret;

    if (pArgs == NULL)
    {
        *err = OS_ERROR;
        return OS_TRUE;
    }

    ret = OS_SUCCESS;
    api_executed = OS_TRUE;

    /******************************************
     *  lif CLI cmds                          *
     ******************************************/

    if ((strcmp(pArgs->name, "cmds-fapi-help") == 0) || (strcmp(pArgs->name, "cmds-gsw-?") == 0))
    {
        cmds_fapi_help();
    }

    /****************************************
     * gsw_API:                             *
     *   - api-gsw-internal-read            *
     *   - api-gsw-internal-write           *
     *   - api-gsw-get-links                *
     *   - api-gsw-read                     *
     *   - api-gsw-write                    *
     * *************************************/

    else if (strcmp(pArgs->name, "fapi-int-gphy-write") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 4)
        {
            printf("Usage: fapi-int-gphy-write phy=<> mmd=<> reg=<reg> data=<>\n");
            printf("phy: phy id\n");
            printf("mmd: mmd addres\n");
            printf("reg: mdio register\n");
            printf("data: value to write\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_int_gphy_write(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-int-gphy-read") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 3)
        {
            printf("Usage: fapi-int-gphy-read phy=<> mmd=<> reg=<reg>\n");
            printf("phy: phy id\n");
            printf("mmd: mmd addres\n");
            printf("reg: mdio register\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_int_gphy_read(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-ext-mdio-write") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 4)
        {
            printf("Usage: fapi-ext-mdio-write phy=<> mmd=<> reg=<reg> data=<>\n");
            printf("phy: phy address\n");
            printf("mmd: mmd addres\n");
            printf("reg: mdio register\n");
            printf("data: value to write\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_ext_mdio_write(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-ext-mdio-read") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 3)
        {
            printf("Usage: fapi-ext-mdio-read phy=<> mmd=<> reg=<reg>\n");
            printf("phy: phy address\n");
            printf("mmd: mmd addres\n");
            printf("reg: mdio register\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_ext_mdio_read(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-ext-mdio-mod") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 4)
        {
            printf("Usage: fapi-ext-mdio-mod phy=<> mmd=<> reg=<reg> mask=<> data=<>\n");
            printf("phy: phy address\n");
            printf("mmd: mmd addres\n");
            printf("reg: mdio register\n");
            printf("mask: mask\n");
            printf("data: data to write\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_ext_mdio_mod(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-int-gphy-mod") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 3)
        {
            printf("Usage: fapi-int-gphy-mod phy=<> mmd=<> reg=<reg> mask=<> data=<>\n");
            printf("phy: phy id\n");
            printf("mmd: mmd addres\n");
            printf("reg: mdio register\n");
            printf("mask: mask\n");
            printf("data: data to write\n");

            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_int_gphy_mod(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-RegisterGet") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RegisterGet nRegAddr=<reg>\n");
            printf("reg: register\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_RegisterGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-RegisterSet") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RegisterSet nRegAddr=<reg> nData=<data>\n");
            printf("reg: register\n");
            printf("data: data to write\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RegisterSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-RegisterMod") == 0)
    {

        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RegisterMod nRegAddr=<reg> nData=<data> nMask=<mask>\n");
            printf("nRegAddr: register\n");
            printf("nData: data to write\n");
            printf("nMask: mask\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RegisterMod(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-PortLinkCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PortLinkCfgGet nPortId=<port>\n");
            printf("port: port index <1-8>\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PortLinkCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PortLinkCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PortLinkCfgSet nPortId=<port> [bDuplexForce=<> eDuplex=<> bSpeedForce=<> eSpeed=<> bLinkForce=<> eLink=<> eMII_Mode=<> eMII_Type=<> eClkMode=<> bLPI=<>]\n");
            printf("port: port index <1-8>\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PortLinkCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-RMON-Clear") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RMON-Clear nRmonId=<ID> eRmonType=<TYPE>\n");
            printf("ID: RMON Counters Identifier\n");
            printf("TYPE: RMON Counters Type\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RMON_Clear(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-MonitorPortCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MonitorPortCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MonitorPortCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MonitorPortCfgSet nPortId=<port> nSubIfId=<ID>\n");
            printf("port: port index <1-8>\n");
            printf("ID: Monitoring Sub-IF id\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MonitorPortCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PortCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-PortCfgGet nPortId=<port>\n");
            printf("port: port index <1-8>\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PortCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PortCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-PortCfgSet nPortId=<port> eClassMode=<CLASS> nTrafficClass=<TR>\n");
            printf("port: port index <1-8>\n");
            printf("eClassMode: Select the packet header field\n");
            printf("nTrafficClass: Default port priority\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PortCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-CPU-PortGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc != 0)
        {
            printf("Usage: fapi-GSW-CPU-PortGet\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_CPU_PortGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-CPU-PortSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc != 1)
        {
            printf("Usage!: fapi-GSW-CPU-PortSet nPortId=x\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_CPU_PortSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-DSCP-ClassGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_DSCP_ClassGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-DSCP-ClassSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-DSCP-ClassSet nTrafficClass=<TC> nDSCP=<DSCP>\n");
            printf("nTrafficClass: Configures the DSCP to traffic class mapping\n");
            printf("nDSCP: DSCP to drop precedence assignment\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_DSCP_ClassSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PCP-ClassGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PCP_ClassGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PCP-ClassSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-PCP-ClassSet nTrafficClass=<TC> nPCP=<priority>\n");
            printf("nTrafficClass: Configures the PCP to traffic class mapping\n");
            printf("nPCP: priority to drop precedence assignment\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PCP_ClassSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-SVLAN-PCP-ClassGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_SVLAN_PCP_ClassGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-SVLAN-PCP-ClassSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-SVLAN-PCP-ClassSet nTrafficClass=<TC> nPCP=<priority>\n");
            printf("nTrafficClass: Configures the SVLAN PCP to traffic class mapping\n");
            printf("nPCP: priority to drop precedence assignment\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_SVLAN_PCP_ClassSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ShaperCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ShaperCfgGet nRateShaperId=<Id>\n");
            printf("id:  Rate shaper index\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ShaperCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ShaperCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ShaperCfgSet nRateShaperId=<Id> bEnable=<En> nCbs=<CB> nRate=<Rt>\n");
            printf("id:  Rate shaper index\n");
            printf("En:  Enable/Disable the rate shaper\n");
            printf("CB:  Committed Burst Size\n");
            printf("Rt:  Rate [kbit/s]\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ShaperCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ShaperQueueGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ShaperQueueGet nQueueId=<Id>\n");
            printf("id:  Rate shaper index\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ShaperQueueGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ShaperQueueAssign") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ShaperQueueAssign nQueueId=<QId> nRateShaperId=<RId>\n");
            printf("QId:  Queue index\n");
            printf("RId:  Rate shaper index\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ShaperQueueAssign(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ShaperQueueDeassign") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ShaperQueueDeassign nQueueId=<QId> nRateShaperId=<RId>\n");
            printf("QId:  Queue index\n");
            printf("RId:  Rate shaper index\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ShaperQueueDeassign(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-SchedulerCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-SchedulerCfgGet nQueueId=<QId>\n");
            printf("QId:  Queue index\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_SchedulerCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-SchedulerCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-SchedulerCfgSet nQueueId=<QId> eType=<Type> nWeight=<We>\n");
            printf("QId:  Queue index\n");
            printf("Type:  Scheduler Type\n");
            printf("We:  Weight in Token. Parameter used for WFQ configuration\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_SchedulerCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-WredCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_WredCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-WredCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-WredCfgSet eProfile=<Prof> eMode=<Md> eThreshMode=<THR> nRed_Min=<RMin> nRed_Max=<RMax> nYellow_Min=<YMin> nYellow_Max=<YMax> nGreen_Min=<GMin> nGreen_Max=<GMax>\n");
            printf("Prof: Drop Probability Profile\n");
            printf("Md:   Automatic or Manual Mode of Thresholds Config\n");
            printf("THR:  WRED Threshold Mode Config\n");
            printf("RMin: WRED Red Threshold Min\n");
            printf("RMax: WRED Red Threshold Max\n");
            printf("YMin: WRED Yellow Threshold Min\n");
            printf("YMax: WRED Yellow Threshold Max\n");
            printf("GMin: WRED Green Threshold Min\n");
            printf("GMax: WRED Green Threshold Max\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_WredCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-WredQueueCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-WredQueueCfgSet nQueueId=<QId>\n");
            printf("QId:  Queue Index\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_WredQueueCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-WredQueueCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-WredQueueCfgSet nQueueId=<QId> nRed_Min=<RMin> nRed_Max=<RMax> nYellow_Min=<YMin> nYellow_Max=<YMax> nGreen_Min=<GMin> nGreen_Max=<GMax>\n");
            printf("QId:  Queue Index\n");
            printf("RMin: WRED Red Threshold Min\n");
            printf("RMax: WRED Red Threshold Max\n");
            printf("YMin: WRED Yellow Threshold Min\n");
            printf("YMax: WRED Yellow Threshold Max\n");
            printf("GMin: WRED Green Threshold Min\n");
            printf("GMax: WRED Green Threshold Max\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_WredQueueCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-WredPortCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-WredPortCfgGet nPortId=<port>\n");
            printf("port:  Port Index\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_WredPortCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-WredPortCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-WredPortCfgSet nPortId=<port> nRed_Min=<RMin> nRed_Max=<RMax> nYellow_Min=<YMin> nYellow_Max=<YMax> nGreen_Min=<GMin> nGreen_Max=<GMax>\n");
            printf("port:  Port Index\n");
            printf("RMin: WRED Red Threshold Min\n");
            printf("RMax: WRED Red Threshold Max\n");
            printf("YMin: WRED Yellow Threshold Min\n");
            printf("YMax: WRED Yellow Threshold Max\n");
            printf("GMin: WRED Green Threshold Min\n");
            printf("GMax: WRED Green Threshold Max\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_WredPortCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-TrunkingCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_TrunkingCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-TrunkingCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-TrunkingCfgSet bIP_Src=<bIP_Src> bIP_Dst=<bIP_Dst> bMAC_Src=<bMAC_Src> bMAC_Dst=<bMAC_Dst> bSrc_Port=<bSrc_Port> bDst_Port=<bDst_Port>\n");
            printf("bIP_Src:  MAC source address Use\n");
            printf("bIP_Dst:  MAC destination address Use\n");
            printf("bMAC_Src: MAC source address Use\n");
            printf("bMAC_Dst: MAC destination address Use\n");
            printf("bSrc_Port:  TCP/UDP source Port Use\n");
            printf("bDst_Port:  TCP/UDP Destination Port Use\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_TrunkingCfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MAC-TableClear") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_TableClear(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-MAC-TableClear-Cond") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MAC-TableClear-Cond eType=<> nPortId=<>\n");
            printf("eType: MAC table clear type\n");
            printf("nPortId: Physical port id\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);

        ret = fapi_GSW_MAC_TableCondClear(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-CfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-CfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CfgSet bIP_Src=<bIP_Src> bIP_Dst=<bIP_Dst> bMAC_Src=<bMAC_Src> bMAC_Dst=<bMAC_Dst> bSrc_Port=<bSrc_Port> bDst_Port=<bDst_Port>\n");
            printf("eMAC_TableAgeTimer: MAC table aging timer\n");
            printf("nAgeTimer:  MAC table aging timer in seconds\n");
            printf("nMaxPacketLen:  Maximum Ethernet packet length\n");
            printf("bLearningLimitAction: Automatic MAC address table learning limitation {False: Drop/True: Forward}\n");
            printf("bMAC_LockingAction: Accept or discard MAC port locking violation packets {False: Drop/True: Forward}\n");
            printf("bMAC_SpoofingAction:  Accept or discard MAC spoofing and port MAC locking violation packets {False: Drop/True: Forward}\n");
            printf("bDst_bPauseMAC_ModeSrcPort: Pause frame MAC source address mode\n");
            printf("nPauseMAC_Src: Pause frame MAC source address\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MAC-TableEntryRemove") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MAC-TableEntryRemove nFId=<FId> nMAC=<MAC> nTci=<Tci>\n");
            printf("FId: Filtering Identifier (FID)\n");
            printf("MAC:  MAC Address to be removed from the table\n");
            printf("Tci:  TCI for (GSWIP-3.2) B-Step\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_TableEntryRemove(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MAC-TableEntryQuery") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-TableEntryQuery nFId=<FId> nMAC=<MAC> nTci=<Tci> nFilterFlag=<Flag>\n");
            printf("FId: Filtering Identifier (FID)\n");
            printf("MAC:  MAC Address to be removed from the table\n");
            printf("Tci:  TCI for (GSWIP-3.2) B-Step\n");
            printf("Flag: Source/Destination MAC address filtering flag {Value 0 - not filter, 1 - source address filter, 2 - destination address filter, 3 - both source and destination filter}\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_TableEntryQuery(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-FlowctrlCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_FlowctrlCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-FlowctrlCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-FlowctrlCfgSet nFlowCtrlNonConform_Min=<NCMin> nFlowCtrlNonConform_Max=<NCMax> nFlowCtrlConform_Min=<CMin> nFlowCtrlConform_Max=<CMax>\n");
            printf("NCMin: Global Buffer Non Conforming Flow Control Threshold Minimum\n");
            printf("NCMax: Global Buffer Non Conforming Flow Control Threshold Maximum\n");
            printf("CMin:  Global Buffer Conforming Flow Control Threshold Minimum\n");
            printf("CMax:  Global Buffer Conforming Flow Control Threshold Maximum\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_FlowctrlCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-FlowctrlPortCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-FlowctrlPortCfgGet nPortId=<port>\n");
            printf("port: Ethernet Port number\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_FlowctrlPortCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-FlowctrlPortCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-FlowctrlPortCfgSet nPortId=<port> nFlowCtrl_Min=<Min> nFlowCtrl_Max=<Max>\n");
            printf("port: Ethernet Port number\n");
            printf("Min: Ingress Port occupied Buffer Flow Control Threshold Minimum\n");
            printf("Max: Ingress Port occupied Buffer Flow Control Threshold Maximum\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_FlowctrlPortCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-MAC-TableEntryAdd") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MAC-TableEntryAdd\n");
            printf("nFId=<> : Filtering Identifier (FID)\n");
            printf("nPortId=<> : Ethernet Port number (zero-based counting)\n");
            printf("nAgeTimer=<> : Aging Time, given in multiples of 1 second in a range\n");
            printf("bStaticEntry=<> : Static Entry (value will be aged out if the entry is not set to static)\n");
            printf("nTrafficClass=<> : Egress queue traffic class\n");
            printf("bIgmpControlled=<> : Packet is marked as IGMP controlled if destination MAC address matches MAC in this entry\n");
            printf("nFilterFlag=<> : Source/Destination MAC address filtering flag\n");
            printf("nSVLAN_Id=<> : STAG VLAN Id. Only applicable in case SVLAN support is enabled on the device\n");
            printf("nSubIfId=<> : In GSWIP-3.1, this field is sub interface ID for WLAN logical port\n");
            printf("nMAC=<> : MAC Address to add to the table\n");
            printf("nAssociatedMAC=<> : Associated Mac address\n");
            printf("nTci=<> : TCI for (GSWIP-3.2) B-Step\n");
            printf("nPortMapValueIndex0=<> : Bridge Port Map 0\n");
            printf("nPortMapValueIndex1=<> : Bridge Port Map 1\n");
            printf("nPortMapValueIndex2=<> : Bridge Port Map 2\n");
            printf("nPortMapValueIndex3=<> : Bridge Port Map 3\n");
            printf("nPortMapValueIndex4=<> : Bridge Port Map 4\n");
            printf("nPortMapValueIndex5=<> : Bridge Port Map 5\n");
            printf("nPortMapValueIndex6=<> : Bridge Port Map 6\n");
            printf("nPortMapValueIndex7=<> : Bridge Port Map 7\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_TableEntryAdd(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MAC-TableEntryRead") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_TableEntryRead(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-QueuePortGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-QueuePortGet nPortId=<> nTrafficClassId=<> bRedirectionBypass=<> bExtrationEnable=<>\n");
            printf("nPortId=<> : Ethernet Port number (zero-based counting)\n");
            printf("nTrafficClassId=<> : Traffic Class index\n");
            printf("bRedirectionBypass=<> : Queue Redirection bypass Option\n");
            printf("bExtrationEnable=<> : Forward CPU (extraction) before external QoS queueing \n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_QueuePortGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-QueuePortSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-QueuePortSet nPortId=<> nTrafficClassId=<> bRedirectionBypass=<> bExtrationEnable=<> eQMapMode=<> nQueueId=<> nRedirectPortId=<> bEnableIngressPceBypass=<> bReservedPortMode=<>\n");
            printf("nPortId=<> : Ethernet Port number (zero-based counting)\n");
            printf("nTrafficClassId=<> : Traffic Class index\n");
            printf("bRedirectionBypass=<> : Queue Redirection bypass Option\n");
            printf("bExtrationEnable=<> : Forward CPU (extraction) before external QoS queueing \n");
            printf("eQMapMode=<> : Ethernet Port number (zero-based counting)\n");
            printf("nQueueId=<> : Traffic Class index\n");
            printf("nRedirectPortId=<> : Queue Redirection bypass Option\n");
            printf("bEnableIngressPceBypass=<> : Forward CPU (extraction) before external QoS queueing \n");
            printf("bReservedPortMode=<> : Ethernet Port number (zero-based counting)\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_QueuePortSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-QueueCfgSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 3)
        {
            printf("Usage: fapi-GSW-QoS-QueueCfgSett nQueueId=(0~255) bEnable=0/1 nPortId=(0~15)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_QueueCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-QueueCfgGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage!: fapi-GSW-QoS-QueueCfgGet nQueueId=n(0~255)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_QueueCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-BridgePortConfigGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-BridgePortConfigGet nBridgePortId=<> eMask=<>\n");
            printf("nBridgePortId=<> : Bridge ID\n");
            printf("eMask=<> : Mask for updating/retrieving fields\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgePortConfigGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-BridgePortConfigSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-BridgePortConfigGet\n");
            printf("nBridgePortId=<> : Bridge ID\n");
            printf("nBridgeId=<> : Bridge ID\n");
            printf("bIngressExtendedVlanEnable=<>\n");
            printf("nIngressExtendedVlanBlockId=<>\n");
            printf("bEgressExtendedVlanEnable=<>\n");
            printf("nEgressExtendedVlanBlockId=<>\n");
            printf("eIngressMarkingMode=<>\n");
            printf("eEgressRemarkingMode=<>\n");
            printf("bIngressMeteringEnable=<>\n");
            printf("nIngressTrafficMeterId=<>\n");
            printf("bEgressMeteringEnable=<>\n");
            printf("nEgressTrafficMeterId=<>\n");
            printf("bEgressBroadcastSubMeteringEnable=<>\n");
            printf("bEgressMulticastSubMeteringEnable=<>\n");
            printf("bEgressUnknownMulticastIPSubMeteringEnable=<>\n");
            printf("bEgressUnknownMulticastNonIPSubMeteringEnable=<>\n");
            printf("bEgressUnknownUnicastSubMeteringEnable=<>\n");
            printf("nEgressBroadcastSubMeteringId=<>\n");
            printf("nEgressMulticastSubMeteringId=<>\n");
            printf("nEgressUnknownMulticastIPSubMeteringId=<>\n");
            printf("nEgressUnknownMulticastNonIPSubMeteringId=<>\n");
            printf("nEgressUnknownUnicastSubMeteringId=<>\n");
            printf("nDestLogicalPortId=<>\n");
            printf("nDestSubIfIdGroup=<>\n");
            printf("bPmapperEnable=<>\n");
            printf("ePmapperMappingMode=<>\n");
            printf("nPmapperDestSubIfIdGroup=<>\n");
            printf("bBridgePortMapEnable=<>\n");
            printf("Index=<>\n");
            printf("MapValue=<>\n");
            printf("bMcDestIpLookupDisable=<>\n");
            printf("bMcSrcIpLookupEnable=<>\n");
            printf("bDestMacLookupDisable=<>\n");
            printf("bSrcMacLearningDisable=<>\n");
            printf("bMacSpoofingDetectEnable=<>\n");
            printf("bPortLockEnable=<>\n");
            printf("bMacLearningLimitEnable=<>\n");
            printf("nMacLearningLimit=<>\n");
            printf("bIngressVlanFilterEnable=<>\n");
            printf("nIngressVlanFilterBlockId=<>\n");
            printf("bBypassEgressVlanFilter1=<>\n");
            printf("bEgressVlanFilter1Enable=<>\n");
            printf("nEgressVlanFilter1BlockId=<>\n");
            printf("bEgressVlanFilter2Enable=<>\n");
            printf("nEgressVlanFilter2BlockId=<>\n");
            printf("bVlanTagSelection=<>\n");
            printf("bVlanSrcMacPriorityEnable=<>\n");
            printf("bVlanSrcMacDEIEnable=<>\n");
            printf("bVlanSrcMacVidEnable=<>\n");
            printf("bVlanDstMacPriorityEnable=<>\n");
            printf("bVlanDstMacDEIEnable=<>\n");
            printf("bVlanDstMacVidEnable=<>\n");
            printf("bVlanMulticastPriorityEnable=<>\n");
            printf("bVlanMulticastDEIEnable=<>\n");
            printf("bVlanMulticastVidEnable=<>\n");
            printf("nLoopViolationCount=<>\n");
            printf("bForce=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgePortConfigSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-CtpPortConfigGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CtpPortConfigGet nLogicalPortId=<> nSubIfIdGroup=<> eMask=<>\n");
            printf("nLogicalPortId=<> : Bridge ID\n");
            printf("nSubIfIdGroup=<> : Sub interface ID group\n");
            printf("eMask=<> : Mask for updating/retrieving fields\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CtpPortConfigGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-CtpPortConfigSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CtpPortConfigSet\n");
            printf("nLogicalPortId=<>\n");
            printf("nSubIfIdGroup=<>\n");
            printf("nBridgePortId=<>\n");
            printf("bForcedTrafficClass=<>\n");
            printf("nDefaultTrafficClass=<>\n");
            printf("bIngressExtendedVlanEnable=<>\n");
            printf("nIngressExtendedVlanBlockId=<>\n");
            printf("bIngressExtendedVlanIgmpEnable=<>\n");
            printf("nIngressExtendedVlanBlockIdIgmp=<>\n");
            printf("bEgressExtendedVlanEnable=<>\n");
            printf("nEgressExtendedVlanBlockId=<>\n");
            printf("bEgressExtendedVlanIgmpEnable=<>\n");
            printf("nEgressExtendedVlanBlockIdIgmp=<>\n");
            printf("bIngressNto1VlanEnable=<>\n");
            printf("bEgressNto1VlanEnable=<>\n");
            printf("eIngressMarkingMode=<>\n");
            printf("eEgressMarkingMode=<>\n");
            printf("bEgressMarkingOverrideEnable=<>\n");
            printf("eEgressMarkingModeOverride=<>\n");
            printf("eEgressRemarkingMode=<>\n");
            printf("bIngressMeteringEnable=<>\n");
            printf("nIngressTrafficMeterId=<>\n");
            printf("bEgressMeteringEnable=<>\n");
            printf("nEgressTrafficMeterId=<>\n");
            printf("bBridgingBypass=<>\n");
            printf("nDestLogicalPortId=<>\n");
            printf("nDestSubIfIdGroup=<>\n");
            printf("bPmapperEnable=<>\n");
            printf("ePmapperMappingMode=<>\n");
            printf("nFirstFlowEntryIndex=<>\n");
            printf("nNumberOfFlowEntries=<>\n");
            printf("bIngressLoopbackEnable=<>\n");
            printf("bIngressDaSaSwapEnable=<>\n");
            printf("bEgressLoopbackEnable=<>\n");
            printf("bEgressDaSaSwapEnable=<>\n");
            printf("bIngressMirrorEnable=<>\n");
            printf("bEgressMirrorEnable=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CtpPortConfigSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-BridgeAlloc") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgeAlloc(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-BridgeFree") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-BridgeFree nBridgeId=<>\n");
            printf("nBridgeId: Bridge ID (FID) to which this bridge port is associated\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgeFree(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-BridgeConfigGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-BridgeConfigGet nBridgeId=<> eMask=<>\n");
            printf("nBridgeId: Bridge ID (FID) to which this bridge port is associated\n");
            printf("eMask: Mask\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgeConfigGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-BridgeConfigSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-BridgeConfigSet\n");
            printf("nBridgeId=<>\n");
            printf("bMacLearningLimitEnable=<>\n");
            printf("nMacLearningLimit=<>\n");
            printf("eForwardBroadcast=<>\n");
            printf("eForwardUnknownMulticastIp=<>\n");
            printf("eForwardUnknownMulticastNonIp=<>\n");
            printf("eForwardUnknownUnicast=<>\n");
            printf("bBroadcastMeterEnable=<>\n");
            printf("nBroadcastMeterId=<>\n");
            printf("bMulticastMeterEnable=<>\n");
            printf("nMulticastMeterId=<>\n");
            printf("bUnknownMulticastIpMeterEnable=<>\n");
            printf("nUnknownMulticastIpMeterId=<>\n");
            printf("bUnknownMulticastNonIpMeterEnable=<>\n");
            printf("nUnknownMulticastNonIpMeterId=<>\n");
            printf("bUnknownUniCastMeterEnable=<>\n");
            printf("nUnknownUniCastMeterId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgeConfigSet(pArgs->prmc, pArgs->prmvs);
    }

    // ###################

    else if (strcmp(pArgs->name, "fapi-GSW-ExtendedVlanAlloc") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-ExtendedVlanAlloc nNumberOfEntries=<>\n");
            printf("nNumberOfEntries: Total number of extended VLAN entries are requested\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_ExtendedVlanAlloc(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-ExtendedVlanFree") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-ExtendedVlanFree nExtendedVlanBlockId=<>\n");
            printf("nExtendedVlanBlockId: Block Id\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_ExtendedVlanFree(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-ExtendedVlanGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-ExtendedVlanGet\n");
            printf("nExtendedVlanBlockId=<>\n");
            printf("nEntryIndex=<>\n");
            printf("bOriginalPacketFilterMode=<>\n");
            printf("eFilter_4_Tpid_Mode=<>\n");
            printf("eTreatment_4_Tpid_Mode=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_ExtendedVlanGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-ExtendedVlanSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-ExtendedVlanSet\n");
            printf("nExtendedVlanBlockId=<>\n");
            printf("nEntryIndex=<>\n");
            printf("eOuterVlanFilterVlanType=<>\n");
            printf("bOuterVlanFilterPriorityEnable=<>\n");
            printf("nOuterVlanFilterPriorityVal=<>\n");
            printf("bOuterVlanFilterVidEnable=<>\n");
            printf("nOuterVlanFilterVidVal=<>\n");
            printf("eOuterVlanFilterTpid=<>\n");
            printf("eOuterVlanFilterDei=<>\n");
            printf("eInnerVlanFilterVlanType=<>\n");
            printf("bInnerVlanFilterPriorityEnable=<>\n");
            printf("nInnerVlanFilterPriorityVal=<>\n");
            printf("bInnerVlanFilterVidEnable=<>\n");
            printf("nInnerVlanFilterVidVal=<>\n");
            printf("eInnerVlanFilterTpid=<>\n");
            printf("eInnerVlanFilterDei=<>\n");
            printf("eEtherType=<>\n");
            printf("eRemoveTagAction=<>\n");
            printf("bOuterVlanActionEnable=<>\n");
            printf("eOuterVlanActionPriorityMode=<>\n");
            printf("eOuterVlanActionPriorityVal=<>\n");
            printf("eOuterVlanActionVidMode=<>\n");
            printf("eOuterVlanActionVidVal=<>\n");
            printf("eOuterVlanActionTpid=<>\n");
            printf("eOuterVlanActioneDei=<>\n");
            printf("bInnerVlanActionEnable=<>\n");
            printf("eInnerVlanActionPriorityMode=<>\n");
            printf("eInnerVlanActionPriorityVal=<>\n");
            printf("eInnerVlanActionVidMode=<>\n");
            printf("eInnerVlanActionVidVal=<>\n");
            printf("eInnerVlanActionTpid=<>\n");
            printf("eInnerVlanActioneDei=<>\n");
            printf("bReassignBridgePortEnable=<>\n");
            printf("nNewBridgePortId=<>\n");
            printf("bNewDscpEnable=<>\n");
            printf("nNewDscp=<>\n");
            printf("bNewTrafficClassEnable=<>\n");
            printf("nNewTrafficClass=<>\n");
            printf("bNewMeterEnable=<>\n");
            printf("sNewTrafficMeterId=<>\n");
            printf("bLoopbackEnable=<>\n");
            printf("bDaSaSwapEnable=<>\n");
            printf("bMirrorEnable=<>\n");
            printf("bDscp2PcpMapEnable=<>\n");
            printf("nDscp2PcpMapValue=<>\n");
            printf("bOriginalPacketFilterMode=<>\n");
            printf("eFilter_4_Tpid_Mode=<>\n");
            printf("eTreatment_4_Tpid_Mode=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_ExtendedVlanSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-VlanFilterAlloc") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        if (pArgs->prmc < 3)
        {
            printf("Usage: fapi-GSW-VlanFilterAlloc nNumberOfEntries=<> bDiscardUntagged=<> bDiscardUnmatchedTagged=<>\n");
            printf("nNumberOfEntries: Total number of extended VLAN entries are requested\n");
            printf("bDiscardUntagged: Discard packet without VLAN tag\n");
            printf("bDiscardUnmatchedTagged: Discard packet not matching\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_VlanFilterAlloc(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-VlanFilterFree") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-VlanFilterFree nVlanFilterBlockId=<>\n");
            printf("nVlanFilterBlockId: Block Id\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_VlanFilterFree(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-VlanFilterGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-VlanFilterGet nVlanFilterBlockId=<> nEntryIndex=<>\n");
            printf("nVlanFilterBlockId: Block Id\n");
            printf("nEntryIndex: Entry Index\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_VlanFilterGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-VlanFilterSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-VlanFilterSet nVlanFilterBlockId=<> nEntryIndex=<> eVlanFilterMask=<> nVal=<> bDiscardMatched=<>\n");
            printf("nVlanFilterBlockId=<>\n");
            printf("nEntryIndex=<>\n");
            printf("eVlanFilterMask=<>\n");
            printf("nVal=<>\n");
            printf("bDiscardMatched=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_VlanFilterSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-STP-PortCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-STP-PortCfgGet nPortId=<>\n");
            printf("nPortId: Port Id\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_STP_PortCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-STP-PortCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-STP-PortCfgSet nPortId=<> ePortState=<>\n");
            printf("nPortId=<>\n");
            printf("ePortState=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_STP_PortCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-STP-BPDU-RuleGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_STP_BPDU_RuleGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-STP-BPDU-RuleSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-STP-BPDU-RuleSet eForwardPort=<> nForwardPortId=<>\n");
            printf("eForwardPort=<>\n");
            printf("nForwardPortId=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_STP_BPDU_RuleSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Debug-RMON-Port-Get") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-Debug-RMON-Port-Get nPortId=<> ePortType=<>\n");
            printf("nPortId=<>\n");
            printf("ePortType=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Debug_RMON_Port_Get(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-MeterCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-MeterCfgGet nMeterId=<> \n");
            printf("nMeterId: Meter index (zero-based counting)\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_MeterCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-MeterCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-MeterCfgSet\n");
            printf("nMeterId: Meter index (zero-based counting)\n");
            printf("bEnable=<>\n");
            printf("eMtrType=<>\n");
            printf("nCbs=<>\n");
            printf("nEbs=<>\n");
            printf("nRate=<>\n");
            printf("nPiRate=<>\n");
            printf("cMeterName=<>\n");
            printf("nColourBlindMode=<>\n");
            printf("bPktMode=<>\n");
            printf("bLocalOverhd=<>\n");
            printf("nLocaloverhd=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_MeterCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-MAC-DefaultFilterGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MAC-DefaultFilterGet eType=<>\n");
            printf("eType: MAC Address Filter Type\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_DefaultFilterGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MAC-DefaultFilterSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MAC-DefaultFilterSet eType=<>\n");
            printf("eType: MAC Address Filter Type\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MAC_DefaultFilterSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-CTP-PortAssignmentGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CTP-PortAssignmentGet nLogicalPortId=<>\n");
            printf("nLogicalPortId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CTP_PortAssignmentGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-CTP-PortAssignmentSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CTP-PortAssignmentSet\n");
            printf("nLogicalPortId=<>\n");
            printf("nFirstCtpPortId=<>\n");
            printf("nNumberOfCtpPort=<>\n");
            printf("eMode=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CTP_PortAssignmentSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-GLBL-CfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-GLBL-CfgSet\n");
            printf("nPmacId=<>\n");
            printf("bRxFCSDis=<>\n");
            printf("eProcFlagsEgCfg=<>\n");
            printf("nBslThreshold0=<>\n");
            printf("nBslThreshold1=<>\n");
            printf("nBslThreshold2=<>\n");
            printf("bAPadEna=<>\n");
            printf("bPadEna=<>\n");
            printf("bVPadEna=<>\n");
            printf("bSVPadEna=<>\n");
            printf("bTxFCSDis=<>\n");
            printf("bIPTransChkRegDis=<>\n");
            printf("bIPTransChkVerDis=<>\n");
            printf("bJumboEna=<>\n");
            printf("nMaxJumboLen=<>\n");
            printf("nJumboThreshLen=<>\n");
            printf("bLongFrmChkDis=<>\n");
            printf("eShortFrmChkType=<>\n");
            printf("bProcFlagsEgCfgEna=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_GLBL_CfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-GLBL-CfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-GLBL-CfgGet\n");
            printf("nPmacId=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_GLBL_CfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-BM-CfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-PMAC-BM-CfgSet\n");
            printf("nTxDmaChanId=<>\n");
            printf("nPmacId=<>\n");
            printf("txQMask=<>\n");
            printf("rxPortMask=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_BM_CfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-BM-CfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-PMAC-BM-CfgGet\n");
            printf("nTxDmaChanId=<>\n");
            printf("nPmacId=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_BM_CfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-EG-CfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-EG-CfgSet\n");
            printf("nPmacId=<>\n");
            printf("bRedirEnable=<>\n");
            printf("bBslSegmentDisable=<>\n");
            printf("nBslTrafficClass=<>\n");
            ;
            printf("bResDW1Enable=<>\n");
            printf("bRes2DW0Enable=<>\n");
            printf("bRes1DW0Enable=<>\n");
            printf("bTCEnable=<>\n");
            printf("nDestPortId=<>\n");
            printf("bProcFlagsSelect=<>\n");
            printf("nTrafficClass=<>\n");
            printf("nFlowIDMsb=<>\n");
            printf("bMpe1Flag=<>\n");
            printf("bMpe2Flag=<>\n");
            printf("bEncFlag=<>\n");
            printf("bDecFlag=<>\n");
            printf("nRxDmaChanId=<>\n");
            printf("bRemL2Hdr=<>\n");
            printf("numBytesRem=<>\n");
            printf("bFcsEna=<>\n");
            printf("bPmacEna=<>\n");
            printf("nResDW1=<>\n");
            printf("nRes1DW0=<>\n");
            printf("nRes2DW0=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_EG_CfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-EG-CfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-EG-CfgGet\n");
            printf("nPmacId=<>\n");
            printf("nDestPortId=<>\n");
            printf("bProcFlagsSelect=<>\n");
            printf("nTrafficClass=<>\n");
            printf("nFlowIDMsb=<>\n");
            printf("bMpe1Flag=<>\n");
            printf("bMpe2Flag=<>\n");
            printf("bEncFlag=<>\n");
            printf("bDecFlag=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_EG_CfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-IG-CfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-IG-CfgGet\n");
            printf("nPmacId=<>\n");
            printf("nTxDmaChanId=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_IG_CfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-IG-CfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-IG-CfgSet\n");
            printf("nPmacId=<>\n");
            printf("nTxDmaChanId=<>\n");
            printf("bErrPktsDisc=<>\n");
            printf("bPmapDefault=<>\n");
            printf("bPmapEna=<>\n");
            printf("bClassDefault=<>\n");
            printf("bClassEna=<>\n");
            printf("eSubId=<>\n");
            printf("bSpIdDefault=<>\n");
            printf("bPmacPresent=<>\n");
            printf("defPmacHdr=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_IG_CfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleRead") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PceRuleRead\n");
            printf("pattern.nIndex=<>\n");
            printf("nLogicalPortId=<>\n");
            printf("nSubIfIdGroup=<>\n");
            printf("region=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleRead(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleWrite") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-PceRuleWrite nLogicalPortId=<> pattern.nIndex=<>\n");
            printf("nLogicalPortId=<>\n");
            printf("nSubIfIdGroup=<>\n");
            printf("region=<>\n");
            printf("pattern.nIndex=<>\n");
            printf("pattern.bEnable=<>\n");
            printf("pattern.bPortIdEnable=<>\n");
            printf("pattern.nPortId=<>\n");
            printf("pattern.bPortId_Exclude=<>\n");
            printf("pattern.bSubIfIdEnable=<>\n");
            printf("pattern.nSubIfId=<>\n");
            printf("pattern.eSubIfIdType=<>\n");
            printf("pattern.bSubIfId_Exclude=<>\n");
            printf("pattern.bInsertionFlag_Enable=<>\n");
            printf("pattern.nInsertionFlag=<>\n");
            printf("pattern.bDSCP_Enable=<>\n");
            printf("pattern.nDSCP=<>\n");
            printf("pattern.bDSCP_Exclude=<>\n");
            printf("pattern.bInner_DSCP_Enable=<>\n");
            printf("pattern.nInnerDSCP=<>\n");
            printf("pattern.bInnerDSCP_Exclude=<>\n");
            printf("pattern.bPCP_Enable=<>\n");
            printf("pattern.nPCP=<>\n");
            printf("pattern.bCTAG_PCP_DEI_Exclude=<>\n");
            printf("pattern.bSTAG_PCP_DEI_Enable=<>\n");
            printf("pattern.nSTAG_PCP_DEI=<>\n");
            printf("pattern.bSTAG_PCP_DEI_Exclude=<>\n");
            printf("pattern.bPktLngEnable=<>\n");
            printf("pattern.nPktLng=<>\n");
            printf("pattern.nPktLngRange=<>\n");
            printf("pattern.bPktLng_Exclude=<>\n");
            printf("pattern.bMAC_DstEnable=<>\n");
            printf("pattern.nMAC_Dst=<>\n");
            printf("pattern.nMAC_DstMask=<>\n");
            printf("pattern.bDstMAC_Exclude=<>\n");
            printf("pattern.bMAC_SrcEnable=<>\n");
            printf("pattern.nMAC_Src=<>\n");
            printf("pattern.nMAC_SrcMask=<>\n");
            printf("pattern.bSrcMAC_Exclude=<>\n");
            printf("pattern.bAppDataMSB_Enable=<>\n");
            printf("pattern.nAppDataMSB=<>\n");
            printf("pattern.bAppMaskRangeMSB_Select=<>\n");
            printf("pattern.nAppMaskRangeMSB=<>\n");
            printf("pattern.bAppMSB_Exclude=<>\n");
            printf("pattern.bAppDataLSB_Enable=<>\n");
            printf("pattern.nAppDataLSB=<>\n");
            printf("pattern.bAppMaskRangeLSB_Select=<>\n");
            printf("pattern.nAppMaskRangeLSB=<>\n");
            printf("pattern.bAppLSB_Exclude=<>\n");
            printf("pattern.eDstIP_Select=<>\n");
            printf("pattern.nDstIP=<>\n");
            printf("pattern.nDstIP=<>\n");
            printf("pattern.nDstIP_Mask=<>\n");
            printf("pattern.bDstIP_Exclude=<>\n");
            printf("pattern.eInnerDstIP_Select=<>\n");
            printf("pattern.nInnerDstIP=<>\n");
            printf("pattern.nInnerDstIP=<>\n");
            printf("pattern.nInnerDstIP_Mask=<>\n");
            printf("pattern.bInnerDstIP_Exclude=<>\n");
            printf("pattern.eSrcIP_Select=<>\n");
            printf("pattern.nSrcIP=<>\n");
            printf("pattern.nSrcIP=<>\n");
            printf("pattern.nSrcIP_Mask=<>\n");
            printf("pattern.bSrcIP_Exclude=<>\n");
            printf("pattern.eInnerSrcIP_Select=<>\n");
            printf("pattern.nInnerSrcIP=<>\n");
            printf("pattern.nInnerSrcIP=<>\n");
            printf("pattern.nInnerSrcIP_Mask=<>\n");
            printf("pattern.bInnerSrcIP_Exclude=<>\n");
            printf("pattern.bEtherTypeEnable=<>\n");
            printf("pattern.nEtherType=<>\n");
            printf("pattern.nEtherTypeMask=<>\n");
            printf("pattern.bEtherType_Exclude=<>\n");
            printf("pattern.bProtocolEnable=<>\n");
            printf("pattern.nProtocol=<>\n");
            printf("pattern.nProtocolMask=<>\n");
            printf("pattern.bProtocol_Exclude=<>\n");
            printf("pattern.bInnerProtocolEnable=<>\n");
            printf("pattern.nInnerProtocol=<>\n");
            printf("pattern.nInnerProtocolMask=<>\n");
            printf("pattern.bInnerProtocol_Exclude=<>\n");
            printf("pattern.bSessionIdEnable=<>\n");
            printf("pattern.nSessionId=<>\n");
            printf("pattern.bSessionId_Exclude=<>\n");
            printf("pattern.bPPP_ProtocolEnable=<>\n");
            printf("pattern.nPPP_Protocol=<>\n");
            printf("pattern.nPPP_ProtocolMask=<>\n");
            printf("pattern.bPPP_Protocol_Exclude=<>\n");
            printf("pattern.bVid=<>\n");
            printf("pattern.nVid=<>\n");
            printf("pattern.bVidRange_Select=<>\n");
            printf("pattern.nVidRange=<>\n");
            printf("pattern.bVid_Exclude=<>\n");
            printf("pattern.bSLAN_Vid=<>\n");
            printf("pattern.nSLAN_Vid=<>\n");
            printf("pattern.bSLANVid_Exclude=<>\n");
            printf("pattern.bPayload1_SrcEnable=<>\n");
            printf("pattern.nPayload1=<>\n");
            printf("pattern.bPayload1MaskRange_Select=<>\n");
            printf("pattern.nPayload1_Mask=<>\n");
            printf("pattern.bPayload1_Exclude=<>\n");
            printf("pattern.bPayload2_SrcEnable=<>\n");
            printf("pattern.nPayload2=<>\n");
            printf("pattern.bPayload2MaskRange_Select=<>\n");
            printf("pattern.nPayload2_Mask=<>\n");
            printf("pattern.bPayload2_Exclude=<>\n");
            printf("pattern.bParserFlagLSB_Enable=<>\n");
            printf("pattern.nParserFlagLSB=<>\n");
            printf("pattern.nParserFlagLSB_Mask=<>\n");
            printf("pattern.bParserFlagLSB_Exclude=<>\n");
            printf("pattern.bParserFlagMSB_Enable=<>\n");
            printf("pattern.nParserFlagMSB=<>\n");
            printf("pattern.nParserFlagMSB_Mask=<>\n");
            printf("pattern.bParserFlagMSB_Exclude=<>\n");
            printf("pattern.bParserFlag1LSB_Enable=<>\n");
            printf("pattern.nParserFlag1LSB=<>\n");
            printf("pattern.nParserFlag1LSB_Mask=<>\n");
            printf("pattern.bParserFlag1LSB_Exclude=<>\n");
            printf("pattern.bParserFlag1MSB_Enable=<>\n");
            printf("pattern.nParserFlag1MSB=<>\n");
            printf("pattern.nParserFlag1MSB_Mask=<>\n");
            printf("pattern.bParserFlag1MSB_Exclude=<>\n");
            printf("pattern.bVid_Original=<>\n");
            printf("pattern.nOuterVidRange=<>\n");
            printf("pattern.bSVidRange_Select=<>\n");
            printf("pattern.bOuterVid_Original=<>\n");
            printf("action.eTrafficClassAction=<>\n");
            printf("action.nTrafficClassAlternate=<>\n");
            printf("action.eSnoopingTypeAction=<>\n");
            printf("action.eLearningAction=<>\n");
            printf("action.eIrqAction=<>\n");
            printf("action.eCrossStateAction=<>\n");
            printf("action.eCritFrameAction=<>\n");
            printf("action.eTimestampAction=<>\n");
            printf("action.ePortMapAction=<>\n");
            printf("action.nForwardPortMap=<>\n");
            printf("action.nForwardPortMap[1~7]=<>\n");
            printf("action.bRemarkAction=<>\n");
            printf("action.bRemarkPCP=<>\n");
            printf("action.bRemarkSTAG_PCP=<>\n");
            printf("action.bRemarkSTAG_DEI=<>\n");
            printf("action.bRemarkDSCP=<>\n");
            printf("action.bRemarkClass=<>\n");
            printf("action.eMeterAction=<>\n");
            printf("action.nMeterId=<>\n");
            printf("action.bRMON_Action=<>\n");
            printf("action.nRMON_Id=<>\n");
            printf("action.eVLAN_Action=<>\n");
            printf("action.nVLAN_Id=<>\n");
            printf("action.nFId=<>\n");
            printf("action.bFidEnable=<>\n");
            printf("action.eSVLAN_Action=<>\n");
            printf("action.nSVLAN_Id=<>\n");
            printf("action.eVLAN_CrossAction=<>\n");
            printf("action.bPortBitMapMuxControl=<>\n");
            printf("action.bCVLAN_Ignore_Control=<>\n");
            printf("action.bPortLinkSelection=<>\n");
            printf("action.bPortTrunkAction=<>\n");
            printf("action.bFlowID_Action=<>\n");
            printf("action.nFlowID=<>\n");
            printf("action.bRoutExtId_Action=<>\n");
            printf("action.nRoutExtId=<>\n");
            printf("action.bRtDstPortMaskCmp_Action=<>\n");
            printf("action.bRtSrcPortMaskCmp_Action=<>\n");
            printf("action.bRtDstIpMaskCmp_Action=<>\n");
            printf("action.bRtSrcIpMaskCmp_Action=<>\n");
            printf("action.bRtInnerIPasKey_Action=<>\n");
            printf("action.bRtAccelEna_Action=<>\n");
            printf("action.bRtCtrlEna_Action=<>\n");
            printf("action.eProcessPath_Action=<>\n");
            printf("action.ePortFilterType_Action=<>\n");
            printf("action.bOamEnable=<>\n");
            printf("action.nRecordId=<>\n");
            printf("action.bExtractEnable=<>\n");
            printf("action.eColorFrameAction=<>\n");
            printf("action.bExtendedVlanEnable=<>\n");
            printf("action.nExtendedVlanBlockId=<>\n");
            printf("pattern.bFlexibleField4Enable=<>\n");
            printf("pattern.bFlexibleField4_ExcludeEnable=<>\n");
            printf("pattern.bFlexibleField4_RangeEnable=<>\n");
            printf("pattern.nFlexibleField4_ParserIndex=<>\n");
            printf("pattern.nFlexibleField4_Value=<>\n");
            printf("pattern.nFlexibleField4_MaskOrRange=<>\n");
            printf("pattern.bFlexibleField3Enable=<>\n");
            printf("pattern.bFlexibleField3_ExcludeEnable=<>\n");
            printf("pattern.bFlexibleField3_RangeEnable=<>\n");
            printf("pattern.nFlexibleField3_ParserIndex=<>\n");
            printf("pattern.nFlexibleField3_Value=<>\n");
            printf("pattern.nFlexibleField3_MaskOrRange=<>\n");
            printf("pattern.bFlexibleField2Enable=<>\n");
            printf("pattern.bFlexibleField2_ExcludeEnable=<>\n");
            printf("pattern.bFlexibleField2_RangeEnable=<>\n");
            printf("pattern.nFlexibleField2_ParserIndex=<>\n");
            printf("pattern.nFlexibleField2_Value=<>\n");
            printf("pattern.nFlexibleField2_MaskOrRange=<>\n");
            printf("pattern.bFlexibleField1Enable=<>\n");
            printf("pattern.bFlexibleField1_ExcludeEnable=<>\n");
            printf("pattern.bFlexibleField1_RangeEnable=<>\n");
            printf("pattern.nFlexibleField1_ParserIndex=<>\n");
            printf("pattern.nFlexibleField1_Value=<>\n");
            printf("pattern.nFlexibleField1_MaskOrRange=<>\n");
            printf("action.bPBB_Action_Enable=<>\n");
            printf("action.sPBB_Action.bIheaderActionEnable=<>\n");
            printf("action.sPBB_Action.eIheaderOpMode=<>\n");
            printf("action.sPBB_Action.bTunnelIdKnownTrafficEnable=<>\n");
            printf("action.sPBB_Action.nTunnelIdKnownTraffic=<>\n");
            printf("action.sPBB_Action.bTunnelIdUnKnownTrafficEnable=<>\n");
            printf("action.sPBB_Action.nTunnelIdUnKnownTraffic=<>\n");
            printf("action.sPBB_Action.bB_DstMac_FromMacTableEnable=<>\n");
            printf("action.sPBB_Action.bReplace_B_SrcMacEnable=<>\n");
            printf("action.sPBB_Action.bReplace_B_DstMacEnable=<>\n");
            printf("action.sPBB_Action.bReplace_I_TAG_ResEnable=<>\n");
            printf("action.sPBB_Action.bReplace_I_TAG_UacEnable=<>\n");
            printf("action.sPBB_Action.bReplace_I_TAG_DeiEnable=<>\n");
            printf("action.sPBB_Action.bReplace_I_TAG_PcpEnable=<>\n");
            printf("action.sPBB_Action.bReplace_I_TAG_SidEnable=<>\n");
            printf("action.sPBB_Action.bReplace_I_TAG_TpidEnable=<>\n");
            printf("action.sPBB_Action.bBtagActionEnable=<>\n");
            printf("action.sPBB_Action.eBtagOpMode=<>\n");
            printf("action.sPBB_Action.bProcessIdKnownTrafficEnable=<>\n");
            printf("action.sPBB_Action.nProcessIdKnownTraffic=<>\n");
            printf("action.sPBB_Action.bProcessIdUnKnownTrafficEnable=<>\n");
            printf("action.sPBB_Action.nProcessIdUnKnownTraffic=<>\n");
            printf("action.sPBB_Action.bReplace_B_TAG_DeiEnable=<>\n");
            printf("action.sPBB_Action.bReplace_B_TAG_PcpEnable=<>\n");
            printf("action.sPBB_Action.bReplace_B_TAG_VidEnable=<>\n");
            printf("action.sPBB_Action.bReplace_B_TAG_TpidEnable=<>\n");
            printf("action.sPBB_Action.bMacTableMacinMacActionEnable=<>\n");
            printf("action.sPBB_Action.eMacTableMacinMacSelect=<>\n");
            printf("action.bDestSubIf_Action_Enable=<>\n");
            printf("action.sDestSubIF_Action.bDestSubIFIDActionEnable=<>\n");
            printf("action.sDestSubIF_Action.bDestSubIFIDAssignmentEnable=<>\n");
            printf("action.sDestSubIF_Action.nDestSubIFGrp_Field=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleWrite(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleDelete") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-PceRuleDelete\n");
            printf("nLogicalPortId=<>\n");
            printf("nSubIfIdGroup=<>\n");
            printf("region=<>\n");
            printf("pattern.nIndex=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleDelete(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleAlloc") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PceRuleAlloc\n");
            printf("num_of_rules=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleAlloc(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleFree") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PceRuleFree\n");
            printf("blockid=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleFree(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleEnable") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PceRuleEnable\n");
            printf("nLogicalPortId=<>\n");
            printf("nSubIfIdGroup=<>\n");
            printf("region=<>\n");
            printf("pattern.nIndex=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleEnable(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleDisable") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PceRuleDisable\n");
            printf("nLogicalPortId=<>\n");
            printf("nSubIfIdGroup=<>\n");
            printf("region=<>\n");
            printf("pattern.nIndex=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PceRuleDisable(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-MulticastRouterPortAdd") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MulticastRouterPortAdd\n");
            printf("nPortId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastRouterPortAdd(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MulticastRouterPortRemove") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MulticastRouterPortRemove\n");
            printf("nPortId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastRouterPortRemove(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MulticastSnoopCfgGet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastSnoopCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MulticastSnoopCfgSet") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MulticastSnoopCfgSet\n");
            printf("eIGMP_Mode=<>\n");
            printf("bCrossVLAN=<>\n");
            printf("eForwardPort=<>\n");
            printf("nForwardPortId=<>\n");
            printf("nClassOfService=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastSnoopCfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MulticastRouterPortRead") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastRouterPortRead(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-MulticastTableEntryAdd") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MulticastTableEntryRemove\n");
            printf("nPortId=<> : Bridge Port ID\n");
            printf("nSubIfId=<> : Sub-Interface Id\n");
            printf("eIPVersion=<> : Selection to use IPv4 or IPv6.\n");
            printf("uIP_Gda=<> : Group Destination IP address (GDA).\n");
            printf("uIP_Gsa=<> : Group Source IP address.\n");
            printf("nFID=<> : Filtering Identifier (FID)\n");
            printf("bExclSrcIP=<> : Includes or Excludes Source IP.\n");
            printf("eModeMember=<> : Group member filter mode.\n");
            printf("nTci=<> : TCI for (GSWIP-3.2) B-Step\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastTableEntryAdd(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MulticastTableEntryRead") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastTableEntryRead(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-MulticastTableEntryRemove") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-MulticastTableEntryRemove\n");
            printf("nPortId=<> : Bridge Port ID\n");
            printf("nSubIfId=<> : Sub-Interface Id\n");
            printf("eIPVersion=<> : Selection to use IPv4 or IPv6.\n");
            printf("uIP_Gda=<> : Group Destination IP address (GDA).\n");
            printf("uIP_Gsa=<> : Group Source IP address.\n");
            printf("nFID=<> : Filtering Identifier (FID)\n");
            printf("bExclSrcIP=<> : Includes or Excludes Source IP.\n");
            printf("eModeMember=<> : Group member filter mode.\n");
            printf("nTci=<> : TCI for (GSWIP-3.2) B-Step\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_MulticastTableEntryRemove(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-FW-Update") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_FW_Update(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-FW-Version") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_FW_Version(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PVT-Meas") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PVT_Meas(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Delay") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Delay\n");
            printf("nMsec=<> : Delay Time in milliseconds\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Delay(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-GPIO-Configure") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-GPIO-Configure\n");
            printf("nEnableMaskIndex0=<> : GPIO Enable Mask Index 0\n");
            printf("nEnableMaskIndex1=<> : GPIO Enable Mask Index 1\n");
            printf("nEnableMaskIndex2=<> : GPIO Enable Mask Index 2\n");
            printf("nAltSel0Index0=<> : GPIO Alt Select 0 Index 0\n");
            printf("nAltSel0Index1=<> : GPIO Alt Select 0 Index 1\n");
            printf("nAltSel0Index2=<> : GPIO Alt Select 0 Index 2\n");
            printf("nAltSel1Index0=<> : GPIO Alt Select 1 Index 0\n");
            printf("nAltSel1Index1=<> : GPIO Alt Select 1 Index 1\n");
            printf("nAltSel1Index2=<> : GPIO Alt Select 1 Index 2\n");
            printf("nDirIndex0=<> : GPIO Direction Index 0\n");
            printf("nDirIndex1=<> : GPIO Direction Index 1\n");
            printf("nDirIndex2=<> : GPIO Direction Index 2\n");
            printf("nOutValueIndex0=<> : GPIO Out Value Index 0\n");
            printf("nOutValueIndex1=<> : GPIO Out Value Index 1\n");
            printf("nOutValueIndex2=<> : GPIO Out Value Index 2\n");
            printf("nTimeoutValue=<> : GPIO Timeout in milliseconds\n");
            goto goto_end_help;
        }

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_GPIO_Configure(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Reboot") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Reboot(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-SysReg-Rd") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-SysReg-Rd addr=<addr>\n");
            printf("addr=<> : 32-bit register address\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_SysReg_Rd(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-SysReg-Wr") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-SysReg-Wr addr=<addr> val=<val>\n");
            printf("addr=<> : 32-bit register address\n");
            printf("val=<> : register value\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_SysReg_Wr(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-SysReg-Mod") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-SysReg-Mod addr=<addr> val=<val> mask=<mask>\n");
            printf("addr=<> : 32-bit register address\n");
            printf("val=<> : register value\n");
            printf("mask=<> : register value mask\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_SysReg_Mod(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Cml-Clk-Get") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Cml-Clk-Get nClk=<>\n");
            printf("nClk=<> : CML Clock Output (0 or 1)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_Cml_Clk_Get(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Cml-Clk-Set") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Cml-Clk-Set nClk=<>\n");
            printf("nClk=<> : CML Clock Output (0 or 1)\n");
            printf("bEn=<> : Enable or disable CML Clock Output\n");
            printf("nEnable=<> : Enable CML Clock Output\n");
            printf("bSrcSel=<> : Change clock source selection\n");
            printf("nSrcSel=<> : Clock source {Value0 - 50MHz, 1 - 156.25MHz, 2 - 25MHz}\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_Cml_Clk_Set(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Sfp-Get") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-Sfp-Get nPortId=<> nOption=<>\n");
            printf("nPortId=<> : Port id (0 or 1)\n");
            printf("nOption=<> : Option id (0 - SFP mode/speed/link-status, 1 - flow control)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_Sfp_Get(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Sfp-Set") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-Sfp-Set nPortId=<> nOption=<>\n");
            printf("nPortId=<> : Port id (0 or 1)\n");
            printf("nOption=<> : Option id (0 - SFP mode/speed/link-status, 1 - flow control)\n");
            printf("nMode=<> : SFP mode {Value0 - auto, 1 - fix, 2 - disable}\n");
            printf("nSpeed=<> : Select speed when mode is 1\n");
            printf("            {Value0 - 10G Quad USXGMII, 1 - 1000BaseX ANeg, 2 - 10G XFI,\n");
            printf("             3 - 10G Single USXGMII, 4 - 2.5G SGMII, 5 - 2500 Single USXGMI,\n");
            printf("             6 - 2500BaseX NonANeg, 7 - 1000BaseX NonANeg, 8 - 1G SGMI}\n");
            printf("bFlowCtrlEn=<> : flow control {Value0 - disable, 1 - enable}\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_Sfp_Set(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-VlanCounterMapSet") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-VlanCounterMapSet\n");
            printf("nCounterIndex=<>\n");
            printf("nCtpPortId=<>\n");
            printf("bPriorityEnable=<>\n");
            printf("nPriorityVal=<>\n");
            printf("bVidEnable=<>\n");
            printf("nVidVal=<>\n");
            printf("bVlanTagSelectionEnable=<>\n");
            printf("eVlanCounterMappingType=<>\n");
            printf("eVlanCounterMappingFilterType=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_VlanCounterMapSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-VlanCounterMapGet") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-VlanCounterMapGet\n");
            printf("nCounterIndex=<>\n");
            printf("eVlanCounterMappingType=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_VlanCounterMapGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Vlan-RMON-Get") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Vlan-RMON-Get\n");
            printf("nVlanCounterIndex=<>\n");
            printf("eVlanRmonType=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Vlan_RMON_Get(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Vlan-RMON-Clear") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Vlan-RMON-Clear\n");
            printf("nVlanCounterIndex=<>\n");
            printf("eVlanRmonType=<>\n");
            printf("eClearAll=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Vlan_RMON_Clear(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Vlan-RMONControl-Set") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Vlan-RMONControl-Set\n");
            printf("bVlanRmonEnable=<>\n");
            printf("bIncludeBroadCastPktCounting=<>\n");
            printf("nVlanLastEntry=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Vlan_RMONControl_Set(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Vlan-RMONControl-Get") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Vlan_RMONControl_Get(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-PBB-TunnelTempate-Config-Get") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PBB-TunnelTempate-Config-Get\n");
            printf("nTunnelTemplateId:\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PBB_TunnelTempate_Config_Get(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PBB-TunnelTempate-Config-Set") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PBB-TunnelTempate-Config-Set\n");
            printf("nTunnelTemplateId:\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PBB_TunnelTempate_Config_Set(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PBB-TunnelTempate-Alloc") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PBB_TunnelTempate_Alloc(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-PBB-TunnelTempate-Free") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PBB-TunnelTempate-Free\n");
            printf("nTunnelTemplateId:\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PBB_TunnelTempate_Free(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-CPU-PortCfgGet") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CPU-PortCfgGet\n");
            printf("nPortId:\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CPU_PortCfgGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-CPU-PortCfgSet") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-CPU-PortCfgSet\n");
            printf("nPortId:\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_CPU_PortCfgSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-RMON-PortGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RMON-PortGet\n");
            printf("nPortId=<> : Ethernet Port number.\n");
            printf("ePortType=<> : Port Type.\n");
            printf("nSubIfIdGroup=<> : Sub interface ID group.\n");
            printf("bPceBypass=<> : Separate set of CTP Tx counters when PCE is bypassed.\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RMON_PortGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-RMON-ModeSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RMON-ModeSet\n");
            printf("eRmonType=<> : RMON Counters Type.\n");
            printf("eCountMode=<> : RMON Counters Mode.\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RMON_ModeSet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-RMON-MeterGet") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RMON-MeterGet\n");
            printf("nMeterId:\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RMON_MeterGet(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-RMON-FlowGet") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RMON-FlowGet\n");
            printf("bIndexd=<>\n");
            printf("nIndex=<>\n");
            printf("nPortId=<>\n");
            printf("nFlowId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RMON_FlowGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-RMON-TFlowClear") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-RMON-TFlowClear\n");
            printf("bIndexd=<>\n");
            printf("nIndex=<>\n");
            printf("nPortId=<>\n");
            printf("nFlowId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_RMON_TFlowClear(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-BridgePortAlloc") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgePortAlloc(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-BridgePortFree") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-BridgePortFree\n");
            printf("nBridgePortId:<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgePortFree(pArgs->prmc, pArgs->prmvs);
    }
        
    else if (strcmp(pArgs->name, "fapi-GSW-UnFreeze") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_UnFreeze(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Freeze") == 0)
    {
        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Freeze(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-MeterFree") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-MeterFree nMeterId=<>\n");
            printf("nMeterId: Meter index (zero-based counting)\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_MeterFree(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-MeterAlloc") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_MeterAlloc(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-PMAC-RMON-Get") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PMAC-RMON-Get\n");
            printf("nPmacId=<>\n");
            printf("nPortId=<>\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_PMAC_RMON_Get(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Debug-PMAC-RMON-Get-All") == 0)
    {

        char *slib = "";
        uint16_t phy = 0;
        GSW_Device_t *gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-Debug-PMAC-RMON-Get-All\n");
            printf("nPmacId=<>\n");
            printf("Start=<>\n");
            printf("End=<>\n");

            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_Debug_PMAC_RMON_Get_All(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-Debug-RMON-Port-GetAll") == 0)
    {
        char* slib ="";
        uint16_t phy = 0;
        GSW_Device_t*   gsw_dev;
        uint16_t myval;

        if (pArgs->prmc < 1)
        {
            printf ("Usage: fapi-GSW-Debug-RMON-Port-GetAll ePortType=<> Start=<> End=<>\n");
            printf ("ePortType: Port Type\n");
            printf ("Start: Start Port\n");
            printf ("End: End Port\n");
            goto goto_end_help;
        }
        slib = slif_lib;
        api_gsw_get_links(slib);
        ret = fapi_GSW_DEBUG_RMON_Port_Get_All(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-SS-Sptag-Get") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-SS-Sptag-Get pid=<>\n");
            printf("pid=<> : Port ID\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_SS_Sptag_Get(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-SS-Sptag-Set") == 0)
    {
        char *slib = "";
        uint16_t reg = 0;
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-SS-Sptag-Set pid=<> mask=<>\n");
            printf("pid=<> : Port ID\n");
            printf("mask=<> : bit 0=rx, bit 1=tx, bit 2=rx_pen, bit 3=tx_pen\n");
            printf("rx=<> : RX special tag mode\n");
            printf("tx=<> : TX special tag mode\n");
            printf("rx_pen=<> : RX special tag info over preamble\n");
            printf("tx_pen=<> : TX special tag info over preamble\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_SS_Sptag_Set(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-QoS-DSCP-DropPrecedenceCfgGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc > 0)
        {
            printf("Usage: fapi-GSW-QoS-DSCP-DropPrecedenceCfgGet\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_DSCP_DropPrecedenceCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-DSCP-DropPrecedenceCfgSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;


        if (pArgs->prmc < 2)
        {
            printf("Usage: fapi-GSW-QoS-DSCP-DropPrecedenceCfgSet nIndex=n nVal=x (0~63) \n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_DSCP_DropPrecedenceCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ColorMarkingTableGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ColorMarkingTableGet eMode=n (0~7)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ColorMarkingTableGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ColorMarkingTableSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ColorMarkingTableSet eMode=n (3~7) nIndex=m (0~15/63) nPriority=x (0~7) nColor=y (0~3)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ColorMarkingTableSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ColorReMarkingTableGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-ColorReMarkingTableGet eMode=n (3~7)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ColorReMarkingTableGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-ColorReMarkingTableSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 3)
        {
		printf("Cmd Format: GSW_QOS_COLOR_REMARKING_TBL_SET eMode=n (3~6) nIndex=m (0~15) dei=0/1 pcp=x (0~7)\n");
		printf("\tor: GSW_QOS_COLOR_REMARKING_TBL_SET eMode=7 nIndex=m (0~15) dscp=x\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_ColorReMarkingTableSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-DSCP2-PCPTableGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-DSCP2-PCPTableGet nIndex=n (0~7)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_DSCP2_PCPTableGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-DSCP2-PCPTableSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 3)
        {
            printf("Usage: fapi-GSW-QoS-DSCP2-PCPTableSet nIndex=n (0~7) nDscpIndex=m (0~63) nVal=x)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_DSCP2_PCPTableSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PortReMarkingCfgGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-PortReMarkingCfgGet nPortId=n (0~15)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PortReMarkingCfgGet(pArgs->prmc, pArgs->prmvs);
    }    

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PortReMarkingCfgSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 4)
        {
            printf("Usage: fapi-GSW-QoS-PortReMarkingCfgSet nPortId=n bDscpIngrEn=0/1 bDscpEgrEn=0/1 bPcpIngrEn=0/1\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PortReMarkingCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-StormCfgGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc > 0)
        {
            printf("Usage!: fapi-GSW-QoS-StormCfgGet\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_StormCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-StormCfgSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-QoS-StormCfgSet bEn=0/1 \n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_StormCfgSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-StormCfgGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc > 0)
        {
            printf("Usage!: fapi-GSW-QoS-StormCfgGet\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_StormCfgGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PmapperTableGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage!: fapi-GSW-QoS-PmapperTableGet nPmapperId=n (0~31)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PmapperTableGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-QoS-PmapperTableSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 3)
        {
            printf("Usage!: fapi-GSW-QoS-PmapperTableset nPmapperId=n (0~31) nEntryIndex=m (0~72) nVal=x\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_QoS_PmapperTableSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleBlockSize") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage: fapi-GSW-PceRuleBlockSize blockid=0\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_Pce_RuleBlockSize(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-BridgePortLoopRead") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage!: fapi-GSW-BridgePortLoopRead nBridgePortId=x\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_BridgePort_LoopRead(pArgs->prmc, pArgs->prmvs);
    }
    else if (strcmp(pArgs->name, "fapi-GSW-TflowCountModeGet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage!: fapi-GSW-TflowCountModeGet eCntType=(0~3)\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_TflowCountModeGet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-TflowCountModeSet") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 5)
        {
            printf("Usage!: fapi-GSW-TflowCountModeSet eCntType=(0~3) eCntMode=(0~3) nBrpLsb=x nCtpLsb=y nPortMsb=z\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_TflowCountModeSet(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-Mac-TableLoopDetect") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 1)
        {
            printf("Usage!: fapi-GSW-Mac-TableLoopDetect bp_map_in[0~3]=x\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_Mac_TableLoopDetect(pArgs->prmc, pArgs->prmvs);
    }

    else if (strcmp(pArgs->name, "fapi-GSW-PceRuleMove") == 0)
    {
        char *slib = "";
        GSW_Device_t *gsw_dev;

        if (pArgs->prmc < 4)
        {
            printf("Usage: fapi-GSW-PceRuleMove\n");
            printf("cur.nLogicalPortId=x1 cur.pattern.nIndex=y1 cur.nSubIfIdGroup=z1 cur.region=s1\n");
            printf("new.nLogicalPortId=x2 new.pattern.nIndex=y2 new.nSubIfIdGroup=z2 new.region=s2\n");
            goto goto_end_help;
        }

        slib = slif_lib;

        api_gsw_get_links(slib);
        ret = fapi_GSW_PCE_RuleMove(pArgs->prmc, pArgs->prmvs);
    }

    /***************
     *  No command  *
     ***************/
    else
    {
        api_executed = OS_FALSE;
    }

goto_end_help:
    *err = (int)ret;
    return api_executed;
}

int cmds_fapi_symlink_set(void)
{
    system("ln -sf ./ethswbox fapi-GSW-RegisterGet");
    system("ln -sf ./ethswbox fapi-GSW-RegisterMod");
    system("ln -sf ./ethswbox fapi-GSW-RegisterSet");
    system("ln -sf ./ethswbox fapi-GSW-PortLinkCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-PortLinkCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-RMON-Clear");
    system("ln -sf ./ethswbox fapi-GSW-MonitorPortCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-MonitorPortCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PortCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PortCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-DSCP-ClassGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-DSCP-ClassSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PCP-ClassGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PCP-ClassSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-SVLAN-PCP-ClassGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-SVLAN-PCP-ClassSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ShaperCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ShaperCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ShaperQueueGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ShaperQueueAssign");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ShaperQueueDeassign");
    system("ln -sf ./ethswbox fapi-GSW-QoS-SchedulerCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-SchedulerCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-WredCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-WredCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-WredQueueCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-WredQueueCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-WredPortCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-WredPortCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-TrunkingCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-TrunkingCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-MAC-TableClear");
    system("ln -sf ./ethswbox fapi-GSW-MAC-TableClear-Cond");
    system("ln -sf ./ethswbox fapi-GSW-CfgGet");
    system("ln -sf ./ethswbox fapi-GSW-CfgSet");
    system("ln -sf ./ethswbox fapi-GSW-MAC-TableEntryRemove");
    system("ln -sf ./ethswbox fapi-GSW-MAC-TableEntryQuery");
    system("ln -sf ./ethswbox fapi-GSW-QoS-FlowctrlCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-FlowctrlCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-FlowctrlPortCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-FlowctrlPortCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-MAC-TableEntryAdd");
    system("ln -sf ./ethswbox fapi-GSW-MAC-TableEntryRead");
    system("ln -sf ./ethswbox fapi-GSW-QoS-QueuePortSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-QueuePortGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-QueueCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-QueueCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-BridgePortConfigGet");
    system("ln -sf ./ethswbox fapi-GSW-BridgePortConfigSet");
    system("ln -sf ./ethswbox fapi-GSW-CtpPortConfigGet");
    system("ln -sf ./ethswbox fapi-GSW-CtpPortConfigSet");
    system("ln -sf ./ethswbox fapi-int-gphy-read");
    system("ln -sf ./ethswbox fapi-int-gphy-write");
    system("ln -sf ./ethswbox fapi-ext-mdio-read");
    system("ln -sf ./ethswbox fapi-ext-mdio-write");
    system("ln -sf ./ethswbox fapi-int-gphy-mod");
    system("ln -sf ./ethswbox fapi-ext-mdio-mod");
    system("ln -sf ./ethswbox fapi-GSW-BridgeConfigGet");
    system("ln -sf ./ethswbox fapi-GSW-BridgeConfigSet");
    system("ln -sf ./ethswbox fapi-GSW-BridgeFree");
    system("ln -sf ./ethswbox fapi-GSW-BridgeAlloc");
    system("ln -sf ./ethswbox fapi-GSW-ExtendedVlanGet");
    system("ln -sf ./ethswbox fapi-GSW-ExtendedVlanSet");
    system("ln -sf ./ethswbox fapi-GSW-ExtendedVlanFree");
    system("ln -sf ./ethswbox fapi-GSW-ExtendedVlanAlloc");
    system("ln -sf ./ethswbox fapi-GSW-VlanFilterGet");
    system("ln -sf ./ethswbox fapi-GSW-VlanFilterSet");
    system("ln -sf ./ethswbox fapi-GSW-VlanFilterFree");
    system("ln -sf ./ethswbox fapi-GSW-VlanFilterAlloc");
    system("ln -sf ./ethswbox fapi-GSW-STP-PortCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-STP-PortCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-STP-BPDU-RuleSet");
    system("ln -sf ./ethswbox fapi-GSW-STP-BPDU-RuleGet");
    system("ln -sf ./ethswbox fapi-GSW-Debug-RMON-Port-Get");

    system("ln -sf ./ethswbox fapi-GSW-QoS-MeterCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-MeterCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-MAC-DefaultFilterGet");
    system("ln -sf ./ethswbox fapi-GSW-MAC-DefaultFilterSet");
    system("ln -sf ./ethswbox fapi-GSW-CTP-PortAssignmentGet");
    system("ln -sf ./ethswbox fapi-GSW-CTP-PortAssignmentSet");

    system("ln -sf ./ethswbox fapi-GSW-PMAC-GLBL-CfgSet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-GLBL-CfgGet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-BM-CfgSet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-BM-CfgGet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-EG-CfgSet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-EG-CfgGet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-IG-CfgGet");
    system("ln -sf ./ethswbox fapi-GSW-PMAC-IG-CfgSet");

    system("ln -sf ./ethswbox fapi-GSW-PceRuleDelete");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleRead");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleWrite");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleAlloc");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleFree");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleEnable");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleDisable");

    system("ln -sf ./ethswbox fapi-GSW-MulticastRouterPortAdd");
    system("ln -sf ./ethswbox fapi-GSW-MulticastRouterPortRemove");
    system("ln -sf ./ethswbox fapi-GSW-MulticastSnoopCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-MulticastSnoopCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-MulticastRouterPortRead");
    system("ln -sf ./ethswbox fapi-GSW-MulticastTableEntryAdd");
    system("ln -sf ./ethswbox fapi-GSW-MulticastTableEntryRead");
    system("ln -sf ./ethswbox fapi-GSW-MulticastTableEntryRemove");

    system("ln -sf ./ethswbox fapi-GSW-FW-Update");
    system("ln -sf ./ethswbox fapi-GSW-FW-Version");
    system("ln -sf ./ethswbox fapi-GSW-PVT-Meas");
    system("ln -sf ./ethswbox fapi-GSW-Delay");
    system("ln -sf ./ethswbox fapi-GSW-GPIO-Configure");
    system("ln -sf ./ethswbox fapi-GSW-Reboot");
    system("ln -sf ./ethswbox fapi-GSW-SysReg-Rd");
    system("ln -sf ./ethswbox fapi-GSW-SysReg-Wr");
    system("ln -sf ./ethswbox fapi-GSW-SysReg-Mod");
    system("ln -sf ./ethswbox fapi-GSW-Cml-Clk-Get");
    system("ln -sf ./ethswbox fapi-GSW-Cml-Clk-Set");
    system("ln -sf ./ethswbox fapi-GSW-Sfp-Get");
    system("ln -sf ./ethswbox fapi-GSW-Sfp-Set");

    system("ln -sf ./ethswbox fapi-GSW-VlanCounterMapSet");
    system("ln -sf ./ethswbox fapi-GSW-VlanCounterMapGet");
    system("ln -sf ./ethswbox fapi-GSW-Vlan-RMON-Get");
    system("ln -sf ./ethswbox fapi-GSW-Vlan-RMON-Clear");
    system("ln -sf ./ethswbox fapi-GSW-Vlan-RMONControl-Set");
    system("ln -sf ./ethswbox fapi-GSW-Vlan-RMONControl-Get");

    system("ln -sf ./ethswbox fapi-GSW-PBB-TunnelTempate-Config-Set");
    system("ln -sf ./ethswbox fapi-GSW-PBB-TunnelTempate-Config-Get");
    system("ln -sf ./ethswbox fapi-GSW-PBB-TunnelTempate-Alloc");
    system("ln -sf ./ethswbox fapi-GSW-PBB-TunnelTempate-Free");
    system ("ln -sf ./ethswbox fapi-GSW-Debug-RMON-Port-GetAll");
    system("ln -sf ./ethswbox fapi-GSW-CPU-PortCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-CPU-PortCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-CPU-PortSet");
    system("ln -sf ./ethswbox fapi-GSW-CPU-PortGet");

    system("ln -sf ./ethswbox fapi-GSW-RMON-PortGet");
    system("ln -sf ./ethswbox fapi-GSW-RMON-ModeSet");
    system("ln -sf ./ethswbox fapi-GSW-RMON-MeterGet");
    system("ln -sf ./ethswbox fapi-GSW-RMON-FlowGet");
    system("ln -sf ./ethswbox fapi-GSW-RMON-TFlowClear");

    system("ln -sf ./ethswbox fapi-GSW-BridgePortAlloc");
    system("ln -sf ./ethswbox fapi-GSW-BridgePortFree");
    system("ln -sf ./ethswbox fapi-GSW-Freeze");
    system("ln -sf ./ethswbox fapi-GSW-UnFreeze");

    system("ln -sf ./ethswbox fapi-GSW-BridgePortLoopGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-MeterFree");
    system("ln -sf ./ethswbox fapi-GSW-QoS-MeterAlloc");

    system("ln -sf ./ethswbox fapi-GSW-PMAC-RMON-Get");
    system("ln -sf ./ethswbox fapi-GSW-Debug-PMAC-RMON-Get-All");

    system("ln -sf ./ethswbox fapi-GSW-SS-Sptag-Set");
    system("ln -sf ./ethswbox fapi-GSW-SS-Sptag-Get");
    system("ln -sf ./ethswbox fapi-GSW-QoS-DSCP-DropPrecedenceCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-DSCP-DropPrecedenceCfgSet");
    
    system("ln -sf ./ethswbox fapi-GSW-QoS-ColorMarkingTableGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ColorMarkingTableSet");   
    system("ln -sf ./ethswbox fapi-GSW-QoS-ColorReMarkingTableGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-ColorReMarkingTableSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-DSCP2-PCPTableGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-DSCP2-PCPTableSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PortReMarkingCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PortReMarkingCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-StormCfgGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-StormCfgSet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PmapperTableGet");
    system("ln -sf ./ethswbox fapi-GSW-QoS-PmapperTableSet");

    system("ln -sf ./ethswbox fapi-GSW-PceRuleBlockSize");
    system("ln -sf ./ethswbox fapi-GSW-BridgePortLoopRead");

    system("ln -sf ./ethswbox fapi-GSW-TflowCountModeGet");
    system("ln -sf ./ethswbox fapi-GSW-TflowCountModeSet");

    system("ln -sf ./ethswbox fapi-GSW-Mac-TableLoopDetect");
    system("ln -sf ./ethswbox fapi-GSW-PceRuleMove");
        
    return OS_SUCCESS;
}

static void cmds_fapi_help(void)
{
    printf("+-----------------------------------------------------------------------+\n");
    printf("|                           HELP !                                      |\n");
    printf("|                        CMDS - gsw                                     |\n");
    printf("|                                                                       |\n");
    printf("+-----------------------------------------------------------------------+\n");
    printf("| fapi_GSW_RegisterGet                                                  |\n");
    printf("\n");
}
