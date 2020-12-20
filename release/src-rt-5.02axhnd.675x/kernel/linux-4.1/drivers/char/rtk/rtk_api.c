#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include "bcm_OS_Deps.h"
#include <linux/string.h>
#include "rtk_error.h"
#include "rtk_switch.h"
#include "cpu.h"
#include "rtl8367c_asicdrv_port.h"
#include <port.h>
#include <led.h>
#include "stat.h"
#include "vlan.h"
#include <board.h>
#include "smi.h"
#include "l2.h"

#define PROC_DIR                "rtkswitch"
#define TESTMODE_FILE           "testmode"

/* Function Name:
 *      rtk_port_phyTestMode_set
 * Description:
 *      Set PHY in test mode.
 * Input:
 *      port - port id.
 *      mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_NOT_ALLOWED      - The Setting is not allowed, caused by set more than 1 port in Test mode.
 * Note:
 *      Set PHY in test mode and only one PHY can be in test mode at the same time.
 *      It means API will return FAILED if other PHY is in test mode.
 *      This API only provide test mode 1 ~ 4 setup.
 */
static rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode)
{
    rtk_uint32          data, regData, i;
    rtk_api_ret_t       retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if(mode >= PHY_TEST_MODE_END)
        return RT_ERR_INPUT;

    if (PHY_TEST_MODE_NORMAL != mode)
    {
        /* Other port should be Normal mode */
        RTK_SCAN_ALL_LOG_PORT(i)
        {
            if(rtk_switch_isUtpPort(i) == RT_ERR_OK)
            {
                if(i != port)
                {
                    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(i), 9, &data)) != RT_ERR_OK)
                        return retVal;

                    if((data & 0xE000) != 0)
                        return RT_ERR_NOT_ALLOWED;
                }
            }
        }
    }

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
        return retVal;

    data &= ~0xE000;
    data |= (mode << 13);
    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), 9, data)) != RT_ERR_OK)
        return retVal;

    if (PHY_TEST_MODE_4 == mode)
    {
        if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
            return retVal;

        if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
            return retVal;

        if( (regData == 0x0276) || (regData == 0x0597) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xbcc2, 0xF4F4)) != RT_ERR_OK)
                return retVal;
        }

        if( (regData == 0x6367) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xbcc2, 0x77FF)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyTestMode_get
 * Description:
 *      Get PHY in which test mode.
 * Input:
 *      port - Port id.
 * Output:
 *      mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get test mode of PHY from register setting 9.15 to 9.13.
 */
static rtk_api_ret_t rtk_port_phyTestMode_get(rtk_port_t port, rtk_port_phy_test_mode_t *pMode)
{
    rtk_uint32      data;
    rtk_api_ret_t   retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
        return retVal;

    *pMode = (data & 0xE000) >> 13;

    return RT_ERR_OK;
}

