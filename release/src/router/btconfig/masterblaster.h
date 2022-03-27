/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in the
 *          documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *          the names of its contributors may be used to endorse or promote
 *          products derived from this software without specific prior written
 *          permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MASTERBLASTER_H_
#define _MASTERBLASTER_H_
#include "btconfig.h"

#define INVALID_MASTERBLASTER_FIELD    (-1)
#define UNUSED(x) (x=x)
// Bluetooth Packet Type Identifiers
#define  MAX_TRANSMIT_POWER_CONTROL_ENTRIES    15

#define LC_JTAG_MODEM_REGS_ADDRESS               0x00020800
#define AGC_BYPASS_ADDRESS                       0x0000040c
#define AGC_BYPASS_ENABLE_MASK                   0x00000001
#define AGC_BYPASS_ENABLE_LSB                    0
#define AGC_BYPASS_ENABLE_SET(x)                 (((x) << AGC_BYPASS_ENABLE_LSB) & AGC_BYPASS_ENABLE_MASK)
#define LC_DEV_PARAM_CTL_ADDRESS                 0x00020010
#define LC_DEV_PARAM_CTL_FREQ_HOP_EN_MASK        0x00000001
#define LC_DEV_PARAM_CTL_RX_FREQ_MASK            0x00007f00
#define LC_DEV_PARAM_CTL_WHITEN_EN_MASK          0x00008000
#define LC_DEV_PARAM_CTL_RX_FREQ_SET(x)          (((x) << LC_DEV_PARAM_CTL_RX_FREQ_LSB) & LC_DEV_PARAM_CTL_RX_FREQ_MASK)
#define LC_DEV_PARAM_CTL_RX_FREQ_LSB             8

#define CORR_PARAM1_ADDRESS                      0x00000048
#define CORR_PARAM1_TIM_THR_MASK                 0xfc000000
#define CORR_PARAM1_TIM_THR_LSB                  26
#define CORR_PARAM1_TIM_THR_SET(x)               (((x) << CORR_PARAM1_TIM_THR_LSB) & CORR_PARAM1_TIM_THR_MASK)

typedef struct tBtHost_Interest {
  UINT16 MagicNumber;
  UINT16 Version;
  UINT32 TraceDataAddr;
  UINT32 GlobalDmaStats;   // BRM Global DMA Statistics
  UINT32 TpcTableAddr;     // SysCfg Transmit power table
  UINT32 AudioStatAddr;
  UINT32 AudioInternalAddr;
  UINT32 SysCfgAddr;
  UINT32 ReservedRamAddr;
  UINT32 HostConfigAddr;
  UINT32 RamVersionAddr;
  // Version 1.01
  UINT32 BrmGlobalDataAddr;  // BRM Global Context pointer
  UINT32 LeGlobalDataAddr;   // LE Global Context pointer
  // Version 1.02
  UINT32 MmGlobalDataAddr;
} tBtHostInterest;

typedef struct PsSysCfgTransmitPowerControlEntry {
    SINT8               TxPowerLevel;
    UINT32              RadioConfig;
    UINT32              EdrRadioConfig;
    SINT16              Slope;
    SINT16              EdrSlope;
    UINT8               TxFreqCorr;
    UINT8               EdrTxFreqCorr;
} tPsSysCfgTransmitPowerControlEntry;

typedef struct PsSysCfgTransmitPowerControlTable {
  int  NumOfEntries;
  tPsSysCfgTransmitPowerControlEntry t[MAX_TRANSMIT_POWER_CONTROL_ENTRIES];
} tPsSysCfgTransmitPowerControlTable;
typedef UCHAR tBRM_PktType;

