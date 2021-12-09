/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
*/
#include <linux/i2c.h>
#include <linux/slab.h>  /* kzalloc() */
#include <linux/types.h>
#include <linux/bcm_log.h>
#include <boardparms.h>
#include <board.h>
#include <bcmsfp_i2c.h>
#include "opticaldet.h"
#include "trx_descr_usr.h"
#include "wan_types.h"

TRX_DESCRIPTOR trx_desc[MAX_I2C_BUS_INCLUDE_MUX];
static int sand[MAX_I2C_BUS_INCLUDE_MUX+2] = { [0] = 0xA5A5A5A5, [MAX_I2C_BUS_INCLUDE_MUX+1] = 0x5A5A5A5A };
static int * const trx_desc_init = &sand[1];
static int printed = 0;
TRX_DESCRIPTOR* trx_curr_p = NULL;

static DECLARE_RWSEM(trx_desc_sem);

static int get_trx_desc_init_state(int bus);
static void set_trx_desc_init_state(int bus, int state);

void print_part(uint8_t *data, char *ptype, char *prefix, int from, int len){
    int i;

    printk("%s: ", prefix);
    for (i = from; i < from + len; i++)
        printk(ptype, data[i]);

    printk("\n");
}

void trx_fixup(int bus)
{
    unsigned short gpio;
    int init = 0;

    if( bus < 0 )
        bus = 0;
            
    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init)
    {
        if (BpGetOpticalModuleTxPwrDownGpio(&gpio) == BP_SUCCESS && gpio != BP_GPIO_NONE)
        {
            /* Set Tx power down pin */
            if ( trx_curr_p->tx_pwr_down_cfg_req)
            {
                kerSysSetGpioDir(gpio);
                printk(KERN_ALERT "Opticaldet: Setting Tx power down pin\n");
                kerSysSetGpioState(gpio, (trx_curr_p->tx_pwr_down_polarity==TRX_ACTIVE_LOW) ? kGpioInactive : kGpioActive);
            }
        }
    }
}

void trx_activate (int bus)
{
    int init = 0;

    if( bus < 0 )
        bus = 0;
            
    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init)
    {
        if (trx_curr_p->activation_func)
        {
            (trx_curr_p->activation_func)(bus);
        }
    }
}


static char *trx_module_ff_table[11] = {
   "UNKNOWN",   /* 0x0 */
   "GBIC",      /* 0x1 */
   "SFF",       /* 0x2 - SFF */
   "SFP/SFP+",  /* 0x3 - SFP or SFP+ */
   "XBI",       /* 0x4 */
   "XENPAK",    /* 0x5 */
   "XFP",       /* 0x6 */
   "XFF",       /* 0x7 */
   "XFP-E",     /* 0x8 */
   "XPAK",      /* 0x9 */
   "X2"         /* 0xA */
} ;

static char *trx_module_type_table[] = {
   "xPON",      /* 0x0 */
   "ETHERNET",  /* 0x1 */
   "UNKNOWN",   /* 0x2 */
};

static char *trx_module_detect_table[] = {
   "Unknown",      /* 0x0 */
   "Known",        /* 0x1 */
   "Detected",     /* 0x2 */
};

static TRX_DESCRIPTOR default_pluggable_trx = {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_UNKNOWN,
    .vendor_name           = "Default",
    .vendor_pn             = "Default",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false
} ;


static TRX_DESCRIPTOR default_on_board_trx = {
    .form_factor           = TRX_SFF,              
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Default",
    .vendor_pn             = "Default",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false
} ;

static TRX_DESCRIPTOR default_pmd_trx = {
    .form_factor           = TRX_PMD,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Broadcom",
    .vendor_pn             = "689xx",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON | SUPPORTED_WAN_TYPES_BIT_EPON_1_1 | SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1 \
        | SUPPORTED_WAN_TYPES_BIT_EPON_10_1 | SUPPORTED_WAN_TYPES_BIT_XGPON
} ;

