#include "bp_defs.h"
#include "boardparms.h"

#if defined(_CFE_)
#if !defined(PINMUXCHECK)
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#endif // !PINMUXCHECK
#define printk  printf
#else
#include "linux/kernel.h"
#endif // _CFE_
#ifdef PRINT_ERRORS
#include "stdio.h"
#endif


/* Externs */
#if defined(CONFIG_BOARDPARMS_VOICE) || defined(_CFE_)
extern bp_elem_t * BpGetVoicePmuxBp( bp_elem_t * pCurrentDataBp );
#endif // CONFIG_BOARDPARMS_VOICE || _CFE_


/* Include all the led id in this list. The cfe/setup initilization code
 * use this list to perform any necessary setup such pinmux selection on these
 * LED gpio pins
 */
static enum bp_id bpLedList[] = {
  bp_usGpioLedAdsl,
  bp_usGpioLedAdslFail,
  bp_usGpioSecLedAdsl,
  bp_usGpioSecLedAdslFail,
  bp_usGpioLedSesWireless,
  bp_usGpioLedWanData,
  bp_usGpioSecLedWanData,
  bp_usGpioLedWanError,
  bp_usGpioLedBlPowerOn,
  bp_usGpioLedBlStop,
  bp_usGpioLedGpon,
  bp_usGpioLedGponFail,
  bp_usGpioLedMoCA,
  bp_usGpioLedMoCAFail,
  bp_usGpioLedEpon,
  bp_usGpioLedEponFail,
  bp_usDuplexLed,
  bp_usLinkLed, // Link/Activity
  bp_usSpeedLed100,
  bp_usSpeedLed1000,
  bp_usGpioLedVoip,
  bp_usGpioVoip1Led,
  bp_usGpioVoip1LedFail,
  bp_usGpioVoip2Led,
  bp_usGpioVoip2LedFail,
  bp_usGpioPotsLed,
  bp_usGpioDectLed,
  bp_usGpioLedOpticalLinkFail,
  bp_usGpioLedOpticalLinkStat,
  bp_usGpioLedLan,
  bp_usGpioLedWL2_4GHz,
  bp_usGpioLedWL5GHz, 
  bp_usGpioLedUSB,
  bp_usGpioLedUSB2,
  bp_usGpioLedSim,
  bp_usGpioLedSim_ITMS,
  bp_usGpioLedReserved,
  bp_usGpioLedPwmReserved,
  bp_usGpioLedAggregateLnk,
  bp_usGpioLedAggregateAct,
  bp_usNetLed0,
  bp_usNetLed1,
  bp_usNetLed2,
  bp_usNetLed3,
#ifdef CONFIG_BP_PHYS_INTF
  bp_usGpioLedWanAct,
  bp_usGpioLedWanErr,
  bp_usGpioLedWanLink,
  bp_usGpioLedWanLinkFail,
#endif
};

#if !defined(_CFE_)
#if defined(CONFIG_NEW_LEDS)

#define STRINGIFY(x) #x
#define MAP(x, y) {x, STRINGIFY(y)}

struct bp_id_e
{
	int id;
	char *bp_name;
};

struct bp_id_e bpLedList_str[] = {
 MAP( bp_usGpioLedAdsl, LedAdsl),
 MAP( bp_usGpioLedAdslFail, AdslFail),
 MAP( bp_usGpioSecLedAdsl, SecLedAdsl),
 MAP( bp_usGpioSecLedAdslFail, SecLedAdslFail),
 MAP( bp_usGpioLedSesWireless, WPSWireless),
 MAP( bp_usGpioLedWanData, WanData),
 MAP( bp_usGpioSecLedWanData, SecLedWanData),
 MAP( bp_usGpioLedWanError, WanError),
 MAP( bp_usGpioLedBlPowerOn, BlPowerOn),
 MAP( bp_usGpioLedBlStop, BlStop),
 MAP( bp_usGpioLedGpon, Gpon),
 MAP( bp_usGpioLedGponFail, GponFail),
 MAP( bp_usGpioLedMoCA, MoCA),
 MAP( bp_usGpioLedMoCAFail, MoCAFail),
 MAP( bp_usGpioLedEpon, Epon),
 MAP( bp_usGpioLedEponFail, EponFail),
 MAP( bp_usDuplexLed, DuplexLed),
 MAP( bp_usGpioLedVoip, Voip),
 MAP( bp_usGpioVoip1Led, Voip1Led),
 MAP( bp_usGpioVoip1LedFail, Voip1LedFail),
 MAP( bp_usGpioVoip2Led, Voip2Led),
 MAP( bp_usGpioVoip2LedFail, Voip2LedFail),
 MAP( bp_usGpioPotsLed, PotsLed),
 MAP( bp_usGpioDectLed, DectLed),
 MAP( bp_usGpioLedOpticalLinkFail, OpticalLinkFail),
 MAP( bp_usGpioLedLan, Lan),
 MAP( bp_usGpioLedWL2_4GHz, WL2_4GHz),
 MAP( bp_usGpioLedWL5GHz, WL5GHz),
 MAP( bp_usGpioLedUSB, USB),
 MAP( bp_usGpioLedSim, Sim),
 MAP( bp_usGpioLedSim_ITMS, Sim_ITMS),
 MAP( bp_usGpioLedReserved, Reserved),
 MAP( bp_usGpioLedPwmReserved, PwmReserved),
 MAP( bp_usNetLed0, NetLed0),
 MAP( bp_usNetLed1, NetLed1),
 MAP( bp_usNetLed2, NetLed2),
 MAP( bp_usNetLed3, NetLed3),
 {-1, NULL}
};
#endif
#endif

/* Include all the gpio id in this list. The cfe/setup initilization code
 * use this list to perform any necessary setup such pinmux selection on these
 * gpio pins
 */
static enum bp_id bpGpioList[] = {
  bp_usGpioWirelessPowerDown,
  bp_usGpioPassDyingGasp,
  bp_usGpioExtAFEReset,
  bp_usGpioExtAFELDPwr,
  bp_usGpioExtAFELDMode,
  bp_usGpioIntAFELDPwr,
  bp_usGpioIntAFELDMode,
  bp_usGpioAFELDRelay,
  bp_usGpioAFEVR5P3PwrEn,
  bp_usGpioUart2Sdin,
  bp_usGpioUart2Sdout,
  bp_usGpioLaserDis,
  bp_usGpioLaserTxPwrEn,
  bp_usGpioSpiSlaveReset,
  bp_usGpioSpiSlaveBootMode,
  bp_usGpioLaserReset,
  bp_usGpio_Intr,
  bp_usGpioEponOpticalSD,
  bp_usGpioPLCPwrEn,
  bp_usGpioPLCReset,
  bp_usGpioPhyReset,
  bp_usGpioBoardReset,
  bp_usGpioUsb0,  
  bp_usGpioUsb1, 
  bp_usGpioPonTxEn,
  bp_usGpioPonRxEn,
  bp_usGpioTsyncPonUnstable,
  bp_usGpioPmdReset,
  bp_usGpioPonReset,
  bp_usGpio1ppsStable,
  bp_usGpioLteReset,
  bp_usGpioStrapTxEn,
  bp_usGpioFxsFxoRst1,   
  bp_usGpioFxsFxoRst2,      
  bp_usGpioFxsFxoRst3,   
  bp_usGpioFxsFxoRst4,   
  bp_usGpioDectRst,           
  bp_usGpioVoipRelayCtrl1,    
  bp_usGpioVoipRelayCtrl2,    
  bp_usGpioLe9540Reset,  
  bp_usGpioSfpDetect,
  bp_usGpioWanSignalDetected,
  bp_usGpioBitbangI2cScl,
  bp_usGpioBitbangI2cSda,
  bp_usGpioNfcWake,
  bp_usGpioNfcPower,
  bp_usGpioBtWake,
  bp_usGpioBtReset,
  bp_usGpioAFELDPwrBoost,
  bp_usGpioTxDis0,
  bp_usGpioTxDis1,
  bp_usGpioTxDis2,
  bp_usGpioTxDis3,
  bp_usGpioOpticalModuleFixup,
  bp_usGpioPonMuxOe,
  bp_usGpioPonMux0,
  bp_usGpioPonMux1,
  bp_usGpioI2CMux,
#ifdef CONFIG_BP_PHYS_INTF
  bp_usGpioAFELDMode,
  bp_usGpioAFELDPwr,
  bp_usGpioAFELDClk,
  bp_usGpioAFELDData,
  bp_usGpioAFEReset,
  bp_usGpioSfpModDetect,
#endif
  bp_usGpioPwrSync,
  bp_usGpioReserved
};

/* Include all the gpio and led id in this list for EPON MAC controlled
 * gpio pins on 6828 device.
 */
static enum bp_id bpEponGpioList[] = {
  bp_usGpioLedEpon,
  bp_usGpioLedEponFail,
  bp_usGpioLedWanData,
  bp_usGpioI2cScl,
  bp_usGpioI2cSda,
  bp_usGpioEponOpticalSD
};

/* Include all the external interrupt id in this list. The cfe/setup initilization code
 * use this list to perform any necessary setup such pinmux selection on these
 * external interrupt pins
 */
static enum bp_id bpExtIntrList[] = {
  bp_usExtIntrResetToDefault,
  bp_usExtIntrResetToDefault2,
  bp_usExtIntrSesBtnWireless,
  bp_usExtIntrNfc,
  bp_usExtIntrMocaHostIntr,
  bp_usExtIntrMocaSBIntr0,
  bp_usExtIntrMocaSBIntr1,
  bp_usExtIntrMocaSBIntr2,
  bp_usExtIntrMocaSBIntr3,
  bp_usExtIntrMocaSBIntr4,
  bp_usExtIntrMocaSBIntrAll,
  bp_usExtIntrPLCStandBy,
  bp_usExtIntrPmdAlarm,
  bp_usExtIntrTrplxrTxFail,
  bp_usExtIntrTrplxrSd,
  bp_usExtIntrWifiOnOff,
  bp_usExtIntrLTE,
  bp_usExtIntrSDCardDetect,
  bp_usButtonExtIntr,
  bp_usExtIntrOpticalModulePresence,
};

/* Global copy of Ethernet MAC Info - to be used to return to drivers/application without reparsing the board parameters */
static PETHERNET_MAC_INFO pEnetMacInfo = NULL;
static ETHERNET_MAC_INFO EnetMacInfoGbl[BP_MAX_ENET_MACS];

#if defined(CONFIG_BCM96858) || defined(_BCM96858_)
#define PHYID_LIST  {bp_ulPhyId0, bp_ulPhyId1, bp_ulPhyId2, bp_ulPhyId3, bp_ulPhyId4, bp_ulPhyId5, bp_ulPhyId6, bp_ulPhyId7, bp_ulPhyId8, bp_last}
#else
#define PHYID_LIST  {bp_ulPhyId0, bp_ulPhyId1, bp_ulPhyId2, bp_ulPhyId3, bp_ulPhyId4, bp_ulPhyId5, bp_ulPhyId6, bp_ulPhyId7, bp_last}
#endif

#if !defined(_BCM960333_) && !defined(CONFIG_BCM960333) && !defined(_BCM963268_) && !defined(CONFIG_BCM963268) && !defined(_BCM96838_) && !defined(CONFIG_BCM96838) && !defined(_BCM947189_) && !defined(CONFIG_BCM947189)
typedef struct bp_pinmux_defs {
  enum bp_id id;
  short port;
  short func; /* PIN or ANY */
  unsigned int mux_info;
} bp_pinmux_defs_t;

typedef struct bp_pinmux_fn_defs {
  unsigned int function;
  unsigned int mux_info;
} bp_pinmux_fn_defs_t;


static enum bp_id bpDslCtlGpioList[] = {
  // These parameters are controlled by the VDSL PHY.  If used, the corresponding GPIO will be Pinmuxed as "bp_ReservedDslCtl"
  bp_usGpioExtAFELDPwr,
  bp_usGpioExtAFELDMode,
  bp_usGpioIntAFELDPwr,
  bp_usGpioIntAFELDMode,
  bp_usGpioExtAFELDData,
  bp_usGpioExtAFELDClk,
  bp_usGpioIntAFELDData,
  bp_usGpioIntAFELDClk,
#ifdef CONFIG_BP_PHYS_INTF
  bp_usGpioAFELDMode,
  bp_usGpioAFELDPwr,
  bp_usGpioAFELDClk,
  bp_usGpioAFELDData,
#endif
};

static bp_pinmux_defs_t *g_pinmux_defs = (void *)0;

#endif 

#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_DECT, 137 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DECT, 138 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DECT, 139 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DECT, 140 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DECT, 141 | BP_PINMUX_VAL_1 },
  // { BP_PINMUX_FNTYPE_SATA, 112 | BP_PINMUX_VAL_2 }, // ONLY FOR SATA_MDIO
  // { BP_PINMUX_FNTYPE_SATA, 113 | BP_PINMUX_VAL_2 }, // ONLY FOR SATA_MDIO
  { BP_PINMUX_FNTYPE_xMII | 1, BP_PINMUX_VAL_DUMMY }, // Dummy entry for internal interface
  { BP_PINMUX_FNTYPE_xMII | 9, BP_PINMUX_VAL_DUMMY }, 
  { BP_PINMUX_FNTYPE_xMII | 12, 60 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 61 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 62 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 63 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 64 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 68 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 69 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 70 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 71 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 72 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 73 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 74 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 75 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 76 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 77 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 78 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 79 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 81 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 82 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 83 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 84 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 85 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 86 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 87 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 7, 88 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 89 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 90 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 91 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 92 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 93 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 94 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 95 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 96 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 97 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 98 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 7, 99 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 11, 100 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 101 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 102 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 103 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 104 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 105 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 106 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 107 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 108 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 109 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 110 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 111 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 46 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 47 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 48 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 49 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 50 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 51 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_HS_SPI, 123 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 124 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 125 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 126 | BP_PINMUX_VAL_0 }, //SS0, this always used for spi flash
  { BP_PINMUX_FNTYPE_PCM, 56 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 57 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 58 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 59 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_I2S, 29 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_I2S, 30 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_I2S, 31 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_DEFAULT, 69 | BP_PINMUX_VAL_5 }, // MII/RGMII_TXCLK default GPIO to reduce EMI
  { BP_PINMUX_FNTYPE_DEFAULT, 82 | BP_PINMUX_VAL_5 }, // RGMII1_TXCLK default GPIO to reduce EMI
  { BP_PINMUX_FNTYPE_DEFAULT, 94 | BP_PINMUX_VAL_5 }, // RGMII2_TXCLK default GPIO to reduce EMI
  { BP_PINMUX_FNTYPE_DEFAULT, 106 | BP_PINMUX_VAL_5 }, // RGMII3_TXCLK default GPIO to reduce EMI
  { BP_PINMUX_FNTYPE_DEFAULT, 132 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 133 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 134 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 135 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_IRQ | 5,  37 | BP_PINMUX_VAL_1 }, // Nfc i2c irq
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usSpeedLed100, 0, 0, 0 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // SPD0/port 0 is led0  pinmux 4
  { bp_usSpeedLed100, 0, 100, 100 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},  // LED0 is also on gpio100
  { bp_usSpeedLed1000, 0, 1, 1 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // SPD1/port 0 is led1  pinmux 4
  { bp_usSpeedLed1000, 0, 101, 101 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)}, 
  { bp_usSpeedLed100, 1, 2, 2 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // SPD0/port 1 is led2 pinmux 4
  { bp_usSpeedLed100, 1, 102, 102 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)}, 
  { bp_usSpeedLed1000, 1, 3, 3 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // SPD1/port 1 is led3 pinmux 4
  { bp_usSpeedLed1000, 1, 103, 103 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usSpeedLed100, 2, 4, 4 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 2, 104, 104 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)}, 
  { bp_usSpeedLed1000, 2, 5, 5 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 2, 105, 105 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)}, 
  { bp_usSpeedLed100, 3, 6, 6 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 3, 106, 106 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)}, 
  { bp_usSpeedLed1000, 3, 7, 7 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 3, 107, 107 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)}, 
  { bp_usSpeedLed100, 4, 8, 8 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 4, 108, 108 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)}, 
  { bp_usSpeedLed1000, 4, 9, 9 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 4, 109, 109 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)}, 
  { bp_usSpeedLed100, 0, 108, 108 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},  
  { bp_usSpeedLed1000, 0, 109, 109 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)}, 
  { bp_usSerialLedData, -1, 0, 0 | BP_PINMUX_VAL_1}, 
  { bp_usSerialLedClk, -1, 1, 1 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 2, 2 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedData, -1, 29, 29 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedClk, -1, 30, 30 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 31, 31 | BP_PINMUX_VAL_1 }, 
  { bp_usGpioUart2Cts, -1, 3, 3 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Rts, -1, 4, 4 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Cts, -1, 20, 20 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Rts, -1, 21, 21 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 5, 5 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 6, 6 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 22, 22 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout, -1, 23, 23 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 27, 27 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdout, -1, 28, 28 | BP_PINMUX_VAL_3},
  { bp_usLinkLed, 0, 10, 10|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // Link/Activity
  { bp_usLinkLed, 0, 110, 110|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)}, 
  { bp_usLinkLed, 1, 11, 11|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 1, 111, 111|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)}, 
  { bp_usLinkLed, 2, 12, 12|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 3, 13, 13|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 4, 14, 14|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // if GPHY4 on swicth port 4
  { bp_usLinkLed, 0, 22, 22|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // wan act on runner port 0
  { bp_usLinkLed, 0, 14, 14|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED |  BP_PINMUX_OPTLED_NUM(22) }, // RGMII wan act on runner port 0
  { bp_usSpeedLed100, 0, 18, 18|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 0, 19, 19|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 0, 20, 20|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },  // wan spd0 on runner port 0
  { bp_usSpeedLed1000, 0, 21, 21|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // wan spd1 on runner port 0
  { bp_usSpeedLed1000, 2, 23, 23|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 3, 24, 24|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 3, 25, 25|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 4, 26, 26|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 4, 27, 27|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 0, 8, 8|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(20)}, // wan spd0 on runner port 0
  { bp_usSpeedLed1000, 0, 9, 9|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) }, // wan spd1 on runner port 0
  { bp_usSpeedLed100, 0, 26, 26|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(20)}, // wan spd0 on runner port 0
  { bp_usSpeedLed1000, 0, 27, 27|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) }, // wan spd1 on runner port 0
  { bp_usSpeedLed100, 0, 108, 108|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(20)}, // wan spd0 on runner port 0
  { bp_usSpeedLed1000, 0, 109, 109|BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) }, // wan spd1 on runner port 0
  { bp_usLinkLed, 0, 28, 28|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, // Link/Activity
  { bp_usLinkLed, 1, 29, 29|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 2, 30, 30|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 3, 31, 31|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, 
  { bp_usGpioLedAggregateLnk, -1, 23, 23|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //Aggregate link LED
  { bp_usGpioLedAggregateAct, -1, 24, 24|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //Aggregate activity LED
  { bp_usGpioLedWanData, -1, 15, 15|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED},
  { bp_usGpioSecLedWanData, -1, 19, 19|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED}, // in B0
  { bp_ReservedDslCtl, -1, 52, 52|BP_PINMUX_VAL_1 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl, -1, 53, 53|BP_PINMUX_VAL_1 | BP_VDSLCTL_1 },
  { bp_ReservedDslCtl, -1, 54, 54|BP_PINMUX_VAL_1 | BP_VDSLCTL_3 },
  { bp_ReservedDslCtl, -1, 55, 55|BP_PINMUX_VAL_1 | BP_VDSLCTL_2 },
  { bp_ReservedDslCtl, -1, 9, 9|BP_PINMUX_VAL_2 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl, -1, 10, 10|BP_PINMUX_VAL_2 | BP_VDSLCTL_5 },
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_5 }, // ALL SW GPIOs use pinmux 5
  { bp_ReservedAnyLed, -1, 100, 100 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_ReservedAnyLed, -1, 101, 101 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_ReservedAnyLed, -1, 102, 102 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_ReservedAnyLed, -1, 103, 103 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_ReservedAnyLed, -1, 104, 104 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_ReservedAnyLed, -1, 105, 105 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_ReservedAnyLed, -1, 106, 106 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_ReservedAnyLed, -1, 107, 107 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_ReservedAnyLed, -1, 108, 108 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_ReservedAnyLed, -1, 109, 109 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_ReservedAnyLed, -1, 110, 110 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_ReservedAnyLed, -1, 111, 111 | BP_PINMUX_VAL_4 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_ReservedAnyLed, -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW LEDs use pinmux 4
  { bp_usGpioI2cSda, -1, 24, 24 | BP_PINMUX_VAL_3 },
  { bp_usGpioI2cScl, -1, 25, 25 | BP_PINMUX_VAL_3 },
  { bp_usGpioI2cSda, -1, 52, 52 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cScl, -1, 53, 53 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cSda, -1, 117, 117 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cScl, -1, 119, 119 | BP_PINMUX_VAL_2 },
  { bp_usSgmiiDetect, -1, 28, 28 | BP_PINMUX_VAL_2 },
  { bp_usSgmiiDetect, -1, 36, 36 | BP_PINMUX_VAL_2 },
  { bp_usGpioLedPwmReserved, -1, 16, 16 | BP_PINMUX_VAL_4 | BP_PINMUX_PWMLED }, 
  { bp_usGpioLedPwmReserved, -1, 17, 17 |  BP_PINMUX_VAL_4 | BP_PINMUX_PWMLED }, 
  { bp_usSpiSlaveSelectNum, 1, 127, 127 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 2, 10, 10 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 25, 25 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 9, 9 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 26, 26 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 8, 8 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 27, 27 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 5, 7, 7 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 5, 28, 28 | BP_PINMUX_VAL_1},
  { bp_usVregSync, -1, 18, 18 | BP_PINMUX_VAL_1 },
  { bp_usVregSync, -1, 37, 37 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrFlt0, -1, 132, 132 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0, -1, 133, 133 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 134, 134 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1, -1, 135, 135 | BP_PINMUX_VAL_1 },
  { bp_usTsync1pps, -1, 4, 4 | BP_PINMUX_VAL_2},
  { bp_usTsync1pps, -1, 13, 13 | BP_PINMUX_VAL_1},
  { bp_usTsync1pps, -1, 26, 26 | BP_PINMUX_VAL_2},
  { bp_usTsync8khz, -1, 7, 7 | BP_PINMUX_VAL_2},
  { bp_usTsync8khz, -1, 24, 24 | BP_PINMUX_VAL_1},
  { bp_usTsync8khz, -1, 27, 27 | BP_PINMUX_VAL_2},
  { bp_usNtrRefIn, -1, 3, 3 | BP_PINMUX_VAL_2},
  { bp_usNtrRefIn, -1, 12, 12 | BP_PINMUX_VAL_1},
  { bp_usNtrRefIn, -1, 25, 25 | BP_PINMUX_VAL_2},
  { bp_usNtrRefIn, -1, 26, 26 | BP_PINMUX_VAL_3},
  { bp_last, -1, -1,  0 }, 
  //  PWM ???
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

#endif

#if  defined(_BCM963381_) || defined(CONFIG_BCM963381)

static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_PCM, 70 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 71 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 72 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 73 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 49 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 50 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 51 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 52 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 53 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 54 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 55 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 56 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 86 | BP_PINMUX_VAL_1 }, // RGMII_MDC
  { BP_PINMUX_FNTYPE_DEFAULT, 87 | BP_PINMUX_VAL_1 }, // RGMII_MDIO
  { BP_PINMUX_FNTYPE_xMII | 4, BP_PINMUX_VAL_DUMMY }, // Dummy entry 
  { BP_PINMUX_FNTYPE_IRQ | 0, 63 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 1, 64 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 2, 61 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 3, 62 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 4, 54 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 5, 44 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 6, 46 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_IRQ | 7, 94 | BP_PINMUX_VAL_4 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 90 | BP_PINMUX_VAL_3 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 91 | BP_PINMUX_VAL_3 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 92 | BP_PINMUX_VAL_3 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 93 | BP_PINMUX_VAL_3 }, //SS0, this always used for spi flash
  { BP_PINMUX_FNTYPE_DEFAULT, 96 |  BP_PINMUX_VAL_1}, 
  { BP_PINMUX_FNTYPE_DEFAULT, 97 |  BP_PINMUX_VAL_1},
};


static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usSerialLedData, -1, 17, 60 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 16, 59 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 24, 67 | BP_PINMUX_VAL_1 }, 
  { bp_usGpioUart2Sdin, -1, 23, 66 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 22, 65 | BP_PINMUX_VAL_1 },
  // LED2
  { bp_usSpeedLed100, 0, 2, 45|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 0, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2) }, 
  // LED16
  { bp_usSpeedLed100, 0, 16, 59|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 0, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) }, 
  // LED3
  { bp_usSpeedLed100, 1, 3, 46|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3) }, 
  // LED17
  { bp_usSpeedLed100, 1, 17, 60|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED },
  { bp_usSpeedLed100, 1, 43, 86 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(17) }, 
  // LED4
  { bp_usSpeedLed100, 2, 4, 47|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 2, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4) }, 
  // LED22
  { bp_usSpeedLed100, 2, 22, 65|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  // LED6
  { bp_usSpeedLed100, 3, 6, 49|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 3, 42, 85 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6) }, 
  // LED24
  { bp_usSpeedLed100, 3, 24, 67|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 

  // LED5
  { bp_usLinkLed, 0, 5, 48|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 0, 35, 78 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5) }, 
  // LED27
  { bp_usLinkLed, 0, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27) }, 
  // LED8
  { bp_usLinkLed, 1, 8, 51|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 1, 33, 76 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8) }, 
  // LED28
  { bp_usLinkLed, 1, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(28) }, 
  // LED9
  { bp_usLinkLed, 2, 9, 52|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 2, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) }, 
  // LED29
  { bp_usLinkLed, 2, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(29) }, 
  // LED10
  { bp_usLinkLed, 3, 10, 53|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 3, 41, 84 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) }, 
  // LED31
  { bp_usLinkLed, 3, 31, 74|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 3, 49, 94 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(31) }, 
  // LED13
  { bp_usGpioLedWanData, -1, 13, 56|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED},
  { bp_usGpioLedWanData, -1, 40, 83 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13) }, 
  // LED19
  { bp_usGpioLedWanData, -1, 19, 62|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED},
  { bp_ReservedAnyGpio, -1, 0, 43 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, // GPIO0 is on pin 43 Pinmux 5
  { bp_ReservedAnyGpio, -1, 1, 44 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 2, 45 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 3, 46 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 4, 47 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 5, 48 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 6, 49 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 7, 50 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 8, 51 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 9, 52 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 10, 53 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 11, 54 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 12, 55 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 13, 56 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 14, 57 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 15, 58 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 16, 59 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 17, 60 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 18, 61 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 19, 62 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 20, 63 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 21, 64 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 22, 65 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 23, 66 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 24, 67 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 25, 68 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 26, 69 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 27, 70 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 28, 71 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 29, 72 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 30, 73 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 31, 74 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 32, 75 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 33, 76 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 34, 77 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 35, 78 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 36, 79 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 37, 80 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 38, 81 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 39, 82 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 40, 83 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 41, 84 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 42, 85 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 43, 86 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 44, 87 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 45, 90 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 46, 91 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 47, 92 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 48, 93 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 49, 94 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 50, 96 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 51, 97 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyLed, -1, 0, 43 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, // LED0 is on pin 43 Pinmux 2
  { bp_ReservedAnyLed, -1, 1, 44 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 2, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 3, 46 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 4, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 5, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 6, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 7, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 8, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 9, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 10, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 11, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 12, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 13, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 14, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 15, 58 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 16, 59 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 17, 60 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 18, 61 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 19, 62 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 20, 63 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 21, 64 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 22, 65 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 23, 66 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 24, 67 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 25, 68 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 26, 69 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 27, 70 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) }, // OPT_LED0 is on GPIO27 and pin 70 Pinmux 2
  { bp_ReservedAnyLed, -1, 28, 71 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) }, 
  { bp_ReservedAnyLed, -1, 29, 72 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) }, 
  { bp_ReservedAnyLed, -1, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) }, 
  { bp_ReservedAnyLed, -1, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) }, 
  { bp_ReservedAnyLed, -1, 33, 76 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) }, 
  { bp_ReservedAnyLed, -1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) }, 
  { bp_ReservedAnyLed, -1, 35, 78 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) }, 
  { bp_ReservedAnyLed, -1, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) }, 
  { bp_ReservedAnyLed, -1, 37, 80 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) }, 
  { bp_ReservedAnyLed, -1, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) }, 
  { bp_ReservedAnyLed, -1, 39, 82 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) }, 
  { bp_ReservedAnyLed, -1, 40, 83 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) }, 
  { bp_ReservedAnyLed, -1, 41, 84 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) }, 
  { bp_ReservedAnyLed, -1, 42, 85 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) }, 
  { bp_ReservedAnyLed, -1, 43, 86 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) }, 
  { bp_ReservedAnyLed, -1, 44, 87 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) }, 
  { bp_ReservedAnyLed, -1, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(27) }, 
  { bp_ReservedAnyLed, -1, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(28) }, 
  { bp_ReservedAnyLed, -1, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(29) }, 
  { bp_ReservedAnyLed, -1, 48, 93 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(30) }, 
  { bp_ReservedAnyLed, -1, 49, 94 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(31) }, 
  { bp_ReservedDslCtl, -1, 15, 58|BP_PINMUX_VAL_1 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl, -1, 26, 69|BP_PINMUX_VAL_1 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl, -1, 25, 68|BP_PINMUX_VAL_1 | BP_VDSLCTL_5 },
  { bp_usSpiSlaveSelectNum, 1, 49, 94 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 2, 18, 61 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 3, 19, 62 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 4, 6, 49 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 5, 7, 50 | BP_PINMUX_VAL_3},
  { bp_usUsbPwrFlt0, -1, 50, 96 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0, -1, 51, 97 | BP_PINMUX_VAL_1 },
  { bp_last, -1, -1,  0 }, 
};

// 63381A0 table -- alternate table 1
static bp_pinmux_defs_t g_pinmux_defs_1[] = {
  { bp_usSerialLedData, -1, 17, 60 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 16, 59 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 24, 67 | BP_PINMUX_VAL_1 }, 
  { bp_usGpioUart2Sdin, -1, 23, 66 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 22, 65 | BP_PINMUX_VAL_1 },
  // LED16 -- same
  { bp_usSpeedLed100, 0, 16, 59 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, // Link is prog0
  { bp_usSpeedLed100, 0, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) }, 
  // LED15 is port 1 Link
  { bp_usSpeedLed100, 1, 15, 58 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 1, 37, 80 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(15) }, 
  // LED29 is also port 1 Link
  { bp_usSpeedLed100, 1, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(29) }, 
  // LED9 is also port 2 Link
  { bp_usSpeedLed100, 2, 9, 52 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 2, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) },  // LED28 is port 2 Link
  { bp_usSpeedLed100, 2, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(28) }, 
 
  // LED26 is port 3 link
  { bp_usSpeedLed100, 3, 26, 69 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 3, 50, 96 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(26)}, 

  // LED27 is port 0 spd
  { bp_usLinkLed, 0, 4, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 0, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4) }, 
  { bp_usLinkLed, 0, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27) }, 

  // LED3 is port 1 spd
  { bp_usLinkLed, 1, 3, 46 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3) }, 
  // LED24 is also port 1 spd
  { bp_usLinkLed, 1, 24, 67 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 

  // LED23 and 2 are port 2 spd
  { bp_usLinkLed, 2, 2, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 2, 23, 66 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 2, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(3)}, 
  { bp_usLinkLed, 2, 51, 97 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED |BP_PINMUX_OPTLED_NUM(23)}, 

  // LED0 and 22 are port 3 spd
  { bp_usLinkLed, 3, 0, 43 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, // LED0 is on pin 43 Pinmux 2
  { bp_usLinkLed, 3, 22, 65 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED }, 
  { bp_usLinkLed, 3, 27, 70 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},

  // // LED19
  { bp_usGpioLedWanData, -1, 19, 62 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED},
  { bp_usGpioLedWanData, -1, 29, 72 |BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},

  { bp_ReservedAnyGpio, -1, 0, 43 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, // GPIO0 is on pin 43 Pinmux 5
  { bp_ReservedAnyGpio, -1, 1, 44 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 2, 45 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 3, 46 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 4, 47 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 5, 48 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 6, 49 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 7, 50 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 8, 51 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 9, 52 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 10, 53 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 11, 54 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 12, 55 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 13, 56 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 14, 57 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 15, 58 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 16, 59 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 17, 60 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 18, 61 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 19, 62 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 20, 63 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 21, 64 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 22, 65 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 23, 66 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 24, 67 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 25, 68 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyGpio, -1, 26, 69 | BP_PINMUX_VAL_5 | BP_PINMUX_SWGPIO }, 
  { bp_ReservedAnyLed, -1, 0, 43 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, // LED0 is on pin 43 Pinmux 2
  { bp_ReservedAnyLed, -1, 1, 44 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 2, 45 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 3, 46 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 4, 47 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 5, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 6, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 7, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 8, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 9, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 10, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 11, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 12, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 13, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 14, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 15, 58 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 16, 59 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 17, 60 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 18, 61 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 19, 62 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 20, 63 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 21, 64 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 22, 65 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 23, 66 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 24, 67 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 25, 68 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 26, 69 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED }, 
  { bp_ReservedAnyLed, -1, 27, 70 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) }, // OPT_LED0 is on GPIO27 and pin 70 Pinmux 2
  { bp_ReservedAnyLed, -1, 28, 71 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) }, 
  { bp_ReservedAnyLed, -1, 29, 72 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) }, 
  { bp_ReservedAnyLed, -1, 30, 73 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) }, 
  { bp_ReservedAnyLed, -1, 31, 74 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) }, 
  { bp_ReservedAnyLed, -1, 33, 76 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) }, 
  { bp_ReservedAnyLed, -1, 34, 77 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) }, 
  { bp_ReservedAnyLed, -1, 35, 78 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) }, 
  { bp_ReservedAnyLed, -1, 36, 79 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) }, 
  { bp_ReservedAnyLed, -1, 37, 80 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) }, 
  { bp_ReservedAnyLed, -1, 38, 81 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) }, 
  { bp_ReservedAnyLed, -1, 39, 82 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) }, 
  { bp_ReservedAnyLed, -1, 40, 83 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) }, 
  { bp_ReservedAnyLed, -1, 41, 84 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) }, 
  { bp_ReservedAnyLed, -1, 42, 85 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) }, 
  { bp_ReservedAnyLed, -1, 43, 86 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) }, 
  { bp_ReservedAnyLed, -1, 44, 87 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(18) }, 
  { bp_ReservedAnyLed, -1, 45, 90 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(27) }, 
  { bp_ReservedAnyLed, -1, 46, 91 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(28) }, 
  { bp_ReservedAnyLed, -1, 47, 92 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(29) }, 
  { bp_ReservedAnyLed, -1, 48, 93 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(30) }, 
  { bp_ReservedAnyLed, -1, 49, 94 | BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(31) }, 
  { bp_ReservedDslCtl, -1, 15, 58|BP_PINMUX_VAL_1 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl, -1, 26, 69|BP_PINMUX_VAL_1 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl, -1, 25, 68|BP_PINMUX_VAL_1 | BP_VDSLCTL_5 },
  { bp_usSpiSlaveSelectNum, 1, 49, 94 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 2, 18, 61 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 3, 19, 62 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 4, 6, 49 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 5, 7, 50 | BP_PINMUX_VAL_3},
  { bp_usUsbPwrFlt0, -1, 50, 96 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0, -1, 51, 97 | BP_PINMUX_VAL_1 },
  { bp_last, -1, -1,  0 }, 
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0, g_pinmux_defs_1 } ;

