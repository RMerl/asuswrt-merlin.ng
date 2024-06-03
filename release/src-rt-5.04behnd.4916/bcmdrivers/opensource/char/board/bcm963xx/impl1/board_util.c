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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <bcmnetlink.h>
#include <net/sock.h>

#include <bcmtypes.h>
#include <board.h>

#include <flash_api.h>
#include <flash_common.h>
#include <shared_utils.h>
#include <bcm_pinmux.h>
#include <linux/bcm_log.h>
#include <bcmSpiRes.h>
#include "bcm_otp.h"

#include "board_util.h"
#include "board_image.h"
#include "board_dt.h"
#if defined(CONFIG_BCM_BOOTSTATE)
#include "bcm_bootstate.h"
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
#if defined(CONFIG_SMP)
#include <linux/cpu.h>
#endif
#include "pmc_dsl.h"
#endif
#include <bcm_strap_drv.h>

#if defined(CONFIG_BCM94908)
#include "clk_rst.h"
#endif

#include "bcm_mbox_map.h"

#include <linux/bcm_version_compat.h>

extern int g_ledInitialized;

// macAddrLock is used by kerSysGetMacAddress and kerSysReleaseMacAddress
// to protect access to g_pMacInfo
static PMAC_INFO g_pMacInfo = NULL;
static DEFINE_SPINLOCK(macAddrLock);
static PGPON_INFO g_pGponInfo = NULL;
static unsigned long g_ulSdramSize;

#define MAX_PAYLOAD_LEN 64
static struct sock *g_monitor_nl_sk;
static int g_monitor_nl_pid = 0 ;

static kerSysMacAddressNotifyHook_t kerSysMacAddressNotifyHook = NULL;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
/*SATA Test module callback */
int (*bcm_sata_test_ioctl_fn)(void *) = NULL; 
EXPORT_SYMBOL(bcm_sata_test_ioctl_fn);
#endif

/* A global variable used by Power Management and other features to determine if Voice is idle or not */
volatile int isVoiceIdle = 1;
EXPORT_SYMBOL(isVoiceIdle);

u32 __iomem *mbox_base = NULL;
  
static void board_init_mbox_base(void)
{
    return;
}

BOARD_IOC* board_ioc_alloc(void)
{
    BOARD_IOC *board_ioc =NULL;
    board_ioc = (BOARD_IOC*) kmalloc( sizeof(BOARD_IOC) , GFP_KERNEL );
    if(board_ioc)
    {
        memset(board_ioc, 0, sizeof(BOARD_IOC));
    }
    return board_ioc;
}

void board_ioc_free(BOARD_IOC* board_ioc)
{
    if(board_ioc)
    {
        kfree(board_ioc);
    }
}

int ConfigCs (BOARD_IOCTL_PARMS *parms)
{
    int                     retv = 0;
    return( retv );
}



PMAC_INFO get_mac_info(void)
{
    return  g_pMacInfo;
}

static int getNvram_ulNumMacAddrs(void)
{
    char value[25];
    int addrs=0, ret=0;

    memset(value, '\0', sizeof(value));
    ret=envram_get_locked(NVRAM_ULNUMMACADDRS, value, sizeof(value));
    if( ret > 0)
    {
        sscanf(value, "%d", &addrs);
    }
return addrs;
}

void getNvram_ucaBaseMacAddr(unsigned char *ucaBaseMacAddr, int len)
{
    char value[50];
    int ret=0;

    memset(value, '\0', sizeof(value));
    ret=envram_get_locked(NVRAM_UCABASEMACADDR, value, sizeof(value));
    if( ret > 0)
    {
        sscanf(value, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &ucaBaseMacAddr[0],
            &ucaBaseMacAddr[1],&ucaBaseMacAddr[2],&ucaBaseMacAddr[3],
            &ucaBaseMacAddr[4],&ucaBaseMacAddr[5]);
    }
	
}

