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

#include "BcmDiagSerdesEth.h"

#define SERDES_CORE0    0
#define SERDES_CORE1    1

#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM96813)
#define  DIAG_SERDES_CORE_BITMAP    ((1 << SERDES_CORE0) | (1 << SERDES_CORE1))
#define  DIAG_SERDES_CORE_MAX       2
#else
#define  DIAG_SERDES_COREBITMAP     0
#define  DIAG_SERDES_CORE_MAX       1
#endif

uint8_t printfDest = 0;

typedef struct {
    srds_access_t           *phy_dev;
    struct net_device   *dbgDev;
    struct net_device   *gdbDev;
    struct sk_buff      *skbModel;
    struct sk_buff      *skbGdb;
    char                *traceBuf;
    int                 traceBufSize;
    int                 ctrl;
    int                 wrRegMask;
    uint8_t             registered;
    srdsDiagApiDepFunctions *pDepApi;
} bcmSerdesDiagCtrlType;

static bcmSerdesDiagCtrlType serdesDiagCtrl[DIAG_SERDES_CORE_MAX] = { 0 };
static srdsDiagApiDepFunctions *gSrdsDepInfo;

static void BcmSerdesDiagCmd(unsigned char lineId, PDIAGDRV_DIAG pPhyDiag);
static uint8_t  evalRunning = 0;

#define UC_LOAD_NOT_STARTED    0
#define UC_LOAD_STARTED        1
#define UC_LOAD_FINISHED       2

static int               diagFlowCtrl = 1;  /* 0 - Xoff, 1 - Xon */
static int               diagUcLoad   = 0;  /* UC_LOAD_xxx states */
static int               diagUcLoadCnt= 0;  /* uCode mem writes count */

void BcmDiagWriteDebugFlowCtrl(int flowCtrl, int serdesCoreIdx)
{    
    DiagDebugData  debugData;

    diagFlowCtrl     = flowCtrl;
    debugData.cmd    = DIAG_DEBUG_CMD_FLOW_CONTROL;
    debugData.cmdId  = 0;
    debugData.param1 = flowCtrl;
    debugData.param2 = 0;
    debugData.diagData[0] = 0;
    BcmDiagWriteData((serdesCoreIdx << DIAG_DRV_LINE_SHIFT)|(DIAG_SERDES_CLIENT << DIAG_DRV_TYPE_SHIFT)|LOG_CMD_DEBUG,(char *)&debugData, sizeof(debugData), NULL, 0); 
}

int BcmDiagIsSerdesEthCoreMapped(int serdesCoreId)
{
    return (DIAG_SERDES_CORE_BITMAP & (1 << serdesCoreId));
}

uint8 BcmDiagSerdesEvalRunning(void)
{
    return evalRunning;
}

uint8 BcmDiagSerdesRegAcessAllowed(void)
{
    return !evalRunning;
}

void BcmDiagSerdesSetEvalRunning(uint8_t val)
{
    evalRunning = val;
}

void BcmDiagSerdesEthRegister(srds_access_t *phy_dev, srdsDiagApiDepFunctions *pSrdsDiagDepApiFuncs)
{
    if(BcmDiagIsSerdesEthCoreMapped(phy_dev->core_index)) {
        int diagCoreIdx = phy_dev->core_index;
        printk("%s: phy_dev=%08x registered=%d gSrdsDiagDepApiFuncs=%08x\n", __FUNCTION__, (uint32_t)(uint64)phy_dev,
            serdesDiagCtrl[diagCoreIdx].registered, (uint32_t)(uint64)pSrdsDiagDepApiFuncs);
        serdesDiagCtrl[diagCoreIdx].pDepApi = pSrdsDiagDepApiFuncs;
        serdesDiagCtrl[diagCoreIdx].phy_dev = phy_dev;
        serdesDiagCtrl[diagCoreIdx].wrRegMask = 0;
        serdesDiagCtrl[diagCoreIdx].traceBuf = NULL;
        serdesDiagCtrl[diagCoreIdx].registered = 1;
        gSrdsDepInfo = pSrdsDiagDepApiFuncs;
        BcmDiagRegisterClient(DIAG_SERDES_CLIENT, BcmSerdesDiagCmd, TIMER_EXPIRE_10MS);
    }
}

static void _BcmDiagSerdesEthDumpUcodeMem(unsigned char diagCoreIdx, int dumpId)
{
    int i;
    srds_access_t *sa__ = serdesDiagCtrl[diagCoreIdx].phy_dev;
    
    wrc_micro_gen_intr_rmi_mbox0wr(1);
    //wrc_micro_gen_intr_rmi_mbox1wr(1);
    wrc_micro_rmi_to_micro_mbox0(0x1234);
    //wrc_micro_rmi_mbox_send_msgin(1);
    for (i = 0; i < 10000; i++)
       ;
    BcmSerdesDiagDebugFileWriteMem(diagCoreIdx, dumpId);
}

void BcmDiagSerdesEthDumpUcodeMem(int serdesCoreId)
{
    if(BcmDiagIsSerdesEthCoreMapped(serdesCoreId))
        _BcmDiagSerdesEthDumpUcodeMem(serdesCoreId, 0);
}

uint8 BcmDiagSerdesEthPrintToDiag(void)
{
    return (printfDest&1);
}

uint8 BcmDiagSerdesEthGetDiagCoreIdx(void)
{
    return (printfDest >> 1);
}

uint8 *merlin_get_ucode_image (void)
{
    uint8 *pImage;
    
    if(BcmSerdesDiagIsDnloadImageAvailable())
        pImage = BcmSerdesDiagGetDnloadImage();
    else
        pImage = gSrdsDepInfo->pSrdsUcImage;
    
    return pImage;
}

uint16 merlin_get_ucode_image_size (void)
{
    uint16 size;
    
    if(BcmSerdesDiagIsDnloadImageAvailable())
        size = BcmSerdesDiagGetDnloadImageSize();
    else
        size = gSrdsDepInfo->ucImageSize;
    
    return size;
}

uint16 merlin_get_ucode_image_crc (void)
{
    uint16 crc;
    
    if(BcmSerdesDiagIsDnloadImageAvailable())
        crc = BcmSerdesDiagGetDnloadImageCrc();
    else
        crc = gSrdsDepInfo->ucImageCrc;
    
    return crc;
}

static void BcmSerdesDiagDisconnect(unsigned char diagCoreIdx)
{
    serdesDiagCtrl[diagCoreIdx].dbgDev = NULL;
    serdesDiagCtrl[diagCoreIdx].skbModel = NULL;
}