static ssize_t ext_phytestmode_write(struct file *file,
				   const char __user * user_buffer,
				   size_t count, loff_t * offset)
{
	char  tmpbuf[64] = {0};
    u32 port = 0, mode = 0;
    int iRet = 0;

	if (count < 1) {
		return -EINVAL;
	}

    if(count > sizeof(tmpbuf) )
        count = sizeof(tmpbuf) - 1;

	if (user_buffer && !copy_from_user(tmpbuf, user_buffer, count)) {
		tmpbuf[count - 1] = '\0';
        iRet = sscanf(tmpbuf, "%x %x", &port, &mode); /*  */
        if(iRet != 2 )
        {
            goto errout;
        }
        rtk_port_phyTestMode_set(port - 1, mode); /*  */

        printk("set : port[0x%04x] -> mode[0x%04x]\n", port, mode);//        
	}
	else
	{
errout:	
	    printk("port mode set error.format: \"port mode\"\n");
	}
    
	return count;
}
static int ext_phytestmode_read(struct seq_file *s, void *v)
{	
	rtk_port_t port; 
    rtk_port_phy_test_mode_t cntrs;
    
    seq_puts(s, "   port       mode\n");

	for (port = UTP_PORT0; port < UTP_PORT4; port++) 
    {
        if (rtk_port_phyTestMode_get(port, &cntrs) != RT_ERR_OK)
				continue;
						
		seq_printf(s, "lan%d\t\t%d\n", port+1, cntrs);
	}
	
	return 0;
}
int ext_phytestmode_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, ext_phytestmode_read, NULL));
}
static ssize_t ext_phytestmode_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return ext_phytestmode_write(file, userbuf,count, off);
}
struct file_operations ext_phytestmode_proc_fops= {
        .open           = ext_phytestmode_single_open,
        .write		    = ext_phytestmode_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
static void rtk_create_proc(void)
{
	struct proc_dir_entry *entry;
    struct proc_dir_entry *parentdir;
    parentdir = proc_mkdir (PROC_DIR, NULL);
    if (parentdir == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return;
    }  
    
	entry = proc_create_data(TESTMODE_FILE, 0, parentdir, &ext_phytestmode_proc_fops, NULL);
	if (!entry) 
    {
		printk("can't create proc entry for %s\n", TESTMODE_FILE);
	}    
   
}

int rtk_ext_swctl_init(void)
{
    rtk_portmask_t portmask;

    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
	int ret;
    rtk_port_t cpuport;
    rtk_cpu_insert_t cpumode;
    int i = 0;
    
    kerSysSetGpioState(10, 0);
    udelay(50);
    kerSysSetGpioState(10, 1);
    while(1)
    {
        mdelay(150);
        i++;
        ret = rtk_switch_init();
        if(!ret)
            break;
        
    }
    printk("init switch %s:%d---%d\n", __FUNCTION__, __LINE__, i);

    rtk_create_proc();

    rtk_port_phyEnableAll_set(ENABLED);
    rtk_stat_global_reset();
    rtk_stat_port_reset(UTP_PORT0);
    rtk_stat_port_reset(UTP_PORT1);
    rtk_stat_port_reset(UTP_PORT2);
    rtk_stat_port_reset(UTP_PORT3);
	memset(&mac_cfg, 0x00, sizeof(rtk_port_mac_ability_t));

    /* Enable LED Group 0&1 from P0 to P4 */
  //  portmask.bits[0]=0x1F;
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT0);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT1);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT2);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT3);
    rtk_led_enable_set(LED_GROUP_0, &portmask);
    rtk_led_enable_set(LED_GROUP_1, &portmask);

    mode = MODE_EXT_RGMII; 
    mac_cfg.forcemode = MAC_FORCE; 
    mac_cfg.speed = PORT_SPEED_1000M; 
    mac_cfg.duplex = PORT_FULL_DUPLEX; 
    mac_cfg.link = PORT_LINKUP; 
    mac_cfg.nway = DISABLED; 
    mac_cfg.txpause = ENABLED; 
    mac_cfg.rxpause = ENABLED;   
    rtk_port_rgmiiDelayExt_set(EXT_PORT1, 1, 0);
        
	if ((ret = rtk_port_macForceLinkExt_set(EXT_PORT1, mode, &mac_cfg))!= RT_ERR_OK)
    {
        printk("init switch failed. %s:%d---%d\n", __FUNCTION__, __LINE__, ret);
	    return ret;
    }
    rtk_port_macForceLinkExt_get(EXT_PORT1, &mode, &mac_cfg);
    printk("init switch %s:%d---port:%d,mode:%d, linkstatus:%d\n", __FUNCTION__, __LINE__, EXT_PORT1,mode,mac_cfg.link );
   // rtk_port_sgmiiNway_set(EXT_PORT0, DISABLED);
    mode = MODE_EXT_DISABLE;
    rtk_port_macForceLinkExt_set(EXT_PORT0,mode,&mac_cfg);
  
    rtk_cpu_enable_set(ENABLED);
    rtk_cpu_tagPort_set(EXT_PORT1, CPU_INSERT_TO_NONE);
    
    rtk_cpu_tagPort_get(&cpuport,&cpumode);
    
    printk("init switch %s:%d---cpuport:%d,mode:%d\n", __FUNCTION__, __LINE__, cpuport,cpumode);

    return 0;	
}

EXPORT_SYMBOL(rtk_ext_swctl_init);


