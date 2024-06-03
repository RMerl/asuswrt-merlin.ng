/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

/*
   If pinmux changes are needed after Linux init, support should be added to the board 
   driver including any necessary locking.
 */

#include "bcm_otp.h"
#include "shared_utils.h"
#include <linux/io.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <bcm_otp_map.h>
#include <bcm_strap_drv.h>
#include <linux/module.h>
#include <linux/of_fdt.h>

static DEFINE_SPINLOCK(otp_spinlock);

#define OTP_DEBUG_IF    0
#define OTP_DBG_ENABLE  0

#if OTP_DBG_ENABLE
#if (INC_BTRM_BUILD==1)
#define OTP_DBG(... ) 
#else
#   define OTP_DBG_LEVEL      KERN_DEBUG
#   define OTP_DBG(fmt, args...) printk( OTP_DBG_LEVEL "OTP_drv: " fmt, ## args)
#endif /* INC_BTRM_BUILD==1 */
#else
#   define OTP_DBG(fmt, args...) /* not DBGging: nothing */
#endif /* OTP_DBG_ENABLE */

typedef enum _bcm_sec_states {
    SEC_STATE_UNSEC = 0,
    SEC_STATE_GEN3_MFG = 0x1,
    SEC_STATE_GEN3_FLD = 0x2
} bcm_sec_state_t;

struct g_otp_t {
    void __iomem *jtag_otp;
    int add_sec_check;
};

static struct g_otp_t g_bcm_otp; 
static bcm_sec_state_t sec_state = SEC_STATE_UNSEC;

int bcm_otp_ioctl_init(struct platform_device *pdev);

/************************************************************
 *  OTP CORE API                                            *
 ************************************************************/

#define OTP_READ_TIMEOUT_CNT       0x10000

#if OTP_DEBUG_IF
uint32_t otp_array[100] = {0};
#else
static int otp_wait_status(uint32_t status_reg_off, int status_mask, int status_val)
{
    int cntr = OTP_READ_TIMEOUT_CNT;
    while( cntr && ((readl(g_bcm_otp.jtag_otp + status_reg_off) & status_mask) != status_val ) )
    {
        cntr--;
    }

    if( cntr )
        return 0;
    else
        return -1;
}

#endif /* OTP_DEBUG_IF */

static int otp_get_cpu_lock(void)
{
    int ret = 0; 

#if defined(OTP_CPU_LOCK_OFF)
    writel(OTP_CPU_LOCK_MASK, g_bcm_otp.jtag_otp + OTP_CPU_LOCK_OFF);
    ret =  otp_wait_status( OTP_CPU_LOCK_OFF, OTP_CPU_LOCK_MASK, OTP_CPU_LOCK_MASK);
#endif

    return ret;
}

static int otp_release_cpu_lock(void)
{
    int ret = 0; 

#if defined(OTP_CPU_LOCK_OFF)
    writel(0, g_bcm_otp.jtag_otp + OTP_CPU_LOCK_OFF);
#endif

    return ret;
}

/************************************************************
 *  OTP CORE API for All Devices                            *
 ************************************************************/

/************************************************************
 *  int bcm_otp_fuse_row 
 *  Input parameters: 
 *     row   - Row address
 *     value - 32-bit OTP value
 *  Return value:
 *      returns 0 if successful
 ***********************************************************/
int bcm_otp_fuse_row(int row, unsigned int val)
{
    return bcm_otp_fuse_row_ecc(row, val, 0);
}
EXPORT_SYMBOL(bcm_otp_fuse_row);

static int bcm_otp_prog_mode_enable(void)
{
    int ret = -1;
    int i;
    int authVal[4] = {0xf,0x4,0x8,0xd};
    u32 data;
    void *otp_base = g_bcm_otp.jtag_otp; 

    /* Enable CPU side programming of OTP */
    data = readl(otp_base + OTP_CTRL1_OFF);
    writel(data|JTAG_OTP_CTRL_CPU_MODE, otp_base + OTP_CTRL1_OFF);

    /* Clear row register. A non-empty row register results in a failed prog-enable sequence on some SoCs */
    writel(0x0, otp_base + OTP_CTRL3_OFF);

    /* Put OTP in program mode --> prog-enable sequence */
    for (i=0;i<4;i++)
    {
        writel(authVal[i], otp_base + OTP_CTRL2_OFF);
        writel(JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_CMD_OTP_PROG_EN | JTAG_OTP_CTRL_PROG_EN, otp_base + OTP_CTRL0_OFF); 

        ret = otp_wait_status( OTP_STATUS1_OFF, JTAG_OTP_STATUS_1_CMD_DONE, JTAG_OTP_STATUS_1_CMD_DONE);
        if( ret )
        {
            OTP_DBG("%s: Error! Timed out waiting for OTP command completion (PROG)! status:0x%08x\n", __FUNCTION__,
                readl(otp_base + OTP_STATUS1_OFF));
            goto exit;
        }
        writel(0x0, otp_base + OTP_CTRL0_OFF); 
    }

exit:

    return ret;
}

