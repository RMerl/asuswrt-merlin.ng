/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  Board device initialization      File: bcm94710_devs.c
    *
    *  This is the "C" part of the board support package.  The
    *  routines to create and initialize the console, wire up
    *  device drivers, and do other customization live here.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *
    *********************************************************************
    *
    *  XX Copyright 2000,2001
    *  Broadcom Corporation. All rights reserved.
    *
    *  BROADCOM PROPRIETARY AND CONFIDENTIAL
    *
    *  This software is furnished under license and may be used and
    *  copied only in accordance with the license.
    ********************************************************************* */

#include "lib_types.h"
#include "lib_printf.h"
#include "cfe_timer.h"
#include "cfe.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "dev_bcm63xx_flash.h"
#include "bcm63xx_util.h"
#include "flash_api.h"
#include "exception.h"
#include "shared_utils.h"
#include "btrm_if.h"
#include "bcm_otp.h"

#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM963381_) || defined (_BCM96848_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM968360_)
#if !defined (_BCM96848_)
#include "pmc_drv.h"
#include "BPCM.h"
#include "bcm_misc_hw_init.h"
#endif
#include "bcm_gpio.h"
#include "bcm_pinmux.h"
#endif
#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM963381_)\
    || defined (_BCM960333_) || defined (_BCM96848_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM968360_)
#include "bcm_led.h"
#endif

#if defined (_BCM947189_)
#include "bcm_misc_hw_init.h"
#endif

#if defined (_BCM96848_)
#include "phys_common_drv.h"
#endif

#ifdef AC2900
#include "bcm63xx_nvram.h"
#endif

#if (INC_SPI_PROG_NAND==0)
#if defined(_BCM94908_)
static int RstBtnPressed(void);
static int checkForSesBtnWirelessHold( void );
static void setPowerOnLedOff(void);
#else
 static int checkForResetToDefaultHold( unsigned short rstToDfltIrq );
#endif
#endif


/*  *********************************************************************
    *  Devices we're importing
    ********************************************************************* */

#if defined (_BCM96838_) || defined (_BCM963138_) || defined (_BCM963148_) || defined(_BCM96848_) || defined (_BCM96858_) || defined (_BCM968360_)
extern cfe_driver_t bcm63xx_uart;
#if (NONETWORK==0)
#if defined (_BCM96858_)
extern cfe_driver_t bcm6xxx_impl3_enet;
#else
extern cfe_driver_t bcm6xxx_impl2_enet;
#endif
#endif
#elif defined (_BCM947189_)
extern cfe_driver_t bcm63xx_uart;
#if (NONETWORK==0)
extern cfe_driver_t bcm47189_enet;
#endif
#else
extern cfe_driver_t bcm63xx_uart;
extern cfe_driver_t bcm63xx_enet;
#endif

#if defined(_BCM963381_)
extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
void second_cpu_icache_fixup(void);

void second_cpu_icache_fixup(void)
{
    _cfe_flushcache(CFE_CACHE_INVAL_I,0,0);

    /* put the TP1 in sleep again for linux to bring up TP1 */
    asm(
"1: \n"
    "sync \n"
    "wait \n"
    "b  1b \n"
    "nop \n"
    ::);

    return;
}
#endif

/*  *********************************************************************
    *  board_console_init()
    *
    *  Add the console device and set it to be the primary
    *  console.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void board_console_init(void)
{
    /* Add the serial port driver. */
#if defined (_BCM960333_)
    /* Minimum GPIO_MUX setup to enable GPIOs for UART
     * Map GPIOs 4 & 5 according to GPIO_MUX (default 4: UARTrx, 5: UARTtx)
     */
    GPIO->GPIOFuncMode |= 0x30;
#endif
    cfe_add_device(&bcm63xx_uart,0,0,0);

    cfe_set_console( "uart0" );
}

#if defined(_BCM963138_) && !defined(CONFIG_BRCM_IKOS)
static void bump_nand_phase(int n) {
    int i;
    for (i = 0 ; i < n ; i++) {
        PROCMON->PMBM[1].wr_data = 0x081e000a;
        PROCMON->PMBM[1].ctrl = 0x8010a00d;
        while (PROCMON->PMBM[1].ctrl & 0x10000000) { };
        PROCMON->PMBM[1].wr_data = 0x001e000a;
        PROCMON->PMBM[1].ctrl = 0x8010a00d;
        while (PROCMON->PMBM[1].ctrl & 0x10000000) { };
    }
    printf(".");
}

static int check_nand_phase(int n) {
    unsigned long i;
    for (i = 0; (i >> n) < 10000 ; i++) {
        if (((NAND->NandRevision & ~0xff) != 0x80000700) 
           ||  ((NAND->NandCmdStart & ~0xff) == 0x80000700)) {
            printf("p%d.\n",i);
            return(1);
        }
    }
    return(0);
}
#endif

/* FIXMET: implement for 68360 */
#if defined (_BCM96858_)
/*this function is used to turn on CCI from secure mode 
  it also turns snooping enable for S3 (XRDP)*/
static void cci400_enable(void)
{
    uint32_t *cci_secure = (uint32_t*)(CCI400_PHYS_BASE + CCI400_SECURE_ACCESS_REG_OFFSET);
    uint32_t *cci_s2_share = (uint32_t*)(CCI400_PHYS_BASE + CCI400_S2_SAHREABLE_OVERRIDE_REG_OFFSET);
    uint32_t *cci_s3_snoop = (uint32_t*)(CCI400_PHYS_BASE + CCI400_S3_SNOOP_CTRL_REG_OFFSET);

    /*Enable access from E2 and below */
    *cci_secure = CCI400_SECURE_ACCESS_UNSECURE_ENABLE;

    /*config XRDP for CCI */
    *cci_s2_share = CCI400_S2_SAHREABLE_OVERRIDE_AX_DOMAIN;
    *cci_s3_snoop |= CCI400_S3_SNOOP_CTRL_EN_SNOOP;
}

#endif

/*  *********************************************************************
    *  board_device_init()
    *
    *  Initialize and add other devices.  Add everything you need
    *  for bootstrap here, like disk drives, flash memory, UARTs, etc
    *  BUT NOT network controllers.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void board_device_init(void)
{
#if !defined (_BCM96838_) && !defined(_BCM963138_) && !defined(_BCM960333_) && !defined(_BCM963381_) && !defined(_BCM963148_) && !defined (_BCM94908_) && !defined(_BCM96848_) && !defined(_BCM96858_) && !defined(_BCM947189_) && !defined(_BCM968360_)
    unsigned int GPIOOverlays;
#endif
#if defined (_BCM963268_) ||  defined (_BCM96318_)
    const ETHERNET_MAC_INFO *EnetInfo;
#endif
#if defined (_BCM963268_) ||  defined (_BCM96328_) ||   defined (_BCM96362_)
    unsigned int DeviceOptions = 0;
#endif
#if defined (_BCM96318_)
    unsigned short SerialMuxSel = 0;
    unsigned short plcGpio = 0;
    unsigned int chipRev = UtilGetChipRev();
#endif
#if defined (_BCM96838_)
    unsigned short MuxSel = 0;
    char    boardIdStr[BP_BOARD_ID_LEN];
#endif

#if defined(_BCM963138_) && !defined(CONFIG_BRCM_IKOS)
    int i = 0;
    if( UtilGetChipRev() == 0xa0 ) {
        printf("CHECKING NAND Phase\n");
        while (!check_nand_phase(0)) {
            bump_nand_phase(1);
            printf("B0");
            if (i++ > 60) {
                break;
            }
        }
        if (i <= 60) {
           while (check_nand_phase(4)) {
               bump_nand_phase(3);
               printf("B1");
           }
           bump_nand_phase(6);
           printf("B2");
        }
        printf("\n");
    }
#endif

#if defined(_BCM963138_) || ((defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963381_) || defined(_BCM968360_)) && (INC_SPI_PROG_NAND==1))
    /* assume all the 63138 board does use nand pin for other function */ 
    bcm_init_pinmux_interface(BP_PINMUX_FNTYPE_NAND);    
