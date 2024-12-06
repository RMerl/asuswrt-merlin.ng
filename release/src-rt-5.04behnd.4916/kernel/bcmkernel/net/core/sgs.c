/*
 * <:copyright-BRCM:2021:DUAL/GPL:standard
 *
 *    Copyright (c) 2021 Broadcom
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
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/blog.h>
#include <linux/sgs.h>

static struct sgs_core_hooks sgs_core_hooks;

void sgs_nf_ct_delete_from_lists(struct nf_conn *ct)
{
	typeof(sgs_core_hooks.delete) fun;

	rcu_read_lock();
	fun = rcu_dereference(sgs_core_hooks.delete);
	if (fun)
		fun(ct);
	rcu_read_unlock();
}
EXPORT_SYMBOL(sgs_nf_ct_delete_from_lists);

int sgs_core_hooks_register(struct sgs_core_hooks *h)
{
	rcu_assign_pointer(sgs_core_hooks.delete, h->delete);
	return 0;
}
EXPORT_SYMBOL(sgs_core_hooks_register);

void sgs_core_hooks_unregister(void)
{
	rcu_assign_pointer(sgs_core_hooks.delete, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(sgs_core_hooks_unregister);