/************************************************************
 *  int bcm_otp_fuse_row_ecc 
 *  Input parameters: 
 *     row   - Row address
 *     ecc   - row 7 bit ecc  
 *     value - 32-bit OTP value
 *  Return value:
 *      returns 0 if successful
 ***********************************************************/
int bcm_otp_fuse_row_ecc(int row, unsigned int val, unsigned int ecc)
{
    int ret = -1;
#if OTP_DEBUG_IF
    otp_array[addr] |= data;
    ret = 0;
#else      
    unsigned long flags;
    void *otp_base = g_bcm_otp.jtag_otp; 
    u32 data;
    spin_lock_irqsave(&otp_spinlock, flags);

    /* Get the hardware spinlock for OTP */
    ret = otp_get_cpu_lock();
    if( ret )
    {
        OTP_DBG("%s: Error! Timed out waiting for OTP CPU lock!\n", __FUNCTION__);
        goto exit_fuse;
    }

    ret = bcm_otp_prog_mode_enable();
    if (ret)
    {
        OTP_DBG("%s: Error! Failed to put OTP in PROG mode!\n", __FUNCTION__);
        goto exit_fuse;
    }

    /* Write OTP row */
    if ((readl(otp_base + OTP_STATUS1_OFF) & JTAG_OTP_STATUS_1_PROG_OK) == JTAG_OTP_STATUS_1_PROG_OK)
    {
        writel(val, otp_base + OTP_CTRL2_OFF);
#if defined(OTP_CTRL2_HI_OFF)
        if (ecc) {
            writel(ecc, otp_base + OTP_CTRL2_HI_OFF);
        }
#endif
        writel(row, otp_base + OTP_CTRL3_OFF);

        if(row == OTP_JTAG_CUST_LOCK_ROW)
        {
            data = 
                JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_CMD_PROG_LOCK | JTAG_OTP_CTRL_ACCESS_MODE | JTAG_OTP_CTRL_PROG_EN;
        }
        else
        {
            data = JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_CMD_PROG | JTAG_OTP_CTRL_ACCESS_MODE | JTAG_OTP_CTRL_PROG_EN;
        }
        writel(data, otp_base + OTP_CTRL0_OFF);


        ret = otp_wait_status(OTP_STATUS1_OFF, JTAG_OTP_STATUS_1_CMD_DONE, JTAG_OTP_STATUS_1_CMD_DONE);
        if( ret )
        {
            OTP_DBG("%s: Error! Timed out waiting for OTP command completion (WRITE)! status: 0x%08x\n", __FUNCTION__,
                readl(otp_base + OTP_STATUS1_OFF));
            goto exit_fuse;
        }

        writel(0x0, otp_base + OTP_CTRL0_OFF); 
        OTP_DBG("OTP fusing has completed as expected\n");
        ret = 0;
    }
    else
    {
        ret = -1;
        OTP_DBG("OTP fusing didn't complete as expected\n");
    }

    data = readl(otp_base + OTP_CTRL1_OFF);
    data &= ~JTAG_OTP_CTRL_CPU_MODE;
    writel(data, otp_base + OTP_CTRL1_OFF);

exit_fuse:

    /* Release hardware spinlock for OTP */
    otp_release_cpu_lock();

    spin_unlock_irqrestore(&otp_spinlock, flags);

#endif  /* OTP_DEBUG_IF */
    return ret;
}
EXPORT_SYMBOL(bcm_otp_fuse_row_ecc);

int bcm_otp_get_row(int row, unsigned int* val)
{
    return bcm_otp_get_row_ecc(row, val, NULL);
}
EXPORT_SYMBOL(bcm_otp_get_row);