void __init set_mac_info( void )
{
    unsigned int ulNumMacAddrs;

    ulNumMacAddrs=getNvram_ulNumMacAddrs();

    if( ulNumMacAddrs > 0 && ulNumMacAddrs <= NVRAM_MAC_COUNT_MAX )
    {
        unsigned long ulMacInfoSize =
            sizeof(MAC_INFO) + ((sizeof(MAC_ADDR_INFO)) * (ulNumMacAddrs-1));

        g_pMacInfo = (PMAC_INFO) kmalloc( ulMacInfoSize, GFP_KERNEL );

        if( g_pMacInfo )
        {
            memset( g_pMacInfo, 0x00, ulMacInfoSize );
            g_pMacInfo->ulNumMacAddrs = ulNumMacAddrs;
            getNvram_ucaBaseMacAddr( g_pMacInfo->ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);
        }
        else
            printk("ERROR - Could not allocate memory for MAC data\n");
    }
    else
        printk("ERROR - Invalid number of MAC addresses (%d) is configured.\n",
        ulNumMacAddrs);
}

static void get_gpon_params_from_nvram(PGPON_INFO g_pGponInfo)
{
char value[256];
int erased=1,r=0, len;

    memset(value, '\0', sizeof(value));
    r=envram_get_locked(NVRAM_GPONPASSWORD, value, sizeof(value));
    if ( r > 0) 
    {
       {
            len = NVRAM_XGPON_PASSWORD_LEN;
            erased = 0;
        }
        if(!erased)
        {
            strncpy( g_pGponInfo->gponPassword, value, len);
            g_pGponInfo->gponPassword[len-1]='\0';
        }
    }
    memset(value, '\0', sizeof(value));
    r=envram_get_locked(NVRAM_GPONSERIALNUMBER, value, sizeof(value));
    if (r > 0)
    {
        erased=1;
       {
                erased = 0;
        }
        if(!erased) {
            strncpy( g_pGponInfo->gponSerialNumber, value,
                    NVRAM_GPON_SERIAL_NUMBER_LEN );
            g_pGponInfo->gponSerialNumber[NVRAM_GPON_SERIAL_NUMBER_LEN-1]='\0';
        }
    }
}

void __init set_gpon_info( void )
{
    g_pGponInfo = (PGPON_INFO) kmalloc( sizeof(GPON_INFO), GFP_KERNEL );

    if( g_pGponInfo )
    {
        strcpy( g_pGponInfo->gponSerialNumber, DEFAULT_GPON_SN );
        strcpy( g_pGponInfo->gponPassword, DEFAULT_GPON_PW );
        //this cheks for the erased as well 
        get_gpon_params_from_nvram(g_pGponInfo);
    }
    else
    {
        printk("ERROR - Could not allocate memory for GPON data\n");
    }
}

//**************************************************************************************
// Utitlities for dump memory, free kernel pages, mips soft reset, etc.
//**************************************************************************************

/***********************************************************************
* Function Name: dumpaddr
* Description  : Display a hex dump of the specified address.
***********************************************************************/
void dumpaddr( unsigned char *pAddr, int nLen )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i, j;
    unsigned int ul;

    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (unsigned long) pAddr );
        p = szLine + strlen(szLine);

        for(i = 0; i < 16 && nLen > 0; i += sizeof(int), nLen -= sizeof(int))
        {
            ul = *(unsigned int *) &pAddr[i];
            q = (unsigned char *) &ul;
            for( j = 0; j < sizeof(int); j++ )
            {
                *p++ = szHexChars[q[j] >> 4];
                *p++ = szHexChars[q[j] & 0x0f];
                *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pAddr[j];
            *p++ = (ch > ' ' && ch < '~') ? ch : '.';
        }

        *p++ = '\0';
        printk( "%s\r\n", szLine );

        pAddr += i;
    }
    printk( "\r\n" );
} /* dumpaddr */



/* This is the low level hardware reset function. Normally called from kernel_restart,
 * the generic linux way of rebooting, which calls a notifier list, stop other cpu, disable 
 * local irq and lets sub-systems know that system is rebooting, and then calls machine_restart, 
 * which eventually call kerSysSoftReset. Do not call this function directly.
 */
void kerSysSoftReset(void)
{
    unsigned long cpu;
    cpu = smp_processor_id();
    printk(KERN_INFO "kerSysSoftReset: called on cpu %lu\n", cpu);
    // FIXME - Once in many thousands of reboots, this function could 
    // fail to complete a reboot.  Arm the watchdog just in case.

#if defined(CONFIG_BCM96XXX_WDT) 
    // give enough time(25ms) for resetPwrmgmtDdrMips and other function to finish
    // 4908 need to access some BPCM register which takes very long time to read/write
    bcmbca_wd_start(25000);
#endif
    resetPwrmgmtDdrMips();
}