diagImageTestData diagImageParam = { NULL, 0, 0, UN_INIT_HDR_MARK };
uint16  imageCrc = 0;


typedef unsigned short crc_size;
#define POLY_BITREV  0x8408              // CRC-16-CCITT
#define POLY         0x1021              // CRC-16-CCITT
#define WIDTH (8*sizeof(crc_size))
#define MSB   (1 << (WIDTH-1))
#define INITIAL_REMAINDER 0xFFFF
#define SIZE_36KB  36864

static uint16 BcmSerdesDiagCalcCrc16(uint8 *pMem, uint32 memSize)
{
    crc_size crc = INITIAL_REMAINDER;
    unsigned int mem_loc = 0;
    unsigned char  bit_loc = 0;

    /* Calculate CRC for each byte in code memory */
    /* Maybe need to determine BIT_REVERSE from uCode image */
    if (1) { /* BIT_REVERSE */
        for (mem_loc=0; mem_loc<memSize; mem_loc++) {
            crc ^= pMem[mem_loc] & 0xFF;
            for(bit_loc=8; bit_loc>0; --bit_loc)
                crc = (crc >> 1) ^ ((crc & 1) ? POLY_BITREV : 0);
        }
    }
    else {
        for (mem_loc=0; mem_loc<memSize; mem_loc++) {
            crc ^= (((pMem[mem_loc] << (WIDTH - 8)) & 0xFF00));
            for(bit_loc=8; bit_loc>0; --bit_loc) {
                if (crc & MSB) {
                    crc = ((crc << 1) ^ POLY);
                } else {
                    crc = (crc << 1);
                }
            }
        }
    }

    return crc;
}

static void BcmSerdesDiagUpdateTraceBuffer(unsigned char diagCoreIdx)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;
    srds_info_t * info_ptr = pDepApi->srdsGetInfoPtr(serdesDiagCtrl[diagCoreIdx].phy_dev);

    char         *pTmp;

    pTmp = serdesDiagCtrl[diagCoreIdx].traceBuf;
    if (info_ptr->trace_memory_concurrent_read) {
      if ((pTmp != NULL) && (serdesDiagCtrl[diagCoreIdx].traceBufSize < info_ptr->trace_mem_ram_size)) {
        kfree(pTmp);
        pTmp = NULL;
      }
      if (NULL == pTmp) {
        serdesDiagCtrl[diagCoreIdx].traceBufSize = info_ptr->trace_mem_ram_size;
        pTmp = kmalloc(serdesDiagCtrl[diagCoreIdx].traceBufSize, GFP_KERNEL);
      }
    }
    else if (pTmp != NULL) {
      kfree(pTmp);
      pTmp = NULL;
    }
    serdesDiagCtrl[diagCoreIdx].traceBuf = pTmp;
}

static void BcmSerdesDiagResetUcodeInfo(unsigned char diagCoreIdx)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;
    srds_info_t * info_ptr = pDepApi->srdsGetInfoPtr(serdesDiagCtrl[diagCoreIdx].phy_dev);

    if (NULL != info_ptr) {
      info_ptr->signature = 0;
      printk("%s: Resetting info signature\n", __FUNCTION__);
    }
}

void BcmSerdesDiagReloadUcodeInfo(unsigned char diagCoreIdx)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;
    mutex_lock(&serdes_mutex);
    BcmSerdesDiagResetUcodeInfo(diagCoreIdx);
    pDepApi->srdsInitInfo(serdesDiagCtrl[diagCoreIdx].phy_dev);
    BcmSerdesDiagUpdateTraceBuffer(diagCoreIdx);
    mutex_unlock(&serdes_mutex);
}

static void BcmSerdesDiagImageTestMsg(unsigned char diagCoreIdx, void *pMsg)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;
    diagCommandStruct   *pCmd = (void *) (((char *) pMsg) - sizeof(pCmd->command));
    char                *pSdram;
    diagImageTestData   *pImageData;
    char                tmpStr[128];
    uint32_t            frNum = 0, dataLen = 0, n;

    if ( ((uint32_t) -1 == pCmd->param.diagImageTestSpec.imagePtr0) &&
         ((uint32_t) -1 == pCmd->param.diagImageTestSpec.imagePtr) ) {
        
        switch (pCmd->param.diagImageTestSpec.type) {
            case kDiagImageTestLoadImage:
            case kDiagImageTestLoadImageOnly:
                if((pCmd->param.diagImageTestSpec.imageSize0) &&
                   (pCmd->param.diagImageTestSpec.imageSize0 < SIZE_36KB)){
                    if(NULL != diagImageParam.pSdram)
                        kfree(diagImageParam.pSdram);
                    diagImageParam.pSdram = kmalloc(pCmd->param.diagImageTestSpec.imageSize0, GFP_KERNEL);
                    if(NULL != diagImageParam.pSdram) {
                        diagImageParam.size = pCmd->param.diagImageTestSpec.imageSize0;
                        diagImageParam.cnt = diagImageParam.frameCnt = 0;
                    }
                    sprintf(tmpStr, "%s: Download ucode starting, pSdram =%p, imageSize0=%d\n", __FUNCTION__, diagImageParam.pSdram, diagImageParam.size);
                    printk("%s", tmpStr);
                    BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                }
                else {
                    sprintf(tmpStr, "%s: Download ucode size exceeds the limit(%d > %d)\n",
                        __FUNCTION__, pCmd->param.diagImageTestSpec.imageSize0, SIZE_36KB);
                    printk("%s", tmpStr);
                    BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                }
                break;
            default:
                break;
        }
        return;
    }

    if (0 == diagImageParam.size)
        return;

    if (pCmd->param.diagImageTestSpec.imageSize0 != 0) {
        pImageData = &diagImageParam;
        frNum = (uint)(uintptr_t) (pCmd->param.diagImageTestSpec.imagePtr0) & 0xFF;
        dataLen = pCmd->param.diagImageTestSpec.imageSize0;
    }
    else
        pImageData = NULL;

    if (NULL != pImageData) {
        if (frNum != pImageData->frameCnt) {
            BcmDiagImageTestAck (DIAG_SERDES_CLIENT, pImageData, (pImageData->frameCnt - 1) & 0xFF, frNum);
            return;
        }
        n = pImageData->size - pImageData->cnt;
        if (dataLen > n)
            dataLen = n;
        pSdram = pImageData->pSdram + pImageData->cnt;
        memcpy (
            pSdram,
            ((char *) pCmd) + sizeof(pCmd->command) + sizeof(pCmd->param.diagImageTestSpec),
            dataLen);
        pImageData->cnt += dataLen;
        pImageData->frameCnt = (pImageData->frameCnt + 1) & 0xFF;
        BcmDiagImageTestAck (DIAG_SERDES_CLIENT, pImageData, frNum, frNum);
    }

    if (diagImageParam.cnt == diagImageParam.size) {
        switch (pCmd->param.diagImageTestSpec.type) {
            case kDiagImageTestLoadImageOnly:
                BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, "LoadImageOnly Done");
                break;
            case kDiagImageTestLoadImage:
            {
                phy_serdes_t *phy_serdes = serdesDiagCtrl[diagCoreIdx].phy_dev->priv;
                char         *pTmp;

                imageCrc = BcmSerdesDiagCalcCrc16(diagImageParam.pSdram, diagImageParam.size);
                BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, "LoadImage Done");
                pTmp = (char *) diagImageParam.pSdram + 20;
                if ((SRDS_TARGET_ID != *pTmp) && (0 != *pTmp)) {
                  sprintf(tmpStr, "%s: wrong ucode IP(0x%X)\n", __FUNCTION__, *pTmp);
                  printk("%s", tmpStr);
                  BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                  break;
                }

                sprintf(tmpStr, "%s: Download ucode done. Start loading ucode, size=%d, imageCrc=0x%02x\n",
                    __FUNCTION__, diagImageParam.size, imageCrc);
                printk("%s", tmpStr);
                BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                mutex_lock(&serdes_mutex);
                phy_serdes->print_log = 1;
                BcmSerdesDiagResetUcodeInfo(diagCoreIdx);
#ifdef SUPPORT_SERDESDRV_GDB
                if (BcmSerdesGdbAttached())
                    BcmDiagSerdesSetEvalRunning(1);
#endif
                pDepApi->srdsInit(serdesDiagCtrl[diagCoreIdx].phy_dev);
                BcmSerdesDiagUpdateTraceBuffer(diagCoreIdx);
                pDepApi->srdsPostInit(serdesDiagCtrl[diagCoreIdx].phy_dev);
                mutex_unlock(&serdes_mutex);
            }
                break;
        }
        diagImageParam.cnt = 0;
        diagImageParam.frameCnt = UN_INIT_HDR_MARK;
    }
}

