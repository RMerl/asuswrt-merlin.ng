/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

#include <os_types.h>
#include <os_linux.h>
#include <gsw_device.h>
#include <gsw_api.h>
#include <host_adapt.h>
#include <gsw_cli_common.h>
#include <gsw_ss.h>
#include <sys_misc.h>

#include <sys/socket.h>
#include <arpa/inet.h>

//#include <gsw_priv.h>
#define lif_id 0
#define NUM_TC 16
#define MAX_NUM_OF_DISPLAY_PORTS 2

// #############################################
static int multicastParamRead(int argc, char *argv[], GSW_multicastTable_t *param)
{
    int ipParamCnt1 = 0;
    int ipParamCnt2 = 0;

    memset(param, 0, sizeof(GSW_multicastTable_t));

    scanParamArg(argc, argv, "nPortId", sizeof(param->nPortId), &param->nPortId);
    scanParamArg(argc, argv, "nSubIfId", sizeof(param->nSubIfId), &param->nSubIfId);
    scanParamArg(argc, argv, "eIPVersion", sizeof(param->eIPVersion), &param->eIPVersion);
    scanParamArg(argc, argv, "eModeMember", sizeof(param->eModeMember), &param->eModeMember);
    scanParamArg(argc, argv, "nFID", sizeof(param->nFID), &param->nFID);
    scanParamArg(argc, argv, "bExclSrcIP", sizeof(param->bExclSrcIP), &param->bExclSrcIP);

    if (param->eIPVersion == GSW_IP_SELECT_IPV4)
    {
        ipParamCnt1 = scanIPv4_Arg(argc, argv, "uIP_Gda", &param->uIP_Gda.nIPv4);
        ipParamCnt2 = scanIPv4_Arg(argc, argv, "uIP_Gsa", &param->uIP_Gsa.nIPv4);
    }
    else
    {
        ipParamCnt1 = scanIPv6_Arg(argc, argv, "uIP_Gda", param->uIP_Gda.nIPv6);
        ipParamCnt2 = scanIPv6_Arg(argc, argv, "uIP_Gsa", param->uIP_Gsa.nIPv6);
    }

    scanParamArg(argc, argv, "nTci", sizeof(param->nTci), &param->nTci);

    if ((param->eModeMember != GSW_IGMP_MEMBER_DONT_CARE) &&
        (ipParamCnt1 == 0) && (ipParamCnt2 == 0))
        return (-2);

    if (ipParamCnt1 == 0)
        return (-3);

    return 0;
}

static void dump_multicast_table_entry(GSW_multicastTableRead_t *ptr)
{
    char sipaddr[64], dipaddr[64];

    // #ifdef CONFIG_NETWORKING
    if (ptr->eIPVersion == GSW_IP_SELECT_IPV4)
    {
        uint32_t dip = htonl(ptr->uIP_Gda.nIPv4);

        inet_ntop(AF_INET, &dip, dipaddr, sizeof(dipaddr));
    }
    else
    {
        uint16_t dip[8], i;

        for (i = 0; i < ARRAY_SIZE(dip); i++)
            dip[i] = htons(ptr->uIP_Gda.nIPv6[i]);
        inet_ntop(AF_INET6, dip, dipaddr, sizeof(dipaddr));
    }
    // #endif
    printf(" %39s |", dipaddr);

    if (ptr->eModeMember != GSW_IGMP_MEMBER_DONT_CARE)
    {
        // #ifdef CONFIG_NETWORKING
        if (ptr->eIPVersion == GSW_IP_SELECT_IPV4)
        {
            uint32_t sip = htonl(ptr->uIP_Gsa.nIPv4);

            inet_ntop(AF_INET, &sip, sipaddr, sizeof(sipaddr));
        }
        else
        {
            uint16_t sip[8], i;

            for (i = 0; i < ARRAY_SIZE(sip); i++)
                sip[i] = htons(ptr->uIP_Gsa.nIPv6[i]);
            inet_ntop(AF_INET6, sip, sipaddr, sizeof(sipaddr));
        }
        // #endif

        printf(" %39s |", sipaddr);
        printf(" %11s |", (ptr->eModeMember == GSW_IGMP_MEMBER_INCLUDE) ? "INCLUDE" : "EXCLUDE");
    }
    else
    {
        printf(" %39s |", "");
        printf(" %11s |", "DON'T CARE");
    }

    printf(" %9d |", ptr->hitstatus);

    if (ptr->nTci)
    {
        printf(" Pri %d, CFI/Dei %d, ID %d",
               ((ptr->nTci & 0xE000) >> 13),
               ((ptr->nTci & 0x1000) >> 12),
               (ptr->nTci & 0x0FFF));
    }
    printf("\n");
}

struct _tbl_dump_
{
    char *tbl_type;
    char *tbl_name;
    u32 entries;
    u32 tbl_addr;
};

typedef struct
{
    u16 num_key;
    u16 num_mask;
    u16 num_val;
} gsw_pce_tbl_t;

struct _tbl_dump_ tbl_dump_gsw33[] = {
    {"PCE", "Tflow Table", 512, 0x0F},
    {"PCE", "Parser Microcode Table", 256, 0x00},
    {"PCE", "VLANMAP Table", 1024, 0x02},
    {"PCE", "PPPoE Table", 16, 0x03},
    {"PCE", "Protocol Table", 32, 0x04},
    {"PCE", "Flags Table", 64, 0x18},
    {"PCE", "App Table", 64, 0x05},
    {"PCE", "IP MSB Table", 64, 0x06},
    {"PCE", "IP LSB Table", 64, 0x07},
    {"PCE", "IP Pktlen Table", 16, 0x08},
    {"PCE", "CTAG Pcp Table", 16, 0x09}, //
    {"PCE", "Dscp Table", 64, 0x0A},
    {"PCE", "Mac Br Table", 4096, 0x0B},
    {"PCE", "Mult Sw Table", 1024, 0x0D}, //
    {"PCE", "Mult Hw Table", 64, 0x0E},
    {"PCE", "Qmap Table", 576, 0x11}, //
    {"PCE", "IGCTP Table", 288, 0x12},
    {"PCE", "EGCTP Table", 288, 0x13},
    {"PCE", "IGBRG Table", 128, 0x14},
    {"PCE", "EGBRG Table", 128, 0x15},
    {"PCE", "Mac Da Table", 64, 0x16},
    {"PCE", "Mac Sa Table", 64, 0x17},
    {"PCE", "BRGCFG Table", 64, 0x19},
    {"PCE", "Spcp Table", 16, 0x1A},
    {"PCE", "COLMARK Table", 128, 0x1B},
    {"PCE", "REMARK Table", 80, 0x1C},
    {"PCE", "Payload Table", 64, 0x1D},
    {"PCE", "Extended VLAN Table", 1024, 0x1E},
    {"PCE", "P-Mapping Table", 292, 0x1F},
    {"PCE", "Dscp2Pcp Table", 64, 0xC},
    {"PCE", "PBB Tunnel Table", 256, 0x10},
};

static const gsw_pce_tbl_t gsw_pce_tbl_33[] = {
    {0, 0, 4}, {2, 0, 0}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 0}, {4, 4, 0}, {4, 4, 0}, {1, 1, 0}, {0, 0, 1}, {0, 0, 1}, {5, 0, 10}, {0, 0, 2}, {20, 0, 10}, {2, 0, 5}, {34, 0, 31}, {0, 0, 11}, {0, 0, 1}, {0, 0, 9}, {0, 0, 7}, {0, 0, 27}, {0, 0, 14}, {3, 1, 0}, {3, 1, 0}, {1, 1, 0}, {0, 0, 10}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {1, 1, 0}, {4, 0, 6}, {0, 0, 1}};

// #############################################################################################################

GSW_return_t fapi_GSW_RegisterMod(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_register_mod_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRegAddr", sizeof(param.nRegAddr), &param.nRegAddr);
    if (rret < 1)
    {
        printf("Parameter not Found: nRegAddr\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nData", sizeof(param.nData), &param.nData);
    if (rret < 1)
    {
        printf("Parameter not Found: nData\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nMask", sizeof(param.nMask), &param.nMask);
    if (rret < 1)
    {
        printf("Parameter not Found: nMask\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RegisterMod(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RegisterMod failed with ret code", ret);
    else
    {
        printf("fapi_GSW_RegisterMod done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_RegisterGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_register_t Param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRegAddr", sizeof(Param.nRegAddr), &Param.nRegAddr);
    if (rret < 1)
    {
        printf("Parameter not Found: nRegAddr\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RegisterGet(gsw_dev, &Param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RegisterGet failed with ret code", ret);
    else
        printf("fapi_GSW_RegisterGet:\n\t reg=0x%x val=0x%x\n", Param.nRegAddr, Param.nData);

    return ret;
}

GSW_return_t fapi_GSW_RegisterSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_register_t Param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRegAddr", sizeof(Param.nRegAddr), &Param.nRegAddr);
    if (rret < 1)
    {
        printf("Parameter not Found: nRegAddr\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nData", sizeof(Param.nData), &Param.nData);
    if (rret < 1)
    {
        printf("Parameter not Found: nData\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RegisterSet(gsw_dev, &Param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RegisterSet failed with ret code", ret);
    else
        printf("fapi_GSW_RegisterSet:\n\t reg=%x val=%x\n", Param.nRegAddr, Param.nData);

    return ret;
}

GSW_return_t fapi_GSW_PortLinkCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_portLinkCfg_t Param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(Param.nPortId), &Param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PortLinkCfgGet(gsw_dev, &Param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PortLinkCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PortLinkCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nPortId", Param.nPortId);
        printf("\t%40s:\t%s\n", "bDuplexForce", Param.bDuplexForce ? "true" : "false");
        printf("\t%40s:\t0x%x\n", "eDuplex", Param.eDuplex);
        printf("\t%40s:\t%s\n", "bSpeedForce", Param.bSpeedForce ? "true" : "false");
        printf("\t%40s:\t0x%x\n", "eSpeed", Param.eSpeed);
        printf("\t%40s:\t%s\n", "bLinkForce", Param.bLinkForce ? "true" : "false");
        printf("\t%40s:\t0x%x\n", "eLink", Param.eLink);
        printf("\t%40s:\t0x%x\n", "eMII_Mode", Param.eMII_Mode);
        printf("\t%40s:\t0x%x\n", "eMII_Type", Param.eMII_Type);
        printf("\t%40s:\t0x%x\n", " eClkMode", Param.eClkMode);
        printf("\t%40s:\t%s\n", "bLPI", Param.bLPI ? "true" : "false");
    }

    return ret;
}

GSW_return_t fapi_GSW_PortLinkCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_portLinkCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PortLinkCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_PortLinkCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "bDuplexForce", sizeof(param.bDuplexForce), &param.bDuplexForce);
    scanParamArg(prmc, prmv, "eDuplex", sizeof(param.eDuplex), &param.eDuplex);
    scanParamArg(prmc, prmv, "bSpeedForce", sizeof(param.bSpeedForce), &param.bSpeedForce);
    scanParamArg(prmc, prmv, "eSpeed", sizeof(param.eSpeed), &param.eSpeed);
    scanParamArg(prmc, prmv, "bLinkForce", sizeof(param.bLinkForce), &param.bLinkForce);
    scanParamArg(prmc, prmv, "eLink", sizeof(param.eLink), &param.eLink);
    scanParamArg(prmc, prmv, "eMII_Mode", sizeof(param.eMII_Mode), &param.eMII_Mode);
    scanParamArg(prmc, prmv, "eMII_Type", sizeof(param.eMII_Type), &param.eMII_Type);
    scanParamArg(prmc, prmv, "eClkMode", sizeof(param.eClkMode), &param.eClkMode);
    scanParamArg(prmc, prmv, "bLPI", sizeof(param.bLPI), &param.bLPI);

    ret = GSW_PortLinkCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PortLinkCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PortLinkCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_RMON_Clear(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_RMON_clear_t param = {0};

    scanParamArg(prmc, prmv, "nRmonId", sizeof(param.nRmonId), &param.nRmonId);
    scanParamArg(prmc, prmv, "eRmonType", sizeof(param.eRmonType), &param.eRmonType);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RMON_Clear(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RMON_Clear failed with ret code", ret);
    else
    {
        printf("fapi_GSW_RMON_Clear done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MonitorPortCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_monitorPortCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_monitorPortCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MonitorPortCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MonitorPortCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MonitorPortCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MonitorPortCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_monitorPortCfg_t param;

    memset(&param, 0, sizeof(GSW_monitorPortCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MonitorPortCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MonitorPortCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MonitorPortCfgGet done\n");
        printf("\t%40s:\t0x%x\n", "nPortId", param.nPortId);
        printf("\t%40s:\t0x%x\n", "nSubIfId", param.nSubIfId);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_PortCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_portCfg_t Param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(Param.nPortId), &Param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_PortCfgGet(gsw_dev, &Param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_PortCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_PortCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nPortId", Param.nPortId);
        printf("\t%40s:\t0x%x\n", "eClassMode", Param.eClassMode);
        printf("\t%40s:\t0x%x\n", "nTrafficClass", Param.nTrafficClass);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_PortCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_portCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_PortCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_PortCfgGet failed with ret code", ret);
        return ret;
    }
    else
    {
        printf("fapi_GSW_QoS_PortCfgSet done\n");
    }

    scanParamArg(prmc, prmv, "eClassMode", sizeof(param.eClassMode), &param.eClassMode);
    scanParamArg(prmc, prmv, "nTrafficClass", sizeof(param.nTrafficClass), &param.nTrafficClass);

    ret = GSW_QoS_PortCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_PortCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_PortCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_DSCP_ClassGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_DSCP_ClassCfg_t param = {0};
    int i;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_DSCP_ClassGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_DSCP_ClassGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_DSCP_ClassGet:\n");
        for (i = 0; i < 64; i++)
            printf("\tnTrafficClass[%d] = %d\n", i, param.nTrafficClass[i]);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_DSCP_ClassSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_DSCP_ClassCfg_t param = {0};
    int rret;
    unsigned char nTrafficClass;
    unsigned int nDSCP;

    rret = scanParamArg(prmc, prmv, "nTrafficClass", sizeof(nTrafficClass), &nTrafficClass);
    if (rret < 1)
    {
        printf("Parameter not Found: nTrafficClass\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nDSCP", sizeof(nDSCP), &nDSCP);
    if (rret < 1)
    {
        printf("Parameter not Found: nDSCP\n");
        return OS_ERROR;
    }

    if (nDSCP >= 64)
    {
        printf("ERROR: Given \"nDSCP\" is out of range (63)\n");
        return (-3);
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_DSCP_ClassGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_DSCP_ClassSet failed with ret code", ret);
        return ret;
    }
    param.nTrafficClass[nDSCP] = nTrafficClass;

    ret = GSW_QoS_DSCP_ClassSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_DSCP_ClassSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_DSCP_ClassSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_PCP_ClassGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_PCP_ClassCfg_t param = {0};
    int i;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_PCP_ClassGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_PCP_ClassGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_PCP_ClassGet:\n");
        for (i = 0; i < 16; i++)
            printf("\tnTrafficClass[%d] = %d\n", i, (int)param.nTrafficClass[i]);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_PCP_ClassSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_PCP_ClassCfg_t param = {0};
    int rret;
    unsigned char nTrafficClass;
    unsigned int nPCP;

    rret = scanParamArg(prmc, prmv, "nTrafficClass", sizeof(nTrafficClass), &nTrafficClass);
    if (rret < 1)
    {
        printf("Parameter not Found: nTrafficClass\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nPCP", sizeof(nPCP), &nPCP);
    if (rret < 1)
    {
        printf("Parameter not Found: nPCP\n");
        return OS_ERROR;
    }

    if (nPCP >= 16)
    {
        printf("ERROR: Given \"nPCP\" is out of range (7)\n");
        return (-3);
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_PCP_ClassGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_PCP_ClassGet failed with ret code", ret);
        return ret;
    }
    param.nTrafficClass[nPCP] = nTrafficClass;

    ret = GSW_QoS_PCP_ClassSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_PCP_ClassSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_PCP_ClassSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_SVLAN_PCP_ClassGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_SVLAN_PCP_ClassCfg_t PCP_ClassCfg = {0};
    int i;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_SVLAN_PCP_ClassGet(gsw_dev, &PCP_ClassCfg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_SVLAN_PCP_ClassGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_SVLAN_PCP_ClassGet:\n");
        for (i = 0; i < 16; i++)
            printf("\tnTrafficClass[%d] = %d\n", i, (int)PCP_ClassCfg.nTrafficClass[i]);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_SVLAN_PCP_ClassSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_SVLAN_PCP_ClassCfg_t PCP_ClassCfg = {0};
    int rret;
    unsigned char nTrafficClass;
    unsigned int nPCP;

    rret = scanParamArg(prmc, prmv, "nTrafficClass", sizeof(nTrafficClass), &nTrafficClass);
    if (rret < 1)
    {
        printf("Parameter not Found: nTrafficClass\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nPCP", sizeof(nPCP), &nPCP);
    if (rret < 1)
    {
        printf("Parameter not Found: nPCP\n");
        return OS_ERROR;
    }

    if (nPCP >= 16)
    {
        printf("ERROR: Given \"nPCP\" is out of range (7)\n");
        return (-3);
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_SVLAN_PCP_ClassGet(gsw_dev, &PCP_ClassCfg);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_SVLAN_PCP_ClassGet failed with ret code", ret);
        return ret;
    }
    PCP_ClassCfg.nTrafficClass[nPCP] = nTrafficClass;

    ret = GSW_QoS_SVLAN_PCP_ClassSet(gsw_dev, &PCP_ClassCfg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_SVLAN_PCP_ClassSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_SVLAN_PCP_ClassSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ShaperCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_ShaperCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRateShaperId", sizeof(param.nRateShaperId), &param.nRateShaperId);
    if (rret < 1)
    {
        printf("Parameter not Found: nRateShaperId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_ShaperCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_ShaperCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_ShaperCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nRateShaperId", param.nRateShaperId);
        printf("\t%40s:\t%s\n", "bEnable", (param.bEnable > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t0x%x\n", "nCbs", param.nCbs);
        printf("\t%40s:\t0x%x\n", "nRate", param.nRate);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ShaperCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_ShaperCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRateShaperId", sizeof(param.nRateShaperId), &param.nRateShaperId);
    if (rret < 1)
    {
        printf("Parameter not Found: nRateShaperId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_ShaperCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_ShaperCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "bEnable", sizeof(param.bEnable), &param.bEnable);
    scanParamArg(prmc, prmv, "nCbs", sizeof(param.nCbs), &param.nCbs);
    scanParamArg(prmc, prmv, "nRate", sizeof(param.nRate), &param.nRate);

    ret = GSW_QoS_ShaperCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_ShaperCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_ShaperCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ShaperQueueAssign(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_ShaperQueue_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRateShaperId", sizeof(param.nRateShaperId), &param.nRateShaperId);
    if (rret < 1)
    {
        printf("Parameter not Found: nRateShaperId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_ShaperQueueAssign(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_ShaperQueueAssign failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_ShaperQueueAssign done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ShaperQueueDeassign(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_ShaperQueue_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nRateShaperId", sizeof(param.nRateShaperId), &param.nRateShaperId);
    if (rret < 1)
    {
        printf("Parameter not Found: nRateShaperId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_ShaperQueueDeassign(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_ShaperQueueDeassign failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_ShaperQueueDeassign done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ShaperQueueGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_ShaperQueueGet_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_ShaperQueueGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_ShaperQueueGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_ShaperQueueGet:\n");
        printf("\t%40s:\t0x%x\n", "nQueueId", param.nQueueId);
        printf("\t%40s\n", "<Shaper 0>");
        printf("\t%40s:\t%s\n", "bAssigned", (param.sShaper[0].bAssigned > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%u (0x%x)\n", "nRateShaperId", param.sShaper[0].nRateShaperId, param.sShaper[0].nRateShaperId);
        printf("\t%40s\n", "<Shaper 1>");
        printf("\t%40s:\t%s\n", "bAssigned", (param.sShaper[1].bAssigned > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%u (0x%x)\n", "nRateShaperId", param.sShaper[1].nRateShaperId, param.sShaper[1].nRateShaperId);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_SchedulerCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_schedulerCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_schedulerCfg_t));

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_SchedulerCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_SchedulerCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_SchedulerCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nQueueId", param.nQueueId);
        printf("\t%40s:\t0x%x\n", "eType", param.eType);
        printf("\t%40s:\t%u (0x%x)\n", "nWeight", param.nWeight, param.nWeight);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_SchedulerCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_schedulerCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_schedulerCfg_t));

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_SchedulerCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_SchedulerCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "eType", sizeof(param.eType), &param.eType);
    scanParamArg(prmc, prmv, "nWeight", sizeof(param.nWeight), &param.nWeight);

    ret = GSW_QoS_SchedulerCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_SchedulerCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_SchedulerCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_WredCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_WRED_Cfg_t param = {0};

    memset(&param, 0, sizeof(GSW_QoS_WRED_Cfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_WredCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_WredCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_WredCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "eProfile", param.eProfile);
        printf("\t%40s:\t0x%x\n", "eMode", param.eMode);
        printf("\t%40s:\t0x%x\n", "eThreshMode", param.eThreshMode);
        printf("\t%40s:\t0x%x\n", "nRed_Min", param.nRed_Min);
        printf("\t%40s:\t0x%x\n", "nRed_Max", param.nRed_Max);
        printf("\t%40s:\t0x%x\n", "nYellow_Min", param.nYellow_Min);
        printf("\t%40s:\t0x%x\n", "nYellow_Max", param.nYellow_Max);
        printf("\t%40s:\t0x%x\n", "nGreen_Min", param.nGreen_Min);
        printf("\t%40s:\t0x%x\n", "nGreen_Max", param.nGreen_Max);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_WredCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_WRED_Cfg_t param = {0};

    memset(&param, 0, sizeof(GSW_QoS_WRED_Cfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_WredCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_WredCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "eProfile", sizeof(param.eProfile), &param.eProfile);
    scanParamArg(prmc, prmv, "eMode", sizeof(param.eMode), &param.eMode);
    scanParamArg(prmc, prmv, "eThreshMode", sizeof(param.eThreshMode), &param.eThreshMode);
    scanParamArg(prmc, prmv, "nRed_Min", sizeof(param.nRed_Min), &param.nRed_Min);
    scanParamArg(prmc, prmv, "nRed_Max", sizeof(param.nRed_Max), &param.nRed_Max);
    scanParamArg(prmc, prmv, "nYellow_Min", sizeof(param.nYellow_Min), &param.nYellow_Min);
    scanParamArg(prmc, prmv, "nYellow_Max", sizeof(param.nYellow_Max), &param.nYellow_Max);
    scanParamArg(prmc, prmv, "nGreen_Min", sizeof(param.nGreen_Min), &param.nGreen_Min);
    scanParamArg(prmc, prmv, "nGreen_Max", sizeof(param.nGreen_Max), &param.nGreen_Max);

    ret = GSW_QoS_WredCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_WredCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_WredCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_WredQueueCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_WRED_QueueCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_WRED_QueueCfg_t));

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_WredQueueCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_WredQueueCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_WredQueueCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nQueueId", param.nQueueId);
        printf("\t%40s:\t0x%x\n", "nRed_Min", param.nRed_Min);
        printf("\t%40s:\t0x%x\n", "nRed_Max", param.nRed_Max);
        printf("\t%40s:\t0x%x\n", "nYellow_Min", param.nYellow_Min);
        printf("\t%40s:\t0x%x\n", "nYellow_Max", param.nYellow_Max);
        printf("\t%40s:\t0x%x\n", "nGreen_Min", param.nGreen_Min);
        printf("\t%40s:\t0x%x\n", "nGreen_Max", param.nGreen_Max);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_WredQueueCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_WRED_QueueCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_WRED_QueueCfg_t));

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_WredQueueCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_WredQueueCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "nRed_Min", sizeof(param.nRed_Min), &param.nRed_Min);
    scanParamArg(prmc, prmv, "nRed_Max", sizeof(param.nRed_Max), &param.nRed_Max);
    scanParamArg(prmc, prmv, "nYellow_Min", sizeof(param.nYellow_Min), &param.nYellow_Min);
    scanParamArg(prmc, prmv, "nYellow_Max", sizeof(param.nYellow_Max), &param.nYellow_Max);
    scanParamArg(prmc, prmv, "nGreen_Min", sizeof(param.nGreen_Min), &param.nGreen_Min);
    scanParamArg(prmc, prmv, "nGreen_Max", sizeof(param.nGreen_Max), &param.nGreen_Max);

    ret = GSW_QoS_WredQueueCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_WredQueueCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_WredQueueCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_WredPortCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_WRED_PortCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_WRED_PortCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_WredPortCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_WredPortCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_WredPortCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nPortId", param.nPortId);
        printf("\t%40s:\t0x%x\n", "nRed_Min", param.nRed_Min);
        printf("\t%40s:\t0x%x\n", "nRed_Max", param.nRed_Max);
        printf("\t%40s:\t0x%x\n", "nYellow_Min", param.nYellow_Min);
        printf("\t%40s:\t0x%x\n", "nYellow_Max", param.nYellow_Max);
        printf("\t%40s:\t0x%x\n", "nGreen_Min", param.nGreen_Min);
        printf("\t%40s:\t0x%x\n", "nGreen_Max", param.nGreen_Max);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_WredPortCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_WRED_PortCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_WRED_PortCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_WredPortCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_WredPortCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "nRed_Min", sizeof(param.nRed_Min), &param.nRed_Min);
    scanParamArg(prmc, prmv, "nRed_Max", sizeof(param.nRed_Max), &param.nRed_Max);
    scanParamArg(prmc, prmv, "nYellow_Min", sizeof(param.nYellow_Min), &param.nYellow_Min);
    scanParamArg(prmc, prmv, "nYellow_Max", sizeof(param.nYellow_Max), &param.nYellow_Max);
    scanParamArg(prmc, prmv, "nGreen_Min", sizeof(param.nGreen_Min), &param.nGreen_Min);
    scanParamArg(prmc, prmv, "nGreen_Max", sizeof(param.nGreen_Max), &param.nGreen_Max);

    ret = GSW_QoS_WredPortCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_WredPortCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_WredPortCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_TrunkingCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_trunkingCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_trunkingCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_TrunkingCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Trunking_CfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Trunking_CfgGet:\n");
        printf("\t%40s:\t0x%x\n", "bIP_Src", param.bIP_Src);
        printf("\t%40s:\t0x%x\n", "bIP_Dst", param.bIP_Dst);
        printf("\t%40s:\t0x%x\n", "bMAC_Src", param.bMAC_Src);
        printf("\t%40s:\t0x%x\n", "bMAC_Dst", param.bMAC_Dst);
        printf("\t%40s:\t0x%x\n", "bSrc_Port", param.bSrc_Port);
        printf("\t%40s:\t0x%x\n", "bDst_Port", param.bDst_Port);
    }

    return ret;
}

GSW_return_t fapi_GSW_TrunkingCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_trunkingCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_trunkingCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_TrunkingCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_TrunkingCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "bIP_Src", sizeof(param.bIP_Src), &param.bIP_Src);
    scanParamArg(prmc, prmv, "bIP_Dst", sizeof(param.bIP_Dst), &param.bIP_Dst);
    scanParamArg(prmc, prmv, "bMAC_Src", sizeof(param.bMAC_Src), &param.bMAC_Src);
    scanParamArg(prmc, prmv, "bMAC_Dst", sizeof(param.bMAC_Dst), &param.bMAC_Dst);
    scanParamArg(prmc, prmv, "bSrc_Port", sizeof(param.bSrc_Port), &param.bSrc_Port);
    scanParamArg(prmc, prmv, "bDst_Port", sizeof(param.bDst_Port), &param.bDst_Port);

    ret = GSW_TrunkingCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Trunking_CfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Trunking_CfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_TableClear(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MAC_TableClear(gsw_dev);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MAC_TableClear failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MAC_TableClear done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_TableCondClear(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    GSW_MAC_tableClearCond_t param = {0};

    scanParamArg(prmc, prmv, "eType", sizeof(param.eType), &param.eType);
    scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MAC_TableClearCond(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MAC_TableCondClear failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MAC_TableCondClear done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_CfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_cfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_cfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_CfgGet:\n");
        printf("\t%40s:\t0x%x\n", "eMAC_TableAgeTimer", param.eMAC_TableAgeTimer);
        printf("\t%40s:\t%u\n", "nAgeTimer", param.nAgeTimer);
        printf("\t%40s:\t%u\n", "nMaxPacketLen", param.nMaxPacketLen);
        printf("\t%40s:\t%d\n", "bLearningLimitAction", param.bLearningLimitAction);
        printf("\t%40s:\t%d\n", "bMAC_LockingAction", param.bMAC_LockingAction);
        printf("\t%40s:\t%d\n", "bMAC_SpoofingAction", param.bMAC_SpoofingAction);
        printf("\t%40s:\t%d\n", "bPauseMAC_ModeSrc", param.bPauseMAC_ModeSrc);
        printf("\t%40s:\t", "nPauseMAC_Src");
        printMAC_Address(param.nPauseMAC_Src);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_CfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_cfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_cfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_CfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "eMAC_TableAgeTimer", sizeof(param.eMAC_TableAgeTimer), &param.eMAC_TableAgeTimer);
    scanParamArg(prmc, prmv, "nAgeTimer", sizeof(param.nAgeTimer), &param.nAgeTimer);
    scanParamArg(prmc, prmv, "nMaxPacketLen", sizeof(param.nMaxPacketLen), &param.nMaxPacketLen);
    scanParamArg(prmc, prmv, "bLearningLimitAction", sizeof(param.bLearningLimitAction), &param.bLearningLimitAction);
    scanParamArg(prmc, prmv, "bMAC_LockingAction", sizeof(param.bLearningLimitAction), &param.bMAC_LockingAction);
    scanParamArg(prmc, prmv, "bMAC_SpoofingAction", sizeof(param.bLearningLimitAction), &param.bMAC_SpoofingAction);
    scanParamArg(prmc, prmv, "bPauseMAC_ModeSrc", sizeof(param.bPauseMAC_ModeSrc), &param.bPauseMAC_ModeSrc);
    scanMAC_Arg(prmc, prmv, "nPauseMAC_Src", param.nPauseMAC_Src);

    ret = GSW_CfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_CfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_TableEntryRemove(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_MAC_tableRemove_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_MAC_tableRemove_t));

    rret = scanMAC_Arg(prmc, prmv, "nMAC", param.nMAC);
    if (rret < 1)
    {
        printf("Parameter not Found: nMAC\n");
        return OS_ERROR;
    }

    printMAC_Address(param.nMAC);
    scanParamArg(prmc, prmv, "nFId", sizeof(param.nFId), &param.nFId);
    scanParamArg(prmc, prmv, "nTci", sizeof(param.nTci), &param.nTci);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MAC_TableEntryRemove(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MAC_TableEntryRemove failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MAC_TableEntryRemove done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_TableEntryQuery(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_MAC_tableQuery_t param = {0};
    int rret, i = 0;

    memset(&param, 0, sizeof(GSW_MAC_tableQuery_t));

    rret = scanMAC_Arg(prmc, prmv, "nMAC", param.nMAC);
    if (rret < 1)
    {
        printf("Parameter not Found: nMAC\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nFId", sizeof(param.nFId), &param.nFId);
    if (rret < 1)
    {
        printf("Parameter not Found: nFId\n");
        return OS_ERROR;
    }
    scanParamArg(prmc, prmv, "nFilterFlag", sizeof(param.nFilterFlag), &param.nFilterFlag);
    scanParamArg(prmc, prmv, "nTci", sizeof(param.nTci), &param.nTci);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MAC_TableEntryQuery(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "GSW_MAC_TableEntryQuery failed with ret code", ret);
    else
    {
        printf("%40s:\t", "nMAC");
        printMAC_Address(param.nMAC);
        printf("\n");
        printf("%40s:\t%d\n", "nFId", param.nFId);
        printf("%40s:\t%d\n", "nFilterFlag", param.nFilterFlag);
        printf("%40s:\t%s\n", "bFound", (param.bFound > 0) ? "1" : "0");
        printf("%40s:\t%s\n", "bStaticEntry", (param.bStaticEntry > 0) ? "TRUE" : "FALSE");
        printf("%40s:\t%d\n", "nSVLAN_Id", param.nSVLAN_Id);

        if (param.bStaticEntry)
        {
            for (i = 0; i < 8; i++)
            {
                printf("%35s[ %d ]:\t0x%x\n", "PortMap", i, param.nPortMap[i]);
            }

            printf("%40s:\t%d\n", "bIgmpControlled", param.bIgmpControlled);
            printf("%40s:\t%d\n", "Hit Status", param.hitstatus);
        }
        else
        { /*Dynamic Entry*/
            printf("%40s:\t%u\n", "nPortId", param.nPortId);
            // printf("%40s:\t%d\n", "nSubIfId", param.nSubIfId); //TODO need use in along with param.nPortId?
            printf("%40s:\t%d\n", "AgeTimer", param.nAgeTimer);
            printf("%40s:\t%s\n", "bEntryChanged", (param.bEntryChanged > 0) ? "TRUE" : "FALSE");
            printf("%40s:\t%d\n", "FirstBridgePortId", param.nFirstBridgePortId);
            printf("%40s:\t", "nAssociatedMAC");
            printMAC_Address(param.nAssociatedMAC);
            printf("\n");
        }

        if (param.nTci)
        {
            printf("\t%40s:\t%d\n", "VLAN PRI", ((param.nTci & 0xE000) >> 13));
            printf("\t%40s:\t%d\n", "VLAN CFI/DEI", ((param.nTci & 0x1000) >> 12));
            printf("\t%40s:\t%d\n", "VLAN ID", (param.nTci & 0x0FFF));
        }
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_FlowctrlCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_FlowCtrlCfg_t param = {0};

    memset(&param, 0, sizeof(GSW_QoS_FlowCtrlCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_FlowctrlCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_FlowctrlCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_FlowctrlCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nFlowCtrlNonConform_Min", param.nFlowCtrlNonConform_Min);
        printf("\t%40s:\t0x%x\n", "nFlowCtrlNonConform_Max", param.nFlowCtrlNonConform_Max);
        printf("\t%40s:\t0x%x\n", "nFlowCtrlConform_Min", param.nFlowCtrlConform_Min);
        printf("\t%40s:\t0x%x\n", "nFlowCtrlConform_Max", param.nFlowCtrlConform_Max);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_FlowctrlCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_FlowCtrlCfg_t param = {0};
    int cnt;

    memset(&param, 0, sizeof(GSW_QoS_FlowCtrlCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_FlowctrlCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_FlowctrlCfgGet failed with ret code", ret);
        return ret;
    }

    cnt = scanParamArg(prmc, prmv, "nFlowCtrlNonConform_Min", sizeof(param.nFlowCtrlNonConform_Min), &param.nFlowCtrlNonConform_Min);
    cnt += scanParamArg(prmc, prmv, "nFlowCtrlNonConform_Max", sizeof(param.nFlowCtrlNonConform_Max), &param.nFlowCtrlNonConform_Max);
    cnt += scanParamArg(prmc, prmv, "nFlowCtrlConform_Min", sizeof(param.nFlowCtrlConform_Min), &param.nFlowCtrlConform_Min);
    cnt += scanParamArg(prmc, prmv, "nFlowCtrlConform_Max", sizeof(param.nFlowCtrlConform_Max), &param.nFlowCtrlConform_Max);

    if (cnt)
    {
        ret = GSW_QoS_FlowctrlCfgSet(gsw_dev, &param);
    }
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_FlowctrlCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_FlowctrlCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_FlowctrlPortCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_FlowCtrlPortCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_FlowCtrlPortCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_FlowctrlPortCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_FlowctrlPortCfgGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_FlowctrlPortCfgGet:\n");
        printf("\t%40s:\t0x%x\n", "nPortId", param.nPortId);
        printf("\t%40s:\t0x%x\n", "nFlowCtrl_Min", param.nFlowCtrl_Min);
        printf("\t%40s:\t0x%x\n", "nFlowCtrl_Max", param.nFlowCtrl_Max);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_FlowctrlPortCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_QoS_FlowCtrlPortCfg_t param = {0};
    int rret, cnt;

    memset(&param, 0, sizeof(GSW_QoS_FlowCtrlPortCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_FlowctrlPortCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_FlowctrlPortCfgGet failed with ret code", ret);
        return ret;
    }

    cnt = scanParamArg(prmc, prmv, "nFlowCtrl_Min", sizeof(param.nFlowCtrl_Min), &param.nFlowCtrl_Min);
    cnt += scanParamArg(prmc, prmv, "nFlowCtrl_Max", sizeof(param.nFlowCtrl_Max), &param.nFlowCtrl_Max);
    if (cnt)
    {
        ret = GSW_QoS_FlowctrlPortCfgSet(gsw_dev, &param);
    }
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_FlowctrlPortCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_FlowctrlPortCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_TableEntryAdd(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_MAC_tableAdd_t param = {0};

    memset(&param, 0, sizeof(GSW_MAC_tableAdd_t));

    scanParamArg(prmc, prmv, "nFId", sizeof(param.nFId), &param.nFId);
    scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    scanParamArg(prmc, prmv, "nAgeTimer", sizeof(param.nAgeTimer), &param.nAgeTimer);
    scanParamArg(prmc, prmv, "bStaticEntry", sizeof(param.bStaticEntry), &param.bStaticEntry);
    scanParamArg(prmc, prmv, "nTrafficClass", sizeof(param.nTrafficClass), &param.nTrafficClass);
    scanParamArg(prmc, prmv, "bIgmpControlled", sizeof(param.bIgmpControlled), &param.bIgmpControlled);
    scanParamArg(prmc, prmv, "nFilterFlag", sizeof(param.nFilterFlag), &param.nFilterFlag);
    scanParamArg(prmc, prmv, "nSVLAN_Id", sizeof(param.bIgmpControlled), &param.nSVLAN_Id);
    scanParamArg(prmc, prmv, "nSubIfId", sizeof(param.nSubIfId), &param.nSubIfId);
    scanMAC_Arg(prmc, prmv, "nMAC", param.nMAC);
    scanMAC_Arg(prmc, prmv, "nAssociatedMAC", param.nAssociatedMAC);
    scanParamArg(prmc, prmv, "nTci", sizeof(param.nTci), &param.nTci);
    scanParamArg(prmc, prmv, "nPortMapValueIndex0", sizeof(param.nPortMap[0]), &param.nPortMap[0]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex1", sizeof(param.nPortMap[1]), &param.nPortMap[1]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex2", sizeof(param.nPortMap[2]), &param.nPortMap[2]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex3", sizeof(param.nPortMap[3]), &param.nPortMap[3]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex4", sizeof(param.nPortMap[4]), &param.nPortMap[4]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex5", sizeof(param.nPortMap[5]), &param.nPortMap[5]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex6", sizeof(param.nPortMap[6]), &param.nPortMap[6]);
    scanParamArg(prmc, prmv, "nPortMapValueIndex7", sizeof(param.nPortMap[7]), &param.nPortMap[7]);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MAC_TableEntryAdd(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MAC_TableEntryAdd failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MAC_TableEntryAdd done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_TableEntryRead(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int i = 0;
    GSW_MAC_tableRead_t MAC_tableRead = {0};

    memset(&MAC_tableRead, 0, sizeof(GSW_MAC_tableRead_t));

    for (i = 0; i < 71; i++)
        printf("-");
    printf("\n");
    printf("%-18s|%-5s|%-10s|%-5s|%-5s|%-5s|%-8s\n",
           "MAC Address", "Port", "Age", "SID", "FID", "Hit", "Stat/Dyn");
    for (i = 0; i < 71; i++)
        printf("-");
    printf("\n");

    MAC_tableRead.bInitial = 1;

    gsw_dev = gsw_get_struc(lif_id, 0);
    for (;;)
    {
        ret = GSW_MAC_TableEntryRead(gsw_dev, &MAC_tableRead);
        if (ret < 0)
        {
            printf("\t%40s:\t0x%x\n", "GSW_MAC_TableEntryRead failed with ret code", ret);
            return ret;
        }

        if (MAC_tableRead.bLast == 1)
            break;

        if (checkValidMAC_Address(MAC_tableRead.nMAC))
        {
            if ((MAC_tableRead.nAgeTimer == 0) && (MAC_tableRead.bStaticEntry == 0))
            {
                /* Do nothing */
                continue;
            }
        }

        if (MAC_tableRead.bStaticEntry)
        {
            unsigned int i = 0;
            printMAC_Address(MAC_tableRead.nMAC);
            if (MAC_tableRead.nPortId & 0x80000000)
                printf(" |MAP  |");
            else
                printf(" |%-5d|", MAC_tableRead.nPortId);
            printf("%-10d|%-5d|%-5d|%-5d|Static\n",
                   MAC_tableRead.nAgeTimer,
                   MAC_tableRead.nSubIfId,
                   MAC_tableRead.nFId,
                   MAC_tableRead.hitstatus);
            printf("%-19s%-5s%-10s%-5s%-5s%-9s%-9s|%-9s: 0x%x\n",
                   "", "", "", "", "", "", "",
                   "nFilterFlag(key)", MAC_tableRead.nFilterFlag);

            for (i = 0; i < ARRAY_SIZE(MAC_tableRead.nPortMap); i++)
            {
                if (!MAC_tableRead.nPortMap[i])
                    continue;

                printf("%-19s%-5s%-10s%-5s%-5s%-9s%-9s|%-8s[%d]: 0x%x\n",
                       "", "", "", "", "", "", "",
                       "PortMap", i, MAC_tableRead.nPortMap[i]);
            }
        }
        else
        {
            printMAC_Address(MAC_tableRead.nMAC);
            printf(" |%-5d|%-10d|%-5d|%-5d|%-5d|Dynamic\n",
                   MAC_tableRead.nPortId,
                   MAC_tableRead.nAgeTimer,
                   MAC_tableRead.nSubIfId,
                   MAC_tableRead.nFId,
                   MAC_tableRead.hitstatus);
        }

        if (MAC_tableRead.nTci)
        {
            printf("%-19s%-5s%-10s%-5s%-55s%-9s%-9s|%-18s: %d\n",
                   "", "", "", "", "", "", "",
                   "VLAN PRI", ((MAC_tableRead.nTci & 0xE000) >> 13));
            printf("%-19s%-5s%-10s%-5s%-55s%-9s%-9s|%-18s: %d\n",
                   "", "", "", "", "", "", "",
                   "VLAN CFI/DEI", ((MAC_tableRead.nTci & 0x1000) >> 12));
            printf("%-19s%-5s%-10s%-5s%-55s%-9s%-9s|%-18s: %d\n",
                   "", "", "", "", "", "", "",
                   "VLAN ID", (MAC_tableRead.nTci & 0x0FFF));
        }
        memset(&MAC_tableRead, 0x00, sizeof(MAC_tableRead));
    }

    for (i = 0; i < 71; i++)
        printf("-");
    printf("\n");

    return ret;
}

GSW_return_t fapi_GSW_QoS_QueuePortSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_queuePort_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_queuePort_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }
    scanParamArg(prmc, prmv, "nTrafficClassId", sizeof(param.nTrafficClassId), &param.nTrafficClassId);
    scanParamArg(prmc, prmv, "bRedirectionBypass", sizeof(param.bRedirectionBypass), &param.bRedirectionBypass);
    scanParamArg(prmc, prmv, "bExtrationEnable", sizeof(param.bExtrationEnable), &param.bExtrationEnable);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_QueuePortGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_QoS_QueuePortGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "eQMapMode", sizeof(param.eQMapMode), &param.eQMapMode);
    scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    scanParamArg(prmc, prmv, "nRedirectPortId", sizeof(param.nRedirectPortId), &param.nRedirectPortId);
    scanParamArg(prmc, prmv, "bEnableIngressPceBypass", sizeof(param.bEnableIngressPceBypass), &param.bEnableIngressPceBypass);
    scanParamArg(prmc, prmv, "bReservedPortMode", sizeof(param.bReservedPortMode), &param.bReservedPortMode);

    ret = GSW_QoS_QueuePortSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_QueuePortSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_QueuePortSet done\n");
    }

    return ret;
}

static int print_queues_gswip32(uint16_t port)
{
    GSW_QoS_queuePort_t queuePortParam = {0};
    int tc;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    queuePortParam.nPortId = port;

    gsw_dev = gsw_get_struc(lif_id, 0);

    for (tc = 0; tc < NUM_TC * 4; tc++)
    {
        queuePortParam.nTrafficClassId = tc & GENMASK(3, 0);
        queuePortParam.bRedirectionBypass = (tc & BIT(4)) >> 4;
        queuePortParam.bExtrationEnable = (tc & BIT(5)) >> 5;

        if (queuePortParam.nTrafficClassId == 0)
        {
            printf("\n");
            printf(" Port | Traffic Class | bRedirectionBypass | bPceIngressBypass | bExtrationEnable | Egress Queue | nRedirectPortId\n");
            printf("------------------------------------------------------------------------------------------------------------------\n");
        }

        ret = GSW_QoS_QueuePortGet(gsw_dev, &queuePortParam);
        if (ret < 0)
        {
            printf("\t%40s:\t0x%x\n", "GSW_QoS_QueuePortGet failed with ret code", ret);
            return ret;
        }

        printf(" %4d | %13d | %18d ",
               queuePortParam.nPortId,
               queuePortParam.nTrafficClassId,
               queuePortParam.bRedirectionBypass);

        if (queuePortParam.bRedirectionBypass)
            printf("| %17s ", "n/a");
        else
            printf("| %17d ", queuePortParam.bEnableIngressPceBypass);

        if (queuePortParam.bRedirectionBypass || !queuePortParam.bEnableIngressPceBypass)
            printf("| %16d ", queuePortParam.bExtrationEnable);
        else
            printf("| %16s ", "n/a");

        printf("| %12d | %15d\n",
               queuePortParam.nQueueId,
               queuePortParam.nRedirectPortId);

        if (queuePortParam.nTrafficClassId == NUM_TC - 1)
            printf("------------------------------------------------------------------------------------------------------------------\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_QueueCfgGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_queueCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_queueCfg_t));
    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_QueueCfgGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_QueueCfgGet failed with ret code: %d\n", ret);
    else
    {
        printf("\tnQueueId: %d\n", param.nQueueId);
        printf("\tbEnable: %d\n", param.bEnable);
        printf("\tnPortId: %d\n", param.nPortId);
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_QueueCfgSet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_queueCfg_t param = {0};
    int rret;
    uint8_t bEnable = 0, nPortId = 0;

    memset(&param, 0, sizeof(GSW_QoS_queueCfg_t));

    rret = scanParamArg(prmc, prmv, "nQueueId", sizeof(param.nQueueId), &param.nQueueId);
    if (rret < 1)
    {
        printf("Parameter not Found: nQueueId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "bEnable", sizeof(param.bEnable), &param.bEnable);
    if (rret < 1)
    {
        printf("Parameter not Found: bEnable\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }
    if (param.nPortId > 15)
    {
        printf("nPortId (%d) is out of range (0~15)\n", param.nPortId);
        return OS_ERROR;
    }
    param.bEnable = param.bEnable % 2;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_QueueCfgSet(gsw_dev, &param);

    if (ret < 0)
    {
        printf("fapi_GSW_QoS_QueueCfgSet failed with ret code: %d\n", ret);
        return ret;
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_QueuePortGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = -1;
    GSW_QoS_queuePort_t queuePortParam = {0};

    memset(&queuePortParam, 0, sizeof(GSW_QoS_queuePort_t));

    if (scanParamArg(prmc, prmv, "nPortId", sizeof(queuePortParam.nPortId), &queuePortParam.nPortId))
    {
        if (scanParamArg(prmc, prmv, "nTrafficClassId", sizeof(queuePortParam.nTrafficClassId), &queuePortParam.nTrafficClassId))
        {
            scanParamArg(prmc, prmv, "bRedirectionBypass", sizeof(queuePortParam.bRedirectionBypass), &queuePortParam.bRedirectionBypass);

            scanParamArg(prmc, prmv, "bExtrationEnable", sizeof(queuePortParam.bExtrationEnable), &queuePortParam.bExtrationEnable);

            gsw_dev = gsw_get_struc(lif_id, 0);
            ret = GSW_QoS_QueuePortGet(gsw_dev, &queuePortParam);
            if (ret < 0)
            {
                printf("\t%40s:\t0x%x\n", "GSW_QoS_QueuePortGet failed with ret code", ret);
                return ret;
            }

            printf("fapi_GSW_QoS_FlowctrlPortCfgGet:\n");
            printf("\t%40s:\t0x%x\n", "nPortId", queuePortParam.nPortId);
            printf("\t%40s:\t0x%x\n", "nTrafficClassId", queuePortParam.nTrafficClassId);
            printf("\t%40s:\t0x%x\n", "bRedirectionBypass", queuePortParam.bRedirectionBypass);
            printf("\t%40s:\t0x%x\n", "bEnableIngressPceBypass", queuePortParam.bEnableIngressPceBypass);
            printf("\t%40s:\t0x%x\n", "bExtrationEnable", queuePortParam.bExtrationEnable);

            if (queuePortParam.eQMapMode == GSW_QOS_QMAP_SINGLE_MODE)
                printf("\t%40s:\tSingle\n", "eQMapMode");
            else
                printf("\t%40s:\tSubifid\n", "eQMapMode");
            printf("\t%40s:\t0x%x\n", "nQueueId", queuePortParam.nQueueId);
            printf("\t%40s:\t0x%x\n", "nRedirectPortId", queuePortParam.nRedirectPortId);
            return ret;
        }
        else
        {
            return print_queues_gswip32(queuePortParam.nPortId);
        }
    }
    printf("Parameter \"nPortId\" is missing.\n");

    return ret;
}

GSW_return_t fapi_GSW_BridgePortConfigGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_BRIDGE_portConfig_t sVar = {0};
    int rret;
    unsigned int i;

    rret = scanParamArg(prmc, prmv, "nBridgePortId", sizeof(sVar.nBridgePortId), &sVar.nBridgePortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgePortId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "eMask", sizeof(sVar.eMask), &sVar.eMask);
    if (!sVar.eMask)
        sVar.eMask = 0xFFFFFFFF;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgePortConfigGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgePortConfigGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t0x%x\n", "nBridgePortId", sVar.nBridgePortId);
        printf("\t%40s:\t0x%x\n", "eMask", sVar.eMask);
        printf("\t%40s:\t0x%x\n", "nBridgeId", sVar.nBridgeId);
        printf("\t%40s:\t0x%x\n", "bIngressExtendedVlanEnable", sVar.bIngressExtendedVlanEnable);
        printf("\t%40s:\t0x%x\n", "nIngressExtendedVlanBlockId", sVar.nIngressExtendedVlanBlockId);
        printf("\t%40s:\t0x%x\n", "bEgressExtendedVlanEnable", sVar.bEgressExtendedVlanEnable);
        printf("\t%40s:\t0x%x\n", "nEgressExtendedVlanBlockId", sVar.nEgressExtendedVlanBlockId);
        printf("\t%40s:\t0x%x\n", "eIngressMarkingMode", sVar.eIngressMarkingMode);
        printf("\t%40s:\t0x%x\n", "eEgressRemarkingMode", sVar.eEgressRemarkingMode);
        printf("\t%40s:\t0x%x\n", "bIngressMeteringEnable", sVar.bIngressMeteringEnable);
        printf("\t%40s:\t0x%x\n", "nIngressTrafficMeterId", sVar.nIngressTrafficMeterId);
        printf("\t%40s:\t0x%x\n", "bEgressMeteringEnable", sVar.bEgressSubMeteringEnable[5]);
        printf("\t%40s:\t0x%x\n", "nEgressTrafficMeterId", sVar.nEgressTrafficSubMeterId[5]);
        printf("\t%40s:\t0x%x\n", "bEgressBroadcastSubMeteringEnable", sVar.bEgressSubMeteringEnable[0]);
        printf("\t%40s:\t0x%x\n", "bEgressMulticastSubMeteringEnable", sVar.bEgressSubMeteringEnable[1]);
        printf("\t%40s:\t0x%x\n", "bEgressUnknownMulticastIPSubMeteringEnable", sVar.bEgressSubMeteringEnable[2]);
        printf("\t%40s:\t0x%x\n", "bEgressUnknownMulticastNonIPSubMeteringEnable", sVar.bEgressSubMeteringEnable[3]);
        printf("\t%40s:\t0x%x\n", "bEgressUnknownUnicastSubMeteringEnable", sVar.bEgressSubMeteringEnable[4]);
        printf("\t%40s:\t0x%x\n", "nEgressBroadcastSubMeteringId", sVar.nEgressTrafficSubMeterId[0]);
        printf("\t%40s:\t0x%x\n", "nEgressMulticastSubMeteringId", sVar.nEgressTrafficSubMeterId[1]);
        printf("\t%40s:\t0x%x\n", "bEgressUnknownMulticastIPSubMeteringEnable", sVar.nEgressTrafficSubMeterId[2]);
        printf("\t%40s:\t0x%x\n", "bEgressUnknownMulticastNonIPSubMeteringEnable", sVar.nEgressTrafficSubMeterId[3]);
        printf("\t%40s:\t0x%x\n", "bEgressUnknownUnicastSubMeteringEnable", sVar.nEgressTrafficSubMeterId[4]);
        printf("\t%40s:\t0x%x\n", "nDestLogicalPortId", sVar.nDestLogicalPortId);
        printf("\t%40s:\t0x%x\n", "nDestSubIfIdGroup", sVar.nDestSubIfIdGroup);
        printf("\t%40s:\t0x%x\n", "bPmapperEnable", sVar.bPmapperEnable);

        if (sVar.bPmapperEnable)
        {
            printf("\t%40s:\t0x%x\n", "ePmapperMappingMode", sVar.ePmapperMappingMode);
            printf("\t%40s:\t0x%x\n", "nPmapperId", sVar.sPmapper.nPmapperId);
            for (i = 0; i < 73; i++)
                printf("\t%40s[%u]:\t0x%x\n", "nDestSubIfIdGroup", i, sVar.sPmapper.nDestSubIfIdGroup[i]);
        }

        for (i = 0; i < ARRAY_SIZE(sVar.nBridgePortMap); i++)
            printf("\t%40s[%u]:\t0x%x\n", "nBridgePortMapIndex", i, sVar.nBridgePortMap[i]);

        printf("\t%40s:\t0x%x\n", "bMcDestIpLookupDisable", sVar.bMcDestIpLookupDisable);
        printf("\t%40s:\t0x%x\n", "bMcSrcIpLookupEnable", sVar.bMcSrcIpLookupEnable);
        printf("\t%40s:\t0x%x\n", "bDestMacLookupDisable", sVar.bDestMacLookupDisable);
        printf("\t%40s:\t0x%x\n", "bSrcMacLearningDisable", sVar.bSrcMacLearningDisable);
        printf("\t%40s:\t0x%x\n", "bMacSpoofingDetectEnable", sVar.bMacSpoofingDetectEnable);
        printf("\t%40s:\t0x%x\n", "bPortLockEnable", sVar.bPortLockEnable);
        printf("\t%40s:\t0x%x\n", "bMacLearningLimitEnable", sVar.bMacLearningLimitEnable);
        printf("\t%40s:\t0x%x\n", "nMacLearningLimit", sVar.nMacLearningLimit);
        printf("\t%40s:\t0x%x\n", "nMacLearningCount", sVar.nMacLearningCount);
        printf("\t%40s:\t0x%x\n", "bIngressVlanFilterEnable", sVar.bIngressVlanFilterEnable);
        printf("\t%40s:\t0x%x\n", "nIngressVlanFilterBlockId", sVar.nIngressVlanFilterBlockId);
        printf("\t%40s:\t0x%x\n", "bBypassEgressVlanFilter1", sVar.bBypassEgressVlanFilter1);
        printf("\t%40s:\t0x%x\n", "bEgressVlanFilter1Enable", sVar.bEgressVlanFilter1Enable);
        printf("\t%40s:\t0x%x\n", "nEgressVlanFilter1BlockId", sVar.nEgressVlanFilter1BlockId);
        printf("\t%40s:\t0x%x\n", "bEgressVlanFilter2Enable", sVar.bEgressVlanFilter2Enable);
        printf("\t%40s:\t0x%x\n", "nEgressVlanFilter2BlockId", sVar.nEgressVlanFilter2BlockId);
        printf("\t%40s:\t0x%x\n", "bVlanTagSelection", sVar.bVlanTagSelection);
        printf("\t%40s:\t0x%x\n", "bVlanSrcMacPriorityEnable", sVar.bVlanSrcMacPriorityEnable);
        printf("\t%40s:\t0x%x\n", "bVlanSrcMacDEIEnable", sVar.bVlanSrcMacDEIEnable);
        printf("\t%40s:\t0x%x\n", "bVlanSrcMacVidEnable", sVar.bVlanSrcMacVidEnable);
        printf("\t%40s:\t0x%x\n", "bVlanDstMacPriorityEnable", sVar.bVlanDstMacPriorityEnable);
        printf("\t%40s:\t0x%x\n", "bVlanDstMacDEIEnable", sVar.bVlanDstMacDEIEnable);
        printf("\t%40s:\t0x%x\n", "bVlanDstMacVidEnable", sVar.bVlanDstMacVidEnable);
        printf("\t%40s:\t0x%x\n", "bVlanMulticastPriorityEnable", sVar.bVlanMulticastPriorityEnable);
        printf("\t%40s:\t0x%x\n", "bVlanMulticastDEIEnable", sVar.bVlanMulticastDEIEnable);
        printf("\t%40s:\t0x%x\n", "bVlanMulticastVidEnable", sVar.bVlanMulticastVidEnable);
        printf("\t%40s:\t0x%x\n", "bLoopViolationCounters", sVar.nLoopViolationCount);
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgePortConfigSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_BRIDGE_portConfig_t sVar;
    int rret;
    unsigned int i, bBridgePortMapEnable = 0, MapValue = 0;
    u16 Index = 0;

    memset(&sVar, 0x00, sizeof(sVar));

    rret = scanParamArg(prmc, prmv, "nBridgePortId", sizeof(sVar.nBridgePortId), &sVar.nBridgePortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgePortId\n");
        return OS_ERROR;
    }

    sVar.eMask = 0xFFFFFFFF;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgePortConfigGet(gsw_dev, &sVar);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_BridgePortConfigGet failed with ret code", ret);
        return ret;
    }
    sVar.eMask = 0x0;

    scanParamArg(prmc, prmv, "nBridgeId", sizeof(sVar.eMask), &sVar.nBridgeId);
    if (findStringParam(prmc, prmv, "nBridgeId"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_ID;

    scanParamArg(prmc, prmv, "bIngressExtendedVlanEnable", sizeof(sVar.bIngressExtendedVlanEnable), &sVar.bIngressExtendedVlanEnable);
    if (findStringParam(prmc, prmv, "bIngressExtendedVlanEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_VLAN;

    scanParamArg(prmc, prmv, "nIngressExtendedVlanBlockId", sizeof(sVar.nIngressExtendedVlanBlockId), &sVar.nIngressExtendedVlanBlockId);
    scanParamArg(prmc, prmv, "bEgressExtendedVlanEnable", sizeof(sVar.bEgressExtendedVlanEnable), &sVar.bEgressExtendedVlanEnable);
    if (findStringParam(prmc, prmv, "bEgressExtendedVlanEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN;

    scanParamArg(prmc, prmv, "nEgressExtendedVlanBlockId", sizeof(sVar.nEgressExtendedVlanBlockId), &sVar.nEgressExtendedVlanBlockId);
    scanParamArg(prmc, prmv, "eIngressMarkingMode", sizeof(sVar.eIngressMarkingMode), &sVar.eIngressMarkingMode);
    if (findStringParam(prmc, prmv, "eIngressMarkingMode"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_MARKING;

    scanParamArg(prmc, prmv, "eEgressRemarkingMode", sizeof(sVar.eEgressRemarkingMode), &sVar.eEgressRemarkingMode);
    if (findStringParam(prmc, prmv, "eEgressRemarkingMode"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_REMARKING;

    scanParamArg(prmc, prmv, "bIngressMeteringEnable", sizeof(sVar.bIngressMeteringEnable), &sVar.bIngressMeteringEnable);
    if (findStringParam(prmc, prmv, "bIngressMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_METER;

    scanParamArg(prmc, prmv, "nIngressTrafficMeterId", sizeof(sVar.nIngressTrafficMeterId), &sVar.nIngressTrafficMeterId);
    scanParamArg(prmc, prmv, "bEgressMeteringEnable", sizeof(sVar.bEgressSubMeteringEnable[5]), &sVar.bEgressSubMeteringEnable[5]);
    if (findStringParam(prmc, prmv, "bEgressMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER;

    scanParamArg(prmc, prmv, "nEgressTrafficMeterId", sizeof(sVar.nEgressTrafficSubMeterId[5]), &sVar.nEgressTrafficSubMeterId[5]);
    scanParamArg(prmc, prmv, "bEgressBroadcastSubMeteringEnable", sizeof(sVar.bEgressSubMeteringEnable[0]), &sVar.bEgressSubMeteringEnable[0]);
    if (findStringParam(prmc, prmv, "bEgressBroadcastSubMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER;

    scanParamArg(prmc, prmv, "bEgressMulticastSubMeteringEnable", sizeof(sVar.bEgressSubMeteringEnable[1]), &sVar.bEgressSubMeteringEnable[1]);
    if (findStringParam(prmc, prmv, "bEgressMulticastSubMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER;

    scanParamArg(prmc, prmv, "bEgressUnknownMulticastIPSubMeteringEnable", sizeof(sVar.bEgressSubMeteringEnable[2]), &sVar.bEgressSubMeteringEnable[2]);
    if (findStringParam(prmc, prmv, "bEgressUnknownMulticastIPSubMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER;

    scanParamArg(prmc, prmv, "bEgressUnknownMulticastNonIPSubMeteringEnable", sizeof(sVar.bEgressSubMeteringEnable[3]), &sVar.bEgressSubMeteringEnable[3]);
    if (findStringParam(prmc, prmv, "bEgressUnknownMulticastNonIPSubMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER;

    scanParamArg(prmc, prmv, "bEgressUnknownUnicastSubMeteringEnable", sizeof(sVar.bEgressSubMeteringEnable[4]), &sVar.bEgressSubMeteringEnable[4]);
    if (findStringParam(prmc, prmv, "bEgressUnknownUnicastSubMeteringEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER;

    scanParamArg(prmc, prmv, "nEgressBroadcastSubMeteringId", sizeof(sVar.nEgressTrafficSubMeterId[0]), &sVar.nEgressTrafficSubMeterId[0]);
    scanParamArg(prmc, prmv, "nEgressMulticastSubMeteringId", sizeof(sVar.nEgressTrafficSubMeterId[1]), &sVar.nEgressTrafficSubMeterId[1]);
    scanParamArg(prmc, prmv, "nEgressUnknownMulticastIPSubMeteringId", sizeof(sVar.nEgressTrafficSubMeterId[2]), &sVar.nEgressTrafficSubMeterId[2]);
    scanParamArg(prmc, prmv, "nEgressUnknownMulticastNonIPSubMeteringId", sizeof(sVar.nEgressTrafficSubMeterId[3]), &sVar.nEgressTrafficSubMeterId[3]);
    scanParamArg(prmc, prmv, "nEgressUnknownUnicastSubMeteringId", sizeof(sVar.nEgressTrafficSubMeterId[4]), &sVar.nEgressTrafficSubMeterId[4]);

    scanParamArg(prmc, prmv, "nDestLogicalPortId", sizeof(sVar.nDestLogicalPortId), &sVar.nDestLogicalPortId);
    if (findStringParam(prmc, prmv, "nDestLogicalPortId"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING;

    scanParamArg(prmc, prmv, "nDestSubIfIdGroup", sizeof(sVar.nDestSubIfIdGroup), &sVar.nDestSubIfIdGroup);
    if (findStringParam(prmc, prmv, "nDestSubIfIdGroup"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING;

    scanParamArg(prmc, prmv, "bPmapperEnable", sizeof(sVar.bPmapperEnable), &sVar.bPmapperEnable);
    if (findStringParam(prmc, prmv, "bPmapperEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING;

    if (sVar.bPmapperEnable)
    {
        scanParamArg(prmc, prmv, "ePmapperMappingMode", sizeof(sVar.ePmapperMappingMode), &sVar.ePmapperMappingMode);
        scanPMAP_Arg(prmc, prmv, "nPmapperDestSubIfIdGroup", sVar.sPmapper.nDestSubIfIdGroup);
        printf("i.  The first entry of each P-mapper index is for Non-IP and Non-VLAN tagging packets\n");
        printf("ii. The entry 8 to 1 of each P-mapper index is for PCP mapping entries\n");
        printf("iii.The entry 72 to 9 of each P-mapper index is for DSCP mapping entries\n");
        printf("User Configured nDestSubIfIdGroup list as below\n");

        for (i = 0; i <= 72; i++)
            printf("sVar.sPmapper.nDestSubIfIdGroup[%d] = %d\n", i, sVar.sPmapper.nDestSubIfIdGroup[i]);
    }

    scanParamArg(prmc, prmv, "bBridgePortMapEnable", sizeof(bBridgePortMapEnable), &bBridgePortMapEnable);
    if (findStringParam(prmc, prmv, "bBridgePortMapEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_PORT_MAP;
    if (bBridgePortMapEnable)
    {
        char buf[] = "nBridgePortMapIndex[0]";

        scanParamArg(prmc, prmv, "Index", sizeof(Index), &Index);
        scanParamArg(prmc, prmv, "MapValue", sizeof(MapValue), &MapValue);

        if (Index >= ARRAY_SIZE(sVar.nBridgePortMap))
        {
            printf("Invalid BridgePortMap Index %d\n", Index);
            return -1;
        }
        sVar.nBridgePortMap[Index] = MapValue;

        for (i = 0; i < ARRAY_SIZE(sVar.nBridgePortMap); i++)
        {
            buf[20] = '0' + i;
            scanParamArg(prmc, prmv, buf, sizeof(sVar.nBridgePortMap[i]), sVar.nBridgePortMap + i);
        }
    }

    scanParamArg(prmc, prmv, "bMcDestIpLookupDisable", sizeof(sVar.bMcDestIpLookupDisable), &sVar.bMcDestIpLookupDisable);
    if (findStringParam(prmc, prmv, "bMcDestIpLookupDisable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_MC_DEST_IP_LOOKUP;

    scanParamArg(prmc, prmv, "bMcSrcIpLookupEnable", sizeof(sVar.bMcSrcIpLookupEnable), &sVar.bMcSrcIpLookupEnable);
    if (findStringParam(prmc, prmv, "bMcSrcIpLookupEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_MC_SRC_IP_LOOKUP;

    scanParamArg(prmc, prmv, "bDestMacLookupDisable", sizeof(sVar.bDestMacLookupDisable), &sVar.bDestMacLookupDisable);
    if (findStringParam(prmc, prmv, "bDestMacLookupDisable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_MC_DEST_MAC_LOOKUP;

    scanParamArg(prmc, prmv, "bSrcMacLearningDisable", sizeof(sVar.bSrcMacLearningDisable), &sVar.bSrcMacLearningDisable);
    if (findStringParam(prmc, prmv, "bSrcMacLearningDisable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_MC_SRC_MAC_LEARNING;

    scanParamArg(prmc, prmv, "bMacSpoofingDetectEnable", sizeof(sVar.bMacSpoofingDetectEnable), &sVar.bMacSpoofingDetectEnable);
    if (findStringParam(prmc, prmv, "bMacSpoofingDetectEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_MAC_SPOOFING;

    scanParamArg(prmc, prmv, "bPortLockEnable", sizeof(sVar.bPortLockEnable), &sVar.bPortLockEnable);
    if (findStringParam(prmc, prmv, "bPortLockEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_PORT_LOCK;

    scanParamArg(prmc, prmv, "bMacLearningLimitEnable", sizeof(sVar.bMacLearningLimitEnable), &sVar.bMacLearningLimitEnable);
    if (findStringParam(prmc, prmv, "bMacLearningLimitEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_MAC_LEARNING_LIMIT;

    scanParamArg(prmc, prmv, "nMacLearningLimit", sizeof(sVar.nMacLearningLimit), &sVar.nMacLearningLimit);

    scanParamArg(prmc, prmv, "bIngressVlanFilterEnable", sizeof(sVar.bIngressVlanFilterEnable), &sVar.bIngressVlanFilterEnable);
    if (findStringParam(prmc, prmv, "bIngressVlanFilterEnable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_VLAN_FILTER;

    scanParamArg(prmc, prmv, "nIngressVlanFilterBlockId", sizeof(sVar.nIngressVlanFilterBlockId), &sVar.nIngressVlanFilterBlockId);

    scanParamArg(prmc, prmv, "bBypassEgressVlanFilter1", sizeof(sVar.bBypassEgressVlanFilter1), &sVar.bBypassEgressVlanFilter1);
    if (findStringParam(prmc, prmv, "bBypassEgressVlanFilter1"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_VLAN_FILTER;

    scanParamArg(prmc, prmv, "bEgressVlanFilter1Enable", sizeof(sVar.bEgressVlanFilter1Enable), &sVar.bEgressVlanFilter1Enable);
    if (findStringParam(prmc, prmv, "bEgressVlanFilter1Enable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN_FILTER1;

    scanParamArg(prmc, prmv, "nEgressVlanFilter1BlockId", sizeof(sVar.nEgressVlanFilter1BlockId), &sVar.nEgressVlanFilter1BlockId);

    scanParamArg(prmc, prmv, "bEgressVlanFilter2Enable", sizeof(sVar.bEgressVlanFilter2Enable), &sVar.bEgressVlanFilter2Enable);
    if (findStringParam(prmc, prmv, "bEgressVlanFilter2Enable"))
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN_FILTER2;

    scanParamArg(prmc, prmv, "nEgressVlanFilter2BlockId", sizeof(sVar.nEgressVlanFilter2BlockId), &sVar.nEgressVlanFilter2BlockId);

    /* VLAN based MAC learning/look-up */
    if (findStringParam(prmc, prmv, "bVlanTagSelection"))
    {
        scanParamArg(prmc, prmv, "bVlanTagSelection", sizeof(sVar.bVlanTagSelection), &sVar.bVlanTagSelection);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    if (findStringParam(prmc, prmv, "bVlanSrcMacPriorityEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanSrcMacPriorityEnable", sizeof(sVar.bVlanSrcMacPriorityEnable), &sVar.bVlanSrcMacPriorityEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    if (findStringParam(prmc, prmv, "bVlanSrcMacDEIEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanSrcMacDEIEnable", sizeof(sVar.bVlanSrcMacDEIEnable), &sVar.bVlanSrcMacDEIEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    if (findStringParam(prmc, prmv, "bVlanSrcMacVidEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanSrcMacVidEnable", sizeof(sVar.bVlanSrcMacVidEnable), &sVar.bVlanSrcMacVidEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    if (findStringParam(prmc, prmv, "bVlanDstMacPriorityEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanDstMacPriorityEnable", sizeof(sVar.bVlanDstMacPriorityEnable), &sVar.bVlanDstMacPriorityEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    if (findStringParam(prmc, prmv, "bVlanDstMacDEIEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanDstMacDEIEnable", sizeof(sVar.bVlanDstMacDEIEnable), &sVar.bVlanDstMacDEIEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    if (findStringParam(prmc, prmv, "bVlanDstMacVidEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanDstMacVidEnable", sizeof(sVar.bVlanDstMacVidEnable), &sVar.bVlanDstMacVidEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING;
    }

    /* VLAN based multicast */
    if (findStringParam(prmc, prmv, "bVlanMulticastPriorityEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanMulticastPriorityEnable", sizeof(sVar.bVlanMulticastPriorityEnable), &sVar.bVlanMulticastPriorityEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MULTICAST_LOOKUP;
    }

    if (findStringParam(prmc, prmv, "bVlanMulticastDEIEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanMulticastDEIEnable", sizeof(sVar.bVlanMulticastDEIEnable), &sVar.bVlanMulticastDEIEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MULTICAST_LOOKUP;
    }

    if (findStringParam(prmc, prmv, "bVlanMulticastVidEnable"))
    {
        scanParamArg(prmc, prmv, "bVlanMulticastVidEnable", sizeof(sVar.bVlanMulticastVidEnable), &sVar.bVlanMulticastVidEnable);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MULTICAST_LOOKUP;
    }

    if (findStringParam(prmc, prmv, "nLoopViolationCount"))
    {
        scanParamArg(prmc, prmv, "nLoopViolationCount", sizeof(sVar.nLoopViolationCount), &sVar.nLoopViolationCount);
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_LOOP_VIOLATION_COUNTER;
    }

    if (findStringParam(prmc, prmv, "bForce"))
    {
        sVar.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_FORCE;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgePortConfigSet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgePortConfigSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_BridgePortConfigSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_CtpPortConfigGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_CTP_portConfig_t sVar = {0};
    int rret;
    unsigned int i;

    memset(&sVar, 0x00, sizeof(sVar));

    rret = scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(sVar.nLogicalPortId), &sVar.nLogicalPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nLogicalPortId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(sVar.nSubIfIdGroup), &sVar.nSubIfIdGroup);
    scanParamArg(prmc, prmv, "eMask", sizeof(sVar.eMask), &sVar.eMask);
    if (!sVar.eMask)
        sVar.eMask = 0xFFFFFFFF;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CtpPortConfigGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CtpPortConfigGet failed with ret code", ret);
    else
    {
        printf("\t nLogicalPortId                     = %u", sVar.nLogicalPortId);
        printf("\n\t nSubIfIdGroup                      = %u", sVar.nSubIfIdGroup);
        printf("\n\t eMask                              = 0x%x", sVar.eMask);
        printf("\n\t nBridgePortId                      = %u", sVar.nBridgePortId);
        printf("\n\t bForcedTrafficClass                = %u", sVar.bForcedTrafficClass);
        printf("\n\t nDefaultTrafficClass               = %u", sVar.nDefaultTrafficClass);
        printf("\n\t bIngressExtendedVlanEnable         = %u", sVar.bIngressExtendedVlanEnable);
        printf("\n\t nIngressExtendedVlanBlockId        = %u", sVar.nIngressExtendedVlanBlockId);
        printf("\n\t bIngressExtendedVlanIgmpEnable     = %u", sVar.bIngressExtendedVlanIgmpEnable);
        printf("\n\t nIngressExtendedVlanBlockIdIgmp    = %u", sVar.nIngressExtendedVlanBlockIdIgmp);
        printf("\n\t bEgressExtendedVlanEnable          = %u", sVar.bEgressExtendedVlanEnable);
        printf("\n\t nEgressExtendedVlanBlockId         = %u", sVar.nEgressExtendedVlanBlockId);
        printf("\n\t bEgressExtendedVlanIgmpEnable      = %u", sVar.bEgressExtendedVlanIgmpEnable);
        printf("\n\t nEgressExtendedVlanBlockIdIgmp     = %u", sVar.nEgressExtendedVlanBlockIdIgmp);
        printf("\n\t bIngressNto1VlanEnable             = %u", sVar.bIngressNto1VlanEnable);
        printf("\n\t bEgressNto1VlanEnable              = %u", sVar.bEgressNto1VlanEnable);
        printf("\n\t eIngressMarkingMode                = %u", sVar.eIngressMarkingMode);
        printf("\n\t eEgressMarkingMode                 = %u", sVar.eEgressMarkingMode);
        printf("\n\t bEgressMarkingOverrideEnable       = %u", sVar.eEgressMarkingModeOverride);
        printf("\n\t eEgressRemarkingMode               = %u", sVar.eEgressRemarkingMode);
        printf("\n\t bIngressMeteringEnable             = %u", sVar.bIngressMeteringEnable);
        printf("\n\t nIngressTrafficMeterId             = %u", sVar.nIngressTrafficMeterId);
        printf("\n\t bEgressMeteringEnable              = %u", sVar.bEgressMeteringEnable);
        printf("\n\t nEgressTrafficMeterId              = %u", sVar.nEgressTrafficMeterId);
        printf("\n\t bBridgingBypass                    = %u", sVar.bBridgingBypass);
        printf("\n\t nDestLogicalPortId                 = %u", sVar.nDestLogicalPortId);
        printf("\n\t nDestSubIfIdGroup                  = %u", sVar.nDestSubIfIdGroup);
        printf("\n\t bPmapperEnable                     = %u", sVar.bPmapperEnable);
        if (sVar.bPmapperEnable)
        {
            printf("\n\t ePmapperMappingMode                = %u", sVar.ePmapperMappingMode);
            printf("\n\t nPmapperId                         = %u", sVar.sPmapper.nPmapperId);

            for (i = 0; i < 73; i++)
                printf("\n\t nDestSubIfIdGroup[%u]         		= %u", i, sVar.sPmapper.nDestSubIfIdGroup[i]);
        }

        printf("\n\t nFirstFlowEntryIndex         	   = %u", sVar.nFirstFlowEntryIndex);
        printf("\n\t nNumberOfFlowEntries         	   = %u", sVar.nNumberOfFlowEntries);
        printf("\n\t bIngressDaSaSwapEnable            = %u", sVar.bIngressDaSaSwapEnable);
        printf("\n\t bEgressDaSaSwapEnable         	   = %u", sVar.bEgressDaSaSwapEnable);
        printf("\n\t bIngressLoopbackEnable            = %u", sVar.bIngressLoopbackEnable);
        printf("\n\t bEgressLoopbackEnable         	   = %u", sVar.bEgressLoopbackEnable);
        printf("\n\t bIngressMirrorEnable         	   = %u", sVar.bIngressMirrorEnable);
        printf("\n\t bEgressMirrorEnable         	   = %u", sVar.bEgressMirrorEnable);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_CtpPortConfigSet(int prmc, char *prmv[])
{
    GSW_CTP_portConfig_t sVar = {0};
    memset(&sVar, 0x00, sizeof(sVar));
    unsigned int i;
    GSW_Device_t *dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(sVar.nLogicalPortId), &sVar.nLogicalPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nLogicalPortId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(sVar.nSubIfIdGroup), &sVar.nSubIfIdGroup);
    sVar.eMask = 0xFFFFFFFF;

    dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CtpPortConfigGet(dev, &sVar);
    sVar.eMask = 0x0;

    scanParamArg(prmc, prmv, "nBridgePortId", sizeof(sVar.nBridgePortId), &sVar.nBridgePortId);
    if (findStringParam(prmc, prmv, "nBridgePortId"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_BRIDGE_PORT_ID;

    scanParamArg(prmc, prmv, "bForcedTrafficClass", sizeof(sVar.bForcedTrafficClass), &sVar.bForcedTrafficClass);
    if (findStringParam(prmc, prmv, "bForcedTrafficClass"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_FORCE_TRAFFIC_CLASS;

    scanParamArg(prmc, prmv, "nDefaultTrafficClass", sizeof(sVar.nDefaultTrafficClass), &sVar.nDefaultTrafficClass);
    if (findStringParam(prmc, prmv, "nDefaultTrafficClass"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_FORCE_TRAFFIC_CLASS;

    scanParamArg(prmc, prmv, "bIngressExtendedVlanEnable", sizeof(sVar.bIngressExtendedVlanEnable), &sVar.bIngressExtendedVlanEnable);
    if (findStringParam(prmc, prmv, "bIngressExtendedVlanEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_INGRESS_VLAN;

    scanParamArg(prmc, prmv, "nIngressExtendedVlanBlockId", sizeof(sVar.nIngressExtendedVlanBlockId), &sVar.nIngressExtendedVlanBlockId);

    scanParamArg(prmc, prmv, "bIngressExtendedVlanIgmpEnable", sizeof(sVar.bIngressExtendedVlanIgmpEnable), &sVar.bIngressExtendedVlanIgmpEnable);
    if (findStringParam(prmc, prmv, "bIngressExtendedVlanIgmpEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_INGRESS_VLAN_IGMP;

    scanParamArg(prmc, prmv, "nIngressExtendedVlanBlockIdIgmp", sizeof(sVar.nIngressExtendedVlanBlockIdIgmp), &sVar.nIngressExtendedVlanBlockIdIgmp);

    scanParamArg(prmc, prmv, "bEgressExtendedVlanEnable", sizeof(sVar.bEgressExtendedVlanEnable), &sVar.bEgressExtendedVlanEnable);
    if (findStringParam(prmc, prmv, "bEgressExtendedVlanEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_EGRESS_VLAN;

    scanParamArg(prmc, prmv, "nEgressExtendedVlanBlockId", sizeof(sVar.nEgressExtendedVlanBlockId), &sVar.nEgressExtendedVlanBlockId);

    scanParamArg(prmc, prmv, "bEgressExtendedVlanIgmpEnable", sizeof(sVar.bEgressExtendedVlanIgmpEnable), &sVar.bEgressExtendedVlanIgmpEnable);
    if (findStringParam(prmc, prmv, "bEgressExtendedVlanIgmpEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_EGRESS_VLAN_IGMP;

    scanParamArg(prmc, prmv, "nEgressExtendedVlanBlockIdIgmp", sizeof(sVar.nEgressExtendedVlanBlockIdIgmp), &sVar.nEgressExtendedVlanBlockIdIgmp);

    scanParamArg(prmc, prmv, "bIngressNto1VlanEnable", sizeof(sVar.bIngressNto1VlanEnable), &sVar.bIngressNto1VlanEnable);
    if (findStringParam(prmc, prmv, "bIngressNto1VlanEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_INRESS_NTO1_VLAN;

    scanParamArg(prmc, prmv, "bEgressNto1VlanEnable", sizeof(sVar.bEgressNto1VlanEnable), &sVar.bEgressNto1VlanEnable);
    if (findStringParam(prmc, prmv, "bEgressNto1VlanEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_EGRESS_NTO1_VLAN;

    scanParamArg(prmc, prmv, "eIngressMarkingMode", sizeof(sVar.eIngressMarkingMode), &sVar.eIngressMarkingMode);
    if (findStringParam(prmc, prmv, "eIngressMarkingMode"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_INGRESS_MARKING;

    scanParamArg(prmc, prmv, "eEgressMarkingMode", sizeof(sVar.eEgressMarkingMode), &sVar.eEgressMarkingMode);
    if (findStringParam(prmc, prmv, "eEgressMarkingMode"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_EGRESS_MARKING;

    scanParamArg(prmc, prmv, "bEgressMarkingOverrideEnable", sizeof(sVar.bEgressMarkingOverrideEnable), &sVar.bEgressMarkingOverrideEnable);
    if (findStringParam(prmc, prmv, "bEgressMarkingOverrideEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_EGRESS_MARKING_OVERRIDE;

    scanParamArg(prmc, prmv, "eEgressMarkingModeOverride", sizeof(sVar.eEgressMarkingModeOverride), &sVar.eEgressMarkingModeOverride);

    scanParamArg(prmc, prmv, "eEgressRemarkingMode", sizeof(sVar.eEgressRemarkingMode), &sVar.eEgressRemarkingMode);
    if (findStringParam(prmc, prmv, "eEgressRemarkingMode"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_EGRESS_REMARKING;

    scanParamArg(prmc, prmv, "bIngressMeteringEnable", sizeof(sVar.bIngressMeteringEnable), &sVar.bIngressMeteringEnable);

    if (findStringParam(prmc, prmv, "bIngressMeteringEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_INGRESS_METER;

    scanParamArg(prmc, prmv, "nIngressTrafficMeterId", sizeof(sVar.nIngressTrafficMeterId), &sVar.nIngressTrafficMeterId);

    scanParamArg(prmc, prmv, "bEgressMeteringEnable", sizeof(sVar.bEgressMeteringEnable), &sVar.bEgressMeteringEnable);
    if (findStringParam(prmc, prmv, "bEgressMeteringEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_EGRESS_METER;

    scanParamArg(prmc, prmv, "nEgressTrafficMeterId", sizeof(sVar.nEgressTrafficMeterId), &sVar.nEgressTrafficMeterId);

    scanParamArg(prmc, prmv, "bBridgingBypass", sizeof(sVar.bBridgingBypass), &sVar.bBridgingBypass);
    if (findStringParam(prmc, prmv, "bBridgingBypass"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_BRIDGING_BYPASS;

    scanParamArg(prmc, prmv, "nDestLogicalPortId", sizeof(sVar.nDestLogicalPortId), &sVar.nDestLogicalPortId);
    scanParamArg(prmc, prmv, "nDestSubIfIdGroup", sizeof(sVar.nDestSubIfIdGroup), &sVar.nDestSubIfIdGroup);

    scanParamArg(prmc, prmv, "bPmapperEnable", sizeof(sVar.bPmapperEnable), &sVar.bPmapperEnable);
    if (sVar.bPmapperEnable)
    {
        scanParamArg(prmc, prmv, "ePmapperMappingMode", sizeof(sVar.ePmapperMappingMode), &sVar.ePmapperMappingMode);
        scanPMAP_Arg(prmc, prmv, "nPmapperDestSubIfIdGroup", sVar.sPmapper.nDestSubIfIdGroup);
        printf("i.  The first entry of each P-mapper index is for Non-IP and Non-VLAN tagging packets\n");
        printf("ii. The entry 8 to 1 of each P-mapper index is for PCP mapping entries\n");
        printf("iii.The entry 72 to 9 of each P-mapper index is for DSCP mapping entries\n");
        printf("User Configured nDestSubIfIdGroup list as below\n");

        for (i = 0; i <= 72; i++)
            printf("sVar.sPmapper.nDestSubIfIdGroup[%d] = %d\n", i, sVar.sPmapper.nDestSubIfIdGroup[i]);
    }

    scanParamArg(prmc, prmv, "nFirstFlowEntryIndex", sizeof(sVar.nFirstFlowEntryIndex), &sVar.nFirstFlowEntryIndex);
    if (findStringParam(prmc, prmv, "nFirstFlowEntryIndex"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_FLOW_ENTRY;

    scanParamArg(prmc, prmv, "nNumberOfFlowEntries", sizeof(sVar.nNumberOfFlowEntries), &sVar.nNumberOfFlowEntries);

    scanParamArg(prmc, prmv, "bIngressLoopbackEnable", sizeof(sVar.bIngressLoopbackEnable), &sVar.bIngressLoopbackEnable);
    if (findStringParam(prmc, prmv, "bIngressLoopbackEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR;

    scanParamArg(prmc, prmv, "bIngressDaSaSwapEnable", sizeof(sVar.bIngressDaSaSwapEnable), &sVar.bIngressDaSaSwapEnable);
    if (findStringParam(prmc, prmv, "bIngressDaSaSwapEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR;

    scanParamArg(prmc, prmv, "bEgressLoopbackEnable", sizeof(sVar.bEgressLoopbackEnable), &sVar.bEgressLoopbackEnable);
    if (findStringParam(prmc, prmv, "bEgressLoopbackEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR;

    scanParamArg(prmc, prmv, "bEgressDaSaSwapEnable", sizeof(sVar.bEgressDaSaSwapEnable), &sVar.bEgressDaSaSwapEnable);
    if (findStringParam(prmc, prmv, "bEgressDaSaSwapEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR;

    scanParamArg(prmc, prmv, "bIngressMirrorEnable", sizeof(sVar.bIngressMirrorEnable), &sVar.bIngressMirrorEnable);
    if (findStringParam(prmc, prmv, "bIngressMirrorEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR;

    scanParamArg(prmc, prmv, "bEgressMirrorEnable", sizeof(sVar.bEgressMirrorEnable), &sVar.bEgressMirrorEnable);
    if (findStringParam(prmc, prmv, "bEgressMirrorEnable"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR;

    if (findStringParam(prmc, prmv, "bForce"))
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_FORCE;

    ret = GSW_CtpPortConfigSet(dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CtpPortConfigSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_CtpPortConfigSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgeAlloc(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_BRIDGE_alloc_t param = {0};

    memset(&param, 0x00, sizeof(param));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgeAlloc(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgeAlloc failed with ret code", ret);
    else
    {
        printf("\n\t Allocated Bridge ID = %u\n", param.nBridgeId);
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgeFree(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_BRIDGE_alloc_t param = {0};
    int rret;

    memset(&param, 0x00, sizeof(param));

    rret = scanParamArg(prmc, prmv, "nBridgeId", sizeof(param.nBridgeId), &param.nBridgeId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgeId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgeFree(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgeFree failed with ret code", ret);
    else
    {
        printf("fapi_GSW_BridgeFree done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgeConfigGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_BRIDGE_config_t param = {0};
    int rret;

    memset(&param, 0x00, sizeof(param));

    rret = scanParamArg(prmc, prmv, "nBridgeId", sizeof(param.nBridgeId), &param.nBridgeId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgeId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "eMask", sizeof(param.eMask), &param.eMask);
    if (!param.eMask)
        param.eMask = 0xFFFFFFFF;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgeConfigGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgeConfigGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_BridgeConfigGet done\n");
        printf("%40s:\t%u\n", "nBridgeId", param.nBridgeId);
        printf("%40s:\t0x%x\n", "eMask", param.eMask);
        printf("%40s:\t%u\n", "bMacLearningLimitEnable", param.bMacLearningLimitEnable);
        printf("%40s:\t%u\n", "nMacLearningLimit", param.nMacLearningLimit);
        printf("%40s:\t%u\n", "nMacLearningCount", param.nMacLearningCount);
        printf("%40s:\t%u\n", "nLearningDiscardEvent", param.nLearningDiscardEvent);
        printf("%40s:\t%u\n", "eForwardBroadcast", param.eForwardBroadcast);
        printf("%40s:\t%u\n", "eForwardUnknownMulticastIp", param.eForwardUnknownMulticastIp);
        printf("%40s:\t%u\n", "eForwardUnknownMulticastNonIp", param.eForwardUnknownMulticastNonIp);
        printf("%40s:\t%u\n", "eForwardUnknownUnicast", param.eForwardUnknownUnicast);
        printf("%40s:\t%u\n", "bBroadcastMeterEnable", param.bSubMeteringEnable[0]);
        printf("%40s:\t%u\n", "nBroadcastMeterId", param.nTrafficSubMeterId[0]);
        printf("%40s:\t%u\n", "bMulticastMeterEnable", param.bSubMeteringEnable[1]);
        printf("%40s:\t%u\n", "nMulticastMeterId", param.nTrafficSubMeterId[1]);
        printf("%40s:\t%u\n", "bUnknownMulticastIpMeterEnable", param.bSubMeteringEnable[2]);
        printf("%40s:\t%u\n", "nUnknownMulticastIpMeterId", param.nTrafficSubMeterId[2]);
        printf("%40s:\t%u\n", "bUnknownMulticastNonIpMeterEnable", param.bSubMeteringEnable[3]);
        printf("%40s:\t%u\n", "nUnknownMulticastNonIpMeterId", param.nTrafficSubMeterId[3]);
        printf("%40s:\t%u\n", "bUnknownUniCastMeterEnable", param.bSubMeteringEnable[4]);
        printf("%40s:\t%u\n", "nUnknownUniCastMeterId", param.nTrafficSubMeterId[4]);
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgeConfigSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_BRIDGE_config_t sVar = {0};
    memset(&sVar, 0x00, sizeof(sVar));
    int rret;

    rret = scanParamArg(prmc, prmv, "nBridgeId", sizeof(sVar.nBridgeId), &sVar.nBridgeId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgeId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "bMacLearningLimitEnable", sizeof(sVar.bMacLearningLimitEnable), &sVar.bMacLearningLimitEnable);
    if (findStringParam(prmc, prmv, "bMacLearningLimitEnable"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_MAC_LEARNING_LIMIT;

    scanParamArg(prmc, prmv, "nMacLearningLimit", sizeof(sVar.nMacLearningLimit), &sVar.nMacLearningLimit);

    scanParamArg(prmc, prmv, "eForwardBroadcast", sizeof(sVar.eForwardBroadcast), &sVar.eForwardBroadcast);
    if (findStringParam(prmc, prmv, "eForwardBroadcast"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_FORWARDING_MODE;

    scanParamArg(prmc, prmv, "eForwardUnknownMulticastIp", sizeof(sVar.eForwardUnknownMulticastIp), &sVar.eForwardUnknownMulticastIp);
    if (findStringParam(prmc, prmv, "eForwardUnknownMulticastIp"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_FORWARDING_MODE;

    scanParamArg(prmc, prmv, "eForwardUnknownMulticastNonIp", sizeof(sVar.eForwardUnknownMulticastNonIp), &sVar.eForwardUnknownMulticastNonIp);
    if (findStringParam(prmc, prmv, "eForwardUnknownMulticastNonIp"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_FORWARDING_MODE;

    scanParamArg(prmc, prmv, "eForwardUnknownUnicast", sizeof(sVar.eForwardUnknownUnicast), &sVar.eForwardUnknownUnicast);
    if (findStringParam(prmc, prmv, "eForwardUnknownUnicast"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_FORWARDING_MODE;

    scanParamArg(prmc, prmv, "bBroadcastMeterEnable", sizeof(sVar.bSubMeteringEnable[0]), &sVar.bSubMeteringEnable[0]);
    if (findStringParam(prmc, prmv, "bBroadcastMeterEnable"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_SUB_METER;

    scanParamArg(prmc, prmv, "nBroadcastMeterId", sizeof(sVar.nTrafficSubMeterId[0]), &sVar.nTrafficSubMeterId[0]);

    scanParamArg(prmc, prmv, "bMulticastMeterEnable", sizeof(sVar.bSubMeteringEnable[1]), &sVar.bSubMeteringEnable[1]);
    if (findStringParam(prmc, prmv, "bMulticastMeterEnable"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_SUB_METER;

    scanParamArg(prmc, prmv, "nMulticastMeterId", sizeof(sVar.nTrafficSubMeterId[1]), &sVar.nTrafficSubMeterId[1]);

    scanParamArg(prmc, prmv, "bUnknownMulticastIpMeterEnable", sizeof(sVar.bSubMeteringEnable[2]), &sVar.bSubMeteringEnable[2]);
    if (findStringParam(prmc, prmv, "bUnknownMulticastIpMeterEnable"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_SUB_METER;

    scanParamArg(prmc, prmv, "nUnknownMulticastIpMeterId", sizeof(sVar.nTrafficSubMeterId[2]), &sVar.nTrafficSubMeterId[2]);

    scanParamArg(prmc, prmv, "bUnknownMulticastNonIpMeterEnable", sizeof(sVar.bSubMeteringEnable[3]), &sVar.bSubMeteringEnable[3]);
    if (findStringParam(prmc, prmv, "bUnknownMulticastNonIpMeterEnable"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_SUB_METER;

    scanParamArg(prmc, prmv, "nUnknownMulticastNonIpMeterId", sizeof(sVar.nTrafficSubMeterId[3]), &sVar.nTrafficSubMeterId[3]);

    scanParamArg(prmc, prmv, "bUnknownUniCastMeterEnable", sizeof(sVar.bSubMeteringEnable[4]), &sVar.bSubMeteringEnable[4]);
    if (findStringParam(prmc, prmv, "bUnknownUniCastMeterEnable"))
        sVar.eMask |= GSW_BRIDGE_CONFIG_MASK_SUB_METER;

    scanParamArg(prmc, prmv, "nUnknownUniCastMeterId", sizeof(sVar.nTrafficSubMeterId[4]), &sVar.nTrafficSubMeterId[4]);
    if (findStringParam(prmc, prmv, "bForce"))
    {
        sVar.eMask |= GSW_CTP_PORT_CONFIG_MASK_FORCE;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgeConfigSet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgeConfigSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_BridgeConfigSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_ExtendedVlanAlloc(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_EXTENDEDVLAN_alloc_t param = {0};
    int rret;

    memset(&param, 0x00, sizeof(param));

    rret = scanParamArg(prmc, prmv, "nNumberOfEntries", sizeof(param.nNumberOfEntries), &param.nNumberOfEntries);
    if (rret < 1)
    {
        printf("Parameter not Found: nNumberOfEntries\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_ExtendedVlanAlloc(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_ExtendedVlanAlloc failed with ret code", ret);
    else
    {
        printf("\n\tAllocated ExtendedVlanblock = %u", param.nExtendedVlanBlockId);
        printf("\n\tNumber of block entries associated with ExtendedVlanblock %d = %u\n",
               param.nExtendedVlanBlockId, param.nNumberOfEntries);
    }

    return ret;
}

GSW_return_t fapi_GSW_ExtendedVlanFree(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_EXTENDEDVLAN_alloc_t param = {0};
    int rret;

    memset(&param, 0x00, sizeof(param));

    rret = scanParamArg(prmc, prmv, "nExtendedVlanBlockId", sizeof(param.nExtendedVlanBlockId), &param.nExtendedVlanBlockId);
    if (rret < 1)
    {
        printf("Parameter not Found: nExtendedVlanBlockId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_ExtendedVlanFree(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_ExtendedVlanFree failed with ret code", ret);
    else
    {
        printf("\n\tNumber of deleted entries associated with ExVlanblock %d  = %u\n",
               param.nExtendedVlanBlockId, param.nNumberOfEntries);
    }

    return ret;
}

GSW_return_t fapi_GSW_ExtendedVlanGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    unsigned char f = 0;
    GSW_EXTENDEDVLAN_config_t sVar = {0};
    int rret;

    memset(&sVar, 0x00, sizeof(sVar));

    rret = scanParamArg(prmc, prmv, "nExtendedVlanBlockId", sizeof(sVar.nExtendedVlanBlockId), &sVar.nExtendedVlanBlockId);
    if (rret < 1)
    {
        printf("Parameter not Found: nExtendedVlanBlockId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nEntryIndex", sizeof(sVar.nEntryIndex), &sVar.nEntryIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nEntryIndex\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_ExtendedVlanGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_ExtendedVlanGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_ExtendedVlanGet done\n");

        printf("%40s:\t%u\n", "nExtendedVlanBlockId", sVar.nExtendedVlanBlockId);
        printf("%40s:\t%u\n", "nEntryIndex", sVar.nEntryIndex);

        printf("%40s:\t%u\n", "eOuterVlanFilterVlanType", sVar.sFilter.sOuterVlan.eType);
        printf("%40s:\t%u\n", "bOuterVlanFilterPriorityEnable", sVar.sFilter.sOuterVlan.bPriorityEnable);
        printf("%40s:\t%u\n", "nOuterVlanFilterPriorityVal", sVar.sFilter.sOuterVlan.nPriorityVal);
        printf("%40s:\t%u\n", "bOuterVlanFilterVidEnable", sVar.sFilter.sOuterVlan.bVidEnable);
        printf("%40s:\t%u\n", "nOuterVlanFilterVidVal", sVar.sFilter.sOuterVlan.nVidVal);
        printf("%40s:\t%u\n", "eOuterVlanFilterTpid", sVar.sFilter.sOuterVlan.eTpid);
        printf("%40s:\t%u\n", "eOuterVlanFilterDei", sVar.sFilter.sOuterVlan.eDei);

        printf("%40s:\t%u\n", "eInnerVlanFilterVlanType", sVar.sFilter.sInnerVlan.eType);
        printf("%40s:\t%u\n", "bInnerVlanFilterPriorityEnable", sVar.sFilter.sInnerVlan.bPriorityEnable);
        printf("%40s:\t%u\n", "nInnerVlanFilterPriorityVal", sVar.sFilter.sInnerVlan.nPriorityVal);
        printf("%40s:\t%u\n", "bInnerVlanFilterVidEnable", sVar.sFilter.sInnerVlan.bVidEnable);
        printf("%40s:\t%u\n", "nInnerVlanFilterVidVal", sVar.sFilter.sInnerVlan.nVidVal);
        printf("%40s:\t%u\n", "eInnerVlanFilterTpid", sVar.sFilter.sInnerVlan.eTpid);
        printf("%40s:\t%u\n", "eInnerVlanFilterDei", sVar.sFilter.sInnerVlan.eDei);

        printf("%40s:\t%u\n", "eEtherType", sVar.sFilter.eEtherType);
        printf("%40s:\t%u\n", "eRemoveTagAction", sVar.sTreatment.eRemoveTag);

        printf("%40s:\t%u\n", "bOuterVlanActionEnable", sVar.sTreatment.bAddOuterVlan);

        printf("%40s:\t%u\n", "eOuterVlanActionPriorityMode", sVar.sTreatment.sOuterVlan.ePriorityMode);
        printf("%40s:\t%u\n", "eOuterVlanActionPriorityVal", sVar.sTreatment.sOuterVlan.ePriorityVal);
        printf("%40s:\t%u\n", "eOuterVlanActionVidMode", sVar.sTreatment.sOuterVlan.eVidMode);
        printf("%40s:\t%u\n", "eOuterVlanActionVidVal", sVar.sTreatment.sOuterVlan.eVidVal);
        printf("%40s:\t%u\n", "eOuterVlanActionTpid", sVar.sTreatment.sOuterVlan.eTpid);
        printf("%40s:\t%u\n", "eOuterVlanActioneDei", sVar.sTreatment.sOuterVlan.eDei);

        printf("%40s:\t%u\n", "bInnerVlanActionEnable", sVar.sTreatment.bAddInnerVlan);

        printf("%40s:\t%u\n", "eInnerVlanActionPriorityMode", sVar.sTreatment.sInnerVlan.ePriorityMode);
        printf("%40s:\t%u\n", "eInnerVlanActionPriorityVal", sVar.sTreatment.sInnerVlan.ePriorityVal);
        printf("%40s:\t%u\n", "eInnerVlanActionVidMode", sVar.sTreatment.sInnerVlan.eVidMode);
        printf("%40s:\t%u\n", "eInnerVlanActionVidVal", sVar.sTreatment.sInnerVlan.eVidVal);
        printf("%40s:\t%u\n", "eInnerVlanActionTpid", sVar.sTreatment.sInnerVlan.eTpid);
        printf("%40s:\t%u\n", "eInnerVlanActioneDei", sVar.sTreatment.sInnerVlan.eDei);

        printf("%40s:\t%u\n", "bNewDscpEnable", sVar.sTreatment.bNewDscpEnable);
        printf("%40s:\t%u\n", "nNewDscp", sVar.sTreatment.nNewDscp);
        printf("%40s:\t%u\n", "bNewTrafficClassEnable", sVar.sTreatment.bNewTrafficClassEnable);
        printf("%40s:\t%u\n", "nNewTrafficClass", sVar.sTreatment.nNewTrafficClass);
        printf("%40s:\t%u\n", "bNewMeterEnable", sVar.sTreatment.bNewMeterEnable);
        printf("%40s:\t%u\n", "sNewTrafficMeterId", sVar.sTreatment.sNewTrafficMeterId);
        printf("%40s:\t%u\n", "bLoopbackEnable", sVar.sTreatment.bLoopbackEnable);
        printf("%40s:\t%u\n", "bDaSaSwapEnable", sVar.sTreatment.bDaSaSwapEnable);
        printf("%40s:\t%u\n", "bMirrorEnable", sVar.sTreatment.bMirrorEnable);
        printf("%40s:\t%u\n", "bReassignBridgePortEnable", sVar.sTreatment.bReassignBridgePort);
        printf("%40s:\t%u\n", "nNewBridgePortId", sVar.sTreatment.nNewBridgePortId);
        if (sVar.sTreatment.sOuterVlan.ePriorityMode == GSW_EXTENDEDVLAN_TREATMENT_DSCP ||
            sVar.sTreatment.sInnerVlan.ePriorityMode == GSW_EXTENDEDVLAN_TREATMENT_DSCP)
        {
            for (f = 0; f < 64; f++)
                printf("\n\t nDscp2PcpMap[%d] = %u", f, sVar.sTreatment.nDscp2PcpMap[f]);
        }

        scanParamArg(prmc, prmv, "bOriginalPacketFilterMode", sizeof(sVar.sFilter.bOriginalPacketFilterMode), &sVar.sFilter.bOriginalPacketFilterMode);
        scanParamArg(prmc, prmv, "eFilter_4_Tpid_Mode", sizeof(sVar.sFilter.eFilter_4_Tpid_Mode), &sVar.sFilter.eFilter_4_Tpid_Mode);
        scanParamArg(prmc, prmv, "eTreatment_4_Tpid_Mode", sizeof(sVar.sTreatment.eTreatment_4_Tpid_Mode), &sVar.sTreatment.eTreatment_4_Tpid_Mode);
        printf("%40s:\t%u\n", "bOriginalPacketFilterMode", sVar.sFilter.bOriginalPacketFilterMode);
        printf("%40s:\t%u\n", "eFilter_4_Tpid_Mode", sVar.sFilter.eFilter_4_Tpid_Mode);
        printf("%40s:\t%u\n", "eTreatment_4_Tpid_Mode", sVar.sTreatment.eTreatment_4_Tpid_Mode);
    }

    return ret;
}

GSW_return_t fapi_GSW_ExtendedVlanSet(int prmc, char *prmv[])
{
    unsigned char bDscp2PcpMapEnable = 0, nDscp2PcpMapValue = 0, f = 0;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_EXTENDEDVLAN_config_t sVar = {0};

    memset(&sVar, 0x00, sizeof(sVar));

    scanParamArg(prmc, prmv, "nExtendedVlanBlockId", sizeof(sVar.nExtendedVlanBlockId), &sVar.nExtendedVlanBlockId);
    scanParamArg(prmc, prmv, "nEntryIndex", sizeof(sVar.nEntryIndex), &sVar.nEntryIndex);

    scanParamArg(prmc, prmv, "eOuterVlanFilterVlanType", sizeof(sVar.sFilter.sOuterVlan.eType), &sVar.sFilter.sOuterVlan.eType);
    scanParamArg(prmc, prmv, "bOuterVlanFilterPriorityEnable", sizeof(sVar.sFilter.sOuterVlan.bPriorityEnable), &sVar.sFilter.sOuterVlan.bPriorityEnable);
    scanParamArg(prmc, prmv, "nOuterVlanFilterPriorityVal", sizeof(sVar.sFilter.sOuterVlan.nPriorityVal), &sVar.sFilter.sOuterVlan.nPriorityVal);
    scanParamArg(prmc, prmv, "bOuterVlanFilterVidEnable", sizeof(sVar.sFilter.sOuterVlan.bVidEnable), &sVar.sFilter.sOuterVlan.bVidEnable);
    scanParamArg(prmc, prmv, "nOuterVlanFilterVidVal", sizeof(sVar.sFilter.sOuterVlan.nVidVal), &sVar.sFilter.sOuterVlan.nVidVal);
    scanParamArg(prmc, prmv, "eOuterVlanFilterTpid", sizeof(sVar.sFilter.sOuterVlan.eTpid), &sVar.sFilter.sOuterVlan.eTpid);
    scanParamArg(prmc, prmv, "eOuterVlanFilterDei", sizeof(sVar.sFilter.sOuterVlan.eDei), &sVar.sFilter.sOuterVlan.eDei);

    scanParamArg(prmc, prmv, "eInnerVlanFilterVlanType", sizeof(sVar.sFilter.sInnerVlan.eType), &sVar.sFilter.sInnerVlan.eType);
    scanParamArg(prmc, prmv, "bInnerVlanFilterPriorityEnable", sizeof(sVar.sFilter.sInnerVlan.bPriorityEnable), &sVar.sFilter.sInnerVlan.bPriorityEnable);
    scanParamArg(prmc, prmv, "nInnerVlanFilterPriorityVal", sizeof(sVar.sFilter.sInnerVlan.nPriorityVal), &sVar.sFilter.sInnerVlan.nPriorityVal);
    scanParamArg(prmc, prmv, "bInnerVlanFilterVidEnable", sizeof(sVar.sFilter.sInnerVlan.bVidEnable), &sVar.sFilter.sInnerVlan.bVidEnable);
    scanParamArg(prmc, prmv, "nInnerVlanFilterVidVal", sizeof(sVar.sFilter.sInnerVlan.nVidVal), &sVar.sFilter.sInnerVlan.nVidVal);
    scanParamArg(prmc, prmv, "eInnerVlanFilterTpid", sizeof(sVar.sFilter.sInnerVlan.eTpid), &sVar.sFilter.sInnerVlan.eTpid);
    scanParamArg(prmc, prmv, "eInnerVlanFilterDei", sizeof(sVar.sFilter.sInnerVlan.eDei), &sVar.sFilter.sInnerVlan.eDei);

    scanParamArg(prmc, prmv, "eEtherType", sizeof(sVar.sFilter.eEtherType), &sVar.sFilter.eEtherType);
    scanParamArg(prmc, prmv, "eRemoveTagAction", sizeof(sVar.sTreatment.eRemoveTag), &sVar.sTreatment.eRemoveTag);

    scanParamArg(prmc, prmv, "bOuterVlanActionEnable", sizeof(sVar.sTreatment.bAddOuterVlan), &sVar.sTreatment.bAddOuterVlan);
    scanParamArg(prmc, prmv, "eOuterVlanActionPriorityMode", sizeof(sVar.sTreatment.sOuterVlan.ePriorityMode), &sVar.sTreatment.sOuterVlan.ePriorityMode);
    scanParamArg(prmc, prmv, "eOuterVlanActionPriorityVal", sizeof(sVar.sTreatment.sOuterVlan.ePriorityVal), &sVar.sTreatment.sOuterVlan.ePriorityVal);
    scanParamArg(prmc, prmv, "eOuterVlanActionVidMode", sizeof(sVar.sTreatment.sOuterVlan.eVidMode), &sVar.sTreatment.sOuterVlan.eVidMode);
    scanParamArg(prmc, prmv, "eOuterVlanActionVidVal", sizeof(sVar.sTreatment.sOuterVlan.eVidVal), &sVar.sTreatment.sOuterVlan.eVidVal);
    scanParamArg(prmc, prmv, "eOuterVlanActionTpid", sizeof(sVar.sTreatment.sOuterVlan.eTpid), &sVar.sTreatment.sOuterVlan.eTpid);
    scanParamArg(prmc, prmv, "eOuterVlanActioneDei", sizeof(sVar.sTreatment.sOuterVlan.eDei), &sVar.sTreatment.sOuterVlan.eDei);

    scanParamArg(prmc, prmv, "bInnerVlanActionEnable", sizeof(sVar.sTreatment.bAddInnerVlan), &sVar.sTreatment.bAddInnerVlan);
    scanParamArg(prmc, prmv, "eInnerVlanActionPriorityMode", sizeof(sVar.sTreatment.sInnerVlan.ePriorityMode), &sVar.sTreatment.sInnerVlan.ePriorityMode);
    scanParamArg(prmc, prmv, "eInnerVlanActionPriorityVal", sizeof(sVar.sTreatment.sInnerVlan.ePriorityVal), &sVar.sTreatment.sInnerVlan.ePriorityVal);
    scanParamArg(prmc, prmv, "eInnerVlanActionVidMode", sizeof(sVar.sTreatment.sInnerVlan.eVidMode), &sVar.sTreatment.sInnerVlan.eVidMode);
    scanParamArg(prmc, prmv, "eInnerVlanActionVidVal", sizeof(sVar.sTreatment.sInnerVlan.eVidVal), &sVar.sTreatment.sInnerVlan.eVidVal);
    scanParamArg(prmc, prmv, "eInnerVlanActionTpid", sizeof(sVar.sTreatment.sInnerVlan.eTpid), &sVar.sTreatment.sInnerVlan.eTpid);
    scanParamArg(prmc, prmv, "eInnerVlanActioneDei", sizeof(sVar.sTreatment.sInnerVlan.eDei), &sVar.sTreatment.sInnerVlan.eDei);

    scanParamArg(prmc, prmv, "bReassignBridgePortEnable", sizeof(sVar.sTreatment.bReassignBridgePort), &sVar.sTreatment.bReassignBridgePort);
    scanParamArg(prmc, prmv, "nNewBridgePortId", sizeof(sVar.sTreatment.nNewBridgePortId), &sVar.sTreatment.nNewBridgePortId);

    scanParamArg(prmc, prmv, "bNewDscpEnable", sizeof(sVar.sTreatment.bNewDscpEnable), &sVar.sTreatment.bNewDscpEnable);
    scanParamArg(prmc, prmv, "nNewDscp", sizeof(sVar.sTreatment.nNewDscp), &sVar.sTreatment.nNewDscp);

    scanParamArg(prmc, prmv, "bNewTrafficClassEnable", sizeof(sVar.sTreatment.bNewTrafficClassEnable), &sVar.sTreatment.bNewTrafficClassEnable);
    scanParamArg(prmc, prmv, "nNewTrafficClass", sizeof(sVar.sTreatment.nNewTrafficClass), &sVar.sTreatment.nNewTrafficClass);

    scanParamArg(prmc, prmv, "bNewMeterEnable", sizeof(sVar.sTreatment.bNewMeterEnable), &sVar.sTreatment.bNewMeterEnable);
    scanParamArg(prmc, prmv, "sNewTrafficMeterId", sizeof(sVar.sTreatment.sNewTrafficMeterId), &sVar.sTreatment.sNewTrafficMeterId);

    scanParamArg(prmc, prmv, "bLoopbackEnable", sizeof(sVar.sTreatment.bLoopbackEnable), &sVar.sTreatment.bLoopbackEnable);
    scanParamArg(prmc, prmv, "bDaSaSwapEnable", sizeof(sVar.sTreatment.bDaSaSwapEnable), &sVar.sTreatment.bDaSaSwapEnable);
    scanParamArg(prmc, prmv, "bMirrorEnable", sizeof(sVar.sTreatment.bMirrorEnable), &sVar.sTreatment.bMirrorEnable);

    scanParamArg(prmc, prmv, "bDscp2PcpMapEnable", sizeof(bDscp2PcpMapEnable), &bDscp2PcpMapEnable);
    scanParamArg(prmc, prmv, "nDscp2PcpMapValue", sizeof(nDscp2PcpMapValue), &nDscp2PcpMapValue);
    if (bDscp2PcpMapEnable)
    {
        for (f = 0; f < 64; f++)
            sVar.sTreatment.nDscp2PcpMap[f] = nDscp2PcpMapValue;
    }

    scanParamArg(prmc, prmv, "bOriginalPacketFilterMode", sizeof(sVar.sFilter.bOriginalPacketFilterMode), &sVar.sFilter.bOriginalPacketFilterMode);
    scanParamArg(prmc, prmv, "eFilter_4_Tpid_Mode", sizeof(sVar.sFilter.eFilter_4_Tpid_Mode), &sVar.sFilter.eFilter_4_Tpid_Mode);
    scanParamArg(prmc, prmv, "eTreatment_4_Tpid_Mode", sizeof(sVar.sTreatment.eTreatment_4_Tpid_Mode), &sVar.sTreatment.eTreatment_4_Tpid_Mode);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_ExtendedVlanSet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_ExtendedVlanSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_ExtendedVlanSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_VlanFilterAlloc(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VLANFILTER_alloc_t param = {0};
    int rret;

    memset(&param, 0x00, sizeof(param));

    rret = scanParamArg(prmc, prmv, "nNumberOfEntries", sizeof(param.nNumberOfEntries), &param.nNumberOfEntries);
    if (rret < 1)
    {
        printf("Parameter not Found: nNumberOfEntries\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "bDiscardUntagged", sizeof(param.bDiscardUntagged), &param.bDiscardUntagged);
    if (rret < 1)
    {
        printf("Parameter not Found: bDiscardUntagged\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "bDiscardUnmatchedTagged", sizeof(param.bDiscardUnmatchedTagged), &param.bDiscardUnmatchedTagged);
    if (rret < 1)
    {
        printf("Parameter not Found: bDiscardUnmatchedTagged\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_VlanFilterAlloc(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_VlanFilterAlloc failed with ret code", ret);
    else
    {
        printf("\n\tAllocated VlanFilterblock = %u", param.nVlanFilterBlockId);
        printf("\n\t Number of block entries associated with VlanFilterblock %d = %u\n",
               param.nVlanFilterBlockId, param.nNumberOfEntries);
    }

    return ret;
}

GSW_return_t fapi_GSW_VlanFilterFree(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VLANFILTER_alloc_t param = {0};
    int rret;

    memset(&param, 0x00, sizeof(param));

    rret = scanParamArg(prmc, prmv, "nVlanFilterBlockId", sizeof(param.nVlanFilterBlockId), &param.nVlanFilterBlockId);
    if (rret < 1)
    {
        printf("Parameter not Found: nVlanFilterBlockId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_VlanFilterFree(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_VlanFilterFree failed with ret code", ret);
    else
    {
        printf("\n\t Number of deleted entries associated with VlanFilterblock %d  = %u\n",
               param.nVlanFilterBlockId, param.nNumberOfEntries);
    }

    return ret;
}

GSW_return_t fapi_GSW_VlanFilterGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    unsigned char f = 0;

    GSW_VLANFILTER_config_t sVar = {0};
    int rret;

    memset(&sVar, 0x00, sizeof(sVar));

    rret = scanParamArg(prmc, prmv, "nVlanFilterBlockId", sizeof(sVar.nVlanFilterBlockId), &sVar.nVlanFilterBlockId);
    if (rret < 1)
    {
        printf("Parameter not Found: nVlanFilterBlockId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nEntryIndex", sizeof(sVar.nEntryIndex), &sVar.nEntryIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nEntryIndex\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_VlanFilterGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_VlanFilterGet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_VlanFilterGet done\n");
        printf("%40s:\t%u\n", "nVlanFilterBlockId", sVar.nVlanFilterBlockId);
        printf("%40s:\t%u\n", "nEntryIndex", sVar.nEntryIndex);
        printf("%40s:\t%u\n", "eVlanFilterMask", sVar.eVlanFilterMask);
        printf("%40s:\t%u\n", "nVal", sVar.nVal);
        printf("%40s:\t%u\n", "bDiscardMatched", sVar.bDiscardMatched);
    }

    return ret;
}

GSW_return_t fapi_GSW_VlanFilterSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    unsigned char f = 0;
    GSW_VLANFILTER_config_t sVar = {0};
    int rret;

    memset(&sVar, 0x00, sizeof(sVar));

    rret = scanParamArg(prmc, prmv, "nVlanFilterBlockId", sizeof(sVar.nVlanFilterBlockId), &sVar.nVlanFilterBlockId);
    if (rret < 1)
    {
        printf("Parameter not Found: nVlanFilterBlockId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nEntryIndex", sizeof(sVar.nEntryIndex), &sVar.nEntryIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nEntryIndex\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "eVlanFilterMask", sizeof(sVar.eVlanFilterMask), &sVar.eVlanFilterMask);
    if (rret < 1)
    {
        printf("Parameter not Found: eVlanFilterMask\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nVal", sizeof(sVar.nVal), &sVar.nVal);
    if (rret < 1)
    {
        printf("Parameter not Found: nVal\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "bDiscardMatched", sizeof(sVar.bDiscardMatched), &sVar.bDiscardMatched);
    if (rret < 1)
    {
        printf("Parameter not Found: bDiscardMatched\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_VlanFilterSet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_VlanFilterSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_VlanFilterSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_STP_PortCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_STP_portCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_STP_PortCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_STP_PortCfgGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nPortId", param.nPortId);
        printf("\t%40s:\t%x\n", "ePortState", param.ePortState);
    }

    return ret;
}

GSW_return_t fapi_GSW_STP_PortCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_STP_portCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "ePortState", sizeof(param.ePortState), &param.ePortState);
    if (rret < 1)
    {
        printf("Parameter not Found: ePortState\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_STP_PortCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_STP_PortCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_STP_PortCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_STP_BPDU_RuleGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_STP_BPDU_Rule_t param = {0};
    int rret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_STP_BPDU_RuleGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_STP_BPDU_RuleGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "eForwardPort", param.eForwardPort);
        printf("\t%40s:\t%x\n", "nForwardPortId", param.nForwardPortId);
    }

    return ret;
}

GSW_return_t fapi_GSW_STP_BPDU_RuleSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_STP_BPDU_Rule_t param = {0};
    int rret;

    scanParamArg(prmc, prmv, "eForwardPort", sizeof(param.eForwardPort), &param.eForwardPort);
    scanParamArg(prmc, prmv, "nForwardPortId", sizeof(param.nForwardPortId), &param.nForwardPortId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_STP_BPDU_RuleSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_STP_BPDU_RuleSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_STP_BPDU_RuleSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_MeterCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_meterCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nMeterId", sizeof(param.nMeterId), &param.nMeterId);
    if (rret < 1)
    {
        printf("Parameter not Found: nMeterId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_MeterCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_MeterCfgGet failed with ret code", ret);
    else
    {
        printf("Returned values:\n----------------\n");
        printf("\t%40s:\t%s\n", "bEnable", (param.bEnable > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%x\n", "nMeterId", param.nMeterId);
        printf("\t%40s:\t%x\n", "eMtrType", param.eMtrType);
        printf("\t%40s:\t%u (0x%x)\n", "nCbs", param.nCbs, param.nCbs);
        printf("\t%40s:\t%u (0x%x)\n", "nEbs", param.nEbs, param.nEbs);
        printf("\t%40s:\t%u (0x%x)\n", "nRate", param.nRate, param.nRate);
        printf("\t%40s:\t%u (0x%x)\n", "nPiRate", param.nPiRate, param.nPiRate);
        // printf("\t%40s:\t%x\n", "cMeterName",  param.cMeterName);
        printf("\t%40s:\t%x\n", "nColourBlindMode", param.nColourBlindMode);
        printf("\t%40s:\t%x\n", "bPktMode", param.bPktMode);
        printf("\t%40s:\t%s\n", "bLocalOverhd", (param.bLocalOverhd > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%u (0x%x)\n", "nLocaloverhd", param.nLocaloverhd, param.nLocaloverhd);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_MeterCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_meterCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nMeterId", sizeof(param.nMeterId), &param.nMeterId);
    if (rret < 1)
    {
        printf("Parameter not Found: nMeterId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "bEnable", sizeof(param.bEnable), &param.bEnable);
    scanParamArg(prmc, prmv, "eMtrType", sizeof(param.eMtrType), &param.eMtrType);
    scanParamArg(prmc, prmv, "nCbs", sizeof(param.nCbs), &param.nCbs);
    scanParamArg(prmc, prmv, "nEbs", sizeof(param.nEbs), &param.nEbs);
    scanParamArg(prmc, prmv, "nRate", sizeof(param.nRate), &param.nRate);
    scanParamArg(prmc, prmv, "nPiRate", sizeof(param.nPiRate), &param.nPiRate);
    scanParamArg(prmc, prmv, "cMeterName", sizeof(param.cMeterName), &param.cMeterName);
    scanParamArg(prmc, prmv, "nColourBlindMode", sizeof(param.nColourBlindMode), &param.nColourBlindMode);
    scanParamArg(prmc, prmv, "bPktMode", sizeof(param.bPktMode), &param.bPktMode);
    scanParamArg(prmc, prmv, "bLocalOverhd", sizeof(param.bLocalOverhd), &param.bLocalOverhd);
    scanParamArg(prmc, prmv, "nLocaloverhd", sizeof(param.nLocaloverhd), &param.nLocaloverhd);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_MeterCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_MeterCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_MeterCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_DefaultFilterGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_MACFILTER_default_t param = {0};
    int rret, i;

    rret = scanParamArg(prmc, prmv, "eType", sizeof(param.eType), &param.eType);
    if (rret < 1)
    {
        printf("Parameter not Found: eType\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_DefaultMacFilterGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MAC_DefaultFilterGet failed with ret code", ret);
    else
    {
        printf("Returned values:\n----------------\n");
        for (i = 0; i <= 7; i++)
            printf("\t nPortmap[%d]            = 0x%x\n", i, param.nPortmap[i]);
    }

    return ret;
}

GSW_return_t fapi_GSW_MAC_DefaultFilterSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_MACFILTER_default_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "eType", sizeof(param.eType), &param.eType);
    if (rret < 1)
    {
        printf("Parameter not Found: eType\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_DefaultMacFilterSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MAC_DefaultFilterSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MAC_DefaultFilterSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_CTP_PortAssignmentGet(int prmc, char *prmv[])
{
    GSW_CTP_portAssignment_t sVar = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(sVar.nLogicalPortId), &sVar.nLogicalPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nLogicalPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CTP_PortAssignmentGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CTP_PortAssignmentGet failed with ret code", ret);
    else
    {
        printf("\n\t nLogicalPortId         = %u", sVar.nLogicalPortId);
        printf("\n\t nFirstCtpPortId        = %u", sVar.nFirstCtpPortId);
        printf("\n\t nNumberOfCtpPort       = %u", sVar.nNumberOfCtpPort);
        printf("\n\t eMode                  = %u", sVar.eMode);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_CTP_PortAssignmentSet(int prmc, char *prmv[])
{
    GSW_CTP_portAssignment_t sVar = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(sVar.nLogicalPortId), &sVar.nLogicalPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nLogicalPortId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nFirstCtpPortId", sizeof(sVar.nFirstCtpPortId), &sVar.nFirstCtpPortId);
    scanParamArg(prmc, prmv, "nNumberOfCtpPort", sizeof(sVar.nNumberOfCtpPort), &sVar.nNumberOfCtpPort);
    scanParamArg(prmc, prmv, "eMode", sizeof(sVar.eMode), &sVar.eMode);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CTP_PortAssignmentSet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CTP_PortAssignmentSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_CTP_PortAssignmentSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_IG_CfgSet(int prmc, char *prmv[])
{
    GSW_PMAC_Ig_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nTxDmaChanId", sizeof(param.nTxDmaChanId), &param.nTxDmaChanId);
    scanParamArg(prmc, prmv, "bErrPktsDisc", sizeof(param.bErrPktsDisc), &param.bErrPktsDisc);
    scanParamArg(prmc, prmv, "bPmapDefault", sizeof(param.bPmapDefault), &param.bPmapDefault);
    scanParamArg(prmc, prmv, "bPmapEna", sizeof(param.bPmapEna), &param.bPmapEna);
    scanParamArg(prmc, prmv, "bClassDefault", sizeof(param.bClassDefault), &param.bClassDefault);
    scanParamArg(prmc, prmv, "bClassEna", sizeof(param.bClassEna), &param.bClassEna);
    scanParamArg(prmc, prmv, "eSubId", sizeof(param.eSubId), &param.eSubId);
    scanParamArg(prmc, prmv, "bSpIdDefault", sizeof(param.bSpIdDefault), &param.bSpIdDefault);
    scanParamArg(prmc, prmv, "bPmacPresent", sizeof(param.bPmacPresent), &param.bPmacPresent);
    scanPMAC_Arg(prmc, prmv, "defPmacHdr", param.defPmacHdr);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_IG_CfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_IG_CfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PMAC_IG_CfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_IG_CfgGet(int prmc, char *prmv[])
{
    GSW_PMAC_Ig_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nTxDmaChanId", sizeof(param.nTxDmaChanId), &param.nTxDmaChanId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_IG_CfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_IG_CfgGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nPmacId	", param.nPmacId);
        printf("\t%40s:\t%x\n", "nTxDmaChanId", param.nTxDmaChanId);
        printf("\t%40s:\t%x\n", "bErrPktsDisc", param.bErrPktsDisc);
        printf("\t%40s:\t%x\n", "bPmapDefault", param.bPmapDefault);
        printf("\t%40s:\t%x\n", "bPmapEna", param.bPmapEna);
        printf("\t%40s:\t%x\n", "bClassDefault", param.bClassDefault);
        printf("\t%40s:\t%x\n", "bClassEna", param.bClassEna);
        printf("\t%40s:\t%x\n", "eSubId	", param.eSubId);
        printf("\t%40s:\t%x\n", "bSpIdDefault", param.bSpIdDefault);
        printf("\t%40s:\t%x\n", "bPmacPresent", param.bPmacPresent);
        printf("\t%40s:\t", "defPmacHdr");
        printf("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
               param.defPmacHdr[0],
               param.defPmacHdr[1],
               param.defPmacHdr[2],
               param.defPmacHdr[3],
               param.defPmacHdr[4],
               param.defPmacHdr[5],
               param.defPmacHdr[6],
               param.defPmacHdr[7]);
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_EG_CfgGet(int prmc, char *prmv[])
{
    GSW_PMAC_Eg_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nDestPortId", sizeof(param.nDestPortId), &param.nDestPortId);
    scanParamArg(prmc, prmv, "bProcFlagsSelect", sizeof(param.bProcFlagsSelect), &param.bProcFlagsSelect);
    scanParamArg(prmc, prmv, "nTrafficClass", sizeof(param.nTrafficClass), &param.nTrafficClass);
    scanParamArg(prmc, prmv, "nFlowIDMsb", sizeof(param.nFlowIDMsb), &param.nFlowIDMsb);
    scanParamArg(prmc, prmv, "bMpe1Flag", sizeof(param.bMpe1Flag), &param.bMpe1Flag);
    scanParamArg(prmc, prmv, "bMpe2Flag", sizeof(param.bMpe2Flag), &param.bMpe2Flag);
    scanParamArg(prmc, prmv, "bEncFlag", sizeof(param.bEncFlag), &param.bEncFlag);
    scanParamArg(prmc, prmv, "bDecFlag", sizeof(param.bDecFlag), &param.bDecFlag);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_EG_CfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_EG_CfgGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nPmacId	", param.nPmacId);
        printf("\t%40s:\t%x\n", "nDestPortId", param.nDestPortId);
        printf("\t%40s:\t%x\n", "nTrafficClass", param.nTrafficClass);
        printf("\t%40s:\t%x\n", "bMpe1Flag", param.bMpe1Flag);
        printf("\t%40s:\t%x\n", "bMpe2Flag", param.bMpe2Flag);
        printf("\t%40s:\t%x\n", "bDecFlag", param.bDecFlag);
        printf("\t%40s:\t%x\n", "bEncFlag", param.bEncFlag);
        printf("\t%40s:\t%x\n", "nFlowIDMsb", param.nFlowIDMsb);
        printf("\t%40s:\t%x\n", "bProcFlagsSelect", param.bProcFlagsSelect);
        printf("\t%40s:\t%x\n", "nRxDmaChanId", param.nRxDmaChanId);
        printf("\t%40s:\t%x\n", "bRemL2Hdr", param.bRemL2Hdr);
        printf("\t%40s:\t%x\n", "numBytesRem", param.numBytesRem);
        printf("\t%40s:\t%x\n", "bFcsEna	", param.bFcsEna);
        printf("\t%40s:\t%x\n", "bPmacEna", param.bPmacEna);
        printf("\t%40s:\t%x\n", "bRedirEnable", param.bRedirEnable);
        printf("\t%40s:\t%x\n", "bBslSegmentDisable", param.bBslSegmentDisable);
        printf("\t%40s:\t%x\n", "nBslTrafficClass", param.nBslTrafficClass);
        printf("\t%40s:\t%x\n", "bResDW1Enable", param.bResDW1Enable);
        printf("\t%40s:\t%x\n", "nResDW1", param.nResDW1);
        printf("\t%40s:\t%x\n", "bRes1DW0Enable", param.bRes1DW0Enable);
        printf("\t%40s:\t%x\n", "nRes1DW0", param.nRes1DW0);
        printf("\t%40s:\t%x\n", "bRes2DW0Enable", param.bRes2DW0Enable);
        printf("\t%40s:\t%x\n", "nRes2DW0", param.nRes2DW0);
        printf("\t%40s:\t%x\n", "bTCEnable", param.bTCEnable);
    }
    return ret;
}

GSW_return_t fapi_GSW_PMAC_EG_CfgSet(int prmc, char *prmv[])
{
    GSW_PMAC_Eg_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "bRedirEnable", sizeof(param.bRedirEnable), &param.bRedirEnable);
    scanParamArg(prmc, prmv, "bBslSegmentDisable", sizeof(param.bBslSegmentDisable), &param.bBslSegmentDisable);
    scanParamArg(prmc, prmv, "nBslTrafficClass", sizeof(param.nBslTrafficClass), &param.nBslTrafficClass);
    scanParamArg(prmc, prmv, "bResDW1Enable", sizeof(param.bResDW1Enable), &param.bResDW1Enable);
    scanParamArg(prmc, prmv, "bRes2DW0Enable", sizeof(param.bRes2DW0Enable), &param.bRes2DW0Enable);
    scanParamArg(prmc, prmv, "bRes1DW0Enable", sizeof(param.bRes1DW0Enable), &param.bRes1DW0Enable);
    scanParamArg(prmc, prmv, "bTCEnable", sizeof(param.bTCEnable), &param.bTCEnable);
    scanParamArg(prmc, prmv, "nDestPortId", sizeof(param.nDestPortId), &param.nDestPortId);
    scanParamArg(prmc, prmv, "bProcFlagsSelect", sizeof(param.bProcFlagsSelect), &param.bProcFlagsSelect);
    scanParamArg(prmc, prmv, "nTrafficClass", sizeof(param.nTrafficClass), &param.nTrafficClass);
    scanParamArg(prmc, prmv, "nFlowIDMsb", sizeof(param.nFlowIDMsb), &param.nFlowIDMsb);
    scanParamArg(prmc, prmv, "bMpe1Flag", sizeof(param.bMpe1Flag), &param.bMpe1Flag);
    scanParamArg(prmc, prmv, "bMpe2Flag", sizeof(param.bMpe2Flag), &param.bMpe2Flag);
    scanParamArg(prmc, prmv, "bEncFlag", sizeof(param.bEncFlag), &param.bEncFlag);
    scanParamArg(prmc, prmv, "bDecFlag", sizeof(param.bDecFlag), &param.bDecFlag);
    scanParamArg(prmc, prmv, "nRxDmaChanId", sizeof(param.nRxDmaChanId), &param.nRxDmaChanId);
    scanParamArg(prmc, prmv, "bRemL2Hdr", sizeof(param.bRemL2Hdr), &param.bRemL2Hdr);
    scanParamArg(prmc, prmv, "numBytesRem", sizeof(param.numBytesRem), &param.numBytesRem);
    scanParamArg(prmc, prmv, "bFcsEna", sizeof(param.bFcsEna), &param.bFcsEna);
    scanParamArg(prmc, prmv, "bPmacEna", sizeof(param.bPmacEna), &param.bPmacEna);
    scanParamArg(prmc, prmv, "nResDW1", sizeof(param.nResDW1), &param.nResDW1);
    scanParamArg(prmc, prmv, "nRes1DW0", sizeof(param.nRes1DW0), &param.nRes1DW0);
    scanParamArg(prmc, prmv, "nRes2DW0", sizeof(param.nRes2DW0), &param.nRes2DW0);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_EG_CfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_EG_CfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PMAC_EG_CfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_BM_CfgGet(int prmc, char *prmv[])
{
    GSW_PMAC_BM_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nTxDmaChanId", sizeof(param.nTxDmaChanId), &param.nTxDmaChanId);
    if (rret < 1)
    {
        printf("Parameter not Found: nTxDmaChanId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_BM_CfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_BM_CfgGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nTxDmaChanId	", param.nTxDmaChanId);
        printf("\t%40s:\t%x\n", "txQMask	", param.txQMask);
        printf("\t%40s:\t%x\n", "rxPortMask	", param.rxPortMask);
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_BM_CfgSet(int prmc, char *prmv[])
{
    GSW_PMAC_BM_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nTxDmaChanId", sizeof(param.nTxDmaChanId), &param.nTxDmaChanId);
    if (rret < 1)
    {
        printf("Parameter not Found: nTxDmaChanId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "txQMask", sizeof(param.txQMask), &param.txQMask);
    scanParamArg(prmc, prmv, "rxPortMask", sizeof(param.rxPortMask), &param.rxPortMask);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_BM_CfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_BM_CfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PMAC_BM_CfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_GLBL_CfgGet(int prmc, char *prmv[])
{
    GSW_PMAC_Glbl_Cfg_t param = {0};
    unsigned int i = 0;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_GLBL_CfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_GLBL_CfgGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nPmacId", param.nPmacId);
        printf("\t%40s:\t%x\n", "bAPadEna", param.bAPadEna);
        printf("\t%40s:\t%x\n", "bPadEna", param.bPadEna);
        printf("\t%40s:\t%x\n", "bVPadEna", param.bVPadEna);
        printf("\t%40s:\t%x\n", "bSVPadEna", param.bSVPadEna);
        printf("\t%40s:\t%x\n", "bRxFCSDis", param.bRxFCSDis);
        printf("\t%40s:\t%x\n", "bTxFCSDis", param.bTxFCSDis);
        printf("\t%40s:\t%x\n", "bIPTransChkRegDis", param.bIPTransChkRegDis);
        printf("\t%40s:\t%x\n", "bIPTransChkVerDis", param.bIPTransChkVerDis);
        printf("\t%40s:\t%x\n", "bJumboEna", param.bJumboEna);
        printf("\t%40s:\t%u\n", "nMaxJumboLen", param.nMaxJumboLen);
        printf("\t%40s:\t%x\n", "nJumboThreshLen", param.nJumboThreshLen);
        printf("\t%40s:\t%x\n", "bLongFrmChkDis", param.bLongFrmChkDis);
        printf("\t%40s:\t%x\n", "eShortFrmChkType", param.eShortFrmChkType);
        printf("\t%40s:\t%x\n", "bProcFlagsEgCfgEna", param.bProcFlagsEgCfgEna);
        printf("\t%40s:\t%x\n", "eProcFlagsEgCfg", param.eProcFlagsEgCfg);

        for (i = 0; i <= 2; i++)
            printf("\t%40s[%i]:\t%u\n", "nBslThreshold", i, param.nBslThreshold[i]);
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_GLBL_CfgSet(int prmc, char *prmv[])
{
    GSW_PMAC_Glbl_Cfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_GLBL_CfgGet(gsw_dev, &param);

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(param.nPmacId), &param.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "bRxFCSDis", sizeof(param.bRxFCSDis), &param.bRxFCSDis);
    scanParamArg(prmc, prmv, "eProcFlagsEgCfg", sizeof(param.eProcFlagsEgCfg), &param.eProcFlagsEgCfg);
    scanParamArg(prmc, prmv, "nBslThreshold0", sizeof(param.nBslThreshold[0]), &param.nBslThreshold[0]);
    scanParamArg(prmc, prmv, "nBslThreshold1", sizeof(param.nBslThreshold[1]), &param.nBslThreshold[1]);
    scanParamArg(prmc, prmv, "nBslThreshold2", sizeof(param.nBslThreshold[2]), &param.nBslThreshold[2]);
    scanParamArg(prmc, prmv, "bAPadEna", sizeof(param.bAPadEna), &param.bAPadEna);
    scanParamArg(prmc, prmv, "bPadEna", sizeof(param.bPadEna), &param.bPadEna);
    scanParamArg(prmc, prmv, "bVPadEna", sizeof(param.bVPadEna), &param.bVPadEna);
    scanParamArg(prmc, prmv, "bSVPadEna", sizeof(param.bSVPadEna), &param.bSVPadEna);
    scanParamArg(prmc, prmv, "bTxFCSDis", sizeof(param.bTxFCSDis), &param.bTxFCSDis);
    scanParamArg(prmc, prmv, "bIPTransChkRegDis", sizeof(param.bIPTransChkRegDis), &param.bIPTransChkRegDis);
    scanParamArg(prmc, prmv, "bIPTransChkVerDis", sizeof(param.bIPTransChkVerDis), &param.bIPTransChkVerDis);
    scanParamArg(prmc, prmv, "bJumboEna", sizeof(param.bJumboEna), &param.bJumboEna);
    scanParamArg(prmc, prmv, "nMaxJumboLen", sizeof(param.nMaxJumboLen), &param.nMaxJumboLen);
    scanParamArg(prmc, prmv, "nJumboThreshLen", sizeof(param.nJumboThreshLen), &param.nJumboThreshLen);
    scanParamArg(prmc, prmv, "bLongFrmChkDis", sizeof(param.bLongFrmChkDis), &param.bLongFrmChkDis);
    scanParamArg(prmc, prmv, "eShortFrmChkType", sizeof(param.eShortFrmChkType), &param.eShortFrmChkType);
    scanParamArg(prmc, prmv, "bProcFlagsEgCfgEna", sizeof(param.bProcFlagsEgCfgEna), &param.bProcFlagsEgCfgEna);

    ret = GSW_PMAC_GLBL_CfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_GLBL_CfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PMAC_GLBL_CfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleRead(int prmc, char *prmv[])
{
    GSW_PCE_rule_t pce_rule = {0};
    int i;

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "pattern.nIndex", sizeof(pce_rule.pattern.nIndex), &pce_rule.pattern.nIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: pattern.nIndex\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(pce_rule.logicalportid), &pce_rule.logicalportid);
    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(pce_rule.subifidgroup), &pce_rule.subifidgroup);
    scanParamArg(prmc, prmv, "region", sizeof(pce_rule.region), &pce_rule.region);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleRead(gsw_dev, &pce_rule);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleRead failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PceRuleRead done\n");

        if (pce_rule.pattern.bEnable)
        {
            printf("\n\tp.nIndex                                           = %u", pce_rule.pattern.nIndex);

            if (pce_rule.pattern.bMAC_DstEnable)
            {
                printf("\n\tp.bMAC_DstEnable                                   = %u", pce_rule.pattern.bMAC_DstEnable);
                printf("\n\tp.bDstMAC_Exclude                                  = %u", pce_rule.pattern.bDstMAC_Exclude);
                printf("\n\tp.nMAC_Dst                                         = ");

                for (i = 0; i < 6; i++)
                {
                    printf("%2.2x", pce_rule.pattern.nMAC_Dst[i]);
                }
                printf("\n\tp.nMAC_DstMask                                     = 0x%x", pce_rule.pattern.nMAC_DstMask);
            }

            if (pce_rule.pattern.bMAC_SrcEnable)
            {
                printf("\n\tp.bMAC_SrcEnable                                   = %u", pce_rule.pattern.bMAC_SrcEnable);
                printf("\n\tp.bSrcMAC_Exclude                                  = %u", pce_rule.pattern.bSrcMAC_Exclude);
                printf("\n\tp.nMAC_Src                                         = ");

                for (i = 0; i < 6; i++)
                {
                    printf("%2.2x", pce_rule.pattern.nMAC_Src[i]);
                }
                printf("\n\tp.nMAC_SrcMask                                     = 0x%x", pce_rule.pattern.nMAC_SrcMask);
            }

            if (pce_rule.pattern.eDstIP_Select)
            {
                printf("\n\tp.eDstIP_Select                                    = %u", pce_rule.pattern.eDstIP_Select);
                printf("\n\tp.bDstIP_Exclude                                   = %u", pce_rule.pattern.bDstIP_Exclude);

                if (pce_rule.pattern.eDstIP_Select == GSW_PCE_IP_V4)
                {
                    printf("\n\tp.nDstIP                                           = 0x%x", pce_rule.pattern.nDstIP.nIPv4);
                    printf("\n\tp.nDstIP_Mask                                      = 0x%x", pce_rule.pattern.nDstIP_Mask);
                }
                else if (pce_rule.pattern.eDstIP_Select == GSW_PCE_IP_V6)
                {
                    printf("\n\tp.nDstIP                                           = ");

                    for (i = 0; i < 8; i++)
                    {
                        if (i == 7)
                            printf("%x", pce_rule.pattern.nDstIP.nIPv6[i]);
                        else
                            printf("%x:", pce_rule.pattern.nDstIP.nIPv6[i]);
                    }
                    printf("\n\tp.nDstIP_Mask                                      = 0x%x", pce_rule.pattern.nDstIP_Mask);
                }
            }

            if (pce_rule.pattern.eInnerDstIP_Select)
            {
                printf("\n\tp.eInnerDstIP_Select                               = %u", pce_rule.pattern.eInnerDstIP_Select);
                printf("\n\tp.bInnerDstIP_Exclude                              = %u", pce_rule.pattern.bInnerDstIP_Exclude);

                if (pce_rule.pattern.eInnerDstIP_Select == GSW_PCE_IP_V4)
                {
                    printf("\n\tp.nInnerDstIP                                      = 0x%x", pce_rule.pattern.nInnerDstIP.nIPv4);
                    printf("\n\tp.nInnerDstIP_Mask                                 = 0x%x", pce_rule.pattern.nInnerDstIP_Mask);
                }
                else if (pce_rule.pattern.eInnerDstIP_Select == GSW_PCE_IP_V6)
                {
                    printf("\n\tp.nInnerDstIP                                      = ");

                    for (i = 0; i < 8; i++)
                    {
                        if (i == 7)
                            printf("%x", pce_rule.pattern.nInnerDstIP.nIPv6[i]);
                        else
                            printf("%x:", pce_rule.pattern.nInnerDstIP.nIPv6[i]);
                    }
                    printf("\n\tp.nInnerDstIP_Mask                                 = 0x%x", pce_rule.pattern.nInnerDstIP_Mask);
                }
            }

            if (pce_rule.pattern.eSrcIP_Select)
            {
                printf("\n\tp.eSrcIP_Select                                    = %u", pce_rule.pattern.eSrcIP_Select);
                printf("\n\tp.bSrcIP_Exclude                                   = %u", pce_rule.pattern.bSrcIP_Exclude);

                if (pce_rule.pattern.eSrcIP_Select == GSW_PCE_IP_V4)
                {
                    printf("\n\tp.nSrcIP                                           = 0x%x", pce_rule.pattern.nSrcIP.nIPv4);
                    printf("\n\tp.nSrcIP_Mask                                      = 0x%x", pce_rule.pattern.nSrcIP_Mask);
                }
                else if (pce_rule.pattern.eSrcIP_Select == GSW_PCE_IP_V6)
                {
                    printf("\n\tp.nSrcIP                                           = ");

                    for (i = 0; i < 8; i++)
                    {
                        if (i == 7)
                            printf("%x", pce_rule.pattern.nSrcIP.nIPv6[i]);
                        else
                            printf("%x:", pce_rule.pattern.nSrcIP.nIPv6[i]);
                    }
                    printf("\n\tp.nSrcIP_Mask                                      = 0x%x", pce_rule.pattern.nSrcIP_Mask);
                }
            }

            if (pce_rule.pattern.eInnerSrcIP_Select)
            {
                printf("\n\tp.eInnerSrcIP_Select                               = %u", pce_rule.pattern.eInnerSrcIP_Select);
                printf("\n\tp.bInnerSrcIP_Exclude                              = %u", pce_rule.pattern.bInnerSrcIP_Exclude);

                if (pce_rule.pattern.eInnerSrcIP_Select == GSW_PCE_IP_V4)
                {
                    printf("\n\tp.nInnerSrcIP                                      = 0x%x", pce_rule.pattern.nInnerSrcIP.nIPv4);
                    printf("\n\tp.nInnerSrcIP_Mask                                 = 0x%x", pce_rule.pattern.nInnerSrcIP_Mask);
                }
                else if (pce_rule.pattern.eInnerSrcIP_Select == GSW_PCE_IP_V6)
                {
                    printf("\n\tp.nInnerSrcIP                                      = ");

                    for (i = 0; i < 8; i++)
                    {
                        if (i == 7)
                            printf("%x", pce_rule.pattern.nInnerSrcIP.nIPv6[i]);
                        else
                            printf("%x:", pce_rule.pattern.nInnerSrcIP.nIPv6[i]);
                    }
                    printf("\n\tp.nInnerSrcIP_Mask                                 = 0x%x", pce_rule.pattern.nInnerSrcIP_Mask);
                }
            }

            if (pce_rule.pattern.bVid)
            {
                printf("\n\tp.bVid                                             = %u", pce_rule.pattern.bVid);
                printf("\n\tp.bVid_Exclude                                     = %u", pce_rule.pattern.bVid_Exclude);
                printf("\n\tp.nVid                                             = %u", pce_rule.pattern.nVid);

                if (pce_rule.pattern.bVidRange_Select)
                    printf("\n\tp.bVidRange_Select                                 = %u (Range Key)", pce_rule.pattern.bVidRange_Select);
                else
                    printf("\n\tp.bVidRange_Select                                 = %u (Mask Key)", pce_rule.pattern.bVidRange_Select);

                printf("\n\tp.nVidRange                                        = %u", pce_rule.pattern.nVidRange);
                printf("\n\tp.bVid_Original                                    = %u", pce_rule.pattern.bVid_Original);
            }

            if (pce_rule.pattern.bSLAN_Vid)
            {
                printf("\n\tp.bSLAN_Vid                                        = %u", pce_rule.pattern.bSLAN_Vid);
                printf("\n\tp.bSLANVid_Exclude                                 = %u", pce_rule.pattern.bSLANVid_Exclude);
                printf("\n\tp.nSLAN_Vid                                        = %u", pce_rule.pattern.nSLAN_Vid);

                if (pce_rule.pattern.bSVidRange_Select)
                    printf("\n\tp.bSVidRange_Select                                = %u (Range Key)", pce_rule.pattern.bSVidRange_Select);
                else
                    printf("\n\tp.bSVidRange_Select                                = %u (Mask Key)", pce_rule.pattern.bSVidRange_Select);

                printf("\n\tp.nOuterVidRange                                   = %u", pce_rule.pattern.nOuterVidRange);
                printf("\n\tp.bOuterVid_Original                               = %u", pce_rule.pattern.bOuterVid_Original);
            }

            if (pce_rule.pattern.bPortIdEnable)
            {
                printf("\n\tp.bPortIdEnable                                    = %u", pce_rule.pattern.bPortIdEnable);
                printf("\n\tp.bPortId_Exclude                                  = %u", pce_rule.pattern.bPortId_Exclude);
                printf("\n\tp.nPortId                                          = %u", pce_rule.pattern.nPortId);
            }

            if (pce_rule.pattern.bSubIfIdEnable)
            {
                printf("\n\tp.bSubIfIdEnable                                   = %u", pce_rule.pattern.bSubIfIdEnable);
                printf("\n\tp.bSubIfId_Exclude                                 = %u", pce_rule.pattern.bSubIfId_Exclude);
                printf("\n\tp.eSubIfIdType                                     = %u", pce_rule.pattern.eSubIfIdType);
                printf("\n\tp.nSubIfId                                         = %u", pce_rule.pattern.nSubIfId);
            }

            if (pce_rule.pattern.bPktLngEnable)
            {
                printf("\n\tp.bPktLngEnable                                    = %u", pce_rule.pattern.bPktLngEnable);
                printf("\n\tp.bPktLng_Exclude                                  = %u", pce_rule.pattern.bPktLng_Exclude);
                printf("\n\tp.nPktLng                                          = %u", pce_rule.pattern.nPktLng);
                printf("\n\tp.nPktLngRange                                     = %u", pce_rule.pattern.nPktLngRange);
            }

            if (pce_rule.pattern.bPayload1_SrcEnable)
            {
                printf("\n\tp.bPayload1_Exclude                                = %u", pce_rule.pattern.bPayload1_Exclude);
                printf("\n\tp.nPayload1                                        = 0x%x", pce_rule.pattern.nPayload1);
                printf("\n\tp.bPayload1MaskRange_Select                        = %u", pce_rule.pattern.bPayload1MaskRange_Select);
                printf("\n\tp.nPayload1_Mask                                   = 0x%x", pce_rule.pattern.nPayload1_Mask);
            }

            if (pce_rule.pattern.bPayload2_SrcEnable)
            {
                printf("\n\tp.bPayload2_Exclude                                = %u", pce_rule.pattern.bPayload2_Exclude);
                printf("\n\tp.nPayload2                                        = 0x%x", pce_rule.pattern.nPayload2);
                printf("\n\tp.bPayload2MaskRange_Select                        = %u", pce_rule.pattern.bPayload2MaskRange_Select);
                printf("\n\tp.nPayload2_Mask                                   = 0x%x", pce_rule.pattern.nPayload2_Mask);
            }

            if (pce_rule.pattern.bParserFlagLSB_Enable)
            {
                printf("\n\tp.bParserFlagLSB_Exclude                           = %u", pce_rule.pattern.bParserFlagLSB_Exclude);
                printf("\n\tp.nParserFlagLSB                                   = 0x%x", pce_rule.pattern.nParserFlagLSB);
                printf("\n\tp.nParserFlagLSB_Mask                              = 0x%x", pce_rule.pattern.nParserFlagLSB_Mask);
            }

            if (pce_rule.pattern.bParserFlagMSB_Enable)
            {
                printf("\n\tp.bParserFlagMSB_Exclude                           = %u", pce_rule.pattern.bParserFlagMSB_Exclude);
                printf("\n\tp.nParserFlagMSB                                   = 0x%x", pce_rule.pattern.nParserFlagMSB);
                printf("\n\tp.nParserFlagMSB_Mask                              = 0x%x", pce_rule.pattern.nParserFlagMSB_Mask);
            }

            if (pce_rule.pattern.bParserFlag1LSB_Enable)
            {
                printf("\n\tp.bParserFlag1LSB_Exclude                          = %u", pce_rule.pattern.bParserFlag1LSB_Exclude);
                printf("\n\tp.nParserFlag1LSB                                  = 0x%x", pce_rule.pattern.nParserFlag1LSB);
                printf("\n\tp.nParserFlag1LSB_Mask                             = 0x%x", pce_rule.pattern.nParserFlag1LSB_Mask);
            }

            if (pce_rule.pattern.bParserFlag1MSB_Enable)
            {
                printf("\n\tp.bParserFlag1MSB_Exclude                          = %u", pce_rule.pattern.bParserFlag1MSB_Exclude);
                printf("\n\tp.nParserFlag1MSB                                  = 0x%x", pce_rule.pattern.nParserFlag1MSB);
                printf("\n\tp.nParserFlag1MSB_Mask                             = 0x%x", pce_rule.pattern.nParserFlag1MSB_Mask);
            }

            if (pce_rule.action.eVLAN_Action)
            {
                printf("\n\ta.eVLAN_Action				= %u", pce_rule.action.eVLAN_Action);
                printf("\n\ta.nVLAN_Id					= %u", pce_rule.action.nVLAN_Id);
            }

            if (pce_rule.action.eSVLAN_Action)
            {
                printf("\n\ta.eSVLAN_Action 			= %u", pce_rule.action.eSVLAN_Action);
                printf("\n\ta.nSVLAN_Id 				= %u", pce_rule.action.nSVLAN_Id);
            }

            if (pce_rule.action.eVLAN_CrossAction)
                printf("\n\ta.eVLAN_CrossAction 		= %u", pce_rule.action.eVLAN_CrossAction);

            if (pce_rule.action.bPortBitMapMuxControl)
                printf("\n\ta.bPortBitMapMuxControl 	= %u", pce_rule.action.bPortBitMapMuxControl);

            if (pce_rule.action.bCVLAN_Ignore_Control)
                printf("\n\ta.bCVLAN_Ignore_Control 	= %u", pce_rule.action.bCVLAN_Ignore_Control);

            if (pce_rule.action.eLearningAction)
                printf("\n\ta.eLearningAction                                  = %u", pce_rule.action.eLearningAction);

            if (pce_rule.action.eSnoopingTypeAction)
                printf("\n\ta.eSnoopingTypeAction                              = %u", pce_rule.action.eSnoopingTypeAction);

            if (pce_rule.pattern.bEtherTypeEnable)
            {
                printf("\n\tp.bEtherType_Exclude                               = 0x%x", pce_rule.pattern.bEtherType_Exclude);
                printf("\n\tp.nEtherType                                       = 0x%x", pce_rule.pattern.nEtherType);
                printf("\n\tp.nEtherTypeMask                                   = 0x%x", pce_rule.pattern.nEtherTypeMask);
            }

            if (pce_rule.pattern.bProtocolEnable)
            {
                printf("\n\tp.bProtocol_Exclude                                = 0x%x", pce_rule.pattern.bProtocol_Exclude);
                printf("\n\tp.nProtocol                                        = 0x%x", pce_rule.pattern.nProtocol);
                printf("\n\tp.nProtocolMask                                    = 0x%x", pce_rule.pattern.nProtocolMask);
            }

            if (pce_rule.pattern.bInnerProtocolEnable)
            {
                printf("\n\tp.bInnerProtocol_Exclude                           = 0x%x", pce_rule.pattern.bInnerProtocol_Exclude);
                printf("\n\tp.nInnerProtocol                                   = 0x%x", pce_rule.pattern.nInnerProtocol);
                printf("\n\tp.nInnerProtocolMask                               = 0x%x", pce_rule.pattern.nInnerProtocolMask);
            }

            if (pce_rule.pattern.bSessionIdEnable)
            {
                printf("\n\tp.bSessionIdEnable                                 = 0x%x", pce_rule.pattern.bSessionIdEnable);
                printf("\n\tp.bSessionId_Exclude                               = 0x%x", pce_rule.pattern.bSessionId_Exclude);
                printf("\n\tp.nSessionId                                       = 0x%x", pce_rule.pattern.nSessionId);
            }

            if (pce_rule.pattern.bPPP_ProtocolEnable)
            {
                printf("\n\tp.bPPP_Protocol_Exclude                            = 0x%x", pce_rule.pattern.bPPP_Protocol_Exclude);
                printf("\n\tp.nPPP_Protocol                                    = 0x%x", pce_rule.pattern.nPPP_Protocol);
                printf("\n\tp.nPPP_ProtocolMask                                = 0x%x", pce_rule.pattern.nPPP_ProtocolMask);
            }

            if (pce_rule.pattern.bAppDataMSB_Enable)
            {
                printf("\n\tp.bAppMSB_Exclude                                  = 0x%x", pce_rule.pattern.bAppMSB_Exclude);
                printf("\n\tp.nAppDataMSB                                      = 0x%x", pce_rule.pattern.nAppDataMSB);
                printf("\n\tp.bAppMaskRangeMSB_Select                          = %u", pce_rule.pattern.bAppMaskRangeMSB_Select);
                printf("\n\tp.nAppMaskRangeMSB                                 = 0x%x", pce_rule.pattern.nAppMaskRangeMSB);
            }

            if (pce_rule.pattern.bAppDataLSB_Enable)
            {
                printf("\n\tp.bAppLSB_Exclude                                  = 0x%x", pce_rule.pattern.bAppLSB_Exclude);
                printf("\n\tp.nAppDataLSB                                      = 0x%x", pce_rule.pattern.nAppDataLSB);
                printf("\n\tp.bAppMaskRangeLSB_Select                          = %u", pce_rule.pattern.bAppMaskRangeLSB_Select);
                printf("\n\tp.nAppMaskRangeLSB                                 = 0x%x", pce_rule.pattern.nAppMaskRangeLSB);
            }

            if (pce_rule.pattern.bDSCP_Enable)
            {
                printf("\n\tp.bDSCP_Exclude                                    = %u", pce_rule.pattern.bDSCP_Exclude);
                printf("\n\tp.nDSCP                                            = %u", pce_rule.pattern.nDSCP);
            }

            if (pce_rule.pattern.bInner_DSCP_Enable)
            {
                printf("\n\tp.bInnerDSCP_Exclude                               = %u", pce_rule.pattern.bInnerDSCP_Exclude);
                printf("\n\tp.nInnerDSCP                                       = %u", pce_rule.pattern.nInnerDSCP);
            }

            if (pce_rule.action.bRemarkAction)
                printf("\n\ta.bRemarkAction                                    = Enabled  val = %u", pce_rule.action.bRemarkAction);

            if (pce_rule.action.bRemarkPCP)
                printf("\n\ta.bRemarkPCP                                       = Disabled val = %u", pce_rule.action.bRemarkPCP);

            if (pce_rule.action.bRemarkDSCP)
                printf("\n\ta.bRemarkDSCP                                      = Disabled val = %u", pce_rule.action.bRemarkDSCP);

            if (pce_rule.action.bRemarkClass)
                printf("\n\ta.bRemarkClass                                     = Disabled val = %u", pce_rule.action.bRemarkClass);

            if (pce_rule.action.bRemarkSTAG_PCP)
                printf("\n\ta.bRemarkSTAG_PCP                                  = Disabled val = %u", pce_rule.action.bRemarkSTAG_PCP);

            if (pce_rule.action.bRemarkSTAG_DEI)
                printf("\n\ta.bRemarkSTAG_DEI                                  = Disabled val = %u", pce_rule.action.bRemarkSTAG_DEI);

            if ((pce_rule.action.bRMON_Action) || (pce_rule.action.bFlowID_Action))
            {
                printf("\n\ta.nFlowID/nRmon_ID                                 = %u", pce_rule.action.nFlowID);
            }

            if (pce_rule.pattern.bPCP_Enable)
            {
                printf("\n\tp.bPCP_Enable                                      = %u", pce_rule.pattern.bPCP_Enable);
                printf("\n\tp.bCTAG_PCP_DEI_Exclude                            = %u", pce_rule.pattern.bCTAG_PCP_DEI_Exclude);
                printf("\n\tp.nPCP                                             = %u", pce_rule.pattern.nPCP);
            }

            if (pce_rule.pattern.bSTAG_PCP_DEI_Enable)
            {
                printf("\n\tp.bSTAG_PCP_DEI_Enable                             = %u", pce_rule.pattern.bSTAG_PCP_DEI_Enable);
                printf("\n\tp.bSTAG_PCP_DEI_Exclude                            = %u", pce_rule.pattern.bSTAG_PCP_DEI_Exclude);
                printf("\n\tp.nSTAG_PCP_DEI                                    = %u", pce_rule.pattern.nSTAG_PCP_DEI);
            }

            if (pce_rule.action.ePortMapAction)
            {
                printf("\n\ta.ePortMapAction                                   = 0x%x", pce_rule.action.ePortMapAction);

                for (i = 0; i < 8; i++)
                {
                    if (pce_rule.action.nForwardPortMap[i])
                        printf("\n\ta.nForwardPortMap[%d]                              = 0x%x", i, pce_rule.action.nForwardPortMap[i]);
                }
            }

            if (pce_rule.action.eTrafficClassAction)
            {
                printf("\n\ta.eTrafficClassAction                              = %u", pce_rule.action.eTrafficClassAction);
                printf("\n\ta.nTrafficClassAlternate                           = %u", pce_rule.action.nTrafficClassAlternate);
            }

            if (pce_rule.action.bPortTrunkAction)
            {
                printf("\n\ta.bPortTrunkAction                                 = Enabled");
                printf("\n\ta.bPortLinkSelection                               = %u", pce_rule.action.bPortLinkSelection);
            }

            if (pce_rule.action.bExtendedVlanEnable)
            {
                printf("\n\ta.bExtendedVlanEnable                              = Enabled");
                printf("\n\ta.nExtendedVlanBlockId                             = %u", pce_rule.action.nExtendedVlanBlockId);
            }

            if (pce_rule.action.ePortFilterType_Action)
            {
                printf("\n\ta.ePortFilterType_Action                           = %u", pce_rule.action.ePortFilterType_Action);

                for (i = 0; i < 8; i++)
                {
                    if (pce_rule.action.nForwardPortMap[i])
                        printf("\n\ta.nForwardPortMap[%d]                              = 0x%x", i, pce_rule.action.nForwardPortMap[i]);
                }
            }

            if (pce_rule.action.eProcessPath_Action)
                printf("\n\ta.eProcessPath_Action                              = %u", pce_rule.action.eProcessPath_Action);

            if (pce_rule.action.bOamEnable)
                printf("\n\ta.bOamEnable                                       = %u", pce_rule.action.bOamEnable);

            if (pce_rule.action.bExtractEnable)
                printf("\n\ta.bExtractEnable                                   = %u", pce_rule.action.bExtractEnable);

            if (pce_rule.action.bOamEnable || pce_rule.action.bExtractEnable)
                printf("\n\ta.nRecordId                                        = %u", pce_rule.action.nRecordId);

            if (pce_rule.action.eColorFrameAction != GSW_PCE_ACTION_COLOR_FRAME_DISABLE)
                printf("\n\ta.eColorFrameAction                                = %u", pce_rule.action.eColorFrameAction);

            if (pce_rule.action.eMeterAction)
            {
                printf("\n\ta.eMeterAction                                     = %u", pce_rule.action.eMeterAction);
                printf("\n\ta.nMeterId                                         = %u", pce_rule.action.nMeterId);
            }

            if (pce_rule.action.bFidEnable)
                printf("\n\ta.nFId                                             = %u", pce_rule.action.nFId);

            if (pce_rule.pattern.bInsertionFlag_Enable)
                printf("\n\tp.nInsertionFlag                                   = %u", pce_rule.pattern.nInsertionFlag);

            if (pce_rule.action.eCrossStateAction == GSW_PCE_ACTION_CROSS_STATE_CROSS)
                printf("\n\tp.eCrossStateAction                                = GSW_PCE_ACTION_CROSS_STATE_CROSS");
            else if (pce_rule.action.eCrossStateAction == GSW_PCE_ACTION_CROSS_STATE_REGULAR)
                printf("\n\tp.eCrossStateAction                                = GSW_PCE_ACTION_CROSS_STATE_REGULAR");
            else
                printf("\n\tp.eCrossStateAction                                = GSW_PCE_ACTION_CROSS_STATE_DISABLE");

            /*Applicable only for GSWIP 3.2*/
            if (pce_rule.pattern.bFlexibleField4Enable)
            {
                printf("\n\tp.bFlexibleField4_ExcludeEnable                    = %u", pce_rule.pattern.bFlexibleField4_ExcludeEnable);
                printf("\n\tp.bFlexibleField4_RangeEnable                      = %u", pce_rule.pattern.bFlexibleField4_RangeEnable);
                printf("\n\tp.nFlexibleField4_ParserIndex                      = %u", pce_rule.pattern.nFlexibleField4_ParserIndex);
                printf("\n\tp.nFlexibleField4_Value                            = %u", pce_rule.pattern.nFlexibleField4_Value);
                printf("\n\tp.nFlexibleField4_MaskOrRange                      = %u", pce_rule.pattern.nFlexibleField4_MaskOrRange);
            }

            if (pce_rule.pattern.bFlexibleField3Enable)
            {
                printf("\n\tp.bFlexibleField3_ExcludeEnable                    = %u", pce_rule.pattern.bFlexibleField3_ExcludeEnable);
                printf("\n\tp.bFlexibleField3_RangeEnable                      = %u", pce_rule.pattern.bFlexibleField3_RangeEnable);
                printf("\n\tp.nFlexibleField3_ParserIndex                      = %u", pce_rule.pattern.nFlexibleField3_ParserIndex);
                printf("\n\tp.nFlexibleField3_Value                            = %u", pce_rule.pattern.nFlexibleField3_Value);
                printf("\n\tp.nFlexibleField3_MaskOrRange                      = %u", pce_rule.pattern.nFlexibleField3_MaskOrRange);
            }

            if (pce_rule.pattern.bFlexibleField2Enable)
            {
                printf("\n\tp.bFlexibleField2_ExcludeEnable                    = %u", pce_rule.pattern.bFlexibleField2_ExcludeEnable);
                printf("\n\tp.bFlexibleField2_RangeEnable                      = %u", pce_rule.pattern.bFlexibleField2_RangeEnable);
                printf("\n\tp.nFlexibleField2_ParserIndex                      = %u", pce_rule.pattern.nFlexibleField2_ParserIndex);
                printf("\n\tp.nFlexibleField2_Value                            = %u", pce_rule.pattern.nFlexibleField2_Value);
                printf("\n\tp.nFlexibleField2_MaskOrRange                      = %u", pce_rule.pattern.nFlexibleField2_MaskOrRange);
            }

            if (pce_rule.pattern.bFlexibleField1Enable)
            {
                printf("\n\tp.bFlexibleField1_ExcludeEnable                    = %u", pce_rule.pattern.bFlexibleField1_ExcludeEnable);
                printf("\n\tp.bFlexibleField1_RangeEnable                      = %u", pce_rule.pattern.bFlexibleField1_RangeEnable);
                printf("\n\tp.nFlexibleField1_ParserIndex                      = %u", pce_rule.pattern.nFlexibleField1_ParserIndex);
                printf("\n\tp.nFlexibleField1_Value                            = %u", pce_rule.pattern.nFlexibleField1_Value);
                printf("\n\tp.nFlexibleField1_MaskOrRange                      = %u", pce_rule.pattern.nFlexibleField1_MaskOrRange);
            }

            if (pce_rule.action.sPBB_Action.bIheaderActionEnable)
            {
                printf("\n\ta.sPBB_Action.bIheaderActionEnable                 = %u", pce_rule.action.sPBB_Action.bIheaderActionEnable);

                switch (pce_rule.action.sPBB_Action.eIheaderOpMode)
                {
                case GSW_PCE_I_HEADER_OPERATION_INSERT:
                    printf("\n\ta.sPBB_Action.eIheaderOpMode                       = GSW_PCE_I_HEADER_OPERATION_INSERT");
                    printf("\n\ta.sPBB_Action.nTunnelIdKnownTraffic                = %u", pce_rule.action.sPBB_Action.nTunnelIdKnownTraffic);
                    printf("\n\ta.sPBB_Action.nTunnelIdUnKnownTraffic              = %u", pce_rule.action.sPBB_Action.nTunnelIdUnKnownTraffic);
                    printf("\n\ta.sPBB_Action.bB_DstMac_FromMacTableEnable	       = %u", pce_rule.action.sPBB_Action.bB_DstMac_FromMacTableEnable);
                    break;

                case GSW_PCE_I_HEADER_OPERATION_REPLACE:
                    printf("\n\ta.sPBB_Action.eIheaderOpMode                       = GSW_PCE_I_HEADER_OPERATION_REPLACE");
                    printf("\n\ta.sPBB_Action.nTunnelIdKnownTraffic                = %u", pce_rule.action.sPBB_Action.nTunnelIdKnownTraffic);
                    printf("\n\ta.sPBB_Action.nTunnelIdUnKnownTraffic              = %u", pce_rule.action.sPBB_Action.nTunnelIdUnKnownTraffic);
                    printf("\n\ta.sPBB_Action.bReplace_B_SrcMacEnable              = %u", pce_rule.action.sPBB_Action.bReplace_B_SrcMacEnable);
                    printf("\n\ta.sPBB_Action.bReplace_B_DstMacEnable              = %u", pce_rule.action.sPBB_Action.bReplace_B_DstMacEnable);
                    printf("\n\ta.sPBB_Action.bReplace_I_TAG_ResEnable             = %u", pce_rule.action.sPBB_Action.bReplace_I_TAG_ResEnable);
                    printf("\n\ta.sPBB_Action.bReplace_I_TAG_UacEnable             = %u", pce_rule.action.sPBB_Action.bReplace_I_TAG_UacEnable);
                    printf("\n\ta.sPBB_Action.bReplace_I_TAG_DeiEnable             = %u", pce_rule.action.sPBB_Action.bReplace_I_TAG_DeiEnable);
                    printf("\n\ta.sPBB_Action.bReplace_I_TAG_PcpEnable             = %u", pce_rule.action.sPBB_Action.bReplace_I_TAG_PcpEnable);
                    printf("\n\ta.sPBB_Action.bReplace_I_TAG_SidEnable             = %u", pce_rule.action.sPBB_Action.bReplace_I_TAG_SidEnable);
                    printf("\n\ta.sPBB_Action.bReplace_I_TAG_TpidEnable            = %u", pce_rule.action.sPBB_Action.bReplace_I_TAG_TpidEnable);
                    break;

                case GSW_PCE_I_HEADER_OPERATION_REMOVE:
                    printf("\n\ta.sPBB_Action.eIheaderOpMode                       = GSW_PCE_I_HEADER_OPERATION_REMOVE");
                    break;

                case GSW_PCE_I_HEADER_OPERATION_NOCHANGE:
                    printf("\n\ta.sPBB_Action.eIheaderOpMode                       = GSW_PCE_I_HEADER_OPERATION_NOCHANGE");
                    break;

                default:
                    break;
                }
            }

            /*Applicable only for GSWIP 3.2*/
            if (pce_rule.action.sPBB_Action.bBtagActionEnable)
            {
                printf("\n\ta.sPBB_Action.bBtagActionEnable                    = %u", pce_rule.action.sPBB_Action.bBtagActionEnable);

                switch (pce_rule.action.sPBB_Action.eBtagOpMode)
                {
                case GSW_PCE_B_TAG_OPERATION_INSERT:
                    printf("\n\ta.sPBB_Action.eBtagOpMode                          = GSW_PCE_B_TAG_OPERATION_INSERT");
                    printf("\n\ta.sPBB_Action.nProcessIdKnownTraffic               = %u", pce_rule.action.sPBB_Action.nProcessIdKnownTraffic);
                    printf("\n\ta.sPBB_Action.nProcessIdUnKnownTraffic             = %u", pce_rule.action.sPBB_Action.nProcessIdUnKnownTraffic);
                    break;

                case GSW_PCE_B_TAG_OPERATION_REPLACE:
                    printf("\n\ta.sPBB_Action.eBtagOpMode                          = GSW_PCE_B_TAG_OPERATION_REPLACE");
                    printf("\n\ta.sPBB_Action.nProcessIdKnownTraffic               = %u", pce_rule.action.sPBB_Action.nProcessIdKnownTraffic);
                    printf("\n\ta.sPBB_Action.nProcessIdUnKnownTraffic             = %u", pce_rule.action.sPBB_Action.nProcessIdUnKnownTraffic);
                    printf("\n\ta.sPBB_Action.bReplace_B_TAG_DeiEnable             = %u", pce_rule.action.sPBB_Action.bReplace_B_TAG_DeiEnable);
                    printf("\n\ta.sPBB_Action.bReplace_B_TAG_PcpEnable             = %u", pce_rule.action.sPBB_Action.bReplace_B_TAG_PcpEnable);
                    printf("\n\ta.sPBB_Action.bReplace_B_TAG_VidEnable             = %u", pce_rule.action.sPBB_Action.bReplace_B_TAG_VidEnable);
                    printf("\n\ta.sPBB_Action.bReplace_B_TAG_TpidEnable            = %u", pce_rule.action.sPBB_Action.bReplace_B_TAG_TpidEnable);
                    break;

                case GSW_PCE_B_TAG_OPERATION_REMOVE:
                    printf("\n\ta.sPBB_Action.eBtagOpMode                          = GSW_PCE_B_TAG_OPERATION_REMOVE");
                    break;

                case GSW_PCE_B_TAG_OPERATION_NOCHANGE:
                    printf("\n\ta.sPBB_Action.eBtagOpMode                          = GSW_PCE_B_TAG_OPERATION_NOCHANGE");
                    break;

                default:
                    break;
                }
            }

            /*Applicable only for GSWIP 3.2*/
            if (pce_rule.action.sPBB_Action.bMacTableMacinMacActionEnable)
            {
                printf("\n\ta.sPBB_Action.bMacTableMacinMacActionEnable                = %u", pce_rule.action.sPBB_Action.bMacTableMacinMacActionEnable);

                switch (pce_rule.action.sPBB_Action.eMacTableMacinMacSelect)
                {
                case GSW_PCE_OUTER_MAC_SELECTED:
                    printf("\n\ta.sPBB_Action.eMacTableMacinMacSelect              = GSW_PCE_OUTER_MAC_SELECTED");
                    break;

                case GSW_PCE_INNER_MAC_SELECTED:
                    printf("\n\ta.sPBB_Action.eMacTableMacinMacSelect              = GSW_PCE_INNER_MAC_SELECTED");
                    break;

                default:
                    break;
                }
            }

            if (pce_rule.action.bDestSubIf_Action_Enable)
            {
                printf("\n\ta.sDestSubIF_Action.bDestSubIFIDActionEnable       = %u", pce_rule.action.sDestSubIF_Action.bDestSubIFIDActionEnable);
                printf("\n\ta.sDestSubIF_Action.bDestSubIFIDAssignmentEnable   = %u", pce_rule.action.sDestSubIF_Action.bDestSubIFIDAssignmentEnable);
                printf("\n\ta.sDestSubIF_Action.nDestSubIFGrp_Field            = %u", pce_rule.action.sDestSubIF_Action.nDestSubIFGrp_Field);
            }
        }
        else
        {
            printf("\n\tp.nIndex rule not set at                           = %u", pce_rule.pattern.nIndex);
        }
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleWrite(int prmc, char *prmv[])
{
    GSW_PCE_rule_t pce_rule = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    memset(&pce_rule, 0, sizeof(GSW_PCE_rule_t));

    rret = scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(pce_rule.logicalportid), &pce_rule.logicalportid);
    if (rret < 1)
    {
        printf("Parameter not Found: nLogicalPortId\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "pattern.nIndex", sizeof(pce_rule.pattern.nIndex), &pce_rule.pattern.nIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: pattern.nIndex\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(pce_rule.subifidgroup), &pce_rule.subifidgroup);
    scanParamArg(prmc, prmv, "region", sizeof(pce_rule.region), &pce_rule.region);

    scanParamArg(prmc, prmv, "pattern.bEnable", sizeof(pce_rule.pattern.bEnable), &pce_rule.pattern.bEnable);
    scanParamArg(prmc, prmv, "pattern.bPortIdEnable", sizeof(pce_rule.pattern.bPortIdEnable), &pce_rule.pattern.bPortIdEnable);
    scanParamArg(prmc, prmv, "pattern.nPortId", sizeof(pce_rule.pattern.nPortId), &pce_rule.pattern.nPortId);
    scanParamArg(prmc, prmv, "pattern.bPortId_Exclude", sizeof(pce_rule.pattern.bPortId_Exclude), &pce_rule.pattern.bPortId_Exclude);
    scanParamArg(prmc, prmv, "pattern.bSubIfIdEnable", sizeof(pce_rule.pattern.bSubIfIdEnable), &pce_rule.pattern.bSubIfIdEnable);
    scanParamArg(prmc, prmv, "pattern.nSubIfId", sizeof(pce_rule.pattern.nSubIfId), &pce_rule.pattern.nSubIfId);
    scanParamArg(prmc, prmv, "pattern.eSubIfIdType", sizeof(pce_rule.pattern.eSubIfIdType), &pce_rule.pattern.eSubIfIdType);
    scanParamArg(prmc, prmv, "pattern.bSubIfId_Exclude", sizeof(pce_rule.pattern.bSubIfId_Exclude), &pce_rule.pattern.bSubIfId_Exclude);
    scanParamArg(prmc, prmv, "pattern.bInsertionFlag_Enable", sizeof(pce_rule.pattern.bInsertionFlag_Enable), &pce_rule.pattern.bInsertionFlag_Enable);
    scanParamArg(prmc, prmv, "pattern.nInsertionFlag", sizeof(pce_rule.pattern.nInsertionFlag), &pce_rule.pattern.nInsertionFlag);
    scanParamArg(prmc, prmv, "pattern.bDSCP_Enable", sizeof(pce_rule.pattern.bDSCP_Enable), &pce_rule.pattern.bDSCP_Enable);
    scanParamArg(prmc, prmv, "pattern.nDSCP", sizeof(pce_rule.pattern.nDSCP), &pce_rule.pattern.nDSCP);
    scanParamArg(prmc, prmv, "pattern.bDSCP_Exclude", sizeof(pce_rule.pattern.bDSCP_Exclude), &pce_rule.pattern.bDSCP_Exclude);
    scanParamArg(prmc, prmv, "pattern.bInner_DSCP_Enable", sizeof(pce_rule.pattern.bInner_DSCP_Enable), &pce_rule.pattern.bInner_DSCP_Enable);
    scanParamArg(prmc, prmv, "pattern.nInnerDSCP", sizeof(pce_rule.pattern.nInnerDSCP), &pce_rule.pattern.nInnerDSCP);
    scanParamArg(prmc, prmv, "pattern.bInnerDSCP_Exclude", sizeof(pce_rule.pattern.bInnerDSCP_Exclude), &pce_rule.pattern.bInnerDSCP_Exclude);
    scanParamArg(prmc, prmv, "pattern.bPCP_Enable", sizeof(pce_rule.pattern.bPCP_Enable), &pce_rule.pattern.bPCP_Enable);
    scanParamArg(prmc, prmv, "pattern.nPCP", sizeof(pce_rule.pattern.nPCP), &pce_rule.pattern.nPCP);
    scanParamArg(prmc, prmv, "pattern.bCTAG_PCP_DEI_Exclude", sizeof(pce_rule.pattern.bCTAG_PCP_DEI_Exclude), &pce_rule.pattern.bCTAG_PCP_DEI_Exclude);
    scanParamArg(prmc, prmv, "pattern.bSTAG_PCP_DEI_Enable", sizeof(pce_rule.pattern.bSTAG_PCP_DEI_Enable), &pce_rule.pattern.bSTAG_PCP_DEI_Enable);
    scanParamArg(prmc, prmv, "pattern.nSTAG_PCP_DEI", sizeof(pce_rule.pattern.nSTAG_PCP_DEI), &pce_rule.pattern.nSTAG_PCP_DEI);
    scanParamArg(prmc, prmv, "pattern.bSTAG_PCP_DEI_Exclude", sizeof(pce_rule.pattern.bSTAG_PCP_DEI_Exclude), &pce_rule.pattern.bSTAG_PCP_DEI_Exclude);
    scanParamArg(prmc, prmv, "pattern.bPktLngEnable", sizeof(pce_rule.pattern.bPktLngEnable), &pce_rule.pattern.bPktLngEnable);
    scanParamArg(prmc, prmv, "pattern.nPktLng", sizeof(pce_rule.pattern.nPktLng), &pce_rule.pattern.nPktLng);
    scanParamArg(prmc, prmv, "pattern.nPktLngRange", sizeof(pce_rule.pattern.nPktLngRange), &pce_rule.pattern.nPktLngRange);
    scanParamArg(prmc, prmv, "pattern.bPktLng_Exclude", sizeof(pce_rule.pattern.bPktLng_Exclude), &pce_rule.pattern.bPktLng_Exclude);
    scanParamArg(prmc, prmv, "pattern.bMAC_DstEnable", sizeof(pce_rule.pattern.bMAC_DstEnable), &pce_rule.pattern.bMAC_DstEnable);
    scanMAC_Arg(prmc, prmv, "pattern.nMAC_Dst", pce_rule.pattern.nMAC_Dst);
    scanParamArg(prmc, prmv, "pattern.nMAC_DstMask", sizeof(pce_rule.pattern.nMAC_DstMask), &pce_rule.pattern.nMAC_DstMask);
    scanParamArg(prmc, prmv, "pattern.bDstMAC_Exclude", sizeof(pce_rule.pattern.bDstMAC_Exclude), &pce_rule.pattern.bDstMAC_Exclude);
    scanParamArg(prmc, prmv, "pattern.bMAC_SrcEnable", sizeof(pce_rule.pattern.bMAC_SrcEnable), &pce_rule.pattern.bMAC_SrcEnable);
    scanMAC_Arg(prmc, prmv, "pattern.nMAC_Src", pce_rule.pattern.nMAC_Src);
    scanParamArg(prmc, prmv, "pattern.nMAC_SrcMask", sizeof(pce_rule.pattern.nMAC_SrcMask), &pce_rule.pattern.nMAC_SrcMask);
    scanParamArg(prmc, prmv, "pattern.bSrcMAC_Exclude", sizeof(pce_rule.pattern.bSrcMAC_Exclude), &pce_rule.pattern.bSrcMAC_Exclude);
    scanParamArg(prmc, prmv, "pattern.bAppDataMSB_Enable", sizeof(pce_rule.pattern.bAppDataMSB_Enable), &pce_rule.pattern.bAppDataMSB_Enable);
    scanParamArg(prmc, prmv, "pattern.nAppDataMSB", sizeof(pce_rule.pattern.nAppDataMSB), &pce_rule.pattern.nAppDataMSB);
    scanParamArg(prmc, prmv, "pattern.bAppMaskRangeMSB_Select", sizeof(pce_rule.pattern.bAppMaskRangeMSB_Select), &pce_rule.pattern.bAppMaskRangeMSB_Select);
    scanParamArg(prmc, prmv, "pattern.nAppMaskRangeMSB", sizeof(pce_rule.pattern.nAppMaskRangeMSB), &pce_rule.pattern.nAppMaskRangeMSB);
    scanParamArg(prmc, prmv, "pattern.bAppMSB_Exclude", sizeof(pce_rule.pattern.bAppMSB_Exclude), &pce_rule.pattern.bAppMSB_Exclude);

    scanParamArg(prmc, prmv, "pattern.bAppDataLSB_Enable", sizeof(pce_rule.pattern.bAppDataLSB_Enable), &pce_rule.pattern.bAppDataLSB_Enable);
    scanParamArg(prmc, prmv, "pattern.nAppDataLSB", sizeof(pce_rule.pattern.nAppDataLSB), &pce_rule.pattern.nAppDataLSB);
    scanParamArg(prmc, prmv, "pattern.bAppMaskRangeLSB_Select", sizeof(pce_rule.pattern.bAppMaskRangeLSB_Select), &pce_rule.pattern.bAppMaskRangeLSB_Select);
    scanParamArg(prmc, prmv, "pattern.nAppMaskRangeLSB", sizeof(pce_rule.pattern.nAppMaskRangeLSB), &pce_rule.pattern.nAppMaskRangeLSB);
    scanParamArg(prmc, prmv, "pattern.bAppLSB_Exclude", sizeof(pce_rule.pattern.bAppLSB_Exclude), &pce_rule.pattern.bAppLSB_Exclude);

    scanParamArg(prmc, prmv, "pattern.eDstIP_Select", sizeof(pce_rule.pattern.eDstIP_Select), &pce_rule.pattern.eDstIP_Select);
    if (pce_rule.pattern.eDstIP_Select == GSW_PCE_IP_V4)
        scanIPv4_Arg(prmc, prmv, "pattern.nDstIP", &pce_rule.pattern.nDstIP.nIPv4);
    else if (pce_rule.pattern.eDstIP_Select == GSW_PCE_IP_V6)
        scanIPv6_Arg(prmc, prmv, "pattern.nDstIP", pce_rule.pattern.nDstIP.nIPv6);

    scanParamArg(prmc, prmv, "pattern.nDstIP_Mask", sizeof(pce_rule.pattern.nDstIP_Mask), &pce_rule.pattern.nDstIP_Mask);
    scanParamArg(prmc, prmv, "pattern.bDstIP_Exclude", sizeof(pce_rule.pattern.bDstIP_Exclude), &pce_rule.pattern.bDstIP_Exclude);

    scanParamArg(prmc, prmv, "pattern.eInnerDstIP_Select", sizeof(pce_rule.pattern.eInnerDstIP_Select), &pce_rule.pattern.eInnerDstIP_Select);
    if (pce_rule.pattern.eInnerDstIP_Select == GSW_PCE_IP_V4)
        scanIPv4_Arg(prmc, prmv, "pattern.nInnerDstIP", &pce_rule.pattern.nInnerDstIP.nIPv4);
    else if (pce_rule.pattern.eInnerDstIP_Select == GSW_PCE_IP_V6)
        scanIPv6_Arg(prmc, prmv, "pattern.nInnerDstIP", pce_rule.pattern.nInnerDstIP.nIPv6);

    scanParamArg(prmc, prmv, "pattern.nInnerDstIP_Mask", sizeof(pce_rule.pattern.nInnerDstIP_Mask), &pce_rule.pattern.nInnerDstIP_Mask);
    scanParamArg(prmc, prmv, "pattern.bInnerDstIP_Exclude", sizeof(pce_rule.pattern.bInnerDstIP_Exclude), &pce_rule.pattern.bInnerDstIP_Exclude);

    scanParamArg(prmc, prmv, "pattern.eSrcIP_Select", sizeof(pce_rule.pattern.eSrcIP_Select), &pce_rule.pattern.eSrcIP_Select);
    if (pce_rule.pattern.eSrcIP_Select == GSW_PCE_IP_V4)
        scanIPv4_Arg(prmc, prmv, "pattern.nSrcIP", &pce_rule.pattern.nSrcIP.nIPv4);
    else if (pce_rule.pattern.eSrcIP_Select == GSW_PCE_IP_V6)
        scanIPv6_Arg(prmc, prmv, "pattern.nSrcIP", pce_rule.pattern.nSrcIP.nIPv6);

    scanParamArg(prmc, prmv, "pattern.nSrcIP_Mask", sizeof(pce_rule.pattern.nSrcIP_Mask), &pce_rule.pattern.nSrcIP_Mask);
    scanParamArg(prmc, prmv, "pattern.bSrcIP_Exclude", sizeof(pce_rule.pattern.bSrcIP_Exclude), &pce_rule.pattern.bSrcIP_Exclude);

    scanParamArg(prmc, prmv, "pattern.eInnerSrcIP_Select", sizeof(pce_rule.pattern.eInnerSrcIP_Select), &pce_rule.pattern.eInnerSrcIP_Select);
    if (pce_rule.pattern.eInnerSrcIP_Select == GSW_PCE_IP_V4)
        scanIPv4_Arg(prmc, prmv, "pattern.nInnerSrcIP", &pce_rule.pattern.nInnerSrcIP.nIPv4);
    else if (pce_rule.pattern.eInnerSrcIP_Select == GSW_PCE_IP_V6)
        scanIPv6_Arg(prmc, prmv, "pattern.nInnerSrcIP", pce_rule.pattern.nInnerSrcIP.nIPv6);

    scanParamArg(prmc, prmv, "pattern.nInnerSrcIP_Mask", sizeof(pce_rule.pattern.nInnerSrcIP_Mask), &pce_rule.pattern.nInnerSrcIP_Mask);
    scanParamArg(prmc, prmv, "pattern.bInnerSrcIP_Exclude", sizeof(pce_rule.pattern.bInnerSrcIP_Exclude), &pce_rule.pattern.bInnerSrcIP_Exclude);

    scanParamArg(prmc, prmv, "pattern.bEtherTypeEnable", sizeof(pce_rule.pattern.bEtherTypeEnable), &pce_rule.pattern.bEtherTypeEnable);
    scanParamArg(prmc, prmv, "pattern.nEtherType", sizeof(pce_rule.pattern.nEtherType), &pce_rule.pattern.nEtherType);
    scanParamArg(prmc, prmv, "pattern.nEtherTypeMask", sizeof(pce_rule.pattern.nEtherTypeMask), &pce_rule.pattern.nEtherTypeMask);
    scanParamArg(prmc, prmv, "pattern.bEtherType_Exclude", sizeof(pce_rule.pattern.bEtherType_Exclude), &pce_rule.pattern.bEtherType_Exclude);

    scanParamArg(prmc, prmv, "pattern.bProtocolEnable", sizeof(pce_rule.pattern.bProtocolEnable), &pce_rule.pattern.bProtocolEnable);
    scanParamArg(prmc, prmv, "pattern.nProtocol", sizeof(pce_rule.pattern.nProtocol), &pce_rule.pattern.nProtocol);
    scanParamArg(prmc, prmv, "pattern.nProtocolMask", sizeof(pce_rule.pattern.nProtocolMask), &pce_rule.pattern.nProtocolMask);
    scanParamArg(prmc, prmv, "pattern.bProtocol_Exclude", sizeof(pce_rule.pattern.bProtocol_Exclude), &pce_rule.pattern.bProtocol_Exclude);

    scanParamArg(prmc, prmv, "pattern.bInnerProtocolEnable", sizeof(pce_rule.pattern.bInnerProtocolEnable), &pce_rule.pattern.bInnerProtocolEnable);
    scanParamArg(prmc, prmv, "pattern.nInnerProtocol", sizeof(pce_rule.pattern.nInnerProtocol), &pce_rule.pattern.nInnerProtocol);
    scanParamArg(prmc, prmv, "pattern.nInnerProtocolMask", sizeof(pce_rule.pattern.nInnerProtocolMask), &pce_rule.pattern.nInnerProtocolMask);
    scanParamArg(prmc, prmv, "pattern.bInnerProtocol_Exclude", sizeof(pce_rule.pattern.bInnerProtocol_Exclude), &pce_rule.pattern.bInnerProtocol_Exclude);

    scanParamArg(prmc, prmv, "pattern.bSessionIdEnable", sizeof(pce_rule.pattern.bSessionIdEnable), &pce_rule.pattern.bSessionIdEnable);
    scanParamArg(prmc, prmv, "pattern.nSessionId", sizeof(pce_rule.pattern.nSessionId), &pce_rule.pattern.nSessionId);
    scanParamArg(prmc, prmv, "pattern.bSessionId_Exclude", sizeof(pce_rule.pattern.bSessionId_Exclude), &pce_rule.pattern.bSessionId_Exclude);

    scanParamArg(prmc, prmv, "pattern.bPPP_ProtocolEnable", sizeof(pce_rule.pattern.bPPP_ProtocolEnable), &pce_rule.pattern.bPPP_ProtocolEnable);
    scanParamArg(prmc, prmv, "pattern.nPPP_Protocol", sizeof(pce_rule.pattern.nPPP_Protocol), &pce_rule.pattern.nPPP_Protocol);
    scanParamArg(prmc, prmv, "pattern.nPPP_ProtocolMask", sizeof(pce_rule.pattern.nPPP_ProtocolMask), &pce_rule.pattern.nPPP_ProtocolMask);
    scanParamArg(prmc, prmv, "pattern.bPPP_Protocol_Exclude", sizeof(pce_rule.pattern.bPPP_Protocol_Exclude), &pce_rule.pattern.bPPP_Protocol_Exclude);

    scanParamArg(prmc, prmv, "pattern.bVid", sizeof(pce_rule.pattern.bVid), &pce_rule.pattern.bVid);
    scanParamArg(prmc, prmv, "pattern.nVid", sizeof(pce_rule.pattern.nVid), &pce_rule.pattern.nVid);
    scanParamArg(prmc, prmv, "pattern.bVidRange_Select", sizeof(pce_rule.pattern.bVidRange_Select), &pce_rule.pattern.bVidRange_Select);
    scanParamArg(prmc, prmv, "pattern.nVidRange", sizeof(pce_rule.pattern.nVidRange), &pce_rule.pattern.nVidRange);
    scanParamArg(prmc, prmv, "pattern.bVid_Exclude", sizeof(pce_rule.pattern.bVid_Exclude), &pce_rule.pattern.bVid_Exclude);

    scanParamArg(prmc, prmv, "pattern.bSLAN_Vid", sizeof(pce_rule.pattern.bSLAN_Vid), &pce_rule.pattern.bSLAN_Vid);
    scanParamArg(prmc, prmv, "pattern.nSLAN_Vid", sizeof(pce_rule.pattern.nSLAN_Vid), &pce_rule.pattern.nSLAN_Vid);
    scanParamArg(prmc, prmv, "pattern.bSLANVid_Exclude", sizeof(pce_rule.pattern.bSLANVid_Exclude), &pce_rule.pattern.bSLANVid_Exclude);

    scanParamArg(prmc, prmv, "pattern.bPayload1_SrcEnable", sizeof(pce_rule.pattern.bPayload1_SrcEnable), &pce_rule.pattern.bPayload1_SrcEnable);
    scanParamArg(prmc, prmv, "pattern.nPayload1", sizeof(pce_rule.pattern.nPayload1), &pce_rule.pattern.nPayload1);
    scanParamArg(prmc, prmv, "pattern.bPayload1MaskRange_Select", sizeof(pce_rule.pattern.bPayload1MaskRange_Select), &pce_rule.pattern.bPayload1MaskRange_Select);
    scanParamArg(prmc, prmv, "pattern.nPayload1_Mask", sizeof(pce_rule.pattern.nPayload1_Mask), &pce_rule.pattern.nPayload1_Mask);
    scanParamArg(prmc, prmv, "pattern.bPayload1_Exclude", sizeof(pce_rule.pattern.bPayload1_Exclude), &pce_rule.pattern.bPayload1_Exclude);

    scanParamArg(prmc, prmv, "pattern.bPayload2_SrcEnable", sizeof(pce_rule.pattern.bPayload2_SrcEnable), &pce_rule.pattern.bPayload2_SrcEnable);
    scanParamArg(prmc, prmv, "pattern.nPayload2", sizeof(pce_rule.pattern.nPayload2), &pce_rule.pattern.nPayload2);
    scanParamArg(prmc, prmv, "pattern.bPayload2MaskRange_Select", sizeof(pce_rule.pattern.bPayload2MaskRange_Select), &pce_rule.pattern.bPayload2MaskRange_Select);
    scanParamArg(prmc, prmv, "pattern.nPayload2_Mask", sizeof(pce_rule.pattern.nPayload2_Mask), &pce_rule.pattern.nPayload2_Mask);
    scanParamArg(prmc, prmv, "pattern.bPayload2_Exclude", sizeof(pce_rule.pattern.bPayload2_Exclude), &pce_rule.pattern.bPayload2_Exclude);

    scanParamArg(prmc, prmv, "pattern.bParserFlagLSB_Enable", sizeof(pce_rule.pattern.bParserFlagLSB_Enable), &pce_rule.pattern.bParserFlagLSB_Enable);
    scanParamArg(prmc, prmv, "pattern.nParserFlagLSB", sizeof(pce_rule.pattern.nParserFlagLSB), &pce_rule.pattern.nParserFlagLSB);
    scanParamArg(prmc, prmv, "pattern.nParserFlagLSB_Mask", sizeof(pce_rule.pattern.nParserFlagLSB_Mask), &pce_rule.pattern.nParserFlagLSB_Mask);
    scanParamArg(prmc, prmv, "pattern.bParserFlagLSB_Exclude", sizeof(pce_rule.pattern.bParserFlagLSB_Exclude), &pce_rule.pattern.bParserFlagLSB_Exclude);

    scanParamArg(prmc, prmv, "pattern.bParserFlagMSB_Enable", sizeof(pce_rule.pattern.bParserFlagMSB_Enable), &pce_rule.pattern.bParserFlagMSB_Enable);
    scanParamArg(prmc, prmv, "pattern.nParserFlagMSB", sizeof(pce_rule.pattern.nParserFlagMSB), &pce_rule.pattern.nParserFlagMSB);
    scanParamArg(prmc, prmv, "pattern.nParserFlagMSB_Mask", sizeof(pce_rule.pattern.nParserFlagMSB_Mask), &pce_rule.pattern.nParserFlagMSB_Mask);
    scanParamArg(prmc, prmv, "pattern.bParserFlagMSB_Exclude", sizeof(pce_rule.pattern.bParserFlagMSB_Exclude), &pce_rule.pattern.bParserFlagMSB_Exclude);

    scanParamArg(prmc, prmv, "pattern.bParserFlag1LSB_Enable", sizeof(pce_rule.pattern.bParserFlag1LSB_Enable), &pce_rule.pattern.bParserFlag1LSB_Enable);
    scanParamArg(prmc, prmv, "pattern.nParserFlag1LSB", sizeof(pce_rule.pattern.nParserFlag1LSB), &pce_rule.pattern.nParserFlag1LSB);
    scanParamArg(prmc, prmv, "pattern.nParserFlag1LSB_Mask", sizeof(pce_rule.pattern.nParserFlag1LSB_Mask), &pce_rule.pattern.nParserFlag1LSB_Mask);
    scanParamArg(prmc, prmv, "pattern.bParserFlag1LSB_Exclude", sizeof(pce_rule.pattern.bParserFlag1LSB_Exclude), &pce_rule.pattern.bParserFlag1LSB_Exclude);

    scanParamArg(prmc, prmv, "pattern.bParserFlag1MSB_Enable", sizeof(pce_rule.pattern.bParserFlag1MSB_Enable), &pce_rule.pattern.bParserFlag1MSB_Enable);
    scanParamArg(prmc, prmv, "pattern.nParserFlag1MSB", sizeof(pce_rule.pattern.nParserFlag1MSB), &pce_rule.pattern.nParserFlag1MSB);
    scanParamArg(prmc, prmv, "pattern.nParserFlag1MSB_Mask", sizeof(pce_rule.pattern.nParserFlag1MSB_Mask), &pce_rule.pattern.nParserFlag1MSB_Mask);
    scanParamArg(prmc, prmv, "pattern.bParserFlag1MSB_Exclude", sizeof(pce_rule.pattern.bParserFlag1MSB_Exclude), &pce_rule.pattern.bParserFlag1MSB_Exclude);

    scanParamArg(prmc, prmv, "pattern.bVid_Original", sizeof(pce_rule.pattern.bVid_Original), &pce_rule.pattern.bVid_Original);
    scanParamArg(prmc, prmv, "pattern.nOuterVidRange", sizeof(pce_rule.pattern.nOuterVidRange), &pce_rule.pattern.nOuterVidRange);
    scanParamArg(prmc, prmv, "pattern.bSVidRange_Select", sizeof(pce_rule.pattern.bSVidRange_Select), &pce_rule.pattern.bSVidRange_Select);
    scanParamArg(prmc, prmv, "pattern.bOuterVid_Original", sizeof(pce_rule.pattern.bOuterVid_Original), &pce_rule.pattern.bOuterVid_Original);

    scanParamArg(prmc, prmv, "action.eTrafficClassAction", sizeof(pce_rule.action.eTrafficClassAction), &pce_rule.action.eTrafficClassAction);
    scanParamArg(prmc, prmv, "action.nTrafficClassAlternate", sizeof(pce_rule.action.nTrafficClassAlternate), &pce_rule.action.nTrafficClassAlternate);
    scanParamArg(prmc, prmv, "action.eSnoopingTypeAction", sizeof(pce_rule.action.eSnoopingTypeAction), &pce_rule.action.eSnoopingTypeAction);
    scanParamArg(prmc, prmv, "action.eLearningAction", sizeof(pce_rule.action.eLearningAction), &pce_rule.action.eLearningAction);
    scanParamArg(prmc, prmv, "action.eIrqAction", sizeof(pce_rule.action.eIrqAction), &pce_rule.action.eIrqAction);
    scanParamArg(prmc, prmv, "action.eCrossStateAction", sizeof(pce_rule.action.eCrossStateAction), &pce_rule.action.eCrossStateAction);
    scanParamArg(prmc, prmv, "action.eCritFrameAction", sizeof(pce_rule.action.eCritFrameAction), &pce_rule.action.eCritFrameAction);
    scanParamArg(prmc, prmv, "action.eTimestampAction", sizeof(pce_rule.action.eTimestampAction), &pce_rule.action.eTimestampAction);
    scanParamArg(prmc, prmv, "action.ePortMapAction", sizeof(pce_rule.action.ePortMapAction), &pce_rule.action.ePortMapAction);
    scanParamArg(prmc, prmv, "action.nForwardPortMap", sizeof(pce_rule.action.nForwardPortMap[0]), &pce_rule.action.nForwardPortMap[0]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[1]", sizeof(pce_rule.action.nForwardPortMap[1]), &pce_rule.action.nForwardPortMap[1]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[2]", sizeof(pce_rule.action.nForwardPortMap[2]), &pce_rule.action.nForwardPortMap[2]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[3]", sizeof(pce_rule.action.nForwardPortMap[3]), &pce_rule.action.nForwardPortMap[3]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[4]", sizeof(pce_rule.action.nForwardPortMap[4]), &pce_rule.action.nForwardPortMap[4]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[5]", sizeof(pce_rule.action.nForwardPortMap[5]), &pce_rule.action.nForwardPortMap[5]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[6]", sizeof(pce_rule.action.nForwardPortMap[6]), &pce_rule.action.nForwardPortMap[6]);
    scanParamArg(prmc, prmv, "action.nForwardPortMap[7]", sizeof(pce_rule.action.nForwardPortMap[7]), &pce_rule.action.nForwardPortMap[7]);
    scanParamArg(prmc, prmv, "action.bRemarkAction", sizeof(pce_rule.action.bRemarkAction), &pce_rule.action.bRemarkAction);
    scanParamArg(prmc, prmv, "action.bRemarkPCP", sizeof(pce_rule.action.bRemarkAction), &pce_rule.action.bRemarkPCP);
    scanParamArg(prmc, prmv, "action.bRemarkSTAG_PCP", sizeof(pce_rule.action.bRemarkSTAG_PCP), &pce_rule.action.bRemarkSTAG_PCP);
    scanParamArg(prmc, prmv, "action.bRemarkSTAG_DEI", sizeof(pce_rule.action.bRemarkSTAG_DEI), &pce_rule.action.bRemarkSTAG_DEI);
    scanParamArg(prmc, prmv, "action.bRemarkDSCP", sizeof(pce_rule.action.bRemarkDSCP), &pce_rule.action.bRemarkDSCP);
    scanParamArg(prmc, prmv, "action.bRemarkClass", sizeof(pce_rule.action.bRemarkClass), &pce_rule.action.bRemarkClass);
    scanParamArg(prmc, prmv, "action.eMeterAction", sizeof(pce_rule.action.eMeterAction), &pce_rule.action.eMeterAction);
    scanParamArg(prmc, prmv, "action.nMeterId", sizeof(pce_rule.action.nMeterId), &pce_rule.action.nMeterId);
    scanParamArg(prmc, prmv, "action.bRMON_Action", sizeof(pce_rule.action.bRMON_Action), &pce_rule.action.bRMON_Action);
    scanParamArg(prmc, prmv, "action.nRMON_Id", sizeof(pce_rule.action.nRMON_Id), &pce_rule.action.nRMON_Id);
    scanParamArg(prmc, prmv, "action.eVLAN_Action", sizeof(pce_rule.action.eVLAN_Action), &pce_rule.action.eVLAN_Action);
    scanParamArg(prmc, prmv, "action.nVLAN_Id", sizeof(pce_rule.action.nVLAN_Id), &pce_rule.action.nVLAN_Id);
    scanParamArg(prmc, prmv, "action.nFId", sizeof(pce_rule.action.nFId), &pce_rule.action.nFId);
    scanParamArg(prmc, prmv, "action.bFidEnable", sizeof(pce_rule.action.bFidEnable), &pce_rule.action.bFidEnable);

    scanParamArg(prmc, prmv, "action.eSVLAN_Action", sizeof(pce_rule.action.eSVLAN_Action), &pce_rule.action.eSVLAN_Action);
    scanParamArg(prmc, prmv, "action.nSVLAN_Id", sizeof(pce_rule.action.nSVLAN_Id), &pce_rule.action.nSVLAN_Id);
    scanParamArg(prmc, prmv, "action.eVLAN_CrossAction", sizeof(pce_rule.action.eVLAN_CrossAction), &pce_rule.action.eVLAN_CrossAction);
    scanParamArg(prmc, prmv, "action.bPortBitMapMuxControl", sizeof(pce_rule.action.bPortBitMapMuxControl), &pce_rule.action.bPortBitMapMuxControl);
    scanParamArg(prmc, prmv, "action.bCVLAN_Ignore_Control", sizeof(pce_rule.action.bCVLAN_Ignore_Control), &pce_rule.action.bCVLAN_Ignore_Control);
    scanParamArg(prmc, prmv, "action.bPortLinkSelection", sizeof(pce_rule.action.bPortLinkSelection), &pce_rule.action.bPortLinkSelection);
    scanParamArg(prmc, prmv, "action.bPortTrunkAction", sizeof(pce_rule.action.bPortTrunkAction), &pce_rule.action.bPortTrunkAction);

    scanParamArg(prmc, prmv, "action.bFlowID_Action", sizeof(pce_rule.action.bFlowID_Action), &pce_rule.action.bFlowID_Action);
    scanParamArg(prmc, prmv, "action.nFlowID", sizeof(pce_rule.action.nFlowID), &pce_rule.action.nFlowID);

    scanParamArg(prmc, prmv, "action.bRoutExtId_Action", sizeof(pce_rule.action.bRoutExtId_Action), &pce_rule.action.bRoutExtId_Action);
    scanParamArg(prmc, prmv, "action.nRoutExtId", sizeof(pce_rule.action.nRoutExtId), &pce_rule.action.nRoutExtId);

    scanParamArg(prmc, prmv, "action.bRtDstPortMaskCmp_Action", sizeof(pce_rule.action.bRtDstPortMaskCmp_Action), &pce_rule.action.bRtDstPortMaskCmp_Action);
    scanParamArg(prmc, prmv, "action.bRtSrcPortMaskCmp_Action", sizeof(pce_rule.action.bRtSrcPortMaskCmp_Action), &pce_rule.action.bRtSrcPortMaskCmp_Action);
    scanParamArg(prmc, prmv, "action.bRtDstIpMaskCmp_Action", sizeof(pce_rule.action.bRtDstIpMaskCmp_Action), &pce_rule.action.bRtDstIpMaskCmp_Action);
    scanParamArg(prmc, prmv, "action.bRtSrcIpMaskCmp_Action", sizeof(pce_rule.action.bRtSrcIpMaskCmp_Action), &pce_rule.action.bRtSrcIpMaskCmp_Action);
    scanParamArg(prmc, prmv, "action.bRtInnerIPasKey_Action", sizeof(pce_rule.action.bRtInnerIPasKey_Action), &pce_rule.action.bRtInnerIPasKey_Action);

    scanParamArg(prmc, prmv, "action.bRtAccelEna_Action", sizeof(pce_rule.action.bRtAccelEna_Action), &pce_rule.action.bRtAccelEna_Action);
    scanParamArg(prmc, prmv, "action.bRtCtrlEna_Action", sizeof(pce_rule.action.bRtCtrlEna_Action), &pce_rule.action.bRtCtrlEna_Action);
    scanParamArg(prmc, prmv, "action.eProcessPath_Action", sizeof(pce_rule.action.eProcessPath_Action), &pce_rule.action.eProcessPath_Action);
    scanParamArg(prmc, prmv, "action.ePortFilterType_Action", sizeof(pce_rule.action.ePortFilterType_Action), &pce_rule.action.ePortFilterType_Action);

    scanParamArg(prmc, prmv, "action.bOamEnable", sizeof(pce_rule.action.bOamEnable), &pce_rule.action.bOamEnable);
    scanParamArg(prmc, prmv, "action.nRecordId", sizeof(pce_rule.action.nRecordId), &pce_rule.action.nRecordId);
    scanParamArg(prmc, prmv, "action.bExtractEnable", sizeof(pce_rule.action.bExtractEnable), &pce_rule.action.bExtractEnable);
    scanParamArg(prmc, prmv, "action.eColorFrameAction", sizeof(pce_rule.action.eColorFrameAction), &pce_rule.action.eColorFrameAction);
    scanParamArg(prmc, prmv, "action.bExtendedVlanEnable", sizeof(pce_rule.action.bExtendedVlanEnable), &pce_rule.action.bExtendedVlanEnable);
    scanParamArg(prmc, prmv, "action.nExtendedVlanBlockId", sizeof(pce_rule.action.nExtendedVlanBlockId), &pce_rule.action.nExtendedVlanBlockId);

    /*Aplicable for GSWIP 3.2*/
    scanParamArg(prmc, prmv, "pattern.bFlexibleField4Enable",
                 sizeof(pce_rule.pattern.bFlexibleField4Enable), &pce_rule.pattern.bFlexibleField4Enable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField4_ExcludeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField4_ExcludeEnable), &pce_rule.pattern.bFlexibleField4_ExcludeEnable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField4_RangeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField4_RangeEnable), &pce_rule.pattern.bFlexibleField4_RangeEnable);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField4_ParserIndex",
                 sizeof(pce_rule.pattern.nFlexibleField4_ParserIndex), &pce_rule.pattern.nFlexibleField4_ParserIndex);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField4_Value",
                 sizeof(pce_rule.pattern.nFlexibleField4_Value), &pce_rule.pattern.nFlexibleField4_Value);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField4_MaskOrRange",
                 sizeof(pce_rule.pattern.nFlexibleField4_MaskOrRange), &pce_rule.pattern.nFlexibleField4_MaskOrRange);

    scanParamArg(prmc, prmv, "pattern.bFlexibleField3Enable",
                 sizeof(pce_rule.pattern.bFlexibleField3Enable), &pce_rule.pattern.bFlexibleField3Enable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField3_ExcludeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField3_ExcludeEnable), &pce_rule.pattern.bFlexibleField3_ExcludeEnable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField3_RangeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField3_RangeEnable), &pce_rule.pattern.bFlexibleField3_RangeEnable);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField3_ParserIndex",
                 sizeof(pce_rule.pattern.nFlexibleField3_ParserIndex), &pce_rule.pattern.nFlexibleField3_ParserIndex);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField3_Value",
                 sizeof(pce_rule.pattern.nFlexibleField3_Value), &pce_rule.pattern.nFlexibleField3_Value);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField3_MaskOrRange",
                 sizeof(pce_rule.pattern.nFlexibleField3_MaskOrRange), &pce_rule.pattern.nFlexibleField3_MaskOrRange);

    scanParamArg(prmc, prmv, "pattern.bFlexibleField2Enable",
                 sizeof(pce_rule.pattern.bFlexibleField2Enable), &pce_rule.pattern.bFlexibleField2Enable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField2_ExcludeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField2_ExcludeEnable), &pce_rule.pattern.bFlexibleField2_ExcludeEnable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField2_RangeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField2_RangeEnable), &pce_rule.pattern.bFlexibleField2_RangeEnable);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField2_ParserIndex",
                 sizeof(pce_rule.pattern.nFlexibleField2_ParserIndex), &pce_rule.pattern.nFlexibleField2_ParserIndex);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField2_Value",
                 sizeof(pce_rule.pattern.nFlexibleField2_Value), &pce_rule.pattern.nFlexibleField2_Value);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField2_MaskOrRange",
                 sizeof(pce_rule.pattern.nFlexibleField2_MaskOrRange), &pce_rule.pattern.nFlexibleField2_MaskOrRange);

    scanParamArg(prmc, prmv, "pattern.bFlexibleField1Enable",
                 sizeof(pce_rule.pattern.bFlexibleField1Enable), &pce_rule.pattern.bFlexibleField1Enable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField1_ExcludeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField1_ExcludeEnable), &pce_rule.pattern.bFlexibleField1_ExcludeEnable);
    scanParamArg(prmc, prmv, "pattern.bFlexibleField1_RangeEnable",
                 sizeof(pce_rule.pattern.bFlexibleField1_RangeEnable), &pce_rule.pattern.bFlexibleField1_RangeEnable);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField1_ParserIndex",
                 sizeof(pce_rule.pattern.nFlexibleField1_ParserIndex), &pce_rule.pattern.nFlexibleField1_ParserIndex);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField1_Value",
                 sizeof(pce_rule.pattern.nFlexibleField1_Value), &pce_rule.pattern.nFlexibleField1_Value);
    scanParamArg(prmc, prmv, "pattern.nFlexibleField1_MaskOrRange",
                 sizeof(pce_rule.pattern.nFlexibleField1_MaskOrRange), &pce_rule.pattern.nFlexibleField1_MaskOrRange);

    scanParamArg(prmc, prmv, "action.bPBB_Action_Enable", sizeof(pce_rule.action.bPBB_Action_Enable), &pce_rule.action.bPBB_Action_Enable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bIheaderActionEnable",
                 sizeof(pce_rule.action.sPBB_Action.bIheaderActionEnable), &pce_rule.action.sPBB_Action.bIheaderActionEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.eIheaderOpMode",
                 sizeof(pce_rule.action.sPBB_Action.eIheaderOpMode), &pce_rule.action.sPBB_Action.eIheaderOpMode);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bTunnelIdKnownTrafficEnable",
                 sizeof(pce_rule.action.sPBB_Action.bTunnelIdKnownTrafficEnable), &pce_rule.action.sPBB_Action.bTunnelIdKnownTrafficEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.nTunnelIdKnownTraffic",
                 sizeof(pce_rule.action.sPBB_Action.nTunnelIdKnownTraffic), &pce_rule.action.sPBB_Action.nTunnelIdKnownTraffic);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bTunnelIdUnKnownTrafficEnable",
                 sizeof(pce_rule.action.sPBB_Action.bTunnelIdUnKnownTrafficEnable), &pce_rule.action.sPBB_Action.bTunnelIdUnKnownTrafficEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.nTunnelIdUnKnownTraffic",
                 sizeof(pce_rule.action.sPBB_Action.nTunnelIdUnKnownTraffic), &pce_rule.action.sPBB_Action.nTunnelIdUnKnownTraffic);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bB_DstMac_FromMacTableEnable",
                 sizeof(pce_rule.action.sPBB_Action.bB_DstMac_FromMacTableEnable), &pce_rule.action.sPBB_Action.bB_DstMac_FromMacTableEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_B_SrcMacEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_B_SrcMacEnable), &pce_rule.action.sPBB_Action.bReplace_B_SrcMacEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_B_DstMacEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_B_DstMacEnable), &pce_rule.action.sPBB_Action.bReplace_B_DstMacEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_I_TAG_ResEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_I_TAG_ResEnable), &pce_rule.action.sPBB_Action.bReplace_I_TAG_ResEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_I_TAG_UacEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_I_TAG_UacEnable), &pce_rule.action.sPBB_Action.bReplace_I_TAG_UacEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_I_TAG_DeiEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_I_TAG_DeiEnable), &pce_rule.action.sPBB_Action.bReplace_I_TAG_DeiEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_I_TAG_PcpEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_I_TAG_PcpEnable), &pce_rule.action.sPBB_Action.bReplace_I_TAG_PcpEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_I_TAG_SidEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_I_TAG_SidEnable), &pce_rule.action.sPBB_Action.bReplace_I_TAG_SidEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_I_TAG_TpidEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_I_TAG_TpidEnable), &pce_rule.action.sPBB_Action.bReplace_I_TAG_TpidEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bBtagActionEnable",
                 sizeof(pce_rule.action.sPBB_Action.bBtagActionEnable), &pce_rule.action.sPBB_Action.bBtagActionEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.eBtagOpMode",
                 sizeof(pce_rule.action.sPBB_Action.eBtagOpMode), &pce_rule.action.sPBB_Action.eBtagOpMode);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bProcessIdKnownTrafficEnable",
                 sizeof(pce_rule.action.sPBB_Action.bProcessIdKnownTrafficEnable), &pce_rule.action.sPBB_Action.bProcessIdKnownTrafficEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.nProcessIdKnownTraffic",
                 sizeof(pce_rule.action.sPBB_Action.nProcessIdKnownTraffic), &pce_rule.action.sPBB_Action.nProcessIdKnownTraffic);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bProcessIdUnKnownTrafficEnable",
                 sizeof(pce_rule.action.sPBB_Action.bProcessIdUnKnownTrafficEnable), &pce_rule.action.sPBB_Action.bProcessIdUnKnownTrafficEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.nProcessIdUnKnownTraffic",
                 sizeof(pce_rule.action.sPBB_Action.nProcessIdUnKnownTraffic), &pce_rule.action.sPBB_Action.nProcessIdUnKnownTraffic);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_B_TAG_DeiEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_B_TAG_DeiEnable), &pce_rule.action.sPBB_Action.bReplace_B_TAG_DeiEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_B_TAG_PcpEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_B_TAG_PcpEnable), &pce_rule.action.sPBB_Action.bReplace_B_TAG_PcpEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_B_TAG_VidEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_B_TAG_VidEnable), &pce_rule.action.sPBB_Action.bReplace_B_TAG_VidEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.bReplace_B_TAG_TpidEnable",
                 sizeof(pce_rule.action.sPBB_Action.bReplace_B_TAG_TpidEnable), &pce_rule.action.sPBB_Action.bReplace_B_TAG_TpidEnable);

    scanParamArg(prmc, prmv, "action.sPBB_Action.bMacTableMacinMacActionEnable",
                 sizeof(pce_rule.action.sPBB_Action.bMacTableMacinMacActionEnable), &pce_rule.action.sPBB_Action.bMacTableMacinMacActionEnable);
    scanParamArg(prmc, prmv, "action.sPBB_Action.eMacTableMacinMacSelect",
                 sizeof(pce_rule.action.sPBB_Action.eMacTableMacinMacSelect), &pce_rule.action.sPBB_Action.eMacTableMacinMacSelect);

    scanParamArg(prmc, prmv, "action.bDestSubIf_Action_Enable",
                 sizeof(pce_rule.action.bDestSubIf_Action_Enable), &pce_rule.action.bDestSubIf_Action_Enable);
    scanParamArg(prmc, prmv, "action.sDestSubIF_Action.bDestSubIFIDActionEnable",
                 sizeof(pce_rule.action.sDestSubIF_Action.bDestSubIFIDActionEnable), &pce_rule.action.sDestSubIF_Action.bDestSubIFIDActionEnable);
    scanParamArg(prmc, prmv, "action.sDestSubIF_Action.bDestSubIFIDAssignmentEnable",
                 sizeof(pce_rule.action.sDestSubIF_Action.bDestSubIFIDAssignmentEnable), &pce_rule.action.sDestSubIF_Action.bDestSubIFIDAssignmentEnable);
    scanParamArg(prmc, prmv, "action.sDestSubIF_Action.nDestSubIFGrp_Field",
                 sizeof(pce_rule.action.sDestSubIF_Action.nDestSubIFGrp_Field), &pce_rule.action.sDestSubIF_Action.nDestSubIFGrp_Field);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleWrite(gsw_dev, &pce_rule);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleWrite failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PceRuleWrite done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleDelete(int prmc, char *prmv[])
{
    GSW_PCE_ruleEntry_t pce_rule;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    memset(&pce_rule, 0, sizeof(GSW_PCE_ruleEntry_t));

    rret = scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(pce_rule.logicalportid), &pce_rule.logicalportid);
    if (rret < 1)
    {
        printf("Parameter not Found: nLogicalPortId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "pattern.nIndex", sizeof(pce_rule.nIndex), &pce_rule.nIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: pattern.nIndex\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(pce_rule.subifidgroup), &pce_rule.subifidgroup);
    scanParamArg(prmc, prmv, "region", sizeof(pce_rule.region), &pce_rule.region);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleDelete(gsw_dev, &pce_rule);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleDelete failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PceRuleDelete done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleAlloc(int prmc, char *prmv[])
{
    GSW_PCE_rule_alloc_t alloc = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "num_of_rules", sizeof(alloc.num_of_rules), &alloc.num_of_rules);
    if (rret < 1)
    {
        printf("Parameter not Found: num_of_rules\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleAlloc(gsw_dev, &alloc);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleAlloc failed with ret code", ret);
    else
    {
        printf("\n\tret          = %d", ret);
        printf("\n\tblockid      = %u", alloc.blockid);
        printf("\n\tnum_of_rules = %u", alloc.num_of_rules);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleFree(int prmc, char *prmv[])
{
    GSW_PCE_rule_alloc_t alloc = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "blockid", sizeof(alloc.num_of_rules), &alloc.blockid);
    if (rret < 1)
    {
        printf("Parameter not Found: blockid\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleFree(gsw_dev, &alloc);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleFree failed with ret code", ret);
    else
    {
        printf("\n\tret          = %d", ret);
        printf("\n\tblockid      = %u", alloc.blockid);
        printf("\n\tnum_of_rules = %u", alloc.num_of_rules);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleEnable(int prmc, char *prmv[])
{
    GSW_PCE_ruleEntry_t pce_rule = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(pce_rule.logicalportid), &pce_rule.logicalportid);
    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(pce_rule.subifidgroup), &pce_rule.subifidgroup);
    scanParamArg(prmc, prmv, "region", sizeof(pce_rule.subifidgroup), &pce_rule.region);
    scanParamArg(prmc, prmv, "pattern.nIndex", sizeof(pce_rule.nIndex), &pce_rule.nIndex);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleEnable(gsw_dev, &pce_rule);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleEnable failed with ret code", ret);
    else
    {
        printf("\n\tret            = %d", ret);
        printf("\n\tregion         = %u", pce_rule.region);
        printf("\n\tpattern.nIndex = %u", pce_rule.nIndex);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PceRuleDisable(int prmc, char *prmv[])
{
    GSW_PCE_ruleEntry_t pce_rule = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    scanParamArg(prmc, prmv, "nLogicalPortId", sizeof(pce_rule.logicalportid), &pce_rule.logicalportid);
    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(pce_rule.subifidgroup), &pce_rule.subifidgroup);
    scanParamArg(prmc, prmv, "region", sizeof(pce_rule.subifidgroup), &pce_rule.region);
    scanParamArg(prmc, prmv, "pattern.nIndex", sizeof(pce_rule.nIndex), &pce_rule.nIndex);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleDisable(gsw_dev, &pce_rule);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PceRuleDisable failed with ret code", ret);
    else
    {
        printf("\n\tret            = %d", ret);
        printf("\n\tregion         = %u", pce_rule.region);
        printf("\n\tpattern.nIndex = %u", pce_rule.nIndex);
        printf("\n");
    }

    return ret;
}

// int gsw_dump_pce_mem(int prmc, char *prmv[])
// {
// 	GSW_table_t sVar = {0};
// 	GSW_register_t reg = {0};
// 	unsigned int i = 0, j = 0, k = 0, m = 0;
// 	int num_of_elem;
// 	GSW_Device_t *gsw_dev;
//     GSW_return_t ret;
//     int rret;
//     gsw_dev = gsw_get_struc(lif_id,0);

// 	num_of_elem = (sizeof(tbl_dump_gsw33) / sizeof(struct _tbl_dump_));

// 	for (i = 0; i < num_of_elem; i++) {
// 		printf("===========================================\n");
// 		printf("Table Name: %s\n", tbl_dump_gsw33[i].tbl_name);
// 		printf("===========================================\n");

// 		for (j = 0; j < tbl_dump_gsw33[i].entries; j++) {
// 			memset(&sVar, 0, sizeof(sVar));

// 			printf("Table Idx: %02d\n", j);
// 			sVar.tbl_entry = j;
// 			sVar.tbl_addr = tbl_dump_gsw33[i].tbl_addr;
// 			sVar.tbl_id = 1;
// 			gsw_cli_ops->gsw_debug_ops.DumpMem(gsw_dev, &sVar);

// 			for (k = 0; k < gsw_pce_tbl_33[sVar.tbl_addr].num_key; k++)
// 				printf("\tKey  %d:  %04x\n", k, sVar.ptdata.key[k]);

// 			for (k = 0; k < gsw_pce_tbl_33[sVar.tbl_addr].num_mask; k++)
// 				printf("\tMask %d:  %04x\n", k, sVar.ptdata.mask[k]);

// 			for (k = 0; k < gsw_pce_tbl_33[sVar.tbl_addr].num_val; k++)
// 				printf("\tVal  %d:  %04x\n", k, sVar.ptdata.val[k]);

// 			printf("\tValid:   %x\n", sVar.ptdata.valid);
// 			printf("\tType:    %x\n", sVar.ptdata.type);
// 			printf("\n");

// 			for (m = 0; m < 500000; m++);
// 		}
// 	}
// 	printf("===========================================\n");
// 	for (i = 0; i < 0xEFF; i++) {

// 		reg.nData = 0;
// 		reg.nRegAddr = i;
// 		GSW_RegisterGet(gsw_dev, &reg);
// 		printf("%08x:  %08x\n", (0xC0D52000 + (i * 0x4)), reg.nData);
// 	}

// 	return ret;
// }

GSW_return_t fapi_GSW_MulticastRouterPortAdd(int prmc, char *prmv[])
{
    GSW_multicastRouter_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MulticastRouterPortAdd(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastRouterPortAdd failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MulticastRouterPortAdd done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MulticastRouterPortRemove(int prmc, char *prmv[])
{
    GSW_multicastRouter_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MulticastRouterPortRemove(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastRouterPortRemove failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MulticastRouterPortRemove done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MulticastSnoopCfgGet(int prmc, char *prmv[])
{
    GSW_multicastSnoopCfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MulticastSnoopCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastSnoopCfgGet failed with ret code", ret);
    else
    {
        printf("Returned values:\n----------------\n");
        printf("\t%40s:\t0x%x\n", "eIGMP_Mode", param.eIGMP_Mode);
        printf("\t%40s:\t%s\n", "bCrossVLAN", (param.bCrossVLAN > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t0x%x\n", "eForwardPort", param.eForwardPort);
        printf("\t%40s:\t0x%x\n", "nForwardPortId", param.nForwardPortId);
        printf("\t%40s:\t0x%x\n", "nClassOfService", param.nClassOfService);
    }

    return ret;
}

GSW_return_t fapi_GSW_MulticastSnoopCfgSet(int prmc, char *prmv[])
{
    GSW_multicastSnoopCfg_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MulticastSnoopCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_MulticastSnoopCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "eIGMP_Mode", sizeof(param.eIGMP_Mode), &param.eIGMP_Mode);
    scanParamArg(prmc, prmv, "bCrossVLAN", sizeof(param.bCrossVLAN), &param.bCrossVLAN);
    scanParamArg(prmc, prmv, "eForwardPort", sizeof(param.eForwardPort), &param.eForwardPort);
    scanParamArg(prmc, prmv, "nForwardPortId", sizeof(param.nForwardPortId), &param.nForwardPortId);
    scanParamArg(prmc, prmv, "nClassOfService", sizeof(param.nClassOfService), &param.nClassOfService);

    ret = GSW_MulticastSnoopCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastSnoopCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MulticastSnoopCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MulticastRouterPortRead(int prmc, char *prmv[])
{
    GSW_multicastRouterRead_t multicastRouterRead = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    multicastRouterRead.bInitial = 1;
    gsw_dev = gsw_get_struc(lif_id, 0);

    for (;;)
    {
        ret = GSW_MulticastRouterPortRead(gsw_dev, &multicastRouterRead);
        if (ret < 0)
        {
            printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastRouterPortRead failed with ret code", ret);
            return ret;
        }

        if (multicastRouterRead.bLast == 1)
            break;

        printf("\t%40s:\t%d\n", "Router Port", multicastRouterRead.nPortId);
        memset(&multicastRouterRead, 0x00, sizeof(multicastRouterRead));
    }

    return ret;
}

GSW_return_t fapi_GSW_MulticastTableEntryAdd(int prmc, char *prmv[])
{
    GSW_multicastTable_t param;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int retval;

    retval = multicastParamRead(prmc, prmv, &param);
    if (retval != 0)
        return retval;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MulticastTableEntryAdd(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastTableEntryAdd failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MulticastTableEntryAdd done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_MulticastTableEntryRead(int prmc, char *prmv[])
{
    GSW_multicastTableRead_t multicastTableRead;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int k = 0, valid = 0;
    u8 mcasthitsts_en = 0;
    GSW_register_t param = {0};

    memset(&param, 0, sizeof(GSW_register_t));
    param.nRegAddr = 0x456;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RegisterGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_RegisterGet failed with ret code", ret);
        return ret;
    }

    if (param.nData & 0x2000)
        mcasthitsts_en = 1;
    else
        mcasthitsts_en = 0;

    printf("-----------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("| %4s | %6s | %3s | %39s | %39s | %11s | %9s | %9s\n", "Port", "Sub Id", "FID", "GDA", "GSA", "Member Mode", "HitStatus", "VLAN Info");
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------\n");

    memset(&multicastTableRead, 0x00, sizeof(multicastTableRead));
    multicastTableRead.bInitial = 1;

    for (;;)
    {
        ret = GSW_MulticastTableEntryRead(gsw_dev, &multicastTableRead);
        if (ret < 0)
            break;

        if (multicastTableRead.bLast == 1)
            break;

        if ((multicastTableRead.nPortId == 0) && (multicastTableRead.nSubIfId == 0) && (multicastTableRead.nFID == 0))
        {
            valid = 0;

            for (k = 0; k < 8; k++)
            {
                if (multicastTableRead.uIP_Gsa.nIPv6[k] != 0)
                    valid = 1;
            }

            for (k = 0; k < 8; k++)
            {
                if (multicastTableRead.uIP_Gda.nIPv6[k] != 0)
                    valid = 1;
            }

            for (k = 0; k < ARRAY_SIZE(multicastTableRead.nPortMap); k++)
            {
                if (multicastTableRead.nPortMap[k] != 0)
                    valid = 1;
            }

            if (valid == 0)
                continue;
        }

        if (multicastTableRead.nPortId & GSW_PORTMAP_FLAG_GET(GSW_multicastTableRead_t))
        {

            unsigned int i = 0, j = 0, mask = 1;

            for (j = 0; j < ARRAY_SIZE(multicastTableRead.nPortMap); j++)
            {
                i = 0;
                mask = 1;

                if (!multicastTableRead.nPortMap[j])
                    continue;

                while (mask <= (1 << 16))
                {
                    if (mask & multicastTableRead.nPortMap[j])
                    {
                        if (mcasthitsts_en && (j == 7) && (mask == (1 << 15)))
                        {
                            break;
                        }

                        printf("| %4d |", (j * 16) + i);
                        printf(" %6d |", multicastTableRead.nSubIfId);
                        printf(" %3d |", multicastTableRead.nFID);
                        dump_multicast_table_entry(&multicastTableRead);
                    }

                    i++;
                    mask = 1 << i;
                }
            }
        }
        else
        {
            printf("| %4d |", multicastTableRead.nPortId);
            printf(" %6d |", multicastTableRead.nSubIfId);
            printf(" %3d |", multicastTableRead.nFID);
            dump_multicast_table_entry(&multicastTableRead);
        }

        memset(&multicastTableRead, 0x00, sizeof(multicastTableRead));
    }

    printf("-----------------------------------------------------------------------------------------------------------------------------------------------\n");

    return ret;
}

GSW_return_t fapi_GSW_MulticastTableEntryRemove(int prmc, char *prmv[])
{
    GSW_multicastTable_t param;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int retval;

    retval = multicastParamRead(prmc, prmv, &param);
    if (retval != 0)
        return retval;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MulticastTableEntryRemove(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_MulticastTableEntryRemove failed with ret code", ret);
    else
    {
        printf("fapi_GSW_MulticastTableEntryRemove done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_FW_Update(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_fw_update(gsw_dev);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_FW_Update failed with ret code", ret);
    else
    {
        printf("fapi_GSW_FW_Update done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_FW_Version(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    struct sys_fw_image_version sys_img_ver = {0};

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_fw_version(gsw_dev, &sys_img_ver);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_FW_Version failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "Major", sys_img_ver.major);
        printf("\t%40s:\t%x\n", "Minor", sys_img_ver.minor);
        printf("\t%40s:\t%u\n", "Revision", sys_img_ver.revision);
        printf("\t%40s:\t%u\n", "APP Revision", sys_img_ver.app_revision);
    }

    return ret;
}

GSW_return_t fapi_GSW_PVT_Meas(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    struct sys_sensor_value sensor_value_temp = {0};
    struct sys_sensor_value sensor_value_volt = {0};

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_pvt_temp(gsw_dev, &sensor_value_temp);
    ret = sys_misc_pvt_voltage(gsw_dev, &sensor_value_volt);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PVT_Temp failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%d.%d\n", "Temp", sensor_value_temp.val1, sensor_value_temp.val2);
        printf("\t%40s:\t%d.%d\n", "Voltage", sensor_value_volt.val1, sensor_value_volt.val2);
    }

    return ret;
}

GSW_return_t fapi_GSW_Delay(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_delay param = {0};

    rret = scanParamArg(prmc, prmv, "nMsec", sizeof(param.m_sec), &param.m_sec);
    if (rret < 1)
    {
        printf("Parameter not Found: nMsec\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_delay(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Delay failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Delay done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_GPIO_Configure(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_gpio_config param = {0};

    scanParamArg(prmc, prmv, "nEnableMaskIndex0", sizeof(param.enable_mask[0]), &param.enable_mask[0]);
    scanParamArg(prmc, prmv, "nEnableMaskIndex1", sizeof(param.enable_mask[1]), &param.enable_mask[1]);
    scanParamArg(prmc, prmv, "nEnableMaskIndex2", sizeof(param.enable_mask[2]), &param.enable_mask[2]);
    scanParamArg(prmc, prmv, "nAltSel0Index0", sizeof(param.alt_sel_0[0]), &param.alt_sel_0[0]);
    scanParamArg(prmc, prmv, "nAltSel0Index1", sizeof(param.alt_sel_0[1]), &param.alt_sel_0[1]);
    scanParamArg(prmc, prmv, "nAltSel0Index2", sizeof(param.alt_sel_0[2]), &param.alt_sel_0[2]);
    scanParamArg(prmc, prmv, "nAltSel1Index0", sizeof(param.alt_sel_1[0]), &param.alt_sel_1[0]);
    scanParamArg(prmc, prmv, "nAltSel1Index1", sizeof(param.alt_sel_1[1]), &param.alt_sel_1[1]);
    scanParamArg(prmc, prmv, "nAltSel1Index2", sizeof(param.alt_sel_1[2]), &param.alt_sel_1[2]);
    scanParamArg(prmc, prmv, "nDirIndex0", sizeof(param.dir[0]), &param.dir[0]);
    scanParamArg(prmc, prmv, "nDirIndex1", sizeof(param.dir[1]), &param.dir[1]);
    scanParamArg(prmc, prmv, "nDirIndex2", sizeof(param.dir[2]), &param.dir[2]);
    scanParamArg(prmc, prmv, "nOutValueIndex0", sizeof(param.out_val[0]), &param.out_val[0]);
    scanParamArg(prmc, prmv, "nOutValueIndex1", sizeof(param.out_val[1]), &param.out_val[1]);
    scanParamArg(prmc, prmv, "nOutValueIndex2", sizeof(param.out_val[2]), &param.out_val[2]);
    scanParamArg(prmc, prmv, "nTimeoutValue", sizeof(param.timeout_val), &param.timeout_val);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_gpio_configure(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_GPIO_Configure failed with ret code", ret);
    else
    {
        printf("fapi_GSW_GPIO_Configure done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Reboot(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_reboot(gsw_dev);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Reboot failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Reboot done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_SysReg_Rd(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_reg_rw sys_reg = {0};

    rret = scanParamArg(prmc, prmv, "addr", sizeof(sys_reg.addr), &sys_reg.addr);
    if (rret < 1)
    {
        printf("Parameter not Found: addr\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_reg_rd(gsw_dev, &sys_reg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_SysReg_Rd failed with ret code", ret);
    else
        printf("fapi_GSW_SysReg_Rd:\n\t addr=0x%x val=0x%x\n", sys_reg.addr, sys_reg.val);

    return ret;
}

GSW_return_t fapi_GSW_SysReg_Wr(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_reg_rw sys_reg = {0};

    rret = scanParamArg(prmc, prmv, "addr", sizeof(sys_reg.addr), &sys_reg.addr);
    if (rret < 1)
    {
        printf("Parameter not Found: addr\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "val", sizeof(sys_reg.val), &sys_reg.val);
    if (rret < 1)
    {
        printf("parameter not Found: val\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_reg_wr(gsw_dev, &sys_reg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_SysReg_Wr failed with ret code", ret);
    else
    {
        printf("fapi_GSW_SysReg_Wr done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_SysReg_Mod(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_reg_mod sys_reg = {0};

    rret = scanParamArg(prmc, prmv, "addr", sizeof(sys_reg.addr), &sys_reg.addr);
    if (rret < 1)
    {
        printf("Parameter not Found: addr\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "val", sizeof(sys_reg.val), &sys_reg.val);
    if (rret < 1)
    {
        printf("parameter not Found: val\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mask", sizeof(sys_reg.mask), &sys_reg.mask);
    if (rret < 1)
    {
        printf("parameter not Found: mask\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_reg_mod(gsw_dev, &sys_reg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_SysReg_Mod failed with ret code", ret);
    else
    {
        printf("fapi_GSW_SysReg_Mod done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Cml_Clk_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_cml_clk param = {0};
    unsigned char nClk;

    rret = scanParamArg(prmc, prmv, "nClk", sizeof(nClk), &nClk);
    if (rret < 1)
    {
        printf("Parameter not Found: nClk\n");
        return OS_ERROR;
    }

    param.clk = nClk;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_cml_clk_get(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Cml_Clk_Get failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%d\n", "Clock", param.clk);
        printf("\t%40s:\t%d\n", "Enable", param.en);
        printf("\t%40s:\t%d\n", "Source Selection", param.src_sel);
    }

    return ret;
}

GSW_return_t fapi_GSW_Cml_Clk_Set(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_cml_clk param = {0};
    unsigned char nClk;
    unsigned char bEn;
    unsigned char nEnable;
    unsigned char bSrcSel;
    unsigned char nSrcSel;

    rret = scanParamArg(prmc, prmv, "nClk", sizeof(nClk), &nClk);
    if (rret < 1)
    {
        printf("Parameter not Found: nClk\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "bEn", sizeof(bEn), &bEn);
    scanParamArg(prmc, prmv, "nEnable", sizeof(nEnable), &nEnable);
    scanParamArg(prmc, prmv, "bSrcSel", sizeof(bSrcSel), &bSrcSel);
    scanParamArg(prmc, prmv, "nSrcSel", sizeof(nSrcSel), &nSrcSel);

    param.clk = nClk;
    param.en_val = bEn;
    param.en = nEnable;
    param.src_val = bSrcSel;
    param.src_sel = nSrcSel;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_cml_clk_set(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Cml_Clk_Set failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Cml_Clk_Set done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Sfp_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_sfp_cfg cfg = {0};
    unsigned char nPort;
    unsigned char nOption;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(nPort), &nPort);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nOption", sizeof(nOption), &nOption);
    if (rret < 1)
    {
        printf("Parameter not Found: nOption\n");
        return OS_ERROR;
    }

    cfg.port_id = nPort;
    cfg.option = nOption;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_sfp_get(gsw_dev, &cfg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Sfp_Get failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%d\n", "nPortId", cfg.port_id);
        printf("\t%40s:\t%d\n", "nOption", cfg.option);
        if (cfg.option)
        {
            printf("\t%40s:\t%d\n", "bFlowCtrlEn", cfg.fc_en);
        }
        else
        {
            printf("\t%40s:\t%d\n", "nMode", cfg.mode);
            printf("\t%40s:\t%d\n", "nSpeed", cfg.speed);
            printf("\t%40s:\t%d\n", "nLink", cfg.link);
        }
    }

    return ret;
}

GSW_return_t fapi_GSW_Sfp_Set(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;
    struct sys_sfp_cfg cfg = {0};
    uint8_t val;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(val), &val);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }
    cfg.port_id = val;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = sys_misc_sfp_get(gsw_dev, &cfg);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "sys_misc_sfp_get failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "nOption", sizeof(val), &val);
    if (rret < 1)
    {
        printf("Parameter not Found: nOption\n");
        return OS_ERROR;
    }

    cfg.option = val;
    if (cfg.option)
    {
        scanParamArg(prmc, prmv, "bFlowCtrlEn", sizeof(val), &val);
        cfg.fc_en = val;
    }
    else
    {
        scanParamArg(prmc, prmv, "nMode", sizeof(val), &val);
        cfg.mode = val;
        scanParamArg(prmc, prmv, "nSpeed", sizeof(val), &val);
        cfg.speed = val;
    }

    ret = sys_misc_sfp_set(gsw_dev, &cfg);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Sfp_Set failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Sfp_Set done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Debug_RMON_Port_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_Debug_RMON_Port_cnt_t sVar;
    memset(&sVar, 0, sizeof(GSW_Debug_RMON_Port_cnt_t));
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(sVar.nPortId), &sVar.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "ePortType", sizeof(sVar.ePortType), &sVar.ePortType);
    if (rret < 1)
    {
        printf("Parameter not Found: ePortType\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_Debug_RMON_Port_Get(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Debug_RMON_Port_Get failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nPortId", sVar.nPortId);
        printf("\t%40s:\t%x\n", "ePortType", sVar.ePortType);
        printf("\t%40s:\t%s\n", "RMON Counter BitMode", "64");
        printf("\n\n");
        printf("\t%40s:\t%u\n", "nRxGoodPkts", sVar.nRxGoodPkts);
        printf("\t%40s:\t%u\n", "nRxUnicastPkts", sVar.nRxUnicastPkts);
        printf("\t%40s:\t%u\n", "nRxBroadcastPkts", sVar.nRxBroadcastPkts);
        printf("\t%40s:\t%u\n", "nRxMulticastPkts", sVar.nRxMulticastPkts);
        printf("\t%40s:\t%u\n", "nRxFCSErrorPkts", sVar.nRxFCSErrorPkts);
        printf("\t%40s:\t%u\n", "nRxUnderSizeGoodPkts", sVar.nRxUnderSizeGoodPkts);
        printf("\t%40s:\t%u\n", "nRxOversizeGoodPkts", sVar.nRxOversizeGoodPkts);
        printf("\t%40s:\t%u\n", "nRxUnderSizeErrorPkts", sVar.nRxUnderSizeErrorPkts);
        printf("\t%40s:\t%u\n", "nRxOversizeErrorPkts", sVar.nRxOversizeErrorPkts);
        printf("\t%40s:\t%u\n", "nRxFilteredPkts", sVar.nRxFilteredPkts);
        printf("\t%40s:\t%u\n", "nRx64BytePkts", sVar.nRx64BytePkts);
        printf("\t%40s:\t%u\n", "nRx127BytePkts", sVar.nRx127BytePkts);
        printf("\t%40s:\t%u\n", "nRx255BytePkts", sVar.nRx255BytePkts);
        printf("\t%40s:\t%u\n", "nRx511BytePkts", sVar.nRx511BytePkts);
        printf("\t%40s:\t%u\n", "nRx1023BytePkts", sVar.nRx1023BytePkts);
        printf("\t%40s:\t%u\n", "nRxMaxBytePkts", sVar.nRxMaxBytePkts);
        printf("\t%40s:\t%u\n", "nRxDroppedPkts", sVar.nRxDroppedPkts);
        printf("\t%40s:\t%u\n", "nRxExtendedVlanDiscardPkts", sVar.nRxExtendedVlanDiscardPkts);
        printf("\t%40s:\t%u\n", "nMtuExceedDiscardPkts", sVar.nMtuExceedDiscardPkts);
        printf("\t%40s:\t%llu 0x%llx\n", "nRxGoodBytes", (unsigned long long)sVar.nRxGoodBytes, (unsigned long long)sVar.nRxGoodBytes);
        printf("\t%40s:\t%llu 0x%llx\n", "nRxBadBytes", (unsigned long long)sVar.nRxBadBytes, (unsigned long long)sVar.nRxBadBytes);

        /*Valid only for GSWIP3.2*/
        printf("\t%40s:\t%u\n", "nRxUnicastPktsYellowRed", sVar.nRxUnicastPktsYellowRed);
        printf("\t%40s:\t%u\n", "nRxBroadcastPktsYellowRed", sVar.nRxBroadcastPktsYellowRed);
        printf("\t%40s:\t%u\n", "nRxMulticastPktsYellowRed", sVar.nRxMulticastPktsYellowRed);
        printf("\t%40s:\t%u\n", "nRxGoodPktsYellowRed", sVar.nRxGoodPktsYellowRed);
        printf("\t%40s:\t%llu 0x%llx\n", "nRxGoodBytesYellowRed", (unsigned long long)sVar.nRxGoodBytesYellowRed, (unsigned long long)sVar.nRxGoodBytesYellowRed);

        printf("\n\n");
        printf("\t%40s:\t%u\n", "nTxGoodPkts", sVar.nTxGoodPkts);
        printf("\t%40s:\t%u\n", "nTxUnicastPkts", sVar.nTxUnicastPkts);
        printf("\t%40s:\t%u\n", "nTxBroadcastPkts", sVar.nTxBroadcastPkts);
        printf("\t%40s:\t%u\n", "nTxMulticastPkts", sVar.nTxMulticastPkts);
        printf("\t%40s:\t%u\n", "nTx64BytePkts", sVar.nTx64BytePkts);
        printf("\t%40s:\t%u\n", "nTx127BytePkts", sVar.nTx127BytePkts);
        printf("\t%40s:\t%u\n", "nTx255BytePkts", sVar.nTx255BytePkts);
        printf("\t%40s:\t%u\n", "nTx511BytePkts", sVar.nTx511BytePkts);
        printf("\t%40s:\t%u\n", "nTx1023BytePkts", sVar.nTx1023BytePkts);
        printf("\t%40s:\t%u\n", "nTxMaxBytePkts", sVar.nTxMaxBytePkts);
        printf("\t%40s:\t%u\n", "nTxDroppedPkts", sVar.nTxDroppedPkts);
        printf("\t%40s:\t%u\n", "nTxOversizeGoodPkts", sVar.nTxOversizeGoodPkts);
        printf("\t%40s:\t%u\n", "nTxUnderSizeGoodPkts", sVar.nTxUnderSizeGoodPkts);
        printf("\t%40s:\t%u\n", "nTxAcmDroppedPkts", sVar.nTxAcmDroppedPkts);
        printf("\t%40s:\t%llu 0x%llx\n", "nTxGoodBytes", (unsigned long long)sVar.nTxGoodBytes, (unsigned long long)sVar.nTxGoodBytes);

        printf("\t%40s:\t%u\n", "nTxUnicastPktsYellowRed", sVar.nTxUnicastPktsYellowRed);
        printf("\t%40s:\t%u\n", "nTxBroadcastPktsYellowRed", sVar.nTxBroadcastPktsYellowRed);
        printf("\t%40s:\t%u\n", "nTxMulticastPktsYellowRed", sVar.nTxMulticastPktsYellowRed);
        printf("\t%40s:\t%u\n", "nTxGoodPktsYellowRed", sVar.nTxGoodPktsYellowRed);
        printf("\t%40s:\t%llu 0x%llx\n", "nTxGoodBytesYellowRed", (unsigned long long)sVar.nTxGoodBytesYellowRed, (unsigned long long)sVar.nTxGoodBytesYellowRed);
    }

    return ret;
}

GSW_return_t fapi_GSW_DEBUG_RMON_Port_Get_All(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_Debug_RMON_Port_cnt_t *sVar_rx;
    GSW_Debug_RMON_Port_cnt_t *sVar_tx;
    u32 i = 0, j = 0, start = 0, end = 0, max_read = 0;
    GSW_RMON_portType_t eCounerType = 0;

    scanParamArg(prmc, prmv, "ePortType", sizeof(eCounerType), &eCounerType);
    scanParamArg(prmc, prmv, "Start", sizeof(start), &start);
    scanParamArg(prmc, prmv, "End", sizeof(end), &end);

    max_read = end - start;
    if (max_read > 16 || end > 16)
    {
        printf("Display only 16 ports, please check start and end\n");
        return -1;
    }

    sVar_rx = malloc(sizeof(*sVar_rx) * (max_read + 1) * 2);
    if (!sVar_rx)
    {
        printf("\n\tERROR: failed in buffer allocation\n");
        return -ENOMEM;
    }

    sVar_tx = &sVar_rx[max_read + 1];

    for (j = 0; j < 2; j++) {
        if (eCounerType == GSW_RMON_CTP_PORT_RX && j == 1) {
            eCounerType = GSW_RMON_CTP_PORT_TX;
        } else if (eCounerType == GSW_RMON_BRIDGE_PORT_RX && j == 1) {
            eCounerType = GSW_RMON_BRIDGE_PORT_TX;
        }

        switch (eCounerType) {
        case 0:
        case 1:
            printf("Reading CTP Port %s Counters\n", j ? "Tx" : "Rx");
            break;
        case 2:
        case 3:
            printf("Reading BRIDGE Port %s Counters\n", j ? "Tx" : "Rx");
            break;
        case 4:
            printf("Reading Bypass-PCE Port Tx Counters\n");
            break;
        default:
            break;
        }

        gsw_dev = gsw_get_struc(lif_id, 0);
        if (j == 0) {	// Getting RX port RMON
            for (i = 0; i <= max_read; i++) {
                sVar_rx[i].nPortId = start + i;
                sVar_rx[i].ePortType = eCounerType;

                ret = GSW_Debug_RMON_Port_Get(gsw_dev, &sVar_rx[i]);
                if (ret < 0)
                {
                    free(sVar_rx);
                    printf("\t%40s:\t0x%x\n", "GSW_Debug_RMON_Port_Get failed with ret code", ret);
                    return ret;
                }
            }
        } else {	// Getting TX port RMON
            for (i = 0; i <= max_read; i++) {
                sVar_tx[i].nPortId = start + i;
                sVar_tx[i].ePortType = eCounerType;

                ret = GSW_Debug_RMON_Port_Get(gsw_dev, &sVar_tx[i]);
                if (ret < 0)
                {
                    free(sVar_rx);
                    printf("\t%40s:\t0x%x\n", "GSW_Debug_RMON_Port_Get failed with ret code", ret);
                    return ret;
                }
            }
        }
    }

    printf("\n\n");
    printf("Port                                   : ");

    for (i = start; i <= end; i++)
        printf("%11u", i);

    printf("\n");

    if (sVar_rx[0].ePortType != 4) {
        printf("\n");
        printf("nRxGoodPkts                            : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxGoodPkts + sVar_rx[i].nRxGoodPktsYellowRed);

        printf("\n");
        printf("nRxUnicastPkts                         : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxUnicastPkts + sVar_rx[i].nRxUnicastPktsYellowRed);

        printf("\n");
        printf("nRxBroadcastPkts                       : ");
		for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxBroadcastPkts + sVar_rx[i].nRxBroadcastPktsYellowRed);

        printf("\n");
        printf("nRxMulticastPkts                       : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxMulticastPkts + sVar_rx[i].nRxMulticastPktsYellowRed);

        printf("\n");
        printf("nRxFCSErrorPkts                        : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxFCSErrorPkts);

        printf("\n");
        printf("nRxUnderSizeGoodPkts                   : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxUnderSizeGoodPkts);

        printf("\n");
        printf("nRxOversizeGoodPkts                    : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxOversizeGoodPkts);

        printf("\n");
        printf("nRxUnderSizeErrorPkts                  : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxUnderSizeErrorPkts);

        printf("\n");
        printf("nRxOversizeErrorPkts                   : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxOversizeErrorPkts);

        printf("\n");
        printf("nRxFilteredPkts                        : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxFilteredPkts);

        printf("\n");
        printf("nRx64BytePkts                          : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRx64BytePkts);

        printf("\n");
        printf("nRx127BytePkts                         : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRx127BytePkts);

        printf("\n");
        printf("nRx255BytePkts                         : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRx255BytePkts);

        printf("\n");
        printf("nRx511BytePkts                         : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRx511BytePkts);

        printf("\n");
        printf("nRx1023BytePkts                        : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRx1023BytePkts);

        printf("\n");
        printf("nRxMaxBytePkts                         : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxMaxBytePkts);

        printf("\n");
        printf("nRxDroppedPkts                         : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxDroppedPkts);

        printf("\n");
        printf("nRxExtendedVlanDiscardPkts             : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nRxExtendedVlanDiscardPkts);

        printf("\n");
        printf("nMtuExceedDiscardPkts                  : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", sVar_rx[i].nMtuExceedDiscardPkts);

        printf("\n");
        printf("nRxGoodBytes                           : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", (u32)(sVar_rx[i].nRxGoodBytes + sVar_rx[i].nRxGoodBytesYellowRed));

        printf("\n");
        printf("nRxBadBytes                            : ");
        for (i = 0; i <= max_read; i++)
            printf("%11u", (u32)sVar_rx[i].nRxBadBytes);

        printf("\n");
    }

    printf("\n");
    printf("nTxGoodPkts                            : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxGoodPkts + sVar_tx[i].nTxGoodPktsYellowRed);

    printf("\n");
    printf("nTxUnicastPkts                         : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxUnicastPkts + sVar_tx[i].nTxUnicastPktsYellowRed);

    printf("\n");
    printf("nTxBroadcastPkts                       : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxBroadcastPkts + sVar_tx[i].nTxBroadcastPktsYellowRed);

    printf("\n");
    printf("nTxMulticastPkts                       : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxMulticastPkts + sVar_tx[i].nTxMulticastPktsYellowRed);

    printf("\n");
    printf("nTx64BytePkts                          : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTx64BytePkts);

    printf("\n");
    printf("nTx127BytePkts                         : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTx127BytePkts);

    printf("\n");
    printf("nTx255BytePkts                         : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTx255BytePkts);

    printf("\n");
    printf("nTx511BytePkts                         : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTx511BytePkts);

    printf("\n");
    printf("nTx1023BytePkts                        : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTx1023BytePkts);

    printf("\n");
    printf("nTxMaxBytePkts                         : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxMaxBytePkts);

    printf("\n");
    printf("nTxDroppedPkts                         : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxDroppedPkts);

    printf("\n");
    printf("nTxOversizeGoodPkts                    : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxOversizeGoodPkts);

    printf("\n");
    printf("nTxUnderSizeGoodPkts                   : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxUnderSizeGoodPkts);

    printf("\n");
    printf("nTxAcmDroppedPkts                      : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", sVar_tx[i].nTxAcmDroppedPkts);

    printf("\n");
    printf("nTxGoodBytes                           : ");
    for (i = 0; i <= max_read; i++)
        printf("%11u", (u32)(sVar_tx[i].nTxGoodBytes + sVar_tx[i].nTxGoodBytesYellowRed));

    printf("\n");
    free(sVar_rx);

	return 0;
}

GSW_return_t fapi_GSW_CPU_PortCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_CPU_PortCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CPU_PortCfgGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PortCfgGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%s\n", "bCPU_PortValid", (param.bCPU_PortValid > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%s\n", "bSpecialTagIngress", (param.bSpecialTagIngress > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%s\n", "bSpecialTagEgress", (param.bSpecialTagEgress > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%s\n", "bFcsCheck", (param.bFcsCheck > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%s\n", "bFcsGenerate", (param.bFcsGenerate > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%d\n", "bSpecialTagEthType", param.bSpecialTagEthType);
        printf("\t%40s:\t%s\n", "bTsPtp", (param.bTsPtp > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%s\n", "bTsNonptp", (param.bTsNonptp > 0) ? "TRUE" : "FALSE");
        printf("\t%40s:\t%d\n", "eNoMPEParserCfg", param.eNoMPEParserCfg);
        printf("\t%40s:\t%d\n", "eMPE1ParserCfg", param.eMPE1ParserCfg);
        printf("\t%40s:\t%d\n", "eMPE2ParserCfg", param.eMPE2ParserCfg);
        printf("\t%40s:\t%d\n", "eMPE1MPE2ParserCfg", param.eMPE1MPE2ParserCfg);
    }

    return ret;
}

GSW_return_t fapi_GSW_CPU_PortGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_CPU_Port_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_CPU_Port_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CPU_PortGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_CPU_PortGet failed with ret code: %d\n", ret);
    else
    {
        printf("\tnPortId: %d\n", param.nPortId);
    }
    return ret;
}

GSW_return_t fapi_GSW_CPU_PortSet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_CPU_Port_t param = {0};
    int rret;
    uint8_t nEntryIndex = 0, nVal = 0, index;

    memset(&param, 0, sizeof(GSW_CPU_Port_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CPU_PortSet(gsw_dev, &param);

    if (ret < 0)
    {
        printf("fapi_GSW_CPU_PortSet failed with ret code: %d\n", ret);
        return ret;
    }
    return ret;
}

GSW_return_t fapi_GSW_CPU_PortCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_CPU_PortCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_CPU_PortCfgGet(gsw_dev, &param);
    if (ret < 0)
    {
        printf("\t%40s:\t0x%x\n", "GSW_CPU_PortCfgGet failed with ret code", ret);
        return ret;
    }

    scanParamArg(prmc, prmv, "bCPU_PortValid", sizeof(param.bCPU_PortValid), &param.bCPU_PortValid);
    scanParamArg(prmc, prmv, "bSpecialTagIngress", sizeof(param.bSpecialTagIngress), &param.bSpecialTagIngress);
    scanParamArg(prmc, prmv, "bSpecialTagEgress", sizeof(param.bSpecialTagEgress), &param.bSpecialTagEgress);
    scanParamArg(prmc, prmv, "bFcsCheck", sizeof(param.bFcsCheck), &param.bFcsCheck);
    scanParamArg(prmc, prmv, "bFcsGenerate", sizeof(param.bFcsGenerate), &param.bFcsGenerate);
    scanParamArg(prmc, prmv, "bSpecialTagEthType", sizeof(param.bSpecialTagEthType), &param.bSpecialTagEthType);
    scanParamArg(prmc, prmv, "bTsPtp", sizeof(param.bTsPtp), &param.bTsPtp);
    scanParamArg(prmc, prmv, "bTsNonptp", sizeof(param.bTsNonptp), &param.bTsNonptp);

    scanParamArg(prmc, prmv, "eNoMPEParserCfg", sizeof(param.eNoMPEParserCfg), &param.eNoMPEParserCfg);
    scanParamArg(prmc, prmv, "eMPE1ParserCfg", sizeof(param.eMPE1ParserCfg), &param.eMPE1ParserCfg);
    scanParamArg(prmc, prmv, "eMPE2ParserCfg", sizeof(param.eMPE2ParserCfg), &param.eMPE2ParserCfg);
    scanParamArg(prmc, prmv, "eMPE1MPE2ParserCfg", sizeof(param.eMPE1MPE2ParserCfg), &param.eMPE1MPE2ParserCfg);

    ret = GSW_CPU_PortCfgSet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_CPU_PortCfgSet failed with ret code", ret);
    else
    {
        printf("fapi_GSW_CPU_PortCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_VlanCounterMapSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VlanCounterMapping_config_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nCounterIndex", sizeof(sVar.nCounterIndex), &sVar.nCounterIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nCounterIndex\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nCtpPortId", sizeof(sVar.nCtpPortId), &sVar.nCtpPortId);
    scanParamArg(prmc, prmv, "bPriorityEnable", sizeof(sVar.bPriorityEnable), &sVar.bPriorityEnable);
    scanParamArg(prmc, prmv, "nPriorityVal", sizeof(sVar.nPriorityVal), &sVar.nPriorityVal);
    scanParamArg(prmc, prmv, "bVidEnable", sizeof(sVar.bVidEnable), &sVar.bVidEnable);
    scanParamArg(prmc, prmv, "nVidVal", sizeof(sVar.nVidVal), &sVar.nVidVal);
    scanParamArg(prmc, prmv, "bVlanTagSelectionEnable", sizeof(sVar.bVlanTagSelectionEnable), &sVar.bVlanTagSelectionEnable);
    scanParamArg(prmc, prmv, "eVlanCounterMappingType", sizeof(sVar.eVlanCounterMappingType), &sVar.eVlanCounterMappingType);
    scanParamArg(prmc, prmv, "eVlanCounterMappingFilterType", sizeof(sVar.eVlanCounterMappingFilterType), &sVar.eVlanCounterMappingFilterType);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_VlanCounterMapSet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_VlanCounterMapSet failed with ret code", ret);
    else
    {

        printf("fapi_GSW_VlanCounterMapSet done\n");
    }
    return ret;
}

GSW_return_t fapi_GSW_VlanCounterMapGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VlanCounterMapping_config_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nCounterIndex", sizeof(sVar.nCounterIndex), &sVar.nCounterIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nCounterIndex\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "eVlanCounterMappingType", sizeof(sVar.eVlanCounterMappingType), &sVar.eVlanCounterMappingType);
    if (rret < 1)
    {
        printf("Parameter not Found: eVlanCounterMappingType\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_VlanCounterMapGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_VlanCounterMapGet failed with ret code", ret);
    else
    {
        printf("\n\t nCounterIndex                              = %u", sVar.nCounterIndex);
        printf("\n\t nCtpPortId                         = %u", sVar.nCtpPortId);
        printf("\n\t bPriorityEnable                    = %u", sVar.bPriorityEnable);
        printf("\n\t nPriorityVal                       = %u", sVar.nPriorityVal);
        printf("\n\t bVidEnable                                 = %u", sVar.bVidEnable);
        printf("\n\t nVidVal                                    = %u", sVar.nVidVal);
        printf("\n\t bVlanTagSelectionEnable            = %u", sVar.bVlanTagSelectionEnable);
        printf("\n\t eVlanCounterMappingType            = %u", sVar.eVlanCounterMappingType);
        printf("\n\t eVlanCounterMappingFilterType      = %u", sVar.eVlanCounterMappingFilterType);
        printf("\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Vlan_RMON_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VLAN_RMON_cnt_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nVlanCounterIndex", sizeof(sVar.nVlanCounterIndex), &sVar.nVlanCounterIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nVlanCounterIndex\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "eVlanRmonType", sizeof(sVar.eVlanRmonType), &sVar.eVlanRmonType);
    if (rret < 1)
    {
        printf("Parameter not Found: eVlanRmonType\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_Vlan_RMON_Get(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Vlan_RMON_Get failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%lu\n", "nByteCount", (sVar.nByteCount));
        printf("\t%40s:\t%u\n", "nTotalPktCount", (sVar.nTotalPktCount));
        printf("\t%40s:\t%u\n", "nMulticastPktCount", (sVar.nMulticastPktCount));
        printf("\t%40s:\t%u\n", "nDropPktCount", (sVar.nDropPktCount));
    }

    return ret;
}

GSW_return_t fapi_GSW_Vlan_RMON_Clear(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VLAN_RMON_cnt_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nVlanCounterIndex", sizeof(sVar.nVlanCounterIndex), &sVar.nVlanCounterIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nVlanCounterIndex\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "eVlanRmonType", sizeof(sVar.eVlanRmonType), &sVar.eVlanRmonType);
    if (rret < 1)
    {
        printf("Parameter not Found: eVlanRmonType\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "eClearAll", sizeof(sVar.eVlanRmonType), &sVar.clear_all);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_Vlan_RMON_Clear(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Vlan_RMON_Clear failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Vlan_RMON_Clear done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Vlan_RMONControl_Set(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VLAN_RMON_control_t sVar = {0};

    scanParamArg(prmc, prmv, "bVlanRmonEnable", sizeof(sVar.bVlanRmonEnable), &sVar.bVlanRmonEnable);
    scanParamArg(prmc, prmv, "bIncludeBroadCastPktCounting", sizeof(sVar.bIncludeBroadCastPktCounting), &sVar.bIncludeBroadCastPktCounting);
    scanParamArg(prmc, prmv, "nVlanLastEntry", sizeof(sVar.nVlanLastEntry), &sVar.nVlanLastEntry);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_Vlan_RMONControl_Set(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Vlan_RMONControl_Set failed with ret code", ret);
    else
    {
        printf("fapi_GSW_Vlan_RMONControl_Set done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_Vlan_RMONControl_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_VLAN_RMON_control_t sVar = {0};

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_Vlan_RMONControl_Get(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Vlan_RMONControl_Get failed with ret code", ret);
    else
    {
        printf("\n\t bVlanRmonEnable                            = %u", sVar.bVlanRmonEnable);
        printf("\n\t bIncludeBroadCastPktCounting           = %u", sVar.bIncludeBroadCastPktCounting);
        printf("\n\t nVlanLastEntry                             = %u\n", sVar.nVlanLastEntry);
    }

    return ret;
}

GSW_return_t fapi_GSW_PBB_TunnelTempate_Config_Set(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PBB_Tunnel_Template_Config_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nTunnelTemplateId", sizeof(sVar.nTunnelTemplateId), &sVar.nTunnelTemplateId);
    if (rret < 1)
    {
        printf("Parameter not Found: nTunnelTemplateId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "bIheaderDstMACEnable", sizeof(sVar.bIheaderDstMACEnable), &sVar.bIheaderDstMACEnable);
    scanMAC_Arg(prmc, prmv, "nIheaderDstMAC", sVar.nIheaderDstMAC);
    scanParamArg(prmc, prmv, "bIheaderSrcMACEnable", sizeof(sVar.bIheaderSrcMACEnable), &sVar.bIheaderSrcMACEnable);
    scanMAC_Arg(prmc, prmv, "nIheaderSrcMAC", sVar.nIheaderSrcMAC);

    scanParamArg(prmc, prmv, "bItagEnable", sizeof(sVar.bItagEnable), &sVar.bItagEnable);
    scanParamArg(prmc, prmv, "bItagTpidEnable", sizeof(sVar.sItag.bTpidEnable), &sVar.sItag.bTpidEnable);
    scanParamArg(prmc, prmv, "nItagTpid", sizeof(sVar.sItag.nTpid), &sVar.sItag.nTpid);
    scanParamArg(prmc, prmv, "bItagPcpEnable", sizeof(sVar.sItag.bPcpEnable), &sVar.sItag.bPcpEnable);
    scanParamArg(prmc, prmv, "nItagPcp", sizeof(sVar.sItag.nPcp), &sVar.sItag.nPcp);
    scanParamArg(prmc, prmv, "bItagDeiEnable", sizeof(sVar.sItag.bDeiEnable), &sVar.sItag.bDeiEnable);
    scanParamArg(prmc, prmv, "nItagDei", sizeof(sVar.sItag.nDei), &sVar.sItag.nDei);
    scanParamArg(prmc, prmv, "bItagUacEnable", sizeof(sVar.sItag.bUacEnable), &sVar.sItag.bUacEnable);
    scanParamArg(prmc, prmv, "nItagUac", sizeof(sVar.sItag.nUac), &sVar.sItag.nUac);
    scanParamArg(prmc, prmv, "bItagResEnable", sizeof(sVar.sItag.bResEnable), &sVar.sItag.bResEnable);
    scanParamArg(prmc, prmv, "nItagRes", sizeof(sVar.sItag.nRes), &sVar.sItag.nRes);
    scanParamArg(prmc, prmv, "bItagSidEnable", sizeof(sVar.sItag.bSidEnable), &sVar.sItag.bSidEnable);
    scanParamArg(prmc, prmv, "nItagSid", sizeof(sVar.sItag.nSid), &sVar.sItag.nSid);

    scanParamArg(prmc, prmv, "bBtagEnable", sizeof(sVar.bBtagEnable), &sVar.bBtagEnable);
    scanParamArg(prmc, prmv, "bBtagTpidEnable", sizeof(sVar.sBtag.bTpidEnable), &sVar.sBtag.bTpidEnable);
    scanParamArg(prmc, prmv, "nBtagTpid", sizeof(sVar.sBtag.nTpid), &sVar.sBtag.nTpid);
    scanParamArg(prmc, prmv, "bBtagPcpEnable", sizeof(sVar.sBtag.bPcpEnable), &sVar.sBtag.bPcpEnable);
    scanParamArg(prmc, prmv, "nBtagPcp", sizeof(sVar.sBtag.nPcp), &sVar.sBtag.nPcp);
    scanParamArg(prmc, prmv, "bBtagDeiEnable", sizeof(sVar.sBtag.bDeiEnable), &sVar.sBtag.bDeiEnable);
    scanParamArg(prmc, prmv, "nBtagDei", sizeof(sVar.sBtag.nDei), &sVar.sBtag.nDei);
    scanParamArg(prmc, prmv, "bBtagVidEnable", sizeof(sVar.sBtag.bVidEnable), &sVar.sBtag.bVidEnable);
    scanParamArg(prmc, prmv, "nBtagVid", sizeof(sVar.sBtag.nVid), &sVar.sBtag.nVid);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PBB_TunnelTempate_Config_Set(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PBB_TunnelTempate_Config_Set failed with ret code", ret);
    else
    {
        printf("fapi_GSW_PBB_TunnelTempate_Config_Set done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PBB_TunnelTempate_Config_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PBB_Tunnel_Template_Config_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nTunnelTemplateId", sizeof(sVar.nTunnelTemplateId), &sVar.nTunnelTemplateId);
    if (rret < 1)
    {
        printf("Parameter not Found: nTunnelTemplateId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PBB_TunnelTempate_Config_Get(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PBB_TunnelTempate_Config_Get failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%u\n", "nTunnelTemplateId", sVar.nTunnelTemplateId);
        printf("\n\t nTunnelTemplateId						= %u", sVar.nTunnelTemplateId);
        printf("\n\t nIheaderDstMAC							=");
        printMAC_Address(sVar.nIheaderDstMAC);
        printf("\n\t nIheaderSrcMAC							=");
        printMAC_Address(sVar.nIheaderSrcMAC);
        printf("\n");
        printf("\t%40s:\t%x\n", "nItagTpid", sVar.sItag.nTpid);
        printf("\t%40s:\t%u\n", "nItagPcp", sVar.sItag.nPcp);
        printf("\t%40s:\t%u\n", "nItagDei", sVar.sItag.nDei);
        printf("\t%40s:\t%u\n", "nItagUac", sVar.sItag.nUac);
        printf("\t%40s:\t%u\n", "nItagRes", sVar.sItag.nRes);
        printf("\t%40s:\t%u\n", "nItagSid", sVar.sItag.nSid);
        printf("\t%40s:\t%x\n", "nBtagTpid", sVar.sBtag.nTpid);
        printf("\t%40s:\t%u\n", "nBtagPcp", sVar.sBtag.nPcp);
        printf("\t%40s:\t%u\n", "nBtagDei", sVar.sBtag.nDei);
        printf("\t%40s:\t%u\n", "nBtagVid", sVar.sBtag.nVid);
    }

    return ret;
}

GSW_return_t fapi_GSW_PBB_TunnelTempate_Free(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PBB_Tunnel_Template_Config_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nTunnelTemplateId", sizeof(sVar.nTunnelTemplateId), &sVar.nTunnelTemplateId);
    if (rret < 1)
    {
        printf("Parameter not Found: nTunnelTemplateId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PBB_TunnelTempate_Free(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PBB_TunnelTempate_Free failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "Freed nTunnelTemplateId", sVar.nTunnelTemplateId);
    }

    return ret;
}

GSW_return_t fapi_GSW_PBB_TunnelTempate_Alloc(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PBB_Tunnel_Template_Config_t sVar = {0};
    int rret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PBB_TunnelTempate_Alloc(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PBB_TunnelTempate_Alloc failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "Allocated nTunnelTemplateId", sVar.nTunnelTemplateId);
    }

    return ret;
}

GSW_return_t fapi_GSW_RMON_FlowGet(int prmc, char *prmv[])
{
    GSW_RMON_flowGet_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    scanParamArg(prmc, prmv, "bIndex", sizeof(param.bIndex), &param.bIndex);
    scanParamArg(prmc, prmv, "nIndex", sizeof(param.nIndex), &param.nIndex);
    scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    scanParamArg(prmc, prmv, "nFlowId", sizeof(param.nFlowId), &param.nFlowId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RMON_FlowGet(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RMON_FlowGet failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nIndex", param.nIndex);
        printf("\t%40s:\t%x\n", "nRxPkts", param.nRxPkts);
        printf("\t%40s:\t%x\n", "nTxPkts", param.nTxPkts);
        printf("\t%40s:\t%x\n", "nTxPceBypassPkts", param.nTxPceBypassPkts);
    }

    return ret;
}

GSW_return_t fapi_GSW_RMON_PortGet(int prmc, char *prmv[])
{
    GSW_RMON_Port_cnt_t sVar = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    scanParamArg(prmc, prmv, "nPortId", sizeof(sVar.nPortId), &sVar.nPortId);
    scanParamArg(prmc, prmv, "ePortType", sizeof(sVar.ePortType), &sVar.ePortType);
    scanParamArg(prmc, prmv, "nSubIfIdGroup", sizeof(sVar.nSubIfIdGroup), &sVar.nSubIfIdGroup);
    scanParamArg(prmc, prmv, "bPceBypass", sizeof(sVar.bPceBypass), &sVar.bPceBypass);

    if (sVar.ePortType != GSW_CTP_PORT && sVar.ePortType != GSW_BRIDGE_PORT)
        sVar.ePortType = GSW_CTP_PORT;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RMON_Port_Get(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RMON_PortGet failed with ret code", ret);
    else
    {
        printf("nPortId                                 : %d\n", sVar.nPortId);

        switch (sVar.ePortType)
        {
        case GSW_LOGICAL_PORT:
            printf("ePortType                               : GSW_LOGICAL_PORT\n");
            break;
        case GSW_PHYSICAL_PORT:
            printf("ePortType                               : GSW_PHYSICAL_PORT\n");
            break;
        case GSW_CTP_PORT:
            printf("ePortType                               : GSW_CTP_PORT\n");
            break;
        case GSW_BRIDGE_PORT:
            printf("ePortType                               : GSW_BRIDGE_PORT\n");
            break;
        default:
            printf("ePortType                               : UNKNOWN\n");
        }

        printf("nSubIfIdGroup                           : %d\n", sVar.nSubIfIdGroup);
        printf("bPceBypass                              : %d\n", sVar.bPceBypass);

        if (!sVar.bPceBypass)
        {
            printf("\n\n");
            printf("nRxGoodPkts 	                        : %u\n", sVar.nRxGoodPkts);
            printf("nRxUnicastPkts                          : %u\n", sVar.nRxUnicastPkts);
            printf("nRxBroadcastPkts                        : %u\n", sVar.nRxBroadcastPkts);
            printf("nRxMulticastPkts                        : %u\n", sVar.nRxMulticastPkts);
            printf("nRxFCSErrorPkts                         : %u\n", sVar.nRxFCSErrorPkts);
            printf("nRxUnderSizeGoodPkts                    : %u\n", sVar.nRxUnderSizeGoodPkts);
            printf("nRxOversizeGoodPkts                     : %u\n", sVar.nRxOversizeGoodPkts);
            printf("nRxUnderSizeErrorPkts                   : %u\n", sVar.nRxUnderSizeErrorPkts);
            printf("nRxOversizeErrorPkts                    : %u\n", sVar.nRxOversizeErrorPkts);
            printf("nRxFilteredPkts                         : %u\n", sVar.nRxFilteredPkts);
            printf("nRx64BytePkts                           : %u\n", sVar.nRx64BytePkts);
            printf("nRx127BytePkts                          : %u\n", sVar.nRx127BytePkts);
            printf("nRx255BytePkts                          : %u\n", sVar.nRx255BytePkts);
            printf("nRx511BytePkts                          : %u\n", sVar.nRx511BytePkts);
            printf("nRx1023BytePkts                         : %u\n", sVar.nRx1023BytePkts);
            printf("nRxMaxBytePkts                          : %u\n", sVar.nRxMaxBytePkts);
            printf("nRxDroppedPkts                          : %u\n", sVar.nRxDroppedPkts);
            printf("nRxExtendedVlanDiscardPkts              : %u\n", sVar.nRxExtendedVlanDiscardPkts);
            printf("nMtuExceedDiscardPkts                   : %u\n", sVar.nMtuExceedDiscardPkts);
            printf("nRxGoodBytes                            : %llu (0x%llx)\n", (unsigned long long)sVar.nRxGoodBytes, (unsigned long long)sVar.nRxGoodBytes);
            printf("nRxBadBytes                             : %llu (0x%llx)\n", (unsigned long long)sVar.nRxBadBytes, (unsigned long long)sVar.nRxBadBytes);
        }

        printf("\n\n");
        printf("nTxGoodPkts                             : %u\n", sVar.nTxGoodPkts);
        printf("nTxUnicastPkts                          : %u\n", sVar.nTxUnicastPkts);
        printf("nTxBroadcastPkts                        : %u\n", sVar.nTxBroadcastPkts);
        printf("nTxMulticastPkts                        : %u\n", sVar.nTxMulticastPkts);
        printf("nTx64BytePkts                           : %u\n", sVar.nTx64BytePkts);
        printf("nTx127BytePkts                          : %u\n", sVar.nTx127BytePkts);
        printf("nTx255BytePkts                          : %u\n", sVar.nTx255BytePkts);
        printf("nTx511BytePkts                          : %u\n", sVar.nTx511BytePkts);
        printf("nTx1023BytePkts                         : %u\n", sVar.nTx1023BytePkts);
        printf("nTxMaxBytePkts                          : %u\n", sVar.nTxMaxBytePkts);
        printf("nTxDroppedPkts                          : %u\n", sVar.nTxDroppedPkts);
        printf("nTxOversizeGoodPkts                     : %u\n", sVar.nTxOversizeGoodPkts);
        printf("nTxUnderSizeGoodPkts                    : %u\n", sVar.nTxUnderSizeGoodPkts);
        printf("nTxAcmDroppedPkts                       : %u\n", sVar.nTxAcmDroppedPkts);
        printf("nTxGoodBytes                            : %llu (0x%llx)\n", (unsigned long long)sVar.nTxGoodBytes, (unsigned long long)sVar.nTxGoodBytes);
    }

    return ret;
}

GSW_return_t fapi_GSW_RMON_ModeSet(int prmc, char *prmv[])
{
    GSW_RMON_mode_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "eRmonType", sizeof(param.eRmonType), &param.eRmonType);
    if (rret < 1)
    {
        printf("Parameter not Found: eRmonType\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "eCountMode", sizeof(param.eCountMode), &param.eCountMode);
    if (rret < 1)
    {
        printf("Parameter not Found: eCountMode\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RMON_Mode_Set(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RMON_ModeSet failed with ret code", ret);
    else
    {
        printf("\t%40s\n", "fapi_GSW_RMON_ModeSet done");
    }

    return ret;
}

GSW_return_t fapi_GSW_RMON_MeterGet(int prmc, char *prmv[])
{
    GSW_RMON_Meter_cnt_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    scanParamArg(prmc, prmv, "nMeterId", sizeof(param.nMeterId), &param.nMeterId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RMON_Meter_Get(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RMON_Meter_Get failed with ret code", ret);
    else
    {
        printf("\t%40s:\t%x\n", "nMeterId", param.nMeterId);
        printf("\t%40s:\t%x\n", "nGreenCount", param.nGreenCount);
        printf("\t%40s:\t%x\n", "nYellowCount", param.nYellowCount);
        printf("\t%40s:\t%x\n", "nRedCount", param.nRedCount);
        printf("\t%40s:\t%x\n", "nResCount", param.nResCount);
    }

    return ret;
}

GSW_return_t fapi_GSW_RMON_TFlowClear(int prmc, char *prmv[])
{
    GSW_RMON_flowGet_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    scanParamArg(prmc, prmv, "bIndex", sizeof(param.bIndex), &param.bIndex);
    scanParamArg(prmc, prmv, "nIndex", sizeof(param.nIndex), &param.nIndex);
    scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    scanParamArg(prmc, prmv, "nFlowId", sizeof(param.nFlowId), &param.nFlowId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_RmonTflowClear(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_RMON_TFlowClear failed with ret code", ret);
    else
    {
        printf("\t%40s\n", "fapi_GSW_RMON_TFlowClear done");
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgePortFree(int prmc, char *prmv[])
{
    GSW_BRIDGE_portAlloc_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    int rret;

    rret = scanParamArg(prmc, prmv, "nBridgePortId", sizeof(param.nBridgePortId), &param.nBridgePortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgePortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgePortFree(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgePortFree failed with ret code", ret);
    else
    {
        printf("\t%40s\n", "fapi_GSW_BridgePortFree done");
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgePortAlloc(int prmc, char *prmv[])
{
    GSW_BRIDGE_portAlloc_t param = {0};
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgePortAlloc(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_BridgePortAlloc failed with ret code", ret);
    else
    {
        printf("\n\tAllocated nBridgePortId = %u\n", param.nBridgePortId);
    }

    return ret;
}

GSW_return_t fapi_GSW_Freeze(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_Freeze(gsw_dev);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_Freeze failed with ret code", ret);
    else
    {
        printf("\t%40s\n", "fapi_GSW_Freeze done");
    }

    return ret;
}

GSW_return_t fapi_GSW_UnFreeze(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_UnFreeze(gsw_dev);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_UnFreeze failed with ret code", ret);
    else
    {
        printf("\t%40s\n", "fapi_GSW_UnFreeze done");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_MeterAlloc(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_meterCfg_t param = {0};

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_MeterAlloc(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_MeterAlloc failed with ret code", ret);
    else
    {
        printf("\n\tAllocated nMeterId = %u\n", param.nMeterId);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_MeterFree(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_meterCfg_t param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nMeterId", sizeof(param.nMeterId), &param.nMeterId);
    if (rret < 1)
    {
        printf("Parameter not Found: nMeterId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_MeterFree(gsw_dev, &param);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_QoS_MeterFree failed with ret code", ret);
    else
    {
        printf("fapi_GSW_QoS_MeterFree done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_PMAC_RMON_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PMAC_Cnt_t sVar = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "nPmacId", sizeof(sVar.nPmacId), &sVar.nPmacId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmacId\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "nPortId", sizeof(sVar.nTxDmaChanId), &sVar.nTxDmaChanId);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PMAC_CountGet(gsw_dev, &sVar);
    if (ret < 0)
        printf("\t%40s:\t0x%x\n", "fapi_GSW_PMAC_RMON_Get failed with ret code", ret);
    else
    {
        printf("\t nPmacId					= %u\n\n", sVar.nPmacId);
        printf("\t nPortId					= %u\n", sVar.nTxDmaChanId);
        printf("\t Egress Total Packet Count			= %u\n", sVar.nEgressPktsCount);
        printf("\t Egress Total Byte Count			= %u\n", sVar.nEgressBytesCount);
        printf("\t Egress Checksum Error Packet Count		= %u\n", sVar.nChkSumErrPktsCount);
        printf("\t Egress Checksum Error Byte Count		= %u\n", sVar.nChkSumErrBytesCount);
        printf("\t Egress Header Packet Count			= %u\n", sVar.nEgressHdrPktsCount);
        printf("\t Egress Header Byte Count			= %u\n", sVar.nEgressHdrBytesCount);
        printf("\t Egress Header Discard Packet Count		= %u\n", sVar.nEgressHdrDiscPktsCount);
        printf("\t Egress Header Discard Byte Count		= %u\n\n", sVar.nEgressHdrDiscBytesCount);
        printf("\t DMA TxCh					= %u\n", sVar.nTxDmaChanId);
        printf("\t Ingress Total Packet Count			= %u\n", sVar.nIngressPktsCount);
        printf("\t Ingress Total Byte Count			= %u\n", sVar.nIngressBytesCount);
        printf("\t Ingress Discard Packet Count			= %u\n", sVar.nDiscPktsCount);
        printf("\t Ingress Discard Byte Count			= %u\n", sVar.nDiscBytesCount);
        printf("\t Ingress Header Packet Count			= %u\n", sVar.nIngressHdrPktsCount);
        printf("\t Ingress Header Byte Count			= %u\n", sVar.nIngressHdrBytesCount);
    }

    return ret;
}

GSW_return_t fapi_GSW_Debug_PMAC_RMON_Get_All(int prmc, char *prmv[])
{
    GSW_PMAC_Cnt_t *eg, *ig;
    u32 i = 0;
    u32 start = 0, end = 16;
    u32 max_read = 0;
    u8 pmacId = 0;
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;

    scanParamArg(prmc, prmv, "nPmacId", sizeof(pmacId), &pmacId);
    scanParamArg(prmc, prmv, "Start", sizeof(start), &start);
    scanParamArg(prmc, prmv, "End", sizeof(end), &end);

    max_read = end - start;
    if (max_read > 16 || end > 16)
    {
        printf("Display only 16 ports, please check start and end\n");
        return -1;
    }

    eg = malloc(sizeof(*eg) * max_read);
    if (!eg)
    {
        printf("\n\tERROR: failed in buffer allocation\n");
        return -ENOMEM;
    }

    ig = eg;

    printf("\n");
    printf("Reading PmacId %d:  %s\n", pmacId, "Egress");
    printf("Reading PmacId %d:  %s\n", pmacId, "Ingress");
    gsw_dev = gsw_get_struc(lif_id, 0);

    for (i = 0; i < max_read; i++)
    {
        eg[i].nPmacId = pmacId;
        eg[i].nTxDmaChanId = start + i;

        ret = GSW_PMAC_CountGet(gsw_dev, &eg[i]);
        if (ret < 0)
        {
            free(eg);
            printf("\t%40s:\t0x%x\n", "GSW_PMAC_CountGet failed with ret code", ret);
            return ret;
        }
    }

    printf("\n");
    printf("Rx Logical Port                              : ");
    for (i = start; i < end; i++)
        printf("%11u", i);

    printf("\n");
    printf("\n");
    printf("Egress Checksum Error Packet Count           : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", eg[i].nChkSumErrPktsCount);

    printf("\n");
    printf("Egress Checksum Error Byte Count             : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", eg[i].nChkSumErrBytesCount);

    printf("\n");
    printf("Egress Total Packet Count                    : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", eg[i].nEgressPktsCount);

    printf("\n");
    printf("Egress Total Byte Count                      : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", eg[i].nEgressBytesCount);

    printf("\n");
    printf("\n");
    printf("\n");

    printf("DMA TxCh                                     : ");
    for (i = start; i < end; i++)
        printf("%11u", i);

    printf("\n");
    printf("\n");

    printf("Ingress Discard Packet Count                 : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", ig[i].nDiscPktsCount);

    printf("\n");
    printf("Ingress Discard Byte Count                   : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", ig[i].nDiscBytesCount);

    printf("\n");
    printf("Ingress Total Packet Count                   : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", ig[i].nIngressPktsCount);

    printf("\n");
    printf("Ingress Total Byte Count                     : ");

    for (i = 0; i < max_read; i++)
        printf("%11u", ig[i].nIngressBytesCount);
    printf("\n");
    free(eg);

    return ret;
}

GSW_return_t fapi_GSW_SS_Sptag_Get(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    struct gsw_ss_sptag param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "pid", sizeof(param.pid), &param.pid);
    if (rret < 1)
    {
        printf("Parameter not Found: pid\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = gsw_ss_sptag_get(gsw_dev, &param);
    if (ret < 0)
        printf("fapi_GSW_SS_Sptag_Get failed with ret code: %d\n", ret);
    else
    {
        printf("fapi_GSW_SS_Sptag_Get:\n");
        printf("\t%40s:\t%d\n", "pid", param.pid);
        printf("\t%40s:\t%d\n", "rx", param.rx);
        printf("\t%40s:\t%d\n", "tx", param.tx);
        printf("\t%40s:\t%d\n", "rx_pen", param.rx_pen);
        printf("\t%40s:\t%d\n", "tx_pen", param.tx_pen);
    }

    return ret;
}

GSW_return_t fapi_GSW_SS_Sptag_Set(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    struct gsw_ss_sptag param = {0};
    int rret;

    rret = scanParamArg(prmc, prmv, "pid", sizeof(param.pid), &param.pid);
    if (rret < 1)
    {
        printf("Parameter not Found: pid\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "mask", sizeof(param.mask), &param.mask);
    if (rret < 1)
    {
        printf("Parameter not Found: mask\n");
        return OS_ERROR;
    }

    scanParamArg(prmc, prmv, "rx", sizeof(param.rx), &param.rx);
    scanParamArg(prmc, prmv, "tx", sizeof(param.tx), &param.tx);
    scanParamArg(prmc, prmv, "rx_pen", sizeof(param.rx_pen), &param.rx_pen);
    scanParamArg(prmc, prmv, "tx_pen", sizeof(param.tx_pen), &param.tx_pen);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = gsw_ss_sptag_set(gsw_dev, &param);
    if (ret < 0)
        printf("fapi_GSW_SS_Sptag_Set failed with ret code: %d\n", ret);
    else
    {
        printf("fapi_GSW_SS_Sptag_Set done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_DSCP_DropPrecedenceCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_DSCP_DropPrecedenceCfg_t param = {0};
    int i;

    memset(&param, 0, sizeof(GSW_QoS_DSCP_DropPrecedenceCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_DSCP_DropPrecedenceCfgGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_WredQueueCfgGet failed with ret code: %d\n", ret);
    else
    {
        printf("GSW QOS DSCP drop precedence configuration:\n");
        printf("\tIndex\t:\tPrecedence\n");
        for (i = 0; i < 64; i++)
            printf("\t%d\t:\t%d\n", i, param.nDSCP_DropPrecedence[0]);
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_DSCP_DropPrecedenceCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_DSCP_DropPrecedenceCfg_t param = {0};
    int rret;
    uint8_t index, Val;

    memset(&param, 0, sizeof(GSW_QoS_DSCP_DropPrecedenceCfg_t));

    rret = scanParamArg(prmc, prmv, "nIndex", 1, &index);
    if (rret < 1)
    {
        printf("Parameter not Found: nIndex\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nVal", 1, &Val);
    if (rret < 1)
    {
        printf("Parameter not Found: nVal\n");
        return OS_ERROR;
    }

    if (index >= 64)
    {
        printf("Wrong nIndex value: %d, should be in range [0 : 63]\n", index);
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_DSCP_DropPrecedenceCfgGet(gsw_dev, &param);
    if (ret < 0) {
        printf("Initial fapi_GSW_QoS_WredQueueCfgGet failed with ret code: %d\n", ret);
        return ret;
    }

    param.nDSCP_DropPrecedence[index] = Val;
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_DSCP_DropPrecedenceCfgSet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_DSCP_DropPrecedenceCfgSet failed with ret code: %d\n", ret);
    else
    {
        printf("fapi_GSW_QoS_DSCP_DropPrecedenceCfgSet done\n");
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_ColorMarkingTableGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_colorMarkingEntry_t param = {0};
    int rret;
    uint8_t index = 0, max_index = 0;
    uint8_t priority = 0, color = 0;

    memset(&param, 0, sizeof(GSW_QoS_colorMarkingEntry_t));

    rret = scanParamArg(prmc, prmv, "eMode", sizeof(param.eMode), &param.eMode);
    if (rret < 1)
    {
        printf("Parameter not Found: eMode\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_ColorMarkingTableGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_ColorMarkingTableGet failed with ret code: %d\n", ret);
    else
    {
        switch (param.eMode)
        {
        case GSW_MARKING_ALL_GREEN: // 0
            printf("All Green Color Mode.\n");
            max_index = 0;
            break;
        case GSW_MARKING_INTERNAL_MARKING: // 1
            printf("Internal Marking Mode.\n");
            max_index = 0;
            break;
        case GSW_MARKING_DEI: // 2
            printf("DEI Mode\n");
            max_index = 0;
            break;
        case GSW_MARKING_PCP_8P0D:
        case GSW_MARKING_PCP_7P1D:
        case GSW_MARKING_PCP_6P2D:
        case GSW_MARKING_PCP_5P3D:
            printf("\tPCP Index : Priority : Color\n");
            max_index = 16;
            break;
        case GSW_MARKING_DSCP_AF:
            printf("\tDSCP Index : Priority : Color\n");
            max_index = 64;
        default:
            printf("Not Supported Mode (%d) of Color.\n", param.eMode);
            break;
        }

        for (index = 0; index < max_index; index++)
        {
            priority = param.nPriority[index];
            color = param.nColor[index];
            printf("\t      %2d  :     %2d     : %2d\n", index, priority, color);
        }
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_ColorMarkingTableSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_QoS_colorMarkingEntry_t param = {0};
    int rret;
    uint8_t index;
    uint8_t priority = 0, color = 0;

    memset(&param, 0, sizeof(GSW_QoS_colorMarkingEntry_t));

    rret = scanParamArg(prmc, prmv, "eMode", sizeof(param.eMode), &param.eMode);
    if (rret < 1)
    {
        printf("Parameter not Found: eMode\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nIndex", 1, &index);
    if (rret < 1)
    {
        printf("Parameter not Found: nIndex\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nPriority", 1, &priority);
    if (rret < 1)
    {
        printf("Parameter not Found: nPriority\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nColor", 1, &color);
    if (rret < 1)
    {
        printf("Parameter not Found: nColor\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_ColorMarkingTableGet(gsw_dev, &param);

    if (ret < 0)
    {
        printf("Initial fapi_GSW_QoS_ColorMarkingTableGet failed with ret code: %d\n", ret);
        return ret;
    }

    if ((param.eMode >= GSW_MARKING_PCP_8P0D) && (param.eMode <= GSW_MARKING_DSCP_AF))
    {
        if (param.eMode != GSW_MARKING_DSCP_AF)
        {
            if (index >= 16)
            {
                printf("nIndex (%d) is out of range (0~15)\n", index);
                return OS_ERROR;
            }
        }
        else
        {
            if (index >= 64)
            {
                printf("nIndex (%d) is out of range (0~63)\n", index);
                return OS_ERROR;
            }
        }
        priority &= 0x7;
        color &= 0x3;
        param.nPriority[index] = priority;
        param.nColor[index] = color;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_ColorMarkingTableSet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_ColorMarkingTableSet failed with ret code: %d\n", ret);
    else
    {
        printf("fapi_GSW_QoS_ColorMarkingTableSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ColorReMarkingTableGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_colorRemarkingEntry_t param = {0};
    int rret;
    uint8_t mode = GSW_REMARKING_NONE;
    uint8_t dei = 0, pcp = 0;
    uint8_t dscp = 0;

    memset(&param, 0, sizeof(GSW_QoS_colorRemarkingEntry_t));

    rret = scanParamArg(prmc, prmv, "eMode", sizeof(param.eMode), &param.eMode);
    if (rret < 1)
    {
        printf("Parameter not Found: eMode\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_ColorReMarkingTableGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_ColorReMarkingTableGet failed with ret code:%d\n", ret);
    else
    {
        printf("Mode of color remarking: %d\n", param.eMode);
        switch (param.eMode)
        {
        case GSW_REMARKING_NONE:
            printf("None Remarking Mode of Color.\n");
            break;

        case GSW_REMARKING_DEI:
            printf("DEI Mode of Color Remarking.\n");
            break;

        case GSW_REMARKING_PCP_8P0D:
        /* For mode 3 there are 16 entries corresponding to Priority + Color Bit Table Entry index from 0 to 15	        */
        case GSW_REMARKING_PCP_7P1D:
        /* For mode 4 there are 16 entries corresponding to Priority + Color Bit Table Entry index from 16 to 31        */
        case GSW_REMARKING_PCP_6P2D:
        /* For mode 5 there are 16 entries corresponding to Priority + Color Bit Table Entry index from 32 to 47	    */
        case GSW_REMARKING_PCP_5P3D:
            /* For mode 6 there are 16 entries corresponding to Priority + Color Bit Table Entry index from 48 to 63	*/

            printf("\tIndex : DEI : PCP\n");
            /* Get Color Bit 0's entries from 0 t0 7, Color Bit 1's entries from 8 t0 15*/
            for (uint8_t index = 0; index <= 15; index++)
            {
                if (index == 0)
                    printf("Green -----------------------\n");
                else if (index == 8)
                    printf("Yellow-----------------------\n");

                /*Get DEI in PCE_TBL_VAL (bit 0)*/
                dei = param.nVal[index] & 0x1;
                /*Get PCP in PCE_TBL_VAL (bit 3:1)*/
                pcp = (param.nVal[index] & 0xE) >> 1;
                printf("\t%2d    : %d   : %3d", index, dei, pcp);
                printf("    (0x%x)", param.nVal[index]); // Only for Debug
                printf("\n");
            }

            break;

        case GSW_REMARKING_DSCP_AF:
            /* For mode 7 there are 16 entries corresponding to Traffic Class + Color Bit Table Entry index from 64 to 79*/

            printf("\tIndex : DSCP\n");
            /*Get Color Bit 0's entries from 0 t0 7 Color Bit 1's entries from 8 t0 15*/
            for (uint8_t index = 0; index <= 15; index++)
            {
                if (index == 0)
                    printf("Green -----------------------\n");
                else if (index == 8)
                    printf("Yellow-----------------------\n");

                /*Get DSCP in PCE_TBL_VAL (bit 5:0)*/
                dscp = param.nVal[index] & 0x3F;
                printf("\t%2d    : %d", index, dscp);
                printf("    (0x%x)", param.nVal[index]); // Only for Debug
                printf("\n");
            }

            break;
        default:
            printf("Not Supported Mode (%d) of Color.\n", param.eMode);
            break;
        }
        return ret;
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_DSCP2_PCPTableGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_DSCP2PCP_map_t param = {0};
    int rret;
    uint8_t index;

    memset(&param, 0, sizeof(GSW_DSCP2PCP_map_t));

    rret = scanParamArg(prmc, prmv, "nIndex", sizeof(param.nIndex), &param.nIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nIndex\n");
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_Dscp2PcpTableGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_DSCP2_PCPTableGet failed with ret code: %d\n", ret);
    else
    {
        printf("\tDSCP2PCP Mapping Table Index : %d\n", param.nIndex);
        printf("\tDSCP2PCP index : PCP value\n");
        for (index = 0; index < 64; index++)
        {
            printf("\t\t%2d     :     %d\n", index, param.nMap[index]);
        }
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_DSCP2_PCPTableSet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_DSCP2PCP_map_t param = {0};
    int rret;
    uint8_t nDscpIndex = 0, nVal = 0;

    memset(&param, 0, sizeof(GSW_DSCP2PCP_map_t));

    rret = scanParamArg(prmc, prmv, "nIndex", sizeof(param.nIndex), &param.nIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nIndex\n");
        return OS_ERROR;
    }
    if (param.nIndex > 7)
    {
        printf("nIndex (%d) is out of range (0~7)\n", param.nIndex);
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nDscpIndex", 1, &nDscpIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nDscpIndex\n");
        return OS_ERROR;
    }
    if (nDscpIndex > 63)
    {
        printf("nDscpIndex (%d) is out of range (0~64)\n", nDscpIndex);
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nVal", 1, &nVal);
    if (rret < 1)
    {
        printf("Parameter not Found: nVal\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_Dscp2PcpTableGet(gsw_dev, &param);

    if (ret < 0)
    {
        printf("Initial GSW_QOS_Dscp2PcpTableGet failed with ret code: %d\n", ret);
        return ret;
    }
    else
    {
        param.nMap[nDscpIndex] = nVal;
        gsw_dev = gsw_get_struc(lif_id, 0);
        ret = GSW_QOS_Dscp2PcpTableSet(gsw_dev, &param);
        if (ret < 0)
        {
            printf("GSW_QOS_Dscp2PcpTableSet failed with ret code: %d\n", ret);
            return ret;
        }
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_ColorReMarkingTableSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_QoS_colorRemarkingEntry_t param = {0};
    int rret;
    uint8_t dei = 0, pcp = 0;
    uint8_t index = 0;
    uint8_t mode = GSW_REMARKING_NONE;

    memset(&param, 0, sizeof(GSW_QoS_colorRemarkingEntry_t));

    rret = scanParamArg(prmc, prmv, "eMode", sizeof(param.eMode), &param.eMode);
    if (rret < 1)
    {
        printf("Parameter not Found: eMode\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "nIndex", 1, &index);
    if (rret < 1)
    {
        printf("Parameter not Found: nIndex\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_ColorReMarkingTableGet(gsw_dev, &param);

    if (ret < 0)
    {
        printf("Initial fapi_GSW_QoS_ColorReMarkingTableSet failed with ret code: %d\n", ret);
        return ret;
    }
    if (param.eMode == GSW_REMARKING_DSCP_AF)
    {

        uint8_t dscp_val = 0;

        rret = scanParamArg(prmc, prmv, "dscp", 1, &dscp_val);
        if (rret < 1)
        {
            printf("Parameter not Found: dscp\n");
            return OS_ERROR;
        }

        scanParamArg(prmc, prmv, "dscp_val", 1, &dscp_val);
        printf("Color Remarking Table Index %d DSCP value Before Changing is %d\n", index, param.nVal[index]);
        param.nVal[index] = dscp_val;
        printf("Color Remarking Table Index %d DSCP value Will be Changed to %d\n", index, dscp_val);
    }
    else
    { // one of GSW_REMARKING_PCP_8P0D, GSW_REMARKING_PCP_7P1D, GSW_REMARKING_PCP_6P2D and GSW_REMARKING_PCP_5P3D

        uint8_t dei = 0, pcp = 0;

        printf("Dbg: Index %d Value read before change: 0x%x\n", index, param.nVal[index]);
        dei = param.nVal[index] & 0x1;
        pcp = (param.nVal[index] & 0xE) >> 1;
        printf("Color Remarking Table Index %d \n\tDEI value Before Changing %d and PCP value Before Changing %d\n", index, dei, pcp);

        rret = scanParamArg(prmc, prmv, "dei", 1, &dei);
        if (rret < 1)
        {
            printf("Parameter not Found: dei\n");
            return OS_ERROR;
        }
        rret = scanParamArg(prmc, prmv, "pcp", 1, &pcp);
        if (rret < 1)
        {
            printf("Parameter not Found: pcp\n");
            return OS_ERROR;
        }

        dei &= 0x1; // DEI is 1 bit value at bit 0
        pcp &= 0x7; // PCP is 3 bits value at bit 1~3
        printf("Color Remarking Table Index %d \n\tDEI value will be changed to %d and PCP value will be changed to %d\n", index, dei, pcp);
        param.nVal[index] = ((pcp << 1) & 0xE) | dei;
        printf("Dbg: Index %d Value changed before writting: 0x%x\n", index, param.nVal[index]);
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_ColorReMarkingTableSet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_ColorReMarkingTableSet failed with ret code: %d\n", ret);
    else
    {
        printf("fapi_GSW_QoS_ColorMarkingTableSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_PortReMarkingCfgGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_portRemarkingCfg_t param;
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_portRemarkingCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_PortRemarkingCfgGet(gsw_dev, &param);

    /* TODO check agains total number of ports including vitual ports would require gsw_data from gsw_priv.h */
    if (param.nPortId >= 16)
    {
        printf("nPortId more than total number of ports\n");
        return GSW_statusErr;
    }

    if (ret < 0)
        printf("fapi_GSW_QoS_PortReMarkingCfgGet failed with ret code: %d\n", ret);
    else
    {
        switch (param.eDSCP_IngressRemarkingEnable)
        {
        case GSW_DSCP_REMARK_DISABLE:
            printf("\tDSCP_REMARK DISABLE: No DSCP Remarking is done on the egress port.\n");
            break;
        case GSW_DSCP_REMARK_TC6:
            printf("\tDSCP_REMARK_TC6: TC DSCP 6-Bit Remarking.\n");
            break;
        case GSW_DSCP_REMARK_TC3:
            printf("\tDSCP_REMARK_TC3: TC DSCP 3-Bit Remarking.\n");
            break;
        case GSW_DSCP_REMARK_DP3:
            printf("\tDSCP_REMARK_DP3: Drop Precedence Remarking.\n");
            break;
        case GSW_DSCP_REMARK_DP3_TC3:
            printf("\tDSCP_REMARK_DP3_TC3: Drop Precedence Remarking and the upper 3-Bits of the DSCP field are remarked based on the traffic class.\n");
            break;
        default:
            printf("Wrong DSCP Ingress Remarking Enable Value.\n");
            break;
        }

        if (param.bDSCP_EgressRemarkingEnable)
            printf("\tEgress DSCP Remarking: Enabled.\n");
        else
            printf("\tEgress DSCP Remarking: Disabled.\n");

        if (param.bPCP_IngressRemarkingEnable)
            printf("\tIngress PCP Remarking: Enabled.\n");
        else
            printf("\tIngress PCP Remarking: Disabled.\n");

        if (param.bPCP_EgressRemarkingEnable)
            printf("\tEgress PCP Remarking: Enabled.\n");
        else
            printf("\tEgress PCP Remarking: Disabled.\n");

        if (param.bSTAG_PCP_IngressRemarkingEnable)
            printf("\tIngress STAG VLAN PCP Remarking: Enabled.\n");
        else
            printf("\tIngress STAG VLAN PCP Remarking: Disabled.\n");

        if (param.bSTAG_DEI_IngressRemarkingEnable)
            printf("\tIngress STAG VLAN DEI Remarking: Enabled.\n");
        else
            printf("\tIngress STAG VLAN DEI Remarking: Disabled.\n");

        if (param.bSTAG_PCP_DEI_EgressRemarkingEnable)
            printf("\tEgress STAG VLAN PCP & DEI Remarking: Enabled.\n");
        else
            printf("\tEgress STAG VLAN PCP & DEI Remarking: Disabled\n");
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_PortReMarkingCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_QoS_portRemarkingCfg_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_QoS_portRemarkingCfg_t));

    rret = scanParamArg(prmc, prmv, "nPortId", sizeof(param.nPortId), &param.nPortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortId\n");
        return OS_ERROR;
    }

    /* TODO check agains total number of ports including vitual ports would require gsw_data from gsw_priv.h */
    if (param.nPortId >= 16)
    {
        printf("nPortId more than total number of ports\n");
        return GSW_statusErr;
    }

    rret = scanParamArg(prmc, prmv, "bDscpIngrEn", sizeof(param.eDSCP_IngressRemarkingEnable), &param.eDSCP_IngressRemarkingEnable);
    if (rret < 1)
    {
        printf("Parameter not Found: nIndex\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "bDscpEgrEn", sizeof(param.bDSCP_EgressRemarkingEnable), &param.bDSCP_EgressRemarkingEnable);
    if (rret < 1)
    {
        printf("Parameter not Found: bDscpEgrEn\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "bPcpIngrEn", sizeof(param.bPCP_IngressRemarkingEnable), &param.bPCP_IngressRemarkingEnable);
    if (rret < 1)
    {
        printf("Parameter not Found: bPcpIngrEn\n");
        return OS_ERROR;
    }

    param.eDSCP_IngressRemarkingEnable = param.eDSCP_IngressRemarkingEnable % 2;
    param.bDSCP_EgressRemarkingEnable = param.bDSCP_EgressRemarkingEnable % 2;
    param.bPCP_IngressRemarkingEnable = param.bPCP_IngressRemarkingEnable % 2;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_PortRemarkingCfgSet(gsw_dev, &param);

    if (ret < 0)
        printf("GSW_QoS_PortRemarkingCfgSet failed with ret code: %d\n", ret);
    else
    {
        printf("GSW_QoS_PortRemarkingCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_StormCfgGet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_QoS_stormCfg_t param;

    memset(&param, 0, sizeof(GSW_QoS_stormCfg_t));

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_StormCfgGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_StormCfgGet failed with ret code: %d\n", ret);
    else
    {
        printf("\tStorm Control Meter ID: %d\n", param.nMeterId);
        printf("\tStorm Control Broadcast: %s\n", param.bBroadcast ? "Enabled" : "Disabled");
        printf("\tStorm Control Multicast: %s\n", param.bMulticast ? "Enabled" : "Disabled");
        printf("\tStorm Control Unknowcast: %s\n", param.bUnknownUnicast ? "Enabled" : "Disabled");
    }
    return ret;
}

GSW_return_t fapi_GSW_QoS_StormCfgSet(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_QoS_stormCfg_t param = {0};
    int rret;
    bool en_flg;
    memset(&param, 0, sizeof(GSW_QoS_stormCfg_t));

    rret = scanParamArg(prmc, prmv, "bEn", 1, &en_flg);
    if (rret < 1)
    {
        printf("Parameter not Found: bEn\n");
        return OS_ERROR;
    }

    en_flg &= 0x1;
    param.bBroadcast = en_flg;
    param.bMulticast = en_flg;
    param.bUnknownUnicast = en_flg;

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QoS_StormCfgSet(gsw_dev, &param);

    if (ret < 0)
        printf("GSW_QoS_StormCfgSet failed with ret code: %d\n ", ret);
    else
    {
        printf("GSW_QoS_StormCfgSet done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_QoS_PmapperTableGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PMAPPER_t param = {0};
    int rret;
    uint8_t index = 0;

    memset(&param, 0, sizeof(GSW_PMAPPER_t));

    rret = scanParamArg(prmc, prmv, "nPmapperId", sizeof(param.nPmapperId), &param.nPmapperId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmapperId\n");
        return OS_ERROR;
    }
    if (param.nPmapperId > 31)
    {
        printf("nPmapperId (%d) is out of range (0~31)\n", param.nPmapperId);
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_PmapperTableGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_QoS_PmapperTableGet failed with ret code. P-Mapper ID invalid\n");
    else
    {
        printf("\tP-Mapper Index: %d\n", param.nPmapperId);
        printf("\tEntryIndex : DestSubIfIdGroup\n");
        for (index = 0; index < 73; index++)
        {
            printf("\t %2d         : 0x%x\n", index, param.nDestSubIfIdGroup[index]);
        }
    }
    return ret;    
}

GSW_return_t fapi_GSW_QoS_PmapperTableSet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PMAPPER_t param = {0};
    int rret;
    uint8_t nEntryIndex = 0, nVal = 0, index;

    memset(&param, 0, sizeof(GSW_PMAPPER_t));

    rret = scanParamArg(prmc, prmv, "nPmapperId", sizeof(param.nPmapperId), &param.nPmapperId);
    if (rret < 1)
    {
        printf("Parameter not Found: nPmapperId\n");
        return OS_ERROR;
    }
    if (param.nPmapperId > 31)
    {
        printf("nPmapperId (%d) is out of range (0~31)\n", param.nPmapperId);
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nEntryIndex", 1, &nEntryIndex);
    if (rret < 1)
    {
        printf("Parameter not Found: nEntryIndex\n");
        return OS_ERROR;
    }
    if (nEntryIndex > 72)
    {
        printf("nEntryIndex (%d) is out of range (0~72)\n", nEntryIndex);
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nVal", 1, &nVal);
    if (rret < 1)
    {
        printf("Parameter not Found: nVal\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_QOS_PmapperTableGet(gsw_dev, &param);

    if (ret < 0)
    {
        /*P-Mapper ID are to be added consecutively.
        If P-Mapper ID is invalid ,find a free P-Mapper table index and allocate
        New P-Mapper configuration table index*/
        printf("Selected nPmapperId (%d) is NOT in use, set nPmapperId to invalid to get a free one.\n", param.nPmapperId);
        param.nPmapperId = 0xFFFF;
        for (index = 0; index < 73; index++)
        {
            if (index == nEntryIndex)
                param.nDestSubIfIdGroup[nEntryIndex] = nVal;
            else
                param.nDestSubIfIdGroup[nEntryIndex] = 0;
        }
        gsw_dev = gsw_get_struc(lif_id, 0);
        ret = GSW_QOS_PmapperTableSet(gsw_dev, &param);
        if (ret < 0)
        {
            printf("GSW_QOS_PmapperTableSet failed with ret code %d\n", ret);
            return ret;
        }
        return ret;
    }
    else
    {
        param.nDestSubIfIdGroup[nEntryIndex] = nVal;
        gsw_dev = gsw_get_struc(lif_id, 0);
        ret = GSW_QOS_PmapperTableSet(gsw_dev, &param);
        if (ret < 0)
        {
            printf("GSW_QOS_PmapperTableSet failed with ret code %d\n", ret);
            return ret;
        }
        return ret;
    }
}

GSW_return_t fapi_GSW_Pce_RuleBlockSize(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_PCE_rule_alloc_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_PCE_rule_alloc_t));
    rret = scanParamArg(prmc, prmv, "blockid", sizeof(param.blockid), &param.blockid);
    if (rret < 1)
    {
        printf("Parameter not Found: blockid\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleBlockSize(gsw_dev, &param);

    if (ret < 0)
        printf("GSW_PceRuleBlockSize failed with ret code %d\n", ret);
    else
    {
        printf("ret          = %d\n", ret);
        printf("blockid      = %u\n", param.blockid);
        printf("num_of_rules = %u\n", param.num_of_rules);
        printf("GSW_PceRuleBlockSize done\n");
    }

    return ret;
}

GSW_return_t fapi_GSW_BridgePort_LoopRead(int prmc, char *prmv[])
{
    GSW_Device_t *gsw_dev;
    GSW_return_t ret = 0;
    GSW_BRIDGE_portLoopRead_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_BRIDGE_portLoopRead_t));
    rret = scanParamArg(prmc, prmv, "nBridgePortId", sizeof(param.nBridgePortId), &param.nBridgePortId);
    if (rret < 1)
    {
        printf("Parameter not Found: nBridgePortId\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_BridgePortLoopRead(gsw_dev, &param);

    if (ret < 0)
        printf("GSW_BridgePortLoopRead failed with ret code: %d", ret);
    else
    {
        printf("BridgePortId       = %u\n", param.nBridgePortId);
        printf("nLoopViolationCount = %u\n", param.nLoopViolationCount);
    }
    return ret;
}

GSW_return_t fapi_GSW_TflowCountModeGet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_TflowCmodeConf_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_TflowCmodeConf_t));

    rret = scanParamArg(prmc, prmv, "eCntType", sizeof(param.eCountType), &param.eCountType);
    if (rret < 1)
    {
        printf("Parameter not Found: eCntType\n");
        return OS_ERROR;
    }
    if (param.eCountType > GSW_TFLOW_COUNTER_PCE_BP_Tx)
    {
        printf("eCntType (%d) is out of range (0~3)\n", param.eCountType);
        return OS_ERROR;
    }
    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_TflowCountModeGet(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_TflowCountModeGet failed with ret code: %d\n", ret);
    else
    {
        switch (param.eCountType)
        {
        case GSW_TFLOW_COUNTER_ALL:
        case GSW_TFLOW_COUNTER_PCE_Rx:
            // 1.PCE TFLOW Rx config.
            // Read PCE TFLOW Rx RMON register.
            printf("RX TFlow Counter:\n");
            break;

        case GSW_TFLOW_COUNTER_PCE_Tx:
            // 2.PCE TFLOW Tx config
            // Read PCE TFLOW Tx RMON register.
            printf("TX TFlow Counter:\n");
            break;

        case GSW_TFLOW_COUNTER_PCE_BP_Tx:
            // 3.PCE-Bypass TFLOW Tx config
            // Write PCE-Bypass TFLOW Tx RMON register.
            printf("TX BP TFlow Counter:\n");
            break;

        default:
            /* Invalid input */
            printf("Invalid eCntType %d!\n", param.eCountType);
            return GSW_statusErr;
            break;
        }

        printf("\teCountMode: %d\n", param.eCountMode);
        if (param.eCountMode == GSW_TFLOW_CMODE_BRIDGE)
            printf("\tnBrpLsb: %d\n", param.nBrpLsb);
        else if (param.eCountMode == GSW_TFLOW_CMODE_CTP)
            printf("\tnCtpLsb: %d\n", param.nCtpLsb);
        else if (param.eCountMode == GSW_TFLOW_CMODE_LOGICAL)
            printf("\tLogical Port Mode.\n");
        else
            printf("\tGlobal Mode.\n");
        printf("\tnPortMsb: %d\n", param.nPortMsb);
    }
    return ret;
}

GSW_return_t fapi_GSW_TflowCountModeSet(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_TflowCmodeConf_t param = {0};
    int rret;

    memset(&param, 0, sizeof(GSW_TflowCmodeConf_t));

    rret = scanParamArg(prmc, prmv, "eCntType", sizeof(param.eCountType), &param.eCountType);
    if (rret < 1)
    {
        printf("Parameter not Found: eCntType\n");
        return OS_ERROR;
    }
    if (param.eCountType > GSW_TFLOW_COUNTER_PCE_BP_Tx)
    {
        printf("eCntType (%d) is out of range (0~3)\n", param.eCountType);
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "eCntMode", sizeof(param.eCountMode), &param.eCountMode);
    if (rret < 1)
    {
        printf("Parameter not Found: eCntMode\n");
        return OS_ERROR;
    }
    if (param.eCountMode > 3)
    {
        printf("eCountMode (%d) is out of range (0~3)\n", param.eCountMode);
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nPortMsb", sizeof(param.nPortMsb), &param.nPortMsb);
    if (rret < 1)
    {
        printf("Parameter not Found: nPortMsb\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nCtpLsb", sizeof(param.nCtpLsb), &param.nCtpLsb);
    if (rret < 1)
    {
        printf("Parameter not Found: nCtpLsb\n");
        return OS_ERROR;
    }

    rret = scanParamArg(prmc, prmv, "nBrpLsb", sizeof(param.nBrpLsb), &param.nBrpLsb);
    if (rret < 1)
    {
        printf("Parameter not Found: nBrpLsb\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_TflowCountModeSet(gsw_dev, &param);

    if (ret < 0)
    {
        printf("fapi_GSW_TflowCountModeSet Return Error: %d\n", ret);
        return ret;
    }
    else
    {
        printf("fapi_GSW_TflowCountModeSet Success\n");
        return ret;
    }
}

GSW_return_t fapi_GSW_Mac_TableLoopDetect(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_MAC_tableLoopDetect_t param = {0};
    int rret;
    size_t index;

    memset(&param, 0, sizeof(GSW_MAC_tableLoopDetect_t));

    rret  = scanParamArg(prmc, prmv, "bp_map_in[0]", sizeof(param.bp_map_in[0]), &(param.bp_map_in[0]));
    rret |= scanParamArg(prmc, prmv, "bp_map_in[1]", sizeof(param.bp_map_in[1]), &(param.bp_map_in[1]));
    rret |= scanParamArg(prmc, prmv, "bp_map_in[2]", sizeof(param.bp_map_in[2]), &(param.bp_map_in[2]));
    rret |= scanParamArg(prmc, prmv, "bp_map_in[3]", sizeof(param.bp_map_in[3]), &(param.bp_map_in[3]));

    if (rret < 1)
    {
        printf("Parameter: bp_map_in[] not found or incorrect\n");
        return OS_ERROR;
    }

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_MAC_TableLoopDetect(gsw_dev, &param);

    if (ret < 0)
        printf("fapi_GSW_Mac_TableLoopDetect failed with ret code: %d\n", ret);
    else
    {
        for (size_t index = 0; index < ARRAY_SIZE(param.bp_map_in); index++)
        {
            printf("  bp_map_in[%lu]  = 0x%08x\n", index, param.bp_map_in[index]);
        }
        for (size_t index = 0; index < ARRAY_SIZE(param.bp_map_out); index++)
        {
            printf("  bp_map_out[%lu] = 0x%08x\n", index, param.bp_map_out[index]);
        }
    }
    return ret;
}

GSW_return_t fapi_GSW_PCE_RuleMove(int prmc, char *prmv[])
{

    GSW_Device_t *gsw_dev;
    GSW_return_t ret;
    GSW_PCE_rule_move_t param = {0};
    int rret;
    size_t index;

    memset(&param, 0, sizeof(GSW_PCE_rule_move_t));

    rret = scanParamArg(prmc, prmv, "cur.nLogicalPortId", sizeof(param.cur.logicalportid), &param.cur.logicalportid);
    if (rret < 1)
    {
        printf("Parameter: cur.nLogicalPortId not found\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "cur.pattern.nIndex", sizeof(param.cur.nIndex), &param.cur.nIndex);
    if (rret < 1)
    {
        printf("Parameter: cur.nIndex not found\n");
        return OS_ERROR;
    }
    scanParamArg(prmc, prmv, "cur.nSubIfIdGroup", sizeof(param.cur.subifidgroup), &param.cur.subifidgroup);
    scanParamArg(prmc, prmv, "cur.region", sizeof(param.cur.region), &param.cur.region);

    rret = scanParamArg(prmc, prmv, "new.nLogicalPortId", sizeof(param.new.logicalportid), &param.new.logicalportid);
    if (rret < 1)
    {
        printf("Parameter: new.nLogicalPortId not found\n");
        return OS_ERROR;
    }
    rret = scanParamArg(prmc, prmv, "new.pattern.nIndex", sizeof(param.new.nIndex), &param.new.nIndex);
    if (rret < 1)
    {
        printf("Parameter: new.nIndex not found\n");
        return OS_ERROR;
    }
    scanParamArg(prmc, prmv, "new.nSubIfIdGroup", sizeof(param.new.subifidgroup), &param.new.subifidgroup);
    scanParamArg(prmc, prmv, "new.region", sizeof(param.new.region), &param.new.region);

    gsw_dev = gsw_get_struc(lif_id, 0);
    ret = GSW_PceRuleMove(gsw_dev, &param);

    if (ret < 0)
    {
        printf("GSW_PceRuleMove failed with ret code: %d\n", ret);
    }
    else
    {
        printf("GSW_PceRuleMove success with ret code: %d\n", ret);
    }
    return ret;
}