/* this function is only called in SPI nor kernel flash update */
void stopOtherCpu(void)
{
#if defined(CONFIG_SMP)
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    /* in ARM, CPU#0 should be the last one to get shut down, and for
     * both 63138 and 63148, we have dualcore system, so we can hardcode
     * cpu_down() on CPU#1. Also, if this function is handled by the 
     * CPU which is going to be shut down, kernel will transfer the
     * current task to another CPU.  Thus when we return from cpu_down(),
     * the task is still running. */
    remove_cpu(1);
#else  
    /* This cause call stack dump unless we hack the system_state to reboot. See
       ipi_cpu_stop() in arch/arm64/kernel/smp.c. But not a big deal as this function
       is only used in SPI nor which is not offically supported in 63158/4908/6858 */
    smp_send_stop();
#endif
#endif
}

#if defined(CONFIG_BCM947622)

#define BCA_2x2AX_IOMAP_SIZE_BYTES (16 * 1024 * 1024) /**< one linear 16 MB region */
#define WLAN_SLICE_A_PHYS_ADDR 0x85000000
#define WLAN_SLICE_A_D11_MWR0_ADDR (WLAN_SLICE_A_PHYS_ADDR + 0x101000)
#define D11_MWR0_IOCTL_OFFSET 0x408
#define D11_MWR0_RESET_OFFSET 0x800

static void bcm47622a0_war(void)
{
    volatile unsigned char *regs = ioremap(WLAN_SLICE_A_D11_MWR0_ADDR, 4096);

    if (regs == NULL) {
        return;
    }
    /* resets WLAN slice-A mac core for boot-from-TXFIFO */
    *(volatile unsigned int*)(regs + D11_MWR0_RESET_OFFSET) = 0x1;
    *(volatile unsigned int*)(regs + D11_MWR0_IOCTL_OFFSET) = 0x3;
    mdelay(1);
    *(volatile unsigned int*)(regs + D11_MWR0_RESET_OFFSET) = 0x0;
}

#endif /* CONFIG_BCM947622 */

void resetPwrmgmtDdrMips(void)
{
#if defined(CONFIG_BCM947622)
    bcm47622a0_war();
#endif /* CONFIG_BCM947622 */
    
#if defined(CONFIG_BCM94908)
    /* reset the pll manually to bypass mode if strap for slow clock */
    if (bcm_strap_parse_and_test(NULL, "strap-cpu-slow-freq") == 1)
    {
        bcm_change_cpu_clk(BCM_CPU_CLK_LOW);
    }
#endif

    // let UART finish printing
    udelay(100);

#if defined(CONFIG_WATCHDOG)
#if defined(CONFIG_BRCM_SMC_BOOT) && defined(CONFIG_BCM_BOOTSTATE)
    // leave intact only activate reason bit
    bcmbca_set_boot_reason(bcmbca_get_boot_reason() & BCM_BOOT_REASON_ACTIVATE);
#endif
#if !defined(CONFIG_BRCM_SMC_BASED) || defined(CONFIG_BRCM_SMC_BOOT)
    bcmbca_wd_start(1);
#else
    {
        // VVV temporary code only for bring up of XIP mode
        volatile unsigned int* sw_reset_reg = ioremap(0x842100B8, 4);
        *sw_reset_reg |= 0x40000000;
    }
#endif
#endif    


    for(;;) {} // spin and wait soft reset to take effect
}

unsigned long kerSysGetMacAddressType( unsigned char *ifName )
{
    unsigned long macAddressType = MAC_ADDRESS_ANY;

    if(strstr(ifName, IF_NAME_ETH))
    {
        macAddressType = MAC_ADDRESS_ETH;
    }
    else if(strstr(ifName, IF_NAME_USB))
    {
        macAddressType = MAC_ADDRESS_USB;
    }
    else if(strstr(ifName, IF_NAME_WLAN))
    {
        macAddressType = MAC_ADDRESS_WLAN;
    }
    else if(strstr(ifName, IF_NAME_ATM))
    {
        macAddressType = MAC_ADDRESS_ATM;
    }
    else if(strstr(ifName, IF_NAME_PTM))
    {
        macAddressType = MAC_ADDRESS_PTM;
    }
    else if(strstr(ifName, IF_NAME_GPON) || strstr(ifName, IF_NAME_VEIP))
    {
        macAddressType = MAC_ADDRESS_GPON;
    }
    else if(strstr(ifName, IF_NAME_EPON))
    {
        macAddressType = MAC_ADDRESS_EPON;
    }

    return macAddressType;
}