/***********************************************************
 *  int bcm_otp_get_row_ecc
 *  Input parameters: 
 *     row    - Row address
 *     *val - Pointer to 32-bit OTP value
 *     *val_hi - Pointer to 32-bit OTP hi word data (typically ecc)
 *  Return value:
 *      returns 0 if successful, value in *value
 ***********************************************************/
int bcm_otp_get_row_ecc(int row, unsigned int* val, unsigned int* val_hi)
{
#if defined(CONFIG_BRCM_IKOS)
    *val =0;
    return 0;
#else
    int ret = -1;
#if OTP_DEBUG_IF
    *value = otp_array[row];
    ret = 0;
#else      
    unsigned long flags;
    u32 data;
    void* otp_base = g_bcm_otp.jtag_otp; 

    spin_lock_irqsave(&otp_spinlock, flags);

    /* Get the hardware spinlock for OTP */
    ret = otp_get_cpu_lock();
    if( ret )
    {
        OTP_DBG("%s: Error! Timed out waiting for OTP CPU lock!\n", __FUNCTION__);
        goto exit_read;
    }

    /* turn on cpu mode, set up row addr, activate read word */
    data = readl(otp_base + OTP_CTRL1_OFF);
    writel(data | JTAG_OTP_CTRL_CPU_MODE, otp_base + OTP_CTRL1_OFF); 

    writel(row, otp_base + OTP_CTRL3_OFF); 
    writel(JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_PROG_EN | JTAG_OTP_CTRL_ACCESS_MODE, otp_base + OTP_CTRL0_OFF);

    /* Wait for low CMD_DONE (current operation has begun), reset countdown, wait for retrieval to complete */
    ret = otp_wait_status(OTP_STATUS1_OFF, JTAG_OTP_STATUS_1_CMD_DONE, 0);

    /* Wait for high CMD_DONE */
    ret = otp_wait_status(OTP_STATUS1_OFF, JTAG_OTP_STATUS_1_CMD_DONE, JTAG_OTP_STATUS_1_CMD_DONE);
    if( ret )
    {
        OTP_DBG("%s: Error! Timed out waiting for OTP command completion (READ)! status: 0x%08x\n", __FUNCTION__,
            readl(otp_base + OTP_STATUS1_OFF));
        goto exit_read;
    }

    /* If read was successful, retrieve data */
    if(ret == 0)
    {
        *val = readl(otp_base + OTP_STATUS0_OFF);
#if defined(OTP_STATUS0_HI_OFF)
        if (val_hi)
        {
            *val_hi = readl(otp_base + OTP_STATUS0_HI_OFF);
        }
#endif
    }

    /* zero out the ctrl_0 reg, turn off cpu mode, return results */
    writel(0x0, otp_base + OTP_CTRL0_OFF); 

    data = readl(otp_base + OTP_CTRL1_OFF);
    data &= ~JTAG_OTP_CTRL_CPU_MODE;
    writel(data, otp_base + OTP_CTRL1_OFF);

exit_read:

    /* Release hardware spinlock for OTP */
    otp_release_cpu_lock();

    spin_unlock_irqrestore(&otp_spinlock, flags);

#endif  /* OTP_DEBUG_IF */
    return ret;
#endif /* CONFIG_BRCM_IKOS */
}
EXPORT_SYMBOL(bcm_otp_get_row_ecc);

/************************************************************
 *  OTP utility functions                                   *
 ************************************************************/


int bcm_is_btrm_boot(void)
{
    int rval = 0;
    unsigned int row;

    if(bcm_strap_parse_and_test(NULL, "bootrom-boot") == 1)
    {
        return 1;
    }

#if defined(CONFIG_BCM96858)
    if (bcm_get_boot_device() != SPI_NOR)
    {
        return 1;
    }
#endif

    rval = bcm_otp_get_row(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &row);
    if ( !rval && (row & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) )
    {
        rval = bcm_otp_get_row( OTP_CUST_BTRM_BOOT_ENABLE_ROW, &row);
        if ( !rval && (row & OTP_CUST_BTRM_BOOT_ENABLE_MASK) )
            rval = 1;
        else
            rval = 0;
    }
    else
        rval = 0;

    return rval;
}
EXPORT_SYMBOL(bcm_is_btrm_boot);

