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
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/random.h>
#include <linux/fcntl.h>
#include "sha256.h"

#include "phy_mac_sec.h"
#include "phy_mac_sec_reg_access.h"


#define PHY_READ(a, b, c, d)        if ((ret = phy_bus_c45_read(a, b, c, d))) goto Exit;
#define PHY_WRITE(a, b, c, d)       if ((ret = phy_bus_c45_write(a, b, c, d))) goto Exit;

static int phy_macsec_init(phy_dev_t *phy_dev, macsec_settings_t* settings);
static int phy_macsec_restart(phy_dev_t *phy_dev);
static int phy_macsec_enable_disable(phy_dev_t *phy_dev, int enable);
static int phy_macsec_vPort_add(secy_device_t* secy_device, uint8_t sc_index);
static int phy_macsec_vPort_remove(secy_device_t* secy_device, uint8_t sc_index);
static int phy_macsec_prepare_sa_params(uint8_t direction, macsec_api_sa_t *api_sa, macsec_sa_t *sa_params);
static int phy_macsec_sa_add(secy_device_t* secy_device, uint32_t sc_index, uint32_t sa_index, macsec_sa_t *SA_p);
static int phy_macsec_sa_chain(secy_device_t* secy_device, uint32_t sa_index_active, uint32_t sa_index, macsec_sa_t *SA_p);
static int phy_macsec_sa_switch(secy_device_t* secy_device, uint32_t sa_index_active, uint32_t sa_index);
static int phy_macsec_sa_remove(secy_device_t* secy_device, uint32_t sc_index, uint32_t sa_index);
static int phy_macsec_rule_add(secy_device_t* secy_device, uint32_t sc_index, uint32_t rule_index, macsec_rule_t *Rule_p);
static int phy_macsec_rule_remove(secy_device_t* secy_device, uint32_t sc_index, uint32_t rule_index);
//static int phy_macsec_rule_update(secy_device_t* secy_device, uint32_t sc_index, uint32_t rule_index, macsec_rule_t *Rule_p);
static int phy_macsec_rule_enable(secy_device_t* secy_device, uint32_t rule_index, uint8_t enable, uint8_t fSync);
static void phy_macsec_rule_prepare(macsec_rule_t *ruleparams, macsec_api_rule_t *api_rule);
static int phy_macsec_vport_e_statistics_get(secy_device_t *secy_device, uint32_t sc_index, macsec_api_secy_e_stats *stats_p, uint8_t fSync);
static int phy_macsec_vport_i_statistics_get(secy_device_t *secy_device, uint32_t sc_index, macsec_api_secy_i_stats *stats_p, uint8_t fSync);
static int phy_macsec_tcam_statistics_get(secy_device_t *secy_device, uint32_t rule_index, macsec_api_secy_tcam_stats *stats_p, uint8_t fSync);
static int phy_macsec_rxcam_statistics_get(secy_device_t *secy_device, uint32_t sa_index, macsec_api_secy_rxcam_stats *stats_p, uint8_t fSync);
static int phy_macsec_sa_e_statistics_get(secy_device_t *secy_device, uint32_t sa_index, macsec_api_secy_sa_e_stats *stats_p, uint8_t fSync);
static int phy_macsec_sa_i_statistics_get(secy_device_t *secy_device, uint32_t sa_index, macsec_api_secy_sa_i_stats *stats_p, uint8_t fSync);

static int macsec_prepare_fw(phy_dev_t *phy_dev, int enable);
static int macsec_xlmac_powerup_init(phy_dev_t *phy_dev, macsec_port_loc_t port_loc);
static int macsec_xlmac_init(phy_dev_t *phy_dev, macsec_port_loc_t port_loc);
static int macsec_xlmac_restart(phy_dev_t *phy_dev, macsec_port_loc_t port_loc);
static int macsec_get_xfi_serdes_mode(phy_dev_t *phy_dev, uint16_t *xfi_speed, uint16_t *mode_config, uint16_t *kr_mode);
static void macsec_settings_convert(macsec_settings_t* p_settings, macsec_api_settings_t* p_api_settings);
static int macsec_secy_init(phy_dev_t *phy_dev, macsec_settings_t* p_settings);
static int macsec_device_init(secy_device_access_t *secy_device_access, const macsec_settings_t * const Settings_p, macsec_Capabilities_t *Capabilities);
static int macsec_EIP160Lib_Device_Init_Done(secy_device_access_t *secy_device_access);
static int macsec_secy_list_init(void ** const ObjectFreeList_pp, uint32_t ObjectType, void ** const ObjectDscr_pp, const uint32_t ObjectCount);
static int macsec_EIP160_HWRevision_Get(secy_device_access_t *secy_device_access, macsec_Capabilities_t * const Capabilities_p);
static int macsec_build_transform_record(macsec_sa_builder_params_t *params, uint32_t *sa_buffer_p);
static int macsec_SCI_Find(secy_device_t *secy_device, uint8_t * SCI, uint32_t vPort, uint32_t * SCIndex_p);
static int macsec_EIP160_SecY_RxCAM_Add(secy_device_access_t *secy_device_access, uint32_t SCIndex, uint32_t SCI_Lo, uint32_t SCI_Hi, uint32_t vPort);
static int macsec_EIP160_SecY_RxCAM_Update(secy_device_access_t *secy_device_access, uint32_t SCIndex, uint32_t SCI_Lo, uint32_t SCI_Hi);
static int macsec_EIP160_SecY_RxCAM_Remove(secy_device_access_t *secy_device_access, uint32_t SCIndex);
static int macsec_EIP160_SecY_SC_SA_Map_I_Update(secy_device_access_t *secy_device_access, const uint32_t SCIndex, const uint32_t AN, const uint32_t SAIndex, const uint8_t fSAInUse);
static int macsec_EIP160_SecY_SAMFlow_Write(secy_device_t *secy_device, const uint32_t vPort, const macsec_sa_t * const SA_p);
static int macsec_EIP160_SecY_SA_Stat_E_Clear(secy_device_t *secy_device, const uint32_t SAIndex);
static int macsec_EIP160_SecY_SA_Stat_I_Clear(secy_device_t *secy_device, const uint32_t SAIndex);


static int macsec_SA_Update_Control_Word_Update(secy_device_access_t *secy_device_access, const uint32_t SAIndex, const SABuilder_UpdCtrl_Params_t * const UpdateParams);
static int macsec_Rule_Update(secy_device_t *secy_device, uint32_t RuleId, macsec_rule_t *Rule_p);
static uint32_t macsec_vPortId_Get(secy_device_t* secy_device, uint32_t sc_index);
static uint32_t macsec_RuleId_Get(secy_device_t* secy_device, uint32_t rule_index);
static uint32_t macsec_RulesCount_Get(secy_device_t* secy_device, uint32_t sc_index);
static uint32_t macsec_SAId_Get(secy_device_t* secy_device, uint32_t sa_index);
static int macsec_SAHandleToIndex(secy_device_t* secy_device, uint32_t sa_index_active, uint32_t *SAIndex_p, uint32_t *SCIndex_p, uint32_t *SAMFlowCtrlIndex_p);
static uint32_t macsec_Device_Sync(secy_device_t* secy_device);


static int macsec_SABuilder_BuildSA(SABuilder_Params_t * const SAParams_p, uint32_t *const SABuffer_p, SABuilder_AESCallback_t AES_CB);
static int macsec_SABuilder_GetSize(SABuilder_Params_t *const SAParams_p, uint32_t *const SAWord32Count_p);
static void macsec_SABuilderLib_CopyKeyMat(uint32_t * const Destination_p, const uint32_t offset, const uint8_t * const Source_p, const uint32_t KeyByteCount);
static int macsec_SABuilder_InitParams(SABuilder_Params_t *const SAParams_p, uint8_t AN, SABuilder_Direction_t direction);
static void macsec_SABuilderLib_SetOffsets(const SABuilder_Params_t * const SAParams_p, SABuilder_Offsets_t * const SAOffsets_p);
static void macsec_SABuilderLib_ControlToParams(uint32_t ControlWord, SABuilder_Params_t * const SAParams_p);
static int macsec_SABuilder_UpdateCtrlOffset_Get(uint32_t ControlWord, uint32_t * const UpdCtrlOffset_p);
static int macsec_SABuilder_UpdateCtrl_Update(const SABuilder_UpdCtrl_Params_t * const UpdateParams_p, uint32_t * const SAWord_p);


List_Status_t macsec_List_Init(const uint32_t ListID, void * const ListInstance_p);
List_Status_t macsec_List_AddToHead(const uint32_t ListID, void * const ListInstance_p, List_Element_t * const Element_p);
List_Status_t macsec_List_RemoveFromTail(const uint32_t ListID, void * const ListInstance_p, List_Element_t ** const Element_pp);
List_Status_t macsec_List_RemoveAnywhere(const uint32_t ListID, void * const ListInstance_p, List_Element_t * const Element_p);
List_Status_t macsec_List_GetListElementCount(const uint32_t ListID, void * const ListInstance_p, uint32_t * const Count_p);
List_Status_t macsec_List_RemoveFromHead(const uint32_t ListID, void * const ListInstance_p, List_Element_t ** const Element_pp);
List_Status_t macsec_List_GetHead(const uint32_t ListID, void * const ListInstance_p, const List_Element_t ** const Element_pp);
uint32_t macsec_List_GetInstanceByteCount(void);
List_Element_t * macsec_List_GetNextElement(const List_Element_t * const Element_p);

uint32_t macsec_buffer_sanitize(uint8_t *inp_ucBuffer, uint32_t in_nByteCount);

const secy_SAHandle_t macsec_XtSecY_SAHandle_NULL = { 0 };
const secy_vPortHandle_t macsec_XtSecY_vPortHandle_NULL = { 0 };
const secy_RuleHandle_t  macsec_XtSecY_RuleHandle_NULL  = { 0 };

static const uint8_t *macsec_YesNo(const uint8_t b)
{
    if (b)
    {
        return "Yes";
    }
    else
    {
        return "No";
    }
}

void phy_macsec_read_array(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr, uint32_t * MemoryDst_p, int32_t Count)
{
    int i;

    if (Count <= 0)
        return;

    for (i = 0; i < Count; i++) 
    {
        //Device_SwapEndian32(pa,WordRead) ???
        MemoryDst_p[i] = phy_macsec_read(phy_dev, base_addr, offset_addr); ;
        offset_addr += 4;
    }
}

void phy_macsec_write_array(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr, uint32_t * MemoryDst_p, int32_t Count)
{
    int i;

    if (Count <= 0)
        return;

    for (i = 0; i < Count; i++) 
    {
        phy_macsec_write(phy_dev, base_addr, offset_addr, MemoryDst_p[i]);
        offset_addr += 4;
    }
}

void _phy_macsec_write(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr, uint32_t val)
{    
    uint16_t temp=0;
    int ret;

    /* Mdio2Arm Low Address Value */
    PHY_WRITE(phy_dev, 0x01, 0xa819, offset_addr);

    /* Mdio2Arm High Address Value */
    PHY_WRITE(phy_dev, 0x01, 0xa81a, base_addr);

    temp = (uint16_t)(val & 0xFFFF);
    /* Mdio2Arm Low Data Value */
    PHY_WRITE(phy_dev, 0x01, 0xa81b, temp);

    temp = (uint16_t)(val >> 16);
    /* Mdio2Arm High Data Value */
    PHY_WRITE(phy_dev, 0x01, 0xa81c, temp);

    /* Do MDIO 32 bit write */
    PHY_WRITE(phy_dev, 0x01, 0xa817, 0x0009);

    /*temp=0;
    while (!temp)
        PHY_READ(phy_dev, 0x01, 0xa818, &temp);*/

    return;

Exit:
    phy_macsec_log(LOG_ERROR, "!!! _phy_macsec_write failed\n");
    return;
}

uint32_t _phy_macsec_read(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr)
{    
    uint16_t temp=0;
    uint32_t value=0;
    int ret;

    /* Mdio2Arm Low Address Value */
    PHY_WRITE(phy_dev, 0x01, 0xa819, offset_addr);

    /* Mdio2Arm High Address Value */
    PHY_WRITE(phy_dev, 0x01, 0xa81a, base_addr);

    /* Do MDIO 32 bit read */
    PHY_WRITE(phy_dev, 0x01, 0xa817, 0x000A);

    /*temp=0;
    while (!temp)
        PHY_READ(phy_dev, 0x01, 0xa818, &temp);*/

    /* Mdio2Arm Low Data Value */
    PHY_READ(phy_dev, 0x01, 0xa81b, (uint16_t*)(&value));

    /* Mdio2Arm High Data Value */
    PHY_READ(phy_dev, 0x01, 0xa81c, &temp);

    value |= (uint32_t)(temp<<16);

    return value;

Exit:
    phy_macsec_log(LOG_ERROR, "!!! _phy_macsec_read failed\n");
    return -1;
}

int phy_macsec_pu_init(phy_dev_t *phy_dev)
{
    int ret = 0;
    phy_macsec_dev *macsec_dev = (phy_macsec_dev*)phy_dev->macsec_dev;
    uint16_t temp = 0;

    if (macsec_dev)
        phy_macsec_log(LOG_INFO, "macsec device already exists\n");
    else
    {
        macsec_dev = kmalloc(sizeof(phy_macsec_dev), GFP_KERNEL);
        if (!macsec_dev)
            phy_macsec_log(LOG_ERROR, "macsec device malloc failed\n");

        macsec_dev->initialized = 0;
        macsec_dev->enabled = 0;
        memset(&macsec_dev->secy_devices[0], 0, sizeof(secy_device_t) * 2);

        phy_dev->macsec_dev = macsec_dev;
    }

    PHY_READ(phy_dev, 0x1E, CRG_CORE_CONFIGr, &temp);
    /* Set default CLK_MACSEC for 10G speed to make XLMAC works. CLK_MACSEC must be changed according to the SerDes speed when enabling MACSEC */
    temp = (temp & (~CRG_CORE_CONFIG_LONGFIN_A0_INIT_MASK)) | CRG_CORE_CONFIG_LONGFIN_A0_INIT;
    PHY_WRITE(phy_dev, 0x1E, CRG_CORE_CONFIGr, temp);
    
    /* bypass MACSEC from datapath by default  (Reg. 30.0x813C MacSec Control Register) */
    PHY_WRITE(phy_dev, 0x1E, CFG_MACSEC_CTRLr, CFG_MACSEC_CTRL__INIT);

    phy_macsec_log(LOG_DEBUG, "\nmacsec_xlmac_powerup_init system side\n");
    ret = macsec_xlmac_powerup_init(phy_dev, macsecPortLocSys);
    if (ret)
        phy_macsec_log(LOG_ERROR, "macsec_xlmac_powerup_init macsecPortLocSys failed\n");

    phy_macsec_log(LOG_DEBUG, "\nmacsec_xlmac_powerup_init line side\n");
    ret = macsec_xlmac_powerup_init(phy_dev, macsecPortLocLine);
    if (ret)
        phy_macsec_log(LOG_ERROR, "macsec_xlmac_powerup_init macsecPortLocLine failed\n"); 

    macsec_prepare_fw(phy_dev, 1);

    phy_macsec_log(LOG_INFO, "macsec init DONE\n");

Exit:
    return ret;
}

int phy_macsec_oper(phy_dev_t *phy_dev, void *data)
{
    int ret = 0;
    macsec_api_data *macsec_data = (macsec_api_data *)data;
    phy_macsec_dev *macsec_dev = (phy_macsec_dev*)phy_dev->macsec_dev;

    if ((!macsec_dev->initialized) && (macsec_data->op != MACSEC_OPER_INIT) && (macsec_data->op != MACSEC_OPER_SET_LOG_LEVEL))
    {
        phy_macsec_log(LOG_ERROR, "macsec is not enabled\n");
        ret = -1;
        goto EXIT;
    }

    switch (macsec_data->op)
    {
        case MACSEC_OPER_RESTART:
        {
            ret = phy_macsec_restart(phy_dev);
            break;
        }
        case MACSEC_OPER_INIT:
        {
            macsec_settings_t Settings;
            
            memset(&Settings, 0, sizeof(macsec_settings_t));

            ret = phy_macsec_restart(phy_dev);

            macsec_settings_convert(&Settings, &macsec_data->ext_data.secy_conf);

            ret = phy_macsec_init(phy_dev, &Settings);

            break;
        }
        case MACSEC_OPER_EN_DS:
        {
            ret = phy_macsec_enable_disable(phy_dev, macsec_data->data1);
            break;
        }
        case MACSEC_OPER_VPORT_ADD:
        {
            ret = phy_macsec_vPort_add(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1);
            break;
        }
        case MACSEC_OPER_VPORT_REMOVE:
        {
            ret = phy_macsec_vPort_remove(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1);
            break;
        }
        case MACSEC_OPER_SA_ADD:
        {
            macsec_sa_t sa_params = {};

            ret = phy_macsec_prepare_sa_params(macsec_data->direction, &macsec_data->ext_data.sa_conf, &sa_params);
            ret = phy_macsec_sa_add(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->index2, &sa_params);

            macsec_buffer_sanitize((uint8_t *)&sa_params.transform_record, sizeof(uint32) * 24);
            macsec_buffer_sanitize((uint8_t *)&macsec_data->ext_data.sa_conf, sizeof(macsec_api_sa_t));
            break;
        }
        case MACSEC_OPER_SA_CHAIN:
        {
            macsec_sa_t sa_params = {};

            ret = phy_macsec_prepare_sa_params(macsec_data->direction, &macsec_data->ext_data.sa_conf, &sa_params);
            ret = phy_macsec_sa_chain(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->index2, &sa_params);

            macsec_buffer_sanitize((uint8_t *)&sa_params.transform_record, sizeof(uint32) * 24);
            macsec_buffer_sanitize((uint8_t *)&macsec_data->ext_data.sa_conf, sizeof(macsec_api_sa_t));
            break;
        }
        case MACSEC_OPER_SA_SWITCH:
        {
            ret = phy_macsec_sa_switch(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->index2);
            break;
        }
        case MACSEC_OPER_SA_REMOVE:
        {
            ret = phy_macsec_sa_remove(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->index2);
            break;
        }
        case MACSEC_OPER_RULE_ADD:
        {
            macsec_rule_t ruleparams = {};

            phy_macsec_rule_prepare(&ruleparams, &macsec_data->ext_data.rule_conf);
            ret = phy_macsec_rule_add(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->index2, &ruleparams);
            break;
        }
        case MACSEC_OPER_RULE_REMOVE:
        {
            ret = phy_macsec_rule_remove(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->index2);
            break;
        }
        case MACSEC_OPER_RULE_ENABLE:
        {
            ret = phy_macsec_rule_enable(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, macsec_data->data1, 1);
            break;
        }
        case MACSEC_OPER_VPORT_E_STAT_GET:
        {
            phy_macsec_vport_e_statistics_get(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, &macsec_data->ext_data.secy_e_stats, 1);
            break;
        }
        case MACSEC_OPER_VPORT_I_STAT_GET:
        {
            phy_macsec_vport_i_statistics_get(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, &macsec_data->ext_data.secy_i_stats, 1);
            break;
        }
        case MACSEC_OPER_TCAM_STAT_GET:
        {
            phy_macsec_tcam_statistics_get(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, &macsec_data->ext_data.tcam_stats, 1);
            break;
        }
        case MACSEC_OPER_RXCAM_STAT_GET:
        {
            phy_macsec_rxcam_statistics_get(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, &macsec_data->ext_data.rxcam_stats, 1);
            break;
        }
        case MACSEC_OPER_SA_E_STAT_GET:
        {
            phy_macsec_sa_e_statistics_get(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, &macsec_data->ext_data.sa_e_stats, 1);
            break;
        }
        case MACSEC_OPER_SA_I_STAT_GET:
        {
            phy_macsec_sa_i_statistics_get(&(macsec_dev->secy_devices[macsec_data->direction]), macsec_data->index1, &macsec_data->ext_data.sa_i_stats, 1);
            break;
        }
        case MACSEC_OPER_SET_LOG_LEVEL:
        {
            log_level = macsec_data->data1;
            break;
        }
        default:
        {
            phy_macsec_log(LOG_ERROR, "operation %u not defined\n", macsec_data->op);
            ret = -1;
        }
    }

EXIT:
    macsec_data->ret_val = ret;
    return ret;
}

static int phy_macsec_init(phy_dev_t *phy_dev, macsec_settings_t* settings)
{
    int ret = 0;

    phy_macsec_dev *macsec_dev = (phy_macsec_dev*)phy_dev->macsec_dev;

    if (macsec_dev->initialized == 1)
    {
        phy_macsec_log(LOG_DEBUG, "macsec already initialized\n");
        return MACSEC_RET_ALREADY_INIT;
    }

    phy_macsec_log(LOG_DEBUG, "\nmacsec_xlmac_init system side\n");
    ret = macsec_xlmac_init(phy_dev, macsecPortLocSys);
    if (ret)
    {
        phy_macsec_log(LOG_ERROR, "macsec_xlmac_init macsecPortLocSys failed\n");
        goto Ret;
    }

    phy_macsec_log(LOG_DEBUG, "\nmacsec_xlmac_init line side\n");
    ret = macsec_xlmac_init(phy_dev, macsecPortLocLine);
    if (ret)
    {
        phy_macsec_log(LOG_ERROR, "macsec_xlmac_init macsecPortLocLine failed\n"); 
        goto Ret;
    }

    phy_macsec_write(phy_dev, MACSEC_EGRESS_REG_BASE, 0xf408, 0xE5880618);
    phy_macsec_write(phy_dev, MACSEC_EGRESS_REG_BASE, 0xf430, 0x3);
    phy_macsec_write(phy_dev, MACSEC_INGRESS_REG_BASE, 0xf430, 0x3);
    phy_macsec_write(phy_dev, MACSEC_EGRESS_REG_BASE, 0xffec, 0x1);
    phy_macsec_write(phy_dev, MACSEC_INGRESS_REG_BASE, 0xffec, 0x1);

    phy_macsec_enable_disable(phy_dev, 1);

    settings->Mode = SECY_ROLE_EGRESS;
    ret = macsec_secy_init(phy_dev, settings);
    if (ret)
    {
        phy_macsec_log(LOG_ERROR, "macsec_secy_init egress failed\n");
        goto Ret;        
    }


    settings->Mode = SECY_ROLE_INGRESS;
    ret = macsec_secy_init(phy_dev, settings);
    if (ret)
    {
        phy_macsec_log(LOG_ERROR, "macsec_secy_init ingress failed\n");
        goto Ret;
    }

    macsec_dev->initialized = 1;
    macsec_dev->enabled = 1;

Ret:
    return ret; 
}