static inline void kerSysMacAddressNotify(unsigned char *pucaMacAddr, MAC_ADDRESS_OPERATION op)
{
    if(kerSysMacAddressNotifyHook)
    {
        kerSysMacAddressNotifyHook(pucaMacAddr, op);
    }
}

int kerSysMacAddressNotifyBind(kerSysMacAddressNotifyHook_t hook)
{
    int nRet = 0;

    if(hook && kerSysMacAddressNotifyHook)
    {
        printk("ERROR: kerSysMacAddressNotifyHook already registered! <0x%p>\n",
               kerSysMacAddressNotifyHook);
        nRet = -EINVAL;
    }
    else
    {
        kerSysMacAddressNotifyHook = hook;
    }

    return nRet;
}

static void getNthMacAddr( unsigned char *pucaMacAddr, unsigned long n)
{
    unsigned long macsequence = 0;
    /* Work with only least significant three bytes of base MAC address */
    macsequence = (pucaMacAddr[3] << 16) | (pucaMacAddr[4] << 8) | pucaMacAddr[5];
    macsequence = (macsequence + n) & 0xffffff;
    pucaMacAddr[3] = (macsequence >> 16) & 0xff;
    pucaMacAddr[4] = (macsequence >> 8) & 0xff;
    pucaMacAddr[5] = (macsequence ) & 0xff;

}
static unsigned long getIdxForNthMacAddr( const unsigned char *pucaBaseMacAddr, unsigned char *pucaMacAddr)
{
    unsigned long macSequence = 0;
    unsigned long baseMacSequence = 0;
    
    macSequence = (pucaMacAddr[3] << 16) | (pucaMacAddr[4] << 8) | pucaMacAddr[5];
    baseMacSequence = (pucaBaseMacAddr[3] << 16) | (pucaBaseMacAddr[4] << 8) | pucaBaseMacAddr[5];

    return macSequence - baseMacSequence;
}
/* Allocates requested number of consecutive MAC addresses */
int kerSysGetMacAddresses( unsigned char *pucaMacAddr, unsigned int num_addresses, unsigned long ulId )
{
    int nRet = -EADDRNOTAVAIL;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL, pMaiFreeIdTemp;
    unsigned long i = 0, j = 0, ulIdxId = 0;

    spin_lock(&macAddrLock);

    /* Start with the base address */
    memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);

    for( i = 0, pMai = g_pMacInfo->MacAddrs; i < g_pMacInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            getNthMacAddr( pucaMacAddr, i );
            pMai->chInUse = 1;
            pMaiFreeId = NULL;
            nRet = 0;
            break;
        } else if( pMai->chInUse == 0 ) {
            /* check if it there are num_addresses to be checked starting from found MAC address */
            if ((i + num_addresses - 1) >= g_pMacInfo->ulNumMacAddrs) {
                break;
            }

            for (j = 1; j < num_addresses; j++) {
                pMaiFreeIdTemp = pMai + j;
                if (pMaiFreeIdTemp->chInUse != 0) {
                    break;
                }
            }
            if (j == num_addresses) {
                pMaiFreeId = pMai;
                ulIdxId = i;
                break;
            }
        }
    }

    if(pMaiFreeId )
    {
        /* An available MAC address was found. */
        getNthMacAddr( pucaMacAddr, ulIdxId );
        pMaiFreeIdTemp = pMai;
        for (j = 0; j < num_addresses; j++) {
            pMaiFreeIdTemp->ulId = ulId;
            pMaiFreeIdTemp->chInUse = 1;
            pMaiFreeIdTemp++;
        }
        nRet = 0;
    }

    spin_unlock(&macAddrLock);

    return( nRet );
} /* kerSysGetMacAddr */
int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    return kerSysGetMacAddresses(pucaMacAddr,1,ulId); /* Get only one address */
} /* kerSysGetMacAddr */