int BcmSerdesDiagIsDnloadImageAvailable(void)
{
    return (NULL != diagImageParam.pSdram);
}

uint8 *BcmSerdesDiagGetDnloadImage(void)
{
    char tmpStr[128];

    sprintf(tmpStr, "%s: pSdram=%p\n", __FUNCTION__, diagImageParam.pSdram);
    printk("%s", tmpStr);
    BcmDiagWriteString(0, DIAG_SERDES_CLIENT, tmpStr);

    return diagImageParam.pSdram;
}

uint16 BcmSerdesDiagGetDnloadImageSize(void)
{
    char tmpStr[128];
    uint16 res;
    
    if(diagImageParam.pSdram)
        res = diagImageParam.size;
    else
        res = 0;

    sprintf(tmpStr, "%s: size=%d\n", __FUNCTION__, res);
    printk("%s", tmpStr);
    BcmDiagWriteString(0, DIAG_SERDES_CLIENT, tmpStr);

    return res;
}

uint16 BcmSerdesDiagGetDnloadImageCrc(void)
{
    char tmpStr[128];
    uint16 res;
    
    if(diagImageParam.pSdram)
        res = imageCrc;
    else
        res = 0;

    sprintf(tmpStr, "%s: crc=0x%02x\n", __FUNCTION__, res);
    printk("%s", tmpStr);
    BcmDiagWriteString(0, DIAG_SERDES_CLIENT, tmpStr);

    return res;
}

static void BcmSerdesDiagDebugReadMem(unsigned char diagCoreIdx, DiagDebugData *pDbgCmd)
{
    DiagDebugData   *pDbgRsp;
    uint16_t        n, dataSize;
    uint8_t         testFrame[508], *pData;
    uint32_t        startAddr, endAddr;
    err_code_t      res;
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;

    pDbgRsp = (DiagDebugData *) testFrame;
    pData   = pDbgRsp->diagData;

    pDbgRsp->cmd = pDbgCmd->cmd;
    pDbgRsp->cmdId = pDbgCmd->cmdId;
    dataSize = sizeof(testFrame) - (pData - testFrame);

    if (CMDADDR_REG == (pDbgCmd->param1 & CMDADDR_REG_MASK)) {
        // Register read
        int i;
        uint16_t *pData16 = (uint16_t *)pData;
        uint32_t devId;
        
        devId = (pDbgCmd->param1 & CMDADDR_DEVID_MASK) >> CMDADDR_DEVID_LSB;
        if (0 == devId) /* default to PMD */
            devId = 1;
        startAddr   = pDbgCmd->param1;
        endAddr     = startAddr + pDbgCmd->param2;
        while(startAddr != endAddr) {
            n = (endAddr - startAddr) * sizeof(uint16_t);
            if (n > dataSize)
                n = dataSize;
            pDbgRsp->param1 = startAddr;
            pDbgRsp->param2 = n;
            if (1 == devId) {
              for(i = 0; i < n/2; i++) {
                *(pData16+i) = pDepApi->srdsPmdRdReg(serdesDiagCtrl[diagCoreIdx].phy_dev, (uint16_t)startAddr+i, &res);
                if(res != ERR_CODE_NONE)
                    break;
              }
            }
            else {
              res = ERR_CODE_NONE;
              for(i = 0; i < n/2; i++)
                pDepApi->srdsMacRdReg(serdesDiagCtrl[diagCoreIdx].phy_dev, devId, (uint16_t)startAddr+i, pData16+i);
            }
            if(ERR_CODE_NONE == res)
                BcmDiagWriteData((diagCoreIdx << DIAG_DRV_LINE_SHIFT)|(DIAG_SERDES_CLIENT << DIAG_DRV_TYPE_SHIFT)|LOG_CMD_DEBUG, testFrame, (pData - testFrame) + n, NULL, 0);
            else {
                if(0 != i) {
                    pDbgRsp->param2 = i*sizeof(uint16_t);
                    BcmDiagWriteData((diagCoreIdx << DIAG_DRV_LINE_SHIFT)|(DIAG_SERDES_CLIENT << DIAG_DRV_TYPE_SHIFT)|LOG_CMD_DEBUG, testFrame, (pData - testFrame) + i*sizeof(uint16_t), NULL, 0);
                }
                break;
            }
            startAddr += i;
        }
        return;
    }
    
    // Memory read
    startAddr   = pDbgCmd->param1;
    if(CMDID_READ_MEM_8BITS == pDbgCmd->cmdId)
        endAddr = startAddr + pDbgCmd->param2;
    else if(CMDID_READ_MEM_16BITS == pDbgCmd->cmdId)
        endAddr = startAddr + pDbgCmd->param2*sizeof(uint16_t);
    else if(CMDID_READ_MEM_32BITS == pDbgCmd->cmdId)
        endAddr = startAddr + pDbgCmd->param2*sizeof(uint32_t);
    else
        endAddr = startAddr + pDbgCmd->param2;  // default to CMDID_READ_MEM_8BITS

    while (startAddr != endAddr) {
        srds_rdblk_callback_arg_t   arg;
        n = endAddr - startAddr;
        if (n > dataSize)
            n = dataSize;
        pDbgRsp->param1 = startAddr;
        pDbgRsp->param2 = n;
        arg.mem_ptr = pData;
        arg.sa      = serdesDiagCtrl[diagCoreIdx].phy_dev;
        res = pDepApi->srdsRdBlkRam(serdesDiagCtrl[diagCoreIdx].phy_dev, startAddr, n, 0, n, &arg, pDepApi->srdsRdBlkCallBack);
        if (ERR_CODE_NONE == res)
            BcmDiagWriteData((diagCoreIdx << DIAG_DRV_LINE_SHIFT)|(DIAG_SERDES_CLIENT << DIAG_DRV_TYPE_SHIFT)|LOG_CMD_DEBUG, (char *)testFrame, (pData - testFrame) + n, NULL, 0);
        else
            break;
        startAddr += n;
    }
}