static int phy_macsec_restart(phy_dev_t *phy_dev)
{
    int ret = 0;
    phy_macsec_dev *macsec_dev = (phy_macsec_dev*)phy_dev->macsec_dev;
    uint16_t xfi_spd = SPEED_10G, mode_config = 0, kr_mode = 0;

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_restart\n");  

    ret = macsec_get_xfi_serdes_mode(phy_dev, &xfi_spd, &mode_config, &kr_mode); 

    /* MACSEC_CONFIG_PORT_CTRL & MACSEC_CONFIG_MBUS */
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, 0x1010, (xfi_spd > SPEED_1000) ? 0x0001032D : 0x00010F2D);

    if (macsec_dev->initialized)
    {
        phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, 0x1018, 0x0);
        msleep(100);
        phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, 0x1018, 0x3);

        macsec_xlmac_restart(phy_dev, macsecPortLocSys);
        macsec_xlmac_restart(phy_dev, macsecPortLocLine);

        if (macsec_dev->enabled)
            phy_macsec_enable_disable(phy_dev, 1);
    }

    phy_macsec_log(LOG_DEBUG, "phy_macsec_restart DONE\n\n");

    return ret;
}

static int phy_macsec_enable_disable(phy_dev_t *phy_dev, int enable)
{
    int ret = 0;
    uint16_t  data = 0;
    phy_macsec_dev *macsec_dev = (phy_macsec_dev*)phy_dev->macsec_dev;

    PHY_READ(phy_dev, 0x1E, PHY_REG_MACSEC_CTRL, &data);

    if ( !enable ) 
    {
        data |= ( PHY_REG_MACSEC_CTRL_BYPASS);      /* 30.0x813C[4]=1 :   set  MAcSEC bypass */
    } 
    else 
    {
        data &= (~PHY_REG_MACSEC_CTRL_BYPASS);      /* 30.0x813C[4]=0 : remove MAcSEC bypass */
    }

    PHY_WRITE(phy_dev, 0x1E, PHY_REG_MACSEC_CTRL, data);
    macsec_dev->enabled = enable;

Exit:
    return ret;
}

static int phy_macsec_vPort_add(secy_device_t* secy_device, uint8_t sc_index)
{
    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_vPort_add [direction=%u, sc_index=%u]\n", secy_device->Role, sc_index);

    if (sc_index > VPORT_NUM_MAX)
    {
        phy_macsec_log(LOG_ERROR, "invalid sc_index %u\n", sc_index);
        return -1;
    }

    if (secy_device->vport_handlers[sc_index].p)
    {
        phy_macsec_log(LOG_DEBUG, "vport_index=[%u] already exists\n", sc_index);
        return MACSEC_RET_ALREADY_INIT;
    }

    if (!(secy_device && secy_device->fInitialized))
    {
        phy_macsec_log(LOG_ERROR, "secy device not initialized\n");
        return -1;
    }

    /* Obtain a free vPort for this classification device */
    {
        List_Element_t * vPortElmt_p;
        secy_vport_descriptor_t * vPortDscr_p;
        void * const PFL_p = secy_device->vPortFreeList_p;
        List_Status_t List_Rc = macsec_List_RemoveFromTail(0, PFL_p, &vPortElmt_p);
        if (List_Rc != LIST_STATUS_OK || vPortElmt_p->DataObject_p == NULL)
        {
            phy_macsec_log(LOG_ERROR, " Failed to obtain a free vPort\n");
            return -1;
        }

        /* Convert vPort descriptor to vPort index and handle */
        vPortDscr_p = (secy_vport_descriptor_t*)vPortElmt_p->DataObject_p;
        secy_device->vport_handlers[sc_index].p = vPortDscr_p;
        vPortDscr_p->InUse.vPortId = vPortDscr_p - secy_device->vPortDscr_p;
        vPortDscr_p->InUse.BoundRulesCount = 0;
        vPortDscr_p->InUse.SACount = 0;
        vPortDscr_p->InUse.SCList = secy_device->vPortListHeads_p + vPortDscr_p->InUse.vPortId * macsec_List_GetInstanceByteCount();

        List_Rc = macsec_List_Init(0, vPortDscr_p->InUse.SCList);
        if (List_Rc != LIST_STATUS_OK)
        {
            phy_macsec_log(LOG_ERROR, " Failed to initialize SC lookup list\n");
            return -1;
        }

        if (secy_device->Role == SECY_ROLE_EGRESS)
        {
            macsec_EIP160_SecY_SecY_Stat_E_Clear(&secy_device->secy_device_access, vPortDscr_p->InUse.vPortId);
            macsec_EIP160_SecY_Ifc_Stat_E_Clear(&secy_device->secy_device_access, vPortDscr_p->InUse.vPortId);
        }
        else
        {
            macsec_EIP160_SecY_SecY_Stat_I_Clear(&secy_device->secy_device_access, vPortDscr_p->InUse.vPortId);
            macsec_EIP160_SecY_Ifc_Stat_I_Clear(&secy_device->secy_device_access, vPortDscr_p->InUse.vPortId);
        }

        phy_macsec_log(LOG_INFO, "DONE: phy_macsec_vPort_add [direction=%u, sc_index=%u, vPort=%u]\n", secy_device->Role, sc_index, vPortDscr_p->InUse.vPortId);
    }

    return 0;
}

static int phy_macsec_vPort_remove(secy_device_t* secy_device, uint8_t sc_index)
{
    uint32_t vPortId = macsec_vPortId_Get(secy_device, sc_index);

    if (vPortId == -1)
        return -1;
    
    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_vPort_add [direction=%u, sc_index=%u, vPortId=%u]\n", secy_device->Role, sc_index, vPortId);

    if (vPortId >= secy_device->vPortCount)
    {
        phy_macsec_log(LOG_ERROR, "ERROR:  vPortId [%u] >= vPortCount [%u]\n", vPortId, secy_device->vPortCount);
        return -1;
    }
    /* Check there are no rules associated with this vPort */
    {
        uint32_t RulesCount = macsec_RulesCount_Get(secy_device, sc_index);

        if (RulesCount)
        {
            phy_macsec_log(LOG_ERROR, " Failed to remove vPort id=%d from device %d, bound rules count %d\n", vPortId, secy_device->Role, RulesCount);
            return -1;
        }
    }

    /* Add vPort to be removed to the vPort free list */
    {
        List_Status_t List_Rc;
        void * const PFL_p = secy_device->vPortFreeList_p;
        List_Element_t * const vPortElmt_p = &secy_device->vPortDscr_p[vPortId].free;
        vPortElmt_p->DataObject_p = &secy_device->vPortDscr_p[vPortId];

        List_Rc = macsec_List_AddToHead(0, PFL_p, vPortElmt_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            phy_macsec_log(LOG_ERROR, " Failed to add vPort id=%d to free list for device %d\n", vPortId, secy_device->Role);
            return -1;
        }
    }

    secy_device->vport_handlers[sc_index] = macsec_XtSecY_vPortHandle_NULL;

    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_vPort_add [direction=%u, sc_index=%u, vPortId=%u]\n", secy_device->Role, sc_index, vPortId);

    return 0;
}

static int phy_macsec_prepare_sa_params(uint8_t direction, macsec_api_sa_t *api_sa, macsec_sa_t *sa_params)
{
    int rc;
    uint8_t build_trec = 0;
    macsec_sa_builder_params_t transform_record_basic;
    uint32_t transform_record[24] = {}; 

    sa_params->action_type = api_sa->action_type; 
    sa_params->dest_port = MACSEC_PORT_TXMAC;
    sa_params->drop_type = MACSEC_SA_DROP_INTERNAL;

    switch (sa_params->action_type)
    {
        case MACSEC_SA_ACTION_INGRESS:
        {
            sa_params->params.ingress.fsa_inuse = api_sa->params.ingress.fsa_inuse;
            sa_params->params.ingress.validate_frames_tagged = api_sa->params.ingress.validate_frames_tagged;
            sa_params->params.ingress.freplay_protect = api_sa->params.ingress.freplay_protect;
            sa_params->params.ingress.sci_p = api_sa->sci;
            sa_params->params.ingress.an = api_sa->params.ingress.an;
            sa_params->params.ingress.fallow_tagged = api_sa->params.ingress.fallow_tagged;
            sa_params->params.ingress.fallow_untagged = api_sa->params.ingress.fallow_untagged;
            sa_params->params.ingress.fvalidate_untagged = api_sa->params.ingress.fvalidate_untagged; 
            sa_params->params.ingress.confidentiality_offset = 0;
            sa_params->params.ingress.fmac_icmac = 0;
            sa_params->params.ingress.pre_sectag_auth_start = 0;
            sa_params->params.ingress.pre_sectag_auth_length = 12;
            sa_params->params.ingress.sectag_offset = 12;
            sa_params->params.ingress.fretain_sectag = 0;
            sa_params->params.ingress.fretain_icv = 0;
            
            build_trec = 1;
            transform_record_basic.operation = MACSEC_SAB_OP_MACSEC;
            transform_record_basic.direction = MACSEC_SAB_DIRECTION_INGRESS;
            transform_record_basic.seq_num_lo = api_sa->params.ingress.seq_num_lo;
            transform_record_basic.an = api_sa->params.ingress.an;

            break;
        }
        case MACSEC_SA_ACTION_EGRESS:
        {
            sa_params->params.egress.fsa_inuse = api_sa->params.egress.fsa_inuse;
            sa_params->params.egress.fprotect_frames = api_sa->params.egress.fprotect_frames;
            sa_params->params.egress.finclude_sci = api_sa->params.egress.finclude_sci; 
            sa_params->params.egress.fconf_protect = api_sa->params.egress.fconf_protect; 
            sa_params->params.egress.fallow_data_pkts = api_sa->params.egress.fallow_data_pkts;
            sa_params->params.egress.confidentiality_offset = 0;
            sa_params->params.egress.fuse_es = api_sa->params.egress.fuse_es;
            sa_params->params.egress.fuse_scb = api_sa->params.egress.fuse_scb;
            sa_params->params.egress.fsl_pad_strip_enb = 0;
            sa_params->params.egress.pre_sectag_auth_start = 0;
            sa_params->params.egress.pre_sectag_auth_length = 12;
            sa_params->params.egress.sectag_offset = 12;
            
            build_trec = 1;
            transform_record_basic.operation = MACSEC_SAB_OP_MACSEC;
            transform_record_basic.direction = MACSEC_SAB_DIRECTION_EGRESS;
            transform_record_basic.seq_num_lo = api_sa->params.egress.seq_num_lo;
            transform_record_basic.an = api_sa->params.egress.an;

            break;
        }
        case MACSEC_SA_ACTION_CRYPT_AUTH:
        {
            sa_params->params.crypt_auth.fzero_length_message = 0;
            sa_params->params.crypt_auth.confidentiality_offset = 0;
            sa_params->params.crypt_auth.iv_mode = 0;
            sa_params->params.crypt_auth.ficv_append = 0;
            sa_params->params.crypt_auth.ficv_verify = api_sa->params.crypt_auth.ficv_verify;
            sa_params->params.crypt_auth.fconf_protect = api_sa->params.crypt_auth.fconf_protect;

            build_trec = 1;
            transform_record_basic.direction = direction;
            transform_record_basic.operation = MACSEC_SAB_OP_ENCAUTH_AES_GCM;
            break;
        }
        /* bypass or drop */
        default: 
        {
            sa_params->params.bypass_drop.fsa_inuse = api_sa->params.bypass_drop.fsa_inuse;
            break;
        }
    }

    if (build_trec)
    {
        sa_params->sa_word_count = 24;
        /* Create Transform Record using API */
        transform_record_basic.flags = 0; //(SAB_MACSEC_FLAG_ROLLOVER | SAB_MACSEC_FLAG_UPDATE_ENABLE | SAB_MACSEC_FLAG_SA_EXPIRED_IRQ);
        transform_record_basic.key_p = api_sa->key;
        transform_record_basic.key_byte_count = api_sa->key_size;
        transform_record_basic.h_key_p = api_sa->hkey;
        if (api_sa->fextended_pn)
        {
            transform_record_basic.salt_p = api_sa->salt;
            transform_record_basic.ssci_p = api_sa->ssci;
        }
        else
        {
            transform_record_basic.salt_p = NULL; //api_sa->salt;
            transform_record_basic.ssci_p = NULL; //api_sa->ssci;
        }
        transform_record_basic.sci_p = api_sa->sci;
        transform_record_basic.seq_num_hi = 0x00000000;
        transform_record_basic.window_size = 0x00000080;
        transform_record_basic.icv_byte_count = 0x0;

        rc = macsec_build_transform_record(&transform_record_basic, transform_record);
        if (rc) 
        {
            phy_macsec_log(LOG_ERROR, "FAILED: MACSec build_transform_record failed for macsec-direction %s \n", direction ? "ingress" : "egress");
            return rc;
        } 
        else 
        {
            phy_macsec_log(LOG_DEBUG, "PASSED: MACSec build_transform_record passed for macsec-direction %s \n", direction ? "ingress" : "egress");
        }  
                     
        memcpy(sa_params->transform_record, transform_record, sizeof(transform_record));
    }

    return 0;
}


static int phy_macsec_sa_add(secy_device_t* secy_device, uint32_t sc_index, uint32_t sa_index, macsec_sa_t *SA_p)
{
    int Rc;
    uint32_t SAIndex;
    SABuilder_UpdCtrl_Params_t SAUpdateCtrWord;
    uint32_t vPortId;
    uint8_t fFirstSA;
    
    vPortId = macsec_vPortId_Get(secy_device, sc_index);

    if (vPortId == -1)
        return -1;

    if (sa_index >= SA_NUM_MAX)
    {
        phy_macsec_log(LOG_ERROR, "invalid sa_index %u\n", sa_index);
        return -1;
    }

    if (secy_device->sa_handlers[sa_index].p)
    {
        secy_sa_descriptor_t *SADscr_p = secy_device->sa_handlers[sa_index].p;
        
        SAIndex = SADscr_p->InUse.SAIndex;
        phy_macsec_log(LOG_DEBUG, "phy_macsec_sa_add: sa_index=[%u] exist (SAId=%u), update transform record\n", sa_index, SAIndex);

        macsec_EIP160_XFORM_REC_RD(&secy_device->secy_device_access, SAIndex, 1, 1, &SA_p->transform_record[1]);       
        macsec_EIP160_XFORM_REC_WR(&secy_device->secy_device_access, SAIndex, SA_p->transform_record, SA_p->sa_word_count);

        if (SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS)
        {
            secy_sc_descriptor_t *SCDscr_p = &(secy_device->SCDscr_p[SADscr_p->InUse.SCIndex]);
          
            memcpy(SCDscr_p->SCI, SA_p->params.ingress.sci_p, 8);

            /* Add entry to RX CAM. */
            Rc = macsec_EIP160_SecY_RxCAM_Update(&secy_device->secy_device_access,
                                              SCDscr_p->SCIndex,
                                              (SCDscr_p->SCI[0] <<  0) | (SCDscr_p->SCI[1] <<  8) | (SCDscr_p->SCI[2] << 16) | (SCDscr_p->SCI[3] << 24),
                                              (SCDscr_p->SCI[4] <<  0) | (SCDscr_p->SCI[5] <<  8) | (SCDscr_p->SCI[6] << 16) | (SCDscr_p->SCI[7] << 24));
        }

        goto clean_stats;
    }

    if (SA_p->action_type == MACSEC_SA_ACTION_INGRESS && SA_p->params.ingress.an > 3)
    {
        /* Error out early on invalid ingress AN. */
        return -1;
    }

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_sa_add [direction=%u, sc_index=%u, sa_index=%u, vPort=%u]\n", secy_device->Role, sc_index, sa_index, vPortId);

    fFirstSA = (secy_device->vPortDscr_p[vPortId].InUse.SACount == 0);

    if (fFirstSA)
        macsec_EIP160_SAM_FLOW_CTRL1_BYPASS_WR(&secy_device->secy_device_access, vPortId, 0, 0, 0, 0, 0);

    /* Obtain a free SA for this XtSecY device */
    {
        List_Element_t * Element_p;
        secy_sa_descriptor_t * SADscr_p;
        List_Status_t List_Rc = macsec_List_RemoveFromTail(0, secy_device->SAFL_p, &Element_p);
        if (List_Rc != LIST_STATUS_OK || Element_p->DataObject_p == NULL)
        {
            phy_macsec_log(LOG_ERROR, " Failed to obtain a free SA for EIP-160 device. error %d\n", List_Rc);
            return -1;
        }

        /* Convert SA descriptor to SA index and to SA handle */
        SADscr_p = (secy_sa_descriptor_t*)Element_p->DataObject_p;
        SAIndex = SADscr_p - secy_device->SADscr_p;
        if (SAIndex >= secy_device->SACount)
        {
            phy_macsec_log(LOG_ERROR, " Illegal SA Index for"
                     "device %d, index %u\n",
                     secy_device->Role,
                     SAIndex);

            return -1;
        }

        SADscr_p->InUse.SAIndex = SAIndex;
        SADscr_p->InUse.SAMFlowCtrlIndex = vPortId;

        switch (SA_p->action_type)
        {
            case MACSEC_SA_ACTION_EGRESS:
                SADscr_p->InUse.MapType = SECY_SA_MAP_EGRESS;
                break;
            case MACSEC_SA_ACTION_INGRESS:
                SADscr_p->InUse.MapType = SECY_SA_MAP_INGRESS;
                break;
            case MACSEC_SA_ACTION_CRYPT_AUTH:
                if (secy_device->Role == SECY_ROLE_EGRESS)
                    SADscr_p->InUse.MapType = SECY_SA_MAP_EGRESS_CRYPT_AUTH;
                else
                    SADscr_p->InUse.MapType = SECY_SA_MAP_INGRESS_CRYPT_AUTH;
                break;
            default:
                SADscr_p->InUse.MapType = SECY_SA_MAP_DETACHED;
        }

        secy_device->sa_handlers[sa_index].p = SADscr_p;
        SADscr_p->InUse.SCIndex = 0xffffffff;


        if (SA_p->sa_word_count > 0)
        {
            uint32_t OldCtxID;

            macsec_EIP160_XFORM_REC_RD(&secy_device->secy_device_access, SAIndex, 1, 1, &OldCtxID);

            /* Increment 24 most significant bits of CtxID so it is unique. */
            SA_p->transform_record[1]=OldCtxID + 0x100;
            //phy_macsec_log(LOG_ERROR, " OldCtxID=%x\n", OldCtxID);
            /* Write transform record to device. */
            macsec_EIP160_XFORM_REC_WR(&secy_device->secy_device_access, SAIndex, SA_p->transform_record, SA_p->sa_word_count);

        }

        if ((SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS) ||
            (SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS_CRYPT_AUTH))
        {
            uint8_t found;
            uint8_t *SCI;
            uint8_t SCI_None[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
            if (SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS)
            {
                SCI = SA_p->params.ingress.sci_p;
            }
            else
            {
                SCI = SCI_None;
            }

            /* Try to find an existing SC entry with same SCI and vPort. */
            found = macsec_SCI_Find(secy_device, SCI, vPortId, &SADscr_p->InUse.SCIndex);
            if (!found)
            {
                /* Try to allocate new SC entry. */
                secy_sc_descriptor_t *SCDscr_p;
                List_Rc = macsec_List_RemoveFromTail(0, secy_device->SCFL_p, &Element_p);
                if (List_Rc != LIST_STATUS_OK ||
                               Element_p->DataObject_p == NULL)
                {
                    phy_macsec_log(LOG_ERROR, " Failed to obtain a free SC for EIP-160"
                             " device for device %d, error %d\n",
                             secy_device->Role,
                             List_Rc);
                    return -1;
                }

                /* Fill in the new SC entry. */
                SCDscr_p = (secy_sc_descriptor_t*)Element_p->DataObject_p;
                SADscr_p->InUse.SCIndex = SCDscr_p->SCIndex;
                memcpy(SCDscr_p->SCI, SCI, 8);
                SCDscr_p->MapType = SADscr_p->InUse.MapType;
                SCDscr_p->SAHandle[0] = macsec_XtSecY_SAHandle_NULL;
                SCDscr_p->SAHandle[1] = macsec_XtSecY_SAHandle_NULL;
                SCDscr_p->SAHandle[2] = macsec_XtSecY_SAHandle_NULL;
                SCDscr_p->SAHandle[3] = macsec_XtSecY_SAHandle_NULL;

                /* Add SC descriptor to vPort specific lookup list. */
                List_Rc = macsec_List_AddToHead(0, secy_device->vPortDscr_p[vPortId].InUse.SCList, Element_p);
                if (List_Rc != LIST_STATUS_OK)
                {
                    phy_macsec_log(LOG_ERROR, " Failed to add SC element to lookup list for "
                             "EIP-160 device for device %d, error %d\n",
                             secy_device->Role,
                             List_Rc);
                    return -1;
                }

                /* Add entry to RX CAM. */
                Rc = macsec_EIP160_SecY_RxCAM_Add(&secy_device->secy_device_access,
                                      SCDscr_p->SCIndex,
                                      (SCI[0] <<  0) |
                                      (SCI[1] <<  8) |
                                      (SCI[2] << 16) |
                                      (SCI[3] << 24) ,
                                      (SCI[4] <<  0) |
                                      (SCI[5] <<  8) |
                                      (SCI[6] << 16) |
                                      (SCI[7] << 24),
                                      vPortId);
                if (Rc)
                {
                    phy_macsec_log(LOG_ERROR, " Failed to write RxCAM to EIP-160 device %d\n", secy_device->Role);
                    return -1;
                }

            }

            if (!macsec_SAHandle_IsSame(&(secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].SAHandle[SA_p->params.ingress.an]), &macsec_XtSecY_SAHandle_NULL))
            {
                /* There was already an ingress SA for this vPOrt. */
                /* Mark it as unused and subtract one from the SA count. */
                secy_sa_descriptor_t * SADscr2_p;

                SADscr2_p = (secy_sa_descriptor_t *)&(secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].SAHandle[SA_p->params.ingress.an].p);
                SADscr2_p->InUse.MapType = SECY_SA_MAP_DETACHED;
                secy_device->vPortDscr_p[vPortId].InUse.SACount--;
            }

            secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].SAHandle[SA_p->params.ingress.an] = secy_device->sa_handlers[sa_index];

            if (SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS)
            {
                /* Enable the correct SC->SA mapping */
                SADscr_p->InUse.AN = SA_p->params.ingress.an;

                Rc = macsec_EIP160_SecY_SC_SA_Map_I_Update(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, SA_p->params.ingress.an, SAIndex, 1);
                if (Rc)
                {
                    phy_macsec_log(LOG_ERROR, " Failed map SC to SC for EIP-160 device %d \n", secy_device->Role);
                    return -1;
                }

                SAUpdateCtrWord.SCIndex = SADscr_p->InUse.SCIndex;
                SAUpdateCtrWord.AN = SA_p->params.ingress.an;
                SAUpdateCtrWord.SAIndex = SAIndex;
                SAUpdateCtrWord.fSAIndexValid = 0;
                SAUpdateCtrWord.fUpdateEnable = 1;
                SAUpdateCtrWord.fUpdateTime = 0;
                SAUpdateCtrWord.fExpiredIRQ = 1;

                Rc = macsec_SA_Update_Control_Word_Update(&secy_device->secy_device_access, SAIndex, &SAUpdateCtrWord);
                if (Rc)
                {
                    phy_macsec_log(LOG_ERROR, " Failed to update SA update control word for device %d \n", secy_device->Role);
                    return -1;
                }
            }
            else
            {
                uint32_t i;
                /* Crypt_Auth, map all four SA slots in SC entry. */
                SADscr_p->InUse.AN = 0;

                for (i=0; i<4; i++)
                {
                    Rc = macsec_EIP160_SecY_SC_SA_Map_I_Update(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, i, SAIndex, 1);
                    if (Rc)
                    {
                        phy_macsec_log(LOG_ERROR, " Failed map SC to SC [%d] for EIP-160 device %d \n", i, secy_device->Role);
                        return -1;
                    }
                }
            }
        }
        else
        {
            SADscr_p->InUse.SCIndex = vPortId;
            SADscr_p->InUse.AN = 0;

            if ((secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].MapType != SECY_SA_MAP_DETACHED) &&
                !macsec_SAHandle_IsSame(&(secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].SAHandle[0]), &macsec_XtSecY_SAHandle_NULL))
            {
                secy_sa_descriptor_t * SADscr2_p;
                phy_macsec_log(LOG_DEBUG, " is same\n");
                /* There was already an egress SA for this vPort. */
                /* Mark it as unused and subtract one from the SA count. */

                SADscr2_p = (secy_sa_descriptor_t*)&(secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].SAHandle[0].p);
                SADscr2_p->InUse.MapType = SECY_SA_MAP_DETACHED;
                secy_device->vPortDscr_p[vPortId].InUse.SACount--;
            }

            secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].SAHandle[0] = secy_device->sa_handlers[sa_index];
            secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].MapType = SADscr_p->InUse.MapType;

            /* Enable the correct SC->SA mapping */
            macsec_EIP160_SC_SA_MAP1_WR(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, SAIndex, 1, 0, 0, 0);

            SAUpdateCtrWord.SCIndex = SADscr_p->InUse.SCIndex;
            SAUpdateCtrWord.AN = 0;
            SAUpdateCtrWord.SAIndex = 0;
            SAUpdateCtrWord.fSAIndexValid = 0;
            SAUpdateCtrWord.fUpdateEnable = 1;
            SAUpdateCtrWord.fExpiredIRQ = 1;
            SAUpdateCtrWord.fUpdateTime = 0;
            
            Rc = macsec_SA_Update_Control_Word_Update(&secy_device->secy_device_access, SAIndex, &SAUpdateCtrWord);
            if (Rc)
            {
                phy_macsec_log(LOG_ERROR, " Failed to update SA update control word for device %d \n", secy_device->Role);
                return -1;
            }
        }

        if (SADscr_p->InUse.MapType != SECY_SA_MAP_DETACHED)
            secy_device->vPortDscr_p[vPortId].InUse.SACount++;
    }

    if (fFirstSA)
    {
        Rc = macsec_EIP160_SecY_SAMFlow_Write(secy_device, vPortId, SA_p);
        if (Rc)
        {
            phy_macsec_log(LOG_ERROR, " Failed to add SA for EIP-160 device %d \n", secy_device->Role);
            return -1;
        }
    }