#endif

    kerSysFlashInit();

#if defined (_BCM96328_)
    if( BpGetDeviceOptions(&DeviceOptions) == BP_SUCCESS ) {
        if(DeviceOptions&BP_DEVICE_OPTION_DISABLE_LED_INVERSION)
            MISC->miscLedXorReg = 0;
    }

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {
        /* Start with all HW LEDs disabled */
        LED->ledHWDis |= 0xFFFFFF;
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->PinMuxSel |= PINMUX_SERIAL_LED_DATA;
            GPIO->PinMuxSel |= PINMUX_SERIAL_LED_CLK;
            LED->ledInit |= LED_SERIAL_LED_EN;
        }
        /* Enable LED controller to drive GPIO */
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            GPIO->PinMuxSel |= PINMUX_EPHY0_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY0_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY0_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            GPIO->PinMuxSel |= PINMUX_EPHY1_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY1_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY1_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            GPIO->PinMuxSel |= PINMUX_EPHY2_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY2_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY2_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_3) {
            GPIO->PinMuxSel |= PINMUX_EPHY3_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY3_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY3_SPD_LED);
        }
    }

    /* These blocks will be enabled by the appropriate driver if they are
     * compiled into a router image.
     */
    PERF->blkEnables &= ~(USBH_CLK_EN | PCM_CLK_EN | USBD_CLK_EN | PCIE_CLK_EN);
    MISC->miscSerdesCtrl &= ~(SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
    PERF->softResetB &= ~(SOFT_RST_PCIE|SOFT_RST_PCIE_EXT|SOFT_RST_PCIE_CORE|SOFT_RST_PCIE_HARD);
    
    {
         unsigned short vregSel1P2;

         /* Set 1V2 on voltage regulator if specifiec in boardparms */
         if( BpGetVregSel1P2(&vregSel1P2) == BP_SUCCESS ) {
             MISC->miscVregCtrl0 = (MISC->miscVregCtrl0 & ~VREG_VSEL1P2_MASK) | (vregSel1P2 << VREG_VSEL1P2_SHIFT);
         }
    }
#endif

#if defined (_BCM96362_)
    if( BpGetDeviceOptions(&DeviceOptions) == BP_SUCCESS ) {
        if(DeviceOptions&BP_DEVICE_OPTION_DISABLE_LED_INVERSION)
            MISC->miscLed_inv = 0;
    }

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {
        /* Start with all HW LEDs disabled */
        LED->ledHWDis |= 0xFFFFFF;
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
            LED->ledInit |= LED_SERIAL_LED_EN;
        }
        /* Map HW LEDs to LED controller inputs and enable LED controller to drive GPIO */
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET0 - 4)) << LED_4_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET0);
            LED->ledHWDis &= ~(1 << LED_ENET0);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET1 - 4)) << LED_5_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET1);
            LED->ledHWDis &= ~(1 << LED_ENET1);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET2 - 4)) << LED_6_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET2);
            LED->ledHWDis &= ~(1 << LED_ENET2);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_3) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET3 - 4)) << LED_7_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET3);
            LED->ledHWDis &= ~(1 << LED_ENET3);
        }
        /* Enable Overlay for SPI SS Pins */            
        if (GPIOOverlays & BP_OVERLAY_SPI_SSB2_EXT_CS) {           
             GPIO->GPIOMode |= GPIO_MODE_LS_SPIM_SSB2;                    
        }
        if (GPIOOverlays & BP_OVERLAY_SPI_SSB3_EXT_CS) {           
             GPIO->GPIOMode |= GPIO_MODE_LS_SPIM_SSB3;                    
        }
    }

    /* These blocks will be enabled by the appropriate driver if they are
     * compiled into a router image.
     */
    PERF->blkEnables &= ~(USBH_CLK_EN | PCM_CLK_EN | SAR_CLK_EN | USBD_CLK_EN | IPSEC_CLK_EN);
    
    {
         unsigned short vregSel1P2;


         /* Set 1V2 on voltage regulator if specifiec in boardparms */
         if( BpGetVregSel1P2(&vregSel1P2) == BP_SUCCESS ) {
             MISC->miscVregCtrl1 = (MISC->miscVregCtrl1 & ~VREG_VSEL1P2_MASK) | (vregSel1P2 << VREG_VSEL1P2_SHIFT);
         }
    }
#endif

#if defined (_BCM963268_)
    if( BpGetDeviceOptions(&DeviceOptions) == BP_SUCCESS ) {
        if(DeviceOptions&BP_DEVICE_OPTION_DISABLE_LED_INVERSION)
            MISC->miscLed_inv = 0;
    }

    /* If previously disabled, Robosw clock needs to be manually re-enabled on soft reboot */
    PERF->blkEnables |= ROBOSW250_CLK_EN;

    /* Turn off test bus */
    PERF->blkEnables &= ~TBUS_CLK_EN;

    /* Start with all HW LEDs disabled */
    LED->ledHWDis |= 0xFFFFFF;
    LED->ledMode = 0;

    EnetInfo = BpGetEthernetMacInfoArrayPtr();

    /* Enable HW to drive LEDs for Ethernet ports in use */
    if (EnetInfo[0].sw.port_map & (1 << 0)) {
        LED->ledHWDis &= ~(1 << LED_EPHY0_ACT);
        LED->ledHWDis &= ~(1 << LED_EPHY0_SPD);
    }
    if (EnetInfo[0].sw.port_map & (1 << 1)) {
        LED->ledHWDis &= ~(1 << LED_EPHY1_ACT);
        LED->ledHWDis &= ~(1 << LED_EPHY1_SPD);
    }
    if (EnetInfo[0].sw.port_map & (1 << 2)) {
        LED->ledHWDis &= ~(1 << LED_EPHY2_ACT);
        LED->ledHWDis &= ~(1 << LED_EPHY2_SPD);
    }
    if (EnetInfo[0].sw.port_map & (1 << 3)) {
        LED->ledHWDis &= ~(1 << LED_GPHY0_ACT);
        LED->ledHWDis &= ~(1 << LED_GPHY0_SPD0);
        LED->ledHWDis &= ~(1 << LED_GPHY0_SPD1);
        LED->ledLinkActSelLow |= ((1 << LED_GPHY0_SPD0) << LED_0_LINK_SHIFT);
        LED->ledLinkActSelLow |= ((1 << LED_GPHY0_SPD1) << LED_1_LINK_SHIFT);
        GPIO->RoboSWLEDControl |= LED_BICOLOR_SPD;
    }

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
            LED->ledInit |= LED_SERIAL_LED_EN;
        }
        /* Enable LED controller to drive GPIO when LEDs are connected to GPIO pins */
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            GPIO->LEDCtrl |= (1 << LED_EPHY0_ACT);
            GPIO->LEDCtrl |= (1 << LED_EPHY0_SPD);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            GPIO->LEDCtrl |= (1 << LED_EPHY1_ACT);
            GPIO->LEDCtrl |= (1 << LED_EPHY1_SPD);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            GPIO->LEDCtrl |= (1 << LED_EPHY2_ACT);
            GPIO->LEDCtrl |= (1 << LED_EPHY2_SPD);
        }
        if (GPIOOverlays & BP_OVERLAY_GPHY_LED_0) {
            GPIO->LEDCtrl |= (1 << LED_GPHY0_ACT);
            GPIO->LEDCtrl |= (1 << LED_GPHY0_SPD0);
            GPIO->LEDCtrl |= (1 << LED_GPHY0_SPD1);
        }
        /* Enable HS SPI SS Pins */
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB4_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_4;
        }
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB5_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_5;
        }
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB6_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_6;
        }
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB7_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_7;
        }
    }

    {
        unsigned short PhyBaseAddr;
        /* clear the base address first. hw does not clear upon soft reset*/
        GPIO->RoboswEphyCtrl &= ~EPHY_PHYAD_BASE_ADDR_MASK;
        if( BpGetEphyBaseAddress(&PhyBaseAddr) == BP_SUCCESS ) {
            GPIO->RoboswEphyCtrl |= ((PhyBaseAddr >>3) & 0x3) << EPHY_PHYAD_BASE_ADDR_SHIFT;
        }

        /* clear the base address first. hw does not clear upon soft reset*/
        GPIO->RoboswGphyCtrl &= ~GPHY_PHYAD_BASE_ADDR_MASK;
        if( BpGetGphyBaseAddress(&PhyBaseAddr) == BP_SUCCESS ) {
            GPIO->RoboswGphyCtrl |= ((PhyBaseAddr >>3) & 0x3) << GPHY_PHYAD_BASE_ADDR_SHIFT;
        }
    }