static int check_pluggable_module(int bus, TRX_TYPE type)
{
    int  match = 1, is_eth_bus;
#ifdef CONFIG_BP_PHYS_INTF
    unsigned short intf_type;
    int intf_idx;
#endif

    /* if trx is unknown, basically anything not in the table, 
       don't need to check, big warnning already printed out early */
    if( type == TRX_TYPE_UNKNOWN ) 
        return match;

#ifdef CONFIG_BP_PHYS_INTF
    bcm_i2c_sfp_get_intf(bus, &intf_type, &intf_idx);

    /* xPON interface be used as gpon, epon or active ethernet. No check here */ 
    if( intf_type == BP_INTF_TYPE_xPON )
        return match;

    if( intf_type == BP_INTF_TYPE_SGMII && type == TRX_TYPE_XPON ) 
       match = 0;

    if( intf_type == BP_INTF_TYPE_xPON )
       is_eth_bus = 0;
    else
       is_eth_bus = 1;
#else
#if !defined(CONFIG_BCM_PON)
    if( type == TRX_TYPE_XPON )
       match = 0;
    is_eth_bus = 1;
#endif
#endif
    if( match == 0 ) {
        printk("************************************************************************\n");
        printk("Mismatch pluggable optical module on i2c bus %d!\n", bus);
        printk("Optical module type %s, board design type on this bus is %s\n", trx_module_type_table[type], trx_module_type_table[is_eth_bus]);
        printk("************************************************************************\n");
    }

    return match;
}

#ifdef DETECT_TRX_TYPE
static TRX_TYPE detect_trx_type(uint8_t trx_ff, uint8_t* eeprom)
{
    uint8_t cc_10g_eth = eeprom[TRX_EEPROM_OFFSET_CODE];
    uint8_t cc_eth = eeprom[TRX_EEPROM_OFFSET_CODE+3];
    uint8_t cc_10g_mask, cc_eth_mask;

    //printk("compliance code 0x%02x 0x%02x\n", cc_10g_eth, cc_eth);

    if( trx_ff == TRX_XFP ) {
        cc_10g_mask = TRX_XFP_EEPROM_CC_10GETH_MASK;
        cc_eth_mask = TRX_XFP_EEPROM_CC_ETH_MASK;
    } else {
        cc_10g_mask = TRX_EEPROM_CC_10GETH_MASK;
        cc_eth_mask = TRX_EEPROM_CC_ETH_MASK;
    }

    if( (cc_10g_eth&cc_10g_mask) || (cc_eth&cc_eth_mask) )
        return TRX_TYPE_ETHERNET;
    else
        return TRX_TYPE_XPON;
}
#endif

static int get_trx_desc_init_state(int bus) 
{
    int init;
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return 0;

    down_read(&trx_desc_sem);
    init = trx_desc_init[bus];
    up_read(&trx_desc_sem);

    return init;
}

static void set_trx_desc_init_state(int bus, int state) 
{
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
         return;

    down_write(&trx_desc_sem);
    trx_desc_init[bus] = state;
    up_write(&trx_desc_sem);

    return;
}

static void trx_print_descriptor(TRX_DESCRIPTOR* trx_p, int detect)
{  
    char* str_ff; 

    if( trx_p->form_factor == TRX_PMD )
        str_ff = "PMD";
    else
        str_ff = trx_module_ff_table[trx_p->form_factor];

    printk("Opticaldet %s Transceiver\n", trx_module_detect_table[detect]);
    printk("Module Form Factor: %s\n", str_ff);
    printk("Module Type       : %s\n", trx_module_type_table[trx_p->type]);
    printk("Vendor Name       : %s\n", trx_p->vendor_name);
    printk("Part Number       : %s\n", trx_p->vendor_pn);
    printk("Part REV          : %s\n", trx_p->vendor_rev);

    /* print out big warning if it is unknown */
    if( detect == 0 ) {
        printk(KERN_ALERT "************************************************************************\n");
        printk(KERN_ALERT "* Opticaldet: Unknown optical module - using default configuration     *\n");
        printk(KERN_ALERT "* Please make sure the optical module is correct for your connection   *\n");
        printk(KERN_ALERT "************************************************************************\n");
    }

    return ;
}

