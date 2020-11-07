#if defined(CONFIG_BCM_KF_RUNNER)
/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "br_private.h"
#include "br_fp.h"

struct br_fp_data *fp_hooks;

void br_fp_set_callbacks(struct br_fp_data *fpdata)
{       
    fp_hooks = fpdata;
}

void br_fp_clear_callbacks(void)
{
    fp_hooks = NULL;
}

EXPORT_SYMBOL(br_fp_set_callbacks);
EXPORT_SYMBOL(br_fp_clear_callbacks);

#endif