clean_stats:

    if (SA_p->action_type == MACSEC_SA_ACTION_EGRESS)
    {
        /* Reset/read egress SA statistics counters */
        Rc = macsec_EIP160_SecY_SA_Stat_E_Clear(secy_device, SAIndex);
        if (Rc)
        {
            phy_macsec_log(LOG_ERROR, " Failed to reset egress SA statistics\n");
            return -1;
        }
    }

    if (SA_p->action_type == MACSEC_SA_ACTION_INGRESS)
    {
        /* Reset/read ingress SA statistics counters */
        Rc = macsec_EIP160_SecY_SA_Stat_I_Clear(secy_device, SAIndex);
        if (Rc)
        {
            phy_macsec_log(LOG_ERROR, " Failed to reset ingress SA statistics\n");
            return -1;
        }
    }

    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_sa_add [direction=%u, sc_index=%u, sa_index=%u, vPort=%u, sa_id=%u]\n", secy_device->Role, sc_index, sa_index, vPortId, SAIndex);

    return 0;
}

/*----------------------------------------------------------------------------
 * phy_macsec_sa_chain
 */
static int phy_macsec_sa_chain(secy_device_t* secy_device, uint32_t sa_index_active, uint32_t sa_index, macsec_sa_t *SA_p)
{
    int Rc;
    uint32_t NewSAIndex, ActiveSAIndex, ActiveSCIndex, SAIndex0, SAIndex1, SAMFlowCtrlIndex, OldCtxID;
    SABuilder_UpdCtrl_Params_t SAUpdateCtrWord;
    uint8_t SAInUse0, SAInUse1;

    if (sa_index >= SA_NUM_MAX)
    {
        phy_macsec_log(LOG_ERROR, "invalid sa_index %u\n", sa_index);
        return -1;
    }

    Rc = macsec_SAHandleToIndex(secy_device, sa_index_active, &ActiveSAIndex, &ActiveSCIndex, &SAMFlowCtrlIndex);
    if (Rc)
    {
        phy_macsec_log(LOG_ERROR, "macsec_SAHandleToIndex failed\n");
        return -1;
    }

    phy_macsec_log(LOG_DEBUG, "Chaining SA [vport=%u, sa_index_active=%u, sa_index=%u, sa_id_active=%u]\n", SAMFlowCtrlIndex, sa_index_active, sa_index, ActiveSAIndex);

    /* Obtain a free SA for this XtSecY device */
    {
        List_Element_t * Element_p;
        secy_sa_descriptor_t *NewSADscr_p;
        secy_sa_descriptor_t *SADscr_p = (secy_sa_descriptor_t*)secy_device->sa_handlers[sa_index_active].p;
        secy_sc_descriptor_t *SCDscr_p = &(secy_device->SCDscr_p[SADscr_p->InUse.SCIndex]);
        List_Status_t List_Rc = macsec_List_RemoveFromTail(0, secy_device->SAFL_p, &Element_p);
        if (List_Rc != LIST_STATUS_OK || Element_p->DataObject_p == NULL)
        {
            phy_macsec_log(LOG_ERROR, " Failed to obtain a free SA for EIP-160 device. error %d\n", List_Rc);
            return -1;
        }

        /* Convert SA descriptor to SA index and to SA handle */
        NewSADscr_p = (secy_sa_descriptor_t*)Element_p->DataObject_p;
        NewSAIndex = NewSADscr_p - secy_device->SADscr_p;
        NewSADscr_p->InUse.SAIndex = NewSAIndex;
        NewSADscr_p->InUse.SAMFlowCtrlIndex = SAMFlowCtrlIndex;
        NewSADscr_p->InUse.SCIndex = ActiveSCIndex;
        NewSADscr_p->InUse.AN = 0;
        NewSADscr_p->InUse.MapType = SECY_SA_MAP_EGRESS;

        secy_device->sa_handlers[sa_index].p = NewSADscr_p;
        SCDscr_p->SAHandle[0] = secy_device->sa_handlers[sa_index];
        SADscr_p->InUse.MapType = SECY_SA_MAP_DETACHED;
    }

    if (SA_p->sa_word_count > 0)
    {
        macsec_EIP160_XFORM_REC_RD(&secy_device->secy_device_access, NewSAIndex, 1, 1, &OldCtxID);

        /* Increment 24 most significant bits of CtxID so it is unique. */
        SA_p->transform_record[1]=OldCtxID + 0x100;
        //phy_macsec_log(LOG_ERROR, " OldCtxID=%x\n", OldCtxID);
        /* Write transform record to device. */
        macsec_EIP160_XFORM_REC_WR(&secy_device->secy_device_access, NewSAIndex, SA_p->transform_record, SA_p->sa_word_count);

    }

    /* Clear SA statistics */
    if (SA_p->action_type == MACSEC_SA_ACTION_EGRESS)
    {
        /* Reset/read egress SA statistics counters */
        Rc = macsec_EIP160_SecY_SA_Stat_E_Clear(secy_device, NewSAIndex);
        if (Rc)
        {
            phy_macsec_log(LOG_ERROR, " Failed to reset egress SA statistics\n");
            return -1;
        }
    }

    /* Enable automatic expiration of new SA. */
    SAUpdateCtrWord.SCIndex = SAMFlowCtrlIndex;
    SAUpdateCtrWord.AN = 0;
    SAUpdateCtrWord.SAIndex = 0;
    SAUpdateCtrWord.fSAIndexValid = 0;
    SAUpdateCtrWord.fUpdateEnable = 1;
    SAUpdateCtrWord.fExpiredIRQ = 1;
    SAUpdateCtrWord.fUpdateTime = 0;

    Rc = macsec_SA_Update_Control_Word_Update(&secy_device->secy_device_access, NewSAIndex, &SAUpdateCtrWord);
    if (Rc)
    {
        phy_macsec_log(LOG_ERROR, " Failed to update SA update control word for device %d \n", secy_device->Role);
        return -1;
    }

    /* Chain SA */
    SAUpdateCtrWord.SCIndex = SAMFlowCtrlIndex;
    SAUpdateCtrWord.AN = 0;
    SAUpdateCtrWord.SAIndex = NewSAIndex;
    SAUpdateCtrWord.fSAIndexValid = 1;
    SAUpdateCtrWord.fUpdateEnable = 1;
    SAUpdateCtrWord.fExpiredIRQ = 1;
    SAUpdateCtrWord.fUpdateTime = 0;

    Rc = macsec_SA_Update_Control_Word_Update(&secy_device->secy_device_access, ActiveSAIndex, &SAUpdateCtrWord);
    if (Rc)
    {
        phy_macsec_log(LOG_ERROR, " Failed to update SA update control word for device %d \n", secy_device->Role);
        return -1;
    }

    /* Sync */
    Rc = macsec_Device_Sync(secy_device);

    macsec_EIP160_SC_SA_MAP1_RD(&secy_device->secy_device_access, SAMFlowCtrlIndex, &SAIndex0, &SAInUse0, &SAIndex1, &SAInUse1);

    if (SAInUse0)
    {
        phy_macsec_log(LOG_INFO, "DONE: Chaining SA [vport=%u, sa_index_active=%u, sa_index=%u, sa_id_active=%u, sa_id=%u]\n", SAMFlowCtrlIndex, sa_index_active, sa_index, ActiveSAIndex, NewSAIndex);

        return 0;
    }
    else
    {
        phy_macsec_log(LOG_ERROR, "Chaining SA failed [vport=%u, sa_index_active=%u, sa_index=%u, sa_id_active=%u, sa_id=%u]\n", SAMFlowCtrlIndex, sa_index_active, sa_index, ActiveSAIndex, NewSAIndex);
        return -1;
    }
}


/*----------------------------------------------------------------------------
 * phy_macsec_SA_Switch
 */
static int phy_macsec_sa_switch(secy_device_t* secy_device, uint32_t sa_index_active, uint32_t sa_index)
{
    int Rc;
    uint32_t NewSAIndex, ActiveSAIndex, SAIndex0, SAIndex1, SAMFlowCtrlIndex;
    uint8_t SAInUse0, SAInUse1;
    secy_sa_descriptor_t *SADscr_p;

    Rc = macsec_SAHandleToIndex(secy_device, sa_index_active, &ActiveSAIndex, NULL, &SAMFlowCtrlIndex);
    if (Rc)
    {
        phy_macsec_log(LOG_ERROR, "macsec_SAHandleToIndex failed\n");
        return -1;
    }

    Rc = macsec_SAHandleToIndex(secy_device, sa_index, &NewSAIndex, NULL, NULL);
    if (Rc)
    {
        phy_macsec_log(LOG_ERROR, "macsec_SAHandleToIndex failed\n");
        return -1;
    }

    phy_macsec_log(LOG_DEBUG, "Switch SA [vport=%u, sa_index_active=%u, sa_index=%u, sa_id_active=%u, sa_id=%u]\n", SAMFlowCtrlIndex, sa_index_active, sa_index, ActiveSAIndex, NewSAIndex);

    SADscr_p = (secy_sa_descriptor_t*)secy_device->sa_handlers[sa_index_active].p;
    macsec_EIP160_SC_SA_MAP1_WR(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, NewSAIndex, 1, 1, 0, 0);

    /* Sync */
    Rc = macsec_Device_Sync(secy_device);

    macsec_EIP160_SC_SA_MAP1_RD(&secy_device->secy_device_access, SAMFlowCtrlIndex, &SAIndex0, &SAInUse0, &SAIndex1, &SAInUse1);

    if (SAInUse0)
    {
        phy_macsec_log(LOG_INFO, "DONE: Switch SA [vport=%u, sa_index_active=%u, sa_index=%u, sa_id_active=%u, sa_id=%u]\n", SAMFlowCtrlIndex, sa_index_active, sa_index, ActiveSAIndex, NewSAIndex);

        return 0;
    }
    else
    {
        phy_macsec_log(LOG_ERROR, "Switch SA failed [vport=%u, sa_index_active=%u, sa_index=%u, sa_id_active=%u, sa_id=%u]\n", SAMFlowCtrlIndex, sa_index_active, sa_index, ActiveSAIndex, NewSAIndex);
        return -1;
    }
}


static int phy_macsec_sa_remove(secy_device_t* secy_device, uint32_t sc_index, uint32_t sa_index)
{
    int Rc;
    uint32_t vPort;
    secy_sa_descriptor_t * SADscr_p;

    if (sa_index >= SA_NUM_MAX)
    {
        phy_macsec_log(LOG_ERROR, "invalid sa_index %u\n", sa_index);
        return -1;
    }

    if (sc_index > VPORT_NUM_MAX)
    {
        phy_macsec_log(LOG_ERROR, "invalid sc_index %u\n", sc_index);
        return -1;
    }
    
    SADscr_p = secy_device->sa_handlers[sa_index].p;

    if (SADscr_p == NULL)
    {
        phy_macsec_log(LOG_DEBUG, "\nphy_macsec_sa_remove [direction=%u, sc_index=%u, sa_index=%u] does not exist\n", secy_device->Role, sc_index, sa_index);
        return 0;
    }

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_sa_remove [direction=%u, sc_index=%u, sa_index=%u]\n", secy_device->Role, sc_index, sa_index);

    vPort = SADscr_p->InUse.SAMFlowCtrlIndex;

    if (vPort >= secy_device->vPortCount)
    {
        phy_macsec_log(LOG_ERROR, "vPort[%u] >= secy_device->vPortCount[%u]\n", vPort, secy_device->vPortCount);
        return -1;
    }

    if (SADscr_p->InUse.MapType != SECY_SA_MAP_DETACHED)
    {
        secy_sc_descriptor_t *SCDscr_p;

        if (SADscr_p->InUse.SCIndex == 0xffffffff)
        {
            phy_macsec_log(LOG_ERROR, "SCIndex=0xffffffff\n");
            return -1;
        }

        SCDscr_p = &secy_device->SCDscr_p[SADscr_p->InUse.SCIndex];

        /* SA has not been detached from SC by overwriting in plp_longfin_XtSecY_SA_Add */
        if ((SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS) ||
            (SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS_CRYPT_AUTH))
        {
            if (SADscr_p->InUse.MapType == SECY_SA_MAP_INGRESS)
            {
                    /* Unmap the SA from the device. */
                    Rc = macsec_EIP160_SecY_SC_SA_Map_I_Update(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, SADscr_p->InUse.AN, SADscr_p->InUse.SAIndex, 0);
                    if (Rc)
                    {
                        phy_macsec_log(LOG_ERROR, " Failed unmap SA for EIP-160 device %d\n", secy_device->Role);
                        return -1;
                    }
                    SCDscr_p->SAHandle[SADscr_p->InUse.AN] = macsec_XtSecY_SAHandle_NULL;
            }
            else
            {
                int i;
                /* Crypt-authenticate. */
                for (i=0; i<4; i++)
                {
                    Rc = macsec_EIP160_SecY_SC_SA_Map_I_Update(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, i, SADscr_p->InUse.SAIndex, 0);
                    if (Rc)
                    {
                        phy_macsec_log(LOG_ERROR, " Failed unmap SA for EIP-160 device %d\n", secy_device->Role);
                        return -1;
                    }

                    SCDscr_p->SAHandle[i] = macsec_XtSecY_SAHandle_NULL;
                }
            }

            if (macsec_SAHandle_IsSame(&SCDscr_p->SAHandle[0], &macsec_XtSecY_SAHandle_NULL) && macsec_SAHandle_IsSame(&SCDscr_p->SAHandle[1], &macsec_XtSecY_SAHandle_NULL) &&
                macsec_SAHandle_IsSame(&SCDscr_p->SAHandle[2], &macsec_XtSecY_SAHandle_NULL) && macsec_SAHandle_IsSame(&SCDscr_p->SAHandle[3], &macsec_XtSecY_SAHandle_NULL))
            {
                /* All four AN slots are empty now, remove CAM entry. */
                List_Status_t List_Rc;
                SCDscr_p->MapType = SECY_SA_MAP_DETACHED;

                /* Remove element from vPort specific search list. */
                List_Rc = macsec_List_RemoveAnywhere(0, secy_device->vPortDscr_p[vPort].InUse.SCList, &secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].OnList);
                if (List_Rc != LIST_STATUS_OK)
                {
                    phy_macsec_log(LOG_ERROR, " Failed to remove SC from lookup list for device %d\n", secy_device->Role);
                    return -1;
                }

                /* Add it to the free list */
                List_Rc = macsec_List_AddToHead(0, secy_device->SCFL_p, &secy_device->SCDscr_p[SADscr_p->InUse.SCIndex].OnList);
                if (List_Rc != LIST_STATUS_OK)
                {
                    phy_macsec_log(LOG_ERROR, " Failed to free an SC for device %d\n", secy_device->Role);
                    return -1;
                }

                Rc = macsec_EIP160_SecY_RxCAM_Remove(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex);
                if (Rc)
                {
                    phy_macsec_log(LOG_ERROR, " Failed unmap SA for device %d \n", secy_device->Role);
                    return -1;
                }
            }
        }
        else
        {
            /* Unmap the SA from the device. */
            macsec_EIP160_SC_SA_MAP1_WR(&secy_device->secy_device_access, SADscr_p->InUse.SCIndex, SADscr_p->InUse.SAIndex, 0, 0, 0, 0);
            SCDscr_p->SAHandle[0] = macsec_XtSecY_SAHandle_NULL;
        }

        /* If last SA removed, clear SAMFlow entry. */
        secy_device->vPortDscr_p[vPort].InUse.SACount--;
        
        if (secy_device->vPortDscr_p[vPort].InUse.SACount == 0)
            macsec_EIP160_SAM_FLOW_CTRL1_BYPASS_WR(&secy_device->secy_device_access, vPort, 0, 0, 0, 0, 0);
    }

    /* Add SA to be removed to the SA free list */
    {
        List_Status_t List_Rc;
        uint32_t SAIndex = SADscr_p->InUse.SAIndex;
        secy_device->SADscr_p[SAIndex].free.DataObject_p = &secy_device->SADscr_p[SAIndex];

        List_Rc = macsec_List_AddToHead(0, secy_device->SAFL_p, &secy_device->SADscr_p[SAIndex].free);
        if (List_Rc != LIST_STATUS_OK)
        {
            phy_macsec_log(LOG_ERROR, " Failed to free an SA for device %d\n", secy_device->Role);
            return -1;
        }
    }

    secy_device->sa_handlers[sa_index] = macsec_XtSecY_SAHandle_NULL;

    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_sa_remove [direction=%u, sc_index=%u, sa_index=%u, vPort=%u, SAIndex=%u]\n", secy_device->Role, sc_index, sa_index, vPort, SADscr_p->InUse.SAIndex);

    return 0;
}

