/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <bcm_dgasp.h>
#include "pmc_dgasp.h"
#include "bcm_dgasphw.h"
#include <bcm_ioremap_shared.h>
#include <bcm_otp.h>

/* In the periph registers, the 'TRIM' field actually holds
 * the THRESH value, while the HYS field holds hys values */
#define DGASP_PERIPH_DBG    1
#define DG_EN_SHIFT         3
#define DG_CTRL_MASK        0x00000030
#define DG_CTRL_SHIFT       4
#define DG_HYS_MASK         DG_CTRL_MASK
#define DG_HYS_SHIFT        DG_CTRL_SHIFT
#define DG_TRIM_MASK        0x00000007
#define DG_TRIM_SHIFT       0

/* In OTP the 'TRIM' value consists of both HYS and THRESH values */
#define OTP_DGASP_THRESH_SHIFT     0   /* THRESH = TRIM[1:0] */
#define OTP_DGASP_THRESH_MASK      0x3
#define OTP_DGASP_HYS_SHIFT        2   /* HYS = TRIM[3:2] */
#define OTP_DGASP_HYS_MASK         0xc

#define DGASP_PERIPH_REG_RES    "dg-periph-reg"
#define DGASP_OTP_TRIM_FLAG     "dg-get-otp-trim"
#define DGASP_OTP_HYS_FLAG      "dg-get-otp-hys"
#define DGASP_TRIM_VAL          "dg-trim"
#define DGASP_HYS_VAL           "dg-hys"

static uint32_t trim = 0;
static uint32_t hys = 0;
static uint32_t otp_trim_flag = 0;
static uint32_t otp_hys_flag = 0;

static volatile uint32_t* dgasp_periph_reg = NULL;

int dgasp_hw_init(struct platform_device *pdev)
{

    struct resource *res1 = NULL;
    void __iomem *reg_base = NULL;

    res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, DGASP_PERIPH_REG_RES );
    if ( !res1 ) 
    {
        dev_err(&pdev->dev, "Platform resource %s is missing\n", DGASP_PERIPH_REG_RES);
        return -EINVAL;
    }
    else
    {
        reg_base = devm_ioremap_shared_resource(&pdev->dev, res1);
#if DGASP_PERIPH_DBG        
        printk("%s: 0x%px<-->0x%px:0x%08x\n", DGASP_PERIPH_REG_RES, (void *)(uintptr_t)res1->start, (void *)reg_base, *(uint32_t*)reg_base);
#endif        

        if (IS_ERR(reg_base)) 
        {
            dev_err(&pdev->dev, "Ioremap failed for %s\n", DGASP_PERIPH_REG_RES);
            return -EINVAL;
        }
        else
            dgasp_periph_reg = reg_base;
    }

    /* get trim values from OTP or DTS as required*/
    of_property_read_u32(pdev->dev.of_node, DGASP_OTP_TRIM_FLAG, &otp_trim_flag);
#if DGASP_PERIPH_DBG        
    printk("%s: 0x%08x\n", DGASP_OTP_TRIM_FLAG, otp_trim_flag);
#endif        
    if( otp_trim_flag )
    {
        bcm_otp_get_dgasp_trim(&trim);
        trim = (trim & OTP_DGASP_THRESH_MASK) >> OTP_DGASP_THRESH_SHIFT;
#if DGASP_PERIPH_DBG        
        printk("otp-%s    : 0x%08x\n", DGASP_TRIM_VAL, trim);
#endif        
    }
    else
    {
        of_property_read_u32(pdev->dev.of_node, DGASP_TRIM_VAL, &trim);
#if DGASP_PERIPH_DBG        
        printk("dtb-%s    : 0x%08x\n", DGASP_TRIM_VAL, trim);
#endif        
    }

    /* get hys values from OTP or DTS as required */
    of_property_read_u32(pdev->dev.of_node, DGASP_OTP_HYS_FLAG, &otp_hys_flag);
#if DGASP_PERIPH_DBG        
    printk("%s : 0x%08x\n", DGASP_OTP_HYS_FLAG, otp_hys_flag);
#endif        
    if( otp_hys_flag )
    {
        bcm_otp_get_dgasp_trim(&hys);
        hys = (hys & OTP_DGASP_HYS_MASK) >> OTP_DGASP_HYS_SHIFT;
#if DGASP_PERIPH_DBG        
        printk("otp-%s     : 0x%08x\n", DGASP_HYS_VAL, hys);
#endif        
    }
    else
    {
        of_property_read_u32(pdev->dev.of_node, DGASP_HYS_VAL, &hys);
#if DGASP_PERIPH_DBG        
        printk("dtb-%s     : 0x%08x\n", DGASP_HYS_VAL, hys);
#endif  
    }

    *dgasp_periph_reg &= ~(DG_CTRL_MASK);
    *dgasp_periph_reg &= ~(DG_TRIM_MASK);
    *dgasp_periph_reg |= (hys << DG_HYS_SHIFT);
    *dgasp_periph_reg |= (trim << DG_TRIM_SHIFT);
    *dgasp_periph_reg |= (1 << DG_EN_SHIFT);

    return 0;
}

int dgasp_hw_disable_irq(void)
{
    return 0;
}

int dgasp_hw_enable_irq( int bIrqMapped )
{
    return 0;
}
