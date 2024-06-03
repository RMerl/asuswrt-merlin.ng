/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/


#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/reboot.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/version.h>

#include <bcmtypes.h>
#include <board.h>
#include <shared_utils.h>

#include "board_wl.h"
#include "board_image.h"
#if defined (WIRELESS)
#include <linux/kmod.h>
#endif
#include <button.h>

extern wait_queue_head_t g_board_wait_queue;

#if defined(WIRELESS)
static atomic_t sesBtn_active;
static atomic_t sesBtn_forced;
static void sesBtn_releaseAction(unsigned long time, void* param);

#define LONG_PUSH_BUTTON_PERIOD (3000)  //three seconds
#define SHORT_PUSH_BUTTON_PERIOD (1)


/* This spinlock is used to avoid race conditions caused by the
 * non-atomic test-and-set of sesBtn_active in sesBtn_read */
static DEFINE_SPINLOCK(sesBtn_newapi_spinlock);
#endif

void __init ses_board_init()
{
#if defined(WIRELESS) && defined(CONFIG_BCM_BUTTON)
    int ret;
    ret = register_button_action("ses_button", "ses_short_period", sesBtn_releaseAction);
    if (ret < 0 && ret != -ENODEV)
    {
        printk("ses_board_init: cannot register ses_short_period action for ses_button\n");
        return;
    }
    ret = register_button_action("ses_button", "ses_long_period", sesBtn_releaseAction);
    if (ret < 0 && ret != -ENODEV)
    {
        printk("ses_board_init: cannot to register ses_long_period action for ses_button\n");
        return;
    }
#endif
}

void __exit ses_board_deinit()
{
#if defined(WIRELESS)  
    atomic_set(&sesBtn_active, 0);
    atomic_set(&sesBtn_forced, 0);
#endif    
}

#if defined(WIRELESS)
static void sesBtn_releaseAction(unsigned long time, void* param) {
    unsigned long flags;
    spin_lock_irqsave(&sesBtn_newapi_spinlock, flags);
    if(time>=3000) {
        printk( "(kernel):%s:%d:	*** long released ***\n",__FUNCTION__,__LINE__);
        atomic_set(&sesBtn_active, SES_BTN_STA);
    } else {
        printk( "(kernel):%s:%d:	*** short released ***\n",__FUNCTION__,__LINE__);
        atomic_set(&sesBtn_active, SES_BTN_AP);
    }
    spin_unlock_irqrestore(&sesBtn_newapi_spinlock, flags);
    wake_up_interruptible(&g_board_wait_queue);
}
#endif

unsigned char kerSysGetWlanFeature(void)
{
   unsigned char wlfeature=0;
   char features[16]={0};
   long feature=0;
   if(envram_get_locked(NVRAM_WLFEATURE,features,16) && !kstrtol(features, 16, &feature))
      wlfeature = (unsigned char)feature;
   return wlfeature;
}

EXPORT_SYMBOL(kerSysGetWlanFeature);

/**
 *_get_wl_nandmanufacture check if system is nand system and manufacture bit is set
 *
 */
int _get_wl_nandmanufacture(void ) {

    int is_nand=0,is_m=0,has_size=0;
    unsigned int rootfs_ofs=0;
    unsigned char wlFeature=0;
    
    is_nand =( kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs) != -1 )? WLAN_MFG_PARTITION_ISNAND:0;
    wlFeature = kerSysGetWlanFeature();
    is_m = (wlFeature & WLAN_FEATURE_DHD_MFG_ENABLE)?WLAN_MFG_PARTITION_MFGSET:0;

    /*TODO: to be depend on UBOOT environment to decide if there is a similar partition existing to hold 
     * srom files for wifi card*/
    has_size = 0; // | WLAN_MFG_PARTITION_HASSIZE;
    has_size |= WLAN_BOOTMODE_IS_UBOOT;

    return is_nand|is_m|has_size;
}

/**
 * _wlsrom_write_file - write char content into file 
 * @name        file name
 * @content     the content to save
 * @size        size of the conten
 *
 * Return 0 success or -1 totherwise
 */