int bcm_otp_is_boot_secure(void)
{
    int rval = 0;
    if( sec_state != SEC_STATE_UNSEC)
    {
        if (g_bcm_otp.add_sec_check)
        {
            if (sec_state == SEC_STATE_GEN3_FLD)
            rval = 1;
        }
        else
            rval = 1;
    }
    return rval;
}
EXPORT_SYMBOL(bcm_otp_is_boot_secure);

int bcm_otp_is_boot_mfg_secure(void)
{
    return (sec_state == SEC_STATE_GEN3_MFG);
}
EXPORT_SYMBOL(bcm_otp_is_boot_mfg_secure);

#if defined(OTP_SGMII_DISABLE_ROW)
int bcm_otp_is_sgmii_disabled(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SGMII_DISABLE_ROW, &row);
    *val = (row & OTP_SGMII_DISABLE_MASK) >> OTP_SGMII_DISABLE_SHIFT;

    return rval;
}
EXPORT_SYMBOL(bcm_otp_is_sgmii_disabled);
#endif

#if defined(OTP_CPU_CLOCK_FREQ_ROW)
int bcm_otp_get_cpu_clk(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_CPU_CLOCK_FREQ_ROW, &row);
    *val = (row & OTP_CPU_CLOCK_FREQ_MASK) >> OTP_CPU_CLOCK_FREQ_SHIFT; 

    return rval;
}
EXPORT_SYMBOL(bcm_otp_get_cpu_clk);
#endif

int bcm_otp_is_usb3_disabled(unsigned int* val)
{
    int rval = 0; 
#if defined(OTP_USB3_DISABLE_ROW)
    unsigned int row;
    rval = bcm_otp_get_row(OTP_USB3_DISABLE_ROW, &row);

    if(row & OTP_USB3_DISABLE_MASK)
        *val = 1;
    else
        *val = 0;
#else
    *val = 0;
#endif

    return rval;
}
EXPORT_SYMBOL(bcm_otp_is_usb3_disabled);

#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM96765)
int bcm_otp_get_chipvar(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SEC_CHIPVAR_ROW, &row);
    *val = (row & OTP_SEC_CHIPVAR_MASK) >> OTP_SEC_CHIPVAR_SHIFT;

    return rval;
}
EXPORT_SYMBOL(bcm_otp_get_chipvar);
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
int bcm_otp_get_chipid(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SEC_CHIPVAR_ROW, &row);
    *val = (row & OTP_SEC_CHIPVAR_MASK) >> OTP_SEC_CHIPVAR_SHIFT;

    return rval;
}

int bcm_otp_get_usb_port_disabled(int port, unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_USB_DISABLE_ROW, &row);
    switch (port)
    {
    case 0:
        *val = (row & OTP_USB_DISABLE0_MASK) >> OTP_USB_DISABLE0_SHIFT;
        break;
    case 1:
        *val = (row & OTP_USB_DISABLE1_MASK) >> OTP_USB_DISABLE1_SHIFT;
        break;
    default:
        *val = (row & OTP_USB_DISABLE_XHCI_MASK) >> OTP_USB_DISABLE_XHCI_SHIFT;
    }

    return rval;
}
EXPORT_SYMBOL(bcm_otp_get_usb_port_disabled);
#endif

int bcm_otp_is_sata_disabled(unsigned int* val)
{
#if defined(OTP_SATA_DISABLE_ROW)
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SATA_DISABLE_ROW, &row);
    *val = (row & OTP_SATA_DISABLE_MASK) >> OTP_SATA_DISABLE_SHIFT;

    return rval;
#else
    *val = 0;
    return 0;
#endif
}
EXPORT_SYMBOL(bcm_otp_is_sata_disabled);

int bcm_otp_get_nr_cpus(unsigned int* val)
{
    int rval = 0;
#if defined(OTP_CPU_CORE_CFG_ROW)
    unsigned int row;
    
    rval = bcm_otp_get_row(OTP_CPU_CORE_CFG_ROW, &row);
    *val = (row & OTP_CPU_CORE_CFG_MASK) >> OTP_CPU_CORE_CFG_SHIFT;
#else
    *val = 0;
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
    if(UtilGetChipId() == CHIP_63152_ID_HEX)
        *val = 2;
#endif

#if defined(_BCM96846_) || defined(CONFIG_BCM96846)
    *val = 0;
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
    {
        unsigned int chipId = 0;

        bcm_otp_get_chipid(&chipId);
        if (chipId == 0x5 || chipId == 0x2 || chipId == 0xb)
            *val = 2;
    }
#endif

#if defined(_BCM96855_) || defined(CONFIG_BCM96855)
    if (*val<=5)
        *val = 0;
    else if (*val==6)
        *val = 1;
    else if (*val==7)
        *val = 2;
#endif

    return rval;
}
EXPORT_SYMBOL(bcm_otp_get_nr_cpus);