#ifndef CONFIG_BP_PHYS_INTF
static int get_bus_from_descriptor(TRX_TYPE type, TRX_FORM_FACTOR *form_factor)
{
    unsigned short bus;
    int def_bus;
 
    def_bus = bcmsfp_get_def_bus();
    if(def_bus >= 0){
        return def_bus;
    }

    if(type == TRX_TYPE_XPON){
        if(BpGetI2cDefXponBus(&bus) == 0){
            if((trx_desc[bus].type == type) && (get_trx_desc_init_state(bus))){
                if(form_factor){
                     *form_factor = trx_desc[bus].form_factor;
                }
                return (int)bus;
            }
        }
    }

    //try to find the 1st match
    for(bus = 0; bus < MAX_I2C_BUS_INCLUDE_MUX; bus++){ 
        if((trx_desc[bus].type == type) && (get_trx_desc_init_state(bus))){
            if(form_factor){
                *form_factor = trx_desc[bus].form_factor;
            }
            return (int)bus;
        }
    }   

    return (-1);
}
#endif

static void (*rogue_onu_trx_insert_cb)(struct work_struct *dummy) = NULL;

void opticaldet_rogue_onu_trx_insert_set_cb(void (*rogue_onu_trx_insert_ptr)(struct work_struct *dummy))
{
    rogue_onu_trx_insert_cb = rogue_onu_trx_insert_ptr;
}
EXPORT_SYMBOL(opticaldet_rogue_onu_trx_insert_set_cb);

static void opticaldet_rogue_onu_trx_insert(struct work_struct *dummy)
{
    if (rogue_onu_trx_insert_cb == NULL)
    {
        printk(KERN_ALERT "%s.%s: rogue_onu_trx_insert_cb == NULL\n", __FILE__, __FUNCTION__);
    }
    else
    {
        (*rogue_onu_trx_insert_cb)(dummy);
    }
}
DECLARE_WORK(opticaldet_rogue_onu_trx_insert_work, opticaldet_rogue_onu_trx_insert);