#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_xMII | 1, BP_PINMUX_VAL_DUMMY }, // Dummy entry for internal interface
  { BP_PINMUX_FNTYPE_xMII | 0, BP_PINMUX_VAL_DUMMY }, // Dummy entry for port 0
  { BP_PINMUX_FNTYPE_xMII | 9, BP_PINMUX_VAL_DUMMY }, // Dummy entry for port 9
  { BP_PINMUX_FNTYPE_xMII | 11, 48 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_xMII | 11, 49 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_xMII | 11, 68 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 69 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 70 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 71 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 72 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 73 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 74 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 75 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 76 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 77 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 78 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 79 | BP_PINMUX_VAL_0 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 11, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 81 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 82 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 83 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 35 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 36 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_EMMC, 46 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_EMMC, 47 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 27 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 28 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 29 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_I2S, 30 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 63 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 64 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 66 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_DEFAULT, 67 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_PCM, 14 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 15 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 16 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 17 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_HS_SPI, 52 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 54 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 53 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 56 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 23 | BP_PINMUX_VAL_2 },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usSpeedLed100, 0, 0, 0 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, // SPD0/port 0 is led0  pinmux 4
  { bp_usSpeedLed1000, 0, 1, 1 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, // SPD1/port 0 is led1  pinmux 4
  { bp_usSpeedLed100, 1, 2, 2 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 1, 3, 3 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 2, 4, 4 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 2, 5, 5 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 3, 6, 6 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 3, 7, 7 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 4, 8, 8 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed1000, 4, 9, 9 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, 
  { bp_usSpeedLed100, 3, 10, 10 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, // Port 3 on Runner is the WAN port
  { bp_usSpeedLed100, 3, 8, 8 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) }, 
  { bp_usSpeedLed1000, 3, 11, 11 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, // DONT confuse runner port 3 (WAN) with Switch port 0 (LAN)
  { bp_usSpeedLed1000, 3, 9, 9 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11) }, 
  { bp_usSpeedLed100, 3, 22, 22 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, // Port 3 on Runner is the WAN port
  { bp_usSpeedLed1000, 3, 23, 23 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, // DONT confuse runner port 3 (WAN) with Switch port 0 (LAN)
  { bp_usLinkLed, 0, 16, 16|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 0, 26, 26|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 1, 17, 17|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 1, 27, 27|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 18, 18|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 2, 28, 28|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 19, 19|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 29, 29|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 4, 20, 20|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 4, 30, 30|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED },
  { bp_usLinkLed, 3, 21, 21|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //Port 3 on Runner is the WAN port
  { bp_usLinkLed, 3, 31, 31|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //Port 3 on Runner is the WAN port
  { bp_usLinkLed, 3, 20, 20|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) }, //Port 3 on Runner is the WAN port
  { bp_usLinkLed, 3, 30, 30|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(31) }, //Port 3 on Runner is the WAN port
  { bp_usGpioLedAggregateLnk, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //Aggregate link LED
  { bp_usGpioLedAggregateLnk, -1, 0, 0|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12) }, 
  { bp_usGpioLedAggregateAct, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //Aggregate activity LED
  { bp_usGpioLedAggregateAct, -1, 1, 1|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13) }, 
  { bp_usGpioLedAggregateLnk, -1, 24, 24|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //Aggregate link LED
  { bp_usGpioLedAggregateAct, -1, 25, 25|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED }, //Aggregate activity LED
  { bp_usMiiMdc,  -1, 48, 48 | BP_PINMUX_VAL_0},
  { bp_usMiiMdio, -1, 49, 49 | BP_PINMUX_VAL_0},
  { bp_usGpioLedPwmReserved, -1, 14, 14 | BP_PINMUX_VAL_3 | BP_PINMUX_PWMLED }, 
  { bp_usGpioLedPwmReserved, -1, 15, 15 | BP_PINMUX_VAL_3 | BP_PINMUX_PWMLED }, 
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW GPIOs use pinmux 4
  { bp_ReservedAnyLed, -1, -1,  BP_PINMUX_VAL_3 }, // ALL SW LEDs use pinmux 3
  { bp_usSerialLedData, -1, 0, 0 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 1, 1 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 2, 2 | BP_PINMUX_VAL_1 }, 
  { bp_usGpioUart2Cts, -1, 10, 10 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Rts, -1, 11, 11 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdin, -1, 12, 12 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdout, -1, 13, 13 | BP_PINMUX_VAL_0},
  { bp_usUsbPwrFlt0, -1, 63, 63 | BP_PINMUX_VAL_0 },
  { bp_usUsbPwrOn0, -1,  64, 64 | BP_PINMUX_VAL_0 },
  { bp_usUsbPwrFlt1, -1, 66, 66 | BP_PINMUX_VAL_0 },
  { bp_usUsbPwrOn1, -1,  67, 67 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda, -1, 18, 18 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, -1, 19, 19 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda, -1, 22, 22 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, -1, 23, 23 | BP_PINMUX_VAL_0 },
  { bp_usSpiSlaveSelectNum, 1, 56, 56 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 2, 23, 23 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 3, 22, 22 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 4, 21, 21 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 5, 20, 20 | BP_PINMUX_VAL_2},
  { bp_last, -1, -1,  0 }, 
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;
#endif

#if  defined(_BCM96848_) || defined(CONFIG_BCM96848)

static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_PCM, 0 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 1 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 2 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 3 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 19 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 20 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 21 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 22 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 23 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 24 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 25 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 26 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 27 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 28 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_IRQ | 0, 49 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 1, 50 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 2, 67 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 3, 68 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 4, 71 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 5, 72 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 6, 33 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_IRQ | 7, 34 | BP_PINMUX_VAL_0 },
  { BP_PINMUX_FNTYPE_HS_SPI, 57 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 58 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 59 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 60 | BP_PINMUX_VAL_1 }, //SS0, this always used for spi flash
  { BP_PINMUX_FNTYPE_xMII | 0, BP_PINMUX_VAL_DUMMY }, // Dummy entry
  { BP_PINMUX_FNTYPE_xMII | 1, BP_PINMUX_VAL_DUMMY }, // Dummy entry
  { BP_PINMUX_FNTYPE_xMII | 2, BP_PINMUX_VAL_DUMMY }, // Dummy entry
  { BP_PINMUX_FNTYPE_xMII | 3, BP_PINMUX_VAL_DUMMY }, // Dummy entry
  { BP_PINMUX_FNTYPE_xMII | 5, BP_PINMUX_VAL_DUMMY }, // Dummy entry
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usTsync1pps, -1, 6, 6 | BP_PINMUX_VAL_4 },
  { bp_usTsync1pps, -1, 52, 52 | BP_PINMUX_VAL_7 },
  { bp_usSerialLedClk, -1, 10, 10 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedData, -1, 11, 11 | BP_PINMUX_VAL_1},
  { bp_usSerialLedClk, -1, 33, 33 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedData, -1, 34, 34 | BP_PINMUX_VAL_3},
  { bp_usSerialLedClk, -1, 65, 65 | BP_PINMUX_VAL_4 },
  { bp_usSerialLedData, -1, 67, 67 | BP_PINMUX_VAL_4},
  { bp_usSerialLedMask, -1, 17, 17 | BP_PINMUX_VAL_1 },
  { bp_usGpioUart2Cts, -1, 13, 13 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Rts, -1, 16, 16 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 14, 14 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout, -1, 15, 15 | BP_PINMUX_VAL_1},
  { bp_usLinkLed, 1, 9, 9|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)}, // Link/Activity
  { bp_usLinkLed, 0, 5, 5|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usLinkLed, 3, 52, 52|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usLinkLed, 2, 54, 54|BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usLinkLed, 1, 10, 10|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usLinkLed, 1, 53, 53|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usLinkLed, 1, 11, 11|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usLinkLed, 1, 54, 54|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usLinkLed, 0, 33, 33|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usLinkLed, 0, 51, 51|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usLinkLed, 0, 34, 34|BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usLinkLed, 0, 52, 52|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usLinkLed, 3, 17, 17|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usLinkLed, 3, 18, 18|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usLinkLed, 2, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usLinkLed, 2, 14, 14|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usLinkLed, 2, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usLinkLed, 2, 15, 15|BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
//  { bp_usGpioLedUSB, -1, 5, 5|BP_PINMUX_VAL_0 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
//  { bp_usGpioLedOpticalLinkStat, -1, 7, 7|BP_PINMUX_VAL_0 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usGpioI2cSda, -1, 64, 64 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl, -1, 63, 63 | BP_PINMUX_VAL_0 },
  { bp_usSpiSlaveSelectNum, 1, 61, 61 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 3, 8, 8 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 9, 9 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 6, 12, 12 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 7, 67, 67 | BP_PINMUX_VAL_1},
  { bp_usSimDat,       -1, 6, 6 | BP_PINMUX_VAL_3},
  { bp_usSimClk,       -1, 7, 7 | BP_PINMUX_VAL_3},
  { bp_usSimPresence,  -1, 8, 8 | BP_PINMUX_VAL_3},
  { bp_usPmdMACEwakeEn, -1, 10, 10 | BP_PINMUX_VAL_3},
  { bp_usSimVccEn,     -1, 14, 14 | BP_PINMUX_VAL_3},
  { bp_usSimVccVolSel, -1, 15, 15 | BP_PINMUX_VAL_3},
  { bp_usSimRst,       -1, 16, 16 | BP_PINMUX_VAL_3},
  { bp_usWanNco10MClk, -1, 16, 16 | BP_PINMUX_VAL_7},
  { bp_usSimVppEn,     -1, 17, 17 | BP_PINMUX_VAL_3},
  { bp_usSimDat,       -1, 43, 43 | BP_PINMUX_VAL_3},
  { bp_usSimClk,       -1, 44, 44 | BP_PINMUX_VAL_3},
  { bp_usSimPresence,  -1, 45, 45 | BP_PINMUX_VAL_3},
  { bp_usSimVccEn,     -1, 35, 35 | BP_PINMUX_VAL_3},
  { bp_usSimVccVolSel, -1, 36, 36 | BP_PINMUX_VAL_3},
  { bp_usSimRst,       -1, 37, 37 | BP_PINMUX_VAL_3},
  { bp_usSimVppEn,     -1, 39, 39 | BP_PINMUX_VAL_3},
  { bp_usMiiMdc,       -1, 47, 47 | BP_PINMUX_VAL_0},
  { bp_usMiiMdio,      -1, 48, 48 | BP_PINMUX_VAL_0},
  { bp_usRogueOnuEn,   -1, 51, 51 | BP_PINMUX_VAL_7},
  { bp_usTrxSignalDetect, -1, 53, 53 | BP_PINMUX_VAL_7},
  { bp_usTxLaserOnOutN, -1, 62, 62 | BP_PINMUX_VAL_1},
  { bp_usMs1588TodAlarm, -1, 65, 65 | BP_PINMUX_VAL_7},
  { bp_usSgmiiDetect,  -1, 66, 66 | BP_PINMUX_VAL_7},
  { bp_usWanNcoProgMClk, -1, 67, 67 | BP_PINMUX_VAL_3},
  { bp_usProbeClk,     -1, 74, 74 | BP_PINMUX_VAL_4},
  { bp_ReservedAnyLed, -1, 33, 33|BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(0) }, // to let LED 0 come out on GPIO33 as a SWLED
  { bp_ReservedAnyLed, -1, 34, 34|BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 10, 10|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 11, 11|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 14, 14|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 15, 15|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 17, 17|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 18, 18|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 5,   5|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 6,   6|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 7,   7|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 8,   8|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 9,   9|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 69, 69|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 53, 53|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 54, 54|BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_5 }, // ALL SW GPIOs use pinmux 5
  { bp_last, -1, -1,  0 },
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 35 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 36 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 39 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 33 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 34 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 35 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 102 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 103 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 104 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_LPORT, 98 | BP_PINMUX_VAL_7 },
  { BP_PINMUX_FNTYPE_LPORT, 99 | BP_PINMUX_VAL_7 },
  { BP_PINMUX_FNTYPE_xMII | 4, 86 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 87 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 88 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 89 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 90 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 91 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 92 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 93 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 94 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 95 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 96 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 97 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 62 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 63 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 64 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 68 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 69 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 70 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 71 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 72 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 5, 73 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 74 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 75 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 76 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 77 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 78 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 79 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 81 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 82 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 83 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 84 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 6, 85 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 0 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 1 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 2 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 3 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_DEFAULT, 113 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 114 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 115 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_DEFAULT, 116 | BP_PINMUX_VAL_1 },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_5 }, // ALL SW GPIOs use pinmux 5
  { bp_usNetLed0, 0,  0,  0 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usNetLed1, 0,  1,  1 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usNetLed2, 0,  2,  2 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usNetLed3, 0, 21, 21 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21)},
  { bp_usNetLed0, 1,  3,  3 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 1,  4,  4 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed2, 1,  5,  5 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed3, 1, 22, 22 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(22)},
  { bp_usNetLed0, 2,  6,  6 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed1, 2,  7,  7 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed2, 2,  8,  8 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed3, 2, 23, 23 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(23)},
  { bp_usNetLed0, 3,  9,  9 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 10, 10 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed2, 3, 11, 11 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed3, 3, 28, 28 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(24)},
  { bp_usNetLed0, 4, 12, 12 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_usNetLed1, 4, 13, 13 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13)},
  { bp_usNetLed2, 4, 14, 14 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(14)},
  { bp_usNetLed3, 4, 29, 29 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(25)},
  { bp_usNetLed0, 5, 15, 15 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(15)},
  { bp_usNetLed1, 5, 16, 16 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16)},
  { bp_usNetLed2, 5, 17, 17 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(17)},
  { bp_usNetLed3, 5, 30, 30 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(26)},
  { bp_usNetLed0, 6, 18, 18 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(18)},
  { bp_usNetLed1, 6, 19, 19 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(19)},
  { bp_usNetLed2, 6, 20, 20 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(20)},
  { bp_usNetLed3, 6, 31, 31 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27)},
  { bp_usNetLed0, 0, 62, 62 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usNetLed1, 0, 63, 63 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usNetLed2, 0, 64, 64 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usNetLed3, 0, 32, 32 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21)},
  { bp_usNetLed0, 1, 65, 65 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 1, 66, 66 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed2, 1, 67, 67 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed3, 1, 29, 29 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(22)},
  { bp_usNetLed0, 2, 68, 68 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed1, 2, 69, 69 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed2, 2, 70, 70 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed3, 2, 30, 30 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(23)},
  { bp_usNetLed0, 3, 71, 71 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 72, 72 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed2, 3, 73, 73 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed3, 3, 31, 31 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(24)},
  { bp_usNetLed0, 4,  3,  3 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_usNetLed1, 4,  4,  4 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13)},
  { bp_usNetLed2, 4,  5,  5 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(14)},
  { bp_usNetLed3, 4, 22, 22 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(25)},
  { bp_usNetLed0, 5,  6,  6 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(15)},
  { bp_usNetLed1, 5,  7,  7 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16)},
  { bp_usNetLed2, 5,  8,  8 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(17)},
  { bp_usNetLed3, 5, 23, 23 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(26)},
  { bp_usNetLed0, 6,  9,  9 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(18)},
  { bp_usNetLed1, 6, 10, 10 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(19)},
  { bp_usNetLed2, 6, 11, 11 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(20)},
  { bp_usNetLed3, 6, 28, 28 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27)},
  { bp_usUart1Sdin,     -1, 27, 27 | BP_PINMUX_VAL_1},
  { bp_usUart1Sdout,    -1, 28, 28 | BP_PINMUX_VAL_1},
  { bp_usUart1Sdin,     -1, 55, 55 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 56, 56 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdin, -1, 0,  0  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout,-1, 1,  1  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Cts,  -1, 2,  2  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Rts,  -1, 3,  3  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 16, 16 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout,-1, 17, 17 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Cts,  -1, 18, 18 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Rts,  -1, 19, 19 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 27, 27 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdout,-1, 28, 28 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Cts,  -1, 25, 25 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Rts,  -1, 26, 26 | BP_PINMUX_VAL_3},
  { bp_usPcmSdin,       -1, 0,  0  | BP_PINMUX_VAL_2},
  { bp_usPcmSdout,      -1, 1,  1  | BP_PINMUX_VAL_2},
  { bp_usPcmClk,        -1, 2,  2  | BP_PINMUX_VAL_2},
  { bp_usPcmFs,         -1, 3,  3  | BP_PINMUX_VAL_2},
  { bp_usPcmSdin,       -1, 58, 58 | BP_PINMUX_VAL_1},
  { bp_usPcmSdout,      -1, 59, 59 | BP_PINMUX_VAL_1},
  { bp_usPcmClk,        -1, 60, 60 | BP_PINMUX_VAL_1},
  { bp_usPcmFs,         -1, 61, 61 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 0, 105, 105 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 1, 106, 106 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2,  0, 0  | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 2, 14, 14 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 51, 51 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 3,  1, 1  | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 3, 13, 13 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 52, 52 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 4,  2, 2  | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 4, 12, 12 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 53, 53 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 5,  3, 3  | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 5, 11, 11 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 5, 54, 54 | BP_PINMUX_VAL_2},
  { bp_usSerialLedData, -1,  4,  4 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedClk,  -1,  5,  5 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1,  6,  6 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedData, -1, 29, 29 | BP_PINMUX_VAL_2 },
  { bp_usSerialLedClk,  -1, 30, 30 | BP_PINMUX_VAL_2 },
  { bp_usSerialLedMask, -1, 31, 31 | BP_PINMUX_VAL_2 },
  { bp_usTsync1pps,     -1, 11, 11 | BP_PINMUX_VAL_2 },
  { bp_usTsync1pps,     -1, 32, 32 | BP_PINMUX_VAL_1 },
  { bp_usSimDat,        -1, 16, 16 | BP_PINMUX_VAL_1 },
  { bp_usSimClk,        -1, 17, 17 | BP_PINMUX_VAL_1 },
  { bp_usSimPresence,   -1, 18, 18 | BP_PINMUX_VAL_1 },
  { bp_usSimVccEn,      -1, 19, 19 | BP_PINMUX_VAL_1 },
  { bp_usSimVccVolSel,  -1, 20, 20 | BP_PINMUX_VAL_1 },
  { bp_usSimRst,        -1, 21, 21 | BP_PINMUX_VAL_1 },
  { bp_usSimVppEn,      -1, 22, 22 | BP_PINMUX_VAL_1 },
  { bp_usSimDat,        -1, 90, 90 | BP_PINMUX_VAL_3 },
  { bp_usSimClk,        -1, 91, 91 | BP_PINMUX_VAL_3 },
  { bp_usSimPresence,   -1, 92, 92 | BP_PINMUX_VAL_3 },
  { bp_usSimVccEn,      -1, 86, 86 | BP_PINMUX_VAL_3 },
  { bp_usSimVccVolSel,  -1, 87, 87 | BP_PINMUX_VAL_3 },
  { bp_usSimRst,        -1, 88, 88 | BP_PINMUX_VAL_3 },
  { bp_usSimVppEn,      -1, 89, 89 | BP_PINMUX_VAL_3 },
  { bp_usGpioI2cSda,    -1, 25, 25 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl,    -1, 26, 26 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cSda,    -1, 39, 39 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cScl,    -1, 40, 40 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2c2Sda,    -1, 24, 24 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2c2Scl,    -1, 23, 23 | BP_PINMUX_VAL_1 },
  { bp_usI2sSdata,      -1, 31, 31 | BP_PINMUX_VAL_1 },
  { bp_usI2sSclk,       -1, 30, 30 | BP_PINMUX_VAL_1 },
  { bp_usI2sLrck,       -1, 29, 29 | BP_PINMUX_VAL_1 },
  { bp_usI2sSdata,      -1, 59, 59 | BP_PINMUX_VAL_2 },
  { bp_usI2sSclk,       -1, 60, 60 | BP_PINMUX_VAL_2 },
  { bp_usI2sLrck,       -1, 61, 61 | BP_PINMUX_VAL_2 },
  { bp_usRogueOnuEn,    -1, 32, 32 | BP_PINMUX_VAL_3 },
  { bp_usRogueOnuEn,    -1, 56, 56 | BP_PINMUX_VAL_2 },
  { bp_usWanNcoProgMClk, -1, 55, 55 | BP_PINMUX_VAL_2 },
  { bp_usWanNco10MClk,  -1, 58, 58 | BP_PINMUX_VAL_3 },
  { bp_usPmdMACEwakeEn,  -1, 55, 55 | BP_PINMUX_VAL_6 },
  { bp_usPmdMACEwakeEn,  -1, 56, 56 | BP_PINMUX_VAL_6 },
  { bp_usGpioWanSignalDetected, -1, 49, 49 | BP_PINMUX_VAL_6 },
  { bp_usUsbPwrOn0,  -1, 113, 113 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt0, -1, 114, 114 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1,  -1, 115, 115 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 116, 116 | BP_PINMUX_VAL_1 },
  { bp_usGpioTxDis0,  -1, 52, 52 | BP_PINMUX_VAL_5 },
  { bp_usSFPSerdesSIGDET0, -1, 57, 57|BP_PINMUX_VAL_6},
  { bp_usSFPSerdesSIGDET1, -1, 58, 58|BP_PINMUX_VAL_6},
  { bp_usSFPSerdesSIGDET2, -1, 59, 59|BP_PINMUX_VAL_6},
  { bp_usSFPSerdesSIGDET3, -1, 60, 60|BP_PINMUX_VAL_6},
  { bp_usSFPSerdesMODDEF0, -1, 24, 24|BP_PINMUX_VAL_7},
  { bp_usSFPSerdesMODDEF1, -1, 25, 25|BP_PINMUX_VAL_7},
  { bp_usSFPSerdesMODDEF2, -1, 26, 26|BP_PINMUX_VAL_7},
  { bp_usSFPSerdesMODDEF3, -1, 27, 27|BP_PINMUX_VAL_7},
  { bp_ReservedAnyLed, -1, 0,  0 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(0) }, // to let LED 0 come out on GPIO0 as a SWLED
  { bp_ReservedAnyLed, -1, 1,  1 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 2,  2 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 3,  3 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 4,  4 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 5,  5 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 6,  6 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 7,  7 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 8,  8 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 9,  9 |BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 10, 10|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 11, 11|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 12, 12|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 13, 13|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 14, 14|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 15, 15|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 16, 16|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 17, 17|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 18, 18|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(18) },
  { bp_ReservedAnyLed, -1, 19, 19|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(19) },
  { bp_ReservedAnyLed, -1, 20, 20|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(20) },
  { bp_ReservedAnyLed, -1, 21, 21|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(21) },
  { bp_ReservedAnyLed, -1, 22, 22|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(22) },
  { bp_ReservedAnyLed, -1, 23, 23|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(23) },
  { bp_ReservedAnyLed, -1, 28, 28|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(24) },
  { bp_ReservedAnyLed, -1, 29, 29|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(25) },
  { bp_ReservedAnyLed, -1, 30, 30|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(26) },
  { bp_ReservedAnyLed, -1, 31, 31|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(27) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(28) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(29) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(30) },
  { bp_ReservedAnyLed, -1, 53, 53|BP_PINMUX_VAL_4 | BP_PINMUX_OPTLED_NUM(31) },
  { bp_ReservedAnyLed, -1, 62, 62 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(0) }, // to let LED 0 come out on GPIO62 as a SWLED
  { bp_ReservedAnyLed, -1, 63, 63 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 64, 64 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 65, 65 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 66, 66 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 67, 67 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 68, 68 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 69, 69 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 70, 70 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 71, 71 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 72, 72 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 73, 73 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 3,  3  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 4,  4  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 5,  5  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 6,  6  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 7,  7  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 8,  8  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 9,  9  |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(18) },
  { bp_ReservedAnyLed, -1, 10, 10 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(19) },
  { bp_ReservedAnyLed, -1, 11, 11 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(20) },
  { bp_ReservedAnyLed, -1, 32, 32 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(21) },
  { bp_ReservedAnyLed, -1, 29, 29 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(22) },
  { bp_ReservedAnyLed, -1, 30, 30 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(23) },
  { bp_ReservedAnyLed, -1, 31, 31 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(24) },
  { bp_ReservedAnyLed, -1, 22, 22 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(25) },
  { bp_ReservedAnyLed, -1, 23, 23 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(26) },
  { bp_ReservedAnyLed, -1, 28, 28 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(27) },
  { bp_ReservedAnyLed, -1, 20, 20 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(28) },
  { bp_ReservedAnyLed, -1, 21, 21 |BP_PINMUX_VAL_6 | BP_PINMUX_OPTLED_NUM(29) },
  { bp_last, -1, -1,  0 },
};

  static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;
#endif

#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 35 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 36 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 75 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 49 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 50 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 51 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 52 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 53 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 54 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 55 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 56 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 57 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 58 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 59 | BP_PINMUX_VAL_1 },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW GPIOs use pinmux 4
  { bp_usNetLed0, 0, 48, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 0, 49, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed0, 1, 50, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed1, 1, 51, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed0, 2, 52, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 2, 53, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed0, 3, 54, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 55, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed0, 4, 56, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed1, 4, 57, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_usNetLed0, 0, 11, 11 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 0, 12, 12 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed0, 1, 13, 13 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed1, 1, 14, 14 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed0, 2, 15, 15 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 2, 16, 16 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed0, 3, 17, 17 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 18, 18 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed0, 4,  0,  0 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed1, 4,  1,  1 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_ReservedAnyLed, -1, 26, 26|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) }, // to let LED 0 come out on GPIO26 as a SWLED
  { bp_ReservedAnyLed, -1, 27, 27|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 28, 28|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 48, 48|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 49, 49|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 53, 53|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 54, 54|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 55, 55|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 56, 56|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 57, 57|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 58, 58|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 59, 59|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1,  9,  9|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 82, 82|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 83, 83|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 11, 11|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 14, 14|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 15, 15|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 16, 16|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 17, 17|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 18, 18|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1,  0,  0|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1,  1,  1|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1,  2,  2|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 56, 56|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 57, 57|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 58, 58|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 59, 59|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_usUart1Sdin,     -1, 24, 24 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 25, 25 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin,     -1, 35, 35 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 36, 36 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdin, -1, 15, 15  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout,-1, 16, 16  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Cts,  -1, 17, 17  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Rts,  -1, 18, 18  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 35, 35 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout,-1, 36, 36 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Cts,  -1, 4,  4  | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Rts,  -1, 5,  5  | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 82, 82 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdout,-1, 83, 83 | BP_PINMUX_VAL_3},
  { bp_usSpiSlaveSelectNum, 0, 68, 68 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 1, 81, 81 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 11, 11 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 2, 25, 25 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 3, 12, 12 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 3, 26, 26 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 4, 13, 14 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 4, 27, 27 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 5, 14, 14 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 5, 28, 28 | BP_PINMUX_VAL_0},
  { bp_usSerialLedData, -1, 26, 26 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedClk,  -1, 27, 27 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1, 28, 28 | BP_PINMUX_VAL_1 },
  { bp_usTsync1pps,     -1,  9,  9 | BP_PINMUX_VAL_3 },
  { bp_usTsync1pps,     -1, 24, 24 | BP_PINMUX_VAL_2 },
  { bp_usSimDat,        -1, 52, 52 | BP_PINMUX_VAL_0 },
  { bp_usSimClk,        -1, 53, 53 | BP_PINMUX_VAL_0 },
  { bp_usSimPresence,   -1, 54, 54 | BP_PINMUX_VAL_0 },
  { bp_usSimVccEn,      -1, 48, 48 | BP_PINMUX_VAL_0 },
  { bp_usSimVccVolSel,  -1, 49, 49 | BP_PINMUX_VAL_0 },
  { bp_usSimRst,        -1, 50, 50 | BP_PINMUX_VAL_0 },
  { bp_usSimVppEn,      -1, 51, 51 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda,    -1, 19, 19 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl,    -1, 20, 20 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cSda,    -1, 83, 83 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl,    -1, 82, 82 | BP_PINMUX_VAL_1 },
  { bp_usI2sSdata,      -1, 0, 0 | BP_PINMUX_VAL_1 },
  { bp_usI2sSclk,       -1, 1, 1 | BP_PINMUX_VAL_1 },
  { bp_usI2sLrck,       -1, 2, 2 | BP_PINMUX_VAL_1 },
  { bp_usRogueOnuEn,    -1, 10, 10 | BP_PINMUX_VAL_2 },
  { bp_usRogueOnuEn,    -1, 62, 62 | BP_PINMUX_VAL_0 },
  { bp_usPmdMACEwakeEn, -1, 24, 24 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0,  -1, 77, 77 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt0, -1, 76, 76 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1,  -1, 79, 79 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 78, 78 | BP_PINMUX_VAL_1 },
  { bp_usMiiMdc,     -1, 60, 60 | BP_PINMUX_VAL_3 },
  { bp_usMiiMdio,    -1, 61, 61 | BP_PINMUX_VAL_3 },
  { bp_usPonLbe, -1, 22, 22 | BP_PINMUX_VAL_1},
  { bp_last, -1, -1,  0 },
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;
#endif

#if defined(_BCM96846_) || defined(CONFIG_BCM96846)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_NAND, 26 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 27 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 28 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 35 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 36 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM,  0  | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM,  1  | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM,  2  | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM,  3  | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 62 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 63 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 64 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 20 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 21 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 42 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 43 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 44 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 45 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 46 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 47 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 48 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 49 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 50 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 51 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 52 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
  { BP_PINMUX_FNTYPE_xMII | 4, 53 | BP_PINMUX_VAL_1 | BP_PINMUX_PADCTL },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_ReservedAnyGpio, -1, -1,      BP_PINMUX_VAL_4 }, // ALL SW GPIOs use pinmux 4
  { bp_usSimVccEn,      -1, 42, 42 | BP_PINMUX_VAL_0 },
  { bp_usSimVccVolSel,  -1, 43, 43 | BP_PINMUX_VAL_0 },
  { bp_usSimRst,        -1, 44, 44 | BP_PINMUX_VAL_0 },
  { bp_usSimVppEn,      -1, 45, 45 | BP_PINMUX_VAL_0 },
  { bp_usSimDat,        -1, 46, 46 | BP_PINMUX_VAL_0 },
  { bp_usSimClk,        -1, 47, 47 | BP_PINMUX_VAL_0 },
  { bp_usSimPresence,   -1, 48, 48 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cScl,    -1, 68, 68 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda,    -1, 69, 69 | BP_PINMUX_VAL_0 },
  { bp_usWanEarlyTxEn,  -1, 4,  4  | BP_PINMUX_VAL_1 },
  { bp_usWanEarlyTxEn,  -1, 10, 10  | BP_PINMUX_VAL_3 },
  { bp_usGpioUart2Cts,  -1, 13, 13  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1, 14, 14  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdout,-1, 15, 15  | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Rts,  -1, 16, 16  | BP_PINMUX_VAL_1},
  { bp_usTsync1pps,     -1, 40, 40  | BP_PINMUX_VAL_1 },
  { bp_usTsync1pps,     -1, 2 , 2   | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrFlt0, -1, 74, 74 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0,  -1, 75, 75 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 76, 76 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1,  -1, 77, 77 | BP_PINMUX_VAL_1 },
  { bp_usRogueOnuEn,    -1, 14, 14 | BP_PINMUX_VAL_3 },
  { bp_usRogueOnuEn,    -1, 56, 56 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedClk,  -1, 18, 18 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedData, -1, 23, 23 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedMask, -1, 25, 25 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedClk,  -1, 43, 43 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedMask, -1, 44, 44 | BP_PINMUX_VAL_3 },
  { bp_usSerialLedData, -1, 45, 45 | BP_PINMUX_VAL_3 },
  { bp_usSpiSlaveSelectNum, 0, 8 , 8  | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 0, 52, 52 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 0, 65, 65 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 1, 53, 53 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 1, 66, 66 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 1, 73, 73 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 5,  5  | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 2, 9,  9  | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 7,  7  | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 12, 12 | BP_PINMUX_VAL_1},
  { bp_usMiiMdc,     -1, 54, 54 | BP_PINMUX_VAL_0 },
  { bp_usMiiMdio,    -1, 55, 55 | BP_PINMUX_VAL_0 }, 
  { bp_usNetLed0,     0, 18, 18 | BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usNetLed0,     1, 19, 19 | BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usNetLed0,     2, 22, 22 | BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usNetLed0,     3, 24, 24 | BP_PINMUX_VAL_1 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed0,     0, 52, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0)},
  { bp_usNetLed0,     1, 53, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1)},
  { bp_usNetLed0,     2, 49, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2)},
  { bp_usNetLed0,     3, 44, 44 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usPonLbe, -1, 67, 67 | BP_PINMUX_VAL_1},
  { bp_ReservedAnyLed, -1, 52, 52 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 53, 53 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 49, 49 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 44, 44 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 46, 46 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 43, 43 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 47, 47 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 45, 45 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 5,  5  |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 18, 18 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 50, 50 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 51, 51 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 56, 56 |BP_PINMUX_VAL_2 | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 18, 18 |BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 19, 19 |BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 22, 22 |BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 24, 24 |BP_PINMUX_VAL_1 | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 9,  9  |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 10, 10 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 11, 11 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 12, 12 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 13, 13 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 16, 16 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 17, 17 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 20, 20 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 21, 21 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 74, 74 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 56, 56 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 57, 57 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 58, 58 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 59, 59 |BP_PINMUX_VAL_0 | BP_PINMUX_OPTLED_NUM(17) },
  { bp_last, -1, -1,  0 },
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;
#endif

