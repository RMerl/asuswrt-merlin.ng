/*
 *  Texas Instrument's NFC Driver For Shared Transport.
 *
 *  NFC Driver acts as interface between NCI core and
 *  TI Shared Transport Layer.
 *
 *  Copyright (C) 2011 Texas Instruments, Inc.
 *
 *  Written by Ilan Elias <ilane@ti.com>
 *
 *  Acknowledgements:
 *  This file is based on btwilink.c, which was written
 *  by Raja Mani and Pavan Savoy.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/firmware.h>
#include <linux/nfc.h>
#include <net/nfc/nci.h>
#include <net/nfc/nci_core.h>
#include <linux/ti_wilink_st.h>

#define NFCWILINK_CHNL			12
#define NFCWILINK_OPCODE		7
#define NFCWILINK_MAX_FRAME_SIZE	300
#define NFCWILINK_HDR_LEN		4
#define NFCWILINK_OFFSET_LEN_IN_HDR	1
#define NFCWILINK_LEN_SIZE		2
#define NFCWILINK_REGISTER_TIMEOUT	8000	/* 8 sec */
#define NFCWILINK_CMD_TIMEOUT		5000	/* 5 sec */

#define BTS_FILE_NAME_MAX_SIZE		40
#define BTS_FILE_HDR_MAGIC		0x42535442
#define BTS_FILE_CMD_MAX_LEN		0xff
#define BTS_FILE_ACTION_TYPE_SEND_CMD	1

#define NCI_VS_NFCC_INFO_CMD_GID	0x2f
#define NCI_VS_NFCC_INFO_CMD_OID	0x12
#define NCI_VS_NFCC_INFO_RSP_GID	0x4f
#define NCI_VS_NFCC_INFO_RSP_OID	0x12

struct nfcwilink_hdr {
	__u8 chnl;
	__u8 opcode;
	__le16 len;
} __packed;

struct nci_vs_nfcc_info_cmd {
	__u8 gid;
	__u8 oid;
	__u8 plen;
} __packed;

struct nci_vs_nfcc_info_rsp {
	__u8 gid;
	__u8 oid;
	__u8 plen;
	__u8 status;
	__u8 hw_id;
	__u8 sw_ver_x;
	__u8 sw_ver_z;
	__u8 patch_id;
} __packed;

struct bts_file_hdr {
	__le32 magic;
	__le32 ver;
	__u8 rfu[24];
	__u8 actions[0];
} __packed;

struct bts_file_action {
	__le16 type;
	__le16 len;
	__u8 data[0];
} __packed;

struct nfcwilink {
	struct platform_device		*pdev;
	struct nci_dev			*ndev;
	unsigned long			flags;

	char				st_register_cb_status;
	long				(*st_write) (struct sk_buff *);

	struct completion		completed;

	struct nci_vs_nfcc_info_rsp	nfcc_info;
};

/* NFCWILINK driver flags */
enum {
	NFCWILINK_RUNNING,
	NFCWILINK_FW_DOWNLOAD,
};

static int nfcwilink_send(struct nci_dev *ndev, struct sk_buff *skb);

static inline struct sk_buff *nfcwilink_skb_alloc(unsigned int len, gfp_t how)
{
	struct sk_buff *skb;

	skb = alloc_skb(len + NFCWILINK_HDR_LEN, how);
	if (skb)
		skb_reserve(skb, NFCWILINK_HDR_LEN);

	return skb;
}

static void nfcwilink_fw_download_receive(struct nfcwilink *drv,
						struct sk_buff *skb)
{
	struct nci_vs_nfcc_info_rsp *rsp = (void *)skb->data;

	/* Detect NCI_VS_NFCC_INFO_RSP and store the result */
	if ((skb->len > 3) && (rsp->gid == NCI_VS_NFCC_INFO_RSP_GID) &&
		(rsp->oid == NCI_VS_NFCC_INFO_RSP_OID)) {
		memcpy(&drv->nfcc_info, rsp,
			sizeof(struct nci_vs_nfcc_info_rsp));
	}

	kfree_skb(skb);

	complete(&drv->completed);
}