enum {
   TxTest_PktType_NULL    = 0x00,
   TxTest_PktType_POLL    = 0x01,
   TxTest_PktType_FHS     = 0x02,
   TxTest_PktType_DM1     = 0x03,
   TxTest_PktType_DH1     = 0x04,
   TxTest_PktType_HV1     = 0x05,
   TxTest_PktType_HV2     = 0x06,
   TxTest_PktType_HV3     = 0x07,
   TxTest_PktType_DV      = 0x08,
   TxTest_PktType_AUX1    = 0x09,
   TxTest_PktType_DM3     = 0x0A,
   TxTest_PktType_DH3     = 0x0B,
   TxTest_PktType_DM5     = 0x0E,
   TxTest_PktType_DH5     = 0x0F,
   TxTest_PktType_2DH1    = 0x24,
   TxTest_PktType_2DH3    = 0x2A,
   TxTest_PktType_2DH5    = 0x2E,
   TxTest_PktType_3DH1    = 0x28,
   TxTest_PktType_3DH3    = 0x2B,
   TxTest_PktType_3DH5    = 0x2F,
   TxTest_PktType_Invalid = 0xff,
};

typedef UCHAR tBRM_eSCO_PktType;

enum {
   TxTest_PktType_EV3     = 0x17,
   TxTest_PktType_EV4     = 0x1C,
   TxTest_PktType_EV5     = 0x1D,
   TxTest_PktType_2EV3    = 0x36,
   TxTest_PktType_2EV5    = 0x3C,
   TxTest_PktType_3EV3    = 0x37,
   TxTest_PktType_3EV5    = 0x3D,
};

typedef UCHAR tBRM_PktMode;

enum {
   eBRM_Mode_Basic = 0,
   eBRM_Mode_2Mbps = 2,
   eBRM_Mode_3Mbps = 3
};

// tBRM_TestMode
enum {
   eBRM_TestMode_Pause = 0,
   eBRM_TestMode_TX_0,
   eBRM_TestMode_TX_1,
   eBRM_TestMode_TX_1010,
   eBRM_TestMode_TX_PRBS,
   eBRM_TestMode_Loop_ACL,
   eBRM_TestMode_Loop_SCO,
   eBRM_TestMode_Loop_ACL_No_Whitening,
   eBRM_TestMode_Loop_SCO_No_Whitening,
   eBRM_TestMode_TX_11110000,
   eBRM_TestMode_Rx,
   eBRM_TestMode_Exit = 255,
};
enum {
   Cont_Tx_Raw_1MHz = 0,
   Cont_Tx_Raw_2MHz,
   Cont_Tx_Raw_3MHz,
   Cont_Tx_Regular,
   CW_Single_Tone,
};
typedef UCHAR tBRM_TestMode;

typedef struct tBRM_TestControl {
   tBRM_TestMode       Mode;
   UCHAR               HopMode;
   UCHAR               TxFreq;
   UCHAR               RxFreq;
   UCHAR               Power;
// UCHAR               PollPeriod;
   UCHAR               Packet;
   UCHAR               SkipRxSlot;
   int              DataLen;
} tBRM_TestControl;

typedef struct tLE_TxParms {
   UCHAR PktPayload;
} tLE_TxParms;

typedef struct tBRM_Control_packet {
   tBRM_TestControl testCtrl;
   UCHAR bdaddr[6];
   UCHAR ContTxMode;    // Continuous TX Mode
   UCHAR ContTxType;
   UCHAR ContRxMode;    // Continuous RX Mode
   UCHAR BERType;
   UCHAR LERxMode;
   UCHAR LETxMode;
   tLE_TxParms LETxParms;
} tBRM_Control_packet;


#define DM1_MAX_PAYLOAD             17
#define DH1_MAX_PAYLOAD             27
#define DM3_MAX_PAYLOAD             121
#define DH3_MAX_PAYLOAD             183
#define DM5_MAX_PAYLOAD             224
#define DH5_MAX_PAYLOAD             339
#define AUX1_MAX_PAYLOAD            29
#define E2_DH1_MAX_PAYLOAD          54
#define E2_DH3_MAX_PAYLOAD          367
#define E2_DH5_MAX_PAYLOAD          679
#define E3_DH1_MAX_PAYLOAD          83
#define E3_DH3_MAX_PAYLOAD          552
#define E3_DH5_MAX_PAYLOAD          1021

