/*
 * NCI based Driver for STMicroelectronics NFC Chip
 *
 * Copyright (C) 2014  STMicroelectronics SAS. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/nfc.h>
#include <net/nfc/nci.h>
#include <net/nfc/nci_core.h>

#include "st21nfcb.h"
#include "st21nfcb_se.h"

#define DRIVER_DESC "NCI NFC driver for ST21NFCB"

#define ST21NFCB_NCI1_X_PROPRIETARY_ISO15693 0x83

static int st21nfcb_nci_open(struct nci_dev *ndev)
{
	struct st21nfcb_nci_info *info = nci_get_drvdata(ndev);
	int r;

	if (test_and_set_bit(ST21NFCB_NCI_RUNNING, &info->flags))
		return 0;

	r = ndlc_open(info->ndlc);
	if (r)
		clear_bit(ST21NFCB_NCI_RUNNING, &info->flags);

	return r;
}

static int st21nfcb_nci_close(struct nci_dev *ndev)
{
	struct st21nfcb_nci_info *info = nci_get_drvdata(ndev);

	if (!test_and_clear_bit(ST21NFCB_NCI_RUNNING, &info->flags))
		return 0;

	ndlc_close(info->ndlc);

	return 0;
}

static int st21nfcb_nci_send(struct nci_dev *ndev, struct sk_buff *skb)
{
	struct st21nfcb_nci_info *info = nci_get_drvdata(ndev);

	skb->dev = (void *)ndev;

	if (!test_bit(ST21NFCB_NCI_RUNNING, &info->flags))
		return -EBUSY;

	return ndlc_send(info->ndlc, skb);
}

static __u32 st21nfcb_nci_get_rfprotocol(struct nci_dev *ndev,
					 __u8 rf_protocol)
{
	return rf_protocol == ST21NFCB_NCI1_X_PROPRIETARY_ISO15693 ?
		NFC_PROTO_ISO15693_MASK : 0;
}

static struct nci_ops st21nfcb_nci_ops = {
	.open = st21nfcb_nci_open,
	.close = st21nfcb_nci_close,
	.send = st21nfcb_nci_send,
	.get_rfprotocol = st21nfcb_nci_get_rfprotocol,
	.discover_se = st21nfcb_nci_discover_se,
	.enable_se = st21nfcb_nci_enable_se,
	.disable_se = st21nfcb_nci_disable_se,
	.se_io = st21nfcb_nci_se_io,
	.hci_load_session = st21nfcb_hci_load_session,
	.hci_event_received = st21nfcb_hci_event_received,
	.hci_cmd_received = st21nfcb_hci_cmd_received,
};

int st21nfcb_nci_probe(struct llt_ndlc *ndlc, int phy_headroom,
		       int phy_tailroom)
{
	struct st21nfcb_nci_info *info;
	int r;
	u32 protocols;

	info = devm_kzalloc(ndlc->dev,
			sizeof(struct st21nfcb_nci_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	protocols = NFC_PROTO_JEWEL_MASK
		| NFC_PROTO_MIFARE_MASK
		| NFC_PROTO_FELICA_MASK
		| NFC_PROTO_ISO14443_MASK
		| NFC_PROTO_ISO14443_B_MASK
		| NFC_PROTO_ISO15693_MASK
		| NFC_PROTO_NFC_DEP_MASK;

	ndlc->ndev = nci_allocate_device(&st21nfcb_nci_ops, protocols,
					phy_headroom, phy_tailroom);
	if (!ndlc->ndev) {
		pr_err("Cannot allocate nfc ndev\n");
		return -ENOMEM;
	}
	info->ndlc = ndlc;

	nci_set_drvdata(ndlc->ndev, info);

	r = nci_register_device(ndlc->ndev);
	if (r) {
		pr_err("Cannot register nfc device to nci core\n");
		nci_free_device(ndlc->ndev);
		return r;
	}

	return st21nfcb_se_init(ndlc->ndev);
}
EXPORT_SYMBOL_GPL(st21nfcb_nci_probe);

void st21nfcb_nci_remove(struct nci_dev *ndev)
{
	nci_unregister_device(ndev);
	nci_free_device(ndev);
}
EXPORT_SYMBOL_GPL(st21nfcb_nci_remove);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