static void phy_macsec_rule_prepare(macsec_rule_t *ruleparams, macsec_api_rule_t *api_rule)
{
    ruleparams->mask.packet_type = MACSEC_RULE_PKT_TYPE_OTHER;
    ruleparams->mask.num_tags = api_rule->num_tags;

    ruleparams->key.packet_type = MACSEC_RULE_PKT_TYPE_OTHER;
    ruleparams->key.num_tags = api_rule->num_tags;

    memcpy(ruleparams->data_mask, api_rule->data_mask, sizeof(uint32_t) * MACSEC_RULE_NON_CTRL_WORD_COUNT);
    memcpy(ruleparams->data, api_rule->data, sizeof(uint32_t) * MACSEC_RULE_NON_CTRL_WORD_COUNT);

    ruleparams->policy.priority = api_rule->priority;
    ruleparams->policy.fcontrol_packet = api_rule->fcontrol_packet;
    ruleparams->policy.fdrop = api_rule->fdrop;
}

static int phy_macsec_rule_add(secy_device_t* secy_device, uint32_t sc_index, uint32_t rule_index, macsec_rule_t *Rule_p)
{
    uint32_t RuleId, vPortId;  
    int rc = 0;

    vPortId = macsec_vPortId_Get(secy_device, sc_index);

    if (vPortId == -1)
        return -1;

    if (rule_index >= RULE_NUM_MAX)
    {
        phy_macsec_log(LOG_ERROR, "invalid rule_index %u\n", rule_index);
        return -1;
    }

    if (secy_device->rule_handlers[rule_index].p)
    {
        phy_macsec_log(LOG_DEBUG, "rule_index=[%u] already exists\n", rule_index);
        return MACSEC_RET_ALREADY_INIT;
    }

    Rule_p->policy.vport_id = vPortId;

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_rule_add [direction=%u, sc_index=%u, rule_index=%u, vPort=%u]\n", secy_device->Role, sc_index, rule_index, Rule_p->policy.vport_id);

    {
        List_Element_t * RuleElmt_p;
        secy_rule_descriptor_t * RuleDscr_p;
        void * const RFL_p = secy_device->RuleFreeList_p;

        List_Status_t List_Rc = macsec_List_RemoveFromTail(0, RFL_p, &RuleElmt_p);
        if (List_Rc != LIST_STATUS_OK || RuleElmt_p->DataObject_p == NULL)
        {
            phy_macsec_log(LOG_ERROR, " Failed to obtain a free Rule for EIP-160 device %d, error %d\n", secy_device->Role, List_Rc);
            return -1;
        }

        /* Convert vPort descriptor to vPort index and handle */
        RuleDscr_p = (secy_rule_descriptor_t*)RuleElmt_p->DataObject_p;
        RuleId = RuleDscr_p - secy_device->RuleDscr_p;
        RuleDscr_p->InUse.RuleId = RuleId;
        RuleDscr_p->InUse.vPortDscr_p = secy_device->vport_handlers[sc_index].p;
        RuleDscr_p->InUse.vPortDscr_p->InUse.BoundRulesCount++;
        secy_device->rule_handlers[rule_index].p = RuleDscr_p;
    }

    /* Now we have RuleId */
    rc = macsec_Rule_Update(secy_device, RuleId, Rule_p);

    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_rule_add [direction=%u, sc_index=%u, rule_index=%u, vPort=%u, RuleId=%u]\n", secy_device->Role, sc_index, rule_index, Rule_p->policy.vport_id, RuleId);

    return rc;
}

static int phy_macsec_rule_remove(secy_device_t* secy_device, uint32_t sc_index, uint32_t rule_index)
{
    uint32_t RuleId;
    
    RuleId = macsec_RuleId_Get(secy_device, rule_index);

    if (RuleId == -1)
        return -1;

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_rule_remove [direction=%u, sc_index=%u, rule_index=%u, ruleId=%u]\n", secy_device->Role, sc_index, rule_index, RuleId);

    macsec_EIP160_TCAM_POLICY_DEFAULT_WR(&secy_device->secy_device_access, RuleId);

    /* Add removed Rule to the Rule free list */
    {
        List_Status_t List_Rc;
        void * const RFL_p = secy_device->RuleFreeList_p;
        List_Element_t * const RuleElmt_p = &secy_device->RuleDscr_p[RuleId].free;

        secy_device->RuleDscr_p[RuleId].InUse.vPortDscr_p->InUse.BoundRulesCount--;
        secy_device->RuleDscr_p[RuleId].InUse.vPortDscr_p = NULL;
        RuleElmt_p->DataObject_p = &secy_device->RuleDscr_p[RuleId]; 

        List_Rc = macsec_List_AddToHead(0, RFL_p, RuleElmt_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            phy_macsec_log(LOG_ERROR, " Failed to add Rule_id=%d to free list for device %d \n", RuleId, secy_device->Role);
            return -1;
        }
    }

    secy_device->rule_handlers[rule_index] = macsec_XtSecY_RuleHandle_NULL;

    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_rule_remove [direction=%u, sc_index=%u, rule_index=%u, ruleId=%u]\n", secy_device->Role, sc_index, rule_index, RuleId);

    return 0;
}

#if 0
static int phy_macsec_rule_update(secy_device_t* secy_device, uint32_t sc_index, uint32_t rule_index, macsec_rule_t *Rule_p)
{
    uint32_t RuleId;
    
    int rc = 0;

    Rule_p->policy.vport_id = macsec_vPortId_Get(secy_device, sc_index);

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_rule_update [direction=%u, sc_index=%u, rule_index=%u, vPort=%u]\n", secy_device->Role, sc_index, rule_index, Rule_p->policy.vport_id);

    RuleId = macsec_RuleId_Get(secy_device, rule_index);

    if (secy_device->RuleDscr_p[RuleId].InUse.vPortDscr_p == NULL)
    {
        phy_macsec_log(LOG_ERROR, " Failed, missing vPort for rule_id %u for device %d\n", RuleId, secy_device->Role);
        return -1;
    }

    rc = macsec_Rule_Update(secy_device, RuleId, Rule_p);

    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_rule_update [direction=%u, sc_index=%u, rule_index=%u, vPort=%u, RuleId=%u]\n", secy_device->Role, sc_index, rule_index, Rule_p->policy.vport_id, RuleId);

    return rc;
}
#endif

static int phy_macsec_rule_enable(secy_device_t* secy_device, uint32_t rule_index, uint8_t enable, uint8_t fSync)
{
    uint32_t RuleId;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    RuleId = macsec_RuleId_Get(secy_device, rule_index);

    if (RuleId == -1)
        return -1;

    phy_macsec_log(LOG_DEBUG, "\nphy_macsec_rule_enable [%s: direction=%u, rule_index=%u]\n", enable ? "enable" : "disable", secy_device->Role, rule_index);

    if (secy_device->RuleDscr_p[RuleId].InUse.vPortDscr_p == NULL)
    {
        phy_macsec_log(LOG_ERROR, " Failed, missing vPort for rule_id=%d for EIP-160 device %d \n", RuleId, secy_device->Role);
        return -1;
    }

    if (enable)
        macsec_EIP160_SAM_ENTRY_SET_WR(&secy_device->secy_device_access, RuleId / 32, BIT_0 << (RuleId % 32));
    else
        macsec_EIP160_SAM_ENTRY_CLEAR_WR(&secy_device->secy_device_access, RuleId / 32, BIT_0 << (RuleId % 32));


    phy_macsec_log(LOG_INFO, "DONE: phy_macsec_rule_enable [%s: direction=%u, rule_index=%u, RuleId=%u]\n", enable ? "enable" : "disable", secy_device->Role, rule_index, RuleId);

    return 0;
}

static int macsec_prepare_fw(phy_dev_t *phy_dev, int enable)
{
    int ret = 0;
    uint16_t  data = 0, temp = 0;
    uint16_t strap_status;
    time_t start_t;
    struct timeval tv;
    

    PHY_READ(phy_dev, 0x1E, 0x401c, &temp);
    temp |= (0x100);
    PHY_WRITE(phy_dev, 0x1E, 0x401c, temp);

    /* make sure handshake bit is cleared */
    do_gettimeofday(&tv);
    start_t = tv.tv_sec;

    PHY_READ(phy_dev, 0x1E, 0x400e, &strap_status);

    while(strap_status & 0x2) 
    {
        PHY_READ(phy_dev, 0x1E, 0x400e, &strap_status);
        do_gettimeofday(&tv);
        if(tv.tv_sec - start_t > STRAP_CHANGE_TIMEOUT)
        {
            phy_macsec_log(LOG_ERROR, "Failed on timeout\n");
            return -1;
        }
    }

    temp = 0;
    PHY_READ(phy_dev, 0x1E, 0x401c, &temp);
    if (!(temp & 0x100))
    {
        phy_macsec_log(LOG_ERROR, "super isolate failed\n");
        return -1;
    }

    PHY_READ(phy_dev, 0x1E, PHY_REG_FW_NOTIFY, &data);
    data &= ~PHY_REG_FW_NOTIFY_MACSEC;
    PHY_WRITE(phy_dev, 0x1E, PHY_REG_FW_NOTIFY, data | enable);

    PHY_READ(phy_dev, 0x1E, 0x401c, &temp);
    temp &= (~0x100);
    PHY_WRITE(phy_dev, 0x1E, 0x401c, temp);

    /* make sure handshake bit is cleared */
    do_gettimeofday(&tv);
    start_t = tv.tv_sec;

    PHY_READ(phy_dev, 0x1E, 0x400e, &strap_status);

    while(strap_status & 0x2) 
    {
        PHY_READ(phy_dev, 0x1E, 0x400e, &strap_status);
        do_gettimeofday(&tv);
        if(tv.tv_sec - start_t > STRAP_CHANGE_TIMEOUT)
        {
            phy_macsec_log(LOG_ERROR, "Failed on timeout\n");
            return -1;
        }
    }

    temp = 0;
    PHY_READ(phy_dev, 0x1E, 0x401c, &temp);
    if (temp & 0x100)
    {
        phy_macsec_log(LOG_ERROR, "super isolate failed\n");
        return -1;
    }

    phy_macsec_log(LOG_INFO, "firmware clock monitor enabled\n");

Exit:
    return 0;
}

static int macsec_xlmac_powerup_init(phy_dev_t *phy_dev, macsec_port_loc_t port_loc)
{
    XLMAC_RX_CTRLr_t rx_ctrl;
    XLMAC_TX_CTRLr_t tx_ctrl;
    XLMAC_PFC_CTRLr_t pfc_ctrl;
    XLMAC_RX_MAX_SIZEr_t rx_maxsize;
   
    XLMAC_TX_CTRLr_CLR(tx_ctrl);
    XLMAC_RX_CTRLr_CLR(rx_ctrl);
    XLMAC_PFC_CTRLr_CLR(pfc_ctrl);
    XLMAC_RX_MAX_SIZEr_CLR(rx_maxsize);

    /* XLMAC_RX_CTRL */
    macsec_xlmac_reg64_read(phy_dev, port_loc, XLMAC_RX_CTRLr, &(rx_ctrl._xlmac_rx_ctrl));
    XLMAC_RX_CTRLr_STRICT_PREAMBLEf_SET(rx_ctrl, 1);
    XLMAC_RX_CTRLr_RUNT_THRESHOLDf_SET(rx_ctrl, 0x40);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_RX_CTRLr, rx_ctrl._xlmac_rx_ctrl);

    /* XLMAC_TX_CTRL */
    macsec_xlmac_reg64_read(phy_dev, port_loc, XLMAC_TX_CTRLr, &(tx_ctrl._xlmac_tx_ctrl));
    XLMAC_TX_CTRLr_AVERAGE_IPGf_SET(tx_ctrl, 0x0c);
    XLMAC_TX_CTRLr_CRC_MODEf_SET(tx_ctrl, 0);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_TX_CTRLr, tx_ctrl._xlmac_tx_ctrl);

    /* PFC */
    macsec_xlmac_reg64_read(phy_dev, port_loc, XLMAC_PFC_CTRLr, &(pfc_ctrl._xlmac_pfc_ctrl));
    XLMAC_PFC_CTRLr_PFC_REFRESH_ENf_SET(pfc_ctrl, 1);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_PFC_CTRLr, pfc_ctrl._xlmac_pfc_ctrl);

    /* RX_MAX_SIZE */
    macsec_xlmac_reg64_read(phy_dev, port_loc, XLMAC_RX_MAX_SIZEr, &(rx_maxsize._xlmac_rx_max_size));
    XLMAC_RX_MAX_SIZEr_RX_MAX_SIZEf_SET(rx_maxsize, 0x3FFF);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_RX_MAX_SIZEr, rx_maxsize._xlmac_rx_max_size);

    return 0;
}

static int macsec_xlmac_init(phy_dev_t *phy_dev, macsec_port_loc_t port_loc)
{
    XLMAC_CTRLr_t     mac_ctrl;
    XLMAC_MODEr_t     mac_mode;
    XLMAC_TX_CTRLr_t  mac_tx_ctrl;
    XLMAC_RX_CTRLr_t  mac_rx_ctrl;

    XLMAC_CTRLr_CLR(mac_ctrl);
    XLMAC_MODEr_CLR(mac_mode);
    XLMAC_TX_CTRLr_CLR(mac_tx_ctrl);
    XLMAC_RX_CTRLr_CLR(mac_rx_ctrl);

    /* XLMAC_REGS_XLMAC_CTRL = 0x00000003 */
    XLMAC_CTRLr_RX_ENf_SET(mac_ctrl, 1);
    XLMAC_CTRLr_TX_ENf_SET(mac_ctrl, 1);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_CTRLr, mac_ctrl._xlmac_ctrl);

    /* always set XLMAC_REGS_XLMAC_MODE.SPEED_MODE = 0x4        
    for all 100M/1G/2.5G/5G/10G speeds  to avoid packet drop */
    XLMAC_MODEr_SPEED_MODEf_SET(mac_mode, 0x4);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_MODEr, mac_mode._xlmac_mode);

    /* need to write XLMAC_CTRL again */
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_CTRLr, mac_ctrl._xlmac_ctrl);

    /* XLMAC_REGS_XLMAC_TX_CTRL = 0x0000110_0x0000C802 */
    /* 0x00000110 - high 32 bit*/
    XLMAC_TX_CTRLr_TX_THRESHOLDf_SET(mac_tx_ctrl, 0x04);
    XLMAC_TX_CTRLr_TX_PREAMBLE_LENGTHf_SET(mac_tx_ctrl, 0x08);
    /* 0x0000C802 - low 32 bit*/
    XLMAC_TX_CTRLr_AVERAGE_IPGf_SET(mac_tx_ctrl, (port_loc == macsecPortLocSys) ? 0x08 : 0x0c);
    XLMAC_TX_CTRLr_PAD_THRESHOLDf_SET(mac_tx_ctrl, 0x40);
    XLMAC_TX_CTRLr_PAD_ENf_SET(mac_tx_ctrl, 1);
    XLMAC_TX_CTRLr_CRC_MODEf_SET(mac_tx_ctrl, 0x02);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_TX_CTRLr, mac_tx_ctrl._xlmac_tx_ctrl);

    /* XLMAC_REGS_XLMAC_RX_CTRL = 0x0000E228 */
    XLMAC_RX_CTRLr_RX_PASS_PFCf_SET(mac_rx_ctrl, 1);
    XLMAC_RX_CTRLr_RX_PASS_PAUSEf_SET(mac_rx_ctrl, 1);
    XLMAC_RX_CTRLr_RX_PASS_CTRLf_SET(mac_rx_ctrl, 1);
    XLMAC_RX_CTRLr_RUNT_THRESHOLDf_SET(mac_rx_ctrl, 0x22);
    XLMAC_RX_CTRLr_STRICT_PREAMBLEf_SET(mac_rx_ctrl, 1);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_RX_CTRLr, mac_rx_ctrl._xlmac_rx_ctrl);

    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, 0x1018, 0x3);
    msleep(100);
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, 0x1018, 0x3);

    macsec_xlmac_restart(phy_dev, port_loc);

    return 0;
}

static int macsec_xlmac_restart(phy_dev_t *phy_dev, macsec_port_loc_t port_loc)
{
    XLMAC_CTRLr_t     mac_ctrl;

    XLMAC_CTRLr_CLR(mac_ctrl);

    /*
     *   reset XLMAC Rx/Tx
     */
    /* XLMAC_REGS_XLMAC_CTRL = 0x00000000  (disable XLMAC Rx/Tx) */
    XLMAC_CTRLr_RX_ENf_SET(mac_ctrl, 0);
    XLMAC_CTRLr_TX_ENf_SET(mac_ctrl, 0);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_CTRLr, mac_ctrl._xlmac_ctrl);
    /* XLMAC_REGS_XLMAC_CTRL = 0x00000003  ( enable XLMAC Rx/Tx) */
    XLMAC_CTRLr_RX_ENf_SET(mac_ctrl, 1);
    XLMAC_CTRLr_TX_ENf_SET(mac_ctrl, 1);
    macsec_xlmac_reg64_write(phy_dev, port_loc, XLMAC_CTRLr, mac_ctrl._xlmac_ctrl);

    return 0;
}

static int macsec_get_xfi_serdes_mode(phy_dev_t *phy_dev, uint16_t *xfi_speed, uint16_t *mode_config, uint16_t *kr_mode)
{
    int ret;
    uint16_t temp = 0;

    PHY_WRITE(phy_dev, 0x1e, 0x4107, 0x403|(phy_dev->addr<<5));
    PHY_WRITE(phy_dev, 0x1e, 0x4117, 0x2004);
    PHY_READ(phy_dev, 0x03, 0xa040, &temp);
    PHY_READ(phy_dev, 0x03, 0xa040, &temp);
    PHY_WRITE(phy_dev, 0x1e, 0x4107, 0);

    if(temp & XFIM_XFI_CTRL0_SPEED_10G) 
    {
        *xfi_speed = SPEED_10G;
        phy_macsec_log(LOG_INFO, "XFI speed=10G");
    } 
    else if(temp & XFIM_XFI_CTRL0_SPEED_5G) 
    {
        *xfi_speed = SPEED_5000;
        phy_macsec_log(LOG_INFO, "XFI speed=5G");
    } 
    else if(temp & XFIM_XFI_CTRL0_SPEED_2P5G) 
    {
        *xfi_speed = SPEED_2500;
        phy_macsec_log(LOG_INFO, "XFI speed=2.5G");
    } 
    else if(temp & XFIM_XFI_CTRL0_SPEED_1G) 
    {
        *xfi_speed = SPEED_1000;
        phy_macsec_log(LOG_INFO, "XFI speed=1G");
    } 
    else if( temp & XFIM_XFI_CTRL0_SPEED_100M) 
    {
        *xfi_speed = SPEED_100;
        phy_macsec_log(LOG_INFO, "XFI speed=100M");
    } 
    else 
    {
        *xfi_speed = SPEED_UNDEFINED;
        phy_macsec_log(LOG_ERROR, " SPEED_UNDEFINED ");
    }

#if 0
    PHY_READ(phy_dev, 0x1e, 0x4011, &temp);
    *mode_config = (temp>>1) & 0xf;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_dev, 0x1e, 0x4037, &temp);

        if (temp != CMD_IN_PROGRESS && temp != CMD_SYSTEM_BUSY)
            break;

        udelay(2000);
    }

    PHY_WRITE(phy_dev, 0x1e, 0x4005, 0x800E);

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_dev, 0x1e, 0x4037, &temp);

        if (temp == CMD_PASS || temp == CMD_ERROR)
            break;

        udelay(2000);
    }

    PHY_READ(phy_dev, 0x1e, 0x4038, kr_mode);

    phy_macsec_log(LOG_INFO, " mode=%u kr_enable=%u\n", *mode_config, *kr_mode);
#endif    


Exit:
    return ret;
}

