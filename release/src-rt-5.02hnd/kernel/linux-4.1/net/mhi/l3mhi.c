#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/*
 * File: l3mhi.c
 *
 * L2 channels to AF_MHI binding.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/mhi.h>
#include <linux/mhi_l2mux.h>

#include <net/af_mhi.h>
#include <net/mhi/sock.h>
#include <net/mhi/dgram.h>

#define MAX_CHANNELS  256

#ifdef CONFIG_MHI_DEBUG
# define DPRINTK(...)    pr_debug("L3MHI: " __VA_ARGS__)
#else
# define DPRINTK(...)
#endif

/* Module parameters - with defaults */
static int l2chs[] = {
	MHI_L3_FILE,
	MHI_L3_XFILE,
	MHI_L3_SECURITY,
	MHI_L3_TEST,
	MHI_L3_TEST_PRIO,
	MHI_L3_LOG,
	MHI_L3_IMS,
	MHI_L3_OEM_CP,
	MHI_L3_THERMAL,
	MHI_L3_MHDP_UDP_FILTER,
	MHI_L3_HIGH_PRIO_TEST,
	MHI_L3_MED_PRIO_TEST,
	MHI_L3_LOW_PRIO_TEST,
};

static int l2cnt = sizeof(l2chs) / sizeof(int);

/* Functions */

static int mhi_netif_rx(struct sk_buff *skb, struct net_device *dev)
{
	skb->protocol = htons(ETH_P_MHI);

	return netif_rx(skb);
}

/* Module registration */

int __init l3mhi_init(void)
{
	int ch, i;
	int err;

	pr_info("MHI: %d Channels\n", l2cnt);
	for (i = 0; i < l2cnt; i++) {
		ch = l2chs[i];
		if (ch >= 0 && ch < MHI_L3_NPROTO) {
			err = l2mux_netif_rx_register(ch, mhi_netif_rx);
			if (err)
				goto error;

			err = mhi_register_protocol(ch);
			if (err)
				goto error;
		}
	}

	return 0;

error:
	for (i = 0; i < l2cnt; i++) {
		ch = l2chs[i];
		if (ch >= 0 && ch < MHI_L3_NPROTO) {
			if (mhi_protocol_registered(ch)) {
				l2mux_netif_rx_unregister(ch);
				mhi_unregister_protocol(ch);
			}
		}
	}

	return err;
}

void __exit l3mhi_exit(void)
{
	int ch, i;

	for (i = 0; i < l2cnt; i++) {
		ch = l2chs[i];
		if (ch >= 0 && ch < MHI_L3_NPROTO) {
			if (mhi_protocol_registered(ch)) {
				l2mux_netif_rx_unregister(ch);
				mhi_unregister_protocol(ch);
			}
		}
	}
}

module_init(l3mhi_init);
module_exit(l3mhi_exit);

module_param_array_named(l2_channels, l2chs, int, &l2cnt, 0444);

MODULE_DESCRIPTION("L3 MHI Binding");
#endif /* CONFIG_BCM_KF_MHI */
