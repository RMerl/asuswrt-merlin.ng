/*
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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

/**************************************************************************
* File Name  : wan_drv.c
*
* Description: This file contains the implementation for the BCM6838 WAN
*              block to handle GPON/EPON/ActiveEthernet
*
* Updates    : 02/26/2013  Created.
***************************************************************************/
#include "bcm_map_part.h"
#include "mdio_drv_impl1.h"
#include "access_macros.h"
#include "bl_os_wraper.h"
#include "pmc_drv.h"
#include "shared_utils.h"
#include "board.h"
#include "boardparms.h"
#include <bcmsfp_i2c.h>
#include "wan_drv.h"
#include <opticaldet.h>

/* 
	the access macros include gives you the following macros to read and write registers
	
	READ_32(address,variable) 	- will read from absolute address into the variable.
	WRITE_32(address,variable) 	- will write from absolute address into the variable.
	FIELD_MREAD_32(address,offset,len,variable)
	FIELD_MWRITE_32(address,offset,len,variable)
	
 */

#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <linux/ctype.h>	/* Necessary for tolower() */

int rdp_post_init_fiber(void);

#define CHECK_ESERDES_STAT 1
PMD_DEV_ENABLE_PRBS pmd_prbs_callback;

#define MAX_AMPL_PRM	31
#define MAX_PARAM_LEN   64
#define PROCFS_MAX_SIZE 64
#define procfs_name "serdesd"

typedef enum
{
	ADVANCE_5bits,
	DELAY_5bits
}BENFIFO_DIR;

/* prototypes */
static int init_wan_serdes(void);
static ssize_t serder_proc_read(struct file *f, char *buffer, size_t cnt, loff_t *off);
static ssize_t serder_proc_write(struct file *f, const char *buffer, size_t cnt, loff_t *off);

static BOOL is_wan_type_auto(void)
{
    int  count;
    char wan_type_buf[PSP_BUFLEN_16] = {};

    count = kerSysScratchPadGet((char *)RDPA_WAN_TYPE_PSP_KEY, (char *)wan_type_buf, (int)sizeof(wan_type_buf));
    if (count == 0)
    {
        printk("Cannot read WAN type from the scratchpad\n");
        return FALSE;
    }

    if (strcasecmp(wan_type_buf, "AUTO") == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int init_wan_serdes(void)
{
    int  count;
    char wan_type_buf[PSP_BUFLEN_16] = {};
    char emac_id_buf[PSP_BUFLEN_16]  = {};
    char wan_rate_buf[PSP_BUFLEN_16] = {};

    count = kerSysScratchPadGet((char *)RDPA_WAN_TYPE_PSP_KEY, (char *)wan_type_buf, (int)sizeof(wan_type_buf));
    if (count == 0)
    {
        printk("Cannot read WAN type from the scratchpad\n");
        return -1;
    }

    if (strcasecmp(wan_type_buf, "GPON") == 0)
    {
        wan_serdes_config(SERDES_WAN_TYPE_GPON);
    }
    else if (strcasecmp(wan_type_buf, "EPON") == 0)
    {
        count = kerSysScratchPadGet((char *)RDPA_EPON_SPEED_PSP_KEY, (char *)wan_rate_buf, (int)sizeof(wan_rate_buf));
        if (count == 0)
        {
            printk("Cannot read EPON speed from the scratchpad\n");
            return -1;
        }

        if (!strcasecmp(wan_rate_buf, "Turbo"))
        {
            wan_serdes_config(SERDES_WAN_TYPE_EPON_2G);
        }
        else if (!strcasecmp(wan_rate_buf, "Normal"))
        {
            wan_serdes_config(SERDES_WAN_TYPE_EPON_1G);
        }
    }
    else if (strcasecmp(wan_type_buf, "GBE") == 0)
    {
        count = kerSysScratchPadGet((char *)RDPA_WAN_OEMAC_PSP_KEY, (char *)emac_id_buf, (int)sizeof(emac_id_buf));
        if (count == 0)
        {
            printk("Cannot read EMAC ID from the scratchpad\n");
            return -1;
        }
        
        if (strcasecmp(emac_id_buf, "EMAC5") == 0)
        {
            wan_serdes_config(SERDES_WAN_TYPE_AE);
        }
        else
        {
            printk("SerDes initialization is not required for %s\n", emac_id_buf);
            return 0;
        }
    }
    else
    {
        printk("SerDes initialization is not required for WAN type %s\n", wan_type_buf);
    }

    return 0;
}


/* This structure hold information about the /proc file */
struct proc_dir_entry *serdes_proc_file;

static struct file_operations serder_proc_fops = {
    .read = serder_proc_read,
    .write = serder_proc_write,
};

static ssize_t serder_proc_read(struct file *f, char *buffer, size_t cnt, loff_t *off)
{
    serdes_error_t rc;
    uint16_t ampl;
    int len = 0;

    printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name);
    if (*off != 0)
        return 0;

    rc = wan_serdes_amplitude_get(&ampl);
    if (rc == DRV_SERDES_NO_ERROR)
    {
        len = sprintf(buffer, "Tx amplitude=%d\n", ampl);
        printk("%s", buffer);
    }

    *off = len;
    return len;
}

static ssize_t serder_proc_write(struct file *f, const char *buffer, size_t cnt, loff_t *off)
{
    char procfs_buffer[PROCFS_MAX_SIZE];
    int procfs_buffer_size = cnt;
    serdes_error_t ret=0;
    int ampl_prm;
    char param_name[MAX_PARAM_LEN];
    char *name_ptr = param_name;
    static int serdes_init = 0;

    if (procfs_buffer_size > PROCFS_MAX_SIZE ) 
    {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }
	
    /* write data to the buffer */
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) 
    {
        return -EFAULT;
    }

    sscanf(procfs_buffer, "%s%d", param_name, &ampl_prm);
    /* make parameter a lower case */
    while (*name_ptr) 
    { 
        *name_ptr = tolower((unsigned char) *name_ptr); 
        name_ptr++; 
    }
	
    if (!strcmp (param_name, "init")) 
    {
        if (!serdes_init)
        {
            if (init_wan_serdes() == 0)
            {
                serdes_init = 1;
            }  
        } 
        else
        {
            printk("serdes already initialized.\n"); 
        }
    }
    else if (!strcmp (param_name, "amplitude"))
    {
        printk("ampl_prm:[%d]\n", ampl_prm);
        if (ampl_prm > 0 && ampl_prm < 32)
        {
            ret = wan_serdes_amplitude_set((uint16_t)ampl_prm);
            printk("fi_configure_serdes_amplitude_reg(): ret=%d\n", ret);
        }
        else 
        {
            printk("wrong amplitude %d. The value must be [1..31] \n", ampl_prm);
        }
    }
    else 
    {
        printk("error unsupported parameter. Supported: amplitude, init \n");
    }
	
    return procfs_buffer_size;
}


int init_serdes_proc(void)
{
    serdes_proc_file = proc_create(procfs_name, 0644, NULL, &serder_proc_fops);
	
    if (serdes_proc_file == NULL) 
    {
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", procfs_name);
        return -ENOMEM;
    }
    	
    return 0;	
}


void cleanup_serdes_proc(void)
{
    remove_proc_entry(procfs_name, NULL);
    printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}

void update_benfifo1_rd_ptr (BENFIFO_DIR benfifo1_rd_ptr_direction )
{
	uint8_t 	benfifo1_rd_ptr 	;
	uint8_t 	benfifo1_wd_ptr	= 0	;   // benfifo1 write_pointer default value
		
//  Read BEN1_RD_POINTER[bits 07:04] value
	FIELD_MREAD_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2, 4, 4 ,benfifo1_rd_ptr );  // <<< --- updated 11.4.2013
	
	if (benfifo1_rd_ptr_direction == ADVANCE_5bits)  {
		benfifo1_rd_ptr = benfifo1_rd_ptr + 1 ;
	}
	
	else if (benfifo1_rd_ptr_direction == DELAY_5bits)  {
		benfifo1_rd_ptr = benfifo1_rd_ptr - 1 ;
	}
	
	else {	
	}
	
// 	Write new  BEN1_RD_POINTER[bits 07:04] value
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2, 4, 4 ,benfifo1_rd_ptr );   // <<< --- updated 11.4.2013
		
// set ben_fifo1 write pointer to default value	to prevent fifo overrun 
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2, 0, 4 ,benfifo1_wd_ptr );   // <<< --- updated 17.4.2013
		
// Load new ptr value by reseting the BEN_FIFO1
// RegAddress = WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET , TXBEN_RESET = bit[02]
	
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET, 2, 1 , 1 ); // reset BEN_FIFO1 -> load ptr values
	udelay(20)	;
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET, 2, 1 , 0 ); // bring BEN_FIFO1 out of reset 
	udelay(50)	;
}

void clear_capture_window (void)
{
	// clear Capture_Window
	// WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_CFG ,  CLEAR_CAPTURE_WINDOW = bit[16]
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_CFG, 16, 1 , 1 )	; // <<< --- updated 11.4.2013
	udelay(20)	;
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_CFG, 16, 1 , 0 )	; // <<< --- updated 11.4.2013
	udelay(50)	;
}

uint8_t CheckESerDesStat (void) 				// check EPON serdes stat     // update 26.Jun.2013
{
    uint32_t reg32_data;	// 32bit data var					
    uint32_t EserdesStat;

    READ_32(ESERDES_STAT_WAN_GPIO_PER_REG ,reg32_data);
    reg32_data &= 0x0000FFFF;      // check bits [15:0]        
    EserdesStat = (reg32_data == 0x47) ? 0 : 0xf;
    printk ("EPON SerDes stat = 0x%x\n",EserdesStat);	
#if CHECK_ESERDES_STAT
    BUG_ON(EserdesStat == 0);
#endif
    return EserdesStat;    
}

