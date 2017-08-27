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
 *  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#ifndef _BTCONFIG_H_
#define _BTCONFIG_H_

#define VERSION		"1.06"
#define PS_ASIC_FILENAME  "/etc/firmware/ar3k/1020200/PS_ASIC.pst"
#define PS_FPGA_FILENAME  "/etc/firmware/ar3k/1020200/PS_FPGA.pst"
#define TESTPATCH_FILENAME  "/etc/firmware/ar3k/1020200/Testpatch.txt"
#define PATCH_FILENAME  "/etc/firmware/ar3k/1020200/RamPatch.txt"

#define TRUE	1
#define FALSE	0	
#define LINE_SIZE_MAX						3000
#define BD_ADDR_SIZE		6
#define BD_ADDR_PSTAG		1
#define WRITE_PATCH		8
#define ENABLE_PATCH		11
#define PS_RESET		2
#define PS_READ			0
#define PS_WRITE		1
#define PS_READ_RAW		3
#define PS_WRITE_RAW		4
#define PS_GET_LENGTH		5
#define PS_SET_ACCESS_MODE	6
#define PS_SET_ACCESS_PRIORITY	7
#define PS_WRITE		1
#define PS_DYNMEM_OVERRIDE	10
#define PS_VERIFY_CRC		9
#define CHANGE_BDADDR		15
#define PS_COMMAND_HEADER	4
#define HCI_EVENT_SIZE		7
#define PS_RETRY_COUNT		3
#define RAM_PS_REGION		(1<<0)
#define RAM_PATCH_REGION	(1<<1)
#define RAM_DYN_MEM_REGION	(1<<2)
#define RAMPS_MAX_PS_DATA_PER_TAG       244
#define RAMPS_MAX_PS_TAGS_PER_FILE      50
#define PSTAG_RF_TEST_BLOCK_START   	(300)
#define PSTAG_SYSTEM_BLOCK_START	(1)
#define BT_SOC_INIT_TOOL_START_MAGIC_WORD 0xB1B1
#define PSTAG_RF_PARAM_TABLE0        (PSTAG_RF_TEST_BLOCK_START+0)
#define PSTAG_SYSCFG_PARAM_TABLE0    (PSTAG_SYSTEM_BLOCK_START+18)
#define PATCH_MAX_LEN                     20000
#define DYN_MEM_MAX_LEN                   40
#define SKIP_BLANKS(str) while (*str == ' ') str++
#define MAX_RADIO_CFG_TABLE_SIZE			1000
#define MAX_BYTE_LENGTH    244
#define DEBUG_EVENT_TYPE_PS		0x02
#define DEBUG_EVENT_TYPE_MEMBLK		0x03
#define HCI_EVENT_HEADER_SIZE		0x03
#define HI_MAGIC_NUMBER	((const unsigned short int) 0xFADE)
#define HI_VERSION	(0x0300)  //Version 3.0
#define EEPROM_CONFIG	0x00020C00
#define FPGA_REGISTER	0x4FFC

// Vendor specific command OCF
#define OCF_PS	0x000B
#define OCF_MEMOP	0x0014
#define OGF_TEST_CMD	0x06
#define OCF_HOST_INTEREST	0x000A
#define OCF_CONT_TX_TESTER	0x0023
#define OCF_TX_TESTER		0x001B
#define OCF_SLEEP_MODE		0x0004
#define OCF_READ_MEMORY		0x0005
#define OCF_WRITE_MEMORY	0x0006
#define OCF_DISABLE_TX		0x002D
#define OCF_TEST_MODE_SEQN_TRACKING	0x0018
#define OCF_READ_VERSION	0x001E
#define OCF_AUDIO_CMD		0x0013
#define OCF_GET_BERTYPE		0x005C
#define OCF_RX_TESTER		0x005B

#define UCHAR unsigned char
#define BOOL unsigned short
#define UINT16 unsigned short int
#define UINT32 unsigned int
#define SINT16 signed short int
#define UINT8 unsigned char
#define SINT8 signed char

typedef struct tPsTagEntry
{
   int   TagId;
   UCHAR   TagLen;
   UCHAR    TagData[RAMPS_MAX_PS_DATA_PER_TAG];
} tPsTagEntry, *tpPsTagEntry;

typedef struct tRamPatch
{
   int   Len;
   UCHAR    Data[PATCH_MAX_LEN];
} tRamPatch, *ptRamPatch;

typedef struct tRamDynMemOverride
{
   int   Len;
   UCHAR    Data[DYN_MEM_MAX_LEN];
} tRamDynMemOverride, *ptRamDynMemOverride;

tPsTagEntry PsTagEntry[RAMPS_MAX_PS_TAGS_PER_FILE];
tRamPatch   RamPatch[50];
tRamDynMemOverride RamDynMemOverride;

enum MB_FILEFORMAT {
	MB_FILEFORMAT_PS,
	MB_FILEFORMAT_PATCH,
	MB_FILEFORMAT_DY,
};
enum RamPsSection
{
   RAM_PS_SECTION,
   RAM_PATCH_SECTION,
   RAM_DYN_MEM_SECTION
};

