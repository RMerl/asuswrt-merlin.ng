/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#ifndef _TRXBUS_H_
struct device;
struct device_node;
int trxbus_dt_bus_get(struct device_node *dn);

#if __KERNEL__
#include <linux/of.h>
__attribute__((unused)) static int wan_i2c_bus_get(void)
{
    struct device_node *np;
    int bus = 0;

    np = of_find_compatible_node(NULL, NULL, "brcm,pon-drv");
    if (!np)
    {
        printk(KERN_ERR "%s: failed to find DT CFG. Use default bus<%d>\n", __FUNCTION__, bus);
    }
    else
    {
        bus = trxbus_dt_bus_get(np);
        of_node_put(np);
    }

    return bus;
}
#endif

#define BCM_I2C_PON_OPTICS_TYPE_LEGACY          0
#define BCM_I2C_PON_OPTICS_TYPE_PMD             1
#define BCM_I2C_PON_OPTICS_TYPE_NON_BRCM_PMD    2
#define BCM_I2C_PON_OPTICS_TYPE_SMTC            3

#define BCM_I2C_OPTICS_ENABLE 1
#define BCM_I2C_OPTICS_DISABLE 0

#define SMTC_CALIBRATION_FILE_PATH "/data/smtc_calibration" /*keeps smtc calibration values */
#define BCM_I2C_PROC_DIR "driver/bcm_i2c_pon_optics_type"

int trxbus_module_init(int bus, struct device *dev);
int trxbus_module_present(int bus, int is_pmd);
int trxbus_module_probe(int bus, struct device *dev);
void trxbus_module_removed(int bus);

void trxbus_register_detect(void *(*detect)(struct device *dev, int is_pmd));
void trxbus_register_mac(int bus, int force_tx_enable_before_detect,
    int (*detect_cb)(int bus, void *opticaldet_desc), int (*remove_cb)(int bus));
void trxbus_unregister_mac(int bus);
void trxbus_register_serdes(int bus, int (*serdes_control)(int bus, void *opticaldet_desc, int mode));

void trxbus_transmitter_control(int bus, int mode);
void trxbus_mac_transmit_ready_set(int bus);
int trxbus_is_pmd(int bus);

struct device *trxbus_get_dev(int bus);
void *trxbus_opticaldet_desc_get(int bus);

/* Deprecated function */
static inline int bcm_i2c_optics_rx_control(int enable __attribute__((unused))) { return 0; }
#endif