void configWanEwake(uint8_t toff_time, uint8_t setup_time, uint8_t hold_time)
{
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 0, 1 ,0 );            //  EARLY_TXEN_BYPASS
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 8, 8 ,toff_time );    //  TOFF_TIME
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 16, 8 ,setup_time );  //  SETUP_TIME
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 24, 8 ,hold_time );   //  HOLD_TIME
}

void configWanGPONSerdes(void)
{
// Defines:
	#define WAN_devAddr			4
	#define Soft_RST_wordOffset	8


// Variables :	
	uint32_t 	reg32_data 				;	// 32bit data var					
	uint16_t 	mdio_data  				;   // MDIO (clause 22) 16bit rd\wr data var
	uint8_t		mdio_phyAddr = 0x01		;   // MDIO 8 bit phyAddr. var, in WAN always equal to 1
	uint8_t  	mdio_regAddr 			;   // MDIO 8 bit addr. var
	
	uint8_t  	BPCMdevAddr 			;
	uint8_t	 	BPCMwordOffset 			;

	
	//uint8_t 	tb_burst_header  	= 0xEA	;
	//uint8_t 	tb_burst_payload 	= 0x6E	;
	//uint8_t 	tb_burst_filler  	= 0 	;
	//uint8_t 	tb_burst_size    	= 4		;
	//uint8_t 	tb_gap_size      	= 21	;
	//uint8_t 	tb_ben_start_bit 	= 16	;
	
	uint8_t 	tb_loop 		 		= 0 	;
	//uint8_t 	tb_benfifo1_wd_ptr 		= 0		;
	//uint8_t 	tb_benfifo1_rd_ptr  	= 12	;
	uint8_t     tb_calibration_done 	= 0		;
    uint8_t     tb_calibration_failed 	= 0		;
	uint8_t 	tb_capture_updated 		= 0 	;
	uint32_t	tb_capture_window 		= 0 	;
	uint32_t benfifo1_rd_ptr;	
	uint16_t	transceiver				= 0		;


//v   assign tb_EPON_pattern = { {4{tb_burst_filler}}, tb_burst_header, {4{tb_burst_payload}} } >> ( 39 - tb_ben_start_bit );
//	uint32_t	ben_expected_pattern = 0x1D4DC	;



/*  ******** START GPON SerDes Init script  ********** */
	printk ("START GPON SerDes Init script\n");
// (***) Pre-init assumption - clocks & resets for wan block allready configured !!!


// (*) Select WAN interface = GPON, Laser not enabled yet !!!
	// LASER_MODE  			= bit[03:02] = 2'b0 = constant output 0
	// WAN_INTERFACE_SELECT = bit[01:00] = 2'b0 = GPON
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 0, 4 ,0 ); //  bit[3, 2, 1, 0] = 0