static void BcmSerdesDiagDebugSetMemDev(unsigned char diagCoreIdx, bcmSerdesDiagCtrlType *pDiagCtrl, DiagDebugData *pDbgCmd)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;

    if (CMDADDR_REG == (pDbgCmd->param1 & CMDADDR_REG_MASK)) {
        // Register write.
        uint16_t mask = pDbgCmd->param2 >> 16;
        uint16_t regAddr  = pDbgCmd->param1 & 0xFFFF;
        uint32_t devId;
        
        devId = (pDbgCmd->param1 & CMDADDR_DEVID_MASK) >> CMDADDR_DEVID_LSB;
        if (0 == devId) /* default to PMD */
            devId = 1;

        pDepApi->srdsSetLane(pDiagCtrl->phy_dev, (pDbgCmd->param1 >> 16) & 0x1F);
        /*
        **  uCode loading detection
        */
        if (0xd0f3 == (pDbgCmd->param1 & 0xFFFF))
            pDiagCtrl->wrRegMask = pDbgCmd->param2;
        else {
            if (UC_LOAD_NOT_STARTED == diagUcLoad) {
                if (0xD206 == regAddr) {
                    diagUcLoadCnt++;
                    if (200 == diagUcLoadCnt) {
                        diagUcLoad = UC_LOAD_STARTED;
                        printk("Start Loading code\n");
                    }
                }
                else
                    diagUcLoadCnt = 0;
            }
            else if (UC_LOAD_STARTED == diagUcLoad) {
                if (0xD206 != regAddr)
                    diagUcLoad = UC_LOAD_FINISHED;
                else
                    diagUcLoadCnt++;
                if (UC_LOAD_FINISHED == diagUcLoad)
                    printk("Ucode loading finished cnt = %d reg=0x%X\n", diagUcLoadCnt*2, regAddr);
            }

            /*
            **  uController power up detection for GDB
            */
#ifdef SUPPORT_SERDESDRV_GDB      
            if ( (devId == 1) && (UC_LOAD_FINISHED == diagUcLoad) && (0xD201 == regAddr) &&
                 ((pDbgCmd->param2 & 0xFFFF) != 0) ) {
                if (BcmSerdesGdbAttached() && ((BcmSerdesGdbGetCore()) == diagCoreIdx) ) {
                    uint16_t  reg;
                    err_code_t res;

                    reg = pDepApi->srdsPmdRdReg(pDiagCtrl->phy_dev, 0xd201, &res);
                    //printk("regD201: reg=0x%X mask=0x%X diagMask=0x%X val=0x%X\n", reg, mask, pDiagCtrl->wrRegMask, pDbgCmd->param2);
                    if (0 == mask)
                        reg = (reg & pDiagCtrl->wrRegMask) | (pDbgCmd->param2 & ~pDiagCtrl->wrRegMask);
                    else
                        reg = (reg & ~mask) | (pDbgCmd->param2 & 0xFFFF & mask);
                    //printk("Setting regD201 to 0x%X\n", reg);
                    if (3 == (reg & 3)) {
                        srds_info_t *info_ptr = pDepApi->srdsGetInfoPtr(pDiagCtrl->phy_dev);
                        /* reload uCode info table */
                        //info_ptr->signature = SRDS_INFO_SIGNATURE + 1;
                        //pDepApi->srdsGetInfoPtrWithCheck(pDiagCtrl->phy_dev);
                        info_ptr->signature = 0;
                        printk("uController power up. Pausing remote eval reg[0xd201]=0x%X p2=0x%X\n", reg, pDbgCmd->param2);
                        BcmDiagWriteDebugFlowCtrl(0, diagCoreIdx);
                        if (0 == mask)
                            pDepApi->srdsPmdWrReg(pDiagCtrl->phy_dev, 0xd0f3, 0); /* clear mask */

                        BcmSerdesGdbTrapOnBoot(diagCoreIdx);
                        
                        if (0 == mask)
                            pDepApi->srdsPmdWrReg(pDiagCtrl->phy_dev, 0xd0f3, pDiagCtrl->wrRegMask); /* restore mask */
                        //serdes_pmd_rdt_reg(sa__, 0xd211, &res);
                        //writeGdbMbox(diagCoreIdx, 1 << 8 | PHY_STUB_CONTINUE);
                        //reg = pDepApi->srdsPmdRdReg(pDiagCtrl->phy_dev, 0xd211, &res);
                        //printk("After writing to mailbox %x\n", reg);
                        //Sleep(3000);
                        //printf("Resume remote eval cnt=%d addr=0x%X val=%d\n", pauseCnt, pDiagDbgData->param1, pDiagDbgData->param2);
                        //BcmDiagWriteDebugFlowCtrl(1);
                    }
                }
                else {
                    if (0 == mask)
                        pDepApi->srdsPmdWrReg(pDiagCtrl->phy_dev, 0xd0f3, 0); /* clear mask */
                    
                    //wrc_micro_rmi_to_micro_mbox0(0);
                    pDepApi->srdsPmdWrReg(pDiagCtrl->phy_dev, 0xd211, 0);
                    
                    if (0 == mask) 
                        pDepApi->srdsPmdWrReg(pDiagCtrl->phy_dev, 0xd0f3, pDiagCtrl->wrRegMask); /* restore mask */
                }
            }
#endif        
        
            //if (0xD0F3 == regAddr)
            //    pDiagCtrl->wrRegMask = pDbgCmd->param2;
            if (0 == mask) {
                if (0 == pDiagCtrl->wrRegMask) {
                    if (1 == devId) {
                        if (regAddr == 0xd201)
                            printk("(1)D201: val=0x%x\n", (uint16_t)pDbgCmd->param2);
                        pDepApi->srdsPmdWrReg(pDiagCtrl->phy_dev, regAddr, (uint16_t)pDbgCmd->param2);
                    }
                    else
                        pDepApi->srdsMacWrReg(pDiagCtrl->phy_dev, devId, regAddr, (uint16_t)pDbgCmd->param2);
                }
                else {
                    if (1 == devId) {
                        if (regAddr == 0xd201)
                            printk("(2)D201: mask=0x%x val=0x%x\n", pDiagCtrl->wrRegMask, (uint16_t)pDbgCmd->param2);                    
                        pDepApi->srdsPmdMskWrReg(pDiagCtrl->phy_dev, regAddr, ~pDiagCtrl->wrRegMask, 0, (uint16_t)pDbgCmd->param2);
                    }
                    else
                        pDepApi->srdsMacMskWrReg(pDiagCtrl->phy_dev, devId, regAddr, ~pDiagCtrl->wrRegMask, 0, (uint16_t)pDbgCmd->param2);
                    pDiagCtrl->wrRegMask = 0;
                }
            }
            else {
                if (1 == devId) {
                    if (regAddr == 0xd201)
                        printk("(3)D201:mask =0x%x, val=0x%x\n", mask, (uint16_t)pDbgCmd->param2);
                    pDepApi->srdsPmdMskWrReg(pDiagCtrl->phy_dev, regAddr, mask, 0, (uint16_t)pDbgCmd->param2);
                }
                else
                    pDepApi->srdsMacMskWrReg(pDiagCtrl->phy_dev, devId, regAddr, mask, 0, pDbgCmd->param2 & 0xFFFF);
            }
        }
    }
    else {
        // Memory write
        if((CMDID_SET_MEM_32BITS == pDbgCmd->cmdId) || (CMDID_SET_MEM_32BITS_1 == pDbgCmd->cmdId))
            pDepApi->srdsWrLMem(pDiagCtrl->phy_dev, pDbgCmd->param1, pDbgCmd->param2);
        else if(CMDID_SET_MEM_16BITS == pDbgCmd->cmdId)
            pDepApi->srdsWrWMem(pDiagCtrl->phy_dev, pDbgCmd->param1, (uint16_t)pDbgCmd->param2);
        else    // default to CMDID_SET_MEM_8BITS
            pDepApi->srdsWrBMem(pDiagCtrl->phy_dev, pDbgCmd->param1, (uint8_t)pDbgCmd->param2);
    }
}