enum E_MasterBlasterFieldID
{
   CR = 0,
   CT,
   LR,
   LT,
   LTM,
   CX,
   TM,
   HM,
   TF,
   RF,
   PT,
   DL,
   PO,
   BA,
   SB,
   GB,
   RX,
   TX,
   EN,
   ST,
   EX,
   EXX,
};

enum E_MasterBlasterTestFlag
{
   MB_NO_TEST = 0,
   MB_RX_TEST,
   MB_TX_TEST,
   MB_CONT_RX_TEST,
   MB_CONT_TX_TEST,
   MB_LE_RX_TEST,
   MB_LE_TX_TEST,
};

enum E_DisableEnable
{
   DISABLE = 0,
   ENABLE = 1,
};

#define MB_MIN_DATALEN			0
#define MB_MAX_DATALEN			1021
#define MB_MIN_FREQUENCY		0
#define MB_MAX_FREQUENCY		79
#define MB_MIN_FREQUENCY_LE		0
#define MB_MAX_FREQUENCY_LE		39
#define MB_MIN_DATALEN_LE		0
#define MB_MAX_DATALEN_LE		37
typedef struct STR_MasterBlasterOption
{
   char *Name;
   int Value;
} tMasterBlasterOption;

typedef struct STR_MasterBlasterField
{
   char *Name;
   char *Alias;
   char *Usage;
   int Default;
   tMasterBlasterOption *Options;
   int (*pFunc)(tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
} tMasterBlasterField;

static tMasterBlasterOption TestModeOption[] =
{
   {"TX_0",                   eBRM_TestMode_TX_0},
   {"TX_1",                   eBRM_TestMode_TX_1},
   {"TX_1010",                eBRM_TestMode_TX_1010},
   {"TX_PRBS",                eBRM_TestMode_TX_PRBS},
   {"TX_11110000",            eBRM_TestMode_TX_11110000},
   {"RX",                     eBRM_TestMode_Rx},
};

static tMasterBlasterOption HopModeOption[] =
{
   {"Disable",                DISABLE},
   {"Enable",                 ENABLE},
};

static tMasterBlasterOption PacketTypeOption[] =
{
   {"DM1",                    TxTest_PktType_DM1},
   {"DM3",                    TxTest_PktType_DM3},
   {"DM5",                    TxTest_PktType_DM5},
   {"DH1",                    TxTest_PktType_DH1},
   {"DH3",                    TxTest_PktType_DH3},
   {"DH5",                    TxTest_PktType_DH5},
   {"2-DH1",                  TxTest_PktType_2DH1},
   {"2-DH3",                  TxTest_PktType_2DH3},
   {"2-DH5",                  TxTest_PktType_2DH5},
   {"3-DH1",                  TxTest_PktType_3DH1},
   {"3-DH3",                  TxTest_PktType_3DH3},
   {"3-DH5",                  TxTest_PktType_3DH5},
};

static int MaxDataLenOption[] =
{
   DM1_MAX_PAYLOAD,
   DM3_MAX_PAYLOAD,
   DM5_MAX_PAYLOAD,
   DH1_MAX_PAYLOAD,
   DH3_MAX_PAYLOAD,
   DH5_MAX_PAYLOAD,
   E2_DH1_MAX_PAYLOAD,
   E2_DH3_MAX_PAYLOAD,
   E2_DH5_MAX_PAYLOAD,
   E3_DH1_MAX_PAYLOAD,
   E3_DH3_MAX_PAYLOAD,
   E3_DH5_MAX_PAYLOAD,
};

enum {
   eBRM_BERMode_ALL = 0, // ALL type
   eBRM_BERMode_ALL_DATA, // All type except dm1,dm3,dm5
   eBRM_BERMode_DM1,
   eBRM_BERMode_DM3,
   eBRM_BERMode_DM5,
   eBRM_BERMode_DH1,
   eBRM_BERMode_DH3,
   eBRM_BERMode_DH5,
   eBRM_BERMode_2DH1,
   eBRM_BERMode_2DH3,
   eBRM_BERMode_2DH5,
   eBRM_BERMode_3DH1,
   eBRM_BERMode_3DH3,
   eBRM_BERMode_3DH5,
};

static tMasterBlasterOption BERPacketTypeOption[] =
{
   {"ALL",                  eBRM_BERMode_ALL},
   {"ALL_DATA",             eBRM_BERMode_ALL_DATA},
   {"DM1",                  eBRM_BERMode_DM1},
   {"DM3",                  eBRM_BERMode_DM3},
   {"DM5",                  eBRM_BERMode_DM5},
   {"DH1",                  eBRM_BERMode_DH1},
   {"DH3",                  eBRM_BERMode_DH3},
   {"DH5",                  eBRM_BERMode_DH5},
   {"2DH1",                 eBRM_BERMode_2DH1},
   {"2DH3",                 eBRM_BERMode_2DH3},
   {"2DH5",                 eBRM_BERMode_2DH5},
   {"3DH1",                 eBRM_BERMode_3DH1},
   {"3DH3",                 eBRM_BERMode_3DH3},
   {"3DH5",                 eBRM_BERMode_3DH5},
};

static tMasterBlasterOption ContTxModeOption[] =
{
   {"Disable",                DISABLE},
   {"Enable",                 ENABLE},
};

static tMasterBlasterOption ContTxTypeOption[] =
{
   {"Raw_1MHz",                  Cont_Tx_Raw_1MHz},
   {"Raw_2MHz",                  Cont_Tx_Raw_2MHz},
   {"Raw_3MHz",                  Cont_Tx_Raw_3MHz},
   {"Regular_BT_Packet_Format",  Cont_Tx_Regular},
   {"CW_Single_Tone",		 CW_Single_Tone},
};

static tMasterBlasterOption ContRxModeOption[] =
{
   {"Disable",                DISABLE},
   {"Enable",                 ENABLE},
};

static tMasterBlasterOption LETxPktPayloadOption[] =
{
   {"Random_9",               0},
   {"11110000",               1},
   {"10101010",               2},
   {"Random_15",              3},
   {"11111111",               4},
   {"00000000",               5},
   {"00001111",               6},
   {"01010101",               7},
};

//----------------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------------

void InitMasterBlaster (tBRM_Control_packet *MasterBlaster, bdaddr_t *BdAddr, UCHAR *SkipRxSlot);

int CheckField (tBRM_Control_packet MasterBlaster, char *FieldAlias);
int GetTestModeOptionIndex (int Mode);
int GetPacketTypeOptionIndex (int PacketType);

void PrintMasterBlasterMenu (tBRM_Control_packet *MasterBlaster);
int SetMasterBlasterTestMode (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterHopMode (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterPacketType (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterTxFreq (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterRxFreq (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterPower (tBRM_Control_packet *MasterBlaster, char *Option);
int SetMasterBlasterDataLen (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterBdAddr (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterContTxMode (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterContTxType (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterNothing (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterContRxMode (tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterLERxMode(tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterLETxMode(tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterLETxPktPayload(tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);
int SetMasterBlasterBERType(tBRM_Control_packet *MasterBlaster, tMasterBlasterOption *Option);

int ToggleOption (int *Value, tMasterBlasterOption *Option, tMasterBlasterOption *OptionArray,
                     int Size, int FieldID, int Step);
int MinMaxOption (int *Value, tMasterBlasterOption *Option, int Min, int Max);
int ToggleMinMaxOption (int *Value, char *Option, int FieldID, int Min, int Max, int Step);

//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

extern tMasterBlasterField MasterBlasterMenu[];
extern tPsSysCfgTransmitPowerControlTable  TpcTable;

#endif // _MASTERBLASTER_H_