#if defined(OTP_PMC_BOOT_ROW)
int bcm_otp_get_pmc_boot_sts(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PMC_BOOT_ROW, &row);
    *val = (row & OTP_PMC_BOOT_MASK) >> OTP_PMC_BOOT_SHIFT;

    return rval;
}
EXPORT_SYMBOL(bcm_otp_get_pmc_boot_sts);
#endif

#if defined(OTP_PCM_DISABLE_ROW)
int bcm_otp_is_pcm_disabled(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PCM_DISABLE_ROW, &row);
    *val = (row & OTP_PCM_DISABLE_MASK) >> OTP_PCM_DISABLE_SHIFT;

    return rval;
}
EXPORT_SYMBOL(bcm_otp_is_pcm_disabled);
#endif

int bcm_otp_is_pcie_port_disabled(unsigned int pcie_port_num, unsigned int* val)
{
#if !defined(CONFIG_BRCM_QEMU) && defined(OTP_PCIE_PORT_DISABLE_ROW)
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PCIE_PORT_DISABLE_ROW, &row);
    row &= (OTP_PCIE_PORT_DISABLE_MASK);
    row = row >> OTP_PCIE_PORT_DISABLE_SHIFT;
    *val = row & (1 << pcie_port_num);
    return rval;
#else
    *val = 0;
    return 0;
#endif
}
EXPORT_SYMBOL(bcm_otp_is_pcie_port_disabled);

int bcm_otp_get_ldo_trim(unsigned int* val)
{
#if defined(OTP_LDO_TRIM_ROW)
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_LDO_TRIM_ROW, &row);

    *val = (row & OTP_LDO_TRIM_MASK) >> OTP_LDO_TRIM_SHIFT;
    return rval;
#else
    *val = 0;
    return 0;
#endif
}

int bcm_otp_is_rescal_enabled(unsigned int* val)
{
    int rval = 0;
#if defined(OTP_RESCAL_ENABLE_ROW)
    unsigned int row;
    rval = bcm_otp_get_row(OTP_RESCAL_ENABLE_ROW, &row);

    *val = (row & OTP_RESCAL_ENABLE_MASK) >> OTP_RESCAL_ENABLE_SHIFT;
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
    *val = 1;
#else
    *val = 0;
#endif
    return rval;
}


int bcm_otp_get_dgasp_trim(unsigned int* val)
{
#if defined(OTP_DGASP_TRIM_ROW)
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_DGASP_TRIM_ROW, &row);

    *val = (row & OTP_DGASP_TRIM_MASK) >> OTP_DGASP_TRIM_SHIFT;
    return rval;
#else
    *val = 0;
    return 0;
#endif
}
EXPORT_SYMBOL(bcm_otp_get_dgasp_trim);

int bcm_otp_auth_prog_mode(void)
{
    int ret = -1;
#if OTP_DEBUG_IF
    otp_array[addr] |= data;
    ret = 0;
#else      
    void *otp_base = g_bcm_otp.jtag_otp; 
    u32 data;
    unsigned long flags;
    spin_lock_irqsave(&otp_spinlock, flags);

    /* Get the hardware spinlock for OTP */
    ret = otp_get_cpu_lock();
    if( ret )
    {
        spin_unlock_irqrestore(&otp_spinlock, flags);
        OTP_DBG("%s: Error! Timed out waiting for OTP CPU lock!\n", __FUNCTION__);
        return ret;
    }
    ret = bcm_otp_prog_mode_enable();
    if (ret)
    {
        spin_unlock_irqrestore(&otp_spinlock, flags);
        OTP_DBG("%s: Error! Failed to put OTP in PROG mode!\n", __FUNCTION__);
        return ret;
    }

    writel(0x0, otp_base + OTP_CTRL0_OFF); 

    data = readl(otp_base + OTP_CTRL1_OFF);
    data &= ~JTAG_OTP_CTRL_CPU_MODE;
    writel(data, otp_base + OTP_CTRL1_OFF);

    /* Release hardware spinlock for OTP */
    otp_release_cpu_lock();
    spin_unlock_irqrestore(&otp_spinlock, flags);

    ret = 0;
#endif  /* OTP_DEBUG_IF */
    return ret;
}
EXPORT_SYMBOL(bcm_otp_auth_prog_mode);

