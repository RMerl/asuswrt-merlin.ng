/*
   Copyright (c) 2023 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2023:DUAL/GPL:standard

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

#ifndef _BCM_DIAG_SERDES_ETH_H
#define _BCM_DIAG_SERDES_ETH_H

#include "phy_drv_dsl_serdes.h"
#include "Serdes146Class/phy_drv_merlin16.h"

#include "serdes_access.h"

#include "merlin16_shortfin_diag.h"


#include "BcmDiagDrv.h"
#include "DiagDef.h"

#define DIAG_SERDES_CLIENT  DIAG_SRDS_ETH_CLIENT
#define SRDS_TARGET_ID      SRDS_MERLIN16_SHORTFIN_ID

#define _merlin16_shortfin_ucode_image          merlin_get_ucode_image()
#define _MERLIN16_SHORTFIN_UCODE_IMAGE_SIZE     merlin_get_ucode_image_size()
#define _MERLIN16_SHORTFIN_UCODE_IMAGE_CRC      merlin_get_ucode_image_crc()

#define srds_rdblk_callback_arg_t   merlin16_shortfin_INTERNAL_rdblk_callback_arg_t
typedef srds_info_t *(*srdsGetInfoPtrType) (srds_access_t *sa__);
typedef int (*srdsInitType) (phy_dev_t *phy_dev);
typedef int (*srdsPostInitType) (phy_dev_t *phy_dev);
typedef err_code_t (*srdsDisplayDiagDataType)(srds_access_t *sa__, uint16_t diag_level, uint16_t lane_select);
typedef err_code_t (*srdsUcCmdType)(srds_access_t *sa__, enum srds_pmd_uc_cmd_enum cmd, uint8_t supp_info, uint32_t timeout_ms);
typedef uint16_t (*srdsPmdRdRegType)(srds_access_t *sa__, uint16_t addr, err_code_t *err_code_p);
typedef int (*srdsMacRdRegType)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val);
typedef err_code_t (*srdsRdBlkCallBackType)(void *arg, uint8_t byte_count, uint16_t data);
typedef err_code_t (*srdsRdBlkRamType)(srds_access_t *sa__,
                                                uint32_t block_addr,
                                                uint16_t block_size,
                                                uint16_t start_offset,
                                                uint16_t cnt,
                                                void *arg,
                                                err_code_t (*callback)(void *, uint8_t, uint16_t));
typedef err_code_t (*srdsRdBlkProgRamType)(srds_access_t *sa__, uint8_t *mem, uint16_t addr, uint16_t cnt);
typedef err_code_t (*srdsRdEventLogWithCallBackType)(srds_access_t *sa__,
                                                        uint8_t micro_num,
                                                        uint8_t bypass_micro,
                                                        void *arg,
                                                        err_code_t (*callback)(void *, uint8_t, uint16_t));
typedef err_code_t (*srdsPmdWrRegType)(srds_access_t *sa__, uint16_t addr, uint16_t valu16);
typedef int (*srdsMacWrRegType)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val);
typedef err_code_t (*srdsPmdMskWrRegType)(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint16_t valu16);
typedef int (*srdsMacMskWrRegType)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t mask, uint16_t shift, uint16_t val);
typedef err_code_t (*srdsWrLMemType)(srds_access_t *sa__, uint32_t addr, uint32_t wr_val);
typedef err_code_t (*srdsWrWMemType)(srds_access_t *sa__, uint32_t addr, uint16_t wr_val);
typedef err_code_t (*srdsWrBMemType)(srds_access_t *sa__, uint32_t addr, uint8_t wr_val);
typedef err_code_t (*srdsInitInfoType)(srds_access_t *sa__);
typedef err_code_t (*srdsSetLaneType)(srds_access_t *sa__, uint8_t lane_index);
typedef srds_info_t *(*srdsGetInfoPtrWithCheckType)(srds_access_t *sa__);

typedef struct
{
    uint8   *pSrdsUcImage;
    uint16  ucImageSize;
    uint16  ucImageCrc;
    srdsGetInfoPtrType  srdsGetInfoPtr;
    srdsInitType        srdsInit;
    srdsPostInitType    srdsPostInit;
    srdsDisplayDiagDataType srdsDisplayDiagData;
    srdsUcCmdType       srdsUcCmd;
    srdsPmdRdRegType    srdsPmdRdReg;
    srdsMacRdRegType    srdsMacRdReg;
    srdsRdBlkCallBackType   srdsRdBlkCallBack;
    srdsRdBlkRamType        srdsRdBlkRam;
    srdsRdBlkProgRamType    srdsRdBlkProgRam;
    srdsRdEventLogWithCallBackType  srdsRdEventLogWithCallBack;
    srdsPmdWrRegType    srdsPmdWrReg;
    srdsMacWrRegType    srdsMacWrReg;
    srdsPmdMskWrRegType srdsPmdMskWrReg;
    srdsMacMskWrRegType srdsMacMskWrReg;
    srdsWrLMemType      srdsWrLMem;
    srdsWrWMemType      srdsWrWMem;
    srdsWrBMemType      srdsWrBMem;
    srdsInitInfoType    srdsInitInfo;
    srdsSetLaneType     srdsSetLane;
    srdsGetInfoPtrWithCheckType srdsGetInfoPtrWithCheck;
} srdsDiagApiDepFunctions;


void BcmDiagSerdesEthRegister(phy_dev_t *phy_dev, srdsDiagApiDepFunctions *pSrdsDiagDepApiFuncs);
int BcmSerdesDiagIsDnloadImageAvailable(void);
uint8 *BcmSerdesDiagGetDnloadImage(void);
uint16 BcmSerdesDiagGetDnloadImageSize(void);
uint16 BcmSerdesDiagGetDnloadImageCrc(void);
phy_dev_t *phy_drv_serdes146_get_diag_dev(int serdesCoreId);
void BcmSerdesDiagDebugFileWriteMem(unsigned char diagCoreIdx, int dumpId);
void BcmDiagSerdesEthDumpUcodeMem(int serdesCoreId);
uint8 BcmDiagSerdesEvalRunning(void);
uint8 BcmDiagSerdesRegAcessAllowed(void);
uint8 BcmDiagSerdesEthPrintToDiag(void);
uint8 BcmDiagSerdesEthGetDiagCoreIdx(void);
void BcmDiagSerdesSetEvalRunning(uint8_t val);
uint8 *merlin_get_ucode_image (void);
uint16 merlin_get_ucode_image_size (void);
uint16 merlin_get_ucode_image_crc (void);

#ifdef SUPPORT_SERDESDRV_GDB
void GdbStubInit(int core_id);
void writeGdbMbox(int core_id, uint16_t cmd);
uint16_t readGdbMbox(int core_id);
void BcmSerdesGdbCmd(void *pCmd, int cmdLen);
void BcmSerdesGdbTask(void);
void phy_read_regs(int core_id, int32_t *regs);
int phy_read_mem(int core_id, char *buf, int len, uint32_t addr);
int phy_write_mem(int core_id, char *buf, int len, uint32_t addr);
void getGdbMboxAddr(int core_id);
void serdesGenInterrupt(int core_id);
void serdesClearInterrupt(int core_id);
int BcmSerdesGdbAttached(void);
int BcmSerdesGdbTrapOnBoot(int core_id);
int BcmSerdesGdbGetCore(void);
void BcmSerdesGdbSetCore_ID(int core);
void BcmDiagWriteDebugFlowCtrl(int flowCtrl, int core);
#endif

#define GDB_MBOX_OFFSET 48
#define MERLIN_SP_OFFSET 12
#define MERLIN_R4_OFFSET 32

#endif