static void BcmSerdesDiagDebugSetMem(unsigned char diagCoreIdx, DiagDebugData *pDbgCmd)
{
    if (CMDID_WREAD_BCAST == (pDbgCmd->cmdId & CMDID_WREAD_BCAST_MASK)) { /* core broadcast */
      int  i;

      pDbgCmd->cmdId &= CMDID_WREAD_SIZE_MASK;
      for (i = 0; i < DIAG_SERDES_CORE_MAX; i++) {
        if (serdesDiagCtrl[i].phy_dev != NULL)
            BcmSerdesDiagDebugSetMemDev(diagCoreIdx, &serdesDiagCtrl[i], pDbgCmd);
      }
    }
    else
      BcmSerdesDiagDebugSetMemDev(diagCoreIdx, &serdesDiagCtrl[diagCoreIdx], pDbgCmd);
}

void BcmSerdesDiagDebugFileWriteMem(unsigned char diagCoreIdx, int dumpId)
{
    uint8_t  ucData[1024];
    char     fname[32];
    uint32_t ucStack, size, ucMemPtr;
    srds_rdblk_callback_arg_t   arg;
    srdsDiagApiDepFunctions     *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;

    pDepApi->srdsRdBlkProgRam(serdesDiagCtrl[diagCoreIdx].phy_dev, (uint8_t *)&ucStack, 0, 4);
    BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, "ucode mem_dump: ucStack=0x%X", ucStack);
    if (0 == dumpId)
      dumpId = ucStack;
    snprintf(fname, sizeof(fname), "uc_data_%X", dumpId);
    BcmDiagOpenFile(diagCoreIdx, DIAG_SERDES_CLIENT, fname);

    ucMemPtr = ((ucStack & 0x20000000) != 0) ? 0x20000000 : 0;
    size = ucStack - ucMemPtr;
    while (size > 0) {
      if (size > sizeof(ucData))
        size = sizeof(ucData);
      arg.mem_ptr = ucData;
      arg.sa      = serdesDiagCtrl[diagCoreIdx].phy_dev;
      pDepApi->srdsRdBlkRam(serdesDiagCtrl[diagCoreIdx].phy_dev, ucMemPtr, size, 0, size, &arg, pDepApi->srdsRdBlkCallBack);
      BcmDiagWriteFile(diagCoreIdx, DIAG_SERDES_CLIENT, fname, ucData, size);
      ucMemPtr += size;
      size = ucStack - ucMemPtr;
    }
}

int BcmSerdesDiagGetEvents(unsigned char diagCoreIdx)
{
    srds_rdblk_callback_arg_t   arg;
    int     nTotal = 0, nRead, flags;
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;
    srds_info_t * info_ptr = pDepApi->srdsGetInfoPtr(serdesDiagCtrl[diagCoreIdx].phy_dev);

    do {
      arg.mem_ptr = serdesDiagCtrl[diagCoreIdx].traceBuf;
      arg.sa = serdesDiagCtrl[diagCoreIdx].phy_dev;
      pDepApi->srdsRdEventLogWithCallBack(serdesDiagCtrl[diagCoreIdx].phy_dev, 0, 3, &arg, pDepApi->srdsRdBlkCallBack);
      nRead = (char *)arg.mem_ptr - serdesDiagCtrl[diagCoreIdx].traceBuf;
      if (nRead) {
        flags = (nRead == info_ptr->trace_mem_ram_size) ? kDiagDbgSerdesEventBufOverflow : 0;
        BcmDiagEventData(diagCoreIdx, DIAG_SERDES_CLIENT, serdesDiagCtrl[diagCoreIdx].traceBuf, nRead, flags | kDiagDbgDataFormatHex);
      }
      nTotal += nRead;
    } while (nRead != 0);
    return nTotal;
}