// (*)  configure gearbox mode	
	// WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0 :
	// 		sapis_rx_gen2[0]       = 1'b1;
	// 		sapis_tx_gen2[8]       = 1'b1;
	// 		rxfifo_20bit_order[1]  = 1'b1;
	// 		txfifo_20bit_order[10] = 1'b1;
	// 		loopback_rx[15]        =  1 \ 0
	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0 ,reg32_data) ;
	reg32_data = 0x00610503 ;   // Loopback_RX[bit 15] = 0 -> No loop back, Normal gearbox mode
	// reg32_data = 0x00618503 ; // Loopback_RX[bit 15] = 1 -> Debug mode ,Output of Rx FIFO is looped back to input of Tx FIFO
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0, reg32_data) ; 

	udelay(100);

    // (*)	configure SATA_PHY
	// 		SATA_MD_EN 			= bit[18] 	 = 1'b1 ;
	// 		SAPIS0_TX_ENABLE 	= bit[16] 	 = 1'b1 ;
	// 		SAPIS0_TX_DATA_RATE = bit[15:14] = 2'b1 ;	// 1 = GEN2
	// 		SAPIS0_RX_DATA_RATE = bit[13:12] = 2'b1 ;   // 1 = GEN2
	// 		INT_REF_EN 			= bit[2] 	 = 1'b1 ;

	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;
	reg32_data = ( reg32_data & 0xFFFF5FFF ) | 0x00055004;  //[bit15 ,13] = 0 , bit[18, 16, 14, 12, 2] = 1'b1
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;

	udelay(10000); 		// Wait for tb_sata_reset_n to settle down.
	
    // (*) Reset Sata Serdes rstb_mdioregs
	//	Vladimir - BPCM Read\Write commands :
	BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
	BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8

	ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
	reg32_data |= 0x00400000;   // rstb_mdioregs [22] = 1
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(10);

	reg32_data &= 0xFFBFFFFF;   // rstb_mdioregs [22] = 0  -   soft_reset_n[22]  Sata Serdes rstb_mdioregs
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(10);

	reg32_data |= 0x00400000;   // rstb_mdioregs [22] = 1
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(1000);


    ////////////////////////////////////////////////////////////////////////////////////////
    ////// (*) START 'gs_init'  - GPON SerDes MDIO = SATA3_DUAL_PHY init    ////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    // (1) Initialize PLL to use 25MHz refclk !!!
    // v	gs_mdio_wr ( 12'h1f, { `TxPll_A, 4'h0 } );    <--> tb_sata_mdio_wr_data = { 2'b01, 2'b01, phyad[4:0] = 5'h1, regad[4:0], 2'b10, data };
    // v	gs_mdio_rd ( `anaPllAControl1_A, regdata );
	// v	regdata[13:9] = 5'b10011;   // 25MHz int_refclk
    // v	gs_mdio_wr ( `anaPllAControl1_A, regdata );
	
	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x8050 ;   	//  { `TxPll_A, 4'h0 }  = {12'h805,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	mdio_regAddr = 0x1B ; // regad[4:0] = anaPllAControl1_A = {1'b1,`anaPllAControl1_Adr} = {1'b1, 4'hB} = 5'h1B ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data = ( mdio_data & 0xE7FF ) | 0x2600; // regdata[13:9] = 5'b10011;  regdata = ( regdata & and_data ) | or_data;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	

    // (2) Reset PLL !!!  !	signal control by - pmc soft reset
	//v     #100;
    //v		force wan_top.wan_serdes.rstb_pll = 1'b0;   // 
    //v     #1200;
    //v     force wan_top.wan_serdes.rstb_pll = 1'b1;   // 
	
    //	Vladimir - BPCM Read\Write commands :
	// ReadBPCMRegister(int devAddr, int wordOffset, uint32 *value);
	// WriteBPCMRegister(int devAddr, int wordOffset, uint32 value);
		
	BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
	BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
	
	ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
	
	reg32_data &= 0xFFDFFFFF;   // rdbt_pll[ bit21] = 0
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	
	udelay(1000);
	
	reg32_data |= 0x00200000;   // rdbt_pll[ bit21] = 1
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	
	udelay(1000);
	

	//enable the MEM_REB
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 5, 1, 0 );
	// (2.a) PLL Recalibrate\Retune 
	// WAN_TOP_WAN_MISC_SATA_CFG[1] = toggle Recalibrate=bit[1]=>  '0'--> '1' --> '0'
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,0 ); 
	udelay(100);
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,1 );  
	udelay(100);
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,0 ); 
	
    // (3) Disable Rx PMD 	
	    // Rx PMD0 Block, Rx PMD Control1 register:
	// v	gs_mdio_wr ( 12'h1f, { `PMDrx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `rxpmd_1rx_control1_A, regdata );
	// v	regdata[13] = 1'b0;  // rxpmd_en_frc_val =0 [bit 13]   
	// v	regdata[12] = 1'b1;  // rxpmd_en_frc     =1 [bit 12]
	// v	regdata[8:0] = 9'd0;  // rx_ppm_val      =0 [bit 8:0]
	// v	gs_mdio_wr ( `rxpmd_1rx_control1_A, regdata );
	
	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81C0 ;   	//  {`PMDrx0_A, 4'h0}  = {12'h81C,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	
	mdio_regAddr 	= 0x11   ;      // `rxpmd_1rx_control1_A = {1'b1,`rxpmd_1rx_control1_Adr} = {1'b1, 4'd1} = 5'h11 ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data = ( mdio_data & 0xDE00 ) | 0x1000;  // [bit13, bit 8:0] = 0 , [bit12] = 1;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);


    // (4) Disable Tx PMD and Host Tracking !!!
	// v	gs_mdio_wr ( 12'h1f, { `PMDtx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `txpmd_1tx_control1_A, regdata );
	// v	regdata[ 9] = 1'b0;  // tx_host_track    =0 [bit 9]
	// v	regdata[ 3] = 1'b0;  // tx_pmd_en_frc_val=0 [bit 3]
	// v	regdata[ 2] = 1'b1;  // tx_pmd_en_frc    =1 [bit 2]
	// v	regdata[ 1] = 1'b0;  // tx_ssc_en_frc_val=0 [bit 1]
	// v	regdata[ 0] = 1'b1;  // tx_ssc_en_frc    =1 [bit 0]
	// v	gs_mdio_wr ( `txpmd_1tx_control1_A, regdata );	
	
	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81A0 ;   	//  {`PMDtx0_A, 4'h0}  = {12'h81A,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	
	
	mdio_regAddr 	= 0x11   ;      // `txpmd_1tx_control1_A = {1'b1,`txpmd_1tx_control1_Adr} = {1'b1, 4'd1} = 5'h11 ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data = ( mdio_data & 0xFDF5 ) | 0x0005;  //[bit9 ,3 ,1 ] = 0 , [bit2 ,0] = 1;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    //(5) Disable comma detect
	// v	gs_mdio_wr ( 12'h1f, {`Rx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `anaRxControl1G_A, regdata );
	// v	regdata[ 8] = 1'b0;  // comdet_en=0         [bit 8]
	// v	gs_mdio_wr ( `anaRxControl1G_A, regdata );

	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x80B0 ;   	//  {`Rx0_A, 4'h0 }  = {12'h80B,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		

	mdio_regAddr 	= 0x19   ;      // `anaRxControl1G_A = {1'b1,`anaRxControl1G_Adr} = {1'b1,4'h9} = 5'h19
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data &= 0xFEFF  ; // [bit8] = 0
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	
	
    // (6) Enable sapis host tracking !!!	
	// v	gs_mdio_wr ( 12'h1f, { `PMDtx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `txpmd_1tx_control1_A, regdata );
	// v	regdata[ 9] = 1'b1;  // tx_host_track=1     [bit 9]
	// v	gs_mdio_wr ( `txpmd_1tx_control1_A, regdata );
	
	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81A0 ;   	// { `PMDtx0_A, 4'h0 } = { 12'h81A, 4'h0 }
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);			
	
	mdio_regAddr 	= 0x11   ;      // `txpmd_1tx_control1_A = {1'b1,`txpmd_1tx_control1_Adr} = {1'b1,4'd1} = 5'h11
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data |= 0x0200  	; 		// [bit9] = 1
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	
    // (7) Use CDR phase information for host tracking
	// v	// Tx PMD0 Block, Tx Frequency Control4 register:
	// v	gs_mdio_wr ( 16'h1f, { `PMDtx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `txpmd_1tx_freq_ctrl_control4_A, regdata );
	// v	regdata[14] = 1'b0;  // hstr_insel=0        [bit 14]
	// v	gs_mdio_wr ( `txpmd_1tx_freq_ctrl_control4_A, regdata );	
	
	mdio_regAddr   	= 0x1F   	;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81A0	;   	// { `PMDtx0_A, 4'h0 } = {12'h81A, 4'h0 }
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);			
	
	mdio_regAddr 	= 0x15   	;      // `txpmd_1tx_freq_ctrl_control4_A = {1'b1,`txpmd_1tx_freq_ctrl_control4_Adr} = {1'b1,4'd5} 
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data 		&= 0xBFFF  	;		//   [bit 14] =0 =hstr_insel
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	
	
    //(8) TX-PI follow RX-PI //Set hstr_upd_dis = 1 	
	// v	// Tx PMD0 Block, Tx Frequency Control5 register:
	// v	gs_mdio_wr (  16'h1f, { `PMDtx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `txpmd_1tx_freq_ctrl_control5_A, regdata );
	// v	regdata[ 2] = 1'b1;  // hstr_upd_dis=1      [bit 2]
	// v	gs_mdio_wr ( `txpmd_1tx_freq_ctrl_control5_A, regdata );	
	
	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81A0 ;    	// { `PMDtx0_A, 4'h0 } = {12'h81A, 4'h0 }
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);			
	
	mdio_regAddr 	= 0x16  ;      // `txpmd_1tx_freq_ctrl_control5_A = {1'b1,`txpmd_1tx_freq_ctrl_control5_Adr} = {1'b1,4'd6} 
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data |= 0x0004   ; //   [bit 2]= 1 = hstr_upd_dis;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	

    // (9) Rx CDR programming
    // Rx PMD0 Block, CDR registers:
	// not changing the values during init
	//`rxpmd_1rx_freq_frc_integ_A  = default reset values
	 
	
    //(10)  XGXS Block 0, XGXS Control register
	// v	gs_mdio_wr ( 16'h1f, { `XgxsBlk0_A, 4'h0 } );
	// v	gs_mdio_rd ( `xgxsControl_A, regdata );
	// v	regdata[14] = 1'b1;  // mdio_cont_en=1  [bit 14] to enable gloop MDIO controls
	// v	regdata[ 5] = 1'b1;  // pll_lock_gate_dis=1 [bit  5] to disable gating of pll_lock output to rxSeqDone_1 (unused link)
								 //  10/18/2012: pll_lock_gate_dis (0/1) has no impact on pll_lock (still desasserted).
	// v	gs_mdio_wr ( `xgxsControl_A, regdata );

	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 16'h1f
	mdio_data 		= 0x8000 ;    	// { `XgxsBlk0_A, 4'h0 } = {12'h800, 4'h0 }
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	

	mdio_regAddr 	= 0x10  ;      // `xgxsControl_A = {1'b1, `xgxsControl_Adr} = {1'b1, 4'h0}
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data |= 0x4020  ; // [bit14 ,5] = 1;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	
	
	
    //(11)  gloop control  = Global \ Remote loopback  = always set to 0
    // XGXS Block 1, Lane Control 2 register:

	// mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 16'h1f
	// mdio_data 		= 0x8010 ;    	// { `XgxsBlk1_A, 4'h0 } = {12'h801, 4'h0 }
	// mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	

	// mdio_regAddr 	= 0x17  ;      // laneCtrl2_A = {1'b1, `laneCtrl2_Adr} = {1'b1,4'h7}=5'h17
	// mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	// mdio_data = 0x0000  ; // gs_gloop_enable == 0 <-> regdata = 16'h0;
	// mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		
	
    //(12) Enable Rx PMD
	// v	// Rx PMD0 Block, Rx PMD Control1 register:
	// v	gs_mdio_wr ( 12'h1f, { `PMDrx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `rxpmd_1rx_control1_A, regdata );
	// v	regdata[13] = 1'b1;  // rxpmd_en_frc_val=1 [bit 13]
	// v	regdata[12] = 1'b1;  // rxpmd_en_frc    =1 [bit 12]
	// v	gs_mdio_wr ( `rxpmd_1rx_control1_A, regdata );

	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81C0 ;   	//  {`PMDrx0_A, 4'h0 }  = {12'h81C, 4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		

	mdio_regAddr 	= 0x11   ;      // `rxpmd_1rx_control1_A = {1'b1,`rxpmd_1rx_control1_Adr} = {1'b1,4'd1} = 5'h11
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data |= 0x3000  ; // [bit 13 ,12] = 1
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	


    //(13)  Enable Tx PMD
	// v	// Tx PMD0 Block, Tx PMD Control1 register:
	// v	gs_mdio_wr ( 12'h1f, { `PMDtx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `txpmd_1tx_control1_A, regdata );
	// v	regdata[ 3] = 1'b1;  // tx_pmd_en_frc_val=1 [bit 3]
	// v	regdata[ 2] = 1'b1;  // tx_pmd_en_frc    =1 [bit 2]
	// v	gs_mdio_wr ( `txpmd_1tx_control1_A, regdata );

	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81A0 ;   	//  {`PMDtx0_A, 4'h0 } }  = {12'h81A, 4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		

	mdio_regAddr 	= 0x11   ;      // `txpmd_1tx_control1_A = {1'b1,`txpmd_1tx_control1_Adr} = {1'b1, 4'd1} = 5'h11 ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data |= 0x000C  ; // [bit 3 ,2 ] = 1
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	


    // (14)  Force PLL lock  = USED ONLY FOR DEBUG !!!
	// v	gs_mdio_wr ( 12'h1f, { `TxPll_A, 4'h0 } );
	// v	gs_mdio_rd ( `anaPllControl_A, regdata );
	// v	regdata[0] = 1'b1;  // pllForcePllLock  =1 [bit 0]   <<<---- Debug only - remove
	// v	gs_mdio_wr ( `anaPllControl_A, regdata );
	/*
	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x8050 ;   	//  {`TxPll_A, 4'h0 } }  = {12'h12'h805, 4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	mdio_regAddr 	= 0x11   ;      // `anaPllControl_A = {1'b1,`anaPllControl_Adr} = {1'b1,4'h1} = 5'h11 ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, mdio_data);

	mdio_data |= 0x0001  ; // [bit 0] = 1
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	*/


    // (15)  Waiting for PLL lock    
	//   Register = WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT [Physical Address = 32'h130f_8008]
	//  [bit 9] = SATA_PLL_LOCK      - Indicates pll lock from sata serdes 
	//  [bit 4] = SAPIS0_RXLOCK_CDR  - 	Indicates that receiver is locked to incoming datas frequency and phase
	//  [bit 5] = SAPIS0_TXLOCK_CDR  - Indicates that transmitter clock frequency and phase is stable and ready for data transmission
    // 				  [ 0x1 = ENABLED    , Reset value is 0x0]

	udelay(1000);

	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
	
	FIELD_MREAD_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT, 31, 1 ,tb_capture_updated );

	while	( !(reg32_data & 0x00000200) )
	{
		READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
		udelay(1000);
	}	 
	udelay(100);
	printk (" !!!!   PLL locked !!!!!!    ");
    // (15)  Waiting for rxlock_cdr  
	while	( !(reg32_data & 0x00000010) )
	{
		READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
		udelay(1000);
	}	
	udelay(100);
	printk (" !!!!   RX CDR locked !!!!!!    ");
	
    // (16)  Waiting for txlock_cdr  
	while	( !(reg32_data & 0x00000020) )
	{
		READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
		udelay(1000);
	}	
		printk (" !!!!   TX CDR locked !!!!!!    ");
	udelay(100);
	
	
    //(17) Enable Tx FIFO	    
	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG ,reg32_data) ;
	reg32_data |= 0x00020000 ;   // [bit 17] = TX0_20B_FIFO_ON = 1'b1
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ; 
	

    ////////////////////////////////////////////////////////////////////////////////////////
    ////// (*) END 'gs_init'  - GPON SerDes MDIO = SATA3_DUAL_PHY init    ////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
	
	

    // (*)  Set Gearbox FIFO's pointers
	reg32_data = 0x00100030 ;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1, reg32_data);
	
	reg32_data = 0x0000E0C0 ;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2, reg32_data);

    // (*)  Reset Gearbox FIFO's pointers	
	reg32_data = 0x0000000F ;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET, reg32_data);
	
	udelay(100);
	
    // (*)  Take out of Reset gearbox FIFOs
	reg32_data = 0x00000000 ;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET, reg32_data);
	
	
    ////////////////////////////////////////////////////////////////////////////////////////
    ///////////////// 	(*) BURST Enable calibration       		////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////	
	
    // Configure Patter Generator -  for burst mode setting:	
    // (*)  Set PG -  Burst Mode and Expected Pattern	
	// v	gpon_gearbox.pg_mode[2:0]   = 3'd1;   // [Generate repetitive TX burst]
	// v	gpon_gearbox.header [15:8]  = 8'hEA
	// v	gpon_gearbox.payload[23:16] = 8'h6E
	// v	gpon_gearbox.filler [31:24] = 8'h0
	
	reg32_data = 0x006EEA01 ;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1, reg32_data);
	
    // (*)  Set PG - burst and gap size	
	// v	gpon_gearbox.burst_size [7:0]   = 8'h4
	// v	gpon_gearbox.gap_size  [15:8]   = 8'h15
	
	reg32_data = 0x00001504 ;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2, reg32_data);
	
    // (*)  Set capture settings	
	// v	gpon_gearbox.CLEAR_BEN_FIFO2_COLLISION [18] =0
	// v	gpon_gearbox.CLEAR_BEN_FIFO1_COLLISION [17] =0
	// v	gpon_gearbox.CLEAR_CAPTURE_WINDOW [16]     = 0
	// v	gpon_gearbox.SINGLE_CAPTURE [15] = 1   // If SINGLE_CAPTURE [15] = 1, the CAPTURE_WINDOW is updated on the next rising edge of Tx burst enable and will not be updated until the capture circuit is re-armed by setting CLEAR_CAPTURE_WINDOW to 1 and then to 0. 
	// v	gpon_gearbox.BEN_START_BIT[12:08] = 5'h10
	// v	gpon_gearbox.BEN_FINE_ADJUST [6:4] = 0    
	// v	gpon_gearbox.BEN_RD_PTR_ADV [1] = 0
	// v	gpon_gearbox.BEN_RD_PTR_DLY [0] = 0	
	
	reg32_data = 0x00009000 ;  // start_bit = 21  
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_CFG, reg32_data);
	
    ///// (*) Start BurstEnable - Coarse Calibration ! 
    tb_calibration_done 	= 0	;
    tb_calibration_failed 	= 0	;

    while	( ( tb_loop < 200 ) & (tb_calibration_done != 1 )  )	
    {
		udelay(10) 	;
        // wait for capture window to be updated
        while (tb_capture_updated	== 0) {  
            FIELD_MREAD_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_STATUS, 31, 1 ,tb_capture_updated );
            udelay(50) ;
        }

        udelay(10);
        FIELD_MREAD_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_BURST_STATUS, 0, 28 ,tb_capture_window ); // read capture ben window
        if 	( tb_capture_window ) {            
            if (tb_capture_window == 0x3 )	{   // old value --> tb_capture_window == 0x3
                tb_calibration_done 	= 1	;
				printk ("GPON BEN Calibration Done \n");
            }           

            else {
                update_benfifo1_rd_ptr (DELAY_5bits) ;
            }
        }	

        else {
            update_benfifo1_rd_ptr (DELAY_5bits) ;

        }

        udelay(10);
        clear_capture_window() ;
        tb_loop = tb_loop + 1;
        FIELD_MREAD_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2, 4, 4 ,benfifo1_rd_ptr ); 
        if (benfifo1_rd_ptr == 1) {
           // (*)  Set default Gearbox FIFO's pointers	
           reg32_data = 0x00100030 ;
           WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1, reg32_data);

           reg32_data = 0x0000E0C0 ;
           WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_2, reg32_data);

           // (*)  Reset Gearbox FIFO's pointers	
           reg32_data = 0x00000007 ;
           WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET, reg32_data);

           udelay(100);

           // (*)  Take out of Reset gearbox FIFOs
           reg32_data = 0x00000000 ;
           WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET, reg32_data);
        }
    }	

    /* ****************   BEN Coarse Calibration  = END ! **************************** */


    // (*) set final GPON Laser Enbable \ Burst Enable calibration using lab measured Ton result
	// accurate_BEN_calibration (); //    !!!!!   <<<<<<<<<<<<<<<----------------------------------- Need Update !!!1
	
    /* *************   BEN 'fine_adjust' Calibration  = END ! **************************** */	

    // (*) Turn OFF - Gearbox PG - BURST mode, give cotrol to GPON-MAC
    reg32_data = 0x0 ;  
    // (...GEARBOX_PATTERN_CFG1 -> )pg_mode[2:0] = 3'd0 =  Pattern generator disabled. GPON MAC has control of Tx output and burst enable.

    WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1, reg32_data); 
    WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2, reg32_data); 

    ////////////////////////////////////////////////////////////////////////////////////////
    ///////////////// 	(*) GPON - Final settings       		////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////	

    // set SATA pre-emphasis settings:   	//    
    // 	amplitude boost  =  1 = txi_ctrl<43>
    //  TX output amplitude = '11111'  = txi_ctrl<42:38>
    // 	post=10101  = txi_ctrl<35:31>
    //  main=11111  = txi_ctrl<30:26>    , 
    //
    // rem pre-emphasis selection main= '11111' , post ='10101' , boost = '1'
    // mml 0x14e00620 0x243f0060 
    // mml 0x14e00620 0x24330fCA 
    // mml 0x14e00620 0x2432FC00 

	mdio_regAddr   	= 0x1F   ;    	//  regad[4:0] = 12'h1f
	mdio_data 		= 0x8060 ;   	//  SATA3_TX0: TX0 Register Bank = 0x0060  
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		

	mdio_regAddr 	= 0x13   ;      // SATA3_TX_txAcontrol3 = 0x083 ==> {1'b1,SATA3_TX_txAcontrol3} = {1'b1, 4'd3} = 5'h13 ; 
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	// txi_ctrl<43>    = mdio_data [bit 11] 	= 1	
	// txi_ctrl<42:38> = mdio_data [bit 10:06]	= 11111
	// txi_ctrl<35:32> = mdio_data [bit 03:00]  = 1010
	mdio_data = ( mdio_data & 0xFFF5 ) | 0x0FC5;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	

	
	mdio_regAddr 	= 0x12   ;      // SATA3_TX_txAcontrol2= 0x082 ==> {1'b1,SATA3_TX_txAcontrol2} = {1'b1, 4'd3} = 5'h12 ; 
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	// txi_ctrl<31>    = mdio_data [bit 15] 	= 1	
	// txi_ctrl<30:26>  = mdio_data [bit 14:10]	= 11111
	mdio_data = ( mdio_data & 0x7FFF ) | 0x7C00;  //
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		
	