static void trx_find_descriptor(int bus, uint8_t trx_ff, uint8_t *eeprom)
{
   int trx_gen_size ;
   int trx_usr_size ;
   int i, known = 0;
   char* vendor = eeprom + TRX_EEPROM_OFFSET_NAME;
   char* part_number = eeprom + TRX_EEPROM_OFFSET_PN;

   trx_curr_p = &trx_desc[bus];
   memset(trx_curr_p, 0x0, sizeof(TRX_DESCRIPTOR));

   trx_gen_size = sizeof (trx_lst) / sizeof (TRX_DESCRIPTOR) ;
   trx_usr_size = sizeof (trx_usr) / sizeof (TRX_DESCRIPTOR) ;

   for (i=0; i<trx_usr_size; i++) {
     if ((!strncmp(vendor, trx_usr[i].vendor_name, strlen(trx_usr[i].vendor_name))) &&
         (!strncmp(part_number, trx_usr[i].vendor_pn, strlen(trx_usr[i].vendor_pn)))) 
     {
          memcpy(trx_curr_p, &trx_usr[i], sizeof(TRX_DESCRIPTOR));
          known = 1;
          break;
     }
   }

   if( known == 0 ) {
      for (i=0; i<trx_gen_size; i++) {
         if (!strncmp(vendor, trx_lst[i].vendor_name, strlen(trx_lst[i].vendor_name)) &&
             !strncmp(part_number, trx_lst[i].vendor_pn, strlen(trx_lst[i].vendor_pn))) 
         {
             memcpy(trx_curr_p, &trx_lst[i], sizeof(TRX_DESCRIPTOR));
             known = 1;
             break;
         }
      }
   }

#ifdef DETECT_TRX_TYPE
   /* if it is pluggable module, trying to detect if it is ethernet type */
   if( known == 0 && trx_ff != TRX_SFF ) {
       TRX_TYPE type = TRX_TYPE_XPON;
       type = detect_trx_type(trx_ff, eeprom);
       if( type == TRX_TYPE_ETHERNET ) {
           trx_curr_p->form_factor = trx_ff;
           trx_curr_p->type = type;
           known = 2;
       }
   }
#endif

   /*
    *    TRX not found
    */
   if( known == 0 ) {
       if( trx_ff == TRX_SFF )
           memcpy(trx_curr_p, &default_on_board_trx, sizeof(TRX_DESCRIPTOR));
       else
           memcpy(trx_curr_p, &default_pluggable_trx, sizeof(TRX_DESCRIPTOR));
   }

   /* copy the vendor info from eeprom */
   strncpy(trx_curr_p->vendor_name, eeprom+TRX_EEPROM_OFFSET_NAME, TRX_EEPROM_LEN_NAME);
   strncpy(trx_curr_p->vendor_pn, eeprom+TRX_EEPROM_OFFSET_PN, TRX_EEPROM_LEN_PN);
   strncpy(trx_curr_p->vendor_rev, eeprom+TRX_EEPROM_OFFSET_REV, trx_ff == TRX_XFP ? TRX_XFP_EEPROM_LEN_REV : TRX_EEPROM_LEN_REV);
   memcpy(trx_curr_p->vendor_sn, eeprom+TRX_EEPROM_OFFSET_SN, TRX_EEPROM_LEN_SN);
   trx_curr_p->tx_wavlen = ntohs(*((uint16_t*)(eeprom+TRX_EEPROM_OFFSET_TX_WAVLEN)));

   trx_print_descriptor(trx_curr_p,  known);

   /* check if the pluggable module type match the board design */
   if( trx_ff != TRX_SFF ) {
       check_pluggable_module(bus, trx_curr_p->type);
   }

   set_trx_desc_init_state(bus, 1);

   if (known == 1) {
       schedule_work(&opticaldet_rogue_onu_trx_insert_work);
       /* will fail upon opticaldet init b/c cb not ready, but will be done in gpon_post_init and upon insert */
   }
}

static int i2c_read_sff_sfp_data(int bus, uint8_t trx_ff)
{
   uint8_t i, data[85] ;
   uint8_t i2c_byte ;
   int ret = 0;

   for (i = 0; i < sizeof(data); i++)
   {
        ret = bcmsfp_read_byte(bus, 0, i, &i2c_byte);
        if (ret != 0)
        {
            printk(KERN_ALERT "Opticaldet:>>>> Error reading from i2c at offset %d\n", i) ;
            return ret;
        }

        data[i] = i2c_byte;
    }

    trx_find_descriptor(bus, trx_ff, data) ;
    return ret;
}


static int i2c_read_xfp_data(int bus, uint8_t trx_ff)
{

   uint8_t i, data[70] ;
   uint8_t i2c_byte ;
   int rc = 0;
   
   rc = bcmsfp_write_byte (bus, 0, TRX_XFP_EEPROM_PAGE_SELECT, 1);
   if (rc != 0)
   {
       printk(">>>> Error writing to i2c at offset %d\n", TRX_XFP_EEPROM_PAGE_SELECT) ;
       return rc;
   }


   for (i = 0; i < 60; i++)
   {
        rc = bcmsfp_read_byte(bus, 0, i + TRX_XFP_EEPROM_PAGE_1, &i2c_byte);
        if (rc != 0)
        {
            printk(KERN_ALERT "Opticaldet:>>>> Error reading from i2c at offset %d\n", i + TRX_XFP_EEPROM_PAGE_1) ;
            return rc;
        }

        data[i] = i2c_byte;
   }

   trx_find_descriptor(bus, trx_ff, data) ;

   return rc;
}