int kerSysReleaseMacAddresses( unsigned char *pucaMacAddr, unsigned int num_addresses )
{
    int i, nRet = -EINVAL;
    unsigned long ulIdx = 0;

    /* Check if we release a zero mac. If yes, return directly. */
    if( is_zero_ether_addr(pucaMacAddr) )
        return 0;

    spin_lock(&macAddrLock);

    ulIdx = getIdxForNthMacAddr(g_pMacInfo->ucaBaseMacAddr, pucaMacAddr);

    if( ulIdx < g_pMacInfo->ulNumMacAddrs )
    {
        for(i=0; i<num_addresses; i++) {
            if ((ulIdx + i) < g_pMacInfo->ulNumMacAddrs) {
                PMAC_ADDR_INFO pMai = &g_pMacInfo->MacAddrs[ulIdx + i];
                if( pMai->chInUse == 1 )
                {
                    pMai->chInUse = 0;
                    nRet = 0;
                }
            } else {
                printk("Request to release %d addresses failed as "
                    " the one or more of the addresses, starting from"
                    " %dth address from given address, requested for release"
                    " is not in the list of available MAC addresses \n", num_addresses, i);
                break;
            }
        }
    }

    spin_unlock(&macAddrLock);

    return( nRet );
} /* kerSysReleaseMacAddr */


int kerSysReleaseMacAddress( unsigned char *pucaMacAddr )
{
    return kerSysReleaseMacAddresses(pucaMacAddr,1); /* Release only one MAC address */

} /* kerSysReleaseMacAddr */


void kerSysGetGponSerialNumber( unsigned char *pGponSerialNumber )
{
    strncpy(pGponSerialNumber, g_pGponInfo->gponSerialNumber, NVRAM_GPON_SERIAL_NUMBER_LEN);
    pGponSerialNumber[NVRAM_GPON_SERIAL_NUMBER_LEN - 1] = 0;
}

void kerSysGetGponPassword( unsigned char *pGponPassword )
{
    strncpy(pGponPassword, g_pGponInfo->gponPassword, NVRAM_XGPON_PASSWORD_LEN - 1);
    pGponPassword[NVRAM_XGPON_PASSWORD_LEN - 1] = 0;
}

unsigned long kerSysGetSdramSize( void )
{
    return( g_ulSdramSize );
} /* kerSysGetSdramSize */
int kerSysGetAfeId( unsigned int *afeId )
{
    char value[50];
    int r;

    memset(value, '\0', sizeof(value));

    r=envram_get_locked(NVRAM_AFEID, value, sizeof(value));
    if ( r > 0)
       sscanf(value, "%x;%x", &afeId [0], &afeId[1]);
    else
        return -1;

    return 0;
}

#if !defined(CONFIG_BCM_BCA_LEGACY_LED_API)
void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    if (g_ledInitialized)
       boardLedCtrl(ledName, ledState);
}
#endif

/*functionto receive message from usersapce
 * Currently we dont expect any messages fromm userspace
 */
void kerSysRecvFrmMonitorTask(struct sk_buff *skb)
{

   /*process the message here*/
   printk(KERN_WARNING "unexpected skb received at %s \n",__FUNCTION__);
   kfree_skb(skb);
   return;
}

void kerSysInitMonitorSocket( void )
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
   g_monitor_nl_sk = netlink_kernel_create(&init_net, NETLINK_BRCM_MONITOR, 0, kerSysRecvFrmMonitorTask, NULL, THIS_MODULE);
#else
   struct netlink_kernel_cfg cfg = {
       .input  = kerSysRecvFrmMonitorTask,
   };
   g_monitor_nl_sk = netlink_kernel_create(&init_net, NETLINK_BRCM_MONITOR, &cfg);
#endif

   if(!g_monitor_nl_sk)
   {
      printk(KERN_ERR "Failed to create a netlink socket for monitor\n");
      return;
   }

}

void kerSysSetMonitorTaskPid(int pid)
{
    g_monitor_nl_pid = pid;
    return;
}