int _wlsrom_write_file(char *name,char *content,int size) {
       struct file *fp=filp_open(name,O_WRONLY|O_CREAT,0);
       int err=0;
       loff_t pos=0L;

       if(!IS_ERR(fp)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
          mm_segment_t old_fs=get_fs();
          set_fs(KERNEL_DS);
#endif
          vfs_write(fp,(void __user *)(content),size,&pos);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
          set_fs(old_fs);
#endif
          err=vfs_fsync(fp,0);
          if(err) 
              printk("SYNC to disk error!!!\n");
          filp_close(fp,NULL);
          return 0;
    }
       return -1;
}


/***************************************************************************/
// BP_BTN_ACTION_SES
void btnHook_Ses(unsigned long timeInMs, void* param) { 
    /* for new button implementation,press will do nothing */
    return;
}

#if defined(WIRELESS)
unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{
    if (atomic_read(&sesBtn_active)) {
        return POLLIN;
    }
    else if (atomic_read(&sesBtn_forced)) {
        atomic_set(&sesBtn_forced,0);
        return POLLIN;
    }
    return 0;
}

ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
    volatile unsigned int event=0;
    ssize_t ret=0;
    unsigned long flags;
    int exit = 0;

    /* Synchronization note: This code does a non-atomic test and set of
     * sesBtn_active that could cause a race-condition with btnHook_Ses,
     * so this must be protected with sesBtn_newapi_spinlock.
     */

    /* New button API: Return the type of SES button press (Short/Long) */
    spin_lock_irqsave(&sesBtn_newapi_spinlock, flags);
    if (atomic_read(&sesBtn_active) == SES_BTN_AP) {
        event = SES_EVENTS | SES_EVENT_BTN_AP;
        atomic_set(&sesBtn_active, 0);
    }
    else if (atomic_read(&sesBtn_active) == SES_BTN_STA) {
        event = SES_EVENTS | SES_EVENT_BTN_STA;
        atomic_set(&sesBtn_active, 0);
    }
    spin_unlock_irqrestore(&sesBtn_newapi_spinlock, flags);

    if (exit)
        return ret;

    if(! __copy_to_user((char*)buffer, (char*)&event, sizeof(event))) {
        count -= sizeof(event);
       buffer += sizeof(event);
       ret += sizeof(event);
    }
    else
        ret=-1;
    return ret;
}


void sesLed_ctrl(int action)
{
    char blinktype = ((action >> 24) & 0xff); /* extract blink type for SES_LED_BLINK  */

    BOARD_LED_STATE led;

    action &= 0xff; /* extract led */

    switch (action) {
    case SES_LED_ON:
        led = kLedStateOn;
        break;
    case SES_LED_BLINK:
        if(blinktype)
            led = blinktype;
        else
            led = kLedStateSlowBlinkContinues;
        break;
    case SES_LED_OFF:
    default:
        led = kLedStateOff;
    }

    kerSysLedCtrl(kLedSes, led);
}

void kerSysSesEventTrigger( int forced )
{
   if (forced) {
      atomic_set (&sesBtn_forced, 1);
   }
   wake_up_interruptible(&g_board_wait_queue);
}


/**
 *kerSysGetWlanSromParamsLen - get wlan calibration data len
 *
 */

int kerSysGetWlanSromParamsLen(void ) 
{
    int nm=_get_wl_nandmanufacture();
    if(nm<WLAN_MFG_PARTITION_HASSIZE) {
       char *hexstring =  kmalloc(2*NVRAM_WLAN_PARAMS_LEN+3, GFP_KERNEL); //*may starts with 0x and '0' end of string
       int len=0, hexstring_len;
       if (hexstring) {
           hexstring_len = envram_get_locked(NVRAM_WLANPARAMS,hexstring,2*NVRAM_WLAN_PARAMS_LEN+3);
           if(hexstring_len >=3) { /* either null or at least 3 */
               if (!strncasecmp(hexstring,"0x",2)) len=(hexstring_len-3)/2;
               else len = (hexstring_len-1)/2;
           }
           kfree(hexstring);
       }
          return len;
    } else {
        struct file *fp=filp_open(WL_SROM_CUSTOMER_FILE,O_RDONLY,0);
        if(!IS_ERR(fp)) {
            int cur_off=generic_file_llseek(fp,0,SEEK_CUR);
            int len=generic_file_llseek(fp,0,SEEK_END);
           generic_file_llseek(fp,cur_off,SEEK_SET);
           filp_close(fp,NULL);
           return len;
        } 
        return 0;
    }
}
EXPORT_SYMBOL(kerSysGetWlanSromParamsLen);

