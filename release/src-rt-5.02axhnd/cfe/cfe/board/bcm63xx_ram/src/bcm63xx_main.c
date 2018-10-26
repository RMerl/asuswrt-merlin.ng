/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Main Module              File: cfe_main.c       
    *  
    *  This module contains the main "C" routine for CFE and 
    *  the main processing loop.  There should not be any board-specific
    *  stuff in here.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
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
    ********************************************************************* */


#include "lib_types.h"
#include "lib_string.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_console.h"
#include "cfe_timer.h"

#include "env_subr.h"
#include "cfe_mem.h"
#include "cfe.h"

#include "exception.h"

#include "bsp_config.h"
#include "cpu_config.h"
#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "bcm_map.h"
#include "bcm_cpu.h"
#include "bcm63xx_util.h"
#include "bcm63xx_blparms.h"
#include "segtable.h"
#include "initdata.h"
#include "shared_utils.h"
#include "flash_api.h"
#include "bcm63xx_nvram.h"
#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "BPCM.h"
#endif

#if defined(BOARD_SEC_ARCH)
#include "bcm63xx_sec.h"
#endif

#if defined (_BCM96858_) || defined(_BCM94908_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_)
#include "bcm_otp.h"
#endif

#include "bcm_memory.h"

#if defined (_BCM947189_)
#include "bcm_misc_hw_init.h"
#endif

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#ifndef CFG_STACK_SIZE
#define STACK_SIZE  8192
#else
#define STACK_SIZE  ((CFG_STACK_SIZE+1023) & ~1023)
#endif

#define CFE_DFLT_PROMPT "CFE> "
#define CFE_MFG_PROMPT  "MFG> "

/*  *********************************************************************
    *  Externs
    ********************************************************************* */

void cfe_main(int,int);
void cfe_command_restart(uint64_t status);

static void cfe_init_sdram_size(void);
static void cfe_set_prompt(void);
extern void cfe_device_poll(void *x);

extern int cfe_web_check(void);
extern void cfe_web_fg_process(void);
extern void cfe_web_poll(void *x);

extern const char *builddate;
extern const char *builduser;
extern int g_invalid_img_rev;

#if defined (_BCM96848_)
extern uint32 otp_get_max_ddr_freq(void);
extern uint32 otp_get_max_clk_sel(void);
#endif

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

static char * g_prompt = CFE_DFLT_PROMPT;
extern unsigned long mem_totalsize;
const char *cfe_boardname = CFG_BOARDNAME;
unsigned int cfe_startflags = 0;
#if defined (_BCM963268_) || defined (_BCM96838_) || defined (_BCM960333_) || defined (_BCM963381_) || \
    defined (_BCM96848_) || defined (_BCM96858_)
static int cfe_bus_speed = 0;
static int cfe_ddr_speed = 0;
#endif
#if defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM96858_) || \
    defined(_BCM96846_) || defined (_BCM96856_)
static unsigned int cfe_rdp_speed = 0;
#endif
unsigned long cfe_sdramsize = 8 * 1024 * 1024;

static void calculateCpuSpeed(void);