#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_NAND, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 32 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 33 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 34 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 37 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 38 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 39 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 35 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 36 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 40 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 41 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 29 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 30 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_EMMC, 31 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_HS_SPI, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 46 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 47 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 75 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 48 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 49 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 50 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 51 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 52 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 53 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 54 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 55 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 56 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 57 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 58 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 4, 59 | BP_PINMUX_VAL_1 },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW GPIOs use pinmux 4
  { bp_usNetLed0, 0, 48, 48 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 0, 49, 49 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed0, 1, 50, 50 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed1, 1, 51, 51 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed0, 2, 52, 52 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 2, 53, 53 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed0, 3, 54, 54 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 55, 55 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed0, 4, 56, 56 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed1, 4, 57, 57 | BP_PINMUX_VAL_2 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_usNetLed0, 0, 11, 11 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3)},
  { bp_usNetLed1, 0, 12, 12 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4)},
  { bp_usNetLed0, 1, 13, 13 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5)},
  { bp_usNetLed1, 1, 14, 14 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6)},
  { bp_usNetLed0, 2, 15, 15 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7)},
  { bp_usNetLed1, 2, 16, 16 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8)},
  { bp_usNetLed0, 3, 17, 17 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9)},
  { bp_usNetLed1, 3, 18, 18 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10)},
  { bp_usNetLed0, 4,  0,  0 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11)},
  { bp_usNetLed1, 4,  1,  1 | BP_PINMUX_VAL_3 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12)},
  { bp_ReservedAnyLed, -1, 26, 26|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) }, // to let LED 0 come out on GPIO26 as a SWLED
  { bp_ReservedAnyLed, -1, 27, 27|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 28, 28|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 48, 48|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 49, 49|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 53, 53|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 54, 54|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 55, 55|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1, 56, 56|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1, 57, 57|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1, 58, 58|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 59, 59|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1,  9,  9|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 82, 82|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 83, 83|BP_PINMUX_VAL_2 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_ReservedAnyLed, -1, 50, 50|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(0) },
  { bp_ReservedAnyLed, -1, 51, 51|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_ReservedAnyLed, -1, 52, 52|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(2) },
  { bp_ReservedAnyLed, -1, 11, 11|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_ReservedAnyLed, -1, 12, 12|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(4) },
  { bp_ReservedAnyLed, -1, 13, 13|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_ReservedAnyLed, -1, 14, 14|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(6) },
  { bp_ReservedAnyLed, -1, 15, 15|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_ReservedAnyLed, -1, 16, 16|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(8) },
  { bp_ReservedAnyLed, -1, 17, 17|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_ReservedAnyLed, -1, 18, 18|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(10) },
  { bp_ReservedAnyLed, -1,  0,  0|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_ReservedAnyLed, -1,  1,  1|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_ReservedAnyLed, -1,  2,  2|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(13) },
  { bp_ReservedAnyLed, -1, 56, 56|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(14) },
  { bp_ReservedAnyLed, -1, 57, 57|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(15) },
  { bp_ReservedAnyLed, -1, 58, 58|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(16) },
  { bp_ReservedAnyLed, -1, 59, 59|BP_PINMUX_VAL_3 | BP_PINMUX_SWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_usUart1Sdin,     -1, 24, 24 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 25, 25 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin,     -1, 35, 35 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 36, 36 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin,     -1, 55, 55 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 54, 54 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdin,     -1,  3,  3 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1,  6,  6 | BP_PINMUX_VAL_3},
  { bp_usUart1Sdout,    -1, 23, 23 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Sdin, -1,  2,  2 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdout,-1, 25, 25 | BP_PINMUX_VAL_1},
  { bp_usGpioUart2Cts,  -1,  0,  0 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Rts,  -1,  1,  1 | BP_PINMUX_VAL_0},
  { bp_usGpioUart2Sdin, -1, 35, 35 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout,-1, 36, 36 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Cts,  -1, 4,  4  | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Rts,  -1, 5,  5  | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1, 82, 82 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdout,-1, 83, 83 | BP_PINMUX_VAL_3},
  { bp_usGpioUart2Sdin, -1,  7,  7 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdout,-1,  6,  6 | BP_PINMUX_VAL_2},
  { bp_usGpioUart2Sdin, -1,  3,  3 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 0, 68, 68 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 1, 81, 81 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 11, 11 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 2, 25, 25 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 3, 12, 12 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 3, 26, 26 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 4, 13, 14 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 4, 27, 27 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 5, 14, 14 | BP_PINMUX_VAL_2},
  { bp_usSpiSlaveSelectNum, 5, 28, 28 | BP_PINMUX_VAL_0},
  { bp_usSerialLedData, -1, 26, 26 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedClk,  -1, 27, 27 | BP_PINMUX_VAL_1 },
  { bp_usSerialLedMask, -1, 28, 28 | BP_PINMUX_VAL_1 },
  { bp_usTsync1pps,     -1,  9,  9 | BP_PINMUX_VAL_3 },
  { bp_usTsync1pps,     -1, 24, 24 | BP_PINMUX_VAL_2 },
  { bp_usSimDat,        -1, 52, 52 | BP_PINMUX_VAL_0 },
  { bp_usSimClk,        -1, 53, 53 | BP_PINMUX_VAL_0 },
  { bp_usSimPresence,   -1, 54, 54 | BP_PINMUX_VAL_0 },
  { bp_usSimVccEn,      -1, 48, 48 | BP_PINMUX_VAL_0 },
  { bp_usSimVccVolSel,  -1, 49, 49 | BP_PINMUX_VAL_0 },
  { bp_usSimRst,        -1, 50, 50 | BP_PINMUX_VAL_0 },
  { bp_usSimVppEn,      -1, 51, 51 | BP_PINMUX_VAL_0 },
  { bp_usGpioI2cSda,    -1, 19, 19 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl,    -1, 20, 20 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cSda,    -1, 83, 83 | BP_PINMUX_VAL_1 },
  { bp_usGpioI2cScl,    -1, 82, 82 | BP_PINMUX_VAL_1 },
  { bp_usI2sSdata,      -1, 0, 0 | BP_PINMUX_VAL_1 },
  { bp_usI2sSclk,       -1, 1, 1 | BP_PINMUX_VAL_1 },
  { bp_usI2sLrck,       -1, 2, 2 | BP_PINMUX_VAL_1 },
  { bp_usRogueOnuEn,    -1, 10, 10 | BP_PINMUX_VAL_2 },
  { bp_usRogueOnuEn,    -1, 62, 62 | BP_PINMUX_VAL_0 },
  { bp_usPmdMACEwakeEn, -1, 24, 24 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0,  -1, 77, 77 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt0, -1, 76, 76 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1,  -1, 79, 79 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 78, 78 | BP_PINMUX_VAL_1 },
  { bp_usMiiMdc,     -1, 60, 60 | BP_PINMUX_VAL_3 },
  { bp_usMiiMdio,    -1, 61, 61 | BP_PINMUX_VAL_3 },
  { bp_usPonLbe, -1, 22, 22 | BP_PINMUX_VAL_1},
  { bp_last, -1, -1,  0 },
};

  static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;

#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
static bp_pinmux_fn_defs_t g_pinmux_fn_defs[] = {
  { BP_PINMUX_FNTYPE_xMII | 0, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 3, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 4, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 9, BP_PINMUX_VAL_DUMMY },
  { BP_PINMUX_FNTYPE_xMII | 12, 64 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 65 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 66 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 67 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 68 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 69 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 70 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 71 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 72 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 73 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 74 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 75 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 76 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 77 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 78 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 12, 79 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 80 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 81 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 82 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 83 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 84 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 85 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 86 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 87 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 88 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 89 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 90 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 11, 91 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_xMII | 13, 92 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 93 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 94 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 95 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 96 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 97 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 98 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 99 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 100 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 101 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 102 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_xMII | 13, 103 | BP_PINMUX_VAL_1 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 108 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 109 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 110 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 111 | BP_PINMUX_VAL_0 }, 
  { BP_PINMUX_FNTYPE_HS_SPI, 112 | BP_PINMUX_VAL_0 },  
  { BP_PINMUX_FNTYPE_I2S, 28 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_I2S, 29 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_I2S, 30 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_I2S, 31 | BP_PINMUX_VAL_2 },
  { BP_PINMUX_FNTYPE_PCM, 42 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 43 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 44 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_PCM, 45 | BP_PINMUX_VAL_1 },
  { BP_PINMUX_FNTYPE_NAND, 47 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 48 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 49 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 50 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 51 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 52 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 53 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 54 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 55 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 56 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 57 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 58 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 59 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 60 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_NAND, 61 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_EMMC, 62 | BP_PINMUX_VAL_3 },
  { BP_PINMUX_FNTYPE_EMMC, 63 | BP_PINMUX_VAL_3 },
};