void kerSysSendtoMonitorTask(int msgType, char *msgData, int msgDataLen)
{

   struct sk_buff *skb =  NULL;
   struct nlmsghdr *nl_msgHdr = NULL;
   unsigned int nl_msgLen;

   if(!g_monitor_nl_pid)
   {
      printk(KERN_INFO "message received before monitor task is initialized %s \n",__FUNCTION__);
      return;
   } 

   if(msgData && (msgDataLen > MAX_PAYLOAD_LEN))
   {
      printk(KERN_ERR "invalid message len in %s",__FUNCTION__);
      return;
   } 

   nl_msgLen = NLMSG_SPACE(msgDataLen);

   /*Alloc skb ,this check helps to call the fucntion from interrupt context */

   if(in_atomic())
   {
      skb = alloc_skb(nl_msgLen, GFP_ATOMIC);
   }
   else
   {
      skb = alloc_skb(nl_msgLen, GFP_KERNEL);
   }

   if(!skb)
   {
      printk(KERN_ERR "failed to alloc skb in %s",__FUNCTION__);
      return;
   }

   nl_msgHdr = (struct nlmsghdr *)skb->data;
   nl_msgHdr->nlmsg_type = msgType;
   nl_msgHdr->nlmsg_pid=0;/*from kernel */
   nl_msgHdr->nlmsg_len = nl_msgLen;
   nl_msgHdr->nlmsg_flags =0;

   if(msgData)
   {
      memcpy(NLMSG_DATA(nl_msgHdr),msgData,msgDataLen);
   }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
   NETLINK_CB(skb).pid = 0; /*from kernel */
#endif

   skb->len = nl_msgLen; 

   netlink_unicast(g_monitor_nl_sk, skb, g_monitor_nl_pid, MSG_DONTWAIT);
   return;
}

void __exit kerSysCleanupMonitorSocket(void)
{
   g_monitor_nl_pid = 0 ;
   sock_release(g_monitor_nl_sk->sk_socket);
}


/***********************************************************************
 * Function Name: kerSysBlParmsGetInt
 * Description  : Returns the integer value for the requested name from
 *                the boot loader parameter buffer.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysBlParmsGetInt( char *name, int *pvalue )
{
    char *p2, *p1 = (char*)bcm_get_blparms();
    int ret = -1;

    *pvalue = -1;

    /* The g_blparms_buf buffer contains one or more contiguous NULL termianted
     * strings that ends with an empty string.
     */
    while( *p1 )
    {
        p2 = p1;

        while( *p2 != '=' && *p2 != '\0' )
            p2++;

        if( *p2 == '=' )
        {
            *p2 = '\0';

            if( !strcmp(p1, name) )
            {
                *p2++ = '=';
                *pvalue = simple_strtol(p2, &p1, 0);
                if( *p1 == '\0' )
                    ret = 0;
                break;
            }

            *p2 = '=';
        }

        p1 += strlen(p1) + 1;
    }

    return( ret );
}

/***********************************************************************
 * Function Name: kerSysBlParmsGetStr
 * Description  : Returns the string value for the requested name from
 *                the boot loader parameter buffer.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysBlParmsGetStr( char *name, char *pvalue, int size )
{
    char *p2, *p1 = (char*)bcm_get_blparms();
    int ret = -1;

    /* The g_blparms_buf buffer contains one or more contiguous NULL termianted
     * strings that ends with an empty string.
     */
    while( *p1 )
    {
        p2 = p1;

        while( *p2 != '=' && *p2 != '\0' )
            p2++;

        if( *p2 == '=' )
        {
            *p2 = '\0';

            if( !strcmp(p1, name) )
            {
                *p2++ = '=';
                strncpy(pvalue, p2, size);
                ret = 0;
                break;
            }

            *p2 = '=';
        }

        p1 += strlen(p1) + 1;
    }

    return( ret );
}



/***************************************************************************
 * Function Name: kerSysGetUbusFreq
 * Description  : Chip specific computation.
 * Returns      : the UBUS frequency value in MHz.
 ***************************************************************************/
unsigned int kerSysGetUbusFreq(unsigned int miscStrapBus)
{
   unsigned int ubus = UBUS_BASE_FREQUENCY_IN_MHZ;


   return (ubus);

}  /* kerSysGetUbusFreq */


/***************************************************************************
 * Function Name: kerSysGetChipId
 * Description  : Map id read from device hardware to id of chip family
 *                consistent with  BRCM_CHIP
 * Returns      : chip id of chip family
 ***************************************************************************/
