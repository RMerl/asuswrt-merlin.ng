/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 */

#define DEBUG_SUBSYSTEM S_CLASS

#include <linux/seq_file.h>
#include <linux/statfs.h>
#include "../include/lprocfs_status.h"
#include "../include/obd_class.h"
#include "lmv_internal.h"

static int lmv_numobd_seq_show(struct seq_file *m, void *v)
{
	struct obd_device       *dev = (struct obd_device *)m->private;
	struct lmv_desc	 *desc;

	LASSERT(dev != NULL);
	desc = &dev->u.lmv.desc;
	seq_printf(m, "%u\n", desc->ld_tgt_count);
	return 0;
}
LPROC_SEQ_FOPS_RO(lmv_numobd);

static const char *placement_name[] = {
	[PLACEMENT_CHAR_POLICY] = "CHAR",
	[PLACEMENT_NID_POLICY]  = "NID",
	[PLACEMENT_INVAL_POLICY]  = "INVAL"
};

static enum placement_policy placement_name2policy(char *name, int len)
{
	int		     i;

	for (i = 0; i < PLACEMENT_MAX_POLICY; i++) {
		if (!strncmp(placement_name[i], name, len))
			return i;
	}
	return PLACEMENT_INVAL_POLICY;
}

static const char *placement_policy2name(enum placement_policy placement)
{
	LASSERT(placement < PLACEMENT_MAX_POLICY);
	return placement_name[placement];
}

static int lmv_placement_seq_show(struct seq_file *m, void *v)
{
	struct obd_device       *dev = (struct obd_device *)m->private;
	struct lmv_obd	  *lmv;

	LASSERT(dev != NULL);
	lmv = &dev->u.lmv;
	seq_printf(m, "%s\n", placement_policy2name(lmv->lmv_placement));
	return 0;
}

#define MAX_POLICY_STRING_SIZE 64

static ssize_t lmv_placement_seq_write(struct file *file,
					const char __user *buffer,
					size_t count, loff_t *off)
{
	struct obd_device *dev = ((struct seq_file *)file->private_data)->private;
	char		     dummy[MAX_POLICY_STRING_SIZE + 1];
	int		      len = count;
	enum placement_policy       policy;
	struct lmv_obd	  *lmv;

	if (copy_from_user(dummy, buffer, MAX_POLICY_STRING_SIZE))
		return -EFAULT;

	LASSERT(dev != NULL);
	lmv = &dev->u.lmv;

	if (len > MAX_POLICY_STRING_SIZE)
		len = MAX_POLICY_STRING_SIZE;

	if (dummy[len - 1] == '\n')
		len--;
	dummy[len] = '\0';

	policy = placement_name2policy(dummy, len);
	if (policy != PLACEMENT_INVAL_POLICY) {
		spin_lock(&lmv->lmv_lock);
		lmv->lmv_placement = policy;
		spin_unlock(&lmv->lmv_lock);
	} else {
		CERROR("Invalid placement policy \"%s\"!\n", dummy);
		return -EINVAL;
	}
	return count;
}
LPROC_SEQ_FOPS(lmv_placement);

static int lmv_activeobd_seq_show(struct seq_file *m, void *v)
{
	struct obd_device       *dev = (struct obd_device *)m->private;
	struct lmv_desc	 *desc;

	LASSERT(dev != NULL);
	desc = &dev->u.lmv.desc;
	seq_printf(m, "%u\n", desc->ld_active_tgt_count);
	return 0;
}
LPROC_SEQ_FOPS_RO(lmv_activeobd);

static int lmv_desc_uuid_seq_show(struct seq_file *m, void *v)
{
	struct obd_device *dev = (struct obd_device *)m->private;
	struct lmv_obd	  *lmv;

	LASSERT(dev != NULL);
	lmv = &dev->u.lmv;
	seq_printf(m, "%s\n", lmv->desc.ld_uuid.uuid);
	return 0;
}
LPROC_SEQ_FOPS_RO(lmv_desc_uuid);

static void *lmv_tgt_seq_start(struct seq_file *p, loff_t *pos)
{
	struct obd_device       *dev = p->private;
	struct lmv_obd	  *lmv = &dev->u.lmv;
	return (*pos >= lmv->desc.ld_tgt_count) ? NULL : lmv->tgts[*pos];
}

static void lmv_tgt_seq_stop(struct seq_file *p, void *v)
{
	return;
}

static void *lmv_tgt_seq_next(struct seq_file *p, void *v, loff_t *pos)
{
	struct obd_device       *dev = p->private;
	struct lmv_obd	  *lmv = &dev->u.lmv;
	++*pos;
	return (*pos >= lmv->desc.ld_tgt_count) ? NULL : lmv->tgts[*pos];
}

static int lmv_tgt_seq_show(struct seq_file *p, void *v)
{
	struct lmv_tgt_desc     *tgt = v;

	if (tgt == NULL)
		return 0;
	seq_printf(p, "%d: %s %sACTIVE\n",
		   tgt->ltd_idx, tgt->ltd_uuid.uuid,
		   tgt->ltd_active ? "" : "IN");
	return 0;
}

static struct seq_operations lmv_tgt_sops = {
	.start		 = lmv_tgt_seq_start,
	.stop		  = lmv_tgt_seq_stop,
	.next		  = lmv_tgt_seq_next,
	.show		  = lmv_tgt_seq_show,
};

static int lmv_target_seq_open(struct inode *inode, struct file *file)
{
	struct seq_file	 *seq;
	int		     rc;

	rc = seq_open(file, &lmv_tgt_sops);
	if (rc)
		return rc;

	seq = file->private_data;
	seq->private = PDE_DATA(inode);

	return 0;
}

LPROC_SEQ_FOPS_RO_TYPE(lmv, uuid);

static struct lprocfs_vars lprocfs_lmv_obd_vars[] = {
	{ "numobd",	  &lmv_numobd_fops,	  NULL, 0 },
	{ "placement",	  &lmv_placement_fops,    NULL, 0 },
	{ "activeobd",	  &lmv_activeobd_fops,    NULL, 0 },
	{ "uuid",	  &lmv_uuid_fops,	  NULL, 0 },
	{ "desc_uuid",	  &lmv_desc_uuid_fops,    NULL, 0 },
	{ NULL }
};

LPROC_SEQ_FOPS_RO_TYPE(lmv, numrefs);

static struct lprocfs_vars lprocfs_lmv_module_vars[] = {
	{ "num_refs",	   &lmv_numrefs_fops, NULL, 0 },
	{ NULL }
};

struct file_operations lmv_proc_target_fops = {
	.owner		= THIS_MODULE,
	.open		 = lmv_target_seq_open,
	.read		 = seq_read,
	.llseek	       = seq_lseek,
	.release	      = seq_release,
};

void lprocfs_lmv_init_vars(struct lprocfs_static_vars *lvars)
{
	lvars->module_vars    = lprocfs_lmv_module_vars;
	lvars->obd_vars       = lprocfs_lmv_obd_vars;
}
