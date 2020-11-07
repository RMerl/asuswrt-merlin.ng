/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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


#include "bdmf_system.h"
#include "bdmf_system_common.h"
/*
 * System buffer abstraction
 */

/** Recycle the data buffer
 * \param[in]   data   Data buffer
 * \param[in]   context -unused,for future use
 */
void bdmf_sysb_databuf_recycle(void *datap, unsigned long context)
{
   return;
}

/** Recycle the system buffer and associated data buffer
 * \param[in]   data   Data buffer
 * \param[in]   context - unused,for future use
 * \param[in]   flags   - indicates what to recyle
 */
void bdmf_sysb_recycle(bdmf_sysb sysb, unsigned long context, uint32_t flags)
{
    return;
}

/*
 * Platform buffer support
 */
static inline void *bdmf_get_tm_ddr_base(void)
{
    return NULL;
}

/** Add data to sysb
 *
 * The function will is similar to skb_put()
 *
 * \param[in]   sysb        System buffer
 * \param[in]   bytes       Bytes to add
 * \returns added block pointer
 */
static inline void *bdmf_sysb_put(const bdmf_sysb sysb, uint32_t bytes)
{
   return NULL;
}

int bdmf_int_connect(int irq, int cpu, int flags,
    int (*isr)(int irq, void *priv), const char *name, void *priv)
{
	/* if needed, put CM code to register for an interrupt */
	return 0;
}

/** Unmask IRQ
 * \param[in]   irq IRQ
 */
void bdmf_int_enable(int irq)
{
    /*
     * in case of Oren the BcmHalMapInterrupt already enables the interrupt.
     */
}

/** Mask IRQ
 * \param[in]   irq IRQ
 */
void bdmf_int_disable(int irq)
{
    /* Supposingly should work for all BCM platforms.
     * If it is not the case - mode ifdefs can be added later.
     */
}