/*Read Wlan Params data from CFE */
int kerSysGetWlanSromParams( unsigned char *wlanParams, unsigned short len)
{
    int nm=_get_wl_nandmanufacture();
    if(nm<WLAN_MFG_PARTITION_HASSIZE) {
       char *hexstring =  kmalloc(2*len + 3 , GFP_KERNEL); //*may starts with 0x and '0' end of string
       if (hexstring) {
           int  hexstring_len = envram_get_locked(NVRAM_WLANPARAMS,hexstring,len);
           if(hexstring_len > 2*len) {
               int startIdx=0,i=0;
               if (!strncasecmp(hexstring,"0x",2))  startIdx = 2;
               for (i = 0; i < len; ++i) 
                   sscanf(hexstring+startIdx+2*i,"%02hhx",wlanParams+i);
           }
           kfree(hexstring);
           return 0;
       } else {
           printk("Failed to alloca memory!!\n");
       }
       printk("%s:%d failed to get wlanParams, expecting get len:%d\n",__FUNCTION__,__LINE__,len);
       return -1;
    } else {
        struct file *fp=filp_open(WL_SROM_CUSTOMER_FILE,O_RDONLY,0);
        if(!IS_ERR(fp)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
           int rl=kernel_read(fp,0,wlanParams,len);
#else
           int rl=kernel_read(fp, (void *)wlanParams, (size_t)len, &fp->f_pos);
#endif
           filp_close(fp,NULL);
           if(rl<=0) 
               return -1;
        } else {
              return -1;
        }
    }
    return 0;
}
EXPORT_SYMBOL(kerSysGetWlanSromParams);


void board_util_wl_godefault(void) {
	char  *envp[]= {"HOME=/",NULL};
	char *argv[]= {"/bin/nvram","godefault",NULL};
	call_usermodehelper(argv[0],argv,envp,UMH_WAIT_EXEC);
}

#if !defined(CONFIG_BCM_BCA_LEGACY_LED_API)
/* Get Internal WiFi Activity Led for the given core */
/* Return 0, if cLED is not cofigured in the board parameters for the core */
/* else return cled name */
unsigned int kerSysGetWifiLed(unsigned char core)
{
	unsigned char led_name = 0;
	unsigned short cled;

	switch (core) {
	    case 0:
	        if (BpGetWL0ActLedGpio(&cled) == BP_SUCCESS)
	            led_name =  kLedWL0;
	        break;
	    case 1:
	        if (BpGetWL1ActLedGpio(&cled) == BP_SUCCESS)
	            led_name =  kLedWL1;
	        break;
	    default:
	        led_name = 0;
	        break;
	}

	/* GPIO not configured */
	if (cled == 0) led_name = 0;
	
	printk("%s(%d) returned %d\n", __FUNCTION__, core, led_name);
	return led_name;
}
EXPORT_SYMBOL(kerSysGetWifiLed);

void kerSysWifiLed(unsigned int led, unsigned int on)
{
	kerSysLedCtrl(led, (on) ? kLedStateOn : kLedStateOff);
}
EXPORT_SYMBOL(kerSysWifiLed);
#endif /* !CONFIG_BCM_BCA_LEGACY_LED_API */
#endif /* WIRELESS */


void __init board_wl_init(void)
{
    ses_board_init();

    return;
}

void __exit board_wl_deinit(void)
{
    ses_board_deinit();
    return;
}
