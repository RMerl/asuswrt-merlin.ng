
/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcm_dgasphw.h                                            */
/*   PURPOSE: Dying gasp hw specific information.odule should include  */
/*                                                                     */
/***********************************************************************/
#ifndef _BCM_DGASPHW_H
#define _BCM_DGASPHW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/of_gpio.h>

/* Dg common resources */
typedef struct 
{
    volatile uint16_t * uart_data_reg;
    struct gpio_desc *dg_out_gpio;
    int dg_irq;
} BCM_DGASPHW_CMN;

int dgasp_hw_init(struct platform_device *pdev);
int dgasp_hw_disable_irq(void);
int dgasp_hw_enable_irq( int bIrqMapped );

#ifdef __cplusplus
}
#endif

#endif /* _BCM_DGASPHW_H */