static void macsec_settings_convert(macsec_settings_t* p_settings, macsec_api_settings_t* p_api_settings)
{
    p_settings->fAutoStatCntrsReset = 1;

    p_settings->NonControlDropBypass.Untagged.fBypass = p_api_settings->non_control_untagged_bypass;
    p_settings->NonControlDropBypass.Untagged.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->NonControlDropBypass.Tagged.fBypass = p_api_settings->non_control_tagged_bypass;
    p_settings->NonControlDropBypass.Tagged.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->NonControlDropBypass.BadTag.fBypass = p_api_settings->non_control_badtag_bypass;
    p_settings->NonControlDropBypass.BadTag.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->NonControlDropBypass.KaY.fBypass = p_api_settings->non_control_kay_bypass;
    p_settings->NonControlDropBypass.KaY.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->ControlDropBypass.Untagged.fBypass = p_api_settings->control_untagged_bypass;
    p_settings->ControlDropBypass.Untagged.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->ControlDropBypass.Tagged.fBypass = p_api_settings->control_tagged_bypass;
    p_settings->ControlDropBypass.Tagged.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->ControlDropBypass.BadTag.fBypass = p_api_settings->control_badtag_bypass;
    p_settings->ControlDropBypass.BadTag.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->ControlDropBypass.KaY.fBypass = p_api_settings->control_kay_bypass;
    p_settings->ControlDropBypass.KaY.DropType = macsec_SECY_SA_DROP_PKT_ERROR;

    p_settings->SAFControl.fEnable         = 1;
    p_settings->SAFControl.fDropClass      = 1;
    p_settings->SAFControl.fDropPP         = 1;
    p_settings->SAFControl.fDropSecFail    = 1;
    p_settings->SAFControl.fDropMTU        = 1;
    p_settings->SAFControl.fDropMACCRC     = 1;
    p_settings->SAFControl.fDropMACErr     = 1;
    p_settings->SAFControl.LowWatermark    = 0x400;
    p_settings->SAFControl.HighWatermark   = 0x600;


    p_settings->CPMatchEnableMask = p_api_settings->fcontrol_mc_group_addr ? 0x80000 : 0x0;

    p_settings->SeqNrThreshold = p_api_settings->SeqNrThreshold;
    p_settings->SeqNrThreshold64Lo = p_api_settings->SeqNrThreshold64Lo;
    p_settings->SeqNrThreshold64Hi = p_api_settings->SeqNrThreshold64Hi;
}


static int macsec_secy_init(phy_dev_t *phy_dev, macsec_settings_t* p_settings)
{
    uint32_t i;
    uint8_t Rc = 0;
    macsec_Capabilities_t Cp;
    secy_device_t *secy_device;
    secy_role_t role = p_settings->Mode;

    secy_device = &(((phy_macsec_dev*)phy_dev->macsec_dev)->secy_devices[role]);

    memset(secy_device, 0, sizeof(secy_device_t));

    secy_device->secy_device_access.phy_dev = phy_dev;
    secy_device->secy_device_access.base_addr = role ? MACSEC_INGRESS_REG_BASE : MACSEC_EGRESS_REG_BASE;
    secy_device->Role = role;

    memset(&Cp, 0, sizeof(macsec_Capabilities_t));

    /* Request the device initialization is done */
    Rc = macsec_device_init(&secy_device->secy_device_access, p_settings, &Cp);
    if (Rc)
    {
        phy_macsec_log(LOG_ERROR, "macsec_device_init failed\n");
        return Rc;
    }

    /* Show those capabilities not propagated to higher layer. */
    phy_macsec_log(LOG_INFO, "macsec device version major/minor/patch: %d/%d/%d\n",
             Cp.EIP160_Version.MajHWRevision,
             Cp.EIP160_Version.MinHWRevision,
             Cp.EIP160_Version.HWPatchLevel);
    phy_macsec_log(LOG_DEBUG, "device capabilities\n");
    phy_macsec_log(LOG_DEBUG, "\tEgress only:               %s\n"
             "\tIngress only:              %s\n"
             "\tNof SA's:                  %d\n"
             "\tNof SC's:                  %d\n"
             "\tNof vPorts:                %d\n"
             "\tNof Rules:                 %d\n"
             "\tNof SA counters:           %d\n"
             "\tNof VLan counters:         %d\n"
             "\tNof Global counters:       %d\n",
             macsec_YesNo(Cp.EIP160_Options.fEgressOnly),
             macsec_YesNo(Cp.EIP160_Options.fIngressOnly),
             Cp.EIP160_Options.SA_Count,
             Cp.EIP160_Options.SC_Count,
             Cp.EIP160_Options.vPort_Count,
             Cp.EIP160_Options.Rule_Count,
             Cp.EIP160_Options.SA_Counters,
             Cp.EIP160_Options.VLAN_Counters,
             Cp.EIP160_Options.Global_Counters
             );

    phy_macsec_log(LOG_DEBUG, "macsec device (EIP-62) version major/minor/patch: %d/%d/%d\n",
             Cp.EIP62_Version.MajHWRevision,
             Cp.EIP62_Version.MinHWRevision,
             Cp.EIP62_Version.HWPatchLevel);
    phy_macsec_log(LOG_DEBUG, "macsec device (EIP-62) capabilities\n");
    phy_macsec_log(LOG_DEBUG, "\tFPGA solution:             %s\n"
             "\tGalois fields s-boxes:     %s\n"
             "\tLookup table s-boxes:      %s\n"
             "\tMACsec AES only:           %s\n"
             "\tAES present:               %s\n"
             "\tAES feedback mode:         %s\n"
             "\tAES Speed (engines):       %d\n"
             "\tAES KeyLengths:            %d\n"
             "\tParameter bits:            %d\n"
             "\tIPSec support:             %s\n"
             "\tHeader extension support:  %s\n"
             "\tSecTAG Offset support:     %s\n"
             "\tGHash present:             %s\n"
             "\tOne cycle core:            %s\n",
             macsec_YesNo(Cp.EIP62_Options.fFPGASolution),
             macsec_YesNo(Cp.EIP62_Options.fGFSboxes),
             macsec_YesNo(Cp.EIP62_Options.fLookupSboxes),
             macsec_YesNo(Cp.EIP62_Options.fMACsecAESOnly),
             macsec_YesNo(Cp.EIP62_Options.fAESPresent),
             macsec_YesNo(Cp.EIP62_Options.fAESFb),
             Cp.EIP62_Options.AESSpeed,
             Cp.EIP62_Options.KeyLengths,
             Cp.EIP62_Options.EopParamBits,
             macsec_YesNo(Cp.EIP62_Options.fIPSec),
             macsec_YesNo(Cp.EIP62_Options.fHdrExtension),
             macsec_YesNo(Cp.EIP62_Options.fSecTAGOffset),
             macsec_YesNo(Cp.EIP62_Options.fGHASHPresent),
             macsec_YesNo(!Cp.EIP62_Options.fOneCycleCore));

    phy_macsec_log(LOG_DEBUG, "macsec device (EIP-217) version major/minor/patch: %d/%d/%d\n",
             Cp.TCAM_Version.MajHWRevision,
             Cp.TCAM_Version.MinHWRevision,
             Cp.TCAM_Version.HWPatchLevel);
    phy_macsec_log(LOG_DEBUG, "macsec device (EIP-217) capabilities\n");
    phy_macsec_log(LOG_DEBUG, "\tNof TCAM hit counters:            %d\n"
             "\tTCAM hit counter size (bits):     %d\n"
             "\tNof TCAM packet counters:         %d\n"
             "\tNof TCAM byte counters:           %d\n"
             "\tNof TCAM global packet counters:  %d\n"
             "\tNof TCAM global byte counters:    %d\n\n",
             Cp.TCAM_Options.TCAMHitCounters_Count,
             Cp.TCAM_Options.TCAMHitCountersWidth_BitCount,
             Cp.TCAM_Options.TCAMHitPktCounters_Count,
             Cp.TCAM_Options.TCAMHitByteCounters_Count,
             Cp.TCAM_Options.TCAMHitPktCountersGlobal_Count,
             Cp.TCAM_Options.TCAMHitByteCountersGlobal_Count);

    /* These clock control functions can be used to fine-grained */
    /* clock control, for now only OFF/ON sequence is done */

    /* Clock control, disable all EIP-160 clock signals */
    macsec_EIP160_FORCE_CLOCK_WR(&secy_device->secy_device_access, EIP160_REG_FORCE_CLOCK_OFF, 
                             EIP160_SECY_DEVICE_PE_CLOCK  |
                             EIP160_SECY_DEVICE_ICE_CLOCK |
                             EIP160_SECY_DEVICE_FLOW_CLOCK |
                             ((p_settings->Mode == SECY_ROLE_INGRESS) ?
                             EIP160_SECY_DEVICE_RXCAM_CLOCK : 0));

    /* Clock control, restore all EIP-160 clock signals, */
    /* e.g. go back to the dynamic clock control */
    macsec_EIP160_FORCE_CLOCK_WR(&secy_device->secy_device_access, EIP160_REG_FORCE_CLOCK_OFF, 0);

    /* Clock control, enable all EIP-160 clock signals */
        /* Clock control, disable all EIP-160 clock signals */
    macsec_EIP160_FORCE_CLOCK_WR(&secy_device->secy_device_access, EIP160_REG_FORCE_CLOCK_ON, 
                             EIP160_SECY_DEVICE_PE_CLOCK  |
                             EIP160_SECY_DEVICE_ICE_CLOCK |
                             EIP160_SECY_DEVICE_FLOW_CLOCK |
                             ((p_settings->Mode == SECY_ROLE_INGRESS) ?
                             EIP160_SECY_DEVICE_RXCAM_CLOCK : 0));

    /* Clock control, restore all EIP-160 clock signals, */
    /* e.g. go back to the dynamic clock control, */
    macsec_EIP160_FORCE_CLOCK_WR(&secy_device->secy_device_access, EIP160_REG_FORCE_CLOCK_ON, 0);


    if ((Cp.EIP160_Options.SA_Counters == 0) ||
            (Cp.EIP160_Options.Global_Counters == 0) ||
            (Cp.EIP160_Options.SA_Count == 0))
    {
        phy_macsec_log(LOG_ERROR, " Failed, the number of SA or Global counters if the number of SAs cannot be zero\n");
        return -1;
    }

    secy_device->SACounters     = Cp.EIP160_Options.SA_Counters;
    secy_device->VLANCounters   = Cp.EIP160_Options.VLAN_Counters;
    secy_device->GlobalCounters = Cp.EIP160_Options.Global_Counters;
    secy_device->SACount        = Cp.EIP160_Options.SA_Count;
    secy_device->SCCount        = Cp.EIP160_Options.SC_Count;
    secy_device->vPortCount     = Cp.EIP160_Options.vPort_Count;
    secy_device->RuleCount      = Cp.EIP160_Options.Rule_Count;
    secy_device->Role           = role;

    secy_device->vPortDscr_p     = NULL;
    secy_device->vPortFreeList_p = NULL;
    secy_device->RuleDscr_p      = NULL;
    secy_device->RuleFreeList_p  = NULL;

    /* Initialize the SA free list for this device */
    if (macsec_secy_list_init(&secy_device->SAFL_p, OBJECT_SA_T, (void**)&secy_device->SADscr_p, secy_device->SACount))
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate SA free list\n");
        kfree(secy_device->SAFL_p);
        kfree(secy_device->SADscr_p);
        secy_device->SAFL_p = NULL;
        secy_device->SADscr_p = NULL;
        return -1;
    }

    /* Initialize the SC free list for this device */
    if (macsec_secy_list_init(&secy_device->SCFL_p, OBJECT_SC_T, (void**)&secy_device->SCDscr_p, secy_device->SCCount))
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate SC free list\n");
        kfree(secy_device->SCFL_p);
        kfree(secy_device->SCDscr_p);
        secy_device->SCFL_p = NULL;
        secy_device->SCDscr_p = NULL;
        return -1;
    }

    for (i = 0; i < secy_device->SCCount; i++)
    {
        /* Set MapType to DETACHED */
        secy_device->SCDscr_p[i].MapType = SECY_SA_MAP_DETACHED;
        secy_device->SCDscr_p[i].SCIndex = i;
    } /* SC free list initialization done */

    /* Initialize the vPort list for this device */
    if (macsec_secy_list_init(&secy_device->vPortFreeList_p, OBJECT_VPORT_T, (void**)&secy_device->vPortDscr_p, secy_device->vPortCount))
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate vPort free list\n");
        kfree(secy_device->vPortFreeList_p);
        kfree(secy_device->vPortDscr_p);
        secy_device->vPortFreeList_p = NULL;
        secy_device->vPortDscr_p = NULL;
        return -1;
    }

    secy_device->vPortListHeads_p = kmalloc(secy_device->vPortCount * macsec_List_GetInstanceByteCount(), GFP_KERNEL);
    if (secy_device->vPortListHeads_p == NULL)
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate vPort SC list array\n");
        return -1;
    }

    /* Initialize the Rule free list for this device */
    if (macsec_secy_list_init(&secy_device->RuleFreeList_p, OBJECT_RULE_T, (void**)&secy_device->RuleDscr_p, secy_device->RuleCount))
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate Rule free list\n");
        kfree(secy_device->RuleFreeList_p);
        kfree(secy_device->RuleDscr_p);
        secy_device->RuleFreeList_p = NULL;
        secy_device->RuleDscr_p = NULL;
        return -1;
    }

    secy_device->fInitialized = 1;

    phy_macsec_log(LOG_INFO, "DONE: macsec_secy_init for device %d\n", role);
    return 0;
}