static int nfcwilink_get_bts_file_name(struct nfcwilink *drv, char *file_name)
{
	struct nci_vs_nfcc_info_cmd *cmd;
	struct sk_buff *skb;
	unsigned long comp_ret;
	int rc;

	skb = nfcwilink_skb_alloc(sizeof(struct nci_vs_nfcc_info_cmd),
					GFP_KERNEL);
	if (!skb) {
		nfc_err(&drv->pdev->dev,
			"no memory for nci_vs_nfcc_info_cmd\n");
		return -ENOMEM;
	}

	cmd = (struct nci_vs_nfcc_info_cmd *)
			skb_put(skb, sizeof(struct nci_vs_nfcc_info_cmd));
	cmd->gid = NCI_VS_NFCC_INFO_CMD_GID;
	cmd->oid = NCI_VS_NFCC_INFO_CMD_OID;
	cmd->plen = 0;

	drv->nfcc_info.plen = 0;

	rc = nfcwilink_send(drv->ndev, skb);
	if (rc)
		return rc;

	comp_ret = wait_for_completion_timeout(&drv->completed,
				msecs_to_jiffies(NFCWILINK_CMD_TIMEOUT));
	dev_dbg(&drv->pdev->dev, "wait_for_completion_timeout returned %ld\n",
		comp_ret);
	if (comp_ret == 0) {
		nfc_err(&drv->pdev->dev,
			"timeout on wait_for_completion_timeout\n");
		return -ETIMEDOUT;
	}

	dev_dbg(&drv->pdev->dev, "nci_vs_nfcc_info_rsp: plen %d, status %d\n",
		drv->nfcc_info.plen, drv->nfcc_info.status);

	if ((drv->nfcc_info.plen != 5) || (drv->nfcc_info.status != 0)) {
		nfc_err(&drv->pdev->dev, "invalid nci_vs_nfcc_info_rsp\n");
		return -EINVAL;
	}

	snprintf(file_name, BTS_FILE_NAME_MAX_SIZE,
			"TINfcInit_%d.%d.%d.%d.bts",
			drv->nfcc_info.hw_id,
			drv->nfcc_info.sw_ver_x,
			drv->nfcc_info.sw_ver_z,
			drv->nfcc_info.patch_id);

	nfc_info(&drv->pdev->dev, "nfcwilink FW file name: %s\n", file_name);

	return 0;
}