int i2c_read_trx_data(int bus)
{
    int rc = 0;
    uint8_t trx_ff;
    uint16_t OpticsTypeFlag = 0;
#ifdef CONFIG_BP_PHYS_INTF
    unsigned short intf_type;
    int intf_idx;
#endif

    printk("Opticaldet: optical module detected on i2c bus %d:\n", bus);
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return -1;

#ifdef CONFIG_BP_PHYS_INTF
    if( bcm_i2c_sfp_get_intf(bus, &intf_type, &intf_idx) != 0 ) {
        return -1;
    }
#endif

    trx_curr_p = &trx_desc[bus];

    bcm_i2c_pon_optics_type_get(&OpticsTypeFlag);
    if (OpticsTypeFlag == BCM_I2C_PON_OPTICS_TYPE_PMD
#ifdef CONFIG_BP_PHYS_INTF
        && intf_type == BP_INTF_TYPE_xPON
#endif
    )
    {
        printk("Opticaldet: PMD found\n");
        memcpy(trx_curr_p, &default_pmd_trx, sizeof(TRX_DESCRIPTOR));
        set_trx_desc_init_state(bus, 1);
    }
    else
    {
        rc = bcmsfp_read_byte(bus, 0, 0, &trx_ff);
        if ( rc != 0)
        {
            printk(KERN_ALERT "Opticaldet: No TRX found on bus %d rc %d type %d\n",
            bus, rc, trx_ff  );
        }
        else
        {
            switch (trx_ff)
            {
                case TRX_SFF:
                    rc = i2c_read_sff_sfp_data(bus, trx_ff);
                break;

                case TRX_SFP:
                    rc = i2c_read_sff_sfp_data(bus, trx_ff);
                break;

                case TRX_XFP:
                    rc = i2c_read_xfp_data(bus, trx_ff);
                break;

                default:
                    printk(KERN_ALERT "Opticaldet: Illegal TRX type %d\n", trx_ff);
                    if( trx_ff == TRX_SFF )
                        memcpy(trx_curr_p, &default_on_board_trx, sizeof(TRX_DESCRIPTOR));
                    else
                        memcpy(trx_curr_p, &default_pluggable_trx, sizeof(TRX_DESCRIPTOR));
                    set_trx_desc_init_state(bus, 1);
            }
        }
    }

    return rc;
}

void i2c_clear_trx_data(int bus)
{
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return;

    trx_curr_p = &trx_desc[bus];
    memset(trx_curr_p, 0x0, sizeof(TRX_DESCRIPTOR));
    set_trx_desc_init_state(bus, 0);
}

int trx_get_lbe_polarity(int bus, TRX_SIG_ACTIVE_POLARITY *lbe_polarity_p)
{
    int init = 0;

    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return OPTICALDET_INVPARM;


    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init) {
        *lbe_polarity_p = trx_curr_p->lbe_polarity ;
        return 0;
    } 
    else {
        return OPTICALDET_NOSFP;
    }
}
EXPORT_SYMBOL(trx_get_lbe_polarity);

int trx_get_tx_sd_polarity(int bus, TRX_SIG_ACTIVE_POLARITY *tx_sd_polarity_p)
{
    int init = 0;

    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return OPTICALDET_INVPARM;

    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init) {
        *tx_sd_polarity_p = trx_curr_p->tx_sd_polarity ;
        return OPTICALDET_SUCCESS;
    } 
    else {
        return OPTICALDET_NOSFP;
    }
}
EXPORT_SYMBOL(trx_get_tx_sd_polarity);

int trx_get_vendor_name_part_num(int bus, char *vendor_name_p, int vendor_name_len,
                                 char *part_num_p, int part_num_len)
{
    int init = 0;
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return OPTICALDET_INVPARM;

    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init) {
        strncpy(vendor_name_p, trx_curr_p->vendor_name, vendor_name_len);
        strncpy(part_num_p, trx_curr_p->vendor_pn, part_num_len);
        return OPTICALDET_SUCCESS;
    } 
    else {
       return OPTICALDET_NOSFP;
    }
}
EXPORT_SYMBOL(trx_get_vendor_name_part_num);