// (*) Tutn ON the LASER
	// LASER_MODE = bit[03:02] = 2'b1 = gpon laser_en
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 2, 2 , 1 ); 

	// enable the Laser ON

	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 6, 1, 1);
	bcm_i2c_pon_optics_type_get(&transceiver);
	if (transceiver != BCM_I2C_PON_OPTICS_TYPE_PMD)
	{   /* disable 25MHZ clk if PMD not present*/
		FIELD_MWRITE_32(PERIPH_BLOCK_PLLCNTRL_PER_OSC_CTRL, 8, 1, 0);
	}
	else
	{
	    unsigned short polarity;

	    BpGetPmdInvSerdesTxPol(&polarity);

	    /* revert tx polarity */
	    if (polarity == pmd_polarity_invert)
	    {
	        //FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 4, 1 , 1 );  // bit [04] = Laser_Invert = â1â  [Reset value = â0â | Invert = â1â  ]
	        mdio_regAddr             = 0x1F   ;         // regad[4:0] = 12'h1f
	        mdio_data                   = 0x8060 ;      // RX0 Register Bank = Block Address = 0xB0
	        mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	        mdio_regAddr             = 0x10   ;      // select Register Address = 0x0
	        mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

	        mdio_data =  mdio_data | 0x0040 ; //     bits[8:7] = â11â
	        mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	    }

	    BpGetPmdInvSerdesRxPol(&polarity);

	    /* revert rx polarity */
	    if (polarity == pmd_polarity_invert)
	    {
	        mdio_regAddr             = 0x1F   ;         // regad[4:0] = 12'h1f
	        mdio_data                   = 0x80B0 ;      // RX0 Register Bank = Block Address = 0xB0
	        mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	        mdio_regAddr             = 0x1A   ;      // select Register Address = 0xA
	        mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

	        mdio_data = ( mdio_data | 0x0180); //     bits[8:7] = .11.
	        mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
	    }
	    configWanEwake(PMD_EWAKE_OFF_TIME, PMD_EWAKE_SETUP_TIME, PMD_EWAKE_HOLD_TIME);
	}

	printk ("GPON SerDes Initialization Sequence Done \n");

	// Setup NTR synch pulse for APM           // [bit 27:24] = clock divider for sync pulse
	// [bit 27:24] = 14, Divide 248.832Mhz GPON Recovered clock by 5 to yeild 256Khz NTR sync pulse
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 24, 4, 5 );
	///////////////      GPON =  END Sata+Gearbox Initialization Sequence	///////////////
}	

/* *********************************************** */