static int nfcwilink_send_bts_cmd(struct nfcwilink *drv, __u8 *data, int len)
{
	struct nfcwilink_hdr *hdr = (struct nfcwilink_hdr *)data;
	struct sk_buff *skb;
	unsigned long comp_ret;
	int rc;

	/* verify valid cmd for the NFC channel */
	if ((len <= sizeof(struct nfcwilink_hdr)) ||
		(len > BTS_FILE_CMD_MAX_LEN) ||
		(hdr->chnl != NFCWILINK_CHNL) ||
		(hdr->opcode != NFCWILINK_OPCODE)) {
		nfc_err(&drv->pdev->dev,
			"ignoring invalid bts cmd, len %d, chnl %d, opcode %d\n",
			len, hdr->chnl, hdr->opcode);
		return 0;
	}

	/* remove the ST header */
	len -= sizeof(struct nfcwilink_hdr);
	data += sizeof(struct nfcwilink_hdr);

	skb = nfcwilink_skb_alloc(len, GFP_KERNEL);
	if (!skb) {
		nfc_err(&drv->pdev->dev, "no memory for bts cmd\n");
		return -ENOMEM;
	}

	memcpy(skb_put(skb, len), data, len);

	rc = nfcwilink_send(drv->ndev, skb);
	if (rc)
		return rc;

	comp_ret = wait_for_completion_timeout(&drv->completed,
				msecs_to_jiffies(NFCWILINK_CMD_TIMEOUT));
	dev_dbg(&drv->pdev->dev, "wait_for_completion_timeout returned %ld\n",
		comp_ret);
	if (comp_ret == 0) {
		nfc_err(&drv->pdev->dev,
			"timeout on wait_for_completion_timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int nfcwilink_download_fw(struct nfcwilink *drv)
{
	unsigned char file_name[BTS_FILE_NAME_MAX_SIZE];
	const struct firmware *fw;
	__u16 action_type, action_len;
	__u8 *ptr;
	int len, rc;

	set_bit(NFCWILINK_FW_DOWNLOAD, &drv->flags);

	rc = nfcwilink_get_bts_file_name(drv, file_name);
	if (rc)
		goto exit;

	rc = request_firmware(&fw, file_name, &drv->pdev->dev);
	if (rc) {
		nfc_err(&drv->pdev->dev, "request_firmware failed %d\n", rc);

		/* if the file is not found, don't exit with failure */
		if (rc == -ENOENT)
			rc = 0;

		goto exit;
	}

	len = fw->size;
	ptr = (__u8 *)fw->data;

	if ((len == 0) || (ptr == NULL)) {
		dev_dbg(&drv->pdev->dev,
			"request_firmware returned size %d\n", len);
		goto release_fw;
	}

	if (__le32_to_cpu(((struct bts_file_hdr *)ptr)->magic) !=
			BTS_FILE_HDR_MAGIC) {
		nfc_err(&drv->pdev->dev, "wrong bts magic number\n");
		rc = -EINVAL;
		goto release_fw;
	}

	/* remove the BTS header */
	len -= sizeof(struct bts_file_hdr);
	ptr += sizeof(struct bts_file_hdr);

	while (len > 0) {
		action_type =
			__le16_to_cpu(((struct bts_file_action *)ptr)->type);
		action_len =
			__le16_to_cpu(((struct bts_file_action *)ptr)->len);

		dev_dbg(&drv->pdev->dev, "bts_file_action type %d, len %d\n",
			action_type, action_len);

		switch (action_type) {
		case BTS_FILE_ACTION_TYPE_SEND_CMD:
			rc = nfcwilink_send_bts_cmd(drv,
					((struct bts_file_action *)ptr)->data,
					action_len);
			if (rc)
				goto release_fw;
			break;
		}

		/* advance to the next action */
		len -= (sizeof(struct bts_file_action) + action_len);
		ptr += (sizeof(struct bts_file_action) + action_len);
	}

release_fw:
	release_firmware(fw);

exit:
	clear_bit(NFCWILINK_FW_DOWNLOAD, &drv->flags);
	return rc;
}

/* Called by ST when registration is complete */
static void nfcwilink_register_complete(void *priv_data, char data)
{
	struct nfcwilink *drv = priv_data;

	/* store ST registration status */
	drv->st_register_cb_status = data;

	/* complete the wait in nfc_st_open() */
	complete(&drv->completed);
}

/* Called by ST when receive data is available */
static long nfcwilink_receive(void *priv_data, struct sk_buff *skb)
{
	struct nfcwilink *drv = priv_data;
	int rc;

	if (!skb)
		return -EFAULT;

	if (!drv) {
		kfree_skb(skb);
		return -EFAULT;
	}

	dev_dbg(&drv->pdev->dev, "receive entry, len %d\n", skb->len);

	/* strip the ST header
	(apart for the chnl byte, which is not received in the hdr) */
	skb_pull(skb, (NFCWILINK_HDR_LEN-1));

	if (test_bit(NFCWILINK_FW_DOWNLOAD, &drv->flags)) {
		nfcwilink_fw_download_receive(drv, skb);
		return 0;
	}

	/* Forward skb to NCI core layer */
	rc = nci_recv_frame(drv->ndev, skb);
	if (rc < 0) {
		nfc_err(&drv->pdev->dev, "nci_recv_frame failed %d\n", rc);
		return rc;
	}

	return 0;
}

/* protocol structure registered with ST */
static struct st_proto_s nfcwilink_proto = {
	.chnl_id = NFCWILINK_CHNL,
	.max_frame_size = NFCWILINK_MAX_FRAME_SIZE,
	.hdr_len = (NFCWILINK_HDR_LEN-1),	/* not including chnl byte */
	.offset_len_in_hdr = NFCWILINK_OFFSET_LEN_IN_HDR,
	.len_size = NFCWILINK_LEN_SIZE,
	.reserve = 0,
	.recv = nfcwilink_receive,
	.reg_complete_cb = nfcwilink_register_complete,
	.write = NULL,
};

static int nfcwilink_open(struct nci_dev *ndev)
{
	struct nfcwilink *drv = nci_get_drvdata(ndev);
	unsigned long comp_ret;
	int rc;

	if (test_and_set_bit(NFCWILINK_RUNNING, &drv->flags)) {
		rc = -EBUSY;
		goto exit;
	}

	nfcwilink_proto.priv_data = drv;

	init_completion(&drv->completed);
	drv->st_register_cb_status = -EINPROGRESS;

	rc = st_register(&nfcwilink_proto);
	if (rc < 0) {
		if (rc == -EINPROGRESS) {
			comp_ret = wait_for_completion_timeout(
			&drv->completed,
			msecs_to_jiffies(NFCWILINK_REGISTER_TIMEOUT));

			dev_dbg(&drv->pdev->dev,
				"wait_for_completion_timeout returned %ld\n",
				comp_ret);

			if (comp_ret == 0) {
				/* timeout */
				rc = -ETIMEDOUT;
				goto clear_exit;
			} else if (drv->st_register_cb_status != 0) {
				rc = drv->st_register_cb_status;
				nfc_err(&drv->pdev->dev,
					"st_register_cb failed %d\n", rc);
				goto clear_exit;
			}
		} else {
			nfc_err(&drv->pdev->dev, "st_register failed %d\n", rc);
			goto clear_exit;
		}
	}

	/* st_register MUST fill the write callback */
	BUG_ON(nfcwilink_proto.write == NULL);
	drv->st_write = nfcwilink_proto.write;

	if (nfcwilink_download_fw(drv)) {
		nfc_err(&drv->pdev->dev, "nfcwilink_download_fw failed %d\n",
			rc);
		/* open should succeed, even if the FW download failed */
	}

	goto exit;

clear_exit:
	clear_bit(NFCWILINK_RUNNING, &drv->flags);

exit:
	return rc;
}

static int nfcwilink_close(struct nci_dev *ndev)
{
	struct nfcwilink *drv = nci_get_drvdata(ndev);
	int rc;

	if (!test_and_clear_bit(NFCWILINK_RUNNING, &drv->flags))
		return 0;

	rc = st_unregister(&nfcwilink_proto);
	if (rc)
		nfc_err(&drv->pdev->dev, "st_unregister failed %d\n", rc);

	drv->st_write = NULL;

	return rc;
}

static int nfcwilink_send(struct nci_dev *ndev, struct sk_buff *skb)
{
	struct nfcwilink *drv = nci_get_drvdata(ndev);
	struct nfcwilink_hdr hdr = {NFCWILINK_CHNL, NFCWILINK_OPCODE, 0x0000};
	long len;

	dev_dbg(&drv->pdev->dev, "send entry, len %d\n", skb->len);

	if (!test_bit(NFCWILINK_RUNNING, &drv->flags)) {
		kfree_skb(skb);
		return -EINVAL;
	}

	/* add the ST hdr to the start of the buffer */
	hdr.len = cpu_to_le16(skb->len);
	memcpy(skb_push(skb, NFCWILINK_HDR_LEN), &hdr, NFCWILINK_HDR_LEN);

	/* Insert skb to shared transport layer's transmit queue.
	 * Freeing skb memory is taken care in shared transport layer,
	 * so don't free skb memory here.
	 */
	len = drv->st_write(skb);
	if (len < 0) {
		kfree_skb(skb);
		nfc_err(&drv->pdev->dev, "st_write failed %ld\n", len);
		return -EFAULT;
	}

	return 0;
}

static struct nci_ops nfcwilink_ops = {
	.open = nfcwilink_open,
	.close = nfcwilink_close,
	.send = nfcwilink_send,
};

static int nfcwilink_probe(struct platform_device *pdev)
{
	static struct nfcwilink *drv;
	int rc;
	__u32 protocols;

	drv = devm_kzalloc(&pdev->dev, sizeof(struct nfcwilink), GFP_KERNEL);
	if (!drv) {
		rc = -ENOMEM;
		goto exit;
	}

	drv->pdev = pdev;

	protocols = NFC_PROTO_JEWEL_MASK
		| NFC_PROTO_MIFARE_MASK | NFC_PROTO_FELICA_MASK
		| NFC_PROTO_ISO14443_MASK
		| NFC_PROTO_ISO14443_B_MASK
		| NFC_PROTO_NFC_DEP_MASK;

	drv->ndev = nci_allocate_device(&nfcwilink_ops,
					protocols,
					NFCWILINK_HDR_LEN,
					0);
	if (!drv->ndev) {
		nfc_err(&pdev->dev, "nci_allocate_device failed\n");
		rc = -ENOMEM;
		goto exit;
	}

	nci_set_parent_dev(drv->ndev, &pdev->dev);
	nci_set_drvdata(drv->ndev, drv);

	rc = nci_register_device(drv->ndev);
	if (rc < 0) {
		nfc_err(&pdev->dev, "nci_register_device failed %d\n", rc);
		goto free_dev_exit;
	}

	dev_set_drvdata(&pdev->dev, drv);

	goto exit;

free_dev_exit:
	nci_free_device(drv->ndev);

exit:
	return rc;
}

static int nfcwilink_remove(struct platform_device *pdev)
{
	struct nfcwilink *drv = dev_get_drvdata(&pdev->dev);
	struct nci_dev *ndev;

	if (!drv)
		return -EFAULT;

	ndev = drv->ndev;

	nci_unregister_device(ndev);
	nci_free_device(ndev);

	return 0;
}

static struct platform_driver nfcwilink_driver = {
	.probe = nfcwilink_probe,
	.remove = nfcwilink_remove,
	.driver = {
		.name = "nfcwilink",
	},
};

module_platform_driver(nfcwilink_driver);

/* ------ Module Info ------ */

MODULE_AUTHOR("Ilan Elias <ilane@ti.com>");
MODULE_DESCRIPTION("NFC Driver for TI Shared Transport");
MODULE_LICENSE("GPL");
