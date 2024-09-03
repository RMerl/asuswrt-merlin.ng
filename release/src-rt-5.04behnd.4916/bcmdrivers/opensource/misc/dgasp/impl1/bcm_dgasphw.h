
/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