static struct of_device_id const bcm_otp_drv_of_match[] = {
    { .compatible = "brcm,otp",},
    {}
};

MODULE_DEVICE_TABLE(of, bcm_otp_drv_of_match);

static const struct of_device_id* fdt_match_node(unsigned long node, const struct of_device_id * match_table)
{
    if (!match_table)
        return NULL;

    while (match_table->compatible)
    {
        if (of_flat_dt_is_compatible(node, match_table->compatible))
            return match_table;
        match_table++;
    }

    return NULL;
}

static int __init fdt_get_memory_prop(unsigned long node, int index, uint64_t* base, uint64_t* size)
{
    const __be32 *endp;
    const __be32 *reg;
    int regsize;
    int idx = 0;
    uint64_t value;

    reg = of_get_flat_dt_prop(node, "reg", &regsize);
    if (reg == NULL)
        return -ENODEV;
    endp = reg + (regsize / sizeof(__be32));
    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells))
    {
        value = dt_mem_next_cell(dt_root_addr_cells, &reg);
        if (base)
            *base = value;
        value = dt_mem_next_cell(dt_root_size_cells, &reg);
        if (size)
            *size = value;
        if (idx == index)
            return 0;

        idx++;
    }

    return -EINVAL;
}

static void __init fdt_get_sec_state(void) 
{
    int size = 0;
    const uint32_t* value;
    
    value = of_get_flat_dt_prop(of_get_flat_dt_root(), "brom_sec_state", &size);
    if( value )
        sec_state = (bcm_sec_state_t)be32_to_cpu(*value);	
    
    OTP_DBG("%s: Security_state:0x%08x\n", __FUNCTION__, sec_state);
}

static int __init fdt_get_bool_prop(unsigned long node, const char* prop_name)
{
    const __be32 *reg;
    int regsize;

    reg = of_get_flat_dt_prop(node, prop_name, &regsize);
    if (reg == NULL)
        return 0;
    else 
        return 1;
}

int __init bcm_otp_early_scan_dt(unsigned long node, const char *uname, int depth, void *data)
{
    const struct of_device_id *match;
    uint64_t base, size, offset;
    int ret = 0;

    if (strncmp(uname, "otp", 3) != 0)
        goto exit;

    match = fdt_match_node(node, bcm_otp_drv_of_match);
    if (!match)
    {
        printk("%s not match OTP driver\n", uname);
        goto exit;
    }

    base = of_flat_dt_translate_address(node);
    if (base == OF_BAD_ADDR) {
        printk("Failed to find otp resources\n");
        goto exit;
    }

    ret = fdt_get_memory_prop(node, 0, &offset, &size);
    if (ret)
    {
        printk("Failed to find otp resource\n");
        goto exit;
    }

    g_bcm_otp.jtag_otp = ioremap(base, size);
    if (IS_ERR_OR_NULL(g_bcm_otp.jtag_otp)) 
    {
        printk("Failed to map the OTP resource\n");
        ret = -ENXIO;
    }
    fdt_get_sec_state();

    g_bcm_otp.add_sec_check = fdt_get_bool_prop(node, "sec-additional-validation");

    ret = 0;

exit:
    return ret;
}

static struct platform_driver bcm_otp_driver = {
    .driver = {
        .name = "bcm-otp-drv",
        .of_match_table = bcm_otp_drv_of_match,
    },
    .probe = bcm_otp_ioctl_init,
};

static int __init bcm_otp_drv_reg(void)
{
    int ret;

    printk("OTP driver initcall\n");

    printk("OTP driver scanning DT\n");
    ret = of_scan_flat_dt(bcm_otp_early_scan_dt, NULL);
    if (ret) 
        goto error;

    return platform_driver_register(&bcm_otp_driver);

error:
    BUG();

    return ret;
}

core_initcall(bcm_otp_drv_reg);

MODULE_DESCRIPTION("Broadcom BCA OTP Driver");
MODULE_LICENSE("GPL v2");
