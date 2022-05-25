/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
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

/*****************************************************************************
 *
 * Copyright (c) 2015 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Tim Ross <tross@broadcom.com>
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/if_ether.h>

#include "fpm_dev.h"
#include "fpm_priv.h"

void fpm_track_token_rx(u32 token)
{
	struct fpmdev *fdev = fpm;
	u32 tok_idx;

	if (!fdev->track_tokens)
		return;

	fpm_check_token(token);
	fpm_track_token_op(fpm_track_token_rx, token, 0);
	tok_idx = FPM_TOKEN_INDEX(token);
	tok_idx |= (FPM_TOKEN_POOL(token) << (FPM_TOKEN_INDEX_SHIFT + 1));
	if (fdev->tok_ref_count[tok_idx]) {
		pr_err("RX token (0x%08x) but internal ref count (%d) != 0\n",
		       token, fdev->tok_ref_count[tok_idx]);
		fdev->track_tokens = fdev->track_on_err;
	}
	fdev->tok_ref_count[tok_idx]++;
}
EXPORT_SYMBOL(fpm_track_token_rx);

void fpm_track_token_src(u32 token, union tok_src_dest *src)
{
	struct fpmdev *fdev = fpm;

	if (!fdev->track_tokens)
		return;
	if (!src) {
		pr_err("%s: Received void ptr for token op src data.\n",
		       __func__);
		return;
	}

	fpm_check_token(token);
	fpm_track_token_op(fpm_track_token_src, token, src->data);
}
EXPORT_SYMBOL(fpm_track_token_src);

void fpm_track_token_tx(u32 token)
{
	struct fpmdev *fdev = fpm;
	u32 tok_idx;

	if (!fdev->track_tokens)
		return;

	fpm_check_token(token);
	fpm_track_token_op(fpm_track_token_tx, token, 0);
	tok_idx = FPM_TOKEN_INDEX(token);
	tok_idx |= (FPM_TOKEN_POOL(token) << (FPM_TOKEN_INDEX_SHIFT + 1));
	fdev->tok_ref_count[tok_idx]--;
	/*
	 * The ref count should go to 0 when all TX's for this FPM
	 * have occurred. If the multi-count was incremented somewhere
	 * in our drivers we don't know if this is the last TX or not
	 * so we can't check for 0 here.
	 *
	 * TODO: Start timer here and check ref count went to 0 at timeout
	 * when all TX's should have occurred.
	 */
}
EXPORT_SYMBOL(fpm_track_token_tx);

void fpm_track_token_dest(u32 token, union tok_src_dest *dest)
{
	struct fpmdev *fdev = fpm;

	if (!fdev->track_tokens)
		return;
	if (!dest) {
		pr_err("%s: Received void ptr for token op dest data.\n",
		       __func__);
		return;
	}

	fpm_check_token(token);
	fpm_track_token_op(fpm_track_token_dest, token, dest->data);
}
EXPORT_SYMBOL(fpm_track_token_dest);