void configWanEPONSerdes(serdes_wan_type_t wan_type)
{
    /*  ********  EPON Serdes Init script  ********** */
    // Defines:
	#define WAN_devAddr			4
	#define Soft_RST_wordOffset	8



    // Variables :
	uint32_t 	reg32_data 				;	// 32bit data var
	uint16_t 	mdio_data  				;   // MDIO (clause 22) 16bit rd\wr data var
	uint8_t		mdio_phyAddr = 0x01		;   // MDIO 8 bit phyAddr. var, in WAN always equal to 1
	uint8_t  	mdio_regAddr 			;   // MDIO 8 bit addr. var
	uint8_t 	tb_capture_updated 	= 0	;
	uint8_t  	BPCMdevAddr 			;
	uint8_t	 	BPCMwordOffset 			;
	uint8_t		Estat = 0x0f 			;
	uint16_t	transceiver = 0			;
	BOOL is_auto = FALSE;

	is_auto = is_wan_type_auto();
	BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
	BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8

	printk ("START EPON SerDes Init script\n");
	

	if (is_auto)
	{
		// //////////////// Start Change: update 18.Feb.2015 (remove extra 'Soft-Reset' from Epon.init.sequence )
		//// // (*) pre-init resets
		//// reg32_data = 0x00000001;   
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		
		
		BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
		ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//	printk ("DEBUG - step0: SoftReset Value =0x%x \n",reg32_data);
			
		//	reg32_data = 0xfff0ff01;  // All Epon & SataSerDes = Out of Reset / Gpon & AE = Held in Reset
		//	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);	
		//	ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//	printk ("DEBUG - step0.1: SoftReset Value =0x%x \n",reg32_data);

		// //////////////// End-change:  update 18.Feb.2015   /////////////////////////	
	}
	else
	{ 
		// (*) pre-init resets
		reg32_data = 0x00000001;   
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	}
	
    // (*) Select WAN interface = EPON, Laser not enabled yet !!!
	// LASER_MODE  			= bit[03:02] = 2'b0 = constant output 0
	// WAN_INTERFACE_SELECT = bit[01:00] = 2'b1 = EPON
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 0, 4 , 1 ); //  bit[3: 2] = 0 , bit[1:0]=1

	Estat = CheckESerDesStat();

    // (*)	configure SATA_PHY
    // 		SATA_MD_EN 			= bit[18] 	 = 1'b1 ;
    // 		SAPIS0_TX_ENABLE 	= bit[16] 	 = 1'b1 ;
    // 		SAPIS0_TX_DATA_RATE = bit[15:14] = 2'b0 ;	// 0 = GEN1  
    // 		SAPIS0_RX_DATA_RATE = bit[13:12] = 2'b1 ;   // Turbo-EPON = 1 = GEN2 / Normal-Epon =0 = GEN1
    //  	INT_REF_EN 			= bit[2] 	 = 1'b1 ;

    READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG,reg32_data) ;
    if (SERDES_WAN_TYPE_EPON_2G == wan_type)
    {
        reg32_data = 0x00051004;     ;     // Turbo-Epon = 0x00051004      // orig. Epon value = 0x00050004 
    }
    else if (SERDES_WAN_TYPE_EPON_1G == wan_type)
    {
        reg32_data = 0x00050004;  
    }
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;

	udelay(10000); 		// Wait for tb_sata_reset_n to settle down.

	if (is_auto)
	{
		// //////////////// Start Change: update 18.Feb.2015 (remove extra 'Soft-Reset' from Epon.init.sequence )
		// (*) update - resets
		//	reg32_data = 0xfff00001;   
		//	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		//
		//
	    //// // (*) Reset Sata Serdes rstb_mdioregs
		//// //	Vladimir - BPCM Read\Write commands :
		//// BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		//// BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
	    //// 
		//// ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//// reg32_data = 0xffb00001;  
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		//// udelay(10);
	    //// 
		//// reg32_data = 0xfff00001;  
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		//// udelay(10);
		
		// (*) Reset Sata Serdes rstb_mdioregs
		//	Vladimir - BPCM Read\Write commands :
		BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
		ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//	printk ("DEBUG - step1: SoftReset Value =0x%x \n",reg32_data);
		reg32_data &= 0xFFBFFFFF;   // rstb_mdioregs [22] = 0  -   soft_reset_n[22]  Sata Serdes rstb_mdioregs
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(10);

		reg32_data |= 0x00400000;   // rstb_mdioregs [22] = 1
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(1000);
		// //////////////// End-change:  update 18.Feb.2015   /////////////////////////	
	}
	else
	{
		// (*) update - resets
		reg32_data = 0xfff00001;   
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);

		// (*) Reset Sata Serdes rstb_mdioregs
		//	Vladimir - BPCM Read\Write commands :
		BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
		ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		reg32_data = 0xffb00001;  
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(10);

		reg32_data = 0xfff00001;  
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(10);  
	}

	udelay(1000);
    ////////////////////////////////////////////////////////////////////////////////////////
    ////// (*) START 'gs_init'  - EPON SerDes MDIO = SATA3_DUAL_PHY init    ////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    // WAN_KIND_EPON
    //------------------


    // (1)  // EPON-TB  // ref clock divide
        // EPON-TB  ser_pcb_rd(12'h8b, test);
        // EPON-TB  test[13:9] =  5'b01110;
        // EPON-TB  ser_pcb_wr(12'h8b, test);

        // Initialize PLL to use 25MHz refclk !!!
        // v	gs_mdio_wr ( 12'h1f, { `TxPll_A, 4'h0 } );    <--> tb_sata_mdio_wr_data = { 2'b01, 2'b01, phyad[4:0] = 5'h1, regad[4:0], 2'b10, data };
    // v	gs_mdio_rd ( `anaPllAControl1_A, regdata );
	// v	regdata[13:9] = 5'b10011;   // 25MHz int_refclk
    // v	gs_mdio_wr ( `anaPllAControl1_A, regdata );


	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x8050 ;   	//  { `TxPll_A, 4'h0 }  = {12'h805,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	mdio_regAddr = 0x1B ; // regad[4:0] = anaPllAControl1_A = {1'b1,`anaPllAControl1_Adr} = {1'b1, 4'hB} = 5'h1B ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

	mdio_data = ( mdio_data & 0xE7FF ) | 0x2600; // regdata[13:9] = 5'b10011;  regdata = ( regdata & and_data ) | or_data;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	if (is_auto)
	{
		// //////////////// Start Change: update 18.Feb.2015 (remove extra 'Soft-Reset' from Epon.init.sequence )
		//// // (2) Reset PLL !!!  !	signal control by - pmc soft reset
		//// //	Vladimir - BPCM Read\Write commands :
		//// BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		//// BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
	    //// 
		//// ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//// reg32_data = 0xffd00001;   // rdbt_pll[ bit21] = 0      - soft_reset_n[21]  Sata Serdes rstb_pll
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		//// udelay(1000);
	    //// 
		//// reg32_data = 0xfff00001;  
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		//// udelay(1000);
		
		BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
		
		ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//	printk ("DEBUG - step2: SoftReset Value =0x%x \n",reg32_data);
		
		reg32_data &= 0xFFDFFFFF;   // rdbt_pll[ bit21] = 0
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		
		udelay(1000);
		
		reg32_data |= 0x00200000;   // rdbt_pll[ bit21] = 1
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(1000);
		// //////////////// End-change:  update 18.Feb.2015   /////////////////////////	
	}
	else
	{        
		// (2) Reset PLL !!!  ! signal control by - pmc soft reset
		//  Vladimir - BPCM Read\Write commands :
		BPCMdevAddr     = WAN_devAddr           ;  // WAN_devAddr = 4
		BPCMwordOffset  = Soft_RST_wordOffset   ;  // Soft_RST_wordOffset = 8

		ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		reg32_data = 0xffd00001;   // rdbt_pll[ bit21] = 0      - soft_reset_n[21]  Sata Serdes rstb_pll
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(1000);

		reg32_data = 0xfff00001;  
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
		udelay(1000);        
	}
	
	// (2.a) PLL Recalibrate\Retune 
	// WAN_TOP_WAN_MISC_SATA_CFG[1] = toggle Recalibrate=bit[1]=>  '0'--> '1' --> '0'
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,0 ); 
	udelay(100);
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,1 );  
	udelay(100);
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,0 ); 
	
	//(3) Disable comma detect
    // EPON-TB  // Disable Comma Detect

	// e	gs_mdio2_wr ( 12'h1f, 16'hb0);
    // e    compute gs_mdio2_rd (12'h19, regdata);
    // e    regdata[8:8] = 1'b0;
    // e    gs_mdio2_wr (12'h19, regdata);

	// v	gs_mdio_wr ( 12'h1f, {`Rx0_A, 4'h0 } );
	// v	gs_mdio_rd ( `anaRxControl1G_A, regdata );
	// v	regdata[ 8] = 1'b0;  // comdet_en=0         [bit 8]
	// v	gs_mdio_wr ( `anaRxControl1G_A, regdata );

	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x80B0 ;   	//  {`Rx0_A, 4'h0 }  = {12'h80B,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	mdio_regAddr 	= 0x19   ;      // `anaRxControl1G_A = {1'b1,`anaRxControl1G_Adr} = {1'b1,4'h9} = 5'h19
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

	mdio_data &= 0xFEFF  ; // [bit8] = 0
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

	mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
	mdio_data 		= 0x81A0 ;   	//  {`PMDtx0_A, 4'h0}  = {12'h81A,4'h0} ;
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	
	
	mdio_regAddr 	= 0x11   ;      // `txpmd_1tx_control1_A = {1'b1,`txpmd_1tx_control1_Adr} = {1'b1, 4'd1} = 5'h11 ;
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	mdio_data =  mdio_data & 0xFDF0;  //[bit9 ,3 ,1 ] = 0 , [bit2 ,0] = 0;   
									 //	update 22.Aprl.2013 = bit[2] = 0  -> must be 0 to use 'host tracking' !!! 
									 // if bit[2] = 1 -> tx_cdr will not lock !
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    // EPON-TB  #1000;
	udelay(1000);