static int macsec_device_init(secy_device_access_t *secy_device_access, const macsec_settings_t * const Settings_p, macsec_Capabilities_t *Capabilities)
{
    uint32_t i;
    uint32_t EmptyWord[2];
    int temp_log;

    phy_macsec_log(LOG_INFO, "macsec device %d init\n", Settings_p->Mode);

    //macsec_SECY_HWRevision_Get(secy_device_access, &pCapabilities->EIP160_Options, &pCapabilities->EIP160_Version);
    //macsec_TCAM_HWRevision_Get(secy_device_access, &pCapabilities->TCAM_Options, &pCapabilities->TCAM_Version);
    macsec_EIP160_HWRevision_Get(secy_device_access, Capabilities);

    /* Check compatibility between requested mode and HW configuration */
    if (Settings_p->Mode == SECY_ROLE_EGRESS && Capabilities->EIP160_Options.fIngressOnly)
    {
        phy_macsec_log(LOG_ERROR, "macsec: HW configuration mismatch\n");
        return -1;
    }

    /* Check compatibility between requested mode and HW configuration */
    if (Settings_p->Mode == SECY_ROLE_INGRESS && Capabilities->EIP160_Options.fEgressOnly)
    {
        phy_macsec_log(LOG_ERROR, "macsec: HW configuration mismatch\n");
        return -1;
    }

    secy_device_access->fMatchSCI = Capabilities->EIP160_Options.fMatchSCI;

    /* Store the maximum number of SA's, Secure Channels, Rules and vPorts */
    /* that the device supports */
    secy_device_access->MaxSACounters = Capabilities->EIP160_Options.SA_Counters;
    secy_device_access->MaxGlobalCounters = Capabilities->EIP160_Options.Global_Counters;
    secy_device_access->MaxVLANCounters = Capabilities->EIP160_Options.VLAN_Counters;
    secy_device_access->MaxSACount = Capabilities->EIP160_Options.SA_Count;
    secy_device_access->MaxSCCount = Capabilities->EIP160_Options.SC_Count;
    secy_device_access->MaxvPortCount = Capabilities->EIP160_Options.vPort_Count;
    secy_device_access->MaxRuleCount = Capabilities->EIP160_Options.Rule_Count;

    temp_log = log_level;
    log_level = LOG_INFO;
    /* Clear SA Transform Record RAM */
    for (i = 0; i < secy_device_access->MaxSACount; i++)
    {
        EmptyWord[0] = 0;
        /* Read old CtxID value. */
        macsec_EIP160_XFORM_REC_RD(secy_device_access, i, 1, 1, &EmptyWord[1]);
        /* Maintain 24 most significant bits of CtxID. */
        EmptyWord[1] = (EmptyWord[1] & 0xffffff00) | i;
        macsec_EIP160_XFORM_REC_WR(secy_device_access, i, EmptyWord, 2);
    }

    log_level = temp_log;

    /* Program the sequence number threshold */
    if (Settings_p->SeqNrThreshold)
    {
        macsec_EIP62_SEQ_NR_THRESH_WR(secy_device_access, Settings_p->SeqNrThreshold);
    }
#if 0
    if (Settings_p->SeqNrThreshold64Lo || Settings_p->SeqNrThreshold64Hi)
    {
        macsec_EIP62_SEQ_NR_THRESH_64_WR(pa,Device,
                                  Settings_p->SeqNrThreshold64Lo,
                                  Settings_p->SeqNrThreshold64Hi);
    }

    /* Threshold for the frame counters */
    macsec_EIP160_SA_COUNT_FRAME_THR_WR(pa,Device,
                                 Settings_p->SACountFrameThrLo,
                                 Settings_p->SACountFrameThrHi);
    macsec_EIP160_SECY_COUNT_FRAME_THR_WR(pa,Device,
                                   Settings_p->SecYCountFrameThrLo,
                                   Settings_p->SecYCountFrameThrHi);
    macsec_EIP160_IFC_COUNT_FRAME_THR_WR(pa,Device,
                                  Settings_p->IFCCountFrameThrLo,
                                  Settings_p->IFCCountFrameThrHi);
    macsec_EIP160_RXCAM_COUNT_FRAME_THR_WR(pa,Device,
                                    Settings_p->RxCAMCountFrameThrLo,
                                    Settings_p->RxCAMCountFrameThrHi);
    macsec_EIP160_TCAM_COUNT_FRAME_THR_WR(pa,Device,
                                   Settings_p->TCAMCountFrameThrLo,
                                   Settings_p->TCAMCountFrameThrHi);

    /* Threshold for the octet counters */
    macsec_EIP160_SA_COUNT_OCTET_THR_WR(pa,Device,
                                 Settings_p->SACountOctetThrLo,
                                 Settings_p->SACountOctetThrHi);
    macsec_EIP160_IFC_COUNT_OCTET_THR_WR(pa,Device,
                                  Settings_p->IFCCountOctetThrLo,
                                  Settings_p->IFCCountOctetThrHi);

    /* Reset all statistics counters and threshold summary registers */
    macsec_EIP160_TCAM_COUNT_CONTROL_WR(pa,Device, true, false, false, false);
    macsec_EIP160_SA_COUNT_CONTROL_WR(pa,Device, true, false, false, false);
    macsec_EIP160_SECY_COUNT_CONTROL_WR(pa,Device, true, false, false, false);
    macsec_EIP160_IFC_COUNT_CONTROL_WR(pa,Device, true, false, false, false);
    macsec_EIP160_RXCAM_COUNT_CONTROL_WR(pa,Device, true, false, false, false);

    /* Configure the statistics module to clear-on-read and saturating */
    macsec_EIP160_TCAM_COUNT_CONTROL_WR(pa,Device,
                                 false,   /* Do not reset */
                                 true,    /* Saturate counters */
                                 Settings_p->fAutoStatCntrsReset,
                                 false);  /* Do not reset */
    macsec_EIP160_SA_COUNT_CONTROL_WR(pa,Device,
                               false,       /* Do not reset */
                               true,        /* Saturate counters */
                               Settings_p->fAutoStatCntrsReset,
                               false);      /* Do not reset */
    macsec_EIP160_SECY_COUNT_CONTROL_WR(pa,Device,
                                 false,     /* Do not reset */
                                 true,      /* Saturate counters */
                                 Settings_p->fAutoStatCntrsReset,
                                 false);    /* Do not reset */
    macsec_EIP160_IFC_COUNT_CONTROL_WR(pa,Device,
                                false,      /* Do not reset */
                                true,       /* Saturate counters */
                                Settings_p->fAutoStatCntrsReset,
                                false);     /* Do not reset */
    macsec_EIP160_RXCAM_COUNT_CONTROL_WR(pa,Device,
                                  false,    /* Do not reset */
                                  true,     /* Saturate counters */
                                  Settings_p->fAutoStatCntrsReset,
                                  false);   /* Do not reset */

    {
        uint16_t SACntrMask, NumSACntrs;
        uint16_t IFCCntrMask, NumIFCCntrs;
        uint16_t SecYCntrMask, NumSecYCntrs;
        uint16_t RxCAMCntrMask, NumRxCAMCntrs;
        uint16_t TCAMCntrMask, NumTCAMCntrs;
        uint32_t GlblCntrMask, NumGlblCntrs;

        /* Retrieve number of counters for every statistics module */
        NumSACntrs = Capabilities->EIP160_Options.SA_Counters;
        NumTCAMCntrs = Capabilities->TCAM_Options.TCAMHitPktCounters_Count;
        NumGlblCntrs = Capabilities->TCAM_Options.TCAMHitPktCountersGlobal_Count;
        NumRxCAMCntrs = sizeof(EIP160_SecY_RxCAM_Stat_t) /
                                sizeof(EIP160_UI64_t);
        if (TrueIOArea_p->Mode == SECY_ROLE_INGRESS)
        {
            NumIFCCntrs = sizeof(EIP160_SecY_Ifc_Stat_I_t) /
                                sizeof(EIP160_UI64_t);
            NumSecYCntrs = sizeof(EIP160_SecY_SecY_Stat_I_t) /
                                sizeof(EIP160_UI64_t);
        } else {
            NumIFCCntrs = sizeof(EIP160_SecY_Ifc_Stat_E_t) /
                                sizeof(EIP160_UI64_t);
            NumSecYCntrs = sizeof(EIP160_SecY_SecY_Stat_E_t) /
                                sizeof(EIP160_UI64_t);
        }

        /* Calculate masks for every statistics module to cover all counters */
        SACntrMask = ((1 << NumSACntrs) - 1);
        TCAMCntrMask = ((1 << NumTCAMCntrs) - 1);
        GlblCntrMask = ((1 << NumGlblCntrs) - 1);
        IFCCntrMask = ((1 << NumIFCCntrs) - 1);
        SecYCntrMask = ((1 << NumSecYCntrs) - 1);
        RxCAMCntrMask = ((1 << NumRxCAMCntrs) - 1);

        /* Enable or disable counters, depending on the value of */
        /* Settings_p->CountIncDisCtrl */
        macsec_EIP160_SA_COUNT_INCEN1_WR(pa,Device, SACntrMask *
            (!(Settings_p->CountIncDisCtrl &
                    EIP160_SECY_SA_COUNT_INC_DIS)));
        macsec_EIP160_IFC_COUNT_INCEN1_WR(pa,Device, IFCCntrMask *
            (!(Settings_p->CountIncDisCtrl &
                    EIP160_SECY_IFC_COUNT_INC_DIS)));
        macsec_EIP160_SECY_COUNT_INCEN1_WR(pa,Device, SecYCntrMask *
            (!(Settings_p->CountIncDisCtrl &
                    EIP160_SECY_SECY_COUNT_INC_DIS)));
        macsec_EIP160_RXCAM_COUNT_INCEN1_WR(pa,Device, RxCAMCntrMask *
            (!(Settings_p->CountIncDisCtrl &
                    EIP160_SECY_RXCAM_COUNT_INC_DIS)));
        macsec_EIP160_TCAM_COUNT_INCEN1_WR(pa,Device, TCAMCntrMask *
            (!(Settings_p->CountIncDisCtrl &
                    EIP160_SECY_TCAM_COUNT_INC_DIS)));
        macsec_EIP160_TCAM_COUNT_INCEN2_WR(pa,Device, GlblCntrMask *
            (!(Settings_p->CountIncDisCtrl &
                    EIP160_SECY_GLOBAL_COUNT_INC_DIS)));
    }

#endif

    /* Configure the EIP-62 context control */
    /* Note: For the EIP-160, correct usage according to the manual would */
    /*       be strictly equal comparison (true) focusing on egress, */
    /*       however greater or equal comparison (false) works better for */
    /*       both egress and ingress. */
    macsec_EIP62_CTX_CTRL_WR(secy_device_access,
                             EIP160_XFORM_REC_SIZE_DEFAULT, /* Context word count */
                             0,         /* Packet Number threshold comparison */
                             0xE588);      /* EtherType for MACSec*/

    /* Configured context update on externally bad packets */
    if (Settings_p->Mode == SECY_ROLE_INGRESS)
    {
        /* To be MACsec compliant disable context update */
        macsec_EIP62_CTX_UPD_CTRL_WR(secy_device_access, 3);
    }
    else
    {
        /* For security reasons enable context update */
        macsec_EIP62_CTX_UPD_CTRL_WR(secy_device_access, 0);
    }

#if 0
    {
        uint32_t Flags;

        Flags = (Settings_p->ErrorDropFlow.ErrCaptReason << 20) |
                (Settings_p->ErrorDropFlow.ErrDestPort   << 18) |
                (Settings_p->ErrorDropFlow.ErrDropAction << 16) |
                (Settings_p->ErrorDropFlow.fDropOnCrcErr  << 15) |
                (Settings_p->ErrorDropFlow.fDropOnPktErr  << 14);

        /* Set device latency and other params */
        macsec_EIP160_MISC_CONTROL_WR(pa,Device,
                               Settings_p->Latency,
                               Settings_p->fStaticBypass,
                               Settings_p->fBypassNoclass,
                               EIP160_REG_MISC_CONTROL_DEFAULT | Flags);

        macsec_EIP160_LATENCY_CONTROL_WR(pa, Device, Settings_p->Latency);
    }

    if (Capabilities->EIP160_Version.MajHWRevision == 5 &&
        Capabilities->EIP160_Version.MajHWRevision >= 1)
    {
        /* Write dynamic latency register. */
        macsec_EIP160_DYN_LATENCY_CONTROL_WR(pa,Device,
                                      Settings_p->DynamicLatencyWords,
                                      Settings_p->fDynamicLatency);
    }
    /* Set the MACsec ID control settings */
    macsec_EIP160_MACSEC_ID_CONTROL_WR(pa,Device, Settings_p->MACsecIDCtrl.MACsecID,
                                        Settings_p->MACsecIDCtrl.fMACsecIDEnb);

    /* Set the Global timer time stamping settings */
    macsec_EIP160_GL_TIME_TICK_WR(pa,Device, Settings_p->GLTimeTick);
    macsec_EIP160_GL_TIME_PRESCALE_WR(pa,Device, Settings_p->GLTimePrescale);
#endif    

    /* Set the Store and Forward buffer settings */
    macsec_EIP160_SAF_CTRL_WR(secy_device_access,
                       Settings_p->SAFControl.fEnable,
                       Settings_p->SAFControl.fDropClass,
                       Settings_p->SAFControl.fDropPP,
                       Settings_p->SAFControl.fDropSecFail,
                       Settings_p->SAFControl.fDropMTU,
                       Settings_p->SAFControl.fDropMACErr,
                       Settings_p->SAFControl.fDropMACCRC);

    macsec_EIP160_SAF_THRESHOLD_WR(secy_device_access,
                            Settings_p->SAFControl.LowWatermark,
                            Settings_p->SAFControl.HighWatermark);
#if 0
    /* Set the Context Update control settings */
    macsec_EIP62_UPDATE_CTRL_WR(pa,Device,
                         Settings_p->UpdateCtrl.fThresholdMode,
                         Settings_p->UpdateCtrl.UpdateThresholdValue);
#endif

    /* Set initial processing rules for packets not matching SA flows */
    {
        uint32_t BypassDropBits;
        uint32_t DestPortBits;
        uint32_t DropNonReservedBits;
        uint32_t DropTypeBits;

        BypassDropBits =
            ((Settings_p->NonControlDropBypass.KaY.fBypass == 0)    << 24) |
            ((Settings_p->NonControlDropBypass.BadTag.fBypass == 0) << 16) |
            ((Settings_p->NonControlDropBypass.Tagged.fBypass == 0) << 8)  |
            ((Settings_p->NonControlDropBypass.Untagged.fBypass == 0));
        DestPortBits =
            (Settings_p->NonControlDropBypass.KaY.DestPort    << 24) |
            (Settings_p->NonControlDropBypass.BadTag.DestPort << 16) |
            (Settings_p->NonControlDropBypass.Tagged.DestPort << 8)  |
            (Settings_p->NonControlDropBypass.Untagged.DestPort);
        DropNonReservedBits =
            (Settings_p->NonControlDropBypass.KaY.fDropNonReserved    << 24) |
            (Settings_p->NonControlDropBypass.BadTag.fDropNonReserved << 16) |
            (Settings_p->NonControlDropBypass.Tagged.fDropNonReserved << 8)  |
            (Settings_p->NonControlDropBypass.Untagged.fDropNonReserved);
        DropTypeBits =
            (Settings_p->NonControlDropBypass.KaY.DropType    << 24) |
            (Settings_p->NonControlDropBypass.BadTag.DropType << 16) |
            (Settings_p->NonControlDropBypass.Tagged.DropType << 8)  |
            (Settings_p->NonControlDropBypass.Untagged.DropType);

        macsec_EIP160_SAM_NM_FLOW_NCP_WR(secy_device_access,
                                  BypassDropBits,
                                  DestPortBits,
                                  DropNonReservedBits,
                                  DropTypeBits);

        BypassDropBits =
            ((Settings_p->ControlDropBypass.KaY.fBypass == 0)    << 24) |
            ((Settings_p->ControlDropBypass.BadTag.fBypass == 0) << 16) |
            ((Settings_p->ControlDropBypass.Tagged.fBypass == 0) << 8)  |
            ((Settings_p->ControlDropBypass.Untagged.fBypass == 0));
        DestPortBits =
            (Settings_p->ControlDropBypass.KaY.DestPort    << 24) |
            (Settings_p->ControlDropBypass.BadTag.DestPort << 16) |
            (Settings_p->ControlDropBypass.Tagged.DestPort << 8)  |
            (Settings_p->ControlDropBypass.Untagged.DestPort);
        DropNonReservedBits =
            (Settings_p->ControlDropBypass.KaY.fDropNonReserved    << 24) |
            (Settings_p->ControlDropBypass.BadTag.fDropNonReserved << 16) |
            (Settings_p->ControlDropBypass.Tagged.fDropNonReserved << 8)  |
            (Settings_p->ControlDropBypass.Untagged.fDropNonReserved);
        DropTypeBits =
            (Settings_p->ControlDropBypass.KaY.DropType    << 24) |
            (Settings_p->ControlDropBypass.BadTag.DropType << 16) |
            (Settings_p->ControlDropBypass.Tagged.DropType << 8)  |
            (Settings_p->ControlDropBypass.Untagged.DropType);

        macsec_EIP160_SAM_NM_FLOW_CP_WR(secy_device_access,
                                 BypassDropBits,
                                 DestPortBits,
                                 DropNonReservedBits,
                                 DropTypeBits);

#if 0
        /* Writing the Non-matching flow capture reason registers */
        macsec_EIP160_SAM_NM_FLOW_CAPT_REASON_WR(pa,Device,
                Settings_p->NonControlDropBypass.KaY.CaptureReason |
                (Settings_p->ControlDropBypass.KaY.CaptureReason << 16),
                Settings_p->NonControlDropBypass.BadTag.CaptureReason |
                (Settings_p->ControlDropBypass.BadTag.CaptureReason << 16),
                Settings_p->NonControlDropBypass.Tagged.CaptureReason |
                (Settings_p->ControlDropBypass.Tagged.CaptureReason << 16),
                Settings_p->NonControlDropBypass.Untagged.CaptureReason |
                (Settings_p->ControlDropBypass.Untagged.CaptureReason << 16));
#endif                

        macsec_EIP160_CP_MATCH_ENABLE_WR(secy_device_access, Settings_p->CPMatchEnableMask);
    }

#if 0
    /* Configure SA counters increments as security fail events */
    macsec_EIP160_COUNT_SECFAIL1_WR(pa,Device,
                             Settings_p->SA_SecFail_Mask,
                             EIP62_FATAL_ERROR_MASK);

    /* Configure SecY counters increments as security fail events */
    macsec_EIP160_COUNT_SECFAIL2_WR(pa,Device, Settings_p->SecY_SecFail_Mask);

    /* Configure Global counters increments as security fail events */
    macsec_EIP160_COUNT_SECFAIL3_WR(pa,Device, Settings_p->Global_SecFail_Mask);
#endif 

    return macsec_EIP160Lib_Device_Init_Done(secy_device_access);
}

static void macsec_SECY_HWRevision_Get(secy_device_access_t *secy_device_access, macsec_EIP160_Options_t * const Options_p, macsec_Version_t * const Version_p)
{
    macsec_EIP160_EIP_REV_RD(secy_device_access,
                      &Version_p->EipNumber,
                      &Version_p->ComplmtEipNumber,
                      &Version_p->HWPatchLevel,
                      &Version_p->MinHWRevision,
                      &Version_p->MajHWRevision);

    macsec_EIP160_CONFIG_RD(secy_device_access,
                     &Options_p->SA_Count,
                     &Options_p->SC_Count,
                     &Options_p->vPort_Count,
                     &Options_p->Rule_Count,
                     &Options_p->fMatchSCI);

    macsec_EIP160_CONFIG2_RD(secy_device_access,
                     &Options_p->SA_Counters,
                     &Options_p->VLAN_Counters,
                     &Options_p->Global_Counters,
                     &Options_p->fIngressOnly,
                     &Options_p->fEgressOnly);
}

static void macsec_TCAM_HWRevision_Get(secy_device_access_t *secy_device_access, macsec_TCAM_Options_t * const Options_p, macsec_Version_t * const Version_p)
{
    /* Channel counters are not available in EIP160, Use dummy parameter. */
    uint8_t Dummy;

    macsec_EIP217_EIP_REV_RD(secy_device_access,
                      &Version_p->EipNumber,
                      &Version_p->ComplmtEipNumber,
                      &Version_p->HWPatchLevel,
                      &Version_p->MinHWRevision,
                      &Version_p->MajHWRevision);

    macsec_EIP217_OPTIONS_RD(secy_device_access,
                      &Options_p->TCAMHitPktCounters_Count,
                      &Options_p->TCAMHitByteCounters_Count,
                      &Options_p->TCAMHitPktCountersGlobal_Count,
                      &Options_p->TCAMHitByteCountersGlobal_Count,
                      &Dummy,
                      &Dummy,
                      &Dummy);

    macsec_EIP217_OPTIONS2_RD(secy_device_access,
                       &Options_p->TCAMHitCounters_Count,
                       &Options_p->TCAMHitCountersWidth_BitCount);
}


static void macsec_EIP62_HWRevision_Get(secy_device_access_t *secy_device_access, macsec_EIP62_Options_t * const Options_p, macsec_Version_t * const Version_p)
{
    macsec_EIP62_EIP_REV_RD(secy_device_access,
                     &Version_p->EipNumber,
                     &Version_p->ComplmtEipNumber,
                     &Version_p->HWPatchLevel,
                     &Version_p->MinHWRevision,
                     &Version_p->MajHWRevision);

    macsec_EIP62_TYPE_RD(secy_device_access,
                  &Options_p->fFPGASolution,
                  &Options_p->fGFSboxes,
                  &Options_p->fLookupSboxes,
                  &Options_p->fMACsecAESOnly,
                  &Options_p->fAESPresent,
                  &Options_p->fAESFb,
                  &Options_p->AESSpeed,
                  &Options_p->KeyLengths,
                  &Options_p->EopParamBits,
                  &Options_p->fIPSec,
                  &Options_p->fHdrExtension,
                  &Options_p->fSecTAGOffset,
                  &Options_p->fGHASHPresent,
                  &Options_p->fOneCycleCore);
}

static int macsec_EIP160Lib_Device_Init_Done(secy_device_access_t *secy_device_access)
{
    int i = 0;
    uint8_t fResetAllTCAM, fResetAllSA, fResetAllSecY, fResetAllIFC, fResetAllRXCAM;
    uint8_t fSaturateCtrs, fAutoCtrReset, fResetSummary;

    while (i++ < MACSEC_INIT_RETRY_COUNT)
    {

        macsec_EIP160_COUNT_CONTROL_RD(secy_device_access,
                                     EIP160_REG_TCAM_STAT_CTRL_OFFS,
                                     &fResetAllTCAM,
                                     &fSaturateCtrs,
                                     &fAutoCtrReset,
                                     &fResetSummary);

        macsec_EIP160_COUNT_CONTROL_RD(secy_device_access,
                                     EIP160_REG_SA_STATISTICS_CONTROLS,
                                     &fResetAllSA,
                                     &fSaturateCtrs,
                                     &fAutoCtrReset,
                                     &fResetSummary);

        macsec_EIP160_COUNT_CONTROL_RD(secy_device_access,
                                     EIP160_REG_SECY_STATISTICS_CONTROLS,
                                     &fResetAllSecY,
                                     &fSaturateCtrs,
                                     &fAutoCtrReset,
                                     &fResetSummary);

        macsec_EIP160_COUNT_CONTROL_RD(secy_device_access,
                                     EIP160_REG_IFC_STATISTICS_CONTROLS,
                                     &fResetAllIFC,
                                     &fSaturateCtrs,
                                     &fAutoCtrReset,
                                     &fResetSummary);

        macsec_EIP160_COUNT_CONTROL_RD(secy_device_access,
                                     EIP160_REG_RXCAM_STATISTICS_CONTROLS,
                                     &fResetAllRXCAM,
                                     &fSaturateCtrs,
                                     &fAutoCtrReset,
                                     &fResetSummary);

        IDENTIFIER_NOT_USED(fSaturateCtrs);
        IDENTIFIER_NOT_USED(fAutoCtrReset);
        IDENTIFIER_NOT_USED(fResetSummary);

        /* Wait till this falls to false indicating that */
        /* the device initialization is completed */
        if (!(fResetAllSA || fResetAllSecY || fResetAllIFC || fResetAllRXCAM || fResetAllTCAM))
        {
            phy_macsec_log(LOG_DEBUG, "macsec_device_init (%d)\n", i-1);
            return 0;
        }

        msleep(10);
    }

    phy_macsec_log(LOG_ERROR, "macsec_device_init failed\n");

    return -1;
}

static int macsec_EIP160_HWRevision_Get(secy_device_access_t *secy_device_access, macsec_Capabilities_t * const Capabilities_p)
{
    macsec_SECY_HWRevision_Get(secy_device_access, &Capabilities_p->EIP160_Options, &Capabilities_p->EIP160_Version);
    macsec_TCAM_HWRevision_Get(secy_device_access, &Capabilities_p->TCAM_Options, &Capabilities_p->TCAM_Version);
    macsec_EIP62_HWRevision_Get(secy_device_access, &Capabilities_p->EIP62_Options, &Capabilities_p->EIP62_Version);

    return 0;
}

static int macsec_secy_list_init(void ** const ObjectFreeList_pp, uint32_t ObjectType, void ** const ObjectDscr_pp, const uint32_t ObjectCount)
{
    uint32_t i;
    List_Status_t List_Rc;
    void * FL_p;
    void * ObjectDscr_p;
    size_t ObjectDscrByteCount;
    const char *ObjectName_p;
    List_Element_t * Elmt_p;
    char *p;

    switch(ObjectType)
    {
        case OBJECT_SA_T:
            ObjectName_p = "SA";
            ObjectDscrByteCount = sizeof(secy_sa_descriptor_t);
            break;
        case OBJECT_SC_T:
            ObjectName_p = "SC";
            ObjectDscrByteCount = sizeof(secy_sc_descriptor_t);
            break;
        case OBJECT_VPORT_T:
            ObjectName_p = "vPort";
            ObjectDscrByteCount = sizeof(secy_vport_descriptor_t);
            break;
        case OBJECT_RULE_T:
            ObjectName_p = "Rule";
            ObjectDscrByteCount = sizeof(secy_rule_descriptor_t);
            break;
        default:
            return -1;
    }

    FL_p = kmalloc(macsec_List_GetInstanceByteCount(), GFP_KERNEL);
    if (!FL_p)
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate %s free list\n", ObjectName_p);
        return -1;
    }

    List_Rc = macsec_List_Init(0, FL_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        phy_macsec_log(LOG_ERROR, " Failed to initialize %s free list\n", ObjectName_p);
        kfree(FL_p);
        return -1; /* error */
    }

    /* Allocate data object descriptors */
    ObjectDscr_p = kmalloc(ObjectCount * ObjectDscrByteCount, GFP_KERNEL);
    if (ObjectDscr_p == NULL)
    {
        phy_macsec_log(LOG_ERROR, " Failed to allocate %s descriptors\n", ObjectName_p);
        kfree(FL_p);
        return -1; /* error */
    }

    p = (char*)ObjectDscr_p;
   
    /* Initialize data objects */
    for (i = 0; i < ObjectCount; i++)
    {
        switch (ObjectType)
        {
            case OBJECT_SA_T:
                Elmt_p = &((secy_sa_descriptor_t*)(void*)p)->free;
                break;
            case OBJECT_SC_T:
                Elmt_p = &((secy_sc_descriptor_t*)(void*)p)->OnList;
                break;
            case OBJECT_VPORT_T:
                Elmt_p = &((secy_vport_descriptor_t*)(void*)p)->free;
                break;
            case OBJECT_RULE_T:
                Elmt_p = &((secy_rule_descriptor_t*)(void*)p)->free;
                break;
            default:
                return -1;
        }

        /* Link the object descriptor to the element */
        Elmt_p->DataObject_p = p;

        /* Add the element to the Rule free list */
        List_Rc = macsec_List_AddToHead(0, FL_p, Elmt_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            phy_macsec_log(LOG_ERROR, " Failed to add element to %s free list\n", ObjectName_p);
            kfree(ObjectDscr_p);
            kfree(FL_p);
            return -1; /* error */
        }

        p += ObjectDscrByteCount;
    } /* for */

    *ObjectFreeList_pp  = FL_p;
    *ObjectDscr_pp      = ObjectDscr_p;

    return 0;
}

static int macsec_SABuilder_InitParams(SABuilder_Params_t *const SAParams_p, uint8_t AN, SABuilder_Direction_t direction)
{
    if (SAParams_p == NULL)
        return -1;

    memset(SAParams_p, 0, sizeof(SABuilder_Params_t));

    SAParams_p->direction = direction;
    SAParams_p->operation = SAB_OP_MACSEC;
    SAParams_p->AN = AN;

    if (direction == SAB_DIRECTION_INGRESS)
    {
        SAParams_p->SeqNumLo = 1;
    }

    return 0;
}