static bp_pinmux_defs_t g_pinmux_defs_0[] = {
  { bp_usNetLed0, 0,  0,  0 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //QGPHY0
  { bp_usNetLed1, 0,  1,  1 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 0, 20, 20 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 1,  2,  2 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //QGPHY1
  { bp_usNetLed1, 1,  3,  3 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 1, 21, 21 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 2,  4,  4 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //QGPHY2
  { bp_usNetLed1, 2,  5,  5 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 2, 22, 22 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 3,  6,  6 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //QGPHY3
  { bp_usNetLed1, 3,  7,  7 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 3, 23, 23 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 4,  8,  8 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //SGPHY connected to P4
  { bp_usNetLed1, 4,  9,  9 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 4, 24, 24 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 4, 10, 10 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //HSGMII connected to P4
  { bp_usNetLed1, 4, 11, 11 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed2, 4, 12, 12 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 4, 25, 25 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 6,  8,  8 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //SGPHY connected to P6
  { bp_usNetLed1, 6,  9,  9 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 6, 24, 24 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 6, 10, 10 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //HSGMII connected to P6
  { bp_usNetLed1, 6, 11, 11 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed2, 6, 12, 12 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 6, 25, 25 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, SF2_WAN_PORT_NUM,  8,  8 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //SGPHY connected to EthWAN
  { bp_usNetLed1, SF2_WAN_PORT_NUM,  9,  9 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, SF2_WAN_PORT_NUM, 24, 24 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, SF2_WAN_PORT_NUM, 10, 10 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //HSGMII connected to EthWAN
  { bp_usNetLed1, SF2_WAN_PORT_NUM, 11, 11 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed2, SF2_WAN_PORT_NUM, 12, 12 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, SF2_WAN_PORT_NUM, 25, 25 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 1, 16, 16 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //EthWAN use dedicated XPORT WAN2 LED
  { bp_usNetLed1, 1, 17, 17 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 1, 27, 27 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, 0, 13, 13 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //xPON WAN Led
  { bp_usNetLed1, 0, 14, 14 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed2, 0, 15, 15 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, 0, 26, 26 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed0, SF2_WAN_PORT_NUM, 13, 13 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //xPON WAN Led used by EthWAN(WAN2 in XPORT)
  { bp_usNetLed1, SF2_WAN_PORT_NUM, 14, 14 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed2, SF2_WAN_PORT_NUM, 15, 15 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },
  { bp_usNetLed3, SF2_WAN_PORT_NUM, 26, 26 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED },

  { bp_usGpioLedAggregateAct, -1, 28, 28 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //Aggregate activity LED
  { bp_usGpioLedAggregateLnk, -1, 29, 29 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED }, //Aggregate link LED
  { bp_usGpioLedWanAct, 0, 30, 30 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED}, //DSL WAN activity
  { bp_usGpioLedWanAct, 1, 31, 31 | BP_PINMUX_VAL_4 | BP_PINMUX_HWLED}, //Second DSL WAN activity
  { bp_usGpioLedPwmReserved,  -1, 18, 18 | BP_PINMUX_VAL_4 | BP_PINMUX_PWMLED }, 
  { bp_usGpioLedPwmReserved,  -1, 19, 19 |  BP_PINMUX_VAL_4 | BP_PINMUX_PWMLED }, 

  { bp_usSerialLedData, -1, 0,  0 | BP_PINMUX_VAL_1}, 
  { bp_usSerialLedClk,  -1, 1,  1 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 2,  2 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedData, -1, 29, 29 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedClk,  -1, 30, 30 | BP_PINMUX_VAL_1 }, 
  { bp_usSerialLedMask, -1, 31, 31 | BP_PINMUX_VAL_1 },

  { bp_usUartSdin,  0, 106, 106 | BP_PINMUX_VAL_1},
  { bp_usUartSdout, 0, 107, 107 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,  1,  25,  25 | BP_PINMUX_VAL_2},
  { bp_usUartSdout, 1,  24,  24 | BP_PINMUX_VAL_2},
  { bp_usUartSdin,  1,  42,  42 | BP_PINMUX_VAL_4},
  { bp_usUartSdout, 1,  43,  43 | BP_PINMUX_VAL_4},
  { bp_usUartSdin,  2,   5,   5 | BP_PINMUX_VAL_1}, /*high speed UART */
  { bp_usUartSdout, 2,   6,   6 | BP_PINMUX_VAL_1},
  { bp_usUartCts,   2,   3,   3 | BP_PINMUX_VAL_1},
  { bp_usUartRts,   2,   4,   4 | BP_PINMUX_VAL_1},
  { bp_usUartSdin,  2,  23,  23 | BP_PINMUX_VAL_2},
  { bp_usUartSdout, 2,  22,  22 | BP_PINMUX_VAL_2},
  { bp_usUartCts,   2,  20,  20 | BP_PINMUX_VAL_2},
  { bp_usUartRts,   2,  21,  21 | BP_PINMUX_VAL_2},
  { bp_usUartSdin,  3,  20,  20 | BP_PINMUX_VAL_3},
  { bp_usUartSdout, 3,  21,  21 | BP_PINMUX_VAL_3},
  { bp_usUartSdin,  3,  26,  26 | BP_PINMUX_VAL_3},
  { bp_usUartSdout, 3,  27,  27 | BP_PINMUX_VAL_3},

  { bp_ReservedDslCtl,  -1, 32, 32 | BP_PINMUX_VAL_2 | BP_VDSLCTL_0 },
  { bp_ReservedDslCtl,  -1, 33, 33 | BP_PINMUX_VAL_2 | BP_VDSLCTL_1 },
  { bp_ReservedDslCtl,  -1, 34, 34 | BP_PINMUX_VAL_2 | BP_VDSLCTL_2 },
  { bp_ReservedDslCtl,  -1, 35, 35 | BP_PINMUX_VAL_2 | BP_VDSLCTL_3 },
  { bp_ReservedDslCtl,  -1, 36, 36 | BP_PINMUX_VAL_2 | BP_VDSLCTL_4 },
  { bp_ReservedDslCtl,  -1, 37, 37 | BP_PINMUX_VAL_2 | BP_VDSLCTL_5 },

  { bp_usRogueOnuEn,    -1, 34, 34 | BP_PINMUX_VAL_3},
  { bp_usRogueOnuEn,    -1, 40, 40 | BP_PINMUX_VAL_2},

  { bp_usGpioI2cSda,     0, 11, 11 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cScl,     0, 12, 12 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cSda,     0, 24, 24 | BP_PINMUX_VAL_3 },
  { bp_usGpioI2cScl,     0, 25, 25 | BP_PINMUX_VAL_3 },
  { bp_usGpioI2cSda,     1, 15, 15 | BP_PINMUX_VAL_6 },
  { bp_usGpioI2cScl,     1, 16, 16 | BP_PINMUX_VAL_6 },
  { bp_usGpioI2cSda,     1, 26, 26 | BP_PINMUX_VAL_2 },
  { bp_usGpioI2cScl,     1, 27, 27 | BP_PINMUX_VAL_2 },

  { bp_usI2sLrck,       -1, 29, 29 | BP_PINMUX_VAL_2 },
  { bp_usI2sSclk,       -1, 30, 30 | BP_PINMUX_VAL_2 },
  { bp_usI2sSdata,      -1, 31, 31 | BP_PINMUX_VAL_2 },

  { bp_usPcmSdin,       -1, 42, 42 | BP_PINMUX_VAL_1},
  { bp_usPcmSdout,      -1, 43, 43 | BP_PINMUX_VAL_1},
  { bp_usPcmClk,        -1, 44, 44 | BP_PINMUX_VAL_1},
  { bp_usPcmFs,         -1, 45, 45 | BP_PINMUX_VAL_1},

  { bp_usSpiSlaveSelectNum, 0, 111, 111 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 1, 112, 112 | BP_PINMUX_VAL_0},
  { bp_usSpiSlaveSelectNum, 2, 10, 10  | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 2, 25, 25 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 9,  9  | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 3, 26, 26 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 8,  8  | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 4, 27, 27 | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 5, 7,  7  | BP_PINMUX_VAL_1},
  { bp_usSpiSlaveSelectNum, 5, 28, 28 | BP_PINMUX_VAL_1},

  { bp_usUsbPwrFlt0, -1, 123, 123 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrOn0,  -1, 124, 124 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrFlt1, -1, 121, 121 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrOn1,  -1, 122, 122 | BP_PINMUX_VAL_2 },
  { bp_usUsbPwrFlt0, -1, 121, 121 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn0,  -1, 122, 122 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrFlt1, -1, 123, 123 | BP_PINMUX_VAL_1 },
  { bp_usUsbPwrOn1,  -1, 124, 124 | BP_PINMUX_VAL_1 },



  { bp_usMiiMdc,     -1, 104, 104 | BP_PINMUX_VAL_1},
  { bp_usMiiMdio,    -1, 105, 105 | BP_PINMUX_VAL_1},

  { bp_usVregSync,   -1, 19, 19 | BP_PINMUX_VAL_2 },

  { bp_usSfpSigDetect,  -1, 19, 19 | BP_PINMUX_VAL_6 }, //SGMII, can connect to their EthWAN or P4, P6
  { bp_usSfpSigDetect,  -1, 35, 35 | BP_PINMUX_VAL_3 },
  { bp_usSfpSigDetect,  0, 10, 10 | BP_PINMUX_VAL_2 }, //xPON
  { bp_usSfpSigDetect,  0, 37, 37 | BP_PINMUX_VAL_1 },
  { bp_usSfpSigDetect,  -1, 21, 21 | BP_PINMUX_VAL_6 }, //SGMII, can connect to their EthWAN or P4, P6
  
  { bp_ReservedAnyGpio, -1, -1,  BP_PINMUX_VAL_5 }, // ALL SW GPIOs use pinmux 5
  { bp_ReservedAnyLed,  -1, -1,  BP_PINMUX_VAL_4 }, // ALL SW LEDs use pinmux 4

  /* LED output on alternate GPIO pin with different pinmux */
  { bp_usNetLed0, 0, 64, 64 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(0) }, //QGPHY0
  { bp_usNetLed1, 0, 65, 65 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(1) },
  { bp_usNetLed3, 0, 84, 84 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(20) },
  { bp_usNetLed0, 1, 66, 66 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(2) }, //QGPHY1
  { bp_usNetLed1, 1, 67, 67 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(3) },
  { bp_usNetLed3, 1, 85, 85 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(21) },
  { bp_usNetLed0, 2, 68, 68 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(4) }, //QGPHY2
  { bp_usNetLed1, 2, 69, 69 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(5) },
  { bp_usNetLed3, 2, 86, 86 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(22) },
  { bp_usNetLed0, 3, 70, 70 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(6) }, //QGPHY3
  { bp_usNetLed1, 3, 71, 71 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(7) },
  { bp_usNetLed3, 3, 87, 87 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(23) },
  { bp_usNetLed0, 4, 72, 72 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8) }, //SGPHY connected to P4
  { bp_usNetLed1, 4, 73, 73 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_usNetLed3, 4, 88, 88 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(24) },
  { bp_usNetLed0, 4, 74, 74 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) }, //HSGMII connected to P4
  { bp_usNetLed1, 4, 75, 75 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_usNetLed2, 4, 76, 76 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_usNetLed3, 4, 89, 89 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(25) },
  { bp_usNetLed0, 6, 72, 72 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8) }, //SGPHY connected to P6
  { bp_usNetLed1, 6, 73, 73 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_usNetLed3, 6, 88, 88 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(24) },
  { bp_usNetLed0, 6, 74, 74 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) }, //HSGMII connected to P6
  { bp_usNetLed1, 6, 75, 75 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_usNetLed2, 6, 76, 76 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_usNetLed3, 6, 89, 89 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(25) },
  { bp_usNetLed0, SF2_WAN_PORT_NUM, 72, 72 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(8) }, //SGPHY connected to EthWAN
  { bp_usNetLed1, SF2_WAN_PORT_NUM, 73, 73 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(9) },
  { bp_usNetLed3, SF2_WAN_PORT_NUM, 88, 88 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(24) },
  { bp_usNetLed0, SF2_WAN_PORT_NUM, 74, 74 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(10) }, //HSGMII connected to EthWAN
  { bp_usNetLed1, SF2_WAN_PORT_NUM, 75, 75 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(11) },
  { bp_usNetLed2, SF2_WAN_PORT_NUM, 76, 76 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(12) },
  { bp_usNetLed3, SF2_WAN_PORT_NUM, 89, 89 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(25) },
  { bp_usNetLed0, 1, 80, 80 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(16) }, //EthWAN use XPORT WAN2 LED
  { bp_usNetLed1, 1, 81, 81 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(17) },
  { bp_usNetLed3, 1, 91, 91 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(27) },
  { bp_usNetLed0, 0, 77, 77 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13)}, //xPON WAN Led
  { bp_usNetLed1, 0, 78, 78 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(14)},
  { bp_usNetLed2, 0, 79, 79 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(15)},
  { bp_usNetLed3, 0, 90, 90 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(26)},
  { bp_usNetLed0, SF2_WAN_PORT_NUM, 77, 77 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(13)}, //xPON WAN Led used by EthWAN(WAN2 in XPORT)
  { bp_usNetLed1, SF2_WAN_PORT_NUM, 78, 78 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(14)},
  { bp_usNetLed2, SF2_WAN_PORT_NUM, 79, 79 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(15)},
  { bp_usNetLed3, SF2_WAN_PORT_NUM, 90, 90 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(26)},
  { bp_usGpioLedAggregateLnk, -1, 92, 92 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(28) }, //Aggregate link LED
  { bp_usGpioLedAggregateAct, -1, 93, 93 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(29) }, //Aggregate activity LED
  { bp_usGpioLedWanAct,        0, 94, 94 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(30)}, //DSL WAN activity
  { bp_usGpioLedWanAct,        1, 95, 95 | BP_PINMUX_VAL_6 | BP_PINMUX_HWLED | BP_PINMUX_OPTLED_NUM(31)}, //Second DSL WAN activi
  { bp_usGpioLedPwmReserved,  -1, 82, 82 | BP_PINMUX_VAL_6 | BP_PINMUX_PWMLED | BP_PINMUX_OPTLED_NUM(18) }, 
  { bp_usGpioLedPwmReserved,  -1, 83, 83 | BP_PINMUX_VAL_6 | BP_PINMUX_PWMLED | BP_PINMUX_OPTLED_NUM(19) }, 
  { bp_last, -1, -1,  0 },
};

static bp_pinmux_defs_t *g_pinmux_defs_tables[] = { g_pinmux_defs_0 } ;
#endif

/* Private function prototypes */
bp_elem_t * BpGetElem(enum bp_id id, bp_elem_t **pstartElem, enum bp_id stopAtId);
char *BpGetSubCp(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
void *BpGetSubPtr(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
unsigned char BpGetSubUc(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
unsigned short BpGetSubUs(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
unsigned int BpGetSubUl(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
int BpGetCp(enum bp_id id, char **pcpValue );
int BpGetUc(enum bp_id id, unsigned char *pucValue );
int BpGetUs(enum bp_id id, unsigned short *pusValue );
int BpGetUl(enum bp_id id, unsigned int *pulValue );
int BpGetGpio(enum bp_id id, unsigned short *pusValue );
#ifdef CONFIG_BP_PHYS_INTF
int BpIsPhyIntfInitDone(void);
bp_elem_t *BpGetIntfStart(unsigned short intfId);
int BpGetCpInIntf(enum bp_id id, char **pcpValue, PHYS_INTF_INFO* pIntf );
int BpGetUcInIntf(enum bp_id id, unsigned char *pucValue, PHYS_INTF_INFO* pIntf );
int BpGetUsInIntf(enum bp_id id, unsigned short *pusValue, PHYS_INTF_INFO* pIntf );
int BpGetUlInIntf(enum bp_id id, unsigned int *pulValue, PHYS_INTF_INFO* pIntf );
static int BpGetDslCtl(enum bp_id id, int intfIdx, unsigned short *pusValue );
static int BpGetGpioInIntf(enum bp_id id, PHYS_INTF_INFO* pIntf, unsigned short *pusValue );
int BpGetExtIntrGpioInIntf(enum bp_id id, PHYS_INTF_INFO* pIntf, unsigned short *pusValue );
#else
int BpGetDslCtl(enum bp_id id, unsigned short *pusValue );
#endif
int BpEnumElement(enum bp_id id, void** token, bp_elem_t **pelemout);
int BpEnumUc(enum bp_id id, void** token, unsigned char *pucValue);
int BpEnumUs(enum bp_id id, void** token, unsigned short *pusValue);
int BpEnumUl(enum bp_id id, void** token, unsigned int *pulValue);
int BpGetExtIntrGpio(enum bp_id id, unsigned short *pusValue);
int BpUseTemplate(bp_elem_t *pBoardParams);
int BpIsIdFromTemplate(enum bp_id id, bp_elem_t *pBoardParams, bp_elem_t **pElem);
int BpIsElemFromTemplate(bp_elem_t *pElem, bp_elem_t *pBoardParams);
bp_elem_t* BpGetLastElem(bp_elem_t *pBoardParams);
bp_elem_t* BpGetBoardParamsFromElem(bp_elem_t *pElem);
/**************************************************************************
* Name       : bpstrcmp
*
* Description: String compare for this file so it does not depend on an OS.
*              (Linux kernel and CFE share this source file.)
*
* Parameters : [IN] dest - destination string
*              [IN] src - source string
*
* Returns    : -1 - dest < src, 1 - dest > src, 0 dest == src
***************************************************************************/
int bpstrcmp(const char *dest,const char *src)
{
    while (*src && *dest) {
        if (*dest < *src) return -1;
        if (*dest > *src) return 1;
        dest++;
        src++;
    }

    if (*dest && !*src) return 1;
    if (!*dest && *src) return -1;
    return 0;
} /* bpstrcmp */


/**************************************************************************
* Name       : BpGetElem
*
* Description: Private function to walk through the profile
*              and find the desired entry
*
* Parameters : [IN] id             - id to search for
*              [IN/OUT] pstartElem - where to start and where it was found
*              [IN] stopAtId       - id to stop at if the searched id is not found
*                                    (allows grouping and repeated ids)
*
* Returns    : ptr to entry found or to last entry otherwise
***************************************************************************/
bp_elem_t * BpGetElem(enum bp_id id, bp_elem_t **pstartElem, enum bp_id stopAtId)
{
    bp_elem_t * pelem;
#ifdef CONFIG_BP_PHYS_INTF
    unsigned short intfId = -1;
    bp_elem_t *pIntfStart = NULL, *pBpIntfStart = NULL;
#endif
    
    // when compiling CFE, it does not like 'NULL' hence using 0
    if ( 0 == *pstartElem )
        *pstartElem = g_pCurrentBp;

    pelem = *pstartElem;
    while(1) 
    {
        if( pelem->id == bp_last || pelem->id == stopAtId )
            break;

        if( pelem->id == id ) 
        {
#ifdef CONFIG_BP_PHYS_INTF
            /* check if this id belong to an interface. If so, check if it is the intf being override */
            if( pIntfStart != NULL && BpIsPhyIntfInitDone() )
            {
                 pBpIntfStart = BpGetIntfStart(intfId);
                 if( pBpIntfStart == NULL ) /* Something really wrong here. Should always find the intf */
                 {
                     printk("BpGetElem can not find intf %d!!!\n", intfId);
                     break;
                 }

                 if( pBpIntfStart != pIntfStart ) /* this intf is from template, ignore it */
                 {
                     pelem++;
                     (*pstartElem)++;
                     continue;
                 }
             }
             break;
#else
             break;
#endif
        }

#ifdef CONFIG_BP_PHYS_INTF
        // save the intf id and its element ptr
        if ( bp_usIntfId == pelem->id )
        {
            intfId = pelem->u.us;
            pIntfStart = pelem;
        }
        if ( bp_usIntfEnd == pelem->id )
        {
            intfId = -1;
            pIntfStart = NULL;
        }
#endif

        // found template so jump to it
        // any entries after bp_elemTemplate will be ignored
        if ( bp_elemTemplate == pelem->id ) {
            *pstartElem = pelem->u.bp_elemp;
            pelem = *pstartElem;
            // ignoring the first element of this new array
            // because it is always bp_cpBoardId
        }

        pelem++;
        (*pstartElem)++;
    }

    return pelem; 
}

#ifdef CONFIG_BP_PHYS_INTF
/**************************************************************************
* Name       : BpGetElemIntf
*
* Description: Private function to walk through the profile
*              and find the desired entry
*
* Parameters : [IN] id             - id to search for
*              [IN/OUT] pstartElem - where to start and where it was found
*              [IN] stopAtId       - id to stop at if the searched id is not found
*                                    (allows grouping and repeated ids)
*
*              [OUT] intf          - the intf id associated with this bp id. -1 if 
*                                    not available 
* Returns    : ptr to entry found or to last entry otherwise
***************************************************************************/
bp_elem_t * BpGetElemIntf(enum bp_id id, bp_elem_t **pstartElem, enum bp_id stopAtId, int* intf)
{
    bp_elem_t * pelem;
    
    *intf = -1;
    // when compiling CFE, it does not like 'NULL' hence using 0
    if ( 0 == *pstartElem )
        *pstartElem = g_pCurrentBp;

    for (pelem = *pstartElem; 
         pelem->id != bp_last && pelem->id != id && pelem->id != stopAtId; 
         pelem++, (*pstartElem)++) 
    {
        // save the intf id 
        if ( bp_usIntfId == pelem->id )
            *intf = (int)pelem->u.us;

        // found template so jump to it
        // any entries after bp_elemTemplate will be ignored
        if ( bp_elemTemplate == pelem->id ) {
            *pstartElem = pelem->u.bp_elemp;
            pelem = *pstartElem;
            // ignoring the first element of this new array
            // because it is always bp_cpBoardId
        }
    }

    return pelem; 
}
#endif

/**************************************************************************
* Name       : BpGetNextElem
*
* Description: Private function to get next entry.
*
* Parameters : [IN] pelem     - element pointer
*
* Returns    : ptr to next entry found or NULL if already at the end
*
* Note:      this has a flaw, in that if you define a template, and
*            reach the end of the template, it will not return you to the
*            original template.  (BpGetElem suffers the same flaw...)
***************************************************************************/
static bp_elem_t * BpGetNextElem(bp_elem_t *pelem)
{   
    if ( 0 == pelem )
        return g_pCurrentBp;

    if ( bp_elemTemplate == pelem->id ) {
        return pelem->u.bp_elemp;
    }

    if ( bp_last == pelem->id )
        return NULL;

    return pelem+1; 
}



/**************************************************************************
* Name       : BpGetSubCp
*
* Description: Private function to get an char * entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char * from the entry
***************************************************************************/
char *BpGetSubCp(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id) { 
        return pelem->u.cp;
    } else { 
        return (char *)BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubPtr
*
* Description: Private function to get an void * entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char * from the entry
***************************************************************************/
void *BpGetSubPtr(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id) {
        return pelem->u.ptr;
    } else {
        return (void *)0;
    }
}

/**************************************************************************
* Name       : BpGetSubUc
*
* Description: Private function to get an unsigned char entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char from the entry
***************************************************************************/
unsigned char BpGetSubUc(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id) {
        return pelem->u.uc;
    } else {
        return (unsigned char)BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubUs
*
* Description: Private function to get an unsigned short entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned short from the entry
***************************************************************************/
unsigned short BpGetSubUs(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id) {
        return pelem->u.us;
    } else {
        return BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubUl
*
* Description: Private function to get an unsigned int entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned int from the entry
***************************************************************************/
unsigned int BpGetSubUl(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id)
        return pelem->u.ul;
    else
        return BP_NOT_DEFINED;
}

/**************************************************************************
* Name       : BpGetCp
*
* Description: Private function to get an char * entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pulValue - char ** found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetCp(enum bp_id id, char **pcpValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pcpValue = BpGetSubCp(id, 0, bp_last);
        nRet = ((char *)BP_NOT_DEFINED != *pcpValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {     
        *pcpValue = (char *)BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUc
*
* Description: Private function to get an unsigned char entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pucValue - unsigned char found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUc(enum bp_id id, unsigned char *pucValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pucValue = BpGetSubUc(id, 0, bp_last);
        nRet = ((unsigned char)BP_NOT_DEFINED != *pucValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pucValue = (unsigned char)BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUs
*
* Description: Private function to get an unsigned short entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pusValue - unsigned short found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUs(enum bp_id id, unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pusValue = BpGetSubUs(id, 0, bp_last);
        nRet = (BP_NOT_DEFINED != *pusValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUl
*
* Description: Private function to get an unsigned int entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pulValue - unsigned int found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUl(enum bp_id id, unsigned int *pulValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pulValue = BpGetSubUl(id, 0, bp_last);
        nRet = (BP_NOT_DEFINED != *pulValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetLastElem
*
* Description: Private function to get the ptr of bp_last element from the 
*              board parameter
*
* Parameters : [IN] pBoardParams  - start address of board parameter
*
* Returns    : ptr of bp_last element
***************************************************************************/
bp_elem_t * BpGetLastElem(bp_elem_t *pBoardParams)
{
    bp_elem_t * pelem = pBoardParams;

    while( pelem->id != bp_last ) 
        pelem++;

    return pelem;
}

#ifdef CONFIG_BP_PHYS_INTF
/**************************************************************************
* Name       : BpGetCpInIntf
*
* Description: Private function to get an strinng entry from the board parameter
*              within the interface range
*
* Parameters : [IN] id       - id to search for
*              [IN] pcpValue - piont to the string 
*              [IN] pIntf    - interface to be searched
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetCpInIntf(enum bp_id id, char **pcpValue, PHYS_INTF_INFO* pIntf )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pcpValue = BpGetSubCp(id, pIntf->pIntfStart, bp_usIntfEnd);
        nRet = ((char *)BP_NOT_DEFINED != *pcpValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {     
        *pcpValue = (char *)BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUcInIntf
*
* Description: Private function to get an unsigned char entry from the board parameter
*              within the interface range
*
* Parameters : [IN] id       - id to search for
*              [IN] pucValue - unsigned char found
*              [IN] pIntf    - interface to be searched
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUcInIntf(enum bp_id id, unsigned char *pucValue, PHYS_INTF_INFO* pIntf )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pucValue = BpGetSubUc(id, pIntf->pIntfStart, bp_usIntfEnd);
        nRet = ((unsigned char)BP_NOT_DEFINED != *pucValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pucValue = (unsigned char)BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUcInIntf
*
* Description: Private function to get an unsigned char entry from the board parameter
*              within the interface range
*
* Parameters : [IN] id       - id to search for
*              [IN] pusValue - unsigned short found
*              [IN] pIntf    - interface to be searched
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUsInIntf(enum bp_id id, unsigned short *pusValue, PHYS_INTF_INFO* pIntf )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pusValue = BpGetSubUs(id, pIntf->pIntfStart, bp_usIntfEnd);
        nRet = (BP_NOT_DEFINED != *pusValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUcInIntf
*
* Description: Private function to get an unsigned char entry from the board parameter
*              within the interface range
*
* Parameters : [IN] id       - id to search for
*              [IN] pusValue - unsigned int found
*              [IN] pIntf    - interface to be searched
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUlInIntf(enum bp_id id, unsigned int *pulValue, PHYS_INTF_INFO* pIntf )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pulValue = BpGetSubUl(id, pIntf->pIntfStart, bp_usIntfEnd);
        nRet = (BP_NOT_DEFINED != *pulValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}
#endif
/**************************************************************************
* Name       : BpEnumElement
*
* Description: Enumerate the board parameters for all the instance of Us boardparms id 
*
* Parameters : [IN] id - boardparm id to enumerate.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pelem - pointer to the bp element of the id
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
***************************************************************************/
int BpEnumElement(enum bp_id id, void** token, bp_elem_t **pelemout)
{
    bp_elem_t *pelem, *pnext;
    int rc;
    enum bp_id stop_id;
#ifdef CONFIG_BP_PHYS_INTF
    /* with the interface based bp, BpGetElem can identidfy what intf this id belongs and ignore the one
       from the template. So always search to the end */
    stop_id = bp_last;
#else
    /* always search whole bp at start. If the id is found in the main body or the template, 
       any other instance of the same id has be to in the main body or the same template too, 
       so don't search the next template to avoid getting duplicated items for the next iteration */
    if( *token == NULL )
        stop_id = bp_last;
    else
        stop_id = bp_elemTemplate;
#endif
    pelem = (bp_elem_t*)(*token);
    pnext = BpGetElem(id, &pelem, stop_id);

    if (id == pnext->id) 
    {
        *pelemout = pnext;
        pnext++;
        *token = (void*)pnext;
        rc = BP_SUCCESS;
    }
    else
    {
        *pelemout = NULL;
        if( *token == NULL )
        {
            /* no such bp item at all */   
            rc = BP_VALUE_NOT_DEFINED;
        }
        else
        {
            /* previous instant exists but we reach to the end... */   
            rc = BP_SUCCESS_LAST;
        }
    }

    return rc;
}

/**************************************************************************
* Name       : BpEnumUc
*
* Description: Enumerate the board parameters for all the instance of Uc boardparms id 
*
* Parameters : [IN] id - boardparm id to enumerate.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pucValue - value of the id
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
***************************************************************************/
int BpEnumUc(enum bp_id id, void** token, unsigned char *pucValue)
{
    int rc; 
    bp_elem_t* pelem;

    rc = BpEnumElement(id, token, &pelem);
    if( rc == BP_SUCCESS )
        *pucValue = pelem->u.uc;
    else 
        *pucValue = (unsigned char)BP_NOT_DEFINED;

    return rc;
}


/**************************************************************************
* Name       : BpEnumUs
*
* Description: Enumerate the board parameters for all the instance of Us boardparms id 
*
* Parameters : [IN] id - boardparm id to enumerate.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pusValue - value of the id
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
***************************************************************************/
int BpEnumUs(enum bp_id id, void** token, unsigned short *pusValue)
{
    int rc; 
    bp_elem_t* pelem;

    rc = BpEnumElement(id, token, &pelem);
    if( rc == BP_SUCCESS )
        *pusValue = pelem->u.us;
    else 
        *pusValue = BP_NOT_DEFINED;

    return rc;
}

/**************************************************************************
* Name       : BpEnumUl
*
* Description: Enumerate the board parameters for all the instance of Ul boardparms id 
*
* Parameters : [IN] id - boardparm id to enumerate.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pulValue - value of the id
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
***************************************************************************/
int BpEnumUl(enum bp_id id, void** token, unsigned int *pulValue)
{
    int rc; 
    bp_elem_t* pelem;

    rc = BpEnumElement(id, token, &pelem);
    if( rc == BP_SUCCESS )
        *pulValue = pelem->u.ul;
    else 
        *pulValue = BP_NOT_DEFINED;

    return rc;
}

/**************************************************************************
* Name       : BpGetVoipDspConfig
*
* Description: Gets the DSP configuration from the board parameter
*              structure for a given DSP index.
*
* Parameters : [IN] dspNum - DSP index (number)
*
***************************************************************************/
VOIP_DSP_INFO g_VoIPDspInfo[BP_MAX_VOIP_DSP] = {{0}};
VOIP_DSP_INFO *BpGetVoipDspConfig( unsigned char dspNum )
{
    VOIP_DSP_INFO *pDspConfig = 0;
    int i;
    bp_elem_t *pelem;
    bp_elem_t *pDspType;
    enum bp_id bp_aucDspType[BP_MAX_VOIP_DSP+1] = {bp_ucDspType0, bp_ucDspType1, bp_last};
    enum bp_id bp_current, bp_next;

    if( g_pCurrentBp ) {
        /* First initialize the structure to known values */
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ ) {
            g_VoIPDspInfo[i].ucDspType = BP_VOIP_NO_DSP;
        }

        /* Now populate it with what we have in the element array */
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ ) {
            pDspType = 0;
            bp_current = bp_aucDspType[i];
            bp_next    = bp_aucDspType[i+1];
            pelem = BpGetElem(bp_current, &pDspType, bp_next);
            if (bp_current != pelem->id) {
                continue;
            }
            g_VoIPDspInfo[i].ucDspType = pelem->u.uc;

            ++pDspType;
            g_VoIPDspInfo[i].ucDspAddress       = BpGetSubUc(bp_ucDspAddress, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioLedVoip      = BpGetSubUs(bp_usGpioLedVoip, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip1Led     = BpGetSubUs(bp_usGpioVoip1Led, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip1LedFail = BpGetSubUs(bp_usGpioVoip1LedFail, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip2Led     = BpGetSubUs(bp_usGpioVoip2Led, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip2LedFail = BpGetSubUs(bp_usGpioVoip2LedFail, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioPotsLed      = BpGetSubUs(bp_usGpioPotsLed, pDspType, bp_next);
            g_VoIPDspInfo[i].usGpioDectLed      = BpGetSubUs(bp_usGpioDectLed, pDspType, bp_next);
        }

        /* Transfer the requested results */
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ ) {
            if( g_VoIPDspInfo[i].ucDspType != BP_VOIP_NO_DSP &&
                g_VoIPDspInfo[i].ucDspAddress == dspNum ) {
                pDspConfig = &g_VoIPDspInfo[i];
                break;
            }
        }
    }

    return pDspConfig;
} /* BpGetVoipDspConfig */

/**************************************************************************
* Name       : BpSetBoardId
*
* Description: This function find the BOARD_PARAMETERS structure for the
*              specified board id string and assigns it to a global, static
*              variable.
*
* Parameters : [IN] pszBoardId - Board id string that is saved into NVRAM.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_FOUND - Error, board id input string does not
*                  have a board parameters configuration record.
***************************************************************************/
int BpSetBoardId(const char *pszBoardId )
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    bp_elem_t **ppcBp;

    for( ppcBp = g_BoardParms; *ppcBp; ppcBp++ ) {
        if( !bpstrcmp((*ppcBp)[0].u.cp, pszBoardId) ) {
            /* Changing the current board ID -
             * Invalidate the Global copy of Ethernet MAC Info, so it get refreshed 
             * NOTE : There is no protection around pEnetMacInfo if two threads setBoardId & GetMacInfo
             * Possible that GetMacInfo may return NULL; */
            pEnetMacInfo = NULL;
            g_pCurrentBp = *ppcBp;

            nRet = BP_SUCCESS;
#ifdef CONFIG_BP_PHYS_INTF
            /* init physical interface configuraiton */
            nRet = BpInitPhyIntfInfo();
#endif
            break;
        }
    }

    return( nRet );
} /* BpSetBoardId */

/**************************************************************************
* Name       : BpGetBoardId
*
* Description: This function returns the current board id strings.
*
* Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
*                  string is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
***************************************************************************/
int BpGetBoardId( char *pszBoardId )
{
    int i;

    if (g_pCurrentBp == 0) {
        return -1;
    }

    for (i = 0; i < BP_BOARD_ID_LEN; i++) {
        pszBoardId[i] = g_pCurrentBp[0].u.cp[i];
    }

    return 0;
}

/**************************************************************************
* Name       : BpGetBoardIdNameByIndex
*
* Description: This function returns the pointer to the board id name indexed by i. 
*
* Parameters : [OUT] Address of a buffer that the board id
*                    string is returned in.
*              [IN]  Index of the board in the g_BoardParms array.            
*
* Returns    : BP_SUCCESS - Success, value is returned.
***************************************************************************/
char * BpGetBoardIdNameByIndex( int i )
{
    return g_BoardParms[i][0].u.cp;
}


/**************************************************************************
* Name       : BpGetBoardIds
*
* Description: This function returns all of the supported board id strings.
*
* Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
*                  strings are returned in.  Each id starts at BP_BOARD_ID_LEN
*                  boundary.
*              [IN] nBoardIdsSize - Number of BP_BOARD_ID_LEN elements that
*                  were allocated in pszBoardIds.
*
* Returns    : Number of board id strings returned.
***************************************************************************/
int BpGetBoardIds( char *pszBoardIds, int nBoardIdsSize )
{
    int i;
    char *src;
    char *dest;
    bp_elem_t **ppcBp;

    for( i = 0, ppcBp = g_BoardParms; *ppcBp && nBoardIdsSize;
        i++, ppcBp++, nBoardIdsSize--, pszBoardIds += BP_BOARD_ID_LEN ) {
        dest = pszBoardIds;
        src = (*ppcBp)[0].u.cp;
        while( *src ) {
            *dest++ = *src++;
        }
        *dest = '\0';
    }

    return( i );
} /* BpGetBoardIds */

int BPGetNumBoardIds(void)
{
    int i = 0;
    while (g_BoardParms[i] != 0) {
       i++;
    }
    return ( i );
}

/**************************************************************************
* Name       : BpUseTemplate
*
* Description: Private function to check if board parameter use template or not
*      
* Parameters : [IN] - start element address of the board parameter
* Returns    : one if board parameter use template, zero otherwise
***************************************************************************/
int BpUseTemplate(bp_elem_t *pBoardParams)
{
    bp_elem_t *pelem = pBoardParams;
    int rc = 0;

    while( pelem->id != bp_last )
    {
        if(pelem->id == bp_elemTemplate )
        {
            rc = 1;
            break;
        }
        pelem++;
    }
    return rc;
}

/**************************************************************************
* Name       : BpIsIdFromTemplate
*
* Description: Private function to check if bp_id is from template of board
*              parameter or not
*      
* Parameters : [IN] id             - id to search for
*              [IN] pBoardParams   - start element address of the board parameter
*              [OUT] pElem         - ptr to bp id entry found or NULL if not found
* Returns    : one if it is from the template, zero from board parameter itself
*              or id not found
***************************************************************************/
int BpIsIdFromTemplate(enum bp_id id, bp_elem_t *pBoardParams, bp_elem_t **pElem)
{
    bp_elem_t *pTargetElem, *pLastElem, *pBpStart = pBoardParams;

    pTargetElem = BpGetElem(id, &pBpStart, bp_last);
    if( pTargetElem->id != id ) 
    {
       *pElem = NULL;
        return 0; 
    }

    *pElem = pTargetElem;
    pLastElem = BpGetLastElem(pBoardParams);

    if( pTargetElem >= pBoardParams && pTargetElem <=  pLastElem )
        return 0;
    else
        return 1;
}

/**************************************************************************
* Name       : BpIsElemFromTemplate
*
* Description: Private function to check if bp element is from template of board
*              parameter or not
*      
* Parameters : [IN] pElem         - ptr to bp element
*              [IN] pBoardParams  - start element address of the board parameter
* Returns    : one if it is from the template, zero from board parameter itself
***************************************************************************/
int BpIsElemFromTemplate(bp_elem_t *pElem, bp_elem_t *pBoardParams)
{
    bp_elem_t *pLastElem = BpGetLastElem(pBoardParams);

    if( pElem >= pBoardParams && pElem <=  pLastElem )
        return 0;
    else
        return 1;
}
/**************************************************************************
* Name       : BpGetBoardParamsFromElem
*
* Description: Find the board paramter that an element entry belongs to
*      
* Parameters : [IN] pElem  - element entry
* Returns    : The start address of the board parameter or NULL if not found
***************************************************************************/
bp_elem_t* BpGetBoardParamsFromElem(bp_elem_t *pElem)
{
    bp_elem_t *pBp, *pLastElem;
    int i, numOfBoard =  BPGetNumBoardIds();

    for( i = 0; i < numOfBoard; i++ )
    {
        pBp = g_BoardParms[i];
        pLastElem = BpGetLastElem(pBp);
        if( pElem >= pBp && pElem <= pLastElem ) {
            return pBp;     
        }
    }

    return NULL;
}


/**************************************************************************
* Name       : BpGetComment
*
* Description: This function returns is used to get a comment for a board.
*
* Parameters : [OUT] pcpValue - comment string.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetComment( char **pcpValue )
{
    return( BpGetCp(bp_cpComment, pcpValue ) );
} /* BpGetComment */


/**************************************************************************
* Name       : BpEnumCompatChipId
*
* Description: This function enumerate the compatible chip ids for the board id
*
* Parameters : [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pulValue - the compatible chip id or BP_VALUE_NOT_DEFINED
*              if compatible chip id is not defined in the board parameter
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
***************************************************************************/
int BpEnumCompatChipId( void** token, unsigned int *pulValue )
{
    return BpEnumUl(bp_ulCompatChipId, token, pulValue);
}


/**************************************************************************
* Name       : BpGetGPIOverlays
*
* Description: This function GPIO overlay configuration
*
* Parameters : [OUT] pusValue - Address of short word that interfaces in use.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGPIOverlays( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulGpioOverlay, pulValue ) );
} /* BpGetGPIOverlays */

// IsParamPortHwLed  -- check if a parameter is a HWLED associated with a network port
//                      and, therefore, can be ignored by the ethernet driver
static int IsParamPortHwLed(enum bp_id id) 
{
#if !defined(_BCM960333_) && !defined(CONFIG_BCM960333) && !defined(_BCM963268_) && !defined(CONFIG_BCM963268) && !defined(_BCM96838_) && !defined(CONFIG_BCM96838) && !defined(_BCM947189_) && !defined(CONFIG_BCM947189)
    int n = 0;
    while (1) {
        if (g_pinmux_defs[n].id  == bp_last) {
            return(0);
        }
        else if ((g_pinmux_defs[n].id == id) 
                 && ((g_pinmux_defs[n].mux_info & BP_PINMUX_HWLED) != 0)
                 && (g_pinmux_defs[n].port >= 0)
                ) {
            return(1);
        }
        n++;
    }
#else
    return(0);
#endif
}

/**************************************************************************
* Name       : BpGetSgmiiGpios
*
* Description: This function returns the GPIO pin assignments for the SFP
*              Module Insertion Detection.
*
* Parameters : [OUT] sfpGpio - Address of short word that the SGMII GPIO pin
*                  is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSfpDetectGpio( unsigned short *sfpDetectGpio)
{
    *sfpDetectGpio = BP_NOT_DEFINED;
    return BpGetGpio (bp_usGpioSfpDetect, sfpDetectGpio);
} /* BpGetSfpDetectGpio */

/**************************************************************************
* Name       : BpGetSgmiiGpios
*
* Description: This function returns the GPIO pin assignments for the SGMII
*              Signal Loss Detection.
*
* Parameters : [OUT] sgmiiGpio - Address of short word that the SGMII GPIO pin
*                  is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSgmiiGpios( unsigned short *sgmiiGpio)
{
    *sgmiiGpio = BP_NOT_DEFINED;
    return BpGetGpio (bp_usSgmiiDetect, sgmiiGpio);
} /* BpGetSgmiiGpios */


/**************************************************************************
* Name       : GetSwitchBpSearchStopId
*
* Description: This function determine the stop search id for switch structure.
*
* Parameters : None
*
* Returns    : bp_elemTemplate if switch bp ids are from the bd parameter itself.
*              Do not search anything from template
*              bp_last if switch bp ids are from the template. Search everything
*              including from the template
***************************************************************************/
static enum bp_id GetSwitchBpSearchStopId(void)
{
    bp_elem_t *pBoardParam = g_pCurrentBp;
    bp_elem_t *pElem = NULL;
    enum bp_id stopAtId = bp_last;

    if ( BpIsIdFromTemplate(bp_ucPhyType0, pBoardParam, &pElem) && pElem )
    {
         /* check the template id is from inherit or not */
         pBoardParam = BpGetBoardParamsFromElem(pElem);
        if( pBoardParam == NULL)
        {
            //printk("The base id of board id %s is not in the board list!!!\n", g_pCurrentBp->u.cp);
            // we remove the deprecated base board id from the board list but then we should redefine new board id
            // without the base to avoid pontential issue when the base id has unwanted id for the new board id
            return bp_last;
        }
        else 
        {
            //printk("switch bp from template of board id %s\n", pBoardParam->u.cp);
        }

        if( BpUseTemplate(pBoardParam) ) {
            //printk("The base id %s use template but switch id from its own body!!!\n", pBoardParam->u.cp);
            stopAtId = bp_elemTemplate;
        }
        else {
            //printk("The base id %s has no template and contains the switch id!!!\n", pBoardParam->u.cp);
            stopAtId = bp_last;
        }
    }
    else {
        if( BpUseTemplate(pBoardParam) ) {
            //printk("The board id %s use template but switch id from its own body!!!\n", pBoardParam->u.cp);
            stopAtId = bp_elemTemplate;
        }
        else {
            //printk("The board id %s has no template and contains the switch id!!!\n", pBoardParam->u.cp);
            stopAtId = bp_last;
        }
    }
    return stopAtId;
}
/**************************************************************************
* Name       : GetEthernetMacInfo
*
* Description: This function returns all of the supported board id strings.
*
* Parameters : [OUT] pEnetInfos - Address of an array of ETHERNET_MAC_INFO
*                  buffers.
*              [IN] nNumEnetInfos - Number of ETHERNET_MAC_INFO elements that
*                  are pointed to by pEnetInfos.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
static int GetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos )
{
    int i, j;
    int crossbar_port;
    int in_crossbar;
    bp_elem_t *pelem;
    bp_elem_t *pPhyType;
    bp_elem_t *pPhyId;
    int nRet = BP_BOARD_ID_NOT_SET;
    PETHERNET_MAC_INFO pE;
    enum bp_id stopAtId = bp_last;
    enum bp_id bp_aucPhyType[BP_MAX_ENET_MACS+1] = {bp_ucPhyType0, bp_ucPhyType1, bp_last};
    enum bp_id bp_current, bp_next;
    enum bp_id bp_aulPhyId[BP_MAX_SWITCH_PORTS+1] = PHYID_LIST;
    enum bp_id bp_current_phyid;

    /* switch bp structure can be either entirely inherited from the base id or define entirely
       in the board parameter. Partially inherited from base is not allowed. Need to 
       determine the switich bp is inherited or not to set the stop search id correctly */
    stopAtId = GetSwitchBpSearchStopId();

    /* First initialize the structure to known values */
    for( i = 0, pE = pEnetInfos; i < nNumEnetInfos; i++, pE++ ) {
        pE->ucPhyType = BP_ENET_NO_PHY;
        /* The old code only initialized the first set, let's tdo the same so the 2 compare without error */
        if (0 == i) {
	  for( j = 0; j < BP_MAX_SWITCH_PORTS; j++ ) {
                pE->sw.ledInfo[j].duplexLed = BP_NOT_DEFINED;
                pE->sw.ledInfo[j].speedLed100 = BP_NOT_DEFINED;
                pE->sw.ledInfo[j].speedLed1000 = BP_NOT_DEFINED;
                pE->sw.ledInfo[j].LedLan = BP_NOT_DEFINED;
            }
        }

        for (j = 0; j < BP_MAX_CROSSBAR_EXT_PORTS ; j++) {
                pE->sw.crossbar[j].phy_id = 0;
                pE->sw.crossbar[j].switch_port = BP_CROSSBAR_NOT_DEFINED;
                pE->sw.crossbar[j].ledInfo.duplexLed = BP_NOT_DEFINED;
                pE->sw.crossbar[j].ledInfo.speedLed100 = BP_NOT_DEFINED;
                pE->sw.crossbar[j].ledInfo.speedLed1000 = BP_NOT_DEFINED;
                pE->sw.crossbar[j].ledInfo.LedLan = BP_NOT_DEFINED;
                pE->sw.crossbar[j].phyReset = BP_GPIO_NONE;
        }
    }

    if( g_pCurrentBp ) {
        /* Populate it with what we have in the element array */
        for( i = 0, pE = pEnetInfos; i < nNumEnetInfos; i++, pE++ ) {
            pPhyType = 0;
            bp_current = bp_aucPhyType[i];
            bp_next    = bp_aucPhyType[i+1];
            pelem = BpGetElem(bp_current, &pPhyType, bp_next);
            if (bp_current != pelem->id)
                continue;
            pE->ucPhyType = pelem->u.uc;
            /* update the search end id to make sure we search the correct range */
            if( bp_next == bp_last )
                bp_next = stopAtId;
            ++pPhyType;
            pE->ucPhyAddress  = BpGetSubUc(bp_ucPhyAddress, pPhyType, bp_next);
            pE->usConfigType  = BpGetSubUs(bp_usConfigType, pPhyType, bp_next);
            pE->sw.port_map   = BpGetSubUl(bp_ulPortMap, pPhyType, bp_next);
            crossbar_port = -1;

            for( j = 0; j < BP_MAX_SWITCH_PORTS; j++ ) {
                pPhyId = pPhyType;
                bp_current_phyid = bp_aulPhyId[j];
                pelem = BpGetElem(bp_current_phyid, &pPhyId, bp_next);
                pE->sw.phyconn[j] = PHY_CONN_TYPE_NOT_DEFINED;
                pE->sw.phy_devName[j] = PHY_DEVNAME_NOT_DEFINED;
                pE->sw.phyinit[j] = (bp_mdio_init_t* )0;
                pE->sw.port_flags[j] = 0;
                pE->sw.oamIndex[j] = -1;
                pE->sw.ledInfo[j].duplexLed = BP_GPIO_NONE;
                pE->sw.ledInfo[j].LedLan = BP_GPIO_NONE;
                pE->sw.ledInfo[j].speedLed100 = BP_GPIO_NONE;
                pE->sw.ledInfo[j].speedLed1000 = BP_GPIO_NONE;
                pE->sw.phyReset[j] = BP_GPIO_NONE;
                in_crossbar = 0;
                if (bp_current_phyid == pelem->id) {
                    pE->sw.phy_id[j] = pelem->u.ul & ~MII_OVER_GPIO_VALID;
                    ++pPhyId;
                    if (bp_current_phyid == pPhyId->id) {
                        pE->sw.phy_id_ext[j] = pPhyId->u.ul;
                        ++pPhyId;
                    }
                    while (pPhyId) {
                        if (in_crossbar == 0) {
                            switch (pPhyId->id) {
                                case bp_usDuplexLed:
                                    pE->sw.ledInfo[j].duplexLed = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usSpeedLed100:
                                    pE->sw.ledInfo[j].speedLed100 = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usSpeedLed1000:
                                    pE->sw.ledInfo[j].speedLed1000 = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usPhyConnType:
                                    pE->sw.phyconn[j] = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usGpioLedLan:
                                    pE->sw.ledInfo[j].LedLan = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_ucPhyDevName:
                                    pE->sw.phy_devName[j] = pPhyId->u.cp;
                                    ++pPhyId;
                                    break;                            
                                case bp_pPhyInit:
                                    pE->sw.phyinit[j] = (bp_mdio_init_t*)pPhyId->u.ptr;
                                    ++pPhyId;
                                    break;
                                case bp_ulPortFlags:
                                    pE->sw.port_flags[j] = pPhyId->u.ul;
                                    ++pPhyId;
                                    break;
                                case bp_usOamIndex:
                                    pE->sw.oamIndex[j] = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usGpioPhyReset:
                                    pE->sw.phyReset[j] = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_ulPortMaxRate:
                                    pE->sw.portMaxRate[j] = pPhyId->u.ul;
                                    ++pPhyId;
                                    break;
                                case bp_ulCrossbar:
                                    crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(pPhyId->u.ul);
                                    pE->sw.crossbar[crossbar_port].switch_port = j;
                                    in_crossbar = 1;
                                    ++pPhyId;
                                    break;
                                default:
                                    if (IsParamPortHwLed( pPhyId->id ) != 0) {
                                        // skip over unknown port parameters that the enet driver doesn't need to know about
                                        ++pPhyId;
                                    } else {
                                        pPhyId = 0;
                                    }
                                    break;
                            }
                        } else {
                            switch (pPhyId->id) {
                                case bp_ulCrossbarPhyId:
                                    if (pE->sw.crossbar[crossbar_port].phy_id == 0) {
                                        pE->sw.crossbar[crossbar_port].phy_id = pPhyId->u.ul;
                                    }
                                    else {
                                        pE->sw.crossbar[crossbar_port].phy_id_ext = pPhyId->u.ul;
                                    }
                                    ++pPhyId;
                                    break;
                                case bp_usDuplexLed:
                                    pE->sw.crossbar[crossbar_port].ledInfo.duplexLed = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usSpeedLed100:
                                    pE->sw.crossbar[crossbar_port].ledInfo.speedLed100 = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usSpeedLed1000:
                                    pE->sw.crossbar[crossbar_port].ledInfo.speedLed1000 = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usPhyConnType:
                                    pE->sw.crossbar[crossbar_port].phyconn = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usGpioLedLan:
                                    pE->sw.crossbar[crossbar_port].ledInfo.LedLan = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_ucPhyDevName:
                                    pE->sw.crossbar[crossbar_port].phy_devName = pPhyId->u.cp;
                                    ++pPhyId;
                                    break;                            
                                case bp_pPhyInit:
                                    pE->sw.crossbar[crossbar_port].phyinit = (bp_mdio_init_t*)pPhyId->u.ptr;
                                    ++pPhyId;
                                    break;
                                case bp_ulPortFlags:
                                    pE->sw.crossbar[crossbar_port].port_flags = pPhyId->u.ul;
                                    ++pPhyId;
                                    break;
                                case bp_usOamIndex:
                                    pE->sw.crossbar[crossbar_port].oamIndex = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_usGpioPhyReset:
                                    pE->sw.crossbar[crossbar_port].phyReset = pPhyId->u.us;
                                    ++pPhyId;
                                    break;
                                case bp_ulPortMaxRate:
                                    pE->sw.crossbar[crossbar_port].portMaxRate = pPhyId->u.ul;
                                    ++pPhyId;
                                    break;
                                case bp_ulCrossbar:
                                    crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(pPhyId->u.ul);
                                    pE->sw.crossbar[crossbar_port].switch_port = j;
                                    ++pPhyId;
                                    break;
                                default:
                                    if (IsParamPortHwLed( pPhyId->id ) != 0) {
                                        // skip over unknown port parameters that the enet driver doesn't need to know about
                                        ++pPhyId;
                                    } else {
                                        pPhyId = 0;
                                    }
                                    break;
                            }
                        }
                    }
                } else {
                    pE->sw.phy_id[j] = 0;
                }
            }
        }
        nRet = BP_SUCCESS;
    }

    return( nRet );

} /* GetEthernetMacInfo */
/**************************************************************************
 * Name       : BpGetEthernetMacInfoArrayPtr
 *
 * Description: This function returns the pointer to static EnetMacInfo structure.
 *
 * Parameters : 
 *
 * Returns    : PETHERNET_MAC_INFO - Pointer to EnetMacInfo.
 ***************************************************************************/
const ETHERNET_MAC_INFO* BpGetEthernetMacInfoArrayPtr( void )
{
    int nRet;
    if (pEnetMacInfo == NULL) {
        nRet = GetEthernetMacInfo(EnetMacInfoGbl,BP_MAX_ENET_MACS);
        if ( nRet != BP_SUCCESS)
        {
            printk("%s : Cannot get EnetMacInfo - Error <%d>\n",__FUNCTION__,nRet);
            return NULL;
        }
        pEnetMacInfo = EnetMacInfoGbl;
    }
    return pEnetMacInfo;
}
/**************************************************************************
 * Name       : BpGetEthernetMacInfo
 *
 * Description: This function returns all of the supported board id strings.
 *
 * Parameters : [OUT] pEnetInfos - Address of an array of ETHERNET_MAC_INFO
 *                  buffers.
 *              [IN] nNumEnetInfos - Number of ETHERNET_MAC_INFO elements that
 *                  are pointed to by pEnetInfos.
 *
 * Returns    : BP_SUCCESS - Success, value is returned.
 *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 ***************************************************************************/
int BpGetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos )
{
    const ETHERNET_MAC_INFO* pLocalEnetInfos = BpGetEthernetMacInfoArrayPtr();
    int nNumEnet=0;

    for (nNumEnet=0; nNumEnet < nNumEnetInfos; nNumEnet++) {
        pEnetInfos[nNumEnet] = pLocalEnetInfos[nNumEnet];
    }
    return BP_SUCCESS;
}

static int intToExtSwPort = -2;
int BpGetPortConnectedToExtSwitch(void)
{
    unsigned int phy_id, port_map;

    if (intToExtSwPort != -2)
    {
        return intToExtSwPort;
    }

    BpGetUl(bp_ulPortMap, &port_map);
    for (intToExtSwPort = 0; intToExtSwPort < BP_MAX_SWITCH_PORTS; intToExtSwPort++)
    {
        if ( port_map &  (1 << intToExtSwPort))
        {
            BpGetUl(bp_ulPhyId0 + intToExtSwPort, &phy_id);
            if (phy_id & EXTSW_CONNECTED)
            {
                break;
            }
        }
    }

    if (intToExtSwPort == BP_MAX_SWITCH_PORTS)
    {
        intToExtSwPort = -1;
    }

    return intToExtSwPort;
}

int BpGetAttachedInfo(int attached_port_idx, BP_ATTACHED_INFO *bp_attached_info)
{
    int j;
    bp_elem_t *pelem, *pAttached_info = NULL;
    enum bp_id bp_aulPhyId[BP_MAX_ATTACHED_PORTS+1] = {bp_ulPhyId0, bp_ulPhyId1,
        bp_ulPhyId2, bp_ulPhyId3, bp_ulPhyId4, bp_ulPhyId5, bp_ulPhyId6,
        bp_ulPhyId7, bp_ulPhyId8, bp_ulPhyId9, bp_ulPhyId10, bp_ulPhyId11,
        bp_ulPhyId12, bp_ulPhyId13, bp_ulPhyId14, bp_ulPhyId15, bp_last};

    if (!g_pCurrentBp || !bp_attached_info)
        return BP_BOARD_ID_NOT_SET;

    bp_attached_info->port_map = 0;

    while (1)
    {
        pelem = BpGetElem(bp_ulAttachedIdx, &pAttached_info, bp_ulAttachedIdx);
        if (bp_ulAttachedIdx != pelem->id)
            return BP_VALUE_NOT_DEFINED;

        /* So next iteration we won't match the last bp_ulAttachedIdx */
        pAttached_info++;
        if (pelem->u.ul != attached_port_idx)
            continue;

        for (j = 0; bp_aulPhyId[j] != bp_last; j++)
        {
            bp_elem_t *pPhyid = pAttached_info;
            enum bp_id bp_current_phyid = bp_aulPhyId[j];

            bp_attached_info->devnames[j] = PHY_DEVNAME_NOT_DEFINED;
            pelem = BpGetElem(bp_current_phyid, &pPhyid, bp_ulAttachedIdx);
            if (bp_current_phyid == pelem->id)
            {
                bp_attached_info->port_map |= 1 << j;
                bp_attached_info->ports[j] = pelem->u.ul;

                pelem++; /* Test if next entry has a devname */
                if (bp_ucPhyDevName == pelem->id)
                    bp_attached_info->devnames[j] = pelem->u.cp;
            }
        }

        return BP_SUCCESS;
    }

    return BP_VALUE_NOT_DEFINED;
}

/**************************************************************************
* Name       : BpGetMiiOverGpioFlag
*
* Description: This function returns logical disjunction of MII over GPIO
*              flag over all PHY IDs.
*
* Parameters : [OUT] pMiiOverGpioFlag - MII over GPIO flag
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
int BpGetMiiOverGpioFlag( unsigned int* pMiiOverGpioFlag )
{
    int i, j;
    bp_elem_t *pelem;
    bp_elem_t *pPhyType;
    bp_elem_t *pPhyId;
    int nRet;
    enum bp_id bp_aucPhyType[BP_MAX_ENET_MACS+1] = {bp_ucPhyType0, bp_ucPhyType1, bp_last};
    enum bp_id bp_current, bp_next;
    enum bp_id bp_aulPhyId[BP_MAX_SWITCH_PORTS+1] = PHYID_LIST;
    enum bp_id bp_current_phyid;

    *pMiiOverGpioFlag = 0;

    if( g_pCurrentBp ) {
        for( i = 0; i < BP_MAX_ENET_MACS; i++ ) {
            pPhyType = 0;
            bp_current = bp_aucPhyType[i];
            bp_next    = bp_aucPhyType[i+1];
            pelem = BpGetElem(bp_current, &pPhyType, bp_next);
            if (bp_current != pelem->id)
                continue;

            ++pPhyType;
            for( j = 0; j < BP_MAX_SWITCH_PORTS; j++ ) {
                pPhyId = pPhyType;
                bp_current_phyid = bp_aulPhyId[j];
                pelem = BpGetElem(bp_current_phyid, &pPhyId, bp_next);
                if (bp_current_phyid == pelem->id) {
                    *pMiiOverGpioFlag |= pelem->u.ul & MII_OVER_GPIO_VALID;                
                    ++pPhyId;
                }
            }
        }
        // Normalize flag value by positioning in lsb position        
        *pMiiOverGpioFlag >>= MII_OVER_GPIO_S;
        nRet = BP_SUCCESS;
    }
    else
    {
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );

} /* BpGetMiiOverGpioFlag */


/**************************************************************************
* Name       : BpGetGpio
*
* Description: Wrapper function to get the Gpio pin number
*
* Parameters : [IN] id       - id to search for
*              [IN] pusValue - unsigned short found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetGpio(enum bp_id id, unsigned short *pusValue )
{
    int nRet;

    nRet = BpGetUs(id, pusValue );
    if(nRet == BP_SUCCESS && *pusValue == BP_GPIO_NONE)
    {
    	*pusValue = BP_NOT_DEFINED;
    	nRet = BP_VALUE_NOT_DEFINED;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetExtIntrGpio
*
* Description: Wrapper function to get the Gpio pin number for shared external
*              interrupt
*
* Parameters : [IN] id       - id for the external interrupt. The bp_usGpio_Intr
*                              id must follow this id
*              [IN] pusValue - unsigned short found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int  BpGetExtIntrGpio(enum bp_id id, unsigned short *pusValue)
{
    bp_elem_t *pelem;
    int ret;
    bp_elem_t *start_elem = 0;

    /* First go to elem with interrupt id */
    pelem = BpGetElem(id,  &start_elem, bp_last);
    if (id != pelem->id)
    {
        *pusValue = BP_NOT_DEFINED;
        ret = BP_VALUE_NOT_DEFINED;
    }
    else
    {
        pelem++;
        /* We should expect the id bp_usGpio_Intr in the next element */
        if (bp_usGpio_Intr != pelem->id)
        {
            *pusValue = BP_NOT_DEFINED;
            ret = BP_VALUE_NOT_DEFINED;
        }
        else
        {
            *pusValue = pelem->u.us;
            ret = BP_SUCCESS;
        }
    }

    return ret;
}

/**************************************************************************
* Name       : BpGetLedPinMuxGpio
*
* Description: Wrapper function to get the Gpio pin number for LED parallel
*              connected to the LED controller
*
* Parameters : [IN] idx       - index of the LED in the led table
*               
*              [IN] pusValue - unsigned short found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int  BpGetLedPinMuxGpio(int idx, unsigned short *pusValue)
{
    bp_elem_t *pelem;
    int ret;
    bp_elem_t *start_elem = 0;
    enum bp_id id = bpLedList[idx];

    /* First go to elem with LED id */
    pelem = BpGetElem(id,  &start_elem, bp_last);
    if (id != pelem->id)
    {
        *pusValue = BP_NOT_DEFINED;
        ret = BP_VALUE_NOT_DEFINED;
    }
    else
    {
        pelem++;
        /* We should expect the id bp_usPinMux in the next element */
        if (bp_usPinMux != pelem->id)
        {
            *pusValue = BP_NOT_DEFINED;
            ret = BP_VALUE_NOT_DEFINED;
        }
        else
        {
            *pusValue = pelem->u.us;
            ret = BP_SUCCESS;
        }
    }

    return ret;
}


/**************************************************************************
* Name       : BpGetRj11InnerOuterPairGpios
*
* Description: This function returns the GPIO pin assignments for changing
*              between the RJ11 inner pair and RJ11 outer pair.
*
* Parameters : [OUT] pusInner - Address of short word that the RJ11 inner pair
*                  GPIO pin is returned in.
*              [OUT] pusOuter - Address of short word that the RJ11 outer pair
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetRj11InnerOuterPairGpios( unsigned short *pusInner,
                                 unsigned short *pusOuter )
{
    *pusInner = BP_NOT_DEFINED;
    *pusOuter = BP_NOT_DEFINED;

    return( BP_VALUE_NOT_DEFINED );
} /* BpGetRj11InnerOuterPairGpios */

/**************************************************************************
* Name       : BpGetUartRtsCtsGpios
*
* Description: This function returns the GPIO pin assignments for RTS and CTS
*              UART signals.
*
* Parameters : [OUT] pusRts - Address of short word that the UART RTS GPIO
*                  pin is returned in.
*              [OUT] pusCts - Address of short word that the UART CTS GPIO
*                  pin is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, board id input string does not
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetRtsCtsUartGpios( unsigned short *pusRts, unsigned short *pusCts )
{
    *pusRts = BP_NOT_DEFINED;
    *pusCts = BP_NOT_DEFINED;

    return( BP_VALUE_NOT_DEFINED );
} /* BpGetUartRtsCtsGpios */

/**************************************************************************
* Name       : BpGetAdslLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAdslLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkLedGpio(BP_INTF_TYPE_xDSL, 0, pusValue);
#else
    return( BpGetGpio(bp_usGpioLedAdsl, pusValue ) );
#endif
} /* BpGetAdslLedGpio */

/**************************************************************************
* Name       : BpGetAdslFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED that is used when there is a DSL connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAdslFailLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkFailLedGpio(BP_INTF_TYPE_xDSL, 0, pusValue);
#else
    return( BpGetGpio(bp_usGpioLedAdslFail, pusValue ) );
#endif
} /* BpGetAdslFailLedGpio */

/**************************************************************************
* Name       : BpGetSecAdslLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED of the Secondary line, applicable more for bonding.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSecAdslLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkLedGpio(BP_INTF_TYPE_xDSL, 1, pusValue);
#else
    return( BpGetGpio(bp_usGpioSecLedAdsl, pusValue ) );
#endif
} /* BpGetSecAdslLedGpio */

/**************************************************************************
* Name       : BpGetSecAdslFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED of the Secondary ADSL line, that is used when there is
*              a DSL connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSecAdslFailLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkFailLedGpio(BP_INTF_TYPE_xDSL, 1, pusValue);
#else
    return( BpGetGpio(bp_usGpioSecLedAdslFail, pusValue ) );
#endif
} /* BpGetSecAdslFailLedGpio */

/**************************************************************************
* Name       : BpGetPwrSyncGpio
*
* Description: This function returns the GPIO pin assignment for the Power Sync OE
*
* Parameters : [OUT] pusValue - Address of short word that the Power Sync
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPwrSyncGpio( unsigned short *pusValue )
{
   return (BpGetGpio(bp_usGpioPwrSync, pusValue ));
} /* BpGetPwrSyncGpio */

/**************************************************************************
* Name       : BpGetVregSyncGpio
*
* Description: This function returns the GPIO pin assignment for the VregSync Gpio
*
* Parameters : [OUT] pusValue - Address of short word that the VregSync
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVregSyncGpio( unsigned short *pusValue )
{
   return (BpGetGpio(bp_usVregSync, pusValue ));
} /* BpGetVregSyncGpio */


/**************************************************************************
* Name       : BpGetWirelessAntInUse
*
* Description: This function returns the antennas in use for wireless
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Antenna
*                  is in use.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessAntInUse( unsigned short *pusValue )
{
    return( BpGetUs(bp_usAntInUseWireless, pusValue ) );
} /* BpGetWirelessAntInUse */

/**************************************************************************
* Name       : BpGetWirelessFlags
*
* Description: This function returns optional control flags for wireless
*
* Parameters : [OUT] pusValue - Address of short word control flags
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessFlags( unsigned short *pusValue )
{
    return( BpGetUs(bp_usWirelessFlags, pusValue ) );
} /* BpGetWirelessAntInUse */

/**************************************************************************
* Name       : BpGetWirelessSesExtIntr
*
* Description: This function returns the external interrupt number for the
*              Wireless Ses Button.
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessSesExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrSesBtnWireless, pusValue ) );
} /* BpGetWirelessSesExtIntr */

/**************************************************************************
* Name       : BpGetWirelessSesExtIntrGpio
*
* Description: This function returns the external interrupt number for the
*              Wireless Ses Button.
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessSesExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrSesBtnWireless, pusValue);
    
    return( ret );  
} /* BpGetWirelessSesExtIntrGpio */


/**************************************************************************
* Name       : BpGetWirelessSesLedGpio
*
* Description: This function returns the GPIO pin assignment for the Wireless
*              Ses Led.
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
*                  Led GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessSesLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedSesWireless, pusValue ) );
} /* BpGetWirelessSesLedGpio */

/**************************************************************************
* Name       : BpGetNfcExtIntr
*
* Description: This function returns the external interrupt number for NFC
*
* Parameters : [OUT] pusValue - Address of short word that the NFC
*                  external interrupt is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetNfcExtIntr( unsigned short *pusValue )
{
    return BpGetUs( bp_usExtIntrNfc, pusValue );
} /* BpGetNfcExtIntr */

/**************************************************************************
* Name       : BpGetNfcPowerGpio
*
* Description: This function returns the GPIO assignment for Nfc Power
*
* Parameters : [OUT] pusValue - Address of short word that the Nfc Power
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetNfcPowerGpio( unsigned short *pusValue )
{
    return BpGetGpio( bp_usGpioNfcPower, pusValue );
} /* BpGetNfcPowerGpio */

/**************************************************************************
* Name       : BpGetNfcWakeGpio
*
* Description: This function returns the GPIO assignment for Nfc Wake
*
* Parameters : [OUT] pusValue - Address of short word that the Nfc Wake
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetNfcWakeGpio( unsigned short *pusValue )
{
    return BpGetGpio( bp_usGpioNfcWake, pusValue );
} /* BpGetNfcWakeGpio */

/**************************************************************************
* Name       : BpGetBitbangSclGpio
*
* Description: This function returns the GPIO assignment for Bitbang Scl
*
* Parameters : [OUT] pusValue - Address of short word that the Bitbang Scl
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBitbangSclGpio( unsigned short *pusValue )
{
    return BpGetGpio( bp_usGpioBitbangI2cScl, pusValue );
} /* BpGetBitbangSclGpio */

/**************************************************************************
* Name       : BpGetBitbangSdaGpio
*
* Description: This function returns the GPIO assignment for Bitbang Sda
*
* Parameters : [OUT] pusValue - Address of short word that the Bitbang Sda
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBitbangSdaGpio( unsigned short *pusValue )
{
    return BpGetGpio( bp_usGpioBitbangI2cSda, pusValue );
} /* BpGetBitbangSdaGpio */

/**************************************************************************
* Name       : BpGetBtResetGpio
*
* Description: This function returns the GPIO assignment for Bt Reset
*
* Parameters : [OUT] pusValue - Address of short word that the Bt Reset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBtResetGpio( unsigned short *pusValue )
{
    return BpGetGpio( bp_usGpioBtReset, pusValue );
} /* BpGetBtResetGpio */

/**************************************************************************
* Name       : BpGetBtWakeGpio
*
* Description: This function returns the GPIO assignment for Bt Wake
*
* Parameters : [OUT] pusValue - Address of short word that the Bt Wake
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBtWakeGpio( unsigned short *pusValue )
{
    return BpGetGpio( bp_usGpioBtWake, pusValue );
} /* BpGetBtWakeGpio */

#if !defined(_CFE_)
/**************************************************************************
* Name       : BpGetMocaInfo
*
* Description: This function returns all of information about the moca chips.
*
* Parameters : [OUT] pMocaInfos - Address of an array of BP_MOCA_INFO
*                  buffers.
*              [IN, OUT] pNumEntry - Set to the maximum Number of BP_MOCA_INFO
*              elements that are pointed to by pEnetInfos and return the actual
*              defined number of moca devices
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
int BpGetMocaInfo( PBP_MOCA_INFO pMocaInfos, int* pNumEntry )
{
    int i, j, k;
    bp_elem_t *pelem;
    bp_elem_t *pMocaType, *pIntId;
    int nRet = BP_BOARD_ID_NOT_SET;
    PBP_MOCA_INFO pMoca;
    enum bp_id bp_ausMocaType[BP_MOCA_MAX_NUM+1] = {bp_usMocaType0, bp_usMocaType1, bp_last};
    enum bp_id bp_current, bp_next;
    enum bp_id bp_ausIntId[BP_MOCA_MAX_INTR_NUM+1] = {bp_usExtIntrMocaHostIntr, bp_usExtIntrMocaSBIntr0, bp_usExtIntrMocaSBIntr1, bp_usExtIntrMocaSBIntr2,
                                                      bp_usExtIntrMocaSBIntr3, bp_usExtIntrMocaSBIntr4, bp_usExtIntrMocaSBIntrAll, bp_last};
    enum bp_id bp_current_intid;


    if( pMocaInfos == 0 || *pNumEntry == 0 )
    {
        *pNumEntry = 0;
        return nRet;
    }

    if( g_pCurrentBp ) {
        /* Populate it with what we have in the element array */
        for( i = 0, j = 0, pMoca = pMocaInfos; i < BP_MOCA_MAX_NUM; i++ ) {
            pMocaType = 0;
            bp_current = bp_ausMocaType[i];
            bp_next    = bp_ausMocaType[i+1];
            pMoca->type = BP_NOT_DEFINED;
            pelem = BpGetElem(bp_current, &pMocaType, bp_next);
            if (bp_current != pelem->id)
                continue;
            pMoca->type = pelem->u.us;
            ++pMocaType;

            pMoca->rfBand   = BpGetSubUs(bp_usMocaRfBand, pMocaType, bp_next);
            pMoca->initCmd  = BpGetSubPtr(bp_pMocaInit, pMocaType, bp_next);
            pMoca->spiDevInfo.resetGpio = BpGetSubUs(bp_usGpioSpiSlaveReset, pMocaType, bp_next);
            pMoca->spiDevInfo.bootModeGpio =  BpGetSubUs(bp_usGpioSpiSlaveBootMode, pMocaType, bp_next);
            pMoca->spiDevInfo.busNum = BpGetSubUs(bp_usSpiSlaveBusNum, pMocaType, bp_next);
            pMoca->spiDevInfo.select = BpGetSubUs(bp_usSpiSlaveSelectNum, pMocaType, bp_next);
            pMoca->spiDevInfo.mode = BpGetSubUs(bp_usSpiSlaveMode, pMocaType, bp_next);
            pMoca->spiDevInfo.ctrlState = BpGetSubUl(bp_ulSpiSlaveCtrlState, pMocaType, bp_next);
            pMoca->spiDevInfo.maxFreq = BpGetSubUl(bp_ulSpiSlaveMaxFreq, pMocaType, bp_next);

            for( k = 0; k < BP_MOCA_MAX_INTR_NUM; k++ ) {
                pIntId = pMocaType;
                bp_current_intid = bp_ausIntId[k];
                pelem = BpGetElem(bp_current_intid, &pIntId, bp_next);
                pMoca->intr[k] = BP_NOT_DEFINED;
                pMoca->intrGpio[k] = BP_NOT_DEFINED;
                if (bp_current_intid == pelem->id) {
                    pMoca->intr[k] = pelem->u.us;
                    ++pIntId;
                    if( pIntId->id == bp_usGpio_Intr )
                        pMoca->intrGpio[k] = pIntId->u.us;
                }
            }

            pMoca++;
            j++;
            if( j >= *pNumEntry )
            	break;
        }

        *pNumEntry = j;
        nRet = BP_SUCCESS;
    }
    return( nRet );

} /* BpGetMocaInfo */
#endif


/**************************************************************************
* Name       : BpUpdateWirelessSromMap
*
* Description: This function patch wireless PA values
*
* Parameters : [IN] unsigned short chipID
*              [IN/OUT] unsigned short* pBase - base of srom map
*              [IN/OUT] int size - size of srom map
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpUpdateWirelessSromMap(unsigned short chipID, unsigned short* pBase, int sizeInWords)
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    int i = 0;
    int j = 0;

    if(chipID == 0 || pBase == 0 || sizeInWords <= 0 )
        return nRet;

    i = 0;
    while ( wlanPaInfo[i].szboardId[0] != 0 ) {
        /* check boardId */
        if ( !bpstrcmp(g_pCurrentBp[0].u.cp, wlanPaInfo[i].szboardId) ) {
            /* check chipId */
            if ( (wlanPaInfo[i].usWirelessChipId == chipID) && (wlanPaInfo[i].usNeededSize <= sizeInWords) ){

                /* valid , patch common to multiple boards entry */
                while ( wlanPaInfo[i].commonEntries[j].wordOffset != 0) {
                    pBase[wlanPaInfo[i].commonEntries[j].wordOffset] = wlanPaInfo[i].commonEntries[j].value;
                    j++;
                }

                j = 0;
                /* valid , patch board specific entry */
                while ( wlanPaInfo[i].uniqueEntries[j].wordOffset != 0) {
                    pBase[wlanPaInfo[i].uniqueEntries[j].wordOffset] = wlanPaInfo[i].uniqueEntries[j].value;
                    j++;
                }

                nRet = BP_SUCCESS;
                goto srom_update_done;
            }
        }
        i++;
    }

srom_update_done:

    return( nRet );

} /* BpUpdateWirelessSromMap */

/**************************************************************************
* Name       : BpUpdateWirelessPciConfig
*
* Description: This function patch wireless PCI Config Header
*              This is not functional critial/necessary but for dvt database maintenance
*
* Parameters : [IN] unsigned int pciID
*              [IN/OUT] unsigned int* pBase - base of pci config header
*              [IN/OUT] int sizeInDWords - size of pci config header
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpUpdateWirelessPciConfig (unsigned int pciID, unsigned int* pBase, int sizeInDWords)
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    int i = 0;
    int j = 0;

    if(pciID == 0 || pBase == 0 || sizeInDWords <= 0 )
        return nRet;

    i = 0;
    while ( wlanPciInfo[i].szboardId[0] != 0 ) {
        /* check boardId */
        if ( !bpstrcmp(g_pCurrentBp[0].u.cp, wlanPciInfo[i].szboardId) ) {
            /* check pciId */
            if ( (wlanPciInfo[i].usWirelessPciId == pciID) && (wlanPciInfo[i].usNeededSize <= sizeInDWords) ){
                /* valid , patch entry */
                while ( wlanPciInfo[i].entries[j].name[0] ) {
                    pBase[wlanPciInfo[i].entries[j].dwordOffset] = wlanPciInfo[i].entries[j].value;
                    j++;
                }
                nRet = BP_SUCCESS;
                goto pciconfig_update_done;
            }
        }
        i++;
    }

pciconfig_update_done:

    return( nRet );

}

/**************************************************************************
* Name       : BpGetWanDataLedGpio
*
* Description: This function returns the GPIO pin assignment for the WAN Data
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the WAN Data LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanDataLedGpio(unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    int nRet;
    /* should use the new API. This compatible layer assumes first xDsl line if bp has xDSL 
     interface. Otherise, use first xPON inteface */
    if( (nRet = BpGetWanActLedGpio(BP_INTF_TYPE_xDSL, 0, pusValue)) == BP_SUCCESS ) 
        return nRet;

    if( (nRet = BpGetWanActLedGpio(BP_INTF_TYPE_xPON, 0, pusValue)) == BP_SUCCESS ) 
        return nRet;

     *pusValue = BP_NOT_DEFINED;
     return BP_VALUE_NOT_DEFINED;
#else
     return( BpGetGpio(bp_usGpioLedWanData, pusValue ) );
#endif
} /* BpGetWanDataLedGpio */

int BpGetSecWanDataLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    int nRet;
    /* should use the new API. This compatible layer assumes second xDsl line if bp has xDSL 
     interface. Otherise, use second xPON inteface */
    if( (nRet = BpGetWanActLedGpio(BP_INTF_TYPE_xDSL, 1, pusValue)) == BP_SUCCESS ) 
        return nRet;

    if( (nRet = BpGetWanActLedGpio(BP_INTF_TYPE_xPON, 1, pusValue)) == BP_SUCCESS ) 
        return nRet;

     *pusValue = BP_NOT_DEFINED;
     return BP_VALUE_NOT_DEFINED;
#else
    return( BpGetGpio(bp_usGpioSecLedWanData, pusValue ) );
#endif
} /* BpGetSecWanDataLedGpio */


/**************************************************************************
* Name       : BpGetWanErrorLedGpio
*
* Description: This function returns the GPIO pin assignment for the WAN
*              LED that is used when there is a WAN connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the WAN LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanErrorLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    int nRet;
    /* should use the new API. This compatible layer assumes first xDsl line if bp has xDSL 
     interface. Otherise, use first xPON inteface */
    if( (nRet = BpGetWanErrLedGpio(BP_INTF_TYPE_xDSL, 0, pusValue)) == BP_SUCCESS ) 
        return nRet;

    if( (nRet = BpGetWanErrLedGpio(BP_INTF_TYPE_xPON, 0, pusValue)) == BP_SUCCESS ) 
        return nRet;

     *pusValue = BP_NOT_DEFINED;
     return BP_VALUE_NOT_DEFINED;
#else
    return( BpGetGpio(bp_usGpioLedWanError, pusValue ) );
#endif
} /* BpGetWanErrorLedGpio */

/**************************************************************************
* Name       : BpGetBootloaderPowerOnLedGpio
*
* Description: This function returns the GPIO pin assignment for the power
*              on LED that is set by the bootloader.
*
* Parameters : [OUT] pusValue - Address of short word that the alarm LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBootloaderPowerOnLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedBlPowerOn, pusValue ) );
} /* BpGetBootloaderPowerOn */

/**************************************************************************
* Name       : BpGetBootloaderPowerOnLedBlinkTimeOn
*
* Description: This function returns the bp_ulLedBlPowerOnBlinkTimeOn
*              parameter if it's present.
*              This parameter defines the number of milliseconds the
*              PowerLED stays ON in each blink cycle.
*
* Parameters : [OUT] pulValue - Number of milliseconds of the PowerLED
*                               ON-cycle
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBootloaderPowerOnLedBlinkTimeOn( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulLedBlPowerOnBlinkTimeOn, pulValue ) );
}

/**************************************************************************
* Name       : BpGetBootloaderPowerOnLedBlinkTimeOff
*
* Description: This function returns the bp_ulLedBlPowerOnBlinkTimeOff
*              parameter if it's present.
*              This parameter defines the number of milliseconds the
*              PowerLED stays OFF in each blink cycle.
*
* Parameters : [OUT] pulValue - Number of milliseconds of the PowerLED
*                               OFF-cycle
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBootloaderPowerOnLedBlinkTimeOff( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulLedBlPowerOnBlinkTimeOff, pulValue ) );
}

/**************************************************************************
* Name       : BpGetBootloaderStopLedGpio
*
* Description: This function returns the GPIO pin assignment for the break
*              into bootloader LED that is set by the bootloader.
*
* Parameters : [OUT] pusValue - Address of short word that the break into
*                  bootloader LED GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBootloaderStopLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedBlStop, pusValue ) );
} /* BpGetBootloaderStopLedGpio */

/**************************************************************************
* Name       : BpGetVoipLedGpio
*
* Description: This function returns the GPIO pin assignment for the VOIP
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the VOIP LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
*
* Note       : The VoIP structure would allow for having one LED per DSP
*              however, the board initialization function assumes only one
*              LED per functionality (ie one LED for VoIP).  Therefore in
*              order to keep this tidy and simple we do not make usage of the
*              one-LED-per-DSP function.  Instead, we assume that the LED for
*              VoIP is unique and associated with DSP 0 (always present on
*              any VoIP platform).  If changing this to a LED-per-DSP function
*              then one need to update the board initialization driver in
*              bcmdrivers\opensource\char\board\bcm963xx\impl1
***************************************************************************/
int BpGetVoipLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedVoip, pusValue ) );
} /* BpGetVoipLedGpio */

/**************************************************************************
* Name       : BpGetVoip1LedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP1.
*              LED which is used when FXS0 is active
* Parameters : [OUT] pusValue - Address of short word that the VoIP1
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip1LedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioVoip1Led, pusValue ) );
} /* BpGetVoip1LedGpio */

/**************************************************************************
* Name       : BpGetVoip1FailLedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP1
*              Fail LED which is used when there's an error with FXS0
* Parameters : [OUT] pusValue - Address of short word that the VoIP1
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip1FailLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioVoip1LedFail, pusValue ) );
} /* BpGetVoip1FailLedGpio */

/**************************************************************************
* Name       : BpGetVoip2LedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP2.
*              LED which is used when FXS1 is active
* Parameters : [OUT] pusValue - Address of short word that the VoIP2
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip2LedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioVoip2Led, pusValue ) );
} /* BpGetVoip2LedGpio */

/**************************************************************************
* Name       : BpGetVoip2FailLedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP2
*              Fail LED which is used when there's an error with FXS1
* Parameters : [OUT] pusValue - Address of short word that the VoIP2
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip2FailLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioVoip2LedFail, pusValue ) );
} /* BpGetVoip2FailLedGpio */

/**************************************************************************
* Name       : BpGetPotsLedGpio
*
* Description: This function returns the GPIO pin assignment for the POTS1.
*              LED which is used when DAA is active
* Parameters : [OUT] pusValue - Address of short word that the POTS11
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPotsLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioPotsLed, pusValue ) );
} /* BpGetPotsLedGpio */

/**************************************************************************
* Name       : BpGetDectLedGpio
*
* Description: This function returns the GPIO pin assignment for the DECT.
*              LED which is used when DECT is active
* Parameters : [OUT] pusValue - Address of short word that the DECT
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetDectLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioDectLed, pusValue ) );
} /* BpGetDectLedGpio */

/**************************************************************************
* Name       : BpGetDyingGaspIntrPin
*
* Description: This function returns the intrerrupt pin used by dying gasp
* Parameters : [OUT] pusValue - Address of int word that the DECT
*                   pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetDyingGaspIntrPin( unsigned int *pusValue )
{
    return( BpGetUl(bp_ulDyingGaspIntrPin, pusValue ) );
} /* BpGetDyingGaspIntrPin */

/**************************************************************************
* Name       : BpGetPassDyingGaspGpio
*
* Description: This function returns the GPIO pin assignment used to pass
*                  a dying gasp interrupt to an external processor.
* Parameters : [OUT] pusValue - Address of short word that the DECT
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPassDyingGaspGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioPassDyingGasp, pusValue ) );
} /* BpGetPassDyingGaspGpio */

/**************************************************************************
* Name       : BpGetFpgaResetGpio
*
* Description: This function returns the GPIO pin assignment for the FPGA
*              Reset signal.
*
* Parameters : [OUT] pusValue - Address of short word that the FPGA Reset
*                  signal GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetFpgaResetGpio( unsigned short *pusValue ) {
    return( BpGetGpio(bp_usGpioFpgaReset, pusValue ) );
} /*BpGetFpgaResetGpio*/

/**************************************************************************
* Name       : BpGetGponLedGpio
*
* Description: This function returns the GPIO pin assignment for the GPON
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the GPON LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGponLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkLedGpio(BP_INTF_TYPE_xPON, 0, pusValue);
#else
    return( BpGetGpio(bp_usGpioLedGpon, pusValue ) );
#endif
} /* BpGetGponLedGpio */

/**************************************************************************
* Name       : BpGetGponFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the GPON
*              LED that is used when there is a GPON connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the GPON LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGponFailLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkFailLedGpio(BP_INTF_TYPE_xPON, 0, pusValue);
#else
    return( BpGetGpio(bp_usGpioLedGponFail, pusValue ) );
#endif
} /* BpGetGponFailLedGpio */



 /**************************************************************************
* Name	   : BpGetOpticalLinkFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the OpticalLink
*              Fail LED which is used when Receive power is Lower than the receive sensitivity.
* Parameters : [OUT] pusValue - Address of short word that the VoIP1
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
  ***************************************************************************/
int BpGetOpticalLinkFailLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkFailLedGpio(BP_INTF_TYPE_xPON, 0, pusValue); 
#else
    return( BpGetGpio(bp_usGpioLedOpticalLinkFail, pusValue ) );
#endif
} /* BpGetOpticalLinkFailLedGpio */



  /**************************************************************************
   * Name		: BpGetUSBLedGpio
   *
   * Description: This function returns the GPIO pin assignment for the USB
   *			  LED.
   *
   * Parameters : [OUT] pusValue - Address of short word that the USB LED
   *				  GPIO pin is returned in.
   *
   * Returns	: BP_SUCCESS - Success, value is returned.
   *			  BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
   *			  BP_VALUE_NOT_DEFINED - At least one return value is not defined
   *				  for the board.
   ***************************************************************************/
   int BpGetUSBLedGpio( unsigned short *pusValue )
   {
	   return( BpGetGpio(bp_usGpioLedUSB, pusValue ) );
   } /* BpGetUSBLedGpio */