// EPON-TB  #130000;
// (8)  Waiting for PLL lock
	//   Register = WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT [Physical Address = 32'h130f_8008]
	//  [bit 9] = SATA_PLL_LOCK      - Indicates pll lock from sata serdes 
	//  [bit 4] = SAPIS0_RXLOCK_CDR  - 	Indicates that receiver is locked to incoming datas frequency and phase
	//  [bit 5] = SAPIS0_TXLOCK_CDR  - Indicates that transmitter clock frequency and phase is stable and ready for data transmission
    // 				  [ 0x1 = ENABLED    , Reset value is 0x0]

	udelay(1000);
	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
	FIELD_MREAD_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT, 31, 1 ,tb_capture_updated );

	while	( !(reg32_data & 0x00000200) )
	{
		READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
		udelay(1000);
//		printk ("DEBUG: EPON SerDes - wait PLL lock \n");
	}	 
	udelay(100);
// (9)  Waiting for rxlock_cdr  
	while	( !(reg32_data & 0x00000010) )
	{
		READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
		udelay(1000);
//		printk ("DEBUG:EPON SerDes - wait Rx-Lock \n");		
	}	
	udelay(1000);
// (10)  Waiting for txlock_cdr  
	while	( !(reg32_data & 0x00000020) )
	{
		READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_STAT ,reg32_data) ;
		udelay(1000);
//		printk ("DEBUG:EPON SerDes - wait Rx-Lock \n");		
	}	
	udelay(1000);

	if (is_auto)
	{
		// //////////////// Start Change: update 18.Feb.2015 (remove extra 'Soft-Reset' from Epon.init.sequence )  
		BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
		BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8
		
		ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
		//	printk ("DEBUG - step4: SoftReset Value =0x%x \n",reg32_data);

		//// // (*) update - reset : After PLL Locked - take soft_reset  Out of Reset !!!
		//// reg32_data = 0xfff0f001;  // EPON gearbox  reset deasert
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	    //// 
	    //// 
		//// reg32_data = 0xfff0f101;  // EPON main clock  reset deasert
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	    //// 
		//// reg32_data = 0xfff0f701;  // EPON BBH  reset deasert
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	    //// 
		//// reg32_data = 0xfff0ff01;  // EPON core  reset deasert
		//// WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);	
		// //////////////// End-change:  update 18.Feb.2015   /////////////////////////	
	}
	else
	{   
		// (*) update - reset : After PLL Locked - take soft_reset  Out of Reset !!!
		reg32_data = 0xfff0f001;  // EPON gearbox  reset deasert
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);


		reg32_data = 0xfff0f101;  // EPON main clock  reset deasert
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);

		reg32_data = 0xfff0f701;  // EPON BBH  reset deasert
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);

		reg32_data = 0xfff0ff01;  // EPON core  reset deasert
		WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data); 
	}
	
     ////// (*) END 'gs_init'  - EPON SerDes MDIO = SATA3_DUAL_PHY init    ////////////////

     ////////////////////////////////////////////////////////////////////////////////////////
     ///////////////// 	(*) EPON - Final settings       		////////////////////////////
     ////////////////////////////////////////////////////////////////////////////////////////	

     // set SATA pre-emphasis settings:   	//    <<<<<<<<<<------------ Those are GPON pre-emphasis settings MAYBE will NEED UPDATE  ?!?!?!?!?!?  !!!!! 
     // 	amplitude boost  =  1 = txi_ctrl<43>
     //  TX output amplitude = '11111'  = txi_ctrl<42:38>
     // 	post=10101  = txi_ctrl<35:31>
     //  main=11111  = txi_ctrl<30:26>    , 
     //
     // rem pre-emphasis selection main= '11111' , post ='10101' , boost = '1'
     // mml 0x14e00620 0x243f0060 
     // mml 0x14e00620 0x24330fCA 
     // mml 0x14e00620 0x2432FC00 


	mdio_regAddr   	= 0x1F   ;    	//  regad[4:0] = 12'h1f
	mdio_data 		= 0x8060 ;   	//  SATA3_TX0: TX0 Register Bank = 0x0060  
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		

	mdio_regAddr 	= 0x13   ;      // SATA3_TX_txAcontrol3 = 0x083 ==> {1'b1,SATA3_TX_txAcontrol3} = {1'b1, 4'd3} = 5'h13 ; 
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	// txi_ctrl<43>    = mdio_data [bit 11] 	= 1	
	// txi_ctrl<42:38> = mdio_data [bit 10:06]	= 11111
	// txi_ctrl<35:32> = mdio_data [bit 03:00]  = 1010
	mdio_data = 0x0FC5;  // 

	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);	

	
	mdio_regAddr 	= 0x12   ;      // SATA3_TX_txAcontrol2= 0x082 ==> {1'b1,SATA3_TX_txAcontrol2} = {1'b1, 4'd3} = 5'h12 ; 
	mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
	
	// txi_ctrl<31>    = mdio_data [bit 15] 	= 1	
	// txi_ctrl<30:26>  = mdio_data [bit 14:10]	= 1111
	mdio_data |=  0x7C00;  // 
	mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);		
	

//(11) Enable Tx FIFO	 // [bit 17] = TX0_20B_FIFO_ON = 1'b1
    //   [post_gs_init]
	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG ,reg32_data) ;
	reg32_data |= 0x00020000 ;   // [bit 17] = TX0_20B_FIFO_ON = 1'b1
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;
	
	
// (*) Tutn ON the LASER
	// LASER_MODE = bit[03:02] = 2'b2 = output/input epon laser_en 
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 2, 2 , 2 ); 

    //enable the MEM_REB and Laser ON
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 5, 1, 0 );
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 6, 1, 1 );
    bcm_i2c_pon_optics_type_get(&transceiver);
    if (transceiver != BCM_I2C_PON_OPTICS_TYPE_PMD) /* disable 25MHZ clk if PMD not present*/
        FIELD_MWRITE_32(PERIPH_BLOCK_PLLCNTRL_PER_OSC_CTRL, 8, 1, 0);
    else
	{
        unsigned short polarity;

        BpGetPmdInvSerdesTxPol(&polarity);

        /* revert tx polarity */
        if (polarity == pmd_polarity_invert)
        {
            //FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 4, 1 , 1 );  // bit [04] = Laser_Invert = â1â  [Reset value = â0â | Invert = â1â  ]
            mdio_regAddr             = 0x1F   ;         // regad[4:0] = 12'h1f
            mdio_data                   = 0x8060 ;      // RX0 Register Bank = Block Address = 0xB0
            mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

            mdio_regAddr             = 0x10   ;      // select Register Address = 0x0
            mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

            mdio_data =  mdio_data | 0x0040 ; //     bits[8:7] = â11â
            mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
        }

        BpGetPmdInvSerdesRxPol(&polarity);

        /* revert rx polarity */
        if (polarity == pmd_polarity_invert)
        {
            mdio_regAddr             = 0x1F   ;         // regad[4:0] = 12'h1f
            mdio_data                   = 0x80B0 ;      // RX0 Register Bank = Block Address = 0xB0
            mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

            mdio_regAddr             = 0x1A   ;      // select Register Address = 0xA
            mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

            mdio_data = ( mdio_data | 0x0180); //     bits[8:7] = â11â
            mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
        }
        configWanEwake(PMD_EWAKE_OFF_TIME, PMD_EWAKE_SETUP_TIME, PMD_EWAKE_HOLD_TIME);
	}


	printk ("EPON SerDes Initialization Sequence Done \n");
	// [bit 27:24] = 14, Divide 250Mhz EPON Recovered clock by 14 to yeild 40Khz NTR sync pulse
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 24, 4, 14 ); 
	
	
///////////////      EPON =  END Sata Initialization Sequence	///////////////
}