int kerSysGetChipId() { 
        int r;
#if defined(CONFIG_BCM96858)
        r = 0x6858;
#elif defined(CONFIG_BCM96846)
        r = 0x68460;
#elif defined(CONFIG_BCM96878)
        r = 0x6878;
#elif defined(CONFIG_BCM96855)
        r = 0x6855;
#elif defined(CONFIG_BCM96856)
        r = 0x68560;
#else
        r = UtilGetChipId();
#endif

        return(r);
}

/***************************************************************************
 * Function Name: kerSysGetDslPhyEnable
 * Description  : returns true if device should permit Phy to load
 * Returns      : true/false
 ***************************************************************************/
int kerSysGetDslPhyEnable() {
#if IS_ENABLED(CONFIG_BCM_ADSL)
    uint32 id = UtilGetChipId();

    if (((id & 0xffff0) != 0x6750) && (id != 0x63141))
        return 1;
#endif
    return 0;
}

/***************************************************************************
 * Function Name: kerSysGetChipName
 * Description  : fills buf with the human-readable name of the device
 * Returns      : pointer to buf
 ***************************************************************************/
char *kerSysGetChipName(char *buf, int n) {
    return(UtilGetChipName(buf, n));
}

extern const struct obs_kernel_param __setup_start[], __setup_end[];
extern const struct kernel_param __start___param[], __stop___param[];

void kerSysSetBootParm(char *name, char *value)
{
    const struct obs_kernel_param *okp = __setup_start;
    const struct kernel_param *kp = __start___param;

    do {
        if (!strcmp(name, okp->str)) {
            if (okp->setup_func) {
                okp->setup_func(value);
                return;
            }
        }
        okp++;
    } while (okp < __setup_end);

    do {
        if (!strcmp(name, kp->name)) {
            if (kp->ops->set) {
                kp->ops->set(value, kp);
                return;
            }
        }
        kp++;
    } while (kp < __stop___param);
}
EXPORT_SYMBOL(kerSysSetBootParm);

int board_ioctl_mem_access(BOARD_MEMACCESS_IOCTL_PARMS* parms, char* kbuf, int len)
{
    int i, j;
    unsigned char *cp,*bcp;
    unsigned short *sp,*bsp;
    unsigned int *ip,*bip;
    void *va;

    bcp = (unsigned char *)kbuf;
    bsp = (unsigned short *)bcp;
    bip = (unsigned int *)bcp;

    switch(parms->space) {
        case BOARD_MEMACCESS_IOCTL_SPACE_REG:
            va = ioremap((long)parms->address, len);
            break;
        case BOARD_MEMACCESS_IOCTL_SPACE_KERN:
            va = (void*)(uintptr_t)parms->address;
            break;
        default:
            va = NULL;
            return EFAULT;
    }
    // printk("memacecssioctl address started %08x mapped to %08x size is %d count is %d\n",(int)parms.address, (int)va,parms.size, parms.count);
    cp = (unsigned char *)va;
    sp = (unsigned short *)((long)va & ~1);
    ip = (unsigned int *)((long)va & ~3);
    for (i=0; i < parms->count; i++) {
        if ((parms->op == BOARD_MEMACCESS_IOCTL_OP_WRITE) 
            || (parms->op == BOARD_MEMACCESS_IOCTL_OP_FILL)) {
            j = 0;
            if (parms->op == BOARD_MEMACCESS_IOCTL_OP_WRITE) 
            {
                j = i;
            }
            switch(parms->size) {
                case 1:
                    cp[i] = bcp[j];
                    break;
                case 2:
                    sp[i] = bsp[j];
                    break;
                case 4:
                    ip[i] = bip[j];
                    break;
            }
        } else {
                switch(parms->size) {
                case 1:
                    bcp[i] = cp[i];
                    break;
                case 2:
                    bsp[i] = sp[i];
                    break;
                case 4:
                    bip[i] = ip[i];
                    break;
            }
        }
    }
    
    if (va != (void*)(uintptr_t)parms->address)
    {
        iounmap(va);
    }
    return 0;
}

void __init board_util_init(void)
{
    g_ulSdramSize = getMemorySize();
    set_mac_info();
    set_gpon_info();

    board_init_mbox_base();

    kerSysInitMonitorSocket();

    return;
}

void __exit board_util_deinit(void)
{
    kerSysCleanupMonitorSocket();
    return;
}