/**************************************************************************
* Name       : BpGetMoCALedGpio
*
* Description: This function returns the GPIO pin assignment for the MoCA
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the MoCA LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMoCALedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedMoCA, pusValue ) );
} /* BpGetMoCALedGpio */

/**************************************************************************
* Name       : BpGetMoCAFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the MoCA
*              LED that is used when there is a MoCA connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the MoCA LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMoCAFailLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedMoCAFail, pusValue ) );
} /* BpGetMoCAFailLedGpio */

/**************************************************************************
* Name       : BpGetEponLedGpio
*
* Description: This function returns the GPIO pin assignment for the Epon
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the Epon LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetEponLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkLedGpio(BP_INTF_TYPE_xPON, 0, pusValue);
#else
    return( BpGetGpio(bp_usGpioLedEpon, pusValue ) );
#endif
} /* BpGetMoCALedGpio */

/**************************************************************************
* Name       : BpGetEponFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the Epon Fail
*              LED that is used when there is a MoCA connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the Epon Fail LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetEponFailLedGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetWanLinkFailLedGpio(BP_INTF_TYPE_xPON, 0, pusValue);
#else
    return( BpGetGpio(bp_usGpioLedEponFail, pusValue ) );
#endif
} /* BpGetMoCAFailLedGpio */

int BpGetSesBtnWirelessExtIntr(void)
{
     unsigned short pusValue;

     return( BpGetUs(bp_usExtIntrSesBtnWireless, &pusValue ) );
} /* BpGetSesBtnWirelessExtIntr */

int BpGetSesBtnWirelessExtIntrGpio( unsigned short *pusValue )
{
     int ret;

     if( !g_pCurrentBp )
     {
         *pusValue = BP_NOT_DEFINED;
         ret       = BP_BOARD_ID_NOT_SET;
         return ret;
     }

     ret = BpGetExtIntrGpio(bp_usExtIntrSesBtnWireless, pusValue);

     return( ret );

} /* BpGetSesBtnWirelessExtIntrGpio */

/**************************************************************************
* Name       : BpGetAggregateLnkLedGpio
*
* Description: This function returns the GPIO pin assignment for aggregate
*              Link LED.
*
* Parameters : [OUT] pusValue - Address of short word that the aggregate link
*              LED GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAggregateLnkLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedAggregateLnk, pusValue ) );
} /* BpGetAggregateLnkLedGpio */

/**************************************************************************
* Name       : BpGetAggregateActLedGpio
*
* Description: This function returns the GPIO pin assignment for aggregate
*              activity LED.
*
* Parameters : [OUT] pusValue - Address of short word that the aggregate activity
*              LED GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAggregateActLedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedAggregateAct, pusValue ) );
} /* BpGetAggregateActLedGpio */

/**************************************************************************
* Name       : BpGetResetToDefaultExtIntr
*
* Description: This function returns the external interrupt number for the
*              reset to default button.
*
* Parameters : [OUT] pusValue - Address of short word that reset to default
*                  external interrupt is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetResetToDefaultExtIntr( unsigned short *pusValue )
{
    
#if defined(_CFE_)
        {
            int ret;
            unsigned short btnIdx, idx, gpio, extIrq;                
            ret = BpGetUs(bp_usCfeResetToDefaultBtnIdx, &btnIdx);
            if (ret == BP_SUCCESS) {
                ret=BpGetButtonInfoByIdx(btnIdx, &idx, &gpio, &extIrq, NULL, NULL, NULL);
                if (ret == BP_SUCCESS) {
                    *pusValue = extIrq;
                    return BP_SUCCESS;
                }
            }
        }
#endif

    return( BpGetUs(bp_usExtIntrResetToDefault, pusValue ) );
} /* BpGetResetToDefaultExtIntr */

/**************************************************************************
* Name       : BpGetResetToDefaultExtIntrGpio
*
* Description: This function returns the external interrupt number for the
*              reset to default button.
*
* Parameters : [OUT] pusValue - Address of short word that reset to default
*                  gpio is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetResetToDefaultExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }

#if defined(_CFE_)
    {
        int ret;
        unsigned short btnIdx;
        unsigned short idx, gpio, extIrq;                
        ret = BpGetUs(bp_usCfeResetToDefaultBtnIdx, &btnIdx);
        if (ret == BP_SUCCESS) {
            ret=BpGetButtonInfoByIdx(btnIdx, &idx, &gpio, &extIrq, NULL, NULL, NULL);
            if (ret == BP_SUCCESS) {
                *pusValue = gpio;
                return BP_SUCCESS;
            }
        }
    }
#endif

    ret = BpGetExtIntrGpio(bp_usExtIntrResetToDefault, pusValue);
    
    return( ret );  

} /* BpGetResetToDefaultExtIntrGpio */

/**************************************************************************
* Name       : BpGetResetToDefault2ExtIntr
*
* Description: This function returns the external interrupt number for the
*              second reset to default button.
*
* Parameters : [OUT] pusValue - Address of short word that reset to default
*                  external interrupt is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetResetToDefault2ExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrResetToDefault2, pusValue ) );
} /* BpGetResetToDefaultExtIntr */

/**************************************************************************
* Name       : BpGetResetToDefault2ExtIntrGpio
*
* Description: This function returns the external interrupt number for the
*              second reset to default button.
*
* Parameters : [OUT] pusValue - Address of short word that reset to default
*                  external interrupt is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetResetToDefault2ExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrResetToDefault2, pusValue);
    
    return( ret );  

} /* BpGetResetToDefaultExtIntrGpio */


  
/**************************************************************************
* Name       : BpGetOpticalModulePresenceExtIntrGpio
*
* Description: This function returns the external interrupt number for the
*              Optical module Presence
*
* Parameters : [OUT] pusValue - Address of short word that reset to default
*                  external interrupt is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
#ifdef CONFIG_BP_PHYS_INTF
int BpGetOpticalModulePresenceExtIntrGpio( unsigned short type, int intfIdx, unsigned short *pusValue )
{
    int ret;
    PHYS_INTF_INFO*  pIntf = NULL;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }
    
    ret = BpGetExtIntrGpioInIntf(bp_usExtIntrOpticalModulePresence, pIntf, pusValue);
    
    return( ret );  

} /* BpGetOpticalModulePresenceExtIntrGpio */


int BpGetOpticalModulePresenceExtIntr( unsigned short type, int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }
    
    return BpGetUsInIntf(bp_usExtIntrOpticalModulePresence, pusValue, pIntf);
} /* BpGetOpticalModulePresenceExtIntr */

#else
int BpGetOpticalModulePresenceExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrOpticalModulePresence, pusValue);
    
    return( ret );  

} /* BpGetOpticalModulePresenceExtIntrGpio */


int BpGetOpticalModulePresenceExtIntr( unsigned short *pusValue )
{
    return BpGetUs(bp_usExtIntrOpticalModulePresence, pusValue);
} /* BpGetOpticalModulePresenceExtIntr */
#endif

int BpGetSDCardDetectExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrSDCardDetect, pusValue);
    
    return( ret );  

} /* BpGetSDCardDetectExtIntrGpio */


int BpGetSDCardDetectExtIntr( unsigned short *pusValue )
{
    return BpGetUs(bp_usExtIntrSDCardDetect, pusValue);
} /* BpGetSDCardDetectExtIntr */

int BpGetI2cMuxAddr( unsigned short *pusValue )
{
    return ( BpGetUs (bp_usI2cmuxAddr, pusValue));
}

/**************************************************************************
* Name       : BpGetOpticalModuleFixupGpio
*
* Description: This function returns the optical module fixup gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetOpticalModuleFixupGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioOpticalModuleFixup, pusValue ) );
} /* BpGetOpticalModuleFixupGpio */



/**************************************************************************
* Name       : BpGetWirelessPowerDownGpio
*
* Description: This function returns the GPIO pin assignment for WLAN_PD
*
*
* Parameters : [OUT] pusValue - Address of short word that the WLAN_PD
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessPowerDownGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioWirelessPowerDown, pusValue ) );
} /* usGpioWirelessPowerDown */

/**************************************************************************
* Name       : BpGetDslPhyAfeIdByIntfIdx
*
* Description: This function returns the DSL PHY AFE ids for primary and
*              secondary PHYs.
*
* Parameters : [INPUT] Interface number
i              [OUT] pulValues-Address of an array of one int word where
*              AFE Id for associated interface proivded in INPUT
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET-Error, BpSetBoardId hasn't been called.
*              BP_VALUE_NOT_DEFINED - No defined AFE Ids.
**************************************************************************/
int BpGetDslPhyAfeIdByIntfIdx( int intfIdx, unsigned int *pulValue )
{
    int nRet = BP_BOARD_ID_NOT_SET;
    if( g_pCurrentBp )
    {
#ifdef CONFIG_BP_PHYS_INTF
        PHYS_INTF_INFO*  pDSLIntf = NULL;

        if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
            *pulValue = BP_AFE_DEFAULT;
            return BP_VALUE_NOT_DEFINED;
        }
        if (BpGetUlInIntf(bp_ulAfeId, pulValue, pDSLIntf) != BP_SUCCESS) {
            *pulValue = BP_AFE_DEFAULT;
        }
        nRet = BP_SUCCESS;
        
#endif
    }
return nRet;
}


/**************************************************************************
* Name       : BpGetDslPhyAfeIds
*
* Description: This function returns the DSL PHY AFE ids for primary and
*              secondary PHYs.
*
* Parameters : [OUT] pulValues-Address of an array of two int words where
*              AFE Id for the primary and secondary PHYs are returned.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET-Error, BpSetBoardId hasn't been called.
*              BP_VALUE_NOT_DEFINED - No defined AFE Ids.
**************************************************************************/
int BpGetDslPhyAfeIds( unsigned int *pulValues )
{
    int nRet;

    if( g_pCurrentBp )
    {
#ifdef CONFIG_BP_PHYS_INTF
        PHYS_INTF_INFO* pDSLIntf = NULL;
        int i;

        for( i = 0 ; i < 2; i++ ) {
            pDSLIntf = NULL;
            pulValues[i] = BP_AFE_DEFAULT;
            if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, i)) ) {
                if (BpGetUlInIntf(bp_ulAfeId, &pulValues[i], pDSLIntf) != BP_SUCCESS) {
                    pulValues[i] = BP_AFE_DEFAULT;
                }
            }
        }
#else
        if (BpGetUl(bp_ulAfeId0, &pulValues[0]) != BP_SUCCESS) {
          pulValues[0] = BP_AFE_DEFAULT;
        }
        if (BpGetUl(bp_ulAfeId1, &pulValues[1]) != BP_SUCCESS) {
          pulValues[1] = BP_AFE_DEFAULT;
        }