int trx_get_tx_sd_supported(int bus, TRX_SIG_PRESENCE *signal_supported_p)
{
    int init = 0;
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return OPTICALDET_INVPARM;

    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init) {
        *signal_supported_p = trx_curr_p->tx_sd_supported ;
        return OPTICALDET_SUCCESS;
    } 
    else {
        return OPTICALDET_NOSFP;
    }
}
EXPORT_SYMBOL(trx_get_tx_sd_supported);

int trx_get_type(int bus, TRX_TYPE *trx_type)
{
    int init = 0;
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return OPTICALDET_INVPARM;

    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init) {
        *trx_type = trx_curr_p->type;
        return OPTICALDET_SUCCESS;
    } 
    else {
        if (((sand[0] != 0xA5A5A5A5) || (sand[MAX_I2C_BUS_INCLUDE_MUX+1] != 0x5A5A5A5A)) && (!printed++))
        {
            printk(KERN_ALERT "memory overflow near trx_desc_init=%p\n", trx_desc_init);
        }
        return OPTICALDET_NOSFP;
    }
}
EXPORT_SYMBOL(trx_get_type);


int trx_get_supported_wan_type_bm(int bus, SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm)
{
    int init = 0;
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX)
        return OPTICALDET_INVPARM;

    init = get_trx_desc_init_state(bus);
    
    if (init) {
        BCM_LOG_DEBUG(BCM_LOG_ID_OPTICALDET, "I2C BUS %d\n", bus);
        trx_curr_p = &trx_desc[bus];
        *wan_type_bm = trx_curr_p->wan_types_bitmap;
        BCM_LOG_DEBUG(BCM_LOG_ID_OPTICALDET, "WAN TYPE BM 0x%x\n", *wan_type_bm);

        trx_print_descriptor(trx_curr_p, 1);
        return OPTICALDET_SUCCESS;
    } 
        
    *wan_type_bm = SUPPORTED_WAN_TYPES_AUTO_SENSE_UNAVAILABLE;
    return OPTICALDET_NOSFP;
}
EXPORT_SYMBOL(trx_get_supported_wan_type_bm);


#ifdef CONFIG_BP_PHYS_INTF
int opticaldet_get_i2c_bus_num(unsigned short intf_type, int intf_idx, int* bus)
{
    TRX_TYPE type = TRX_TYPE_XPON;

    /* get the bus assignment from the board design */
    if ( bcm_i2c_sfp_get_bus_num(intf_type, intf_idx, bus) != 0 ) {
        printk("**** No i2c bus found for interface type %d idx %d. Wrong board parameter or board design! ****\n", intf_type, intf_idx);
        return OPTICALDET_NOBUS;
    }

    /* check the detected sfp type against the requested type */
    if( trx_get_type(*bus, &type) != OPTICALDET_SUCCESS ) {
        printk("**** No optical module plugged in for interface type %d idx %d on bus %d! ****\n", intf_type, intf_idx, *bus);
        return OPTICALDET_NOSFP;
    }

    if( type == TRX_TYPE_UNKNOWN )
        return OPTICALDET_SUCCESS;

    if( intf_type == BP_INTF_TYPE_SGMII && type == TRX_TYPE_XPON ) {
        printk("**** Invalid optical module type %s for SGMII intf idx %d on bus %d! ****\n",trx_module_type_table[type], intf_idx, *bus);
        return OPTICALDET_INVSFP;
    }

    /* no need to check xpon interface because it can be either GPON, EPON or AE and they have different SFP */

    return OPTICALDET_SUCCESS;
}
EXPORT_SYMBOL(opticaldet_get_i2c_bus_num);
#endif