static void macsec_SABuilderLib_SetOffsets(const SABuilder_Params_t * const SAParams_p, SABuilder_Offsets_t * const SAOffsets_p)
{
    uint32_t long_key;
    memset(SAOffsets_p, 0, sizeof(SABuilder_Offsets_t));
    /* For the EIP-165, offsets are defined by Tables 4 and5 in */
    /* Programmer Manual. */
    SAOffsets_p->MTUOffs = 0;
    SAOffsets_p->KeyOffs = 2;
    if (SAParams_p->KeyByteCount == 16)
    {
        long_key = 0;
    }
    else
    {
        long_key = 4;
    }
    SAOffsets_p->HKeyOffs = long_key + 6;
    SAOffsets_p->SeqNumOffs = long_key + 10;
    if (SAParams_p->direction == SAB_DIRECTION_EGRESS)
    {
        if ((SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
        {
            SAOffsets_p->CtxSaltOffs = long_key + 13;
            SAOffsets_p->IVOffs = long_key + 16;
            if (long_key)
                SAOffsets_p->UpdateCtrlOffs = 16;
            else
                SAOffsets_p->UpdateCtrlOffs = 19;
        }
        else
        {
            SAOffsets_p->IVOffs = long_key + 11;
            SAOffsets_p->UpdateCtrlOffs = long_key + 15;
        }
    }
    else
    {
        if ((SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
        {
            SAOffsets_p->MaskOffs = long_key + 12;
            SAOffsets_p->CtxSaltOffs = long_key + 13;
            if (long_key)
                SAOffsets_p->UpdateCtrlOffs = 20;
            else
                SAOffsets_p->UpdateCtrlOffs = 19;
        }
        else
        {
            SAOffsets_p->MaskOffs = long_key + 11;
            SAOffsets_p->IVOffs = long_key + 12;
            SAOffsets_p->UpdateCtrlOffs = long_key + 15;
        }
    }
    SAOffsets_p->MaxOffs = SAB_MACSEC_SA_WORD_COUNT - 1;
}

static int macsec_SABuilder_GetSize(SABuilder_Params_t *const SAParams_p, uint32_t *const SAWord32Count_p)
{
    SABuilder_Offsets_t SAOffsets;

    if (SAWord32Count_p == NULL)
        return -1;

    /* Compute offsets for various fields. */
    macsec_SABuilderLib_SetOffsets(SAParams_p, &SAOffsets);

    /* Use a constant size SA record. */
    *SAWord32Count_p = SAOffsets.MaxOffs+1;

    return 0;
}

static void macsec_SABuilderLib_CopyKeyMat(uint32_t * const Destination_p, const uint32_t offset, const uint8_t * const Source_p, const uint32_t KeyByteCount)
{
    uint32_t *dst = Destination_p + offset;
    const uint8_t *src = Source_p;
    uint32_t i,j;
    uint32_t w;
    if (Destination_p == NULL)
        return;
    for(i=0; i<(KeyByteCount+3)/4; i++)
    {
        w=0;
        for(j=0; j<4; j++)
            w=(w>>8)|(*src++ << 24);
        *dst++ = w;
    }
}

static int macsec_SABuilder_BuildSA(SABuilder_Params_t * const SAParams_p, uint32_t *const SABuffer_p, SABuilder_AESCallback_t AES_CB)
{
    SABuilder_Offsets_t SAOffsets;
    if (SAParams_p == NULL || SABuffer_p == NULL)
        return -1;

    if (SAParams_p->direction != SAB_DIRECTION_EGRESS &&
        SAParams_p->direction != SAB_DIRECTION_INGRESS)
    {
        phy_macsec_log(LOG_ERROR, "macsec_SABuilder_BuildSA: Invalid direction\n");
        return -1;
    }

    /* Compute offsets for various fields. */
    macsec_SABuilderLib_SetOffsets(SAParams_p, &SAOffsets);

    phy_macsec_log(LOG_DEBUG, "\n KeyOffs=%u, HKeyOffs=%u, SeqNumOffs=%u, MaskOffs=%u, CtxSaltOffs=%u, IVOffs=%u, UpdateCtrlOffs=%u, MTUOffs=%u, MaxOffs=%u\n",
                SAOffsets.KeyOffs, SAOffsets.HKeyOffs, SAOffsets.SeqNumOffs, SAOffsets.MaskOffs, SAOffsets.CtxSaltOffs, SAOffsets.IVOffs, SAOffsets.UpdateCtrlOffs, SAOffsets.MTUOffs, SAOffsets.MaxOffs);


    /* Fill the entire SA record with zeros. */
    memset(SABuffer_p, 0, (SAOffsets.MaxOffs+1) * sizeof(uint32_t));

    /* Fill in Context Control word. */
    switch (SAParams_p->operation)
    {
    case SAB_OP_MACSEC:
        if (SAParams_p->direction == SAB_DIRECTION_EGRESS)
        {
            if ( (SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_EG64;
            }
            else
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_EG32;
                SAOffsets.CtxSaltOffs = 0;
            }
            if (SAParams_p->AN > 3)
            {
                phy_macsec_log(LOG_ERROR, "macsec_SABuilder_BuildSA: AN value out of range: %d\n",
                         SAParams_p->AN);
            }
            SABuffer_p[0] |= (SAParams_p->AN) << 26;
        }
        else /* SAB_DIRECTION_INGRESS */
        {
            if ( (SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_IG64;
                SAOffsets.IVOffs = 0;
            }
            else
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_IG32;
                SAOffsets.CtxSaltOffs = 0;
            }
            if ((SAParams_p->flags & SAB_MACSEC_FLAG_RETAIN_ICV) != 0)
                SABuffer_p[0] |= SAB_CW0_KEEP_ICV;
            if ((SAParams_p->flags & SAB_MACSEC_FLAG_RETAIN_SECTAG) != 0)
                SABuffer_p[0] |= SAB_CW0_KEEP_SECTAG;

        }
        if ((SAParams_p->flags & SAB_MACSEC_FLAG_ROLLOVER) != 0)
            SABuffer_p[0] |= SAB_CW0_ROLLOVER;
        break;
    case SAB_OP_ENCAUTH_AES_GCM:
        SAOffsets.CtxSaltOffs = 0;
        SAOffsets.IVOffs = 0;
        if (SAParams_p->direction == SAB_DIRECTION_EGRESS)
        {
            SABuffer_p[0] = SAB_CW0_ENCAUTH_AES_GCM;
        }
        else
        {
            SABuffer_p[0] = SAB_CW0_AUTHDEC_AES_GCM;
        }

        if (SAParams_p->ICVByteCount == 0)
        {
            /* Treat as 16. */
            SABuffer_p[0] |= BIT_22;
        }
        else if (SAParams_p->ICVByteCount >= 8)
        {
            SABuffer_p[0] |= ((SAParams_p->ICVByteCount % 4) << 26) |
                (((SAParams_p->ICVByteCount - 8) / 4) << 21);
        }
        else
        {
            phy_macsec_log(LOG_ERROR, "macsec_SABuilder_BuildSA: Invalid ICVByteCount=%d\n",
                SAParams_p->ICVByteCount);
            return -1;
        }
        break;
    case SAB_OP_ENC_AES_CTR:
        SAOffsets.CtxSaltOffs = 0;
        SAOffsets.IVOffs = 0;
        SAOffsets.HKeyOffs = 0;
        SABuffer_p[0] = SAB_CW0_ENC_AES_CTR;
        break;
    default:
        phy_macsec_log(LOG_ERROR, "macsec_SABuilder_BuildSA: Invalid operation\n");
        return -1;
    }

    /* Set cipher algorithm bits in control word. */
    switch (SAParams_p->KeyByteCount)
    {
    case 16:
        SABuffer_p[0] |= SAB_CW0_AES128;
        break;
    case 24:
        SABuffer_p[0] |= SAB_CW0_AES192;
        break;
    case 32:
        SABuffer_p[0] |= SAB_CW0_AES256;
        break;
    default:
        phy_macsec_log(LOG_ERROR, "macsec_SABuilder_BuildSA: Unsupported AES key size %d\n",
                 SAParams_p->KeyByteCount);
        return -1;
    }

    /* Copy cipher key. */
    if (SAParams_p->Key_p == NULL)
        return -1;

    macsec_SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.KeyOffs, SAParams_p->Key_p,
                            SAParams_p->KeyByteCount);
    /* Copy HKey. */
    if (SAOffsets.HKeyOffs > 0)
    {
        if (SAParams_p->HKey_p == NULL)
        {
            uint8_t tmp[16];
            if (AES_CB == NULL)
                return  -1;
            /* Encrypt a single all-zero block */
            AES_CB((uint8_t *)(SABuffer_p+SAOffsets.HKeyOffs),
                   /* HKEY location still contains all zeros */
                   tmp,
                   SAParams_p->Key_p,
                   SAParams_p->KeyByteCount);
            macsec_SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.HKeyOffs,
                                    tmp, 16);
        }
        else
        {

            macsec_SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.HKeyOffs,
                                    SAParams_p->HKey_p, 16);
        }
    }

    /* fill in sequence number/seqmask. */
    if (SAParams_p->operation == SAB_OP_MACSEC)
    {
        SABuffer_p[SAOffsets.SeqNumOffs] = SAParams_p->SeqNumLo;
        if ((SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
            SABuffer_p[SAOffsets.SeqNumOffs + 1] = SAParams_p->SeqNumHi;
        if (SAParams_p->direction == SAB_DIRECTION_INGRESS)
            SABuffer_p[SAOffsets.MaskOffs] = SAParams_p->WindowSize;
    }

    /* Fill in CtxSalt field. */
    if (SAOffsets.CtxSaltOffs > 0)
    {
        const uint8_t * const salt = SAParams_p->Salt_p;
        const uint8_t * const ssci = SAParams_p->SSCI_p;
        uint32_t w;

        if (salt == NULL || ssci == NULL)
            return -1;

        w =  (salt[0] ^ ssci[0])        |
            ((salt[1] ^ ssci[1]) << 8)  |
            ((salt[2] ^ ssci[2]) << 16) |
            ((salt[3] ^ ssci[3]) << 24);
        SABuffer_p[SAOffsets.CtxSaltOffs] = w;
        macsec_SABuilderLib_CopyKeyMat(SABuffer_p,
                                SAOffsets.CtxSaltOffs + 1,
                                salt + 4,
                                8);
    }

    /* Fill in IV fields. */
    if (SAOffsets.IVOffs > 0)
    {
        if (SAParams_p->SCI_p == NULL)
            return -1;

        macsec_SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.IVOffs,
                                SAParams_p->SCI_p, 8);
    }

    return 0;
}

static void macsec_SABuilderLib_ControlToParams(uint32_t ControlWord, SABuilder_Params_t * const SAParams_p)
{
    SABuilder_Direction_t direction;
    if ((ControlWord & MASK_1_BIT) == 1)
        direction = SAB_DIRECTION_INGRESS;
    else
        direction = SAB_DIRECTION_EGRESS;

    macsec_SABuilder_InitParams(SAParams_p, 0, direction);

    if (((ControlWord >> 29) & MASK_1_BIT) != 0)
        SAParams_p->flags |= SAB_MACSEC_FLAG_LONGSEQ;

    switch (ControlWord & (MASK_3_BITS << 17))
    {
        case SAB_CW0_AES128:
            SAParams_p->KeyByteCount = 16;
            break;
        case SAB_CW0_AES192:
            SAParams_p->KeyByteCount = 24;
            break;
        case SAB_CW0_AES256:
            SAParams_p->KeyByteCount = 32;
            break;
    }
    if ((ControlWord & BIT_10) != 0)
    {
        SAParams_p->flags |= SAB_MACSEC_FLAG_ROLLOVER;
    }
}

static int macsec_SABuilder_UpdateCtrlOffset_Get(uint32_t ControlWord, uint32_t * const UpdCtrlOffset_p)
{
    SABuilder_Params_t SAParams;
    SABuilder_Offsets_t SAOffsets;

    if (UpdCtrlOffset_p == NULL)
    {
        return -1;
    }

    macsec_SABuilderLib_ControlToParams(ControlWord, &SAParams);
    if ((SAParams.flags & SAB_MACSEC_FLAG_ROLLOVER) == 0)
    {
        macsec_SABuilderLib_SetOffsets(&SAParams, &SAOffsets);

        *UpdCtrlOffset_p = SAOffsets.UpdateCtrlOffs;
    }
    else
    {
        *UpdCtrlOffset_p = 0;
    }

    return 0;
}

/*----------------------------------------------------------------------------
 * macsec_SABuilder_UpdateCtrl_Update
 */
static int macsec_SABuilder_UpdateCtrl_Update(const SABuilder_UpdCtrl_Params_t * const UpdateParams_p, uint32_t * const SAWord_p)
{
    uint32_t Value;

    if (UpdateParams_p == NULL || SAWord_p == NULL)
    {
        return -1;
    }

    Value = (UpdateParams_p->SAIndex & MASK_13_BITS) |
        ((UpdateParams_p->SCIndex & MASK_13_BITS) << 16) |
        ((UpdateParams_p->AN & MASK_2_BITS) << 29);
    if (UpdateParams_p->fUpdateEnable)
        Value |= BIT_31;
    if (UpdateParams_p->fSAIndexValid)
        Value |= BIT_15;
    if (UpdateParams_p->fExpiredIRQ)
        Value |= BIT_14;
    if (UpdateParams_p->fUpdateTime)
        Value |= BIT_13;

    *SAWord_p = Value;

    return 0;
}

static int macsec_build_transform_record(macsec_sa_builder_params_t *params, uint32_t *sa_buffer_p)
{
    SABuilder_Params_t SAParams = {};
    int rc;
    uint32_t SAWordCount;

    if (params == NULL) {
        phy_macsec_log(LOG_ERROR, "macsec_build_transform_record: bcm_plp_sa_builder_params_t is NULL\n");
        return -1;
    }
    if ( sa_buffer_p == NULL) {
        phy_macsec_log(LOG_ERROR, "macsec_build_transform_record: sa_buffer_p is NULL\n");
        return -1;
    }

    /*rc = macsec_SABuilder_InitParams(&SAParams, params->an, params->direction);
    if (rc != 0)
    {
        phy_macsec_log(LOG_ERROR, "macsec_build_transform_record: macsec_SABuilder_InitParams error\n");
        return -1;
    }*/

    SAParams.direction = params->direction;
    SAParams.operation = params->operation;
    SAParams.AN = params->an;
    SAParams.flags = params->flags;
    SAParams.Key_p = params->key_p;
    SAParams.KeyByteCount = params->key_byte_count;
    SAParams.SCI_p = params->sci_p;
    SAParams.SeqNumLo = params->seq_num_lo;
    SAParams.WindowSize = params->window_size;
    SAParams.HKey_p = params->h_key_p;

    if (params->salt_p)
    {
        SAParams.flags |= SAB_MACSEC_FLAG_LONGSEQ;
        SAParams.Salt_p = params->salt_p;
        SAParams.SSCI_p = params->ssci_p;
        SAParams.SeqNumHi = params->seq_num_hi;
    }

    rc = macsec_SABuilder_GetSize(&SAParams, &SAWordCount);
    if (rc != 0)
    {
        phy_macsec_log(LOG_ERROR, "macsec_build_transform_record: macsec_SABuilder_GetSize error\n");
        return -1;
    }

    phy_macsec_log(LOG_DEBUG, " SAWordCount=%u, flags=%u, direction=%u, operation=%u\n", SAWordCount, SAParams.flags, SAParams.direction, SAParams.operation);

    rc = macsec_SABuilder_BuildSA(&SAParams, sa_buffer_p, NULL);
    if (rc != 0)
    {
        phy_macsec_log(LOG_ERROR, "macsec_build_transform_record: error building SA %d\n",rc);
        return -1;
    }

    return 0;
}

static int macsec_SCI_Find(secy_device_t *secy_device, uint8_t * SCI, uint32_t vPort, uint32_t * SCIndex_p)
{
    List_Status_t List_Rc;
    void *SCList = secy_device->vPortDscr_p[vPort].InUse.SCList;
    const List_Element_t *cur;

    List_Rc = macsec_List_GetHead(0, SCList, &cur);
    if (List_Rc != LIST_STATUS_OK)
    {
        phy_macsec_log(LOG_ERROR, " Failed, no SCI list for device %d\n", secy_device->Role);
        return 0;
    }

    while (cur != NULL)
    {
        secy_sc_descriptor_t *SCDscr_p = cur->DataObject_p;
        if (memcmp(SCDscr_p->SCI, SCI, 8) == 0)
        {
            *SCIndex_p = SCDscr_p->SCIndex;
            return 1;
        }
        cur = macsec_List_GetNextElement(cur);
    }

    return 0;
}

static int macsec_EIP160_SecY_RxCAM_Add(secy_device_access_t *secy_device_access, uint32_t SCIndex, uint32_t SCI_Lo, uint32_t SCI_Hi, uint32_t vPort)
{
    /* Add an entry to the RX CAM (translate vPort+SCI->SCIndex) */
    macsec_EIP160_RXSC_CAM_SCI_LO_WR(secy_device_access, SCIndex, SCI_Lo);
    macsec_EIP160_RXSC_CAM_SCI_HI_WR(secy_device_access, SCIndex, SCI_Hi);
    macsec_EIP160_RXSC_CAM_CTRL_WR(secy_device_access, SCIndex, vPort);
    macsec_EIP160_RXSC_ENTRY_ENABLE_CTRL_WR(secy_device_access, SCIndex, 1, 0, 0, 0, 0);

    return 0;
}

static int macsec_EIP160_SecY_RxCAM_Update(secy_device_access_t *secy_device_access, uint32_t SCIndex, uint32_t SCI_Lo, uint32_t SCI_Hi)
{
    /* Add an entry to the RX CAM (translate vPort+SCI->SCIndex) */
    macsec_EIP160_RXSC_CAM_SCI_LO_WR(secy_device_access, SCIndex, SCI_Lo);
    macsec_EIP160_RXSC_CAM_SCI_HI_WR(secy_device_access, SCIndex, SCI_Hi);

    return 0;
}

static int macsec_EIP160_SecY_RxCAM_Remove(secy_device_access_t *secy_device_access, uint32_t SCIndex)
{
    macsec_EIP160_SC_SA_MAP1_WR(secy_device_access, SCIndex, 0, 0, 0, 0, 0);    
    macsec_EIP160_SC_SA_MAP2_WR(secy_device_access, SCIndex, 0, 0, 0,0);
    macsec_EIP160_RXSC_ENTRY_ENABLE_CTRL_WR(secy_device_access, 0, 0, 0, SCIndex, 1, 0);

    return 0;
}

static int macsec_EIP160_SecY_SC_SA_Map_I_Update(secy_device_access_t *secy_device_access, const uint32_t SCIndex, const uint32_t AN, const uint32_t SAIndex, const uint8_t fSAInUse)
{
    uint32_t SAIndex0, SAIndex1;
    uint8_t fSAInUse0, fSAInUse1;

    switch(AN)
    {
        case 0:
            macsec_EIP160_SC_SA_MAP1_RD(secy_device_access, SCIndex, &SAIndex0, &fSAInUse0, &SAIndex1, &fSAInUse1);
            SAIndex0 = SAIndex;
            fSAInUse0 = fSAInUse;
            macsec_EIP160_SC_SA_MAP1_WR(secy_device_access, SCIndex, SAIndex0, fSAInUse0, 0, SAIndex1, fSAInUse1);
            break;
        case 1:
            macsec_EIP160_SC_SA_MAP1_RD(secy_device_access, SCIndex, &SAIndex0, &fSAInUse0, &SAIndex1, &fSAInUse1);
            SAIndex1 = SAIndex;
            fSAInUse1 = fSAInUse;
            macsec_EIP160_SC_SA_MAP1_WR(secy_device_access, SCIndex, SAIndex0, fSAInUse0, 0, SAIndex1, fSAInUse1);
            break;
        case 2:
            macsec_EIP160_SC_SA_MAP2_RD(secy_device_access, SCIndex, &SAIndex0, &fSAInUse0, &SAIndex1, &fSAInUse1);
            SAIndex0 = SAIndex;
            fSAInUse0 = fSAInUse;
            macsec_EIP160_SC_SA_MAP2_WR(secy_device_access, SCIndex, SAIndex0, fSAInUse0, SAIndex1,fSAInUse1);
            break;
        case 3:
            macsec_EIP160_SC_SA_MAP2_RD(secy_device_access, SCIndex, &SAIndex0, &fSAInUse0, &SAIndex1, &fSAInUse1);
            SAIndex1 = SAIndex;
            fSAInUse1 = fSAInUse;
            macsec_EIP160_SC_SA_MAP2_WR(secy_device_access, SCIndex, SAIndex0, fSAInUse0, SAIndex1,fSAInUse1);
            break;
        default:
            return -1;
    }
    return 0;
}

static int macsec_SA_Update_Control_Word_Update(secy_device_access_t *secy_device_access, const uint32_t SAIndex, const SABuilder_UpdCtrl_Params_t * const UpdateParams)
{
    int Rc;
    uint32_t Offset = 0;
    uint32_t SAUpdateCtrlWord = 0;
    uint32_t Transform_0 = 0;

    macsec_EIP160_XFORM_REC_RD(secy_device_access, SAIndex, 0, 1, &Transform_0);

    Rc = macsec_SABuilder_UpdateCtrlOffset_Get(Transform_0, &Offset);
    if (Rc)
    {
        return -1;
    }

    Rc = macsec_SABuilder_UpdateCtrl_Update(UpdateParams, &SAUpdateCtrlWord);
    if (Rc)
    {
        return -1;
    }

    if (Offset > 0)
    {
        macsec_EIP160_XFORM_REC_WORD_WR(secy_device_access, SAIndex, Offset, SAUpdateCtrlWord);
    }

    return 0;
}                                                                                  

static int macsec_EIP160_SecY_SAMFlow_Write(secy_device_t *secy_device, const uint32_t vPort, const macsec_sa_t * const SA_p)
{
    switch (SA_p->action_type)
    {
        case MACSEC_SA_ACTION_EGRESS:
            /* Check if the SA type is supported in the device mode */
            if (secy_device->Role == SECY_ROLE_INGRESS)
            {
                return -1;
            }

            /* Enable egress SA */
            macsec_EIP160_SAM_FLOW_CTRL1_EGRESS_WR(&secy_device->secy_device_access,
                                    vPort,
                                    SA_p->dest_port,
                                    0,
                                    0, /* fFlowCyptAuth */
                                    SA_p->drop_type,
                                    SA_p->capture_reason,
                                    SA_p->params.egress.fprotect_frames,
                                    SA_p->params.egress.fconf_protect,
                                    SA_p->params.egress.finclude_sci,
                                    SA_p->params.egress.fuse_es,
                                    SA_p->params.egress.fuse_scb,
                                    SA_p->params.egress.fallow_data_pkts,
                                    SA_p->params.egress.fEoMPLS_ctrl_word,
                                    SA_p->params.egress.fEoMPLS_subport,
                                    SA_p->params.egress.fsl_pad_strip_enb,
                                    SA_p->params.egress.fec_from_st_vlan);

            macsec_EIP160_SAM_FLOW_CTRL2_WR(&secy_device->secy_device_access,
                                    vPort,
                                    SA_p->params.egress.confidentiality_offset,
                                    SA_p->params.egress.pre_sectag_auth_start,
                                    SA_p->params.egress.pre_sectag_auth_length,
                                    SA_p->params.egress.sectag_offset);
            break;

        case MACSEC_SA_ACTION_INGRESS:
            /* Check if the SA type is supported in the device mode */
            if (secy_device->Role == SECY_ROLE_EGRESS)
            {
                return -1;
            }

            if (SA_p->params.ingress.fretain_icv && !SA_p->params.ingress.fretain_sectag)
            {
                return -1;
            } 

            /* Enable ingress SA */
            macsec_EIP160_SAM_FLOW_CTRL1_INGRESS_WR(&secy_device->secy_device_access,
                                    vPort,
                                    SA_p->dest_port,
                                    0,
                                    0, /* fFlowCyptAuth */
                                    SA_p->drop_type,
                                    SA_p->capture_reason,
                                    SA_p->params.ingress.freplay_protect,
                                    SA_p->params.ingress.fallow_tagged,
                                    SA_p->params.ingress.fallow_untagged,
                                    SA_p->params.ingress.validate_frames_tagged,
                                    SA_p->params.ingress.fvalidate_untagged,
                                    SA_p->params.ingress.fEoMPLS_ctrl_word,
                                    SA_p->params.ingress.fEoMPLS_subport,
                                    SA_p->params.ingress.fretain_icv,
                                    SA_p->params.ingress.fretain_sectag);

            macsec_EIP160_SAM_FLOW_CTRL2_WR(&secy_device->secy_device_access,
                                    vPort,
                                    SA_p->params.ingress.confidentiality_offset,
                                    SA_p->params.ingress.pre_sectag_auth_start,
                                    SA_p->params.ingress.pre_sectag_auth_length,
                                    SA_p->params.ingress.sectag_offset);
            break;

        case MACSEC_SA_ACTION_CRYPT_AUTH:

            /* Enable Crypt-authenticate SA */
            macsec_EIP160_SAM_FLOW_CTRL1_CRYPTAUTH_WR(&secy_device->secy_device_access,
                                    vPort,
                                    SA_p->dest_port,
                                    0,
                                    1, /* fFlowCryptAuth */
                                    SA_p->drop_type,
                                    SA_p->capture_reason,
                                    SA_p->params.crypt_auth.fconf_protect,
                                    SA_p->params.crypt_auth.ficv_append,
                                    SA_p->params.crypt_auth.iv_mode,
                                    SA_p->params.crypt_auth.ficv_verify);

            {
                uint8_t ConfOffset = SA_p->params.crypt_auth.confidentiality_offset;

                if (SA_p->params.crypt_auth.fzero_length_message)
                {
                    if (SA_p->params.crypt_auth.ficv_verify)
                    {
                        ConfOffset = BIT_6;
                    }
                }

                macsec_EIP160_SAM_FLOW_CTRL2_WR(&secy_device->secy_device_access, vPort, ConfOffset, 0, 0, 0);
            }
            break;

        case MACSEC_SA_ACTION_BYPASS:
            /* Enable non-MACsec SA */
            macsec_EIP160_SAM_FLOW_CTRL1_BYPASS_WR(&secy_device->secy_device_access,
                                            vPort,
                                            SA_p->dest_port,
                                            0,
                                            0, /* fFlowCyptAuth */
                                            SA_p->drop_type,
                                            SA_p->capture_reason);
            break;

        default:
        case MACSEC_SA_ACTION_DROP:
            /* Enable non-MACsec SA */
            macsec_EIP160_SAM_FLOW_CTRL1_DROP_WR(&secy_device->secy_device_access,
                                          vPort,
                                          SA_p->dest_port,
                                          0,
                                          0, /* fFlowCyptAuth */
                                          SA_p->drop_type,
                                          SA_p->capture_reason);
            break;
    }

    return 0;
}

static int macsec_EIP160_SecY_SA_Stat_E_Clear(secy_device_t *secy_device, const uint32_t SAIndex)
{
    if (secy_device->Role != SECY_ROLE_EGRESS)
    {
        return -1;
    }

    /* Egress SA statistics */
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_E_OUT_OCTETS_ENC_PROT);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_E_OUT_PKTS_ENC_PROT);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_E_OUT_PKTS_TOO_LONG);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_E_OUT_PKTS_SA_NOT_IN_USE);

    return 0;
}