void configWanAESerdes(void)
{
/*  *********  AE Serdes Init script  *********** */
// Defines:
	#define WAN_devAddr			4
	#define Soft_RST_wordOffset	8



// Variables :
	uint32_t 	reg32_data 				;	// 32bit data var
//	uint16_t 	mdio_data  				;   // MDIO (clause 22) 16bit rd\wr data var
//	uint8_t		mdio_phyAddr = 0x01		;   // MDIO 8 bit phyAddr. var, in WAN always equal to 1
//	uint8_t  	mdio_regAddr 			;   // MDIO 8 bit addr. var

	uint8_t  	BPCMdevAddr 			;
	uint8_t	 	BPCMwordOffset 			;
	uint16_t    mdio_data               ;   // MDIO (clause 22) 16bit rd\wr data var
	uint8_t     mdio_phyAddr = 0x01     ;   // MDIO 8 bit phyAddr. var, in WAN always equal to 1
	uint8_t     mdio_regAddr            ;   // MDIO 8 bit addr. var


	// (*) Reset Sata Serdes rstb_mdioregs
	//	Vladimir - BPCM Read\Write commands :
	BPCMdevAddr 	= WAN_devAddr 			;  // WAN_devAddr = 4
	BPCMwordOffset 	= Soft_RST_wordOffset	;  // Soft_RST_wordOffset = 8

	reg32_data = 0x3000002;
	printk ("START AE SerDes Init script WAN BPCM reg = 0x%x\n",reg32_data);
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, reg32_data) ;
	 // (*)	configure SATA_PHY
	// 		SATA_MD_EN 			= bit[18] 	 = 1'b1 ;
	// 		SAPIS0_TX_ENABLE 	= bit[16] 	 = 1'b1 ;
	// 		SAPIS0_TX_DATA_RATE = bit[15:14] = 2'b0 ;	// 0 = GEN1
	// 		SAPIS0_RX_DATA_RATE = bit[13:12] = 2'b0 ;   // 0 = GEN1
	// 		INT_REF_EN 			= bit[2] 	 = 1'b1
	 // (*) Select WAN interface = GPON, Laser not enabled yet !!!
	// LASER_MODE  			= bit[03:02] = 2'b0 = constant output 0
	// WAN_INTERFACE_SELECT = bit[01:00] = 2'b2 = AE
	reg32_data = 0x00050004;
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;

	udelay(10000); 		// Wait for tb_sata_reset_n to settle down.


	// (*) update - resets
    // (*) Reset Sata Serdes rstb_mdioregs
	//	Vladimir - BPCM Read\Write c
	ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
	reg32_data &= 0xFFBFFFFF;   // rstb_mdioregs [22] = 0  -   soft_reset_n[22]  Sata Serdes rstb_mdioregs
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(10);
	reg32_data |= 0x00400000;   // rstb_mdioregs [22] = 1
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(1000);


    // (*)	configure SATA_PHY
    printk ("(*) Configure SATA_PHY  - TX=RX = GEN1 = AE bit rate");

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x50;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1b;
	mdio_data = 0xa600;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	ReadBPCMRegister(BPCMdevAddr, BPCMwordOffset, (uint32*)&reg32_data);
	reg32_data &= 0xffdfffff;   // rstb_mdioregs [22] = 0  -   soft_reset_n[22]  Sata Serdes rstb_mdioregs
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(10);
	reg32_data |= 0x00200000;   // rstb_mdioregs [22] = 1
	WriteBPCMRegister(BPCMdevAddr, BPCMwordOffset, reg32_data);
	udelay(1000);

    // (*) PLL Recalibrate\Retune 
    // WAN_TOP_WAN_MISC_SATA_CFG[1] = toggle Recalibrate=bit[1]=> '0'--> '1' --> '0'
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,0 ); 
    udelay(100);
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,1 ); 
    udelay(100);
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, 1, 1 ,0 );
	udelay(10000);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0xb0;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x19;
	mdio_data = 0x07d3;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);


	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x19;
	mdio_data = 0xa00;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x81a0;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x11;
	mdio_data = 0x1005;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x81a0;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x11;
	mdio_data = 0x1200;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x81a0;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x15;
	mdio_data = 0x165c;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x81a0;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x16;
	mdio_data = 0x34;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);


	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x60;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);


	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x13;
	mdio_data = 0x0fCA;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x12;
	mdio_data = 0x0fc00;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x1f;
	mdio_data = 0x60;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);

	mdio_phyAddr = 0x1;
	mdio_regAddr = 0x10;
	mdio_data = 0x0;
	if ( mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data))
		printk ("failed MDIO_SATA write reg=0x%x data=0x%x\n ",mdio_regAddr,mdio_data);


	READ_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG,reg32_data) ;
	reg32_data =  0x00070004;  //[bit15,14 ,13,12 ] = 0 , bit[18, 16, 2] = 1'b1
	WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;

    set_pinmux(13,5);
    gpio_set_dir(13,1);
    gpio_set_data(13,1);

    set_pinmux(0,5);
	set_pinmux(1,5);

	set_pinmux(61,7);
	gpio_set_dir(0,1);
	gpio_set_dir(1,1);
	gpio_set_data(0,1);
	gpio_set_data(1,1);

	//FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 2, 2 , 2 );
	//enable the MEM_REB and Laser ON
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 5, 1, 0 );
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 6, 1, 1 );
}

int wan_serdes_config(serdes_wan_type_t wan_type)
{
    unsigned short pol, is_ae = (wan_type == SERDES_WAN_TYPE_AE);

    set_pinmux(62, is_ae ? 5 : 1); /// PTIN: Change GPIO_62 to be on GPIO mode
    gpio_set_dir(62,1); ///PTIN: GPIO_62 as output

    if (is_ae)
    {
        BpGetAePolarity(&pol); /// PTIN: Gets the Active Ethernet TX_EN polarity from BP.
        gpio_set_data(62,pol); ///PTIN: setting value from BP (default is 0) on GPIO_62 - its connected to sfp TX Disable or to sff TX_EN.
    }

    switch(wan_type)
    {
    case SERDES_WAN_TYPE_GPON:
         configWanGPONSerdes();
         break;
    case SERDES_WAN_TYPE_EPON_1G:
    case SERDES_WAN_TYPE_EPON_2G:
         configWanEPONSerdes(wan_type);
         break;
    case SERDES_WAN_TYPE_AE:
         configWanAESerdes();
         break;
    default:
        printk("Error on %s , wrong wan_type=%d\n",__FUNCTION__,wan_type);
        return -1;
    }
    /* Set WAN MAC and laser according to type */
    rdp_post_init_fiber();

    return 0;
}
EXPORT_SYMBOL(wan_serdes_config);

void wan_register_pmd_prbs_callback(PMD_DEV_ENABLE_PRBS callback)
{
    pmd_prbs_callback = callback;
}
EXPORT_SYMBOL(wan_register_pmd_prbs_callback);

void wan_prbs_gen(uint32_t enable, int enable_host_tracking, int mode,
    serdes_wan_type_t wan_type, bdmf_boolean *valid)
{
    // Variables :
    uint16_t    mdio_data               ;   // MDIO (clause 22) 16bit rd\wr data var
    uint8_t     mdio_phyAddr = 0x01     ;   // MDIO 8 bit phyAddr. var, in WAN always equal to 1
    uint8_t     mdio_regAddr            ;   // MDIO 8 bit addr. var
    uint32_t    reg32_data              ;   // 32bit data var
    uint16_t    transceiver;

    bcm_i2c_optics_tx_control(BCM_I2C_OPTICS_ENABLE);

    bcm_i2c_pon_optics_type_get(&transceiver);
    if (transceiver == BCM_I2C_PON_OPTICS_TYPE_PMD && enable)
    {   
        if (pmd_prbs_callback)
            pmd_prbs_callback((uint16_t)enable, 1);
    }     

    if (wan_type != SERDES_WAN_TYPE_AE)
    {
        if (enable)
        {
            set_pinmux(62, 5);
            gpio_set_dir(62, 1);
            gpio_set_data(62, 1);
        }
        else
        {
            set_pinmux(62, 1);
        }
    }

    *valid = 0;
    printk ("START SerDes Init script");
    /* Check if disable configure the serdess again */
    if (!enable)
        goto reconf_serdess;

    if (wan_type == SERDES_WAN_TYPE_GPON)
    {
        // Disable Tx PMD and Host Tracking !!!
        mdio_regAddr   	= 0x1F   ;    	// regad[4:0] = 12'h1f
        mdio_data 		= 0x81A0 ;   	//  {`PMDtx0_A, 4'h0}  = {12'h81A,4'h0} ;
        mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

        mdio_regAddr 	= 0x11   ;      //
        mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

        if (!enable_host_tracking)
            mdio_data =  mdio_data & 0xFDF0;  //[bit9 ,3 ,1 ] = 0 , [bit2 ,0] = 0;
        else
            mdio_data |= 0x0200; //[bit9] = 1 TX host track
        mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    }

    // '  --> Register controlled RX data rate :
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8000 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);
    mdio_regAddr = 0x1E     ;
    if (wan_type == SERDES_WAN_TYPE_GPON)
    {
        mdio_data    = 0x1400   ;
    }
    else if (wan_type == SERDES_WAN_TYPE_EPON_1G)
    {
        mdio_data    = 0x0400   ;
    }
    else if (wan_type == SERDES_WAN_TYPE_EPON_2G)
    {
        mdio_data    = 0x1400   ;     // Turbo-Eppon = mdio_data    = 0x0400
    }
    //    bit[10] = 1: use data_rate_frc_val0
    //    bit[13:12] = data_rate_frc_val0 [Channel 0 data_rate bypass value]
    //  mdio_data = 0x0400  // [data_rate = 1.25Gbps]
    //  mdio_data = 0x1400  // [data_rate = 2.5Gbps]
    //  mdio_data = 0x2400  // [data_rate = 5Gbps]
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    udelay(100);

    //  --> Register controlled TX data rate:
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8010 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);
    mdio_regAddr = 0x13     ;
    mdio_data    = 0x0010   ;
    //  bit[04] = 1: tx0_data_rate_frc_en
    //  bit[01:00] = tx0_data_rate_val [Channel 0 TX data rate value when tx0_data_rate_frc_en=1]
    // mdio_data = 0x0010   // [data_rate = 1.25Gbps]
    // mdio_data = 0x0011   // [data_rate = 2.5Gbps]
    // mdio_data = 0x0012   // [data_rate = 5Gbps]
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    udelay(100);

    //  --> *** Override SAPIS_TX_ENA ****
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8000 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);

    mdio_regAddr = 0x1D     ;
    mdio_data    = 0x1e00   ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    udelay(100);


    //  --> Transmit PRBS data  TX0
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8060 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);

    mdio_regAddr = 0x10     ;
    mdio_data    = 0x0002   ;
    // mdio_data     = 0x0042   ;  // Invert the analog TX transmit output data
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    udelay(100);

    //  --> Enable PRBS Monitors for Ch0
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8010 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);

    mdio_regAddr = 0x18     ;
    switch (mode)
    {
    case 0:
        mdio_data    = 0x0008   ;     // PRBS 7
        break;
    case 1:
        mdio_data     = 0x0009   ;      // PRBS 15
        break;
    case 2:
        mdio_data     = 0x000A   ;      // PRBS 23
        break;
    case 3:
        mdio_data     = 0x000B   ;      // PRBS 31
        break;
    default:
        printk ("wan_type not supported");
        goto reconf_serdess;
        break;
    }

    // [bit 03] = Enable Channel 0 PRBS monitor = '1'
    // [bit 02] = Channel 0 Invert PRBS polynomial = '0'
    // [bit 01:00] = Channel 0 PRBS order\type
    // mdio_data     = 0x0009   ;      // PRBS 15
    // mdio_data     = 0x000A   ;      // PRBS 23
    // mdio_data     = 0x000B   ;      // PRBS 31
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    udelay(100);


    //  -->  Channel 0 : Check RX PRBS monitor :
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x80B0 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);

    mdio_regAddr = 0x19     ;
    mdio_data    = 0x0B00   ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    mdio_regAddr = 0x11     ;
    mdio_data    = 0x5C87   ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(100);

    mdio_regAddr    = 0x10   ;
    mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
    //  if  mdio_data == 0x8000 <=> PRBS locked with no errors !!!!]
    printk ("mdio_data %x",mdio_data);
    mdio_data = mdio_data & 0x0000ffff;
    if (mdio_data == 0x8000)
        *valid = 1;

    if (wan_type == SERDES_WAN_TYPE_GPON)
    {
        // LASER_MODE            = bit[03:02] = 3 => constant output 1
        FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 2, 2 ,3 ); //  bits[3, 2] = 1
    }
    else if(wan_type == SERDES_WAN_TYPE_EPON_1G || wan_type == SERDES_WAN_TYPE_EPON_2G)
    {
        // LASER_MODE            = bit[03:02] = 3 => constant output 1
        // WAN_INTERFACE_SELECT = bit[01:00] = 2'b1 = EPON
        FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 0, 4 ,13 ); //  bits[3, 2] = 1
    }

    printk ("Done PRBS config");
    return;