int opticaldet_get_xpon_i2c_bus_num(int* bus)
{
#ifdef CONFIG_BP_PHYS_INTF
    /* assume only one xpon interface per board */
    return opticaldet_get_i2c_bus_num(BP_INTF_TYPE_xPON, 0, bus);
#else
    TRX_TYPE type = TRX_TYPE_XPON;

    *bus = get_bus_from_descriptor(TRX_TYPE_XPON, NULL);
    if(*bus == (-1))/* set 0 as default when unknown */
        *bus = 0;
    /* check the detected sfp type against the requested type */
    if( trx_get_type(*bus, &type) != OPTICALDET_SUCCESS ) {
        /*
        printk("**** No optical module plugged in for xpon interface on bus %d! ****\n", *bus);
        */
        return OPTICALDET_NOSFP;
    }

    /* no need to check xpon interface because it can be either GPON, EPON or AE and they have different SFP */

    return OPTICALDET_SUCCESS;
#endif
}
EXPORT_SYMBOL(opticaldet_get_xpon_i2c_bus_num);

int opticaldet_get_sgmii_i2c_bus_num(int* bus)
{
#ifdef CONFIG_BP_PHYS_INTF
    /* assume only one sgmii interface per board */
    return opticaldet_get_i2c_bus_num(BP_INTF_TYPE_SGMII, 0, bus);
#else
    TRX_TYPE type = TRX_TYPE_XPON;

    *bus = get_bus_from_descriptor(TRX_TYPE_ETHERNET, NULL);
    if(*bus == (-1)){
        *bus = get_bus_from_descriptor(TRX_TYPE_UNKNOWN, NULL);
        if(*bus == (-1))/* set 0 as default when unknown */
            *bus = 0;       
    }
    /* check the detected sfp type against the requested type */
    if( trx_get_type(*bus, &type) != OPTICALDET_SUCCESS ) {
        printk("**** No optical module plugged in for sgmii interface on bus %d! ****\n", *bus);
        return OPTICALDET_NOSFP;
    }

    if( type == TRX_TYPE_ETHERNET || type == TRX_TYPE_UNKNOWN )
        return OPTICALDET_SUCCESS;
    else {
        printk("**** Invalid optical module type %s for sgmii interface! ****\n", trx_module_type_table[type]);
        return OPTICALDET_INVSFP;
    }
#endif

}
EXPORT_SYMBOL(opticaldet_get_sgmii_i2c_bus_num);

int trx_get_full_info(int bus, TRX_INFOMATION *trx_info)
{
    int init = 0;
    if( bus < 0 || bus >= MAX_I2C_BUS_INCLUDE_MUX )
        return OPTICALDET_INVPARM;

    init = get_trx_desc_init_state(bus);
    trx_curr_p = &trx_desc[bus];
    if (init) {
        trx_info->form_factor = trx_curr_p->form_factor;
        trx_info->type = trx_curr_p->type;
        strncpy(trx_info->vendor_name, trx_curr_p->vendor_name, TRX_EEPROM_LEN_NAME);
        strncpy(trx_info->vendor_pn, trx_curr_p->vendor_pn, TRX_EEPROM_LEN_PN);
        memcpy(trx_info->vendor_sn, trx_curr_p->vendor_sn, TRX_EEPROM_LEN_SN);
        trx_info->wan_types_bitmap = trx_curr_p->wan_types_bitmap;
        trx_info->power_budget = trx_curr_p->power_budget;
        trx_info->tx_wavlen = trx_curr_p->tx_wavlen;
        trx_info->rx_wavlen = trx_curr_p->rx_wavlen;
        return OPTICALDET_SUCCESS;
    }
    else {
       return OPTICALDET_NOSFP;
    }
}

int opticaldet_is_xpon_sfp_present(void)
    {
    int bus = -1;
    int rc;

    rc = opticaldet_get_xpon_i2c_bus_num(&bus);
    if (rc != OPTICALDET_SUCCESS)
        {
        printk("*** Can't get optical device i2c bus number! ***\n");
        return FALSE;
        }
    else
        {
            return get_trx_desc_init_state(bus);
        }
    }
EXPORT_SYMBOL(opticaldet_is_xpon_sfp_present);