static int macsec_EIP160_SecY_SA_Stat_I_Clear(secy_device_t *secy_device, const uint32_t SAIndex)
{
    if (secy_device->Role != SECY_ROLE_INGRESS)
    {
        return -1;
    }

    /* Ingress SA statistics */
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_OCTETS_DEC);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_OCTETS_VALIDATED);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_UNCHECKED);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_DELAYED);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_LATE);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_OK);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_INVALID);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_NOT_VALID);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_NOT_USING_SA);
    macsec_EIP160_COUNTER_64_DEFAULT_WR(&secy_device->secy_device_access,
                                 EIP160_REG_SA_STATISTICS(SAIndex),
                                 EIP160_SA_STAT_I_IN_PKTS_UNUSED_SA);

    return 0;
}

static int phy_macsec_vport_e_statistics_get(secy_device_t *secy_device, uint32_t sc_index, macsec_api_secy_e_stats *stats_p, uint8_t fSync)
{
    uint32_t vPortId = macsec_vPortId_Get(secy_device, sc_index);

    if (vPortId == -1)
        return -1;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    memset(stats_p, 0, sizeof(macsec_api_secy_e_stats));

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS, 
                                &stats_p->OutPktsTransformError.low, &stats_p->OutPktsTransformError.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_E_OUT_PKTS_CTRL, 
                                &stats_p->OutPktsControl.low, &stats_p->OutPktsControl.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_E_OUT_PKTS_UNTAGGED, 
                                &stats_p->OutPktsUntagged.low, &stats_p->OutPktsUntagged.high);


    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_OCTETS_COMMON, 
                                &stats_p->OutOctetsCommon.low, &stats_p->OutOctetsCommon.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED, 
                                &stats_p->OutOctetsUncontrolled.low, &stats_p->OutOctetsUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_OCTETS_CONTROLLED, 
                                &stats_p->OutOctetsControlled.low, &stats_p->OutOctetsControlled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED, 
                                &stats_p->OutPktsUnicastUncontrolled.low, &stats_p->OutPktsUnicastUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED, 
                                &stats_p->OutPktsMulticastUncontrolled.low, &stats_p->OutPktsMulticastUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED, 
                                &stats_p->OutPktsBroadcastUncontrolled.low, &stats_p->OutPktsBroadcastUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED, 
                                &stats_p->OutPktsUnicastControlled.low, &stats_p->OutPktsUnicastControlled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED, 
                                &stats_p->OutPktsMulticastControlled.low, &stats_p->OutPktsMulticastControlled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED, 
                                &stats_p->OutPktsBroadcastControlled.low, &stats_p->OutPktsBroadcastControlled.high);    

    return 0;
}

static int phy_macsec_vport_i_statistics_get(secy_device_t *secy_device, uint32_t sc_index, macsec_api_secy_i_stats *stats_p, uint8_t fSync)
{
    uint32_t vPortId = macsec_vPortId_Get(secy_device, sc_index);

    if (vPortId == -1)
        return -1;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    memset(stats_p, 0, sizeof(macsec_api_secy_i_stats));

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS, 
                                &stats_p->InPktsTransformError.low, &stats_p->InPktsTransformError.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_CTRL, 
                                &stats_p->InPktsControl.low, &stats_p->InPktsControl.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_UNTAGGED, 
                                &stats_p->InPktsUntagged.low, &stats_p->InPktsUntagged.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_NOTAG, 
                                &stats_p->InPktsNoTag.low, &stats_p->InPktsNoTag.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_BADTAG, 
                                &stats_p->InPktsBadTag.low, &stats_p->InPktsBadTag.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_NOSCI, 
                                &stats_p->InPktsNoSCI.low, &stats_p->InPktsNoSCI.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_UNKNOWNSCI, 
                                &stats_p->InPktsUnknownSCI.low, &stats_p->InPktsUnknownSCI.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_XTSECY_STATISTICS(vPortId), EIP160_SECY_STAT_I_IN_PKTS_TAGGEDCTRL, 
                                &stats_p->InPktsTaggedCtrl.low, &stats_p->InPktsTaggedCtrl.high);

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_OCTETS_UNCONTROLLED, 
                                &stats_p->InOctetsUncontrolled.low, &stats_p->InOctetsUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_OCTETS_CONTROLLED, 
                                &stats_p->InOctetsControlled.low, &stats_p->InOctetsControlled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED, 
                                &stats_p->InPktsUnicastUncontrolled.low, &stats_p->InPktsUnicastUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED, 
                                &stats_p->InPktsMulticastUncontrolled.low, &stats_p->InPktsMulticastUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED, 
                                &stats_p->InPktsBroadcastUncontrolled.low, &stats_p->InPktsBroadcastUncontrolled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED, 
                                &stats_p->InPktsUnicastControlled.low, &stats_p->InPktsUnicastControlled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED, 
                                &stats_p->InPktsMulticastControlled.low, &stats_p->InPktsMulticastControlled.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_IFC_STATISTICS(vPortId), EIP160_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED, 
                                &stats_p->InPktsBroadcastControlled.low, &stats_p->InPktsBroadcastControlled.high);

    return 0;
}

static int phy_macsec_tcam_statistics_get(secy_device_t *secy_device, uint32_t rule_index, macsec_api_secy_tcam_stats *stats_p, uint8_t fSync)
{
    uint32_t RuleId = macsec_RuleId_Get(secy_device, rule_index);

    if (RuleId == -1)
        return -1;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    memset(stats_p, 0, sizeof(macsec_api_secy_tcam_stats));

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_TCAM_STATISTICS(RuleId), EIP160_TCAM_STAT_HIT, 
                                &stats_p->tcam_hit.low, &stats_p->tcam_hit.high);

    return 0;
}

static int phy_macsec_rxcam_statistics_get(secy_device_t *secy_device, uint32_t sa_index, macsec_api_secy_rxcam_stats *stats_p, uint8_t fSync)
{
    secy_sa_descriptor_t *SADscr_p = secy_device->sa_handlers[sa_index].p;

    if (!SADscr_p)
        return -1;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    memset(stats_p, 0, sizeof(macsec_api_secy_rxcam_stats));

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_RXCAM_STATISTICS(SADscr_p->InUse.SCIndex), EIP160_RXCAM_STAT_HIT, 
                                &stats_p->cam_hit.low, &stats_p->cam_hit.high);

    return 0;
}


static int phy_macsec_sa_e_statistics_get(secy_device_t *secy_device, uint32_t sa_index, macsec_api_secy_sa_e_stats *stats_p, uint8_t fSync)
{
    uint32_t SAId = macsec_SAId_Get(secy_device, sa_index);

    if (SAId == -1)
        return -1;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    memset(stats_p, 0, sizeof(macsec_api_secy_sa_e_stats));

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_E_OUT_OCTETS_ENC_PROT, 
                                &stats_p->OutOctetsEncryptedProtected.low, &stats_p->OutOctetsEncryptedProtected.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_E_OUT_PKTS_ENC_PROT, 
                                &stats_p->OutPktsEncryptedProtected.low, &stats_p->OutPktsEncryptedProtected.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_E_OUT_PKTS_TOO_LONG, 
                                &stats_p->OutPktsTooLong.low, &stats_p->OutPktsTooLong.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_E_OUT_PKTS_SA_NOT_IN_USE, 
                                &stats_p->OutPktsSANotInUse.low, &stats_p->OutPktsSANotInUse.high);

    return 0;
}

static int phy_macsec_sa_i_statistics_get(secy_device_t *secy_device, uint32_t sa_index, macsec_api_secy_sa_i_stats *stats_p, uint8_t fSync)
{
    uint32_t SAId = macsec_SAId_Get(secy_device, sa_index);

    if (SAId == -1)
        return -1;

    /* Synchronize with the device if required */
    if (fSync)
        macsec_Device_Sync(secy_device);

    memset(stats_p, 0, sizeof(macsec_api_secy_sa_i_stats));

    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_OCTETS_DEC, 
                                &stats_p->InOctetsDecrypted.low, &stats_p->InOctetsDecrypted.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_OCTETS_VALIDATED, 
                                &stats_p->InOctetsValidated.low, &stats_p->InOctetsValidated.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_UNCHECKED, 
                                &stats_p->InPktsUnchecked.low, &stats_p->InPktsUnchecked.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_DELAYED, 
                                &stats_p->InPktsDelayed.low, &stats_p->InPktsDelayed.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_LATE, 
                                &stats_p->InPktsLate.low, &stats_p->InPktsLate.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_OK, 
                                &stats_p->InPktsOK.low, &stats_p->InPktsOK.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_INVALID, 
                                &stats_p->InPktsInvalid.low, &stats_p->InPktsInvalid.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_NOT_VALID, 
                                &stats_p->InPktsNotValid.low, &stats_p->InPktsNotValid.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_NOT_USING_SA, 
                                &stats_p->InPktsNotUsingSA.low, &stats_p->InPktsNotUsingSA.high);
    macsec_EIP160_COUNTER_64_RD(&secy_device->secy_device_access, EIP160_REG_SA_STATISTICS(SAId), EIP160_SA_STAT_I_IN_PKTS_UNUSED_SA, 
                                &stats_p->InPktsUnusedSA.low, &stats_p->InPktsUnusedSA.high);

    return 0;
}

static int macsec_Rule_Update(secy_device_t *secy_device, uint32_t RuleId, macsec_rule_t *Rule_p)
{
    phy_macsec_log(LOG_INFO, "macsec_EIP160_Rule_Update \n");

    /* Write TCAM matching rule */
    /* Write key-half of the rule entry */
    macsec_EIP160_TCAM_CTRL_WR(&secy_device->secy_device_access,
                            EIP160_REG_TCAM_CTRL_KEY(RuleId),
                            Rule_p->key.num_tags,
                            Rule_p->key.packet_type,
                            Rule_p->key.st_vlan,
                            Rule_p->key.frame_type,
                            Rule_p->key.macsec_tagged,
                            Rule_p->key.from_Redirect);

    macsec_EIP160_WriteArray(&secy_device->secy_device_access,
                             EIP160_REG_TCAM_KEY(RuleId, 1),
                             Rule_p->data,
                             EIP160_RULE_NON_CTRL_WORD_COUNT);

    /* Write mask-half of the rule entry */
    macsec_EIP160_TCAM_CTRL_WR(&secy_device->secy_device_access,
                             EIP160_REG_TCAM_CTRL_MASK(RuleId),
                             Rule_p->mask.num_tags,
                             Rule_p->mask.packet_type,
                             Rule_p->mask.st_vlan,
                             Rule_p->mask.frame_type,
                             Rule_p->mask.macsec_tagged,
                             Rule_p->mask.from_Redirect);

    macsec_EIP160_WriteArray(&secy_device->secy_device_access,
                             EIP160_REG_TCAM_MASK(RuleId, 1),
                             Rule_p->data_mask,
                             EIP160_RULE_NON_CTRL_WORD_COUNT);

    /* Write TCAM policy */
    macsec_EIP160_TCAM_POLICY_WR(&secy_device->secy_device_access,
                          RuleId,
                          Rule_p->policy.vport_id,
                          Rule_p->policy.priority,
                          Rule_p->policy.fdrop,
                          Rule_p->policy.fcontrol_packet);

    return 0;
}

static uint32_t macsec_vPortId_Get(secy_device_t* secy_device, uint32_t sc_index)
{
    secy_vport_descriptor_t *vport_desc;

    if (sc_index >= VPORT_NUM_MAX)
        goto Err;

    vport_desc = secy_device->vport_handlers[sc_index].p;

    if (!vport_desc)
        goto Err;

    return vport_desc->InUse.vPortId;

Err: 
    phy_macsec_log(LOG_ERROR, "invalid sc_index %u\n", sc_index);
    return -1;
    
}

static uint32_t macsec_RuleId_Get(secy_device_t* secy_device, uint32_t rule_index)
{
    secy_rule_descriptor_t *rule_desc;

    if (rule_index >= RULE_NUM_MAX)
        goto Err;

    rule_desc = secy_device->rule_handlers[rule_index].p;

    if (!rule_desc)
        goto Err;

    return rule_desc->InUse.RuleId;

Err:    
    phy_macsec_log(LOG_ERROR, "invalid rule_index %u\n", rule_index);
    return -1;
}

static uint32_t macsec_RulesCount_Get(secy_device_t* secy_device, uint32_t sc_index)
{
    secy_vport_descriptor_t *vport_desc = secy_device->vport_handlers[sc_index].p;

    return vport_desc->InUse.BoundRulesCount;
}

static uint32_t macsec_SAId_Get(secy_device_t* secy_device, uint32_t sa_index)
{
    secy_sa_descriptor_t *SADscr_p;

    if (sa_index >= SA_NUM_MAX)
        goto Err;

    SADscr_p = secy_device->sa_handlers[sa_index].p;

    if (!SADscr_p)
        goto Err;

    return SADscr_p->InUse.SAIndex;

Err:    
    phy_macsec_log(LOG_ERROR, "invalid sa_index %u\n", sa_index);
    return -1;
}

static int macsec_SAHandleToIndex(secy_device_t* secy_device, uint32_t sa_index_active, uint32_t *SAIndex_p, uint32_t *SCIndex_p, uint32_t *SAMFlowCtrlIndex_p)
{
    secy_sa_descriptor_t *SADscr_p;

    if (sa_index_active >= SA_NUM_MAX)
        goto Err;

    SADscr_p = secy_device->sa_handlers[sa_index_active].p;

    if (!SADscr_p)
        goto Err;

    *SAIndex_p = SADscr_p->InUse.SAIndex;

    if (SAMFlowCtrlIndex_p)
    {
        *SAMFlowCtrlIndex_p = SADscr_p->InUse.SAMFlowCtrlIndex;
    }

    if (SCIndex_p)
    {
        *SCIndex_p = SADscr_p->InUse.SCIndex;
    }

    return 0;

Err:    
    phy_macsec_log(LOG_ERROR, "invalid sa_index %u\n", sa_index_active);
    return -1;    
}

static uint32_t macsec_Device_Sync(secy_device_t* secy_device)
{
    uint32_t LoopCounter = ADAPTER_EIP160_MAX_NOF_SYNC_RETRY_COUNT;
    uint32_t numSyncDone = 3;
    uint8_t Unsafe, InFlight;

    macsec_EIP160_SAM_IN_FLIGHT_WR(&secy_device->secy_device_access, 1);
    
    /* Wait till the device synchronization is done */
    while (LoopCounter)
    {
        macsec_EIP160_SAM_IN_FLIGHT_RD(&secy_device->secy_device_access, &Unsafe, &InFlight);
        
        if ((Unsafe == 0) && (--numSyncDone == 0))
            break;

        msleep(10);
        LoopCounter--;
    }

    if (numSyncDone)
    {
        phy_macsec_log(LOG_ERROR, " device sync failed after %u retries. [numSync = %u]\n", LoopCounter, LoopCounter);
        return -1;
    }

    return 0;
}

uint32_t macsec_getRandomNumber(uint8_t *outp_ucRandomNumber, uint32_t in_nLen)
{
    int lenInBlk=0;
    uint8_t dataBlock[SHA_BLK_SIZE_IN_BYTE]={0};
    uint8_t hash[SHA_BLK_SIZE_IN_BYTE]={0};
    int index;

    if (outp_ucRandomNumber==NULL)
    {
       phy_macsec_log(LOG_ERROR, "Null pointer\n");
       return -1;
    }
    if (in_nLen == 0)
    {
        phy_macsec_log(LOG_ERROR, "Invalid data len %d !\n", in_nLen);
        return -1;
    }

    if (in_nLen%SHA_BLK_SIZE_IN_BYTE) 
        lenInBlk=in_nLen/SHA_BLK_SIZE_IN_BYTE+1;
    else 
        lenInBlk=in_nLen/SHA_BLK_SIZE_IN_BYTE;

    for(index=0; index<lenInBlk; index++)
    {
        get_random_bytes(dataBlock, SHA_BLK_SIZE_IN_BYTE);
        sha256(dataBlock, SHA_BLK_SIZE_IN_BYTE, hash);

        memcpy((outp_ucRandomNumber+index*SHA_BLK_SIZE_IN_BYTE), hash, index == (lenInBlk-1) ? (in_nLen-index*SHA_BLK_SIZE_IN_BYTE) : SHA_BLK_SIZE_IN_BYTE);
    }

    return 0;
}

uint32_t macsec_buffer_sanitize(uint8_t *inp_ucBuffer, uint32_t in_nByteCount)
{
    if(inp_ucBuffer==NULL)
    {
        phy_macsec_log(LOG_ERROR, "Invalid parameter!\n");
        return -1;
    }

    memset(inp_ucBuffer, 0, in_nByteCount);
    
    if(macsec_getRandomNumber(inp_ucBuffer, in_nByteCount)!=0)
    {
        phy_macsec_log(LOG_ERROR, "Error generate random number!\n");
        return -1;
    }

    return 0;
}
