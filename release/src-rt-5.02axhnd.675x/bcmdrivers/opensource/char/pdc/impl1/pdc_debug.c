/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>

#include "pdc.h"

void
pdc_print_buf(const char *prefix_str, uint8_t *data, uint16_t len)
{
#ifdef DEBUG
	print_hex_dump_debug(prefix_str, DUMP_PREFIX_NONE,
			     16, 1, data, len, false);
#endif
}

static ssize_t pdc_debugfs_read(struct file *filp, char __user *ubuf,
				size_t count, loff_t *offp)
{
	struct pdc_global *pdcg = filp->private_data;
	struct pdc_state *pdcs;
	char *buf;
	ssize_t ret, out_offset, out_count;
	int ch;

	out_count = 512 * pdcg->num_chan;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset = 0;
	for (ch=0; ch < pdcg->num_chan; ch++) {
		pdcs = &pdcg->pdc_state[ch];
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
				       "\nCH %u stats:\n", pdcs->channel);
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
				       "Requests............%u\n",
				       pdcs->requests);
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
				       "Responses...........%u\n",
				       pdcs->replies);
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
				       "Tx err ring full........%u\n",
				       pdcs->txnobuf);
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
				       "Rx err ring full........%u\n",
				       pdcs->rxnobuf);
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
				       "Receive overflow........%u\n",
				       pdcs->rx_oflow);

		if (out_offset > out_count)
			out_offset = out_count;
	}
	ret = simple_read_from_buffer(ubuf, count, offp, buf, out_offset);
	kfree(buf);
	return ret;
}

/*
 * Create the debug FS directories. If the top-level directory has not yet
 * been created, create it now. Create a stats file in this directory for
 * a SPU.
 */
void pdc_setup_debugfs(struct platform_device *pdev)
{
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	char spu_stats_name[16];

	if (!debugfs_initialized())
		return;

	snprintf(spu_stats_name, 16, "spu_stats");
	if (!pdcg->debugfs_dir)
	{
		pdcg->debugfs_dir = debugfs_create_dir(KBUILD_MODNAME, NULL);
	}

	pdcg->debugfs_fo.owner = THIS_MODULE;
	pdcg->debugfs_fo.open = simple_open;
	pdcg->debugfs_fo.read = pdc_debugfs_read;
	pdcg->debugfs_stats = debugfs_create_file(spu_stats_name, S_IRUSR,
	                                          pdcg->debugfs_dir, pdcg,
	                                          &pdcg->debugfs_fo);
}

void pdc_free_debugfs(struct platform_device *pdev)
{
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	if (pdcg->debugfs_dir && simple_empty(pdcg->debugfs_dir)) {
		debugfs_remove(pdcg->debugfs_dir);
		pdcg->debugfs_dir = NULL;
	}
}

void pdc_free_debugfs_stats(struct platform_device *pdev)
{
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	debugfs_remove(pdcg->debugfs_stats);
}
