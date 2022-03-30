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

#ifndef _LINUX_SGS_H_
#define _LINUX_SGS_H_

#define SGS_CT_ACCEPT_BIT	0
#define SGS_CT_BLOCK_BIT	1
#define SGS_CT_SESSION_BIT	2
#define SGS_CT_TERMINATED_BIT	3

#define SGS_MAGIC		0x600df00d /* Good Food */
struct sgs_info {
	unsigned long		valid;
	unsigned long		flags;
	unsigned long		rcvcnt;
	int			rstcnt;
};

struct nf_conn;

struct sgs_core_hooks {
    void (*delete)(struct nf_conn *ct);
};

int  sgs_core_hooks_register(struct sgs_core_hooks *h);
void sgs_nf_ct_delete_from_lists(struct nf_conn *ct);
void sgs_core_hooks_unregister(void);

#endif /* _LINUX_SGS_H_ */