#ifdef SUPPORT_SERDESDRV_GDB

uint32_t GdbMboxAddr;

void getGdbMboxAddr(int core_id)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[core_id].pDepApi;
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    srds_info_t * info_ptr = pDepApi->srdsGetInfoPtrWithCheck(sa__);
    
    printk("***%s: mbox_addr= %x\n", __FUNCTION__, info_ptr->micro_var_ram_base+GDB_MBOX_OFFSET);
    GdbMboxAddr = info_ptr->micro_var_ram_base + GDB_MBOX_OFFSET;
    //return (info_ptr->micro_var_ram_base + GDB_MBOX_OFFSET);
}

void serdesGenInterrupt(int core_id)
{
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    wrc_micro_gen_intr_rmi_mbox0wr(1);
    wrc_micro_rmi_to_micro_mbox0(0x0109);
}

void serdesClearInterrupt(int core_id)
{
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    wrc_micro_gen_intr_rmi_mbox0wr(0);
}

void writeGdbMbox(int core_id, uint16_t val)
{
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    wrc_micro_rmi_to_micro_mbox0(val);
}

uint16_t readGdbMbox(int core_id)
{
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    err_code_t __err;
    return rdc_micro_to_rmi_mbox0();
}

void phy_read_regs(int core_id, int32_t *regs)
{
    srds_rdblk_callback_arg_t arg;
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[core_id].pDepApi;
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    srds_info_t * info_ptr = pDepApi->srdsGetInfoPtrWithCheck(sa__);
    uint32_t sp_addr = info_ptr->micro_var_ram_base + MERLIN_SP_OFFSET;
    uint32_t sp, reg_addr;
    //int i;
    arg.mem_ptr = (char *)&sp;
    arg.sa = sa__;

    pDepApi->srdsRdBlkRam(sa__, sp_addr, 4, 0, 4, &arg, pDepApi->srdsRdBlkCallBack);
    //printk("***%s sp=%x\n", __FUNCTION__, sp);
    reg_addr = sp; 
    /* read registers r0-r3 */
    arg.mem_ptr = (char *)regs;
    pDepApi->srdsRdBlkRam(sa__, reg_addr, 4*4, 0, 4*4, &arg, pDepApi->srdsRdBlkCallBack);
    /* read registers r4-r7 */
    arg.mem_ptr = (char *)(regs + 4);
    reg_addr = info_ptr->micro_var_ram_base + MERLIN_R4_OFFSET;
    pDepApi->srdsRdBlkRam(sa__, reg_addr, 4*4, 0, 4*4, &arg, pDepApi->srdsRdBlkCallBack);
    /* read register r12 */
    arg.mem_ptr = (char *)(regs+12);
    reg_addr = sp + 16;
    pDepApi->srdsRdBlkRam(sa__, reg_addr, 4, 0, 4, &arg, pDepApi->srdsRdBlkCallBack);
    /* read register r14, r15(pc) */
    arg.mem_ptr = (char *)(regs+14);
    reg_addr = sp + 20;
    pDepApi->srdsRdBlkRam(sa__, reg_addr, 4*2, 0, 4*2, &arg, pDepApi->srdsRdBlkCallBack);
    /* read register cpsr */
    arg.mem_ptr = (char *)(regs+41);
    reg_addr = sp + 28;
    pDepApi->srdsRdBlkRam(sa__, reg_addr, 4, 0, 4, &arg, pDepApi->srdsRdBlkCallBack);
    regs[41] |= 0x00000020;
    /* save sp (r13) */
    sp += 0x20;
    regs[13] = sp;

#if 0
    printk("+++%s: registers:\n", __FUNCTION__);
    printk("addr regs=%p\n", regs);
    for (i=0; i < 16; i++)
        printk("reg[%d]=%x\n", i, regs[i]);
    printk("cpsr=%x\n", regs[41]);
#endif
}

int phy_read_mem(int core_id, char *buf, int len, uint32_t addr)
{
    srds_rdblk_callback_arg_t arg;
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[core_id].pDepApi;
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    arg.mem_ptr = buf;
    arg.sa = sa__;

    if ((addr >> 28) == 0x5) {
        err_code_t res;
        uint32_t devId = (addr & CMDADDR_DEVID_MASK) >> CMDADDR_DEVID_LSB;
        if (0 == devId) /* default to PMD */
            devId = 1;
        if (devId == 1) {
            *(uint16_t *)buf = pDepApi->srdsPmdRdReg(sa__, (uint16_t)(addr & 0xffff), &res);
            //printk("***%s: %x:%x\n", __FUNCTION__, addr, *(uint16_t *)buf);
        }
        else {
            pDepApi->srdsMacRdReg(sa__, devId, (uint16_t)(addr & 0xffff), (uint16_t *)buf);
        }
    }
    else
        pDepApi->srdsRdBlkRam(sa__, addr, len, 0, len, &arg, pDepApi->srdsRdBlkCallBack);
    return 1;    
}

int phy_write_mem(int core_id, char *buf, int len, uint32_t addr)
{
    srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[core_id].pDepApi;
    srds_access_t *sa__ = serdesDiagCtrl[core_id].phy_dev;
    //int i;
    //printk("***%s: addr=%x len=%d\n", __FUNCTION__, addr, len);
    //for (i=0; i < len; i++)
    //printk("buf[%d]=%x\n", i, buf[i]);
    if ((addr >> 28) == 0x5) {
        //printk("***%s: %x: %x\n", __FUNCTION__, (uint16_t)(addr & 0xffff), *(uint16_t *)buf);
        uint32_t devId = (addr & CMDADDR_DEVID_MASK) >> CMDADDR_DEVID_LSB;
        if (0 == devId) /* default to PMD */
            devId = 1;
        if (devId == 1)
            pDepApi->srdsPmdWrReg(sa__, (uint16_t)(addr & 0xffff), *(uint16_t *)buf);
        else
            pDepApi->srdsMacWrReg(sa__, devId, (uint16_t)(addr & 0xffff), *(uint16_t *)buf);
    }
    else {
        while (len > 0) {
            pDepApi->srdsWrBMem(sa__, addr, *buf);
            len--;
            buf++;
            addr++;
        }
    }
    return 1;
}
#endif