#if defined (_BCM963268_)
const uint32 cpu_speed_table[0x20] = {
    0, 0, 400, 320, 0, 0, 0, 0, 0, 0, 333, 400, 0, 0, 320, 400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const uint32 ddr_speed_table[0x20] = {
    0, 0, 267, 267, 0, 0, 0, 0, 0, 0, 333, 333, 0, 0, 400, 400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const uint32 bus_speed_table[0x20] = {
    0, 0, 133, 133, 0, 0, 0, 0, 0, 0, 167, 167, 0, 0, 200, 200,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif

#if defined (_BCM960333_)
const uint32 cpu_speed_table[4] = {
    200, 400, 333, 0
};
const uint32 ddr_speed_table[4] = {
    200, 400, 333, 0
};
const uint32 bus_speed_table[2] = {
    100, 167
};
#endif

#if defined (_BCM96838_)
const uint32 cpu_speed_table[0x3] = {
    600, 400, 240
};
const uint32 ddr_speed_table[0x16] = {
    533, 333, 333, 800, 400, 533, 400, 533
};
const uint32 bus_speed_table[0x1] = {
    240
};
#endif

#if defined (_BCM963381_)
const uint32 cpu_speed_table[4] = {
    300, 800, 480, 600
};
const uint32 ddr_speed_table[4] = {
    200, 300, 266, 400
};
const uint32 bus_speed_table[4] = {
    200, 200, 171, 300
};
#endif

#if defined (_BCM96848_)
const uint32 cpu_speed_table[8] = {
    250, 250, 400, 400, 250, 250, 428, 600 
};
const uint32 ddr_speed_table[4] = {
    533, 400, 333, 533, 
};
const uint32 bus_speed_table[8] = {
    285, 285, 285, 285, 300, 300, 300, 300
};
const uint32 rdp_speed_table[8] = {
    400, 400, 400, 400, 600, 600, 600, 428
};
#endif

#if defined (_BCM96858_)
const uint32 cpu_speed_table[4] = {
    1500, 1000, 500, 0
};
const uint32 ddr_speed_table[1] = {
    1067
};
const uint32 bus_speed_table[2] = {
    500, 1000
};
#endif

#if defined (_BCM96846_)
const uint32 cpu_speed_table[4] = {
    1000, 1000, 750, 0
};
#endif

unsigned long cfe_get_sdram_size(void);

/*  *********************************************************************
    *  cfe_setup_default_env()
    *  
    *  Initialize the default environment for CFE.  These are all
    *  the temporary variables that do not get stored in the NVRAM
    *  but are available to other programs and command-line macros.
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */

static void cfe_setup_default_env(void)
{
    char buffer[80];

    xsprintf(buffer,"%d.%d.%d",CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD);
    env_setenv("CFE_VERSION",buffer,ENV_FLG_BUILTIN | ENV_FLG_READONLY);

    if (cfe_boardname) {
        env_setenv("CFE_BOARDNAME",(char *) cfe_boardname,
                   ENV_FLG_BUILTIN | ENV_FLG_READONLY);
    }
}


/*  *********************************************************************
    *  cfe_ledstr(leds)
    *  
    *  Display a string on the board's LED display, if it has one.
    *  This routine depends on the board-support package to
    *  include a "driver" to write to the actual LED, if the board
    *  does not have one this routine will do nothing.
    *  
    *  The LEDs are written at various places in the initialization
    *  sequence, to debug board problems.
    *  
    *  Input parameters: 
    *      leds - pointer to four-character ASCII string
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */

void cfe_ledstr(const char *leds)
{
#if 0
    unsigned int val;

    val = ((((unsigned int) leds[0]) << 24) |
       (((unsigned int) leds[1]) << 16) |
       (((unsigned int) leds[2]) << 8) |
       ((unsigned int) leds[3]));

    cfe_leds(val);
#endif
}


/*  *********************************************************************
    *  cfe_say_hello()
    *  
    *  Print out the CFE startup message and copyright notice
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */

void cfe_command_restart(uint64_t status)
{
}

static void cfe_say_hello(void)
{
    xprintf("\n\n");
    //xprintf("Base: %d.%d_%s\n", BRCM_VERSION, BRCM_RELEASE, BRCM_EXTRAVERSION );
    xprintf("CFE version %d.%d.%d-%d.%d"
#ifdef CFE_VER_RELEASE
        ".%d"
#endif
        " for %s (%s)\n",
        CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD, BCM63XX_MAJOR, BCM63XX_MINOR,
#ifdef CFE_VER_RELEASE
        CFE_VER_RELEASE,
#endif
        cfe_boardname,
#ifdef __long64
        "64bit,"
#else
        "32bit,"
#endif  
#if CFG_MULTI_CPUS
        "MP,"
#else
        "SP,"
#endif
#if defined (__MIPSEL) || defined(__ARMEL__)
        "LE"
#endif
#ifdef __MIPSEB
        "BE"
#endif
#if CFG_VAPI
        ",VAPI"
#endif
    );

    xprintf("Build Date: %s (%s)\n",builddate,builduser);
    xprintf("Copyright (C) 2000-2015 Broadcom Corporation.\n");
    xprintf("\n");
}


/*  *********************************************************************
    *  cfe_restart()
    *  
    *  Restart CFE from scratch, jumping back to the boot vector.
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      does not return
    ********************************************************************* */

void cfe_restart(void)
{
    _exc_restart();
}


/*  *********************************************************************
    *  cfe_start(ept)
    *  
    *  Start a user program
    *  
    *  Input parameters: 
    *      ept - entry point
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */
void cfe_start(unsigned long ept)
{
    cfe_launch(ept);
}

/*  *********************************************************************
    *  cfe_startup_info()
    *  
    *  Display startup memory configuration messages
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */
static void cfe_startup_info(void)
{
    char chipname[BRCM_MAX_CHIP_NAME_LEN];
#if defined (_BCM960333_) || (_BCM963381_)
	int bSDR = 0;

#if defined(_BCM963381_)
        bSDR = !(MISC->miscStrapBus&MISC_STRAP_BUS_DDR_N_SDRAM_SELECT);
#else
	bSDR = (STRAP->strapOverrideBus&STRAP_BUS_SDR_MASK)>>STRAP_BUS_SDR_SHIFT;
#endif
#endif
    xprintf("Chip ID: BCM%s, ", UtilGetChipName(chipname,BRCM_MAX_CHIP_NAME_LEN));
#if defined (_BCM963138_)
    xprintf("ARM Cortex A9 Dual Core: %dMHz",cfe_cpu_speed/1000000);
#elif defined(_BCM947189_)
    xprintf("ARM Cortex A7: %dMHz",cfe_cpu_speed/1000000);
#elif  defined(_BCM96846_)
    xprintf("ARM Cortex A7 Dual Core: %dMHz",cfe_cpu_speed/1000000);
#elif defined(_BCM963148_)
    xprintf("Broadcom B15 Dual Core: %dMHz",cfe_cpu_speed/1000000);
#elif defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_)
    char *nr_cores = NULL;
#if defined(_BCM96858_)
    uint32 val;
    if ( !bcm_otp_get_nr_cpus(&val) )
    {
        if (val == 0)
            nr_cores = "Quad";
        else if (val == 2)
            nr_cores = "Dual";
        else if (val == 3)
            nr_cores = "Single";
    }
#elif defined(_BCM94908_) || defined(_BCM963158_)
    nr_cores = "Quad";
#elif defined(_BCM96856_)
    uint32 val;
    if ( !bcm_otp_get_nr_cpus(&val) )
    {
        if (val == 0)
            nr_cores = "Dual";
        else if (val == 1)
            nr_cores = "Single";
    }
#endif
    xprintf("Broadcom B53 %s Core: %dMHz", nr_cores, cfe_cpu_speed/1000000);
#else
    xprintf("MIPS: %dMHz",cfe_cpu_speed/1000000);
#endif
#if _BCM963268_ ||     defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM96858_)
    xprintf(", DDR: %dMHz, Bus: %dMHz\n", cfe_ddr_speed/1000000, cfe_bus_speed/1000000);
#elif defined (_BCM960333_) || defined (_BCM963381_)
    xprintf(", %s: %dMHz, Bus: %dMHz\n", bSDR ? "SDRAM":"DDR", bSDR ? cfe_ddr_speed/1000000/2:cfe_ddr_speed/1000000, cfe_bus_speed/1000000);
#else
    xprintf("\n");
#endif
#if defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
    xprintf("RDP: %dMHz\n",cfe_rdp_speed/1000000);
#endif
    {
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    unsigned long tp = 0;
#if defined(CP0_CMT_TPID)
    __asm __volatile(
    "mfc0 $9, $22, 3;"
    "move %0, $9"
    :"=r" (tp));
    tp = ((tp & CP0_CMT_TPID) == CP0_CMT_TPID) ? 1 : 0;
#endif    
    xprintf("Main Thread: TP%d\n", tp);
#endif
    }

#if defined (_BCM963268_) ||  defined (_BCM96838_)
    if (MEMC->TEST_CFG1 & 0x2) {   /* Memory Test is finished */
        xprintf("Memory Test ");
        if (MEMC->TEST_CFG1 & 0x4)
            xprintf("Failed\n");
        else
            xprintf("Passed\n");
    }
#endif

    cfe_sdramsize = cfe_get_sdram_size();
    xprintf("Total Memory: %lu bytes (%luMB)\n", cfe_sdramsize, cfe_sdramsize >> 20);
#if defined (_BCM960333_) || defined (_BCM963268_) || defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM963381_)
    if (flash_get_flash_type() != FLASH_IFC_SPINAND)
        xprintf("Boot Address: 0x%x\n\n", FLASH_BASE);
#endif
}

/*
    *  handle_cfe_abort()
    *  
    *  Read uart and if 'p' is pressed
    *  only initiaze Flash and jump to cfe prompt
    *  only purpose of this function is recovery 
    *  in case if the board hangs or resets
    *  before reaching the real prompt
    *  
*/
static void handle_cfe_abort(void)
{
char interactive='\0';
    if(console_status())
    {
        console_read(&interactive, 1);
	if(interactive == 'p')
        {
            printf("*************** CFERAM ABORT DETECTED *************** \n");
            printf("dropping to cfe prompt without board initiliazation\n");
            board_bootdevice_init();
            bcm63xx_run_ex(1,1,0);
            cfe_set_prompt();
            cfe_command_loop();

        }
    }
}

/*  *********************************************************************
    *  cfe_main(a,b)
    *  
    *  It's gotta start somewhere.
    *  
    *  Input parameters: 
    *         a,b - not used
    *         
    *  Return value:
    *         does not return
    ********************************************************************* */
void cfe_main(int a,int b)
{
    int rc = 0;
    /*
     * By the time this routine is called, the following things have
     * already been done:
     *
     * 1. The processor(s) is(are) initialized.
     * 2. The caches are initialized.
     * 3. The memory controller is initialized.
     * 4. BSS has been zeroed.
     * 5. The data has been moved to R/W space.
     * 6. The "C" Stack has been initialized.
     */
    cfe_init_sdram_size();
    cfe_bg_init();                       /* init background processing */
    cfe_attach_init();
#if (INC_PMC_DRIVER==1)
    pmc_initmode();
#endif
    calculateCpuSpeed();

    cfe_timer_init();                /* Timer process */
    cfe_bg_add(cfe_device_poll,NULL);

    /*
     * Initialize the memory allocator
     */
    KMEMINIT((unsigned char *) (uintptr_t) (ALIGN(sizeof(void*),MEMORY_AREA_HEAP_ADDR)), MEMORY_AREA_HEAP_SIZE);
    /*
     * Initialize the console.  It is done before the other devices
     * get turned on. 
     */
    board_console_init();

    cfe_setup_exceptions();
    cfe_say_hello();

#if defined(BOARD_SEC_ARCH)
    cfe_sec_init();
#endif

#if !defined (_BCM960333_) && !defined(_BCM96838_) && !defined(_BCM947189_)
    xprintf("Boot Strap Register:  0x%x\n", MISC->miscStrapBus);
#else
#if defined (_BCM960333_)
    xprintf("Boot Strap Register:  0x%x\n", STRAP->strapOverrideBus);
#endif
#if defined(_BCM96838_)
    xprintf("Boot Strap Register:  0x%x\n", GPIO->strap_bus);
#endif
#endif

    handle_cfe_abort();//read uart and directly jump to cfe promp if needed

    cfe_arena_init();
    cfe_startup_info();

    board_device_init();

#if defined(IKOS_BD_LINUX)
    {
    blparms_init();
#if defined (_BCM963138_) || defined(_BCM963148_) || defined(_BCM947189_) || defined(_BCM96846_)
   static unsigned long linuxStartAddr=0x8000;
//   printf("L2C_FILT_START 0x%x L2C_FILT_END 0x%x\n", *((unsigned int*)0x8001dc00), *((unsigned int*)0x8001dc04));
#elif defined(_BCM963381_)
   static unsigned long linuxStartAddr=0x80010400;
#elif defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_)
   static unsigned long linuxStartAddr=0x80000;/* 512KB offset */ 
   /* to boot in aarch32 mode, enable the code below */
#if 0
   extern int g_switch2aarch32;
   g_switch2aarch32 = 1;
   linuxStartAddr=0x8000;
#endif

#else
    /*0x694b6f32 (iKo2) is replaced with actual addr during the build process*/
    static unsigned long linuxStartAddr=0x694b6f32;
#endif
    printf("IKOS Build: Jump to Linux start address 0x%8.8lx.\n\n",
        linuxStartAddr);
    /* test blparms */ 
    blparms_set_str("bl string", "ikos test");
    blparms_set_int("bl int", linuxStartAddr);
    blparms_install(NULL);

    cfe_launch(linuxStartAddr);
    }
#endif

    cfe_setup_default_env();
    ui_init_cmddisp();
    rc = getBootLine(0);
    getBoardParam();
    blparms_init();
    board_final_init(rc);
    cfe_set_prompt();
    cfe_command_loop();
}

/*  *********************************************************************
    *  cfe_command_loop()
    *  
    *  This routine reads and processes user commands
    *  
    *  Input parameters: 
    *         nothing
    *         
    *  Return value:
    *         does not return
    ********************************************************************* */


void cfe_command_loop()
{
    char buffer[300];
    int status;
    char *prompt = g_prompt;

#if (NONETWORK==0)
    /* Start Web interface. */
    if (!g_invalid_img_rev) { /* If image is invalid, network will not be up; */
        cfe_bg_add(cfe_web_poll,NULL);
    }
#endif

    /* Ensure proper privilege levels are set for console access */
#if defined(BOARD_SEC_ARCH)
    cfe_sec_set_console_privLvl();
#endif    

    for (;;) {
        console_readline(prompt,buffer,sizeof(buffer));

        if (cfe_web_check())
            cfe_web_fg_process();
        else {
            status = ui_docommands(buffer);
            if (status != CMD_ERR_BLANK) {
                xprintf("*** command status = %d\n", status);
            }
        }
    }
}

/*  *********************************************************************
    *  cfe_errortext(err)
    *  
    *  Returns an error message with a number in it.  The number can be
    *  looked up in cfe_error.h. 
    *  
    *  Input parameters: 
    *         err - error code
    *         
    *  Return value:
    *         string description of error
    ********************************************************************* */

const char *cfe_errortext(int err)
{
    static char err_buf[20];

    sprintf(err_buf, "CFE error %d", err);
    return (const char *) err_buf;
}

/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate BCM6368 CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fref_clk = (64 * (P2/P1) * (MIPSDDR_NDIV / REF_MDIV)
    *      Fbus_clk = (64 * (P2/P1) * (MIPSDDR_NDIV / DDR_MDIV)
    *      Fcpu_clk = (64 * (P2/P1) * (MIPSDDR_NDIV / CPU_MDIV)
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
void static calculateCpuSpeed(void)
{
#if defined (_BCM960333_)
    uint32 mips_freq_strap =  (STRAP->strapOverrideBus & STRAP_BUS_MIPS_FREQ_MASK) >> STRAP_BUS_MIPS_FREQ_SHIFT;
    uint32 sdr_freq_strap  =  (STRAP->strapOverrideBus & STRAP_BUS_SDR_FREQ_MASK) >> STRAP_BUS_SDR_FREQ_SHIFT;
    uint32 bus_freq_strap  =  (STRAP->strapOverrideBus & STRAP_BUS_UBUS_FREQ_MASK) >> STRAP_BUS_UBUS_FREQ_SHIFT;

    cfe_cpu_speed = cpu_speed_table[mips_freq_strap] * 1000000;
    cfe_ddr_speed = ddr_speed_table[sdr_freq_strap] * 1000000;
    cfe_bus_speed = bus_speed_table[bus_freq_strap] * 1000000;
#elif defined (_BCM96838_)
	uint32 ddr_max_freq[] = {1000, 666, 533, 400, 333};
    uint32 ddr_freq_straps;
    uint32 cpu_freq_otp = (OTP->BrcmBits[0] & OTP_BRCM_VIPER_FREQ_MASK) >> OTP_BRCM_VIPER_FREQ_SHIFT;
    cfe_cpu_speed = cpu_speed_table[cpu_freq_otp] * 1000000;
//    ddr_freq_straps = (MEMC->PhyControl.STRAP_CONTROL & PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_MASK) >> PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_STRT;
//    ddr_freq_straps |= (MEMC->PhyControl.STRAP_CONTROL & PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_MASK) >> (PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_STRT-2);
    ddr_freq_straps = ((*(volatile unsigned int*)(GPIO_BASE+GPIO_DATA_MID)) & (0x7 << (GPIO_STRAP_PIN_STRT-32))) >> (GPIO_STRAP_PIN_STRT-32);
    cfe_ddr_speed = ddr_speed_table[ddr_freq_straps];
	// if DDR speed(by strap) is greater then chips max DDR speed(by OTP) enforce 333MHz
	if(cfe_ddr_speed > 	ddr_max_freq[(*((volatile unsigned long*)(OTP_BASE+OTP_SHADOW_BRCM_BITS_0_31)) & OTP_BRCM_DDR_MAX_FREQ_MASK) >> OTP_BRCM_DDR_MAX_FREQ_SHIFT])
		cfe_ddr_speed = ddr_speed_table[1];
	cfe_ddr_speed *= 1000000;
    cfe_bus_speed = bus_speed_table[0] * 1000000;
    get_rdp_freq(&cfe_rdp_speed);
    cfe_rdp_speed *= 1000000;
#elif defined (_BCM963138_)
    cfe_set_cpu_freq(1000);
#elif defined(_BCM947189_)
    cfe_cpu_speed = pmu_clk(PMU_PLL_CTRL_M1DIV_SHIFT);
#elif defined(_BCM963148_)
    // FIXME! as of 2/27/2014, CPU boots up at 750 MHz, and we set it to 1 GHZ
    cfe_cpu_speed = 750*1000000;
    cfe_set_cpu_freq(1500);
#elif defined (_BCM94908_)
    cfe_cpu_speed = ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ) ? 400*1000000 : 1800*1000000;
    cfe_set_cpu_freq(1800);
#elif defined(_BCM963158_)
    cfe_cpu_speed = ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ) ? 400*1000000 : 1675*1000000;
    cfe_set_cpu_freq(1675);
#elif defined (_BCM96858_)
    uint32 bus_freq_strap  =  (MISC->miscStrapBus & MISC_STRAP_BUS_UBUS_FREQ_MASK) >> MISC_STRAP_BUS_UBUS_FREQ_SHIFT;
    pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, (unsigned int *)&cfe_cpu_speed);
    cfe_bus_speed = bus_speed_table[bus_freq_strap] * 1000000;
    cfe_cpu_speed *= 1000000;
#if defined(CONFIG_BRCM_IKOS)
    cfe_rdp_speed = 1000;
#else
    get_rdp_freq(&cfe_rdp_speed);
#endif
    cfe_rdp_speed *= 1000000;
    cfe_ddr_speed = ddr_speed_table[0] * 1000000;
#elif defined (_BCM96856_)
#if defined(CONFIG_BRCM_IKOS)
    cfe_rdp_speed = 1400 * 1000000;
    cfe_cpu_speed = 1500 * 1000000;
#else
    get_rdp_freq(&cfe_rdp_speed);
    cfe_rdp_speed *= 1000000;
    pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, (unsigned int *)&cfe_cpu_speed);
    cfe_cpu_speed *= 1000000;
#endif
#elif defined (_BCM96846_)
#if defined(CONFIG_BRCM_IKOS)
    cfe_cpu_speed = 1000;
    cfe_rdp_speed = 1400; 
#else
    unsigned int clk_index;
    get_rdp_freq(&cfe_rdp_speed);
    cfe_rdp_speed *= 1000000;
    if ( !bcm_otp_get_cpu_clk(&clk_index) )
        cfe_cpu_speed = cpu_speed_table[clk_index] * 1000000;
    else
        cfe_cpu_speed = 0;
#endif
#elif defined (_BCM963381_)
    uint32 mips_freq_strap =  (MISC->miscStrapBus & MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK) >> MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT;
    uint32 sdr_freq_strap  =  (MISC->miscStrapBus & MISC_STRAP_BUS_MEMC_FREQ_MASK) >> MISC_STRAP_BUS_MEMC_FREQ_SHIFT;
    uint32 bus_freq_strap  =  (MISC->miscStrapBus & MISC_STRAP_BUS_UBUS_FREQ_MASK) >> MISC_STRAP_BUS_UBUS_FREQ_SHIFT;

    cfe_cpu_speed = cpu_speed_table[mips_freq_strap] * 1000000;
    cfe_ddr_speed = ddr_speed_table[sdr_freq_strap] * 1000000;
    cfe_bus_speed = bus_speed_table[bus_freq_strap] * 1000000;
#elif defined (_BCM96848_)
    uint32 clock_sel_strap = (MISC->miscStrapBus & MISC_STRAP_CLOCK_SEL_MASK) >> MISC_STRAP_CLOCK_SEL_SHIFT;
    uint32 ddr_freq_strap = (MISC->miscStrapBus & MISC_STRAP_DDR_FREQ_MASK) >> MISC_STRAP_DDR_FREQ_SHIFT;
    uint32 clock_sel_otp = otp_get_max_clk_sel();
    uint32 ddr_freq_otp = otp_get_max_ddr_freq();

    if (cpu_speed_table[clock_sel_strap] <= cpu_speed_table[clock_sel_otp])
        cfe_cpu_speed = cpu_speed_table[clock_sel_strap] * 1000000;
    else
        cfe_cpu_speed = cpu_speed_table[clock_sel_otp] * 1000000;

    if (rdp_speed_table[clock_sel_strap] <= rdp_speed_table[clock_sel_otp])
        cfe_rdp_speed = rdp_speed_table[clock_sel_strap] * 1000000;
    else
        cfe_rdp_speed = rdp_speed_table[clock_sel_otp] * 1000000;

    if (ddr_speed_table[ddr_freq_strap] <= ddr_speed_table[ddr_freq_otp])
        cfe_ddr_speed = ddr_speed_table[ddr_freq_strap] * 1000000;
    else
        cfe_ddr_speed = ddr_speed_table[ddr_freq_otp] * 1000000;

    cfe_bus_speed = bus_speed_table[clock_sel_strap < clock_sel_otp ? clock_sel_strap:clock_sel_otp] * 1000000;
#else
    uint32 mips_pll_fvco;

    mips_pll_fvco = MISC->miscStrapBus & MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK;
    mips_pll_fvco >>= MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT;
    cfe_cpu_speed = cpu_speed_table[mips_pll_fvco] * 1000000;
    cfe_ddr_speed = ddr_speed_table[mips_pll_fvco] * 1000000;
    cfe_bus_speed = bus_speed_table[mips_pll_fvco] * 1000000;
#endif
}

/*  *********************************************************************
    *  cfe_get_sdram_size(void)
    *  
    *  Return amount of SDRAM on the board.
    *  
    *  Input parameters: 
    *         None.
    *         
    *  Return value:
    *         Amount of SDRAM on the board.
    ********************************************************************* */
unsigned long cfe_get_sdram_size(void)
{
    return (mem_totalsize << 10);
}

static void cfe_set_prompt(void)
{
    char * prompt;

#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || \
    defined(_BCM96856_)
    if (bcm_otp_is_boot_mfg_secure())
        prompt = CFE_MFG_PROMPT;
    else
#endif
    {
        prompt = env_getenv("PROMPT");
    }
    if( prompt )
        g_prompt = prompt;
}

static void cfe_init_sdram_size(void)
{

#if defined(_BCM960333_) || defined(_BCM963381_)
    uint32 memCfg;
#ifdef _BCM963381_
    memCfg = MEMC->SDR_CFG.SDR_CFG;
#else
    memCfg = MEMC->SDR_CFG;
#endif
    memCfg = (memCfg&MEMC_SDRAM_SPACE_MASK)>>MEMC_SDRAM_SPACE_SHIFT;
    mem_totalsize = (1<<(memCfg+20))>>10;
#elif defined(_BCM96848_)
    mem_totalsize = (1<<(((MEMC->GLB_GCFG&MEMC_GLB_GCFG_SIZE1_MASK)>>MEMC_GLB_GCFG_SIZE1_SHIFT)+20))>>10;
#elif defined(_BCM96838_) || defined(_BCM963268_)
    mem_totalsize = (MEMC->CSEND << 14);
#endif

#if defined(IKOS_NO_DDRINIT)
    mem_totalsize = SZ_64K;
#endif

    /* Updating max heap size */
    if (MEMORY_AREA_FREE_MEM_SIZE > SZ_64M )  {
        mem_topofmem += SZ_2M;
    } 
}