#endif
        nRet = BP_SUCCESS;
    }
    else
    {
        pulValues[0] = BP_AFE_DEFAULT;
        pulValues[1] = BP_AFE_DEFAULT;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetDslPhyAfeIds */

/**************************************************************************
* Name       : BpGetUart2SdoutGpio
*
* Description: This function returns the GPIO pin assignment for UART2 SDOUT
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioUart2Sdout
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetUart2SdoutGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioUart2Sdout, pusValue ) );
} /* BpGetUart2SdoutGpio */

/**************************************************************************
* Name       : BpGetUart2SdinGpio
*
* Description: This function returns the GPIO pin assignment for UART2 SDIN
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioUart2Sdin
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetUart2SdinGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioUart2Sdin, pusValue ) );
} /* BpGetUart2SdinGpio */

/**************************************************************************
* Name       : BpGetAFELDPwrBoostGpio
*
* Description: This function returns the GPIO pin assignment for changing the AFE LD power
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDPwrBoost
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
#ifdef CONFIG_BP_PHYS_INTF
int BpGetAFELDPwrBoostGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFELDPwrBoost, pDSLIntf, pusValue ) );
} /* BpGetAFELDPwrBoostGpio */
#else
int BpGetAFELDPwrBoostGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioAFELDPwrBoost, pusValue ) );
} /* BpGetAFELDPwrBoostGpio */
#endif
/**************************************************************************
* Name       : BpGetExtAFEResetGpio
*
* Description: This function returns the GPIO pin assignment for resetting the external AFE chip
*
*
* Parameters : [OUT] pusValue - Address of short word that the ExtAFEReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFEResetGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFEResetGpio(1, pusValue);
#else
    return( BpGetGpio(bp_usGpioExtAFEReset, pusValue ) );
#endif
} /* BpGetExtAFEResetGpio */

/**************************************************************************
* Name       : BpGetAFELDRelayGpio
*
* Description: This function returns the GPIO pin assignment for switching LD relay
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDRelay
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
#ifdef CONFIG_BP_PHYS_INTF
int BpGetAFELDRelayGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFELDRelay, pDSLIntf, pusValue ) );
} /* BpGetAFELDRelayGpio */
#else
int BpGetAFELDRelayGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioAFELDRelay, pusValue ) );
} /* BpGetAFELDRelayGpio */
#endif
/**************************************************************************
* Name       : BpGetIntAFELDModeGpio
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*                  for the internal path.
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioIntAFELDMode
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetIntAFELDModeGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFELDModeGpio(0, pusValue );
#else
    return( BpGetGpio(bp_usGpioIntAFELDMode, pusValue ) );
#endif
} /* BpGetIntAFELDModeGpio */

/**************************************************************************
* Name       : BpGetIntAFELDPwrGpio
*
* Description: This function returns the GPIO pin assignment for turning on/off the internal AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDPwr
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetIntAFELDPwrGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFELDPwrGpio(0, pusValue );
#else
    return( BpGetGpio(bp_usGpioIntAFELDPwr, pusValue ) );
#endif
} /* BpGetIntAFELDPwrGpio */

/**************************************************************************
* Name       : BpGetExtAFELDModeGpio
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*                  for the external path.
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDMode
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDModeGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFELDModeGpio(1, pusValue );
#else
    return( BpGetGpio(bp_usGpioExtAFELDMode, pusValue ) );
#endif
} /* BpGetExtAFELDModeGpio */

/**************************************************************************
* Name       : BpGetExtAFELDPwrGpio
*
* Description: This function returns the GPIO pin assignment for turning on/off the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDPwr
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDPwrGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFELDPwrGpio(1, pusValue );
#else
    return( BpGetGpio(bp_usGpioExtAFELDPwr, pusValue ) );
#endif
} /* BpGetExtAFELDPwrGpio */

/**************************************************************************
* Name       : BpGetExtAFELDDataGpio
*
* Description: This function returns the GPIO pin assignment for sending config data to the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioExtAFELDData
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDDataGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFELDDataGpio(1, pusValue );
#else
    return( BpGetGpio(bp_usGpioExtAFELDData, pusValue ) );
#endif
} /* BpGetExtAFELDDataGpio */

/**************************************************************************
* Name       : BpGetExtAFELDClkGpio
*
* Description: This function returns the GPIO pin assignment for sending the clk to the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioExtAFELDClk
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetExtAFELDClkGpio( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return BpGetAFELDClkGpio(1, pusValue );
#else
    return( BpGetGpio(bp_usGpioExtAFELDClk, pusValue ) );
#endif
} /* BpGetExtAFELDClkGpio */

/**************************************************************************
* Name       : BpGetAFEVR5P3PwrEnGpio
*
* Description: This function returns the GPIO pin assignment for 5.3V Voltage 
*              regulator enable signal for some line driver chip
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFEVR5P3PwrEn 
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
#ifdef CONFIG_BP_PHYS_INTF
int BpGetAFEVR5P3PwrEnGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;
  
    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFEVR5P3PwrEn, pDSLIntf, pusValue ) );
}
#else
int BpGetAFEVR5P3PwrEnGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioAFEVR5P3PwrEn, pusValue ) );
} /* BpGetAFEVR5P3PwrEnGpio */
#endif
/**************************************************************************
* Name       : BpGet6829PortInfo
*
* Description: This function checks the ENET MAC info to see if a 6829
*              is connected
*
* Parameters : [OUT] portInfo6829 - 0 if 6829 is not present
*                                 - 6829 port information otherwise
*
* Returns    : BP_SUCCESS           - Success, value is returned.
*              BP_BOARD_ID_NOT_SET  - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGet6829PortInfo( unsigned char *portInfo6829 )
{
   const ETHERNET_MAC_INFO *enetMacInfo;
   const ETHERNET_SW_INFO *pSwInfo;
   int               retVal;
   int               i;

   *portInfo6829 = 0;
   enetMacInfo = BpGetEthernetMacInfoArrayPtr();
   if ( NULL != enetMacInfo ) {
      pSwInfo = &(enetMacInfo[0].sw);
      for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
         if ( ((pSwInfo->phy_id[i] & PHYID_LSBYTE_M) != 0xFF) &&
              ((pSwInfo->phy_id[i] & PHYID_LSBYTE_M) &  0x80) ) {
            *portInfo6829 = pSwInfo->phy_id[i] & PHYID_LSBYTE_M;
            retVal        = BP_SUCCESS;
            break;
         }
      }
   }

   return retVal;

}

/**************************************************************************
* Name       : BpGetEthSpdLedGpio
*
* Description: This function returns the GPIO pin assignment for the
*              specified port and link speed 
*
* Parameters : [IN] port - Internal phy number
*              [IN] enetIdx - index for Ethernet MAC info
*              [IN] ledIdx - 0 -> duplex GPIO
*                          - 1 -> spd 100 GPIO
*                          - 2 -> spd 1000 GPIO
*              [OUT] pusValue - Address of a short word to store the GPIO
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetEthSpdLedGpio( unsigned short port, unsigned short enetIdx,
                        unsigned short ledIdx, unsigned short *pusValue )
{
    const ETHERNET_MAC_INFO *enetMacInfos;
    const unsigned short *pShort;
    int nRet;

    if( g_pCurrentBp ) {
        enetMacInfos = BpGetEthernetMacInfoArrayPtr();

        if ((enetIdx >= BP_MAX_ENET_MACS) ||
            (port >= BP_MAX_ENET_INTERNAL) ||
            (enetMacInfos[enetIdx].ucPhyType == BP_ENET_NO_PHY)) {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_VALUE_NOT_DEFINED;
        } else {
           pShort   = &enetMacInfos[enetIdx].sw.ledInfo[port].duplexLed;
           pShort   += ledIdx;
           *pusValue = *pShort;
           if( *pShort == BP_NOT_DEFINED ) {
               nRet = BP_VALUE_NOT_DEFINED;
           } else {
               nRet = BP_SUCCESS;
           }
        }
    } else {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetEthSpdLedGpio */


/**************************************************************************
* Name       : BpGetLaserDisGpio
*
* Description: This function returns the GPIO pin assignment for disabling
*              the laser
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioLaserDis
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetLaserDisGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLaserDis, pusValue ) );
} /* BpGetLaserDisGpio */


/**************************************************************************
* Name       : BpGetLaserTxPwrEnGpio
*
* Description: This function returns the GPIO pin assignment for enabling
*              the transmit power of the laser
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioLaserTxPwrEn
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetLaserTxPwrEnGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLaserTxPwrEn, pusValue ) );
} /* BpGetLaserTxPwrEnGpio */

/**************************************************************************
* Name       : BpGetLaserResetGpio
*
* Description: This function returns the GPIO pin assignment for resetting
* the optical module
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioLaserReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetLaserResetGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLaserReset, pusValue ) );
} /* BpGetLaserResetGpio */

/**************************************************************************
* Name       : BpGetEponOpticalSDGpio
*
* Description: This function returns the GPIO pin assignment for Epon optical
* signal dectect
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioEponOpticalSD
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetEponOpticalSDGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioEponOpticalSD, pusValue ) );
} /* BpGetLaserResetGpio */


/**************************************************************************
* Name       : BpGetPLCPwrEnGpio
*
* Description: This function returns the GPIO pin assignment for PLC Power enable pin
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioPLCPwrEn
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetPLCPwrEnGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioPLCPwrEn, pusValue ) );
} /* BpGetPLCPwrEnGpio */

/**************************************************************************
* Name       : BpGetPLCResetGpio
*
* Description: This function returns the GPIO pin assignment for PLC reset pin
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioPLCReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetPLCResetGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioPLCReset, pusValue ) );
} /* BpGetPLCResetGpio */

/**************************************************************************
* Name       : BpGetPLCStandByExtIntr
*
* Description: This function returns the external interrupt number for the
*              PLC Standby Button.
*
* Parameters : [OUT] pusValue - Address of short word that the PLC Standby
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPLCStandByExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrPLCStandBy, pusValue ) );
} /* BpGetWirelessSesExtIntr */

/**************************************************************************
* Name       : BpGetVregSel1P2
*
* Description: This function returns the desired voltage level for 1V2
*
* Parameters : [OUT] pusValue - Address of short word that the 1V2 level
*                  is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetVregSel1P2( unsigned short *pusValue )
{
    return( BpGetUs(bp_usVregSel1P2, pusValue ) );
} /* BpGetVregSel1P2 */

/**************************************************************************
* Name       : BpGetVreg1P8
*
* Description: This function returns the desired state for 1P8 regulator
*
* Parameters : [OUT] pucValue - Address of unsigned char where to return
*                               whether 1P8 regulator is external or not
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetVreg1P8( unsigned char *pucValue )
{
    return( BpGetUc(bp_ucVreg1P8, pucValue ) );
} /* BpGetVreg1P8 */

/**************************************************************************
* Name       : BpGetVregAvsMin
*
* Description: This function returns the desired min voltage level for AVS
*
* Parameters : [OUT] pusValue - Address of short word that the min voltage
*                  is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetVregAvsMin( unsigned short *pusValue )
{
    return( BpGetUs(bp_usVregAvsMin, pusValue ) );
} /* BpGetVregAvsMin */

#if !defined(_CFE_)
/**************************************************************************
* Name       : BpGetI2cGpios
*
* Description: This function returns the GPIO pin assignments for the I2C
*              clock (SCL) and data (SDA).
*
* Parameters : [OUT] pusScl - Address of short word that the SCL GPIO pin
*                  is returned in.
*              [OUT] pusSda - Address of short word that the SDA GPIO pin
*                  pin is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetI2cGpios( unsigned short *pusScl, unsigned short *pusSda )
{
    int ret;

    *pusScl = *pusSda = BP_NOT_DEFINED;
    if( (ret = BpGetGpio (bp_usGpioI2cScl, pusScl)) == BP_SUCCESS )
        ret = BpGetGpio (bp_usGpioI2cSda, pusSda);
    
    return(ret);
} /* BpGetRj11InnerOuterPairGpios */

#endif


/**************************************************************************
* Name       : BpGetFemtoResetGpio
*
* Description: This function returns the GPIO that needs to be toggled high
*              for 2 msec at least to reset the FEMTO chip
*
* Parameters : [OUT] pusValue - Address of short word that the GPIO for
*                  resetting FEMTO chip is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetFemtoResetGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioFemtoReset, pusValue ) );
} /*  BpGetFemtoResetGpio */



/**************************************************************************
* Name       : BpGetEphyBaseAddress
*
* Description: This function returns the base address requested for
*              the internal EPHYs
*
* Parameters : [OUT] pusValue - Address of short word for returned value.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetEphyBaseAddress( unsigned short *pusValue )
{
    return( BpGetUs(bp_usEphyBaseAddress, pusValue ) );
} /*  BpGetEphyBaseAddress */


/**************************************************************************
* Name       : BpGetGphyBaseAddress
*
* Description: This function returns the base address requested for
*              the internal GPHYs
*
* Parameters : [OUT] pusValue - Address of short word for returned value.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetGphyBaseAddress( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGphyBaseAddress, pusValue ) );
} /*  BpGetGphyBaseAddress */

/**************************************************************************
* Name       : BpGetSpiSlaveResetGpio
*
* Description: This function returns the GPIO pin assignment for the resetting the 
*              the SPI slave.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave reset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveResetGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpiSlaveReset, pusValue ) );
} /* BpGetSpiSlaveResetGpio */

/**************************************************************************
* Name       : BpGetSpiSlaveBootModeGpio
*
* Description: This function returns the GPIO pin assignment for setting the
*              boot mode for the SPI slave.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave boot
*                  mode GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveBootModeGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpiSlaveBootMode, pusValue ) );
} /* BpGetSpiSlaveBootModeGpio */

/**************************************************************************
* Name       : BpGetSpiSlaveBusNum
*
* Description: This function returns the bus number of the SPI slave device.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave select number
*                    is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveBusNum( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveBusNum, pusValue ) );
} /* BpGetSpiSlaveBusNum */

/**************************************************************************
* Name       : BpGetSpiSlaveSelectNum
*
* Description: This function returns the SPI slave select number connected  
*              to the slave device.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave select number
*                    is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveSelectNum( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveSelectNum, pusValue ) );
} /* BpGetSpiSlaveSelectNum */

/**************************************************************************
* Name       : BpGetSpiSlaveMode
*
* Description: This function returns the SPI slave select number connected  
*              to the slave device.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave mode is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveMode( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveMode, pusValue ) );
} /* BpGetSpiSlaveMode */

/**************************************************************************
* Name       : BpGetSpiSlaveCtrlState
*
* Description: This function returns the spi controller state that is needed to talk
*              to the spi slave device.
*
* Parameters : [OUT] pusValue - Address of int word that the spi controller state is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveCtrlState( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulSpiSlaveCtrlState, pulValue ) );
} /* BpGetSpiSlaveCtrlState */

/**************************************************************************
* Name       : BpGetSpiSlaveMaxFreq
*
* Description: This function returns the SPI slaves max frequency for communication.
*
* Parameters : [OUT] pusValue - Address of int word that the max freq is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveMaxFreq( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulSpiSlaveMaxFreq, pulValue ) );
} /* BpGetSpiSlaveMaxFreq */

/**************************************************************************
* Name       : BpGetSpiSlaveProtoRev
*
* Description: This function returns the protocol revision that the slave device uses.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave protocol revision
                                is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveProtoRev( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveProtoRev, pusValue ) );
} /* BpGetSpiSlaveProtoRev */

/**************************************************************************
* Name       : BpGetSerialLEDMuxSel
*
* Description: This function returns the serial LED Mux selection.
*
* Parameters : [OUT] pusValue - Address of short word that the serial LED MUX Selection
                                is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSerialLEDMuxSel( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSerialLEDMuxSel, pusValue ) );
} /* BpGetSerialLEDMuxSel */

/**************************************************************************
* Name       : BpGetSwitchPortMap
*
* Description: This function returns the switch port map.
*
* Parameters : [OUT] pulValue - Bitmap of switch ports enabled.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - bp_ulPortMap is not defined
*                  for the board.
***************************************************************************/
int BpGetSwitchPortMap (unsigned int *pulValue)
{
    *pulValue = BpGetSubUl(bp_ulPortMap, 0, bp_last);
    if (*pulValue == BP_NOT_DEFINED ){
        *pulValue = 0;
        return BP_VALUE_NOT_DEFINED;
    } else {
        return BP_SUCCESS;
    }
}

/**************************************************************************
* Name       : BpGetDeviceOptions
*
* Description: This function returns the serial LED Mux selection.
*
* Parameters : [OUT] pulValue - Address of word that device options bitmap
                                is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetDeviceOptions( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulDeviceOptions, pulValue ) );
} /* BpGetDeviceOptions */

#if defined(CONFIG_EPON_SDK)
/**************************************************************************
* Name       : BpGetPortMacType
*
* Description: This function returns the mac type of uni port
*
* Parameters : [IN ] port - port index which we are interested
*              [OUT] pulValue - return mac type of the uni port
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPortMacType(unsigned short port, unsigned int *pulValue )
{
    unsigned int phy_id, port_map, mac_type;
 
    BpGetUl(bp_ulPortMap, &port_map);

    if((port < BP_MAX_SWITCH_PORTS) &&
       (port_map & (1 << port)))                 
    {
        BpGetUl(bp_ulPhyId0 + port, &phy_id);
        mac_type = phy_id & MAC_IFACE;
        *pulValue = mac_type;
        return BP_SUCCESS;
    }
    return BP_VALUE_NOT_DEFINED;
} /* BpGetPortMacType */

/**************************************************************************
* Name       : BpGetNumFePorts
*
* Description: This function returns the number of FE ports
*              Please note the code assumes any phy not marked as GMII will be 
*              treated as FE ports. 
* Parameters : [OUT] pulValue - number of FE UNI ports in this board design
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetNumFePorts( unsigned int *pulValue )
{
    unsigned int phy_id, port_map, mac_type;
    int i = 0, fe_ports = 0;

    BpGetUl(bp_ulPortMap, &port_map);
    
    for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
        if ( port_map & (1 << i)) {
            BpGetUl(bp_ulPhyId0 + i, &phy_id);
            if(phy_id & BCM_WAN_PORT)
                continue;
            mac_type = phy_id & MAC_IFACE;
            switch(mac_type)
            {
                case MAC_IF_GMII:
                case MAC_IF_RGMII:
                case MAC_IF_RGMII_3P3V:
                case MAC_IF_QSGMII:
                case MAC_IF_SGMII:    
                case MAC_IF_HSGMII:
                case MAC_IF_XFI:
                     break;

                default:
                     fe_ports++;
                     break;
            }
        }
    }
    *pulValue = fe_ports;
    return BP_SUCCESS;
} /* BpGetNumFePorts */

/**************************************************************************
* Name       : BpGetNumGePorts
*
* Description: This function returns the number of GE ports
*              Please note the code assumes phy_id to be marked as MAC GMII
*              for GE ports. 
* Parameters : [OUT] pulValue - number of GE UNI ports in this board design
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetNumGePorts( unsigned int *pulValue )
{
    unsigned int phy_id, port_map, mac_type;
    int i = 0, ge_ports = 0;

    BpGetUl(bp_ulPortMap, &port_map);
    
    for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
        if ( port_map & (1 << i)) {
            BpGetUl(bp_ulPhyId0 + i, &phy_id);
            if(phy_id & BCM_WAN_PORT)
                continue;
            mac_type = phy_id & MAC_IFACE;
            switch(mac_type)
            {
                case MAC_IF_GMII:
                case MAC_IF_RGMII:
                case MAC_IF_RGMII_3P3V:
                case MAC_IF_QSGMII:
                case MAC_IF_SGMII:    
                case MAC_IF_HSGMII:
                case MAC_IF_XFI:
                     ge_ports++;
                     break;

                default:
                     break;
            }
        }
    }
    *pulValue = ge_ports;
    return BP_SUCCESS;
} /* BpGetNumGePorts */

/**************************************************************************
* Name       : BpGetNumVoipPorts
*
* Description: This function returns the number of VOIP ports
*
* Parameters : [OUT] pulValue - number of VOIP ports in this board design
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetNumVoipPorts( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulNumVoipPorts, pulValue ) );
} /* BpGetNumVoipPorts */


#endif //CONFIG_EPON_SDK

/**************************************************************************
* Name       : BpGetExtIntrNumGpio
*
* Description: Iterate throught the board parameters and get the ExtIntr Num and its GPIO 
*              if any for a ExtIntr boardparm id in the bpExtIntrList. User call this 
*              function multiple times to get all assignment for a boardparm id.
* Parameters : [IN] idx - ExtIntr boardparm id index to the bpExtIntrList array.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pusExtInt - the external interrupt number or BP_VALUE_NOT_DEFINED
*              if that external interrupt id is not defined in the board parameter 
*              [OUT] pusGpio - the associated gpio number or BP_VALUE_NOT_DEFINED
*              if that external interrupt id is not defined or does not require gpio
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
*              BP_MAX_ITEM_EXCEEDED - idx exceed the maximum led id in the list
***************************************************************************/
int BpGetExtIntrNumGpio(int listIdx, void** token, unsigned short *pusExtInt, unsigned short *pusGpio)
{
    int listSize = sizeof(bpExtIntrList)/sizeof(enum bp_id);
    int rc;
    bp_elem_t *pelem = NULL;

    if(listIdx >= listSize)
        return BP_MAX_ITEM_EXCEEDED;
    else
    {
        rc = BpEnumUs(bpExtIntrList[listIdx], token, pusExtInt);
        if( rc == BP_SUCCESS ) {
            /* token already point to the next element of the interrupt bp id which should 
               be the associated gpio number */
            pelem = (bp_elem_t*)*token;
            if (pelem->id == bp_usGpio_Intr) {
                *pusGpio = pelem->u.us;
            } else {
                *pusGpio = BP_NOT_DEFINED;
            }
        }

        return rc;
    }
}

/**************************************************************************
* Name       : BpCheckExtIntr
* Description: This function check if there is any duplciated external interrupt
*              and its associated gpio number usage if any
* Parameters : [IN] ext_int_num - external interrupt number
*              [IN] gpio - gpio number
*              [IN] first - first time call this function
* Returns    : BP_SUCCESS - Success, valid interrupt and gpio usage
*              BP_GPIO_ALREADY_USED_AS_INTERRUPT - GPIO already used by interrupt
*              BP_INTERRUPT_ALREADY_DEFINED - Interrupt already used.
***************************************************************************/
int BpCheckExtIntr(int ext_int_num, int gpio, int first)
{
    static unsigned long gpios[(BP_GPIO_NUM_MAX/sizeof(long)) + 1];
    static unsigned long ints[(BP_EXT_INTR_NUM_MAX/sizeof(long)) + 1];
    unsigned long input=0, idx, bit;

    if(first) {
        memset(gpios, 0x0, sizeof(gpios));
        memset(ints, 0x0, sizeof(int));
    }

    input = gpio&BP_GPIO_NUM_MASK;
    idx=input/(sizeof(long)*8);
    bit=(1L << input%(sizeof(long)*8));

    if( gpio != BP_NOT_DEFINED )
    {
        if(BP_GPIO_NUM_MAX > input )
        {
            if(gpios[idx]&bit)
            {
                return BP_GPIO_ALREADY_USED_AS_INTERRUPT;
            }
            else
            {
                gpios[idx]|=bit;
            }
        }
        else
            return BP_INVALID_GPIO;
    }
     
    input = ext_int_num&BP_EXT_INTR_NUM_MASK;
    idx=input/(sizeof(long)*8);
    if( BP_EXT_INTR_NUM_MAX > input)
    {
        bit=(1L << input%(sizeof(long)*8));

        if(ints[idx]&bit)
        {
            return BP_INTERRUPT_ALREADY_DEFINED;
        }
        else
        {
            ints[idx]|=bit;
        }
    }
    else
         return BP_INVALID_INTERRUPT;

    return BP_SUCCESS;
}

/**************************************************************************
* Name       : BpGetLedGpio
*
* Description: Iterate throught the board parameters and get the gpio pin assignment 
*              for a LED boardparm id in the bpLedList. User call this function multiple 
*              times to get all assignment for a boardparm id.
* Parameters : [IN] idx - LED boardparm id index to the bpLedList array.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pusValue - the GPIO number for the LED or BP_VALUE_NOT_DEFINED
*              if that LED is not defined in the board parameter
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - this Led id is not defined in the boardparm
*              BP_MAX_ITEM_EXCEEDED - idx exceed the maximum led id in the list
***************************************************************************/
int BpGetLedGpio(int idx, void** token,  unsigned short *pusValue)
{
    int total = sizeof(bpLedList)/sizeof(enum bp_id);

    if(idx >= total)
        return BP_MAX_ITEM_EXCEEDED;
    else
        return BpEnumUs(bpLedList[idx], token, pusValue);
}

/**************************************************************************
* Name       : BpGetGpioGpio
*
* Description: Iterate throught the board parameters and get the gpio pin assignment 
*              for a GPIO boardparm id in the bpGpioList. User call this function multiple 
*              times to get all assignment for a boardparm id.
* Parameters : [IN] idx - GPIO boardparm id idx to the bpGpioList array.
*              [IN] token - transparent to caller. Set a pointer to NULL when first time call
*              [OUT] pusValue - the GPIO number for the Gpio or BP_VALUE_NOT_DEFINED
*              if that GPIO is not defined in the board parameter
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_SUCCESS_LAST - Last entry reached, no value is returened in pusValue
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - this GPIO id is not defined in the boardparm
*              BP_MAX_ITEM_EXCEEDED - idx exceed the maximum led id in the list
***************************************************************************/
int BpGetGpioGpio(int idx, void** token, unsigned short *pusValue)
{
    int total = sizeof(bpGpioList)/sizeof(enum bp_id);

    if(idx >= total)
        return BP_MAX_ITEM_EXCEEDED;
    else
        return BpEnumUs(bpGpioList[idx], token, pusValue);
}

/**************************************************************************
* Name       : BpIsGpioInUse
*
* Description: Check if the gpio pin is used in boardparamter for LED and other GPIO function
*              Does not including any serial gpio usage as they don't take gpin pin
*
* Parameters : [IN]  GPIO number
*
* Returns    : 0 if it is not used.
*              Non-zero if it is used in the currrent boardparameter
***************************************************************************/
int BpIsGpioInUse(unsigned short gpio)
{
    int i = 0, rc = 0, checkLed = 0;
    int inUse = 0;
    void* token = NULL;
    unsigned short gpio2;

    while( checkLed < 2 )
    {
        i = 0;
        token = 0;
        for(;;)
        {
            if( checkLed )
	        rc = BpGetLedGpio(i, &token, &gpio2);
            else
	        rc = BpGetGpioGpio(i, &token, &gpio2);

            if( rc == BP_MAX_ITEM_EXCEEDED )
                break;
            else if( rc == BP_SUCCESS )
            {
                /* serial led gpio does not take GPIO pin */
              	if( ((gpio2&BP_GPIO_SERIAL) == 0x0) && (gpio&BP_GPIO_NUM_MASK) == (gpio2&BP_GPIO_NUM_MASK) )
                {
                    inUse = 1;
                    break;
                }
            }
            else 
	    {
	        token = 0;
                i++;
            }
        }

        if( inUse )
            break;
        checkLed++;
    }

    return inUse;
}

/**************************************************************************
* Name       : BpGetEponGpio
*
* Description: Get the gpio pin assignment for all the EPON gpio related boardparm id in the
* bpEponGpioList
*
* Parameters : [IN] idx - GPIO boardparm id idx to the bpEponGpioList array.
*              [OUT] pusValue - the GPIO number for the Gpio or BP_VALUE_NOT_DEFINED
*              if that GPIO is not defined in the board parameter
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - this GPIO id is not defined in the boardparm
*              BP_MAX_ITEM_EXCEEDED - idx exceed the maximum led id in the list
***************************************************************************/
int BpGetEponGpio(int idx, unsigned short *pusValue)
{
    int total = sizeof(bpEponGpioList)/sizeof(enum bp_id);

	if(idx >= total)
        return BP_MAX_ITEM_EXCEEDED;
    else
        return BpGetGpio(bpEponGpioList[idx], pusValue);
}

/**************************************************************************
* Name       : BpGetPhyResetGpio
*
* Description: This function returns the GPIO pin assignment for PHYs reset pin
*
* Parameters : [IN] port - phy index
*              [OUT] pucValue - Address of a short that the bp_ucsGpioPhyReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetPhyResetGpio(int unit, int port, unsigned short *pusValue)
{
	const ETHERNET_MAC_INFO* enetMacInfo;
	enetMacInfo = BpGetEthernetMacInfoArrayPtr();

    if( unit >= BP_MAX_ENET_MACS  || port >= BP_MAX_PHY_PORTS )
        return BP_VALUE_NOT_DEFINED;

    if(BP_IS_CROSSBAR_PORT(port) ) 
    {
       port = BP_PHY_PORT_TO_CROSSBAR_PORT(port);
       printk("BpGetPhyResetGpio: port %d\n", port);
       *pusValue  =  enetMacInfo[unit].sw.crossbar[port].phyReset;
    }
    else
       *pusValue = enetMacInfo[unit].sw.phyReset[port];

	return BP_SUCCESS;
}

/**************************************************************************
* Name       : BpGetBoardResetGpio
*
* Description: This function returns the GPIO pin assignment for board reset pin
*
* Parameters : [OUT] pucValue - Address of a short that the bp_usGpioBoardReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetBoardResetGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioBoardReset, pusValue ) );
} 
/**************************************************************************
* Name       : BpGetUSBGpio
*
* Description: This function returns the GPIO pin assignment for USB overcurrent GPIOs
*
* Parameters :  [IN] usb - usb number on board
* 				[OUT] pusValue - Address of a GPIO_USB_INFO struct that the bp_usGpioUsbX
*                  GPIO pins is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetUSBGpio( int usb, GPIO_USB_INFO *gpios )
{
	unsigned short pusValue;
    int nRet;

	switch (usb)
	{
	   case 0:
		   nRet = BpGetGpio(bp_usGpioUsb0, &pusValue);
		   break;
	   case 1:
		   nRet = BpGetGpio(bp_usGpioUsb1, &pusValue);
		   break;
	   default:
		   return BP_VALUE_NOT_DEFINED;
	}
	if (nRet == BP_SUCCESS)
	{
		gpios->gpio_for_oc_detect = (pusValue & 0xff00) >> 8; // input pin
		gpios->gpio_for_oc_output = pusValue & 0xff;
	}
    return nRet;
} 
/**************************************************************************
* Name       : BpGetPhyAddr
*
* Description: This function returns the phy address for the required port
*
* Parameters :  [IN] unit - punit number
*               [IN] port - port number
* 				[OUT] phyId - value of the phy address to be used with any MDC/MDIO command.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetPhyAddr(int unit, int port)
{
   int phyId = -1;
   const ETHERNET_MAC_INFO *pE;

   pE = BpGetEthernetMacInfoArrayPtr();

   if ( unit == 0 && (pE[0].ucPhyType == BP_ENET_NO_PHY) && BpGetPortConnectedToExtSwitch() >=0)  /* switch virtual PHY Address */
                   phyId = pE[0].ucPhyAddress; 
   else if (unit < BP_MAX_ENET_MACS  && port < BP_MAX_SWITCH_PORTS)
         phyId = pE[unit].sw.phy_id[port] & BCM_PHY_ID_M;

   return phyId;
}
/**************************************************************************
* Name       : BpGetOpticalWan
*
* Description: This function returns the optical transceiver type
*
* Parameters : [OUT] pulValue - tranceiver type.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetOpticalWan( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulOpticalWan, pulValue ) );
} 

/**************************************************************************
* Name       : BpGetSimInterfaces
*
* Description: This function returns the simcard interface number.
*
* Parameters : [OUT] pusValue - Address of short word that simcard
*                  interface is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSimInterfaces( unsigned short *pusValue )
{
    return( BpGetUs(bp_ulSimInterfaces, pusValue ) );
} /* BpGetSimInterfaces */

/**************************************************************************
* Name       : BpGetSlicInterfaces
*
* Description: This function returns the slic interface number.
*
* Parameters : [OUT] pusValue - Address of short word that slic
*                  interface is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSlicInterfaces( unsigned short *pusValue )
{
    return( BpGetUs(bp_ulSlicInterfaces, pusValue ) );
} /* BpGetSlicInterfaces */

/**************************************************************************
* Name       : BpGetAePolarity 
*
* Description: This function returns the Active Ethernet TRX Polarity.
*
* Parameters : [OUT] pusValue - polarity value 1(high) or 0(low).
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAePolarity( unsigned short *pusValue )
{
    return( BpGetUs(bp_usAePolarity, pusValue ) );
} /* BpGetAePolarity */


/**************************************************************************
* Name       : BpGetPonTxEnGpio
*
* Description: This function returns the pon tx enable gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPonTxEnGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioPonTxEn, pusValue ) );
} /* BpGetPonTxEnGpio */


/**************************************************************************
* Name       : BpGetPonResetGpio
*
* Description: This function returns the Pon Reset gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPonResetGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioPonReset, pusValue ) );
} /* BpGetPonResetGpio */

/**************************************************************************
* Name       : BpGetPonRxEnGpio
*
* Description: This function returns the gpon rx enable gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPonRxEnGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioPonRxEn, pusValue ) );
} /* BpGetPonRxEnGpio */

/**************************************************************************
* Name       : BpGetRogueOnuEn
*
* Description: This function returns true if internal rogue onu is connected.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  value is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetRogueOnuEn( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usRogueOnuEn, pusValue ) );
} /* BpGetRogueOnuEn */

/**************************************************************************
* Name       : BpGetGpioLedSim
*
* Description: This function returns simcard led pin.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  value is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioLedSim( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedSim, pusValue ) );
} /* BpGetGpioLedSim */

/**************************************************************************
* Name       : BpGetGpioLedSimITMS
*
* Description: This function returns simcard ITMS led pin.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  value is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioLedSimITMS( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLedSim_ITMS, pusValue ) );
} /* BpGetGpioLedSimITMS */

/**************************************************************************
* Name       : BpGetWanSignalDetectedGpio
*
* Description: This function returns the wan SD gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanSignalDetectedGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioWanSignalDetected, pusValue ) );
} /* BpGetWanSignalDetectedGpio */


/* Time Synchronization */

/**************************************************************************
* Name       : BpGetTsync1025mhzPin
*
* Description: This function returns the 10/25MHz pin number.
*
* Parameters : [OUT] pusValue - Address of short word that T-Sync 10/25MHz
*                  pin number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTsync1025mhzPin( unsigned short *pusValue )
{
    return( BpGetUs(bp_usTsync1025mhz, pusValue ) );
} /* BpGetTsync1025mhzPin */

/**************************************************************************
* Name       : BpGetTsync8khzPin
*
* Description: This function returns the 8KHz pin number.
*
* Parameters : [OUT] pusValue - Address of short word that T-Sync 8KHz
*                  pin number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTsync8khzPin( unsigned short *pusValue )
{
    return( BpGetUs(bp_usTsync8khz, pusValue ) );
} /* BpGetTsync8khzPin */

/**************************************************************************
* Name       : BpGetTsync1ppsPin
*
* Description: This function returns the 1PPS pin number.
*
* Parameters : [OUT] pusValue - Address of short word that T-Sync 1PPS
*                  pin number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTsync1ppsPin( unsigned short *pusValue )
{
    return( BpGetUs(bp_usTsync1pps, pusValue ) );
} /* BpGetTsync1ppsPin */