static void BcmSerdesDiagCmd(unsigned char diagCoreIdx, PDIAGDRV_DIAG pPhyDiag)
{
    int     i, res;
    char    tmpStr[128];
#if 0
    sprintf(tmpStr, "%s: diagCmd=%d\n", __FUNCTION__, pPhyDiag->diagCmd);
    printk("%s", tmpStr);
    BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
#endif
    if((diagCoreIdx >= DIAG_SERDES_CORE_MAX) || !serdesDiagCtrl[diagCoreIdx].registered)
        return;

    switch (pPhyDiag->diagCmd) {
        case LOG_CMD_CONNECT:
        {
            if(pPhyDiag->diagMap & DIAG_DATA_GDB_ID) {
#ifdef SUPPORT_SERDESDRV_GDB
                GdbStubInit(diagCoreIdx);
                serdesDiagCtrl[diagCoreIdx].skbGdb = (struct sk_buff *)pPhyDiag->skbModel;
                serdesDiagCtrl[diagCoreIdx].gdbDev = serdesDiagCtrl[diagCoreIdx].skbGdb->dev;
#else
                serdesDiagCtrl[diagCoreIdx].gdbDev = NULL;
#endif
                sprintf(tmpStr, "%s: gdbDev=0x%px skbGdb=0x%px\n", __FUNCTION__, serdesDiagCtrl[diagCoreIdx].gdbDev, serdesDiagCtrl[diagCoreIdx].skbGdb);
            }
            else {
                serdesDiagCtrl[diagCoreIdx].skbModel           = (struct sk_buff *)pPhyDiag->skbModel;
                serdesDiagCtrl[diagCoreIdx].dbgDev         = serdesDiagCtrl[diagCoreIdx].skbModel->dev;
                BcmSerdesDiagUpdateTraceBuffer(diagCoreIdx);
                sprintf(tmpStr, "%s: pDev=0x%px skbModel=0x%px\n", __FUNCTION__, serdesDiagCtrl[diagCoreIdx].dbgDev, serdesDiagCtrl[diagCoreIdx].skbModel);
            }
            printk("%s", tmpStr);
            BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
#ifdef SUPPORT_SERDESDRV_GDB
            diagFlowCtrl = 1;  /* 0 - Xoff, 1 - Xon */
            diagUcLoad   = 0;  /* UC_LOAD_xxx states */
            diagUcLoadCnt= 0;  /* uCode mem writes count */
#endif
        }
        break;
        case LOG_CMD_ENABLE_CLIENT: /* sent after connect command */
        {
            printk("*** %s: Diag enable SERDES client request received ***\n", __FUNCTION__);
            if((NULL == serdesDiagCtrl[diagCoreIdx].skbModel) && (NULL == serdesDiagCtrl[diagCoreIdx].dbgDev) && (NULL != pPhyDiag->skbModel)) {
                serdesDiagCtrl[diagCoreIdx].skbModel           = (struct sk_buff *)pPhyDiag->skbModel;
                serdesDiagCtrl[diagCoreIdx].dbgDev         = serdesDiagCtrl[diagCoreIdx].skbModel->dev;
                printk("%s: pDev=0x%px skbModel=0x%px\n", __FUNCTION__, serdesDiagCtrl[diagCoreIdx].dbgDev, serdesDiagCtrl[diagCoreIdx].skbModel);
            }
        }
        break;
        case LOG_CMD_DISABLE_CLIENT:
        {
            printk("*** %s: Diag disable SERDES client request received ***\n", __FUNCTION__);
            serdesDiagCtrl[diagCoreIdx].skbModel       = NULL;
            serdesDiagCtrl[diagCoreIdx].dbgDev     = NULL;
        }
            break;
        case LOG_CMD_DISCONNECT:
        {
            BcmSerdesDiagDisconnect(diagCoreIdx);
        }
            break;
        case LOG_CMD_DEBUG:
        {
            DiagDebugData *pDbgCmd = (void *)pPhyDiag->diagMap;
            srdsDiagApiDepFunctions *pDepApi = serdesDiagCtrl[diagCoreIdx].pDepApi;
            //printk("%s: Executing dbgcmd=%d 0x%08x 0x%08x\n", __FUNCTION__, pDbgCmd->cmd, pDbgCmd->param1, pDbgCmd->param2);
            
            switch (pDbgCmd->cmd) {
                case DIAG_DEBUG_CMD_RESET_PHY:
                {
                    phy_serdes_t *phy_serdes = serdesDiagCtrl[diagCoreIdx].phy_dev->priv;
                    sprintf(tmpStr, "%s: Resetting PHY, phy_dev=%p inited=%d\n", __FUNCTION__, serdesDiagCtrl[diagCoreIdx].phy_dev, phy_serdes->inited);
                    printk("%s", tmpStr);
                    BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                    mutex_lock(&serdes_mutex);
                    if(NULL != diagImageParam.pSdram) {
                        kfree(diagImageParam.pSdram);
                        diagImageParam.pSdram = NULL;
                    }
                    phy_serdes->print_log = 1;
                    if(pDbgCmd->param1)
                        phy_serdes->inited = 0;
                    BcmSerdesDiagResetUcodeInfo(diagCoreIdx);
                    pDepApi->srdsInit(serdesDiagCtrl[diagCoreIdx].phy_dev);
                    BcmSerdesDiagUpdateTraceBuffer(diagCoreIdx);
                    pDepApi->srdsPostInit(serdesDiagCtrl[diagCoreIdx].phy_dev);
                    mutex_unlock(&serdes_mutex);
                }
                    break;
                case DIAG_DEBUG_CMD_LOG_DATA:
                    if(pDbgCmd->param1 & 1) {   // eyeData
                        mutex_lock(&serdes_mutex);
                        printfDest = (diagCoreIdx << 1) | 1;
                        res = pDepApi->srdsDisplayDiagData(serdesDiagCtrl[diagCoreIdx].phy_dev, SRDS_DIAG_EYE, 1 | (EYE_FORMAT_RAW << 8));
                        printfDest = 0;
                        mutex_unlock(&serdes_mutex);
                        sprintf(tmpStr, "%s: merlinx_display_diag_data( , SRDS_DIAG_EYE), ret=%d\n", __FUNCTION__, res);
                        printk("%s", tmpStr);
                        BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                    }
                    break;
                case DIAG_DEBUG_CMD_SERDES_DIAG:
                {
                    mutex_lock(&serdes_mutex);
                    printfDest = (diagCoreIdx << 1) | (pDbgCmd->cmdId != 0);    // Print output to serial console(0)/Diag(1)/Diags in console format(2)
                    res = pDepApi->srdsDisplayDiagData(serdesDiagCtrl[diagCoreIdx].phy_dev, pDbgCmd->param1,
                                pDbgCmd->param2 | (((1 == pDbgCmd->cmdId) ? EYE_FORMAT_RAW: EYE_FORMAT_REG) << 8)); // pDbgCmd->param2 = lane_select
                    printfDest = 0;
                    mutex_unlock(&serdes_mutex);
                    
                    sprintf(tmpStr, "%s: merlinx_display_diag_data( , 0x%X), ret=%d\n", __FUNCTION__, pDbgCmd->param1, res);
                    printk("%s", tmpStr);
                    BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                }
                    break;
                case DIAG_DEBUG_CMD_READ_MEM:
                    mutex_lock(&serdes_mutex);
                    BcmSerdesDiagDebugReadMem(diagCoreIdx, pDbgCmd);
                    mutex_unlock(&serdes_mutex);
                    break;
                case DIAG_DEBUG_CMD_SET_MEM:
                    mutex_lock(&serdes_mutex);
                    BcmSerdesDiagDebugSetMem(diagCoreIdx, pDbgCmd);
                    mutex_unlock(&serdes_mutex);
                    break;
                case DIAG_DEBUG_CMD_RESET_GET_CAPS:
                {
                    uint8_t         testFrame[sizeof(DiagDebugData) + 2];
                    DiagDebugData   *pDbgRsp = (DiagDebugData *) testFrame;
                    uint16_t        *pData   = (uint16_t *) pDbgRsp->diagData;

                    pDbgRsp->cmd = DIAG_DEBUG_CMD_READ_MEM;
                    pDbgRsp->cmdId = CMDID_READ_MEM_16BITS;
                    pDbgRsp->param1 = pDbgCmd->param1;
                    pDbgRsp->param2 = 2;
                    *pData = SYSCAP_RESET_DONE + SYSCAP_UCLOAD;
                    //*pData = SYSCAP_RESET_DONE + SYSCAP_UCLOAD + SYSCAP_MASKED_WRITE;
                    BcmDiagWriteData((diagCoreIdx << DIAG_DRV_LINE_SHIFT)|(DIAG_SERDES_CLIENT << DIAG_DRV_TYPE_SHIFT)|LOG_CMD_DEBUG,
                        testFrame, sizeof(DiagDebugData) + 2, NULL, 0);
                }
                    break;

                case DIAG_DEBUG_CMD_SERDES_CMD:
                    {
                    srds_access_t *sa__ = serdesDiagCtrl[diagCoreIdx].phy_dev;

                    mutex_lock(&serdes_mutex);
                    reg_wr_DSC_A_DSC_SCRATCH(pDbgCmd->param2);
                    pDepApi->srdsUcCmd(serdesDiagCtrl[diagCoreIdx].phy_dev, pDbgCmd->cmdId, pDbgCmd->param1, 200);
                    mutex_unlock(&serdes_mutex);
                    }
                    break;
                case DIAG_DEBUG_CMD_SERDES_CTRL:
                    serdesDiagCtrl[diagCoreIdx].ctrl = pDbgCmd->param1;
                    break;

                case DIAG_DEBUG_CMD_WRITE_FILE:
                    mutex_lock(&serdes_mutex);
                    if ((int) pDbgCmd->param1 < 0)
                      _BcmDiagSerdesEthDumpUcodeMem(diagCoreIdx, -pDbgCmd->param1);
                    else
                      BcmSerdesDiagDebugFileWriteMem(diagCoreIdx, pDbgCmd->param1);
                    mutex_unlock(&serdes_mutex);
                    break;

                case DIAG_DEBUG_CMD_READ_EVENTS:
                    if (serdesDiagCtrl[diagCoreIdx].traceBuf) {
                      mutex_lock(&serdes_mutex);
                      BcmSerdesDiagGetEvents(diagCoreIdx);
                      mutex_unlock(&serdes_mutex);
                    }
                    break;

                case DIAG_DEBUG_CMD_RESET_UCODE_INFO:
                    BcmSerdesDiagReloadUcodeInfo(diagCoreIdx);
                    break;

                case DIAG_DEBUG_CMD_EVAL_TEST:
                    if (-1 == pDbgCmd->param1) {
#ifdef SUPPORT_SERDESDRV_GDB
                      BcmDiagWriteString(0, DIAG_SERDES_CLIENT, "eval=%d gdb=%d", evalRunning, BcmSerdesGdbAttached());
#else
                      BcmDiagWriteString(0, DIAG_SERDES_CLIENT, "eval=%d", evalRunning);
#endif
                    }
                    else if (0 == pDbgCmd->param1) {
                      evalRunning &= ~(1 << diagCoreIdx);
                      BcmSerdesDiagReloadUcodeInfo(diagCoreIdx);
                    }
                    else
                      evalRunning |= (1 << diagCoreIdx);
                    break;

                case DIAG_DEBUG_CMD_TIMER_EXPIRE:
                    if (TIMER_EXPIRE_10MS == pDbgCmd->param1) {
                      for(i = 0; i < DIAG_SERDES_CORE_MAX; i++)
                      if ((serdesDiagCtrl[i].ctrl & SRDS_CTRL_EVENT_MONITORING) && serdesDiagCtrl[i].traceBuf) {
                        if (0 == mutex_trylock(&serdes_mutex))
                          break;
                        BcmSerdesDiagGetEvents(i);
                        mutex_unlock(&serdes_mutex);
                      }
#ifdef SUPPORT_SERDESDRV_GDB
                      if (0 == mutex_trylock(&serdes_mutex))
                          break;
                      BcmSerdesGdbTask();
                      mutex_unlock(&serdes_mutex);
#endif
                    }
                    break;
                default:
                    break;
            }
        }
            break;
        case LOG_CMD_GDB:
#ifdef SUPPORT_SERDESDRV_GDB
            {
                //char    tmpStr[1024];
                //sprintf(tmpStr, "%s: Received LOG_CMD_GDB command %s\n", __FUNCTION__, (char *)pPhyDiag->diagMap);
                //printk("%s", tmpStr);
                mutex_lock(&serdes_mutex);
                //BcmDiagWriteString(diagCoreIdx, DIAG_SERDES_CLIENT, tmpStr);
                BcmSerdesGdbCmd((void *)pPhyDiag->diagMap, pPhyDiag->logTime);
                if (!BcmSerdesGdbAttached())
                    BcmDiagSerdesSetEvalRunning(0);  
                //printk("***%s: after BcmSerdesGdbCmd\n", __FUNCTION__);
                mutex_unlock(&serdes_mutex);
            }
#endif
            break;
        case (kDiagLoadPhyTestCmd - kFirstDiagCommandCode):
            BcmSerdesDiagImageTestMsg(diagCoreIdx, (void *)pPhyDiag->diagMap);
            break;
        default:
            break;
    }
}

