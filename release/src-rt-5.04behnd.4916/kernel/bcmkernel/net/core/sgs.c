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