#endif


#if defined (_BCM96318_)
    /* set the PLC pin to a known state. This can be done with a function if more boards
     support PLC */
    if( BpGetPLCPwrEnGpio(&plcGpio) == BP_SUCCESS )
    {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(plcGpio);
        /* set active on power enable */
        if( plcGpio & BP_ACTIVE_LOW)
            GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(plcGpio);
        else
            GPIO->GPIOio |= GPIO_NUM_TO_MASK(plcGpio);
    }

    if( BpGetPLCResetGpio(&plcGpio) == BP_SUCCESS )
    {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(plcGpio);
        /* set inactive for plc reset */
        if( plcGpio & BP_ACTIVE_LOW)
            GPIO->GPIOio |= GPIO_NUM_TO_MASK(plcGpio);
        else
            GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(plcGpio);
    }

    initGpioPinMux();

    /* Start with all HW LEDs disabled */
    LED->ledHWDis |= 0xFFFFFF;
    LED->ledMode = 0;

    EnetInfo = BpGetEthernetMacInfoArrayPtr();

    /* Enable HW to drive LEDs for Ethernet ports in use */
    if (EnetInfo[0].sw.port_map & (1 << 0)) {
        LED->ledHWDis &= ~(1 << EPHY0_SPD_LED);
        LED->ledHWDis &= ~(1 << EPHY0_ACT_LED);
        /* set up link and speed mapping */
        LED->ledLinkActSelLow |= ((1<<(EPHY0_ACT_LED-4))<<LED_0_LINK_SHIFT);
        LED->ledLinkActSelHigh |= ((1<<(EPHY0_ACT_LED-4))<<LED_0_LINK_SHIFT);

        /* workaround for a0 hw which invert the active low to active high */
        if(chipRev == 0xa0)
        {
            LED->ledXorReg |= (1 << EPHY0_SPD_LED);
            LED->ledXorReg |= (1 << EPHY0_ACT_LED);
        }
    }
    if (EnetInfo[0].sw.port_map & (1 << 1)) {
        LED->ledHWDis &= ~(1 << EPHY1_SPD_LED);
        LED->ledHWDis &= ~(1 << EPHY1_ACT_LED);
        LED->ledLinkActSelLow |= ((1<<(EPHY1_ACT_LED-4))<<LED_1_LINK_SHIFT);
        LED->ledLinkActSelHigh |= ((1<<(EPHY1_ACT_LED-4))<<LED_1_LINK_SHIFT);
        if(chipRev == 0xa0)
        {
            LED->ledXorReg |= (1 << EPHY1_SPD_LED);
            LED->ledXorReg |= (1 << EPHY1_ACT_LED);
        }
    }
    if (EnetInfo[0].sw.port_map & (1 << 2)) {
        LED->ledHWDis &= ~(1 << EPHY2_SPD_LED);
        LED->ledHWDis &= ~(1 << EPHY2_ACT_LED);
        LED->ledLinkActSelLow |= ((1<<(EPHY2_ACT_LED-4))<<LED_2_LINK_SHIFT);
        LED->ledLinkActSelHigh |= ((1<<(EPHY2_ACT_LED-4))<<LED_2_LINK_SHIFT);
        if(chipRev == 0xa0)
        {
            LED->ledXorReg |= (1 << EPHY2_SPD_LED);
            LED->ledXorReg |= (1 << EPHY2_ACT_LED);
        }
    }
    if (EnetInfo[0].sw.port_map & (1 << 3)) {
        LED->ledHWDis &= ~(1 << EPHY3_SPD_LED);
        LED->ledHWDis &= ~(1 << EPHY3_ACT_LED);
        LED->ledLinkActSelLow |= ((1<<(EPHY3_ACT_LED-4))<<LED_3_LINK_SHIFT);
        LED->ledLinkActSelHigh |= ((1<<(EPHY3_ACT_LED-4))<<LED_3_LINK_SHIFT);
        if(chipRev == 0xa0)
        {
            LED->ledXorReg |= (1 << EPHY3_SPD_LED);
            LED->ledXorReg |= (1 << EPHY3_ACT_LED);
        }
    }

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) 
    {      
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (1 << SERIAL_LED_DATA);
            GPIO->GPIOMode |= (1 << SERIAL_LED_CLK);

            /* enable shift only on led status change mode to solve the dim led issue. only available in b0 or newer chip */
            if(chipRev != 0xa0)
            {
                LED->ledInit &= ~LED_SERIAL_SHIFT_MODE_MASK;
                LED->ledInit |= LED_SERIAL_SHIFT_MODE_CHANGE;
            }

            LED->ledSerialMuxSelect = 0x0;
            if( BpGetSerialLEDMuxSel(&SerialMuxSel) == BP_SUCCESS )
            {
                if( SerialMuxSel == (BP_SERIAL_MUX_SEL_GROUP0|BP_SERIAL_MUX_SEL_GROUP2) )
                    LED->ledSerialMuxSelect = 0xff;
                    /* otherwise either non supported combination or default 8 to 23 LED*/
            }

            /* For default Serial MUX selection, XOR workaround is not needed for EPHY 3 SPD and ACT
             * as EPHY 3 LED function is not available anyway. Otherwise, serial data/clk will be inverted too.
             * But for non default Serial MUX selection, we need it to make all EPHY LINK/SPD LED work.
             * However LED 16 to 23 are inverted too. Will fix in next hw revision */
            if(chipRev == 0xa0)
            {
                if( LED->ledSerialMuxSelect == 0x0 )
                {
                    LED->ledXorReg &= ~(1 << SERIAL_LED_DATA);
                    LED->ledXorReg &= ~(1 << SERIAL_LED_CLK);
                    LED->ledInit |= (LED_SERIAL_LED_EN|LED_SERIAL_LED_MUX_SEL);
                }
                else
                    LED->ledInit |= (LED_SERIAL_LED_EN|LED_SERIAL_LED_MUX_SEL|LED_SERIAL_LED_CLK_NPOL);
            }
            else
                LED->ledInit |= (LED_SERIAL_LED_EN|LED_SERIAL_LED_MUX_SEL);
        }

        /* Enable LED controller to drive GPIO */
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            GPIO->GPIOMode |= (1 << EPHY0_SPD_LED);
            GPIO->GPIOMode |= (1 << EPHY0_ACT_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            GPIO->GPIOMode |= (1 << EPHY1_SPD_LED);
            GPIO->GPIOMode |= (1 << EPHY1_ACT_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            GPIO->GPIOMode |= (1 << EPHY2_SPD_LED);
            GPIO->GPIOMode |= (1 << EPHY2_ACT_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_3) {
            GPIO->GPIOMode |= (1 << EPHY3_SPD_LED);
            GPIO->GPIOMode |= (1 << EPHY3_ACT_LED);
        }
    }
   
#endif

#if defined (_BCM96838_)

    BpGetBoardId(boardIdStr);
    /* FIXME board dependency should be removed */
    if ((0 == strcmp(boardIdStr, "968380FTTDPS")) || (0 == strcmp(boardIdStr, "968380DP2")))
    {
        set_pinmux(0,5);
        set_pinmux(1,5);
        set_pinmux(2,5);
        set_pinmux(3,5);
        set_pinmux(4,5);
        set_pinmux(5,5);
        set_pinmux(6,5);
        set_pinmux(7,5);
        set_pinmux(8,5);
        set_pinmux(9,5);
        set_pinmux(12,5);
        set_pinmux(68,5);
        set_pinmux(69,5);
    }

    /* FIXME board dependency should be removed */
    if (0 == strcmp(boardIdStr, "968380DP2"))
    {
        set_pinmux(67,5);
    }

    /* FIXME board dependency should be removed */
    if ((0 == strcmp(boardIdStr, "965200F_CO")) ||
        (0 == strcmp(boardIdStr, "965200F_CPE")))
    {
        set_pinmux(0, 5);
        set_pinmux(1, 5);
        set_pinmux(8, 5);
        set_pinmux(10, 5);
        set_pinmux(11, 5);
        set_pinmux(12, 5);
        set_pinmux(13, 5);
        set_pinmux(14, 5);
        set_pinmux(15, 5);
        set_pinmux(67, 5);
    }

    /* set slic muxing */
    if( BpGetSlicInterfaces((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel == BP_SLIC_GROUPC)
        {
            set_pinmux(PINMUX_CSLIC_CLK_PIN, PINMUX_SLICC_FUNC);
            set_pinmux(PINMUX_CSLIC_STDOUT_PIN, PINMUX_SLICC_FUNC);
            set_pinmux(PINMUX_CSLIC_STDIN_PIN, PINMUX_SLICC_FUNC);
            set_pinmux(PINMUX_CSLIC_ENABLE_PIN, PINMUX_SLICC_FUNC);
        }
        else if (MuxSel == BP_SLIC_GROUPD)
        {
            set_pinmux(PINMUX_DSLIC_CLK_PIN, PINMUX_SLICD_FUNC);
            set_pinmux(PINMUX_DSLIC_STDOUT_PIN, PINMUX_SLICD_FUNC);
            set_pinmux(PINMUX_DSLIC_STDIN_PIN, PINMUX_SLICD_FUNC);
            set_pinmux(PINMUX_DSLIC_ENABLE_PIN, PINMUX_SLICD_FUNC);
        }
    }

    /* set phy leds pinmuxing - FIXME board dependency should be removed*/
    if (strcmp(boardIdStr, "968380FTTDPS") &&
        strcmp(boardIdStr, "968380GWAN") &&
        strcmp(boardIdStr, "965200DPF"))
    {
        set_pinmux(PINMUX_EGPHY0_LED_PIN,PINMUX_EGPHY0_LED_FUNC);
        set_pinmux(PINMUX_EGPHY1_LED_PIN,PINMUX_EGPHY1_LED_FUNC);
        set_pinmux(PINMUX_EGPHY2_LED_PIN,PINMUX_EGPHY2_LED_FUNC);
        set_pinmux(PINMUX_EGPHY3_LED_PIN,PINMUX_EGPHY3_LED_FUNC);
    }

    /* Set simcard pinmuxing */
    if( BpGetSimInterfaces((unsigned short*)&MuxSel) == BP_SUCCESS ) 
    {   
        set_pinmux(PINMUX_SIM_DAT, PINMUX_SIM_FUNC);
        set_pinmux(PINMUX_SIM_CLK, PINMUX_SIM_FUNC);
        set_pinmux(PINMUX_SIM_PRESENCE, PINMUX_SIM_FUNC);

        if (MuxSel == BP_SIMCARD_GROUPA)
        {
            set_pinmux(PINMUX_SIM_A_VCC_EN_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_A_VCC_VOL_SEL_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_A_VCC_RST_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_A_VPP_EN_PIN, PINMUX_SIM_FUNC);
        }
        else if (MuxSel == BP_SIMCARD_GROUPA_OD)
        {
            set_pinmux(PINMUX_SIM_A_VCC_EN_PIN, PINMUX_GPIO_FUNC);
            set_pinmux(PINMUX_SIM_A_VCC_VOL_SEL_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_A_VCC_RST_PIN, PINMUX_GPIO_FUNC);
            set_pinmux(PINMUX_SIM_A_VPP_EN_PIN, PINMUX_SIM_FUNC);
        }
        else if (MuxSel == BP_SIMCARD_GROUPB)
        {
            set_pinmux(PINMUX_SIM_B_VCC_EN_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_B_VCC_VOL_SEL_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_B_VCC_RST_PIN, PINMUX_SIM_FUNC);
            set_pinmux(PINMUX_SIM_B_VPP_EN_PIN, PINMUX_SIM_FUNC);
        }
    }

    /* set serial leds pinmuxing */
    if ( BpGetSerialLEDMuxSel((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel == BP_SERIAL_LED_MUX_GROUPA)
        {
            set_pinmux(PINMUX_SER_LED_CLK_A_PIN, PINMUX_SER_LED_A_FUNC);
            set_pinmux(PINMUX_SER_LED_DAT_A_PIN, PINMUX_SER_LED_A_FUNC);
        }
        else if (MuxSel == BP_SERIAL_LED_MUX_GROUPB)
        {
            set_pinmux(PINMUX_SER_LED_CLK_B_PIN, PINMUX_SER_LED_B_FUNC);
            set_pinmux(PINMUX_SER_LED_DAT_B_PIN, PINMUX_SER_LED_B_FUNC);
        }
        else if (MuxSel == BP_SERIAL_LED_MUX_GROUPC)
        {
            set_pinmux(PINMUX_SER_LED_CLK_C_PIN, PINMUX_SER_LED_C_FUNC);
            set_pinmux(PINMUX_SER_LED_DAT_C_PIN, PINMUX_SER_LED_C_FUNC);
        }
        set_pinmux(PINMUX_SER_LED_GATE_PIN, PINMUX_SER_LED_GATE_FUNC);
    }

    /* set rogue onu pinmuxing */
    if( BpGetRogueOnuEn((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel)
            set_pinmux(PINMUX_ROGUE_ONU_PIN, PINMUX_ROGUE_ONU_FUNC);
    }

    /* set wan nco 10Mhz clock pinmuxing */
    if( BpGetWanNco10M((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel)
            set_pinmux(PINMUX_WAN_NCO_10M_PIN, PINMUX_WAN_NCO_10M_FUNC);
    }

    /* set trx signal detect pinmuxing */
    if( BpGetTrxSignalDetect((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel)
            set_pinmux(PINMUX_TRX_SD_PIN, PINMUX_TRX_SD_FUNC);
    }

    /* set E_WAKE pinmuxing */
    if( BpGetPmdMACEwakeEn((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel)
            set_pinmux(PINMUX_EWAKE_GPON_MAC_PIN, PINMUX_EWAKE_GPON_MAC_FUNC);
    }

    /* Time Synchronization */

    /* 10/25MHz */
    if( BpGetTsync1025mhzPin((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        set_pinmux(BP_PIN_TSYNC_1025MHZ_11 & BP_GPIO_NUM_MASK, PINMUX_TSYNC_1025MHZ_11_FUNC);
    }

    /* 8KHz */
    if( BpGetTsync8khzPin((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        set_pinmux(BP_PIN_TSYNC_8KHZ_4 & BP_GPIO_NUM_MASK, PINMUX_TSYNC_8KHZ_4_FUNC);
    }

    /* 1PPS */
    if( BpGetTsync1ppsPin((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel == BP_PIN_TSYNC_1PPS_6)
            set_pinmux(BP_PIN_TSYNC_1PPS_6 & BP_GPIO_NUM_MASK, PINMUX_TSYNC_1PPS_6_FUNC);
        else if (MuxSel == BP_PIN_TSYNC_1PPS_52)
            set_pinmux(BP_PIN_TSYNC_1PPS_52 & BP_GPIO_NUM_MASK, PINMUX_TSYNC_1PPS_52_FUNC);
    }

    /* MII interface */
    if( BpGetMiiInterfaceEn((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        set_pinmux(PINMUX_MII_RXCOL, PINMUX_MII_FUNC);
        set_pinmux(PINMUX_MII_RXCRS, PINMUX_MII_FUNC);
        set_pinmux(PINMUX_MII_RXER, PINMUX_MII_FUNC);
    }
   
    const ETHERNET_MAC_INFO    *pE;
    
    if ( (pE = BpGetEthernetMacInfoArrayPtr()) != NULL )
    {
        if (IsRgmiiDirect(pE[0].sw.phy_id[4]) || IsExtPhyId(pE[0].sw.phy_id[4]) || IsTMII(pE[0].sw.phy_id[4])  )
        {
          printf("Configuring RGMII pinpux\n");
          set_pinmux(PINMUX_RGMII_TXCLK_PIN, PINMUX_RGMII_FUNC);
          set_pinmux(PINMUX_RGMII_TXCTL_PIN, PINMUX_RGMII_FUNC);
          set_pinmux(PINMUX_RGMII_TXD_00_PIN, PINMUX_RGMII_FUNC);
          set_pinmux(PINMUX_RGMII_TXD_01_PIN, PINMUX_RGMII_FUNC);
          set_pinmux(PINMUX_RGMII_TXD_02_PIN, PINMUX_RGMII_FUNC);
          set_pinmux(PINMUX_RGMII_TXD_03_PIN, PINMUX_RGMII_FUNC);
          set_pinmux(PINMUX_RGMII_MDC, PINMUX_RGMII_FUNC);
        }
    }
    
    if (IsTMII(pE[0].sw.phy_id[4]))
    {
        set_pinmux(PINMUX_MII_TXER,  PINMUX_MII_FUNC);
    }

    /* set tx laser on out N pinmuxing */
    if( BpGetTxLaserOnOutN((unsigned short*)&MuxSel) == BP_SUCCESS )
    {
        if (MuxSel)
            set_pinmux(PINMUX_TX_LASER_ON_N_PIN, PINMUX_TX_LASER_ON_N_FUNC);
    }
   
    initGpioPinMux();

#endif

#if defined (_BCM96328_) || defined (_BCM96362_) ||  defined (_BCM963268_) ||    defined (_BCM96318_)
    LED->ledInit &= ~LED_FAST_INTV_MASK;
    LED->ledInit |= (LED_INTERVAL_20MS * 4) << LED_FAST_INTV_SHIFT;
#elif defined (_BCM96838_)
    LED->ledInit &= ~LED_FAST_INTV_MASK;
    LED->ledInit |= (LED_INTERVAL_20MS * 2) << LED_FAST_INTV_SHIFT;
    LED->ledInit &= ~LED_SLOW_INTV_MASK;
    LED->ledInit |= (LED_INTERVAL_20MS * 8) << LED_SLOW_INTV_SHIFT;
    LED->ledInit |= LED_SERIAL_SHIFT_FRAME_POL | LED_SERIAL_LED_EN;
    /* in 968380GWAN board, EGPHY LEDs are shifted out serialy 
            FIXME board dependency should be removed*/
    if (!strcmp(boardIdStr, "968380GWAN"))
    {
        LED->ledSerialMuxSelect = 0x000000ff;
        LED->ledLinkActSelLow = 00410000;
        LED->ledHWDis = 0xfffffffc;
    }
    else
        /* Disable HW Link/Activity */
        LED->ledHWDis = 0xffffffff;
#elif defined (_BCM963138_) || defined (_BCM963381_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)
#ifdef _BCM963381_
    if( (flash_get_flash_type() == FLASH_IFC_SPINAND) || bcm_otp_is_btrm_boot() )
        cfe_boot_second_cpu((unsigned long)second_cpu_icache_fixup);
#endif
#if defined (_BCM96858_) || defined(_BCM94908_)
    cfe_boot_second_cpu((unsigned long)CFG_BOOT_AREA_ADDR);
#endif
#if !defined (_BCM968360_)
// FIXMET: need to add for 68360 when drivers enabled
    pmc_init();
#endif
#if !defined(CONFIG_BRCM_IKOS)
    bcm_misc_hw_init();
#endif
    bcm_init_pinmux();
    bcm_common_led_init();
#elif defined (_BCM960333_)
    bcm_common_led_init();
#elif defined (_BCM96848_)
    bcm_init_pinmux();
    bcm_common_led_init();
    phy_read_register(1, 2); // dummy read for mdio issue after reset.
#elif defined(_BCM947189_)
    bcm_misc_hw_init();
#else
    /* Set blink rate for hardware LEDs. */
    GPIO->LEDCtrl &= ~LED_INTERVAL_SET_MASK;
    GPIO->LEDCtrl |= LED_INTERVAL_SET_80MS;
#endif
#if defined (_BCM96858_)
    cci400_enable();
#endif
}

/*  *********************************************************************
    *  board_netdevice_init()
    *
    *  Do only net device initialization, due to a fact that we only need
    *  to initialize network device when we need it in CFE
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
void board_netdevice_init(void)
{
#if defined(_BCM96838_) || defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM96848_) || defined(_BCM96858_)
#if (NONETWORK==0)
#if defined (_BCM96858_)
    cfe_add_device( &bcm6xxx_impl3_enet, 0, 0, 0);
#else
    cfe_add_device( &bcm6xxx_impl2_enet, 0, 0, 0);
#endif
#endif
#elif defined (_BCM947189_)
#if (NONETWORK==0)
    cfe_add_device( &bcm47189_enet, 0, 0, 0);
#endif
#else
    /* Add the ethernet driver. */
#if (NONETWORK==0)
    cfe_add_device( &bcm63xx_enet, 0, 0, 0);
#endif
#endif
}

/*  *********************************************************************
    *  board_final_init()
    *
    *  Do any final initialization, such as adding commands to the
    *  user interface.
    *
    *  If you don't want a user interface, put the startup code here.
    *  This routine is called just before CFE starts its user interface.
    *
    *  Input parameters:
    *      int force_cfe.  Stop at cfe it is non zero
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void board_final_init(int force_cfe)
{
#if !defined(_BCM94908_)
    unsigned short rstToDfltIrq;
#endif
    int breakIntoCfe = force_cfe;
#if defined(_BCM94908_)
    int ret, i = 0;
    unsigned short gpio;
#endif

    setAllLedsOff();
    setPowerOnLedOn();

#if (INC_SPI_PROG_NAND==1)
    rstToDfltIrq = 1 ;
    if (bootInfo.runFrom != 'n')
        breakIntoCfe = rstToDfltIrq;
#else
    if( (bootInfo.runFrom == 'f') && getBootImageTag() == NULL ) {
        setBreakIntoCfeLed();
        printf("** Flash image not found. **\n\n");
        kerSysErasePsi();
        breakIntoCfe = 1;
    }

#if defined(_BCM94908_)
    if (RstBtnPressed()
#ifdef AC2900
        && !NVRAM.noUpdatingFirmware
#endif
    ) {
        printf("\nEnter Rescue Mode (by Force) ...\n\n");

        bcm63xx_run(0, 0);

        if (BpGetBootloaderPowerOnLedGpio(&gpio) != BP_SUCCESS)
            gpio = 20 | BP_ACTIVE_LOW | BP_LED_USE_GPIO;  // use WPS LED instead

        /* Wait forever for an image */
	while ((ret = ui_docommand("w 255.255.255.255:ASUSSPACELINK")) == CFE_ERR_TIMEOUT || ret == -1) {
            if (i%2 == 0)
                setLed(gpio, LED_OFF);
            else
                setLed(gpio, LED_ON);

            i++;
            if (i==0xffffff)
                i = 0;
        }
    } else
        printf("Skip Rescue Mode\n\n");
#endif

#if !defined(_BCM94908_)
    if( BpGetResetToDefaultExtIntr( &rstToDfltIrq ) == BP_SUCCESS ) {
        if (checkForResetToDefaultHold( rstToDfltIrq )) {
            kerSysErasePsi();
            /* Reset the default bootline if the board IP address has changed. */
            if (strcmp(bootInfo.boardIp, DEFAULT_BOARD_IP) != 0) {
                setDefaultBootline();
            }
            breakIntoCfe = 1;
        }
    }
#else
    if( BpGetSesBtnWirelessExtIntr() == BP_SUCCESS ) {
        if (checkForSesBtnWirelessHold()) {
            kerSysErasePsi();
            /* Reset the default bootline if the board IP address has changed. */
            if (strcmp(bootInfo.boardIp, DEFAULT_BOARD_IP) != 0) {
                setDefaultBootline();
            }
            breakIntoCfe = 1;
        }
    }
#endif
#endif
    bcm63xx_run(breakIntoCfe, 1);
    setBreakIntoCfeLed();
}

#if (INC_SPI_PROG_NAND==0)
#if defined(_BCM94908_)
static int RstBtnPressed(void)
{
    int buttonPressed = 0, ActiveHigh = 0;
    unsigned short gpioPin = 0;

    BpGetResetToDefaultExtIntrGpio(&gpioPin);
    if (gpioPin & BP_ACTIVE_LOW)
        ActiveHigh = 0;
    else
        ActiveHigh = 1;

    if (ActiveHigh)
        buttonPressed = bcm_gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);
    else
        buttonPressed = !bcm_gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);

    return buttonPressed;
}

static int checkForSesBtnWirelessHold(void)
{
    const int nDelay = 5;
    int ret = 0, i = 0, buttonPressed = 0;
    int ActiveHigh = 0;
    unsigned short gpioPin = 0;

    BpGetSesBtnWirelessExtIntrGpio(&gpioPin);
    if (gpioPin & BP_ACTIVE_LOW)
        ActiveHigh = 0;
    else
        ActiveHigh = 1;

    while (1) {
        if (ActiveHigh)
            buttonPressed = bcm_gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);
        else
            buttonPressed = !bcm_gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);

        if (buttonPressed) {
            printf("wps button pressed...\n");
            if (i == nDelay) {
                printf("You could release wps button now\n\n");
		setPowerOnLedOff();
                ret = 1;
                break;
            }
            cfe_sleep(CFE_HZ);
            i++;
        }
        else
            break;
    }

    return ret;
}
#endif
#endif

/*  *********************************************************************
    * Miscellaneous Board Functions
    ********************************************************************* */

/*  *********************************************************************
    *  checkForResetToDefaultHold()
    *
    *  Determines if the user is holding the reset to default button.
    *
    *  Input parameters:
    *      Reset to default irq#
    *
    *  Return value:
    *      1 - break into the CFE, 0 - continue boot sequence
    ********************************************************************* */
#if (INC_SPI_PROG_NAND==0)
#if !defined(_BCM94908_)
static int checkForResetToDefaultHold( unsigned short rstToDfltIrq )
{
#if defined (_BCM960333_) || defined  (_BCM963381_) || defined (_BCM947189_)
    //FIXME to add GPIO support
    return 0;
#else
    const int nBreakIntoCfeDelay = 5;
    int ret = 0, i = 0, buttonPressed = 0;
#if !defined (_BCM96848_)
    int irqActiveHigh = 0;
#endif
#if !defined (_BCM96838_) && !defined(_BCM963138_) && !defined(_BCM963148_) && !defined(_BCM94908_)  && !defined(_BCM96848_) && !defined(_BCM96858_) && !defined(_BCM947189_) && !defined(_BCM968360_)
    int irqShared = 0;
    unsigned short gpioPin = 0;
#endif
#if !defined (_BCM96838_) && !defined (_BCM94908_) && !defined (_BCM96858_)  && !defined (_BCM968360_)
    uint32 irqBit;
    volatile uint32 *extIrqReg;
#endif

#if defined (_BCM96838_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM968360_)
    unsigned short gpioPin = 0;
    BpGetResetToDefaultExtIntrGpio(&gpioPin);
    if (gpioPin & BP_ACTIVE_LOW)
        irqActiveHigh = 0;
    else
        irqActiveHigh = 1;
#else
#if !defined (_BCM96848_)
    irqActiveHigh = IsExtIntrTypeActHigh(rstToDfltIrq);
#endif
#if !defined (_BCM96838_) && !defined(_BCM963138_) && !defined(_BCM963148_) && !defined(_BCM94908_) && !defined(_BCM96848_) && !defined(_BCM96858_) && !defined(_BCM947189_) && !defined(_BCM968360_)
    irqShared = IsExtIntrShared(rstToDfltIrq);
    if( irqShared )
    {
        BpGetResetToDefaultExtIntrGpio(&gpioPin);
        /* make it as input */
        GPIO->GPIODir &= ~GPIO_NUM_TO_MASK(gpioPin);
#if defined(_BCM963268_)
        /* enable PERIPH CTRL for pin 36 to 51 in 63268 */
        if( (gpioPin&BP_GPIO_NUM_MASK) >= 32 )
            GPIO->GPIOCtrl |= GPIO_NUM_TO_MASK(gpioPin-32);
#endif
    }
#endif

    rstToDfltIrq &= ~BP_EXT_INTR_FLAGS_MASK;
#if defined(_BCM963138_) || defined(_BCM963148_)
    extIrqReg = &PERF->ExtIrqStatus;
#elif defined(_BCM96848_)
    extIrqReg = &PERF->ExtIrqSts;
#else
    extIrqReg = &PERF->ExtIrqCfg;
#endif
    irqBit = 1 << (rstToDfltIrq + EI_STATUS_SHFT);

#endif

    /* Loop while the reset to default button is depressed. */
    while(1) {
#if defined (_BCM96848_)
        buttonPressed = (*extIrqReg & irqBit);
#else
        if( irqActiveHigh  ){
#if defined (_BCM96838_)
            buttonPressed = gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);
#elif defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM968360_)
            buttonPressed = bcm_gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);
#else
            buttonPressed = (*extIrqReg & irqBit);
#endif
#if !defined (_BCM96838_) && !defined(_BCM963138_) && !defined(_BCM963148_) && !defined(_BCM94908_)  && !defined(_BCM96848_) && !defined(_BCM96858_) && !defined(_BCM947189_) && !defined(_BCM968360_)
            if( irqShared && !(GPIO->GPIOio&GPIO_NUM_TO_MASK(gpioPin)) )
                buttonPressed = 0;
#endif
        }
        else{
#if defined (_BCM96838_)
            buttonPressed = !gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);
#elif defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM968360_)
            buttonPressed = !bcm_gpio_get_data(gpioPin&BP_GPIO_NUM_MASK);
#else
            buttonPressed = !(*extIrqReg & irqBit);
#endif
#if !defined (_BCM96838_) && !defined(_BCM963138_) && !defined(_BCM963148_) && !defined(_BCM94908_)  && !defined(_BCM96848_) && !defined(_BCM96858_) && !defined(_BCM947189_) && !defined(_BCM968360_)
            if( irqShared && (GPIO->GPIOio&GPIO_NUM_TO_MASK(gpioPin)) )
                buttonPressed = 0;
#endif
        }
#endif

        if( buttonPressed ){
            if (i == nBreakIntoCfeDelay) {
                setBreakIntoCfeLed();
                printf("\n*** Break into CFE console ***\n\n");
                ret = 1;
                break;
            }
            cfe_sleep(CFE_HZ);
            i++;
        }
        else
            break;
    }

    return( ret );
#endif
}
#endif
#endif
/*  *********************************************************************
    *  setLed(led_gpio, led_state)
    *
    *  Turns on an LED.
    *
    *  Input parameters:
    *      LED purpose
    *      LED State
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setLed (unsigned short led_gpio, unsigned short led_state)
{
#if !defined(_BCM960333_) && !defined (_BCM963138_) && !defined (_BCM963381_) && !defined(_BCM963148_) && !defined(_BCM94908_) && !defined(_BCM96848_) && !defined(_BCM96858_) && !defined(_BCM947189_) && !defined (_BCM968360_)
    unsigned short gpio_state;

    if (((led_gpio & BP_ACTIVE_LOW) && (led_state == LED_ON)) || 
        (!(led_gpio & BP_ACTIVE_LOW) && (led_state == LED_OFF)))
        gpio_state = 0;
    else
        gpio_state = 1;
#endif

    if ( (led_gpio&BP_GPIO_NUM_MASK) == BP_GPIO_NONE )
      return;

#if defined(_BCM96328_) || defined(_BCM96318_)
    /* Enable LED controller to drive this GPIO */
    if (!(led_gpio & BP_GPIO_SERIAL))
        GPIO->GPIOMode |= GPIO_NUM_TO_MASK(led_gpio);
#endif

#if defined(_BCM96362_) || defined (_BCM963268_)
    /* Enable LED controller to drive this GPIO */
    if (!(led_gpio & BP_GPIO_SERIAL))
        GPIO->LEDCtrl |= GPIO_NUM_TO_MASK(led_gpio);
#endif

#if defined(_BCM96328_) || defined(_BCM96362_) ||  defined (_BCM963268_) ||    defined(_BCM96318_)
    LED->ledMode &= ~(LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    if( gpio_state )
        LED->ledMode |= (LED_MODE_OFF << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    else
        LED->ledMode |= (LED_MODE_ON << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));

#elif defined(_BCM96838_)
    if ( (led_gpio&BP_LED_PIN) || (led_gpio&BP_GPIO_SERIAL) )
    {
        LED->ledMode &= ~(LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
        if( gpio_state )
            LED->ledMode |= (LED_MODE_OFF << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
        else
            LED->ledMode |= (LED_MODE_ON << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    }
    else
    {
        unsigned short gpio_pin = led_gpio & BP_GPIO_NUM_MASK;
        gpio_set_dir(gpio_pin, 1);
        if( gpio_state )
            gpio_set_data(gpio_pin, 1);
        else
            gpio_set_data(gpio_pin, 0);
    }
#elif defined (_BCM963138_) || defined(_BCM963148_) || defined (_BCM963381_)\
    || defined(_BCM960333_) || defined(_BCM94908_) || defined (_BCM96848_) || defined (_BCM96858_) || defined (_BCM968360_)
    bcm_led_driver_set(led_gpio, led_state);
#elif defined(_BCM947189_)
    /* 47189 TODO: Placeholder */
#else
    if (led_gpio & BP_GPIO_SERIAL) {
        while (GPIO->SerialLedCtrl & SER_LED_BUSY);
        if( gpio_state )
            GPIO->SerialLed |= GPIO_NUM_TO_MASK(led_gpio);
        else
            GPIO->SerialLed &= ~GPIO_NUM_TO_MASK(led_gpio);
    }
    else {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
        if( gpio_state )
            GPIO->GPIOio |= GPIO_NUM_TO_MASK(led_gpio);
        else
            GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(led_gpio);
    }
#endif
}

/*  *********************************************************************
    *  setPinMuxGpio()
    *
    *  Set pin mux to GPIO function
    *
    *  Input parameters:
    *      unsigned short gpio pin number
    *      int i LED index in the bpLedList (used only in 6838)
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

static void setPinMuxGpio(unsigned short gpio, int i)
{
#if defined (_BCM96318_)
    uint32 reg, shift;
    volatile uint32* pinMuxSel;

        /* on 6318 device, gpio 13 to 41 pin is not default to gpio function. must change
        * the pinmux to gpio function
        */

        if( (gpio&BP_GPIO_NUM_MASK) == BP_GPIO_NONE )
                return;

    /* not for any serial LED gpio as they don't take any gpio pins */
    if( (gpio&BP_GPIO_SERIAL) == BP_GPIO_SERIAL )
        return;

    gpio = gpio&BP_GPIO_NUM_MASK;
    if( gpio >= 13 && gpio <= 41 )
    {
        /* set the pad control to gpio */
        reg = GPIO->PadControl[gpio>>3];
        shift = (gpio&0x7)<<2;
        reg &= ~(PAD_CTRL_GPIO0_MASK<<shift);
        reg |= (PAD_CTRL_GPIO<<shift);
        GPIO->PadControl[gpio>>3] = reg;

        /* set pin mux to gpio */
        pinMuxSel = &GPIO->PinMuxSel0;
        pinMuxSel += gpio>>4;
        reg = *pinMuxSel;
        shift = (gpio&0xf)<<1;
        reg &= ~(PINMUX_SEL_GPIO0_MASK<<shift);
        reg |= (PINMUX_SEL_GPIO<<shift);
        *pinMuxSel = reg;
    }
#elif defined (_BCM96838_)

    if( (gpio&BP_GPIO_NUM_MASK) == BP_GPIO_NONE )
        return;
    
    if( (gpio&BP_LED_PIN) == BP_LED_PIN )
    {   
        /* For parallel connected Leds to LED controller - set pinmux */
        unsigned short pinmux;
        if ( BpGetLedPinMuxGpio(i, &pinmux) == BP_SUCCESS )
        {
            pinmux &= BP_GPIO_NUM_MASK;
            if (LED_CTRL_FUNC[pinmux] != NO_PINMUX)
                set_pinmux(pinmux, LED_CTRL_FUNC[pinmux]);
        }
    }
    else if ( (gpio&BP_GPIO_SERIAL) != BP_GPIO_SERIAL )
        /* For LEDs connected directly to GPIO - set pinmux */
        set_pinmux(gpio&BP_GPIO_NUM_MASK, PINMUX_GPIO_FUNC);
#endif
    return;
}

/*  *********************************************************************
    *  initGpioPinMux()
    *
    *  Initialize the gpio pin mux register setting. On some chip like 6318, Certain
    *  gpio pin are muxed with other function and  are not default to gpio. so init
    *  code needs to set the mux to gpio if they are used by led or gpio boardparm
    *
    *
    *  Input parameters: none
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
void initGpioPinMux(void)
{
    int i = 0,  rc;
    void* token = NULL;
    unsigned short gpio;

    /* walk through all the led bp */
    for(;;)
    {
        rc = BpGetLedGpio(i, &token, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
            break;
        else if( rc == BP_SUCCESS )
            setPinMuxGpio(gpio, i);
        else
        {
            token = NULL;
            i++;
        }
    }

    /* walk through all the gpio bp */
    i = 0;
    token = NULL;
    for(;;)
    {
        rc = BpGetGpioGpio(i, &token, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
            break;
        else if( rc == BP_SUCCESS )
            setPinMuxGpio(gpio,i);
        else
        {
            token = NULL;
            i++;
        }
    }

    return;
}


/*  *********************************************************************
    *  setAllLedsOff()
    *
    *  Turns off all board LEDs on init
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setAllLedsOff(void)
{
#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM963381_) || defined(_BCM94908_) || defined(_BCM96848_) || defined(_BCM96858_) || defined(_BCM968360_)
    bcm_common_led_setAllSoftLedsOff();
#else
    unsigned short gpio;
    int i = 0, rc;
    void* token = NULL;

    for(;;)
    {
        rc = BpGetLedGpio(i, &token, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
            break;
        else if( rc == BP_SUCCESS )
        {
            setLed( gpio, LED_OFF );
        }
        else
    {
        token = NULL;
            i++;
    }
    }
#endif
    return;
}

/*  *********************************************************************
    *  setPowerOnLedOn()
    *
    *  Turns on the Power LED.
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setPowerOnLedOn(void)
{
    unsigned short gpio;
    if( BpGetBootloaderStopLedGpio( &gpio ) == BP_SUCCESS )
        setLed( gpio, LED_OFF );
    if( BpGetBootloaderPowerOnLedGpio( &gpio ) == BP_SUCCESS )
        setLed( gpio, LED_ON );
}

#if defined(_BCM94908_)
static void setPowerOnLedOff(void)
{
     unsigned short gpio;
     if( BpGetBootloaderPowerOnLedGpio( &gpio ) == BP_SUCCESS )
         setLed( gpio, LED_OFF );
}
#endif

/*  *********************************************************************
    *  setBreakIntoCfeLed()
    *
    *  Turns on the alarm LED.
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setBreakIntoCfeLed(void)
{
    unsigned short gpio;
    if( BpGetBootloaderStopLedGpio( &gpio ) == BP_SUCCESS ) {
        setLed( gpio, LED_ON );
        if( BpGetBootloaderPowerOnLedGpio( &gpio ) == BP_SUCCESS )
            setLed( gpio, LED_OFF );
    }
}

/*  *********************************************************************
    *  softReset()
    *
    *  Resets the board.
    *
    *  Input parameters:
    *      delay in second
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void softReset(unsigned int delay)
{
#if defined(_BCM96318_)
    unsigned short plcGpio = 0;
#endif

#if defined(_BCM94908_)
    PLL_CTRL_REG ctrl_reg;
#endif


    printf( "\nResetting board in %d seconds...\n", delay );


#if defined(_BCM96318_)
    /* Use GPIO to control the PLC and wifi chip reset on 6319 PLC board*/
    if( BpGetPLCPwrEnGpio(&plcGpio) == BP_SUCCESS )
    {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(plcGpio);
           /* set inactive on power enable */
        if( plcGpio & BP_ACTIVE_LOW)
            GPIO->GPIOio |= GPIO_NUM_TO_MASK(plcGpio);
        else
            GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(plcGpio);
        /* reset for 5ms */
        cfe_usleep(5000);
    }
#endif

#if defined(_BCM94908_)
    /* reset the pll manually to bypass mode if strap for slow clock */
    if (MISC->miscStrapBus&MISC_STRAP_BUS_CPU_SLOW_FREQ)
    {
        ReadBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 1;
        WriteBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
    }
#endif

    if( delay == 0 )
    {
#if defined(_BCM96328_) || defined(_BCM96318_) || defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM963381_) || defined(_BCM94908_) || defined(_BCM96848_) ||  defined (_BCM96858_) || defined(_BCM968360_)
        TIMER->SoftRst |= SOFT_RESET;
#elif defined(_BCM96838_)
        PERF->TimerControl |= SOFT_RESET_0;
#elif defined(_BCM960333_)
        /*
         * After a soft-reset, one of the reserved bits of TIMER->SoftRst remains
         * enabled and the next soft-reset won't work unless TIMER->SoftRst is
         * set to 0.
         */
        TIMER->SoftRst = 0;
        TIMER->SoftRst |= SOFT_RESET;
#elif defined(_BCM947189_)
        /*
         * In theory, since 47189 has PMU enabled (MISC_1->capabilities &
         * CC_CAP_PMU), reset should be done through the PMU watchdog
         * (PMU->pmuwatchdog). But this hangs the BCM947189ACNRM_2 P235
         * evaluation board.
         * However, using the ChipCommon core watchdog works, so we're going
         * with that for now.
         */
        GPIO_WATCHDOG->watchdog = 1;
#else
        PERF->pll_control |= SOFT_RESET;    // soft reset mips
#endif
    }
    else
    {
#if defined (_BCM96838_)
        WDTIMER->WD0DefCount = delay*FPERIPH;
        WDTIMER->WD0Ctl = 0xFF00;
        WDTIMER->WD0Ctl = 0x00FF;
#elif defined(_BCM947189_)
        /*
         * Watchdog reset - TODO: Get PLL freq to calculate the watchdog counter
         * value.
         */
#else
        TIMER->WatchDogDefCount = delay*FPERIPH;
        TIMER->WatchDogCtl = 0xFF00;
        TIMER->WatchDogCtl = 0x00FF;
#endif
    }
    while (1);
}