/**************************************************************************
* Name       : BpGetTsyncPonUnstableGpio
*
* Description: This function returns the PON Unstable pin number.
*
* Parameters : [OUT] pusValue - Address of short word that T-Sync PON Unstable
*                  pin number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTsyncPonUnstableGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioTsyncPonUnstable, pusValue ) );
} /* BpGetTsyncPonUnstableGpio */


/**************************************************************************
* Name       : BpGetMemoryConfig
*
* Description: This function returns the DDR Memory Config
*
* Parameters : [OUT] pulValue 
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMemoryConfig( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulMemoryConfig, pulValue ) );
} 

/**************************************************************************
* Name       : BpGetBatteryEnable
*
* Description: Indicates if the board has the necessary HW to support a Battery
*              using the BMU (Battery Management Unit)
*
*
* Parameters : [OUT] pusValue - 1 if BMU driver needs to run
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetBatteryEnable( unsigned short *pusValue )
{
    return( BpGetUs(bp_usBatteryEnable, pusValue ) );
} /* BpGetBatteryEnable */


int BpGetSerialLedData( unsigned short *pusValue )
{
    return( BpGetUs( bp_usSerialLedData, pusValue ) );
} 

/**************************************************************************
* Name       : BpGetMaxNumCpu
*
* Description: This function returns max cpu number configured for the board id.
*
* Parameters : [OUT] pulValue - Address of short word for max cpu number
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMaxNumCpu( unsigned int *pulValue )
{
    return( BpGetUl(bp_ulMaxNumCpu, pulValue ) );
} /* BpGetMaxNumCpu */

#if !defined(_BCM960333_) && !defined(CONFIG_BCM960333) && !defined(_BCM963268_) && !defined(CONFIG_BCM963268) && !defined(_BCM96838_) && !defined(CONFIG_BCM96838) && !defined(_BCM947189_) && !defined(CONFIG_BCM947189)

static int BpGrepElemList(enum bp_id id, enum bp_id *list, int n) {
    int i;
    for (i = 0 ; i < n ; i++) {
        if (list[i] == id) {
            return(1);
        }
    }
    return(0);
}


#ifdef CONFIG_BP_PHYS_INTF
static int BpGrepPinmuxListInIntf(enum bp_id id, PHYS_INTF_INFO* pIntf, unsigned short *pusFunction, unsigned int *pulMuxInfo) {
    int i;
    bp_elem_t *current_bp = NULL;
    bp_elem_t *start_elem = pIntf->pIntfStart;
    enum bp_id this_id;

    if (0 == g_pCurrentBp) {
        return(BP_BOARD_ID_NOT_SET);
    }
    if (0 == g_pinmux_defs) {
        printk("ERROR:BpGrepPinmuxList called before pinmux table selected\n");
        return(BP_VALUE_NOT_DEFINED);
    }

    current_bp = BpGetElem(id, &start_elem, bp_usIntfEnd);
    if (id != current_bp->id) {
        return(BP_VALUE_NOT_DEFINED);
    }

    this_id = current_bp->id;
    // Check for vdsl_ctl membership
    if (BpGrepElemList(this_id, bpDslCtlGpioList, sizeof(bpDslCtlGpioList)/sizeof(enum bp_id))) {
        this_id = bp_ReservedDslCtl;
    }

    for (i = 0 ; g_pinmux_defs[i].id  != bp_last ; i++) {
        if ((g_pinmux_defs[i].id == this_id) && (g_pinmux_defs[i].port == -1) ) {
            if ((g_pinmux_defs[i].func == (current_bp->u.us & BP_GPIO_NUM_MASK))
                || (g_pinmux_defs[i].func == -1)) {
                *pusFunction = g_pinmux_defs[i].func;
                *pulMuxInfo = g_pinmux_defs[i].mux_info | BP_PINMUX_VALID;
                return(BP_SUCCESS);
            }
        }
    }
    return(BP_VALUE_NOT_DEFINED);
}

int BpGetDslCtl(enum bp_id id, int intfIdx, unsigned short *pusValue ) {
    int ret;
    unsigned short Function;
    unsigned int MuxInfo;
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }
    ret = BpGrepPinmuxListInIntf(id, pDSLIntf, &Function, &MuxInfo);
    if ((ret == BP_SUCCESS) && ((MuxInfo & BP_PINMUX_OP_MASK) == BP_PINMUX_VDSLCTL)) {
        *pusValue = (MuxInfo & BP_PINMUX_ARG_MASK) >>  BP_PINMUX_ARG_SHIFT ;
    } else {
        *pusValue = BP_NOT_DEFINED;
        ret = BP_VALUE_NOT_DEFINED;
    }
    return(ret);
}
#else
static int BpGrepPinmuxList(enum bp_id id,  unsigned short *pusFunction, unsigned int *pulMuxInfo) {
    int i;
    bp_elem_t *current_bp;
    bp_elem_t *start_elem = 0;
    enum bp_id this_id;
    if (0 == g_pCurrentBp) {
        return(BP_BOARD_ID_NOT_SET);
    }
    if (0 == g_pinmux_defs) {
        printk("ERROR:BpGrepPinmuxList called before pinmux table selected\n");
        return(BP_VALUE_NOT_DEFINED);
    }

    current_bp = BpGetElem(id, &start_elem, bp_last);
    if (id != current_bp->id) {
        return(BP_VALUE_NOT_DEFINED);
    }

    this_id = current_bp->id;
    // Check for vdsl_ctl membership
    if (BpGrepElemList(this_id, bpDslCtlGpioList, sizeof(bpDslCtlGpioList)/sizeof(enum bp_id))) {
        this_id = bp_ReservedDslCtl;
    }

    for (i = 0 ; g_pinmux_defs[i].id  != bp_last ; i++) {
        if ((g_pinmux_defs[i].id == this_id) && (g_pinmux_defs[i].port == -1) ) {
            if ((g_pinmux_defs[i].func == (current_bp->u.us & BP_GPIO_NUM_MASK))
                || (g_pinmux_defs[i].func == -1)) {
                *pusFunction = g_pinmux_defs[i].func;
                *pulMuxInfo = g_pinmux_defs[i].mux_info | BP_PINMUX_VALID;
                return(BP_SUCCESS);
            }
        }
    }
    return(BP_VALUE_NOT_DEFINED);
}

int BpGetDslCtl(enum bp_id id, unsigned short *pusValue) {
    int ret;
    unsigned short Function;
    unsigned int MuxInfo;

    ret = BpGrepPinmuxList(id, &Function, &MuxInfo);
    if ((ret == BP_SUCCESS) && ((MuxInfo & BP_PINMUX_OP_MASK) == BP_PINMUX_VDSLCTL)) {
        *pusValue = (MuxInfo & BP_PINMUX_ARG_MASK) >>  BP_PINMUX_ARG_SHIFT ;
    } else {
        *pusValue = BP_NOT_DEFINED;
        ret = BP_VALUE_NOT_DEFINED;
    }
    return(ret);
}
#endif

static int BpGrepPinmuxListOnly(enum bp_id id, unsigned short gpionum,  unsigned short *pusFunction, unsigned int *pulMuxInfo) {
    int i;
    if (0 == g_pinmux_defs) {
        printk("ERROR:BpGrepPinmuxListOnly called before pinmux table selected\n");
        return(BP_VALUE_NOT_DEFINED);
    }
    for (i = 0 ; g_pinmux_defs[i].id  != bp_last ; i++) {
        if ((g_pinmux_defs[i].id == id) && (g_pinmux_defs[i].port == -1) ) {
            if ( g_pinmux_defs[i].func == gpionum
                || g_pinmux_defs[i].func == -1 ) {
                *pusFunction = g_pinmux_defs[i].func;
                *pulMuxInfo = g_pinmux_defs[i].mux_info | BP_PINMUX_VALID;
                if (g_pinmux_defs[i].func == -1) {
                    *pulMuxInfo |= (gpionum & BP_GPIO_NUM_MASK);
                }
                return(BP_SUCCESS);
            }
        }
    }
    return(BP_VALUE_NOT_DEFINED);
}

int BpGetIfacePinmux(unsigned int interface, int maxnum, int *outcnt, int *errcnt, unsigned short *pusFunction, unsigned int *pulMuxInfo)
{
    int i;
    *outcnt = 0;
    *errcnt = 0;
    // printf("look for %lx\n",interface);
    for (i = 0 ; i < sizeof(g_pinmux_fn_defs)/sizeof(bp_pinmux_fn_defs_t); i++) {
        if (interface == g_pinmux_fn_defs[i].function) {
            pusFunction[*outcnt] = 0;
            pulMuxInfo[*outcnt] = g_pinmux_fn_defs[i].mux_info | BP_PINMUX_VALID;
            // printf("got %d\n", g_pinmux_fn_defs[i].mux_info & 0xff);
            (*outcnt)++;
            if (*outcnt >= maxnum) {
                return(BP_MAX_ITEM_EXCEEDED);
            }
        }
    }
    return(BP_SUCCESS);
}

/**************************************************************************
* Name       : BpGetAllPinmux
*
* Description: Iterate throught the board parameters and get the gpio/led pinmux assignment 
*              Call this function multiple times to get all assignments for a boardparm id.
* Parameters : 
*              [IN] maxnum -- the maximum number of assignments that may be returned
*              [OUT] outcnt -- the number of assignments actually returned
*              [OUT] errcnt -- the number of errors detected
*              [OUT] pusFunction -- array of functions (usually a BP_GPIO_ or BP_SERIAL_GPIO value)
*              [OUT] pulMuxinfo -- array of pinmux tuples to be set
*
* Returns    : BP_SUCCESS - Success, value is returned in pusValue
*              BP_BOARD_ID_NOT_SET
*              BP_MAX_ITEM_EXCEEDED
*              BP_VALUE_NOT_DEFINED - attempted to use an invalid pin
***************************************************************************/
int BpGetAllPinmux(int maxnum, int *outcnt, int *errcnt, unsigned short *pusFunction, unsigned int *pulMuxInfo)
{
    bp_elem_t *current_bp, *next_bp;
    int port;
    short func = 0;
    int i,j;
    unsigned int u;
    int dummy;
    int hw_led;
    int check;
    enum bp_id bp_aulPhyId[BP_MAX_SWITCH_PORTS+1] = PHYID_LIST;
    enum bp_id this_id;
    const ETHERNET_MAC_INFO *Enet;
    if (BP_SUCCESS != BpGetUl(bp_ulPinmuxTableSelect, &u )) {
        u = 0;
    }
    g_pinmux_defs  = g_pinmux_defs_tables[u];
   
    *outcnt = 0;
    *errcnt = 0;
    
#if defined(CONFIG_BOARDPARMS_VOICE) || defined(_CFE_)
    // Add voice pinmux related info. Last element of returned
    // structure is bp_elemp which points to g_pCurrentBp
    current_bp = BpGetVoicePmuxBp( g_pCurrentBp );
    
    // Voice pinmux not valid, use g_pCurrentBp directly
    if( !current_bp ) 
#endif // CONFIG_BOARDPARMS_VOICE || _CFE_
    {
        current_bp = g_pCurrentBp;
    }
        
    port = -1;
    if (0 == g_pCurrentBp) {
        return(BP_BOARD_ID_NOT_SET);
    }
    // Check for network ports requiring MAC interfaces to be active
    Enet = BpGetEthernetMacInfoArrayPtr();
    for (i = 0 ; i < BP_MAX_ENET_MACS ; i++) {
        for (j = 0; j < BP_MAX_SWITCH_PORTS ; j++) {
            if ((Enet[i].sw.port_map >> j) & 0x01)  {
                 // printf("switch %d port %d\n",i,j);
                     u = j;
#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || defined(CONFIG_BCM96836) || defined(_BCM96836_) || defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(CONFIG_BCM96856) || defined(_BCM96856_)
                 if ((Enet[i].sw.phy_id[j] & MAC_IFACE) == MAC_IF_RGMII) {
#else
                 if (Enet[i].sw.phy_id[j] & MAC_IFACE) {
#endif
                     check = 0;
                     if (BP_SUCCESS == BpGetIfacePinmux (BP_PINMUX_FNTYPE_xMII | u, maxnum - *outcnt, &check, &dummy,  &pusFunction[*outcnt], &pulMuxInfo[*outcnt])) {
                         if ((pulMuxInfo[*outcnt] & BP_PINMUX_VAL_MASK) != BP_PINMUX_VAL_DUMMY) {
                             // Only count response if not a DUMMY
                             *outcnt = *outcnt + check;
                         }
                     }
                     if (check == 0) {
#ifdef PRINT_ERRORS
                         printf("error - no xMII interface for port %d\n",u);
#endif
                         (*errcnt)++;
                     }
                 }
              }
        }
        for (j = 0; j < BP_MAX_CROSSBAR_EXT_PORTS ; j++) {
            u = BP_CROSSBAR_PORT_TO_PHY_PORT(j);
            if (Enet[i].sw.crossbar[j].switch_port != BP_CROSSBAR_NOT_DEFINED)  {
                 if (Enet[i].sw.crossbar[j].phy_id & MAC_IFACE) {
                     check = 0;
                     if (BP_SUCCESS == BpGetIfacePinmux (BP_PINMUX_FNTYPE_xMII | u, maxnum - *outcnt, &check, &dummy,  &pusFunction[*outcnt], &pulMuxInfo[*outcnt])) {
                         if ((pulMuxInfo[*outcnt] & BP_PINMUX_VAL_MASK) != BP_PINMUX_VAL_DUMMY) {
                             // Only count response if not a DUMMY
                             *outcnt = *outcnt + check;
                         }
                     }
                     if (check == 0) {
#ifdef PRINT_ERRORS
                         printf("error - no xMII interface for port %d\n",u);
#endif
                         (*errcnt)++;
                     }
                 }
            }
        }
    }
    // iterate over boardparms, following links to templates
    // since a pin may be assigned one way in a template and superceded by the board-id inheriting it,
    // pinmux settings must be applied in reverse order
    while ( bp_last != current_bp->id ) {
        if ( bp_elemTemplate == current_bp->id ) {
            current_bp = current_bp->u.bp_elemp;
        }
        
        // keep track of switch port numbers when encountered
        for (i = 0 ; i <= BP_MAX_SWITCH_PORTS ; i++) {
             if (bp_aulPhyId[i] == current_bp->id) {
                port = i;
             }
        }

#ifdef CONFIG_BP_PHYS_INTF
        /* get the port number if it is new physical inteface usage */
        if( current_bp->id == bp_usIntfId ) {
            bp_elem_t * pStart = current_bp;
            next_bp = BpGetElem(bp_usPortNum, &pStart, bp_usIntfEnd);
            if( next_bp->id == bp_usPortNum )
                port = next_bp->u.us;
            else {
                /* this interface is from template and being overridden, it is checked in the 
                 template bp. so just ignore here */
                current_bp = (next_bp->id == bp_usIntfEnd) ? next_bp+1 : current_bp+1;
                continue;
            }
        }
#endif

        /* check for spi slave select definition.bp_usSpiSlaveSelectNum must be follwed by bp_usSpiSlaveSelectGpioNum */ 
        if( current_bp->id == bp_usSpiSlaveSelectNum )
        {
            port = current_bp->u.us;
            next_bp = current_bp + 1;
            /* if gpio num is not specified, default gpio(first entry in the pinmux_def) will be used */
            if( next_bp->id == bp_usSpiSlaveSelectGpioNum )
                func = next_bp->u.us&BP_GPIO_NUM_MASK;
            else
                func = -1;
        }

        // Check for Interface Enable
        check = 0;
        if (current_bp->id == bp_ulInterfaceEnable) {
            BpGetIfacePinmux(current_bp->u.ul, maxnum - *outcnt, &check, &j, &pusFunction[*outcnt], &pulMuxInfo[*outcnt]);
            *outcnt = *outcnt + check;
            if (check == 0) {
#ifdef PRINT_ERRORS
                printf("error - no interface matching %d\n",current_bp->u.ul);
#endif
                (*errcnt)++;
            }
        }

        hw_led = -1;
        this_id = current_bp->id;

        // Check for vdsl_ctl membership
        if (BpGrepElemList(this_id, bpDslCtlGpioList, sizeof(bpDslCtlGpioList)/sizeof(enum bp_id))) {
            this_id = bp_ReservedDslCtl;
        }

        for (i = 0 ; g_pinmux_defs[i].id  != bp_last ; i++) {
            if ((g_pinmux_defs[i].id == this_id) && ((g_pinmux_defs[i].port == -1) || (g_pinmux_defs[i].port == port))) {
                hw_led = i;
                if ((g_pinmux_defs[i].func == (current_bp->u.us & BP_GPIO_NUM_MASK)) 
                    || ((this_id == bp_usSpiSlaveSelectNum) && (func == -1 || func == g_pinmux_defs[i].func))
                    || ((BP_GPIO_SERIAL == (current_bp->u.us & BP_GPIO_SERIAL))
                       && ((g_pinmux_defs[i].mux_info & (BP_PINMUX_OPTLED_MASK | BP_PINMUX_OPTLED_VALID)) 
                          == (BP_PINMUX_OPTLED_VALID | BP_PINMUX_OPTLED_NUM(current_bp->u.us & BP_GPIO_NUM_MASK))))
                       ) 
                {
                    check++;
                    if(this_id == bp_usSpiSlaveSelectNum)
                        pusFunction[*outcnt] = g_pinmux_defs[i].func;
                    else
                        pusFunction[*outcnt] = current_bp->u.us;
                    pulMuxInfo[*outcnt] = g_pinmux_defs[i].mux_info | BP_PINMUX_VALID;
                    (*outcnt)++;
                    if (*outcnt >= maxnum) {
                        return(BP_MAX_ITEM_EXCEEDED);
                    }
                }
            }
        }
        if (hw_led < 0) {
            // Not a HW LED, Check for SW LED or GPIO assignment
            if (BpGrepElemList(this_id, bpLedList, sizeof(bpLedList)/sizeof(enum bp_id))) {
                // Get Muxinfo for this pin
                // FIXME -- if a PWM LED, look up the FNTYPE_PWM instead
                // FIXME -- note error if not found
                if (BP_GPIO_SERIAL == (current_bp->u.us & BP_GPIO_SERIAL)) {
                    pusFunction[*outcnt] = current_bp->u.us; 
                    pulMuxInfo[*outcnt] = BP_PINMUX_SWLED; 
                    (*outcnt)++;
                    check++;
                } else if (BP_LED_USE_GPIO == (current_bp->u.us & BP_LED_USE_GPIO)) {
                  if (BP_SUCCESS == BpGrepPinmuxListOnly(bp_ReservedAnyGpio, current_bp->u.us & BP_GPIO_NUM_MASK, &pusFunction[*outcnt], &pulMuxInfo[*outcnt])) {
                       pusFunction[*outcnt] = current_bp->u.us;
                       pulMuxInfo[*outcnt] |= BP_PINMUX_SWGPIO | BP_PINMUX_VALID;
                       (*outcnt)++;
                       check++;
                  }
                } else if (BP_SUCCESS == BpGrepPinmuxListOnly(bp_ReservedAnyLed, current_bp->u.us & BP_GPIO_NUM_MASK, &pusFunction[*outcnt], &pulMuxInfo[*outcnt])) {
                    pusFunction[*outcnt] = current_bp->u.us; 
                    pulMuxInfo[*outcnt] |= BP_PINMUX_SWLED | BP_PINMUX_VALID;
                    (*outcnt)++;
                    check++;
                }
            }
            else if (BpGrepElemList(this_id, bpGpioList, sizeof(bpGpioList)/sizeof(enum bp_id))) {
                // Get Muxinfo for this pin
                // FIXME -- note error if not found
                if ( current_bp->u.us != BP_NOT_DEFINED ) {                
                   if (BP_SUCCESS == BpGrepPinmuxListOnly(bp_ReservedAnyGpio, current_bp->u.us & BP_GPIO_NUM_MASK, &pusFunction[*outcnt], &pulMuxInfo[*outcnt])) {
                       pusFunction[*outcnt] = current_bp->u.us; 
                       pulMuxInfo[*outcnt] |= BP_PINMUX_SWGPIO | BP_PINMUX_VALID;
                       (*outcnt)++;
                       check++;
                    }
                }
            } else {
                // none of the above
                check++;
            }
            // otherwise, don't return and go onto the next element
        } else if (check == 0) {
            // ERROR
#ifdef PRINT_ERRORS
            printf("error on pinmux id %d value %d port %d\n", current_bp->id, current_bp->u.us & 0xff, port); // Report erroneous value on error
#endif
            (*errcnt)++;
        }
        if (*outcnt >= maxnum) {
            return(BP_MAX_ITEM_EXCEEDED);
        }
        current_bp++;
    }
    // Set defaults last so they are lowest priority in reverse order
    BpGetIfacePinmux(BP_PINMUX_FNTYPE_DEFAULT, maxnum - *outcnt, &check, &j, &pusFunction[*outcnt], &pulMuxInfo[*outcnt]);
    *outcnt = *outcnt + check;
    return(BP_SUCCESS);

}

/**************************************************************************
* Name       : BpGetIntAFELDModeDslCtl
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*                  for the internal path.
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioIntAFELDMode
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetIntAFELDModeDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDModeDslCtl(0, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioIntAFELDMode, pusValue ) );
#endif
} /* BpGetIntAFELDModeDslCtl */

/**************************************************************************
* Name       : BpGetIntAFELDPwrDslCtl
*
* Description: This function returns the GPIO pin assignment for turning on/off the internal AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDPwr
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetIntAFELDPwrDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDPwrDslCtl(0, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioIntAFELDPwr, pusValue ) );
#endif
} /* BpGetIntAFELDPwrDslCtl */

/**************************************************************************
* Name       : BpGetExtAFELDModeDslCtl
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*                  for the external path.
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDMode
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDModeDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDModeDslCtl(1, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioExtAFELDMode, pusValue ) );
#endif
} /* BpGetExtAFELDModeDslCtl */

/**************************************************************************
* Name       : BpGetExtAFELDPwrDslCtl
*
* Description: This function returns the GPIO pin assignment for turning on/off the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDPwr
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDPwrDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDPwrDslCtl(1, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioExtAFELDPwr, pusValue ) );
#endif
} /* BpGetExtAFELDPwrDslCtl */

/**************************************************************************
* Name       : BpGetExtAFELDDataDslCtl
*
* Description: This function returns the GPIO pin assignment for sending config data to the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioExtAFELDData
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDDataDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDDataDslCtl(1, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioExtAFELDData, pusValue ) );
#endif
} /* BpGetExtAFELDDataDslCtl */

int BpGetIntAFELDDataDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDDataDslCtl(0, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioIntAFELDData, pusValue ) );
#endif
} /* BpGetIntAFELDDataDslCtl */


/**************************************************************************
* Name       : BpGetExtAFELDClkDslCtl
*
* Description: This function returns the GPIO pin assignment for sending the clk to the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioExtAFELDClk
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetExtAFELDClkDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDClkDslCtl(1, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioExtAFELDClk, pusValue ) );
#endif
} /* BpGetExtAFELDClkDslCtl */

int BpGetIntAFELDClkDslCtl( unsigned short *pusValue )
{
#ifdef CONFIG_BP_PHYS_INTF
    return  BpGetAFELDClkDslCtl(0, pusValue );
#else
    return( BpGetDslCtl(bp_usGpioIntAFELDClk, pusValue ) );
#endif
} /* BpGetIntAFELDClkDslCtl */

#endif

#if defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM96858_) || defined(_BCM96836_) || defined(_BCM96846_) || defined(_BCM96856_) || defined(CONFIG_BCM_PON) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
/****************************************************************************
* Name       : BpGetPmdMACEwakeEn
*
* Description: This function returns the PMD PON MAC E-Wake control .
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPmdMACEwakeEn( unsigned short *pusValue )
{
    return( BpGetUs(bp_usPmdMACEwakeEn, pusValue ) );
} /* BpGetGpioPmdMACEwakeEn */

/**************************************************************************
* Name       : BpGetPmdAlarmExtIntr
*
* Description: This function returns the external interrupt number for the
*              PMD Fault.
*
* Parameters : [OUT] pusValue - Address of short word that the PMD Fault
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPmdAlarmExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrPmdAlarm, pusValue ) );
} /* BpGetPmdAlarmExtIntr */

/**************************************************************************
* Name       : BpGetPmdInvSerdesRxPol
*
* Description: This function determines if the serdes Rx polarity should
*              be inverted
*
* Parameters : [OUT] 1 - revert
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPmdInvSerdesRxPol( unsigned short *pusValue )
{
    return( BpGetUs(bp_InvSerdesRxPol, pusValue ) );
} /* BpGetPmdInvSerdesRxPol */

/**************************************************************************
* Name       : BpGetPmdInvSerdesTxPol
*
* Description: This function determines if the serdes Tx polarity should
*              be inverted
*
* Parameters : [OUT] 1 - revert
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPmdInvSerdesTxPol( unsigned short *pusValue )
{
    return( BpGetUs(bp_InvSerdesTxPol, pusValue ) );
} /* BpGetPmdInvSerdesTxPol */


/**************************************************************************
* Name       : BpGetWanSignalDetectedExtIntr
*
* Description: This function returns the external interrupt number for the
*              PMD siganl detect.
*
* Parameters : [OUT] pusValue - Address of short word that the PMD Fault
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanSignalDetectedExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrWanSignalDetected, pusValue ) );
} /* BpGetWanSignalDetectedExtIntr */


/**************************************************************************
* Name       : BpGetPmdAlarmExtIntrGpio
*
* Description: This function returns the external interrupt GPIO number for
*              PMD Alarm.
*
* Parameters : [OUT] pusValue - Address of short word that the PMD Alarm
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPmdAlarmExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrPmdAlarm, pusValue);
    
    return( ret );  
} /* BpGetPmdAlarmExtIntrGpio */


/**************************************************************************
* Name       : BpGetWanSignalDetectedExtIntrGpio
*
* Description: This function returns the external interrupt GPIO number for
*              PMD signal detect.
*
* Parameters : [OUT] pusValue - Address of short word that the PMD Alarm
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanSignalDetectedExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp )
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }

    ret = BpGetExtIntrGpio(bp_usExtIntrWanSignalDetected, pusValue);

    return( ret );
} /* BpGetWanSignalDetectedExtIntrGpio */



/**************************************************************************
* Name       : BpGetGpioPmdReset
*
* Description: This function returns the PMD Reset gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioPmdReset( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioPmdReset, pusValue ) );
} /* BpGetGpioPmdReset */

/**************************************************************************
* Name       : BpGetPmdFunc
*
* Description: This function returns the PMD functionalities.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  value is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPmdFunc( unsigned short *pusValue )
{
    return( BpGetUs(bp_pmdFunc, pusValue ) );
} /* BpGetPmdFunc */

/**************************************************************************
* Name       : BpGetGpioSpromClk
*
* Description: This function returns the sprom clock gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioSpromClk( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpromClk, pusValue ) );
} /*  BpGetGpioSpromClk */

/**************************************************************************
* Name       : BpGetGpioSpromData
*
* Description: This function returns the sprom data gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioSpromData( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpromData, pusValue ) );
} /*  BpGetGpioSpromData */

/**************************************************************************
* Name       : BpGetGpioSpromRst
*
* Description: This function returns the sprom reset gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioSpromRst( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpromRst, pusValue ) );
} /*  BpGetGpioSpromRst */

/**************************************************************************
* Name       : BpGetGpioAttachedDevReset
*
* Description: This function returns the gpio number driving attached device 
*              reset.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGpioAttachedDevReset( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioAttachedDevReset, pusValue ) );
} /*  BpGetGpioAttachedDevReset */

#if !defined(_CFE_)
/**************************************************************************
* Name       : BpGetXdslDistpointInfo
*
* Description: This function returns hardware information relating to xdsl
*              distpoint userspace application.
*
* Parameters :  [OUT] pXdslDistpointInfo - Addres of xdsl distpoint information
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
int BpGetXdslDistpointInfo(PXDSL_DISTPOINT_INFO pInfo)
{
    unsigned short found = 1;
    bp_elem_t *pstartElem = 0;
    bp_elem_t *bp_elem = 0;

    if( 0 == g_pCurrentBp ) {
        return BP_BOARD_ID_NOT_SET;
    }

    // retrieve spi slave information
    if (BP_SUCCESS != BpGetUs(bp_usSpiSlaveBusNum, &pInfo->spi.busNum))
        printk("ERROR BpGetXdslDistpointInfo no board parameter info for spi "
               "busNum\n");
    if (BP_SUCCESS != BpGetUs(bp_usSpiSlaveMode, &pInfo->spi.mode))
        printk("ERROR BpGetXdslDistpointInfo no board parameter info for spi "
               "mode\n");
    if (BP_SUCCESS != BpGetUl(bp_ulSpiSlaveCtrlState, &pInfo->spi.ctrlState))
        printk("ERROR BpGetXdslDistpointInfo no board parameter info for spi "
               "ctrlState\n");
    if (BP_SUCCESS != BpGetUs(bp_usSpiSlaveMode, &pInfo->spi.mode))
        printk("ERROR BpGetXdslDistpointInfo no board parameter info for spi "
               "mode\n");
    if (BP_SUCCESS != BpGetUl(bp_ulSpiSlaveMaxFreq, &pInfo->spi.maxFreq))
        printk("ERROR BpGetXdslDistpointInfo no board parameter info for spi "
               "maxFreq\n");
    if (BP_SUCCESS != BpGetUs(bp_usSpiSlaveProtoRev, &pInfo->spi.protoRev))
        printk("ERROR BpGetXdslDistpointInfo no board parameter info for spi "
               "protoRev\n");
    pInfo->spi.nbSlaves = 0;
    bp_elem = BpGetElem(bp_usSpiSlaveProtoRev, &pstartElem, bp_last);
    if (bp_elem->id == bp_last) {
        return -1;
    }
    while ((pInfo->spi.nbSlaves < BP_XDSL_DISTPOINT_MAX_SPI_SLAVE) && found) {
        bp_elem = BpGetElem(bp_usSpiSlaveSelectNum, &pstartElem, bp_last);
        if (bp_elem->id == bp_last) {
            found = 0;
        }
        else {
            pInfo->spi.selectNum[pInfo->spi.nbSlaves] = pstartElem->u.us;
            pstartElem++;
            pInfo->spi.reset[pInfo->spi.nbSlaves] = BpGetSubUs(
                bp_usGpioSpiSlaveReset, pstartElem, bp_usSpiSlaveSelectNum);
            pInfo->spi.nbSlaves++;
        }
    }

    // retrieve reset signals
    pstartElem = 0;
    found = 1;
    pInfo->nbReset = 0;
    while ((pInfo->nbReset < BP_XDSL_DISTPOINT_MAX_RESET) && found) {
        bp_elem = BpGetElem(bp_usXdResetGpio, &pstartElem, bp_last);
        if (bp_elem->id == bp_last) {
            found = 0;
        }
        else {
            pInfo->reset[pInfo->nbReset].gpio = pstartElem->u.us;
            pstartElem++;
            pInfo->reset[pInfo->nbReset].name = BpGetSubCp(
                bp_cpXdResetName, pstartElem, bp_usXdResetGpio);
            pInfo->reset[pInfo->nbReset].releaseOnInit = BpGetSubUs(
                bp_usXdResetReleaseOnInit, pstartElem, bp_usXdResetGpio);
            pInfo->nbReset++;
        }
    }

    // retrieve non reset gpio signals
    pstartElem = 0;
    found = 1;
    pInfo->nbGpio = 0;
    while ((pInfo->nbGpio < BP_XDSL_DISTPOINT_MAX_GPIO) && found) {
        bp_elem = BpGetElem(bp_usXdGpio, &pstartElem, bp_last);
        if (bp_elem->id == bp_last) {
            found = 0;
        }
        else {
            pInfo->gpio[pInfo->nbGpio].gpio = pstartElem->u.us;
            pstartElem++;
            pInfo->gpio[pInfo->nbGpio].initValue = BpGetSubUs(
                bp_usXdGpioInitValue, pstartElem, bp_usXdGpio);
            pInfo->gpio[pInfo->nbGpio].info = BpGetSubCp(
                bp_cpXdGpioInfo, pstartElem, bp_usXdGpio);
            pInfo->gpio[pInfo->nbGpio].infoValue0 = BpGetSubCp(
                bp_cpXdGpioInfoValue0, pstartElem, bp_usXdGpio);
            pInfo->gpio[pInfo->nbGpio].infoValue1 = BpGetSubCp(
                bp_cpXdGpioInfoValue1, pstartElem, bp_usXdGpio);
            pInfo->nbGpio++;
        }
    }

    return BP_SUCCESS;
}
#endif
#endif

/**************************************************************************
* Name       : BpGetTrplxrTxFailExtIntr
*
* Description: This function returns the external interrupt number for the
*              triplexer TX fail interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the triplexer tx fail
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTrplxrTxFailExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrTrplxrTxFail, pusValue ) );
} /* BpGetTrplxrTxFailExtIntr */

/**************************************************************************
* Name       : BpGetTrplxrTxFailExtIntrGpio
*
* Description: This function returns the external interrupt GPIO number for
*              triplexer tc fail interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the triplexer tx fail GPIO
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTrplxrTxFailExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrTrplxrTxFail, pusValue);
    
    return( ret );  
} /* BpGetTrplxrTxFailExtIntrGpio */

/**************************************************************************
* Name       : BpGetTrplxrSdExtIntr
*
* Description: This function returns the external interrupt number for the
*              triplexer signal detect interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the triplexer signal detect
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTrplxrSdExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrTrplxrSd, pusValue ) );
} /* BpGetTrplxrSdExtIntr */

/**************************************************************************
* Name       : BpGetTrplxrSdExtIntrGpio
*
* Description: This function returns the external interrupt GPIO number for
*              triplexer signal detect interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the triplexer signal detect GPIO
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTrplxrSdExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrTrplxrSd, pusValue);
    
    return( ret );  
} /* BpGetTrplxrSdExtIntrGpio */

/**************************************************************************
* Name       : BpGetTxLaserOnOutN
*
* Description: This function returns true if tx laser on out n is enabled.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  value is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTxLaserOnOutN( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usTxLaserOnOutN, pusValue ) );
} /* BpGetTxLaserOnOutN */

/**************************************************************************
  * Name       : BpGet1ppsStableGpio
  *
  * Description: This function returns the GPIO pin assignment for 1pps stable
  *
  * Parameters : [OUT] pusValue - Address of short word that the 1pps stable
  *                  GPIO pin is returned in.
  *
  * Returns    : BP_SUCCESS - Success, value is returned.
  *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
  *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
  *                  for the board.
  ***************************************************************************/
int BpGet1ppsStableGpio( unsigned short *pusValue )
    {
        return( BpGetGpio(bp_usGpio1ppsStable, pusValue ) );
    } /* BpGet1ppsStableGpio */


/**************************************************************************
  * Name       : BpGetLteResetGpio
  *
  * Description: This function returns the GPIO pin assignment for LTE reset
  *
  * Parameters : [OUT] pusValue - Address of short word that the LTE reset
  *                  GPIO pin is returned in.
  *
  * Returns    : BP_SUCCESS - Success, value is returned.
  *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
  *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
  *                  for the board.
  ***************************************************************************/
int BpGetLteResetGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioLteReset, pusValue ) );
} /* BpGetLteResetGpio */