enum eType {
   eHex,
   edecimal,
};

struct ST_PS_DATA_FORMAT {
   enum eType   eDataType;
   BOOL    bIsArray;
};
#define CONV_DEC_DIGIT_TO_VALUE(c) ((c) - '0')
#define IS_HEX(c)   (IS_BETWEEN((c), '0', '9') || IS_BETWEEN((c), 'a', 'f') || IS_BETWEEN((c), 'A', 'F'))
#define IS_BETWEEN(x, lower, upper) (((lower) <= (x)) && ((x) <= (upper)))
#define IS_DIGIT(c) (IS_BETWEEN((c), '0', '9'))
#define CONV_HEX_DIGIT_TO_VALUE(c) (IS_DIGIT(c) ? ((c) - '0') : (IS_BETWEEN((c), 'A', 'Z') ? ((c) - 'A' + 10) : ((c) - 'a' + 10)))
#define BYTES_OF_PS_DATA_PER_LINE    16
struct ST_READ_STATUS {
	unsigned uTagID;
   	unsigned uSection;
	unsigned uLineCount;
	unsigned uCharCount;
	unsigned uByteCount;
};

//DUT MODE related
#define MC_BCAM_COMPARE_ADDRESS           0x00008080
#define HCI_3_PATCH_SPACE_LENGTH_1            (0x80)
#define HCI_3_PATCH_SPACE_LENGTH_2            (0x279C)
#define MEM_BLK_DATA_MAX                (244)
#define MC_BCAM_VALID_ADDRESS                    0x00008100

//Audio stat

typedef struct tAudio_Stat {
    UINT16      RxSilenceInsert;
    UINT16      RxAirPktDump;
    UINT16      RxCmplt;
    UINT16      TxCmplt;
    UINT16      MaxPLCGenInterval;
    UINT16      RxAirPktStatusGood;
    UINT16      RxAirPktStatusError;
    UINT16      RxAirPktStatusLost;
    UINT16      RxAirPktStatusPartial;
    SINT16      SampleMin;
    SINT16      SampleMax;
    UINT16      SampleCounter;
    UINT16      SampleStatEnable;
} tAudioStat;

//DMA stats

typedef struct tBRM_Stats {
  // DMA Stats
  UINT32 DmaIntrs;

  // Voice Stats
  UINT16 VoiceTxDmaIntrs;
  UINT16 VoiceTxErrorIntrs;
  UINT16 VoiceTxDmaErrorIntrs;
  UINT16 VoiceTxPktAvail;
  UINT16 VoiceTxPktDumped;
  UINT16 VoiceTxDmaSilenceInserts;

  UINT16 VoiceRxDmaIntrs;
  UINT16 VoiceRxErrorIntrs;
  UINT16 VoiceRxGoodPkts;
  UINT16 VoiceRxErrCrc;
  UINT16 VoiceRxErrUnderOverFlow;
  UINT16 VoiceRxPktDumped;

  UINT16 VoiceTxReapOnError;
  UINT16 VoiceRxReapOnError;
  UINT16 VoiceSchedulingError;
  UINT16 SchedOnVoiceError;

  UINT16 Temp1;
  UINT16 Temp2;

  // Control Stats
  UINT16 ErrWrongLlid;
  UINT16 ErrL2CapLen;
  UINT16 ErrUnderOverFlow;
  UINT16 RxBufferDumped;
  UINT16 ErrWrongLmpPktType;
  UINT16 ErrWrongL2CapPktType;
  UINT16 HecFailPkts;
  UINT16 IgnoredPkts;
  UINT16 CrcFailPkts;
  UINT16 HwErrRxOverflow;

  UINT16 CtrlErrNoLmpBufs;

  // ACL Stats
  UINT16 DataTxBuffers;
  UINT16 DataRxBuffers;
  UINT16 DataRxErrCrc;
  UINT16 DataRxPktDumped;
  UINT16 LmpTxBuffers;
  UINT16 LmpRxBuffers;
  UINT16 ForceOverQosJob;

  // Sniff Stats
  UINT16 SniffSchedulingError;
  UINT16 SniffIntervalNoCorr;

  // Test Mode Stats
  UINT16 TestModeDroppedTxPkts;
  UINT16 TestModeDroppedLmps;

  // Error Stats
  UINT16 TimePassedIntrs;
  UINT16 NoCommandIntrs;

} tBRM_Stats;

typedef struct tSYSUTIL_ChipId {
  char *pName;
  UINT32 HwRev;
} tSYSUTIL_ChipId;

typedef struct tSU_RevInfo {
  tSYSUTIL_ChipId *pChipId;
  tSYSUTIL_ChipId *pChipRadioId;
  UINT32 ChipRadioId;
  UINT32 SubRadioId;
  UINT32 RomVersion;
  UINT32 RomBuildNumber;
  UINT32 BuildVersion;
  UINT32 BuildNumber;
  UINT16 RadioFormat;
  UINT16 RadioContent;
  UINT16 SysCfgFormat;
  UINT16 SysCfgContent;
  UINT8 ProductId;
} tSU_RevInfo;

#endif