reconf_serdess:
    switch (wan_type)
    {
    case SERDES_WAN_TYPE_GPON:
        configWanGPONSerdes();
        break;
    case SERDES_WAN_TYPE_EPON_1G:
    case SERDES_WAN_TYPE_EPON_2G:
        // configWanEPONSerdes();  // caution - repeating the EPON init sequence will cause resets to varios WAN block section ! 
		
	// '  --> reset the  "Register controlled RX data rate" setting
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8000 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);
    mdio_regAddr = 0x1E     ;
    mdio_data    = 0x0		;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(100);

    //  --> reset the  " Register controlled TX data rate" setting
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8010 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);
    mdio_regAddr = 0x13     ;
    mdio_data    = 0x0000   ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(100);

    //  --> reset the  " Override SAPIS_TX_ENA " setting
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8000 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);
    mdio_regAddr = 0x1D     ;
    mdio_data    = 0x0	    ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(100);


    //  --> reset the  " Transmit PRBS data  TX0 " setting
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x8060 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);
    mdio_regAddr = 0x10     ;
    mdio_data    = 0x0	    ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(100);
	
    // Select WAN interface = EPON 
	// LASER_MODE  			= bit[03:02] = 2'b2 = EPON MAC control
	// WAN_INTERFACE_SELECT = bit[01:00] = 2'b1 = EPON
	FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 0, 4 , 9 ); //  bit[3: 2] = 10 , bit[1:0]=01

    // configure SATA_PHY  - setting EPON rates &  TX transmit enable
    if (SERDES_WAN_TYPE_EPON_1G == wan_type)
    {
        reg32_data = 0x00070004;  
    }
    else if (SERDES_WAN_TYPE_EPON_2G == wan_type)
    {
        reg32_data = 0x00071004;   // Turbo-EPON = 00071004   //  Orig. Normal EPON = reg32_data = 0x00070004
    }
    WRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_SATA_CFG, reg32_data) ;
	udelay(1000);
	
        break;
    default:
        printk ("wan_type not supported");
        break;
    }

    //enable the MEM_REB and Laser ON
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 5, 1, 0 );
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_WAN_CFG, 6, 1, 1 );

    if (transceiver == BCM_I2C_PON_OPTICS_TYPE_PMD)
    {   
        if (pmd_prbs_callback)
            pmd_prbs_callback((uint16_t)enable, 1);
    }  
    return;

}
EXPORT_SYMBOL(wan_prbs_gen);


void wan_prbs_status(bdmf_boolean *valid, uint32_t *errors)
{
    // Variables :
    uint16_t    mdio_data               ;   // MDIO (clause 22) 16bit rd\wr data var
    uint8_t     mdio_phyAddr = 0x01     ;   // MDIO 8 bit phyAddr. var, in WAN always equal to 1
    uint8_t     mdio_regAddr            ;   // MDIO 8 bit addr. var

    //  -->  Channel 0 : Check RX PRBS monitor :
    mdio_regAddr    = 0x1F   ;
    mdio_data       = 0x80B0 ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(1);

    mdio_regAddr = 0x19     ;
    mdio_data    = 0x0B00   ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);

    mdio_regAddr = 0x11     ;
    mdio_data    = 0x5C87   ;
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);
    udelay(100);

    mdio_regAddr    = 0x10   ;
    mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
    //  if  mdio_data == 0x8000 <=> PRBS locked with no errors !!!!]

    if (mdio_data == 0x8000)
        *valid = 1;
    else
        *valid = 0;
		
	*errors = mdio_data & 0x3fff;

    return;
}
EXPORT_SYMBOL(wan_prbs_status);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_configure_serdes_amplitude_reg                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   GBE/SERDES Driver - Set SerDes amplitude                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the chosen SerDes amplitude configuration.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   ampt_prm - 0-31 (0 = max , 31 = roughly 15% amplitude)                   */
/*                                                                            */
/******************************************************************************/
serdes_error_t wan_serdes_amplitude_set(uint16_t ampl_prm)
{
    uint16_t 	mdio_data  				;   /* MDIO (clause 22) 16bit rd\wr data var */
    uint8_t		mdio_phyAddr = 0x01		;   /* MDIO 8 bit phyAddr. var, in WAN always equal to 1 */
    uint8_t  	mdio_regAddr 			;   /* MDIO 8 bit addr. var */
    uint16_t    ampl_val =  MAX_AMPL_PRM - ampl_prm;  /* convert ampl_param to ampl value: 31 - max amplitude, 0 - min amplitude */

    if (ampl_val > MAX_AMPL_PRM)
        return DRV_SERDES_AMPLITUDE_IS_OUT_OF_RANGE;
	
    mdio_regAddr = 0x1F;             /*  regad[4:0] = 12'h1f */
    mdio_data    = 0x8060;           /*  SATA3_TX0: TX0 Register Bank = 0x0060  */
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);                

    mdio_regAddr         = 0x13   ;      /* SATA3_TX_txAcontrol3 = 0x083 ==> {1'b1,SATA3_TX_txAcontrol3} = {1'b1, 4'd3} = 5'h13 ; */
    //mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);
    mdio_data = 0x0800 + (ampl_val << 6) + 5;
    printk("mdio_write_c22_register mdio_data=%x\n\r", mdio_data ) ;
    mdio_write_c22_register(MDIO_SATA, mdio_phyAddr, mdio_regAddr, mdio_data);        

    return DRV_SERDES_NO_ERROR;
}
EXPORT_SYMBOL(wan_serdes_amplitude_set);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_get_serdes_amplitude                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   GBE/SERDES Driver - Get serdes amplitude configuration                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the chosen serdes amplitude configuration.            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   ampt_prm - 0-31 (0 = max , 31 = roughly 15% amplitude)                   */
/*                                                                            */
/*   BL_ERROR_DTE - Return code                                               */
/*                                                                            */
/******************************************************************************/
serdes_error_t wan_serdes_amplitude_get(uint16_t *ampl_prm)
{
	uint16_t 	mdio_data  				;   /* MDIO (clause 22) 16bit rd\wr data var */
	uint8_t		mdio_phyAddr = 0x01		;   /* MDIO 8 bit phyAddr. var, in WAN always equal to 1 */
	uint8_t  	mdio_regAddr 			;   /* MDIO 8 bit addr. var */
	uint16_t    ampl_val;

    mdio_regAddr = 0x1F;             /*  regad[4:0] = 12'h1f */
    mdio_data    = 0x8060;           /*  SATA3_TX0: TX0 Register Bank = 0x0060  */
    mdio_write_c22_register(MDIO_SATA ,mdio_phyAddr, mdio_regAddr, mdio_data);                

    mdio_regAddr         = 0x13   ;      /* SATA3_TX_txAcontrol3 = 0x083 ==> {1'b1,SATA3_TX_txAcontrol3} = {1'b1, 4'd3} = 5'h13 ; */
    mdio_read_c22_register(MDIO_SATA , mdio_phyAddr, mdio_regAddr, &mdio_data);

    printk("mdio_write_c22_register mdio_data=%x\n\r", mdio_data ) ;
	ampl_val  = (mdio_data & 0x07C0) >> 6;

	if (ampl_val > MAX_AMPL_PRM)
       return DRV_SERDES_AMPLITUDE_IS_OUT_OF_RANGE;

	*ampl_prm  = MAX_AMPL_PRM - ampl_val;
    return DRV_SERDES_NO_ERROR;
}
EXPORT_SYMBOL(wan_serdes_amplitude_get);

void pon_wan_top_reset_gearbox_tx()
{
    //TODO
    return;
}
EXPORT_SYMBOL(pon_wan_top_reset_gearbox_tx);

/* Should be called before optical module TX power is enabled / MAC is set to transmit */
void wan_top_transmitter_control(enum transmitter_control mode)
{
}
EXPORT_SYMBOL(wan_top_transmitter_control);