/**************************************************************************
  * Name       : BpGetStrapTxEnGpio
  *
  * Description: This function returns the GPIO pin assignment for strap tx enable
  *
  * Parameters : [OUT] pusValue - Address of short word that the strap tx enable
  *                  GPIO pin is returned in.
  *
  * Returns    : BP_SUCCESS - Success, value is returned.
  *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
  *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
  *                  for the board.
  ***************************************************************************/
int BpGetStrapTxEnGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioStrapTxEn, pusValue ) );
} /* BpGetStrapTxEnGpio */

/**************************************************************************
  * Name       : BpGetWanNco10MClk
  *
  * Description: This function returns if Wan NCO 10Mhz clock should be configured. 
  *
  * Parameters : [OUT] pusValue - Address of short word that the value
  *                  is returned in.
  *
  * Returns    : BP_SUCCESS - Success, value is returned.
  *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
  *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
  *                  for the board.
  ***************************************************************************/
int BpGetWanNco10M( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usWanNco10MClk, pusValue ) );
} /* BpGetWanNco10MClk */

/**************************************************************************
  * Name       : BpGetTrxSignalDetect
  *
  * Description: This function returns if Wan trx signal detect should be configured. 
  *
  * Parameters : [OUT] pusValue - Address of short word that the value
  *                  is returned in.
  *
  * Returns    : BP_SUCCESS - Success, value is returned.
  *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
  *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
  *                  for the board.
  ***************************************************************************/
int BpGetTrxSignalDetect( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usTrxSignalDetect, pusValue ) );
} /* BpGetTrxSignalDetect */

/**************************************************************************
* Name       : BpGetWifiOnOffExtIntr
*
* Description: This function returns the external interrupt number for the
*              wifi on off button interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWifiOnOffExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrWifiOnOff, pusValue ) );
} /* BpGetWifiOnOffExtIntr */

/**************************************************************************
* Name       : BpGetWifiOnOffExtIntrGpio
*
* Description: This function returns the external interrupt GPIO number for
*              wifi on off button interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the wifi on off button GPIO
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWifiOnOffExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrWifiOnOff, pusValue);
    
    return( ret );  
} /* BpGetWifiOnOffExtIntrGpio */

/**************************************************************************
* Name       : BpGetLteExtIntr
*
* Description: This function returns the external interrupt number for the
*              LTE interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetLteExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrLTE, pusValue ) );
} /* BpGetLteExtIntr */

/**************************************************************************
* Name       : BpGetLteExtIntrGpio
*
* Description: This function returns the external interrupt GPIO number for
*              LTE interrupt.
*
* Parameters : [OUT] pusValue - Address of short word that the LTE GPIO
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetLteExtIntrGpio( unsigned short *pusValue )
{
    int ret;

    if( !g_pCurrentBp ) 
    {
        *pusValue = BP_NOT_DEFINED;
        ret       = BP_BOARD_ID_NOT_SET;
        return ret;
    }
    
    ret = BpGetExtIntrGpio(bp_usExtIntrLTE, pusValue);
    
    return( ret );  
} /* BpGetLteExtIntrGpio */

/**************************************************************************
* Name       : BpGetMiiInterfaceEn
*
* Description: This function returns true if mii interface is enabled.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  value is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMiiInterfaceEn( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usMiiInterfaceEn, pusValue ) );
} /* BpGetMiiInterfaceEn */

/**************************************************************************
* Name       : BpGetButtonInfo
*
* Description: This function returns information relating to a button
*              definition.  Call this function multiple times to get all 
*              assignments for a boardparm id.
*
* Parameters :  [IN/OUT] token - iterator variable.  Should be 0 the first
*                                time this is called.
*               [OUT] pusIdx    - The button index
*               [OUT] pusGpio   - The gpio to which the button is associated
                                  (in bp format)
*               [OUT] pusExtIrq - The external irq to which the button should
                                  be associated with (in bp format)
                [IN/OUT]pusNumTriggers - In: the maximum number of triggers to
                                  record.
                                  Out: the number of triggers recorded.
*               [OUT] parrusTriggers - An array of triggers (in bp format). 
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_MAX_ITEM_EXCEEDED - no more buttons defined in list
***************************************************************************/
int BpGetButtonInfo( void              **token,
                         unsigned short *pusIdx, 
                         unsigned short *pusGpio,  
                         unsigned short *pusExtIrq,
                         unsigned short *pusNumHooks,
                         unsigned short *parrusHooks,
                         void           **parrptrHookParms
                         )
{
    bp_elem_t *pBtnIdxElem;
    int ret;
    short maxHooks = pusNumHooks?*pusNumHooks:0;
    if (pusNumHooks)
        *pusNumHooks=0;

    pBtnIdxElem = BpGetElem(bp_usButtonIdx, (bp_elem_t **)token, bp_last);
    
    if (bp_usButtonIdx != pBtnIdxElem->id)
    {
        ret = BP_MAX_ITEM_EXCEEDED;
    }
    else
    {
        bp_elem_t *pelem;
        ret = BP_SUCCESS;
        
        *pusIdx=pBtnIdxElem->u.us;

        for (pelem=BpGetNextElem(pBtnIdxElem);pelem != NULL;pelem=BpGetNextElem(pelem)) {
            switch (pelem->id) {
            case bp_usButtonExtIntr:
                *pusExtIrq=pelem->u.us;
                break;
            case bp_usGpio_Intr:
                *pusGpio=pelem->u.us;
                break;
            case bp_usButtonAction:
                if (pusNumHooks == NULL)
                    break;
                if (*pusNumHooks < maxHooks) {
                    parrusHooks[*pusNumHooks]=pelem->u.us;
                    parrptrHookParms[*pusNumHooks]=NULL;
                    (*pusNumHooks)++;
                } else {
                    printk("ERROR: exceeded max hooks for PB_BUTTON_%d (%d/%d)\n",*pusIdx, *pusNumHooks, maxHooks);
                }
                break;
            case bp_ulButtonActionParm:
                if (pusNumHooks == NULL)
                    break;                
                // TBD: switch this to an if statement on the previous case
                if (*pusNumHooks > 0 && *pusNumHooks < maxHooks)
                    parrptrHookParms[*pusNumHooks-1]=pelem->u.ptr;
                else
                    printk("ERROR: parameter mismatch for PB_BUTTON_%d (%d)\n",*pusIdx, *pusNumHooks);                    
                break;
            default:
                goto done;
            }
        }
        done:
        *token=pelem;
    }
    return ret;
} /* BpGetWirelessSesExtIntr */

/**************************************************************************
* Name       : BpGetButtonInfoByIdx
*
* Description: This function returns information relating to a button
*              definition.  Call this function multiple times to get all 
*              assignments for a boardparm id.
*
* Parameters :  [IN] idx - The button index.
*               [OUT] pusIdx    - The button index
*               [OUT] pusGpio   - The gpio to which the button is associated
                                  (in bp format)
*               [OUT] pusExtIrq - The external irq to which the button should
                                  be associated with (in bp format)
                [IN/OUT]pusNumTriggers - In: the maximum number of triggers to
                                  record.
                                  Out: the number of triggers recorded.
*               [OUT] parrusTriggers - An array of triggers (in bp format). 
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_MAX_ITEM_EXCEEDED - no more buttons defined in list
***************************************************************************/
int BpGetButtonInfoByIdx(    unsigned short     btnIdx,
                             unsigned short     *pusIdx, 
                             unsigned short     *pusGpio,  
                             unsigned short     *pusExtIrq,
                             unsigned short     *pusNumHooks,
                             unsigned short     *parrusHooks,
                             void               **parrptrHookParms
                             )
{
    void *iter = NULL;
    unsigned short idx;
    int ret;
    do {
        ret = BpGetButtonInfo(&iter, &idx, pusGpio, pusExtIrq, pusNumHooks, parrusHooks, parrptrHookParms);
        if (ret != BP_SUCCESS)
            return BP_NOT_DEFINED;
        if (btnIdx == idx) {
            *pusIdx = idx;
            return BP_SUCCESS;
        }
    } while(1);
}

/**************************************************************************
* Name       : BpGetUsbPwrOn0
*
* Description: This function returns the UsbPowerOn0 gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetUsbPwrOn0( unsigned short *pusValue )
{
    return( BpGetUs(bp_usUsbPwrOn0, pusValue ) );
} /* BpGetUsbPwrOn0 */

/**************************************************************************
* Name       : BpGetUsbPwrOn1
*
* Description: This function returns the UsbPowerOn1 gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetUsbPwrOn1( unsigned short *pusValue )
{
    return( BpGetUs(bp_usUsbPwrOn1, pusValue ) );
} /* BpGetUsbPwrOn1 */

/**************************************************************************
* Name       : BpGetUsbPwrFlt0
*
* Description: This function returns the UsbPowerFlt0 gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetUsbPwrFlt0( unsigned short *pusValue )
{
    return( BpGetUs(bp_usUsbPwrFlt0, pusValue ) );
} /* BpGetUsbFlt0 */

/**************************************************************************
* Name       : BpGetUsbPwrFlt1
*
* Description: This function returns the UsbPowerFlt1 gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetUsbPwrFlt1( unsigned short *pusValue )
{
    return( BpGetUs(bp_usUsbPwrFlt1, pusValue ) );
} /* BpGetGpioUsbPwrFlt1 */

/**************************************************************************
* Name       : BpGetDHDMemReserve
*
* Description: This function returns the bp_ucDHDMemReserveX value.
*
* Parameters : [OUT] pucValue - Address of byte that the
*                  dhd reserved memory size in MB is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetDHDMemReserve( int index, unsigned char *pucValue )
{
    int nRet;

    switch (index)
    {
        case 0:
            nRet = BpGetUc(bp_ucDHDMemReserve0, pucValue);
            break;
        case 1:
            nRet = BpGetUc(bp_ucDHDMemReserve1, pucValue);
            break;
        case 2:
            nRet = BpGetUc(bp_ucDHDMemReserve2, pucValue);
            break;
        default:
            nRet = BP_VALUE_NOT_DEFINED;
    }

    return nRet;
} /* BpGetDHDMemReserve */


#if !defined(_CFE_)
#if defined(CONFIG_NEW_LEDS)
/**************************************************************************
* Name       : BpGetLedName
*
* Description: This function returns the led name from bp_LedList_str
*
*
* Returns    : Led Name  - Success, Mapped Led name
*              NULL - Failure, cannot map to Led name
***************************************************************************/
int BpGetLedName(int idx, void** token,  unsigned short *pusValue, char **ledName)
{

    int ret=BP_VALUE_NOT_DEFINED, index=0;

    int total = sizeof(bpLedList)/sizeof(enum bp_id);

    if(idx >= total)
        return BP_MAX_ITEM_EXCEEDED;
    else
        ret= BpEnumUs(bpLedList[idx], token, pusValue);

    if(ret == BP_SUCCESS && ledName != NULL) {
        *ledName=NULL;
        while(bpLedList_str[index].bp_name != NULL) {
            if(bpLedList[idx] == bpLedList_str[index].id ) {
                *ledName=bpLedList_str[index].bp_name;
                break;
            }
            index++;
        }
    }
    return ret;
}
#endif

/**************************************************************************
* Name       : BpGetTxDisGpio
*
* Description: This function returns the TxDis gpio number for Merlin SerDes lane.
*
* Parameters :  [IN]  lane index (0-3)
*               [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetTxDisGpio( int lane, unsigned short *pusValue )
{
    switch (lane) {
    case 0:
        return( BpGetGpio(bp_usGpioTxDis0, pusValue ) );
    case 1:
        return( BpGetGpio(bp_usGpioTxDis1, pusValue ) );
    case 2:
        return( BpGetGpio(bp_usGpioTxDis2, pusValue ) );
    case 3:
        return( BpGetGpio(bp_usGpioTxDis3, pusValue ) );
    }

    return BP_VALUE_NOT_DEFINED;
} /* BpGetTxDis */

#endif
/**************************************************************************
* Name       : BpGetLedsAdvancedInfo
*
* Description: This function returns array of all ports Leds configuration.
*
* Parameters : [OUT] pLedsInfo - Address of LED_ADVANCED_INFO buffers.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
int BpGetLedsAdvancedInfo(LEDS_ADVANCED_INFO *pLedsInfo)
{
    int i, j;
    LEDS_ADVANCED_INFO *pE;
    bp_elem_t *pelem;
    bp_elem_t *pPhyType;
    bp_elem_t *pPhyId;
    int nRet = BP_BOARD_ID_NOT_SET;
    enum bp_id bp_aucPhyType[BP_MAX_ENET_MACS+1] = {bp_ucPhyType0, bp_ucPhyType1, bp_last};
    enum bp_id bp_current, bp_next;
    enum bp_id bp_aulPhyId[BP_MAX_SWITCH_PORTS+1] = PHYID_LIST;
    enum bp_id bp_current_phyid;
    int led_index = 0;

    /* First initialize the structure to known values */
    for (i = 0, pE = pLedsInfo; i < BP_MAX_SWITCH_PORTS; i++)
    {
        for ( j = 0; j < 4; j++ )
        {
            pE->ledInfo[i].SpeedLed[j] = 0;
            pE->ledInfo[i].ActivityLed[j] = 0;
        }
    }

    if (g_pCurrentBp)
    {
        /* Populate it with what we have in the element array */
        for( i = 0, pE = pLedsInfo; i < BP_MAX_ENET_MACS; i++ )
        {
            pPhyType = 0;
            bp_current = bp_aucPhyType[i];
            bp_next    = bp_aucPhyType[i+1];
            pelem = BpGetElem(bp_current, &pPhyType, bp_next);
            if (bp_current != pelem->id)
                continue;

            ++pPhyType;

            for( j = 0; j < BP_MAX_SWITCH_PORTS; j++ )
            {
                pPhyId = pPhyType;
                bp_current_phyid = bp_aulPhyId[j];
                pelem = BpGetElem(bp_current_phyid, &pPhyId, bp_next);

                if (bp_current_phyid == pelem->id)
                {
                    ++pPhyId;
                    while (pPhyId) {
                        switch (pPhyId->id) {
                        case bp_usNetLed0:
                            led_index = 0;
                            ++pPhyId;
                            break;
                        case bp_usNetLed1:
                            led_index = 1;
                            ++pPhyId;
                            break;
                        case bp_usNetLed2:
                            led_index = 2;
                            ++pPhyId;
                            break;
                        case bp_usNetLed3:
                            led_index = 3;
                            pE->is_activity_led_present = 1;
                            ++pPhyId;
                            break;
                        case bp_ulNetLedLink:
                            pE->ledInfo[j].SpeedLed[led_index] = pPhyId->u.ul;
                            ++pPhyId;
                            break;
                        case bp_ulNetLedActivity:
                            pE->ledInfo[j].ActivityLed[led_index] = pPhyId->u.ul;
                            ++pPhyId;
                            break;
                        case bp_ulPortFlags:
                        case bp_usOamIndex:
                        case bp_ucPhyDevName:
                            /* skip phy related parameters */
                            ++pPhyId;
                            break;
                        default:
                             if (IsParamPortHwLed( pPhyId->id ) != 0) {
                                 // skip over unknown port parameters that the enet driver doesn't need to know about
                                 ++pPhyId;
                             } else {
                                 pPhyId = 0;
                             }
                             break;
                        }
                    }
                }
            }
        }
        nRet = BP_SUCCESS;
    }

    return( nRet );
} /* BpGetLedsAdvancedInfo */

/**************************************************************************
* Name       : BpGetPciPortDualLane
*
* Description: This function returns 1 if PCI Dual lane mode is enabled, 0 otherwise. Default value is 0
*
* Parameters : [IN] PCI port number
*              [OUT] flag determines whether dual lane is enabled for the given port
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
    
*              
***************************************************************************/
int BpGetPciPortDualLane( int port, int *enabled )
{
    unsigned int flags;
    int rc;

    /* Valid only for port#0 */
    if (port != 0)
        return BP_VALUE_NOT_DEFINED;

    *enabled = 0;
    if ((rc = BpGetUl(bp_ulPciFlags, &flags) != BP_SUCCESS))
        return rc;

    *enabled = (flags & BP_PCI0_DUAL_LANE);
    return BP_SUCCESS;
}  /* BpGetPciPortDualLane */

#if defined(_BCM947189_) || defined(CONFIG_BCM947189)
/**************************************************************************
* Name       : BpGetMoCAResetGpio
*
* Description: This function returns the MoCA reset gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMoCAResetGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioMocaReset, pusValue ) );
}

/**************************************************************************
* Name       : BpGetSpiClkGpio
*
* Description: This function returns the SPI clock gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiClkGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpiClk, pusValue ) );
}

/**************************************************************************
* Name       : BpGetSpiCsGpio
*
* Description: This function returns the SPI chip select gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiCsGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpiCs, pusValue ) );
}

/**************************************************************************
* Name       : BpGetSpiMisoGpio
*
* Description: This function returns the SPI MISO gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiMisoGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpiMiso, pusValue ) );
}

/**************************************************************************
* Name       : BpGetSpiMosiGpio
*
* Description: This function returns the SPI MOSI gpio number.
*
* Parameters : [OUT] pusValue - Address of short word that the
*                  gpio number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiMosiGpio( unsigned short *pusValue )
{
    return( BpGetGpio(bp_usGpioSpiMosi, pusValue ) );
}
#endif 

#ifdef CONFIG_BP_PHYS_INTF

//#define DEBUG_PHYS_INTF

/**************************************************************************
* Physcial interface configuration related the functions    
***************************************************************************/

static PHYS_INTF_INFO physIntfInfo[BP_MAX_PHYS_INTF_PORTS];
static int physIntfCnt[BP_INTF_TYPE_MAX];
static int physIntfTotalCnt = 0;
static int physIntfInitDone = 0;

static int BpCheckPhyIntfIdExist(unsigned short intfId)
{
    int i, rc = 0;

    for( i = 0; i < physIntfTotalCnt; i++ ) {
        if( physIntfInfo[i].intfId == intfId ) {
            rc = 1;
            break;
        }
    }

    return rc;
}

bp_elem_t *BpGetIntfStart(unsigned short intfId)
{
    int i;
    bp_elem_t* pStart = NULL;

    for( i = 0; i < physIntfTotalCnt; i++ ) {
        if( physIntfInfo[i].intfId == intfId ) {
            pStart = physIntfInfo[i].pIntfStart;
            break;
        }
    }

    return pStart;
}

int BpIsPhyIntfInitDone(void)
{
    return physIntfInitDone;
}
/**************************************************************************
* Name       : BpInitPhyIntfInfo
*
* Description: Enumerate all the physical interface in the board paramter and
*              alloate, initialize the PHYS_INTF_INFO structure 
* Parameters : [IN] None
*              [OUT] None
*
* Returns    : BP_SUCCESS - Success, PhyIntfInfo data structure initialized.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called. 
*              BP_VALUE_NOT_DEFINED - Error, incorrect board paramter setting.
*              
***************************************************************************/
int BpInitPhyIntfInfo(void)
{
    int rc = BP_SUCCESS, done = 0, i;
    bp_elem_t *pStartElem = NULL, *pElem;
    PHYS_INTF_INFO* pIntfInfo = &physIntfInfo[0];

   if (!g_pCurrentBp)
        return BP_BOARD_ID_NOT_SET;

    physIntfTotalCnt = 0;
    physIntfInitDone = 0;
    memset((void*)physIntfInfo, 0, sizeof(PHYS_INTF_INFO)*BP_MAX_PHYS_INTF_PORTS);
    memset((void*)physIntfCnt, 0, sizeof(int)*BP_INTF_TYPE_MAX);
    for( i = 0; i < BP_MAX_PHYS_INTF_PORTS; i++ ) {
        physIntfInfo[i].intfId = (unsigned short)(-1);
    }

    while(1) {
        pElem = BpGetElem(bp_usIntfId, &pStartElem, bp_usIntfEnd);
        if( pElem->id !=  bp_usIntfId)
            break; 

        /* check if we have duplicated intfId */
        if( BpCheckPhyIntfIdExist(pElem->u.us) ) {
            if( !BpIsElemFromTemplate(pElem, g_pCurrentBp) ) {
                /* real duplicate id, report error. but bptest should catch that during build */
                printk("duplicated intf id %d detected, intf ignored!\n", pElem->u.us);
#ifdef PINMUXCHECK /* for bp test */
                return BP_INVALID_INTF;
#endif
            }
            /* otherwise it is interface from template being override. just skip it */
            while( (pElem = BpGetNextElem(pElem)) != NULL ) {
                if( pElem->id == bp_usIntfEnd ) {
                    pStartElem = BpGetNextElem(pElem);
                    break;
                }
            }

            if( pElem == NULL || pStartElem == NULL ) {
                physIntfInitDone = 1;
                return rc;
            }
            continue;
        }

        pIntfInfo->intfId = pElem->u.us;
        pIntfInfo->pIntfStart = pElem;

        done = 0;
        for (pElem=BpGetNextElem(pElem);pElem != NULL;pElem=BpGetNextElem(pElem)) {
            switch (pElem->id) {
            case bp_usIntfType:
                pIntfInfo->intfType = pElem->u.us;
                break;
            case bp_usPortNum:
                pIntfInfo->portNum =pElem->u.us;
                break;
            case bp_usIntfEnd:
                pElem=BpGetNextElem(pElem);
                done = 1;
                break;
            default:
                break;
            }

            if (done || pElem->id == bp_usIntfId) /* invalid id, no ending mark bp_usIntfEnd? */
               break;
        }

        if( done == 0 ) {
            rc = BP_INVALID_INTF;
            printk("Missing bp_usIntfEnd for port id %d, interface ignored!!!\r\n", pIntfInfo->intfId);
#ifdef PINMUXCHECK /* for bp test */
            return rc;
#endif
        } else {
            if( pIntfInfo->intfType >= BP_INTF_TYPE_MAX ) {
                rc = BP_INVALID_INTF;
                printk("Invalid or missing type %d for port id %d, interface ignored!!!\r\n", 
                    pIntfInfo->intfType, pIntfInfo->intfId);
#ifdef PINMUXCHECK /* for bp test */
                return rc;
#endif
            } else {
                physIntfCnt[pIntfInfo->intfType]++;
                physIntfTotalCnt++;
                pIntfInfo++;
            }
        }

        if( physIntfTotalCnt > BP_MAX_PHYS_INTF_PORTS ) {
            printk("Exceeded max supported interface number %d!!!\r\n", BP_MAX_PHYS_INTF_PORTS);
            rc = BP_INVALID_INTF;
#ifdef PINMUXCHECK /* for bp test */
            return rc;
#endif
            break;
        }

        if( pElem == NULL)
            break;
        pStartElem = pElem;
    }
 
#ifdef DEBUG_PHYS_INTF
    {
    int i = 0, j;
    PHYS_INTF_ADV_LEDS_INFO ledInfo[BP_MAX_PHYS_INTF_PORTS];
    int cnt = BP_MAX_PHYS_INTF_PORTS;

    printk("Total phys intf cnt %d\n", physIntfTotalCnt);
    for( i = 0; i < BP_INTF_TYPE_MAX; i++ )
        printk("type %d - cnt %d\n", i, physIntfCnt[i]);

    for( i = 0; i < physIntfTotalCnt; i++ )
        printk("intf %d id %d type %d port num %d, bp start %p\n", 
          i, physIntfInfo[i].intfId, physIntfInfo[i].intfType, physIntfInfo[i].portNum,
          physIntfInfo[i].pIntfStart);

    BpGetAllAdvLedInfo(ledInfo, &cnt);
    printk("%d intf with adv led info\n", cnt);
    for( i = 0; i < cnt; i++ )
    {
        printk("intf %d, id %d type %d, port %d\n", i, ledInfo[i].pIntf->intfId, ledInfo[i].pIntf->intfType, ledInfo[i].pIntf->portNum);
        for( j = 0; j < MAX_LEDS_PER_PORT; j++ ) 
        {
            printk("netled%d, speed %d, act %d\n",  j, ledInfo[i].SpeedLed[j], ledInfo[i].ActivityLed[j]);
        }
    }
    }
#endif
    physIntfInitDone = 1;
    return rc;
}

/**************************************************************************
* Name       : BpGetAllPhyIntfInfo
*
* Description: Get all the phy interface info
* 
* Parameters : [IN] None
*              [OUT] total number of interface
*
* Returns    : pointer to the first physical interface structure.
*              
***************************************************************************/
PHYS_INTF_INFO* BpGetAllPhyIntfInfo(int *totalIntfNum)
{
    *totalIntfNum =  physIntfTotalCnt;
    if( physIntfTotalCnt == 0 )
        return NULL;
    else
        return &physIntfInfo[0];
}

/**************************************************************************
* Name       : BpGetPhyIntfInfo
*
* Description: Get the phy interface info by the interface number
* 
* Parameters : [IN] interface index
*              [OUT] None
*
* Returns    : pointer to the physical interface structure.
*              
***************************************************************************/
PHYS_INTF_INFO* BpGetPhyIntfInfo(int intfIdx)
{
    if( intfIdx >= physIntfTotalCnt )
        return NULL;

    return &physIntfInfo[intfIdx];
}

/**************************************************************************
* Name       : BpGetPhyIntfNumByType
*
* Description: Get the phy interface count for a type 
* 
* Parameters : [IN] interface type
*              [OUT] None
*
* Returns    : number of interface for this type.
*              
***************************************************************************/
unsigned short BpGetPhyIntfNumByType(unsigned short type)
{
    if( type >= BP_INTF_TYPE_MAX )
        return 0;
    else 
        return physIntfCnt[type];
}

/**************************************************************************
* Name       : BpGetPhyIntfInfoByType
*
* Description: Get the phy interface info for a type 
* 
* Parameters : [IN] interface type and intf number
*              [OUT] None
*
* Returns    : pointer to the physial interface info.
*              
***************************************************************************/
PHYS_INTF_INFO* BpGetPhyIntfInfoByType(unsigned short type, int intfIdx)
{
    int i;
    PHYS_INTF_INFO*  pPhysIntfInfo = NULL;
    int index = 0;

    if( type >= BP_INTF_TYPE_MAX )
        return NULL;

    if( intfIdx >= physIntfCnt[type] )
        return NULL;

    pPhysIntfInfo = &physIntfInfo[0];
    for( i = 0; i < physIntfTotalCnt; i++ ) {
        if( pPhysIntfInfo->intfType == type ) {
            if( index == intfIdx ) 
                break;
            index++;
        }
        pPhysIntfInfo++;
    }

    return pPhysIntfInfo;
}

static int BpGetGpioInIntf(enum bp_id id, PHYS_INTF_INFO* pIntf, unsigned short *pusValue )
{
    int nRet;

    nRet = BpGetUsInIntf(id, pusValue, pIntf );
    if(nRet == BP_SUCCESS && *pusValue == BP_GPIO_NONE)
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_VALUE_NOT_DEFINED;
    }

    return( nRet );
}

int BpGetExtIntrGpioInIntf(enum bp_id id, PHYS_INTF_INFO* pIntf, unsigned short *pusValue )
{
    bp_elem_t *pelem, *pstart;
    int ret;
  
    pstart = pIntf->pIntfStart;
    /* First go to elem with interrupt id */
    pelem = BpGetElem(id, &pstart , bp_usIntfEnd);
    if (id != pelem->id)
    {
        *pusValue = BP_NOT_DEFINED;
        ret = BP_VALUE_NOT_DEFINED;
    }
    else
    {
        pelem++;
        /* We should expect the id bp_usGpio_Intr in the next element */
        if (bp_usGpio_Intr != pelem->id)
        {
            *pusValue = BP_NOT_DEFINED;
            ret = BP_VALUE_NOT_DEFINED;
        }
        else
        {
            *pusValue = pelem->u.us;
            ret = BP_SUCCESS;
        }
    }

    return ret;
}

/**************************************************************************
* Name       : BpGetAFELDClkGpio
*
* Description: This function returns the GPIO pin assignment for AFE LD CLK signal
*              for a  xDSL port
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDClk
*                  GPIO pin is returned in.
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
*
**************************************************************************/
int BpGetAFELDClkGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFELDClk, pDSLIntf, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDModeGpio
*
* Description: This function returns the GPIO pin assignment for AFE LD Mode signal
*              for a  xDSL port
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDMode
*                  GPIO pin is returned in.
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
*
**************************************************************************/
int BpGetAFELDModeGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFELDMode, pDSLIntf, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDPwrGpio
*
* Description: This function returns the GPIO pin assignment for AFE LD Pwr signal
*              for a  xDSL port
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDPwr
*                  GPIO pin is returned in.
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
*
**************************************************************************/
int BpGetAFELDPwrGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFELDPwr, pDSLIntf, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDDataGpio
*
* Description: This function returns the GPIO pin assignment for AFE LD Data signal
*              for a  xDSL port
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDData
*                  GPIO pin is returned in.
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
*
**************************************************************************/
int BpGetAFELDDataGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFELDData, pDSLIntf, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDClkDslCtl
*
* Description: This function returns the DSL CTRL number for AFE LD Clock pin
*
*
* Parameters : [OUT] pusValue - DSL Control number
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetAFELDClkDslCtl( int intfIdx, unsigned short *pusValue )
{
    return( BpGetDslCtl(bp_usGpioAFELDClk, intfIdx, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDModeDslCtl
*
* Description: This function returns the DSL CTRL number for AFE LD Mode pin
*
*
* Parameters : [OUT] pusValue - DSL Control number
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetAFELDModeDslCtl( int intfIdx, unsigned short *pusValue )
{
    return( BpGetDslCtl(bp_usGpioAFELDMode, intfIdx, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDPwrDslCtl
*
* Description: This function returns the DSL CTRL number for AFE LD Pwr pin
*
*
* Parameters : [OUT] pusValue - DSL Control number
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetAFELDPwrDslCtl( int intfIdx, unsigned short *pusValue )
{
    return( BpGetDslCtl(bp_usGpioAFELDPwr, intfIdx, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFELDDataDslCtl
*
* Description: This function returns the DSL CTRL number for AFE LD Data pin
*
*
* Parameters : [OUT] pusValue - DSL Control number
*              [IN] intfIdx - DSL port index
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetAFELDDataDslCtl( int intfIdx, unsigned short *pusValue )
{
    return( BpGetDslCtl(bp_usGpioAFELDData, intfIdx, pusValue ) );
}

/**************************************************************************
* Name       : BpGetAFEResetGpio
*
* Description: This function returns the GPIO pin assignment for resetting the AFE chip
*
*
* Parameters : [OUT] pusValue - Address of short word that the AFEReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetAFEResetGpio( int intfIdx, unsigned short *pusValue )
{
    PHYS_INTF_INFO*  pDSLIntf = NULL;

    if( (pDSLIntf = BpGetPhyIntfInfoByType(BP_INTF_TYPE_xDSL, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioAFEReset, pDSLIntf, pusValue ) );
}

int BpGetWanActLedGpio(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioLedWanAct, pIntf, pusValue ) );
}

int BpGetWanErrLedGpio(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioLedWanErr, pIntf, pusValue ) );
}

int BpGetWanLinkLedGpio(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioLedWanLink, pIntf, pusValue ) );
}

int BpGetWanLinkFailLedGpio(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioLedWanLinkFail, pIntf, pusValue ) );
}

int BpGetIntfPortNum(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return BpGetUsInIntf(bp_usPortNum, pusValue, pIntf );
}


int BpGetIntfMgmtType(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return BpGetUsInIntf(bp_usIntfMgmtType, pusValue, pIntf );
}

int BpGetIntfMgmtBusNum(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return BpGetUsInIntf(bp_usIntfMgmtBusNum, pusValue, pIntf );
}

int BpGetSfpModDetectGpio(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return( BpGetGpioInIntf(bp_usGpioSfpModDetect, pIntf, pusValue ) );
}

int BpGetSfpSigDetect(int type, int intfIdx, unsigned short *pusValue)
{
    PHYS_INTF_INFO*  pIntf = NULL;

    if( (pIntf = BpGetPhyIntfInfoByType(type, intfIdx)) == NULL ) {
        *pusValue = BP_NOT_DEFINED;
        return BP_VALUE_NOT_DEFINED;
    }

    return BpGetUsInIntf(bp_usSfpSigDetect, pusValue, pIntf );
}

static int BpGetAdvLedInfoInIntf(PHYS_INTF_INFO* pIntf, PHYS_INTF_ADV_LEDS_INFO* ledInfo)
{
    int i, nRet = BP_SUCCESS, noLed = 1;
    bp_elem_t *pElemNet;

    ledInfo->pIntf = pIntf;
    for ( i = 0; i < MAX_LEDS_PER_PORT; i++ )
    {
        ledInfo->SpeedLed[i] = BP_NOT_DEFINED;
        ledInfo->ActivityLed[i] = BP_NOT_DEFINED;
        ledInfo->LedSettings[i] = BP_NOT_DEFINED;
    }

    pElemNet = pIntf->pIntfStart;
    while( pElemNet->id != bp_usIntfEnd ){
        switch( pElemNet->id ) {
        case bp_usNetLed0:
            i = 0;
            break;
        case bp_usNetLed1:
            i = 1;
            break;
        case bp_usNetLed2:
            i = 2;
            break;
        case bp_usNetLed3:
            i = 3;
            break;
        case bp_ulNetLedLink:
            ledInfo->SpeedLed[i] = pElemNet->u.us;
            break;
        case bp_ulNetLedActivity:
            ledInfo->ActivityLed[i] = pElemNet->u.us;
            break;
        case bp_ulNetLedSetting:
            ledInfo->LedSettings[i] = pElemNet->u.ul;
            break;
 
        default:
            break;
        }

        pElemNet++;
    }

    /* if any of the port has a led speed/activity, this intf has adv led */
    for ( i = 0; i < MAX_LEDS_PER_PORT; i++ ) {
        if( ledInfo->SpeedLed[i] != BP_NOT_DEFINED || ledInfo->ActivityLed[i] != BP_NOT_DEFINED ) 
            noLed = 0;
    }

    if( noLed )
        nRet = BP_VALUE_NOT_DEFINED;

    return nRet;
}

int BpGetAllAdvLedInfo(PHYS_INTF_ADV_LEDS_INFO* ledInfo, int* numEntry)
{
    int i, maxCnt = *numEntry, totalCnt = 0;
    PHYS_INTF_INFO* pIntf;
    PHYS_INTF_ADV_LEDS_INFO led;
    int rc;

    for( i = 0; i < physIntfTotalCnt && totalCnt < maxCnt; i++ ) {
        pIntf =  BpGetPhyIntfInfo(i);
        rc = BpGetAdvLedInfoInIntf(pIntf, &led);
        if( rc == BP_SUCCESS ) {
            *ledInfo = led;
            ledInfo++;
            totalCnt++;
	}
    }

    *numEntry = totalCnt;
    return BP_SUCCESS;
}

int BpGetAdvLedInfo(unsigned short type, int intfIdx, PHYS_INTF_ADV_LEDS_INFO* ledInfo)
{
    PHYS_INTF_INFO* pIntf;

    pIntf =  BpGetPhyIntfInfoByType(type, intfIdx);
    if( pIntf )
        return BpGetAdvLedInfoInIntf(pIntf, ledInfo);
    else
        return BP_VALUE_NOT_DEFINED;
}
#endif


int BpGetUsbDis(unsigned short *pusValue)
{
    return BpGetUs(bp_usUsbDis, pusValue);
}

int BpGetPciDis(unsigned short *pusValue)
{
    return BpGetUs(bp_usPciDis, pusValue);
}

int BpGetSataDis(unsigned short *pusValue)
{
    return BpGetUs(bp_usSataDis, pusValue);
}

