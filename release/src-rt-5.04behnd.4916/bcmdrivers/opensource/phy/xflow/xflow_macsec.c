/***********************************************************************
 *
 * Copyright (c) 2021  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2021:DUAL/GPL:standard
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
 *
 ************************************************************************/
#include <linux/netdevice.h>
#include "bcm_macsec.h"
#include "macsec_common.h"
#include "macsec_defs.h"
#include "macsec_dev.h"
#include "macsec_debug.h"
#include "bchp_regs_int.h"
#include "xflow_macsec_cfg_params.h"
#include "xflow_macsec_esw_defs.h"

#define VALID_PORT(p) ((p) < CMBB_FL_MACSEC_MAX_PORT_NUM)

/* Forward declaration */
int fl_macsec_mdo_del_rxsc(struct macsec_context *ctx, void *priv);

#define XPN_MACSEC_IMPLEMENTED 1

int fl_macsec_mdo_dev_open(struct macsec_context *ctx, void *priv)
{
    uint32_t port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id);


    printk("fl_macsec_mdo_dev_open macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    /* Add MAC DA for MGMT packets */
//    xflow_macsec_firelight_mac_addr_control_set(mdev->macsec_unit, 0, xflowMacsecMgmtDstMac0 + mdev->macsec_port,
//        ctx->netdev->dev_addr, MACSEC_CONFIG_EAPOL_MGMT_PKT_TYPE);

    /* Enable the MGMT match rule for the DA/SA pair */
//    xflow_macsec_firelight_port_control_set(mdev->macsec_unit, BCM_XFLOW_MACSEC_DECRYPT, mdev->macsec_port,
//        xflowMacsecPortMgmtPktRulesEnable,
//        XFLOW_MACSEC_MGMT_DEST_MAC_0X0180C200000 | XFLOW_MACSEC_MGMT_DEST_MAC_0X01000CCCCCCC |
//        (XFLOW_MACSEC_MGMT_DEST_MAC0 << mdev->macsec_port) | XFLOW_MACSEC_MGMT_DEST_MAC_ETYPE0 |
//        XFLOW_MACSEC_MGMT_E1_C0);

    xflow_macsec_firelight_port_control_set(mdev->macsec_unit, BCM_XFLOW_MACSEC_DECRYPT, mdev->macsec_port,
        xflowMacsecPortMgmtPktRulesEnable, XFLOW_MACSEC_MGMT_DEST_MAC_ETYPE0 | XFLOW_MACSEC_MGMT_E1_C0);

    mdev->enabled = 1;

    return 0;
}

int fl_macsec_mdo_dev_stop(struct macsec_context *ctx, void *priv)
{
    uint32_t rval, port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];
    soc_ubus_field_t soft_reset_fld = XC_SOFT_RESET_fld;

    if (ctx->prepare)
        return !VALID_PORT(port_id);

    printk("fl_macsec_mdo_dev_stop macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    rval = 1;
    soc_ubus_xlmac_reg_fields32_modify(mdev->macsec_unit, XLMAC_CTRLreg, mdev->macsec_port, &soft_reset_fld, &rval);
    /* Assert soft reset and bypass for the specified MACSEC port */
    soc_ubus_reg32_get(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, &rval);
    soc_ubus_reg_field32_set(mdev->macsec_unit, MACSEC_CTRLreg, &rval, SOFT_RESET_fld, 1);
    soc_ubus_reg_field32_set(mdev->macsec_unit, MACSEC_CTRLreg, &rval, BYPASS_EN_fld, 1);
    soc_ubus_reg32_set(mdev->macsec_unit, MACSEC_CTRLreg, mdev->macsec_port, rval);

    /* Disable the MGMT match rule for the DA/SA pair */
    xflow_macsec_firelight_port_control_set(mdev->macsec_unit, BCM_XFLOW_MACSEC_DECRYPT, mdev->macsec_port,
        xflowMacsecPortMgmtPktRulesEnable, 0);
    /* Remove MAC DA for MGMT packets */
    xflow_macsec_firelight_mac_addr_control_set(mdev->macsec_unit, 0, xflowMacsecMgmtDstMac0 + mdev->macsec_port,
        "\0\0\0\0\0\0", MACSEC_CONFIG_EAPOL_MGMT_PKT_TYPE);

    rval = 0;
    soc_ubus_xlmac_reg_fields32_modify(mdev->macsec_unit, XLMAC_CTRLreg, mdev->macsec_port, &soft_reset_fld, &rval);

    mdev->enabled = 0;
    return 0;
}

int _fl_l2fl_crypto(struct macsec_secy *secy)
{
    switch (secy->key_len) {
    case XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE:
        return secy->xpn ? xflowMacsecCryptoAes128GcmXpn : xflowMacsecCryptoAes128Gcm;
    case XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE:
        return secy->xpn ? xflowMacsecCryptoAes256GcmXpn : xflowMacsecCryptoAes256Gcm;
    default:
        PR_ERR("Unsupported Crypto suite: Key length %d, XPN %d", secy->key_len, secy->xpn);
    }
    return 0;
}

int fl_macsec_mdo_add_secy(struct macsec_context *ctx, void *priv)
{
    int rc;
    uint32_t flag, port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];
    xflow_macsec_secure_chan_info_t sc = {};

    if (ctx->prepare)
        return !VALID_PORT(port_id);


    printk("fl_macsec_mdo_add_secy macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    /* Translate Linux to xflow */
    mdev->egress_sci = cpu_to_be64(ctx->secy->sci);
    mdev->assoc_num = ctx->secy->tx_sc.encoding_sa;
    mdev->include_sci = ctx->secy->tx_sc.send_sci;

    printk("egres_sci %llu assoc_num %d include_sci %d\n", mdev->egress_sci, mdev->assoc_num, mdev->include_sci);
    ctx->secy->port_id = port_id;
    /* Set the SCI value and mask. */
    sc.sci = mdev->egress_sci;
    sc.sci_mask = mdev->egress_sci_mask;

    /* Select the crypto algorithm. */
    sc.crypto = _fl_l2fl_crypto(ctx->secy);

    /* Insert SecTAG after (DA,SA, and any VIDs) */
    sc.sectag_offset = MACSEC_CONFIG_ENCRYPT_SECTAG_OFFSET;
    sc.mtu = ctx->netdev->mtu;
    sc.active_an = mdev->assoc_num;

    if (ctx->secy->tx_sc.encrypt)
    {
        /* Set TCI E bit field */
        sc.tci = (MACSEC_CONFIG_ENCRYPT_TCI_E_BIT_SETTING << 3);

        /* Set TCI C bit field */
        sc.tci |= (MACSEC_CONFIG_ENCRYPT_TCI_C_BIT_SETTING << 2);
    }
    else if (ctx->secy->icv_len != MACSEC_STD_ICV_LEN)
        sc.tci |= (MACSEC_CONFIG_ENCRYPT_TCI_C_BIT_SETTING << 2);

    /* Set current TCI association number */
    sc.tci |= (sc.active_an & 0x3);

    sc.flags = MACSEC_CONFIG_ENCRYPT_POLICY_FLAG_SETTINGS;

    /* Set include_sci field if present */
    if (mdev->include_sci)
    {
        sc.tci |= (0x1 << 5);
        sc.flags |= XFLOW_MACSEC_SECURE_CHAN_INFO_INCLUDE_SCI;
    }

    mdev->chan_id_encrypt = XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(XFLOW_MACSEC_ENCRYPT, mdev->macsec_port);

    printk("fl_macsec_mdo_add_secy mdev->chan_id_encrypt=0x%X\n", mdev->chan_id_encrypt);

    flag = XFLOW_MACSEC_SECURE_CHAN_WITH_ID | XFLOW_MACSEC_ENCRYPT;

    /* Create a new Security Channel for an encrypt flow */
    rc = xflow_macsec_secure_chan_create(mdev->macsec_unit, flag, &sc, 0/*prio*/, &mdev->chan_id_encrypt);
    if (rc != BCM_E_NONE)
        PR_ERR("FAILED! xflow_macsec_secure_chan_create rc = %d\n", rc);
    return 0;
}

int fl_macsec_mdo_upd_secy(struct macsec_context *ctx, void *priv)
{
    int rc = 0;
    uint32_t port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id);

    printk("fl_macsec_mdo_upd_secy mdev->chan_id_encrypt=0x%X\n", mdev->chan_id_encrypt);

    return rc;
}

int fl_macsec_mdo_del_secy(struct macsec_context *ctx, void *priv)
{
    int i, rc = 0;
    uint32_t port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        if (VALID_PORT(port_id) &&
            mdev->chan_id_encrypt)
        {
            return 0;
        }
        else
            return -ENODEV;
    }

    printk("fl_macsec_mdo_del_secy mdev->chan_id_encrypt=0x%X\n", mdev->chan_id_encrypt);

    /* Removing RX SC */
    fl_macsec_mdo_del_rxsc(ctx, priv);    
        
    /* Removing RX SAs */
    for (i = 0; i < XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_ENCRYPT); i++)
    {
        if (mdev->assoc_id_encrypt[i])
        {
            rc = xflow_macsec_secure_assoc_destroy(mdev->macsec_unit, mdev->assoc_id_encrypt[i]);
            if (!rc)
                mdev->assoc_id_encrypt[i] = 0;
            else
                PR_ERR("xflow_macsec_secure_assoc_destroy failed rc=%d\n", rc);
        }
    }

    rc = xflow_macsec_secure_chan_destroy(mdev->macsec_unit, mdev->chan_id_encrypt);
    if (rc)
        PR_ERR("xflow_macsec_secure_chan_destroy failed rc=%d\n", rc);
    else
        mdev->chan_id_encrypt = 0;

    return rc;
}

int fl_macsec_mdo_add_rxsc(struct macsec_context *ctx, void *priv)
{
    int rc;
    uint32_t flag, port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];
    bcm_xflow_macsec_decrypt_policy_info_t policy_info = {};
    bcm_xflow_macsec_decrypt_flow_info_t flow_info = {};
    xflow_macsec_secure_chan_info_t sc = {};

    if (ctx->prepare)
    {
        if (VALID_PORT(port_id))
            if (ctx->secy->n_rx_sc > 1)
                return -EOPNOTSUPP;
            else
                return 0;
        else
            return -ENODEV;
    }

    printk("fl_macsec_mdo_add_rxsc macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    /* Translate Linux to xflow */
    mdev->ingress_sci = cpu_to_be64(ctx->rx_sc->sci);

    /* Create decrypt policy */
//    policy_info.flags = XFLOW_MACSEC_DECRYPT_POLICY_TAGGED_CONTROL_PORT_ENABLE;
//    policy_info.flags = MACSEC_CONFIG_DECRYPT_POLICY_FLAG_SETTINGS;
    policy_info.flags = XFLOW_MACSEC_DECRYPT_POLICY_TAGGED_CONTROL_PORT_ENABLE | 
                        XFLOW_MACSEC_DECRYPT_POLICY_POINT_TO_POINT_ENABLE;

    policy_info.sci = mdev->ingress_sci;

    if (ctx->secy->validate_frames == MACSEC_VALIDATE_STRICT)
        policy_info.tag_validate = xflowMacsecTagValidateStrict;
    else
    {
        policy_info.flags |= XFLOW_MACSEC_DECRYPT_POLICY_UNTAGGED_CONTROL_PORT_ENABLE;
        if (ctx->secy->validate_frames == MACSEC_VALIDATE_CHECK)
            policy_info.tag_validate = xflowMacsecTagValidateCheckICV;
        else
            policy_info.tag_validate = xflowMacsecTagValidateCheckNone;
    }
    policy_info.sectag_offset = MACSEC_CONFIG_DECRYPT_SECTAG_OFFSET;
    policy_info.mtu = ctx->netdev->mtu;

    /* 
     * Create a decryption policy.
     * In this call the decrypt_policy ID is converted to a decrypt policy hardware
     * ID. Then, the SUB_PORT_POLICY_TABLE is programmed with the policy settings,
     * at the policy hardware ID index.
     */
    mdev->policy_id_decrypt = XFLOW_MACSEC_POLICY_ID_CREATE(xflowMacsecIdTypePolicy, mdev->macsec_port);
    rc = xflow_macsec_policy_create(mdev->macsec_unit, (XFLOW_MACSEC_POLICY_WITH_ID | XFLOW_MACSEC_DECRYPT), &policy_info, &mdev->policy_id_decrypt);
    if (rc != BCM_E_NONE)
    {
        PR_ERR("FAILED! xflow_macsec_policy_create rc = %d\n", rc);
        return 0;
    }

    /* Create decrypt flow */
    flow_info.policy_id = mdev->policy_id_decrypt;
    flow_info.frame_type = xflowMacsecFlowFrameAny;

    mdev->flow_id_decrypt = BCM_XFLOW_MACSEC_DECRYPT_FLOW_ID_CREATE(mdev->macsec_port);
    rc = xflow_macsec_flow_create(mdev->macsec_unit, (XFLOW_MACSEC_DECRYPT | XFLOW_MACSEC_FLOW_WITH_ID), &flow_info, 0, 
                                  &mdev->flow_id_decrypt);

    if (rc != BCM_E_NONE)
    {
        PR_ERR("FAILED! xflow_macsec_flow_create rc = %d\n", rc);
        return 0;
    }
    /* Set the SCI value and mask. */
    sc.sci = mdev->ingress_sci;
    sc.sci_mask = mdev->ingress_sci_mask;

    /* Select the crypto algorithm. */
    sc.crypto = _fl_l2fl_crypto(ctx->secy);

    sc.an_control = xflowMacsecSecureAssocAnNormal;
    sc.policy_id = mdev->policy_id_decrypt;

    /* Allow both, data + management pkts */
    sc.flags = XFLOW_MACSEC_SECURE_CHAN_INFO_CONTROLLED_PORT;
    
    if (mdev->include_sci)
        sc.flags |= XFLOW_MACSEC_SECURE_CHAN_INFO_INCLUDE_SCI;

    mdev->chan_id_decrypt = XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(XFLOW_MACSEC_DECRYPT, mdev->macsec_port);
    flag = XFLOW_MACSEC_SECURE_CHAN_WITH_ID | XFLOW_MACSEC_DECRYPT;

    /* Create a new Security Channel for an decrypt flow */
    rc = xflow_macsec_secure_chan_create(mdev->macsec_unit, flag, &sc, 0/*prio*/, &mdev->chan_id_decrypt);
    if (rc != BCM_E_NONE)
    {
        PR_ERR("FAILED! xflow_macsec_secure_chan_create rc = %d\n", rc);
        return 0;
    }

    /* Enable secure channel */
    if (ctx->rx_sc->active)
    {
        rc = xflow_macsec_secure_chan_enable_set(mdev->macsec_unit, mdev->chan_id_decrypt, 1);
        if (rc != BCM_E_NONE)
            PR_ERR("FAILED! xflow_macsec_secure_chan_enable_set rc = %d\n", rc);
        rc = xflow_macsec_flow_enable_set(mdev->macsec_unit, mdev->flow_id_decrypt, 1);
        if (rc != BCM_E_NONE)
            PR_ERR("FAILED! xflow_macsec_flow_enable_set rc = %d\n", rc);
    }

    return 0;
}

int fl_macsec_mdo_upd_rxsc(struct macsec_context *ctx, void *priv)
{
    int rc = 0;
    uint32_t port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        if (VALID_PORT(port_id))
            return 0;
        else
            return -ENODEV;
    }

    printk("fl_macsec_mdo_upd_rxsc macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    return rc;
}

int fl_macsec_mdo_del_rxsc(struct macsec_context *ctx, void *priv)
{
    int rc, i;
    uint32_t port_id = (uintptr_t)priv;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        if (VALID_PORT(port_id) &&
            mdev->policy_id_decrypt &&
            mdev->flow_id_decrypt &&
            mdev->chan_id_decrypt)
        {
            return 0;
        }
        else
            return -ENODEV;
    }

    printk("fl_macsec_mdo_del_rxsc macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    /* Removing RX SAs */
    for (i = 0; i < XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_DECRYPT); i++)
    {
        if (mdev->assoc_id_decrypt[i])
        {
            rc = xflow_macsec_secure_assoc_destroy(mdev->macsec_unit, mdev->assoc_id_decrypt[i]);
            if (!rc)
                mdev->assoc_id_decrypt[i] = 0;
            else
                PR_ERR("xflow_macsec_secure_assoc_destroy failed rc=%d\n", rc);
        }
    }
    rc = xflow_macsec_policy_destroy(mdev->macsec_unit, mdev->policy_id_decrypt);
    if (rc)
    {
        PR_ERR("xflow_macsec_policy_destroy failed rc=%d\n", rc);
        goto exit;
    }
    mdev->policy_id_decrypt = 0;

    rc = xflow_macsec_flow_destroy(mdev->macsec_unit, mdev->flow_id_decrypt);
    if (rc)
    {
        PR_ERR("xflow_macsec_flow_destroy failed rc=%d\n", rc);
        goto exit;
    }
    mdev->flow_id_decrypt = 0;

    rc = xflow_macsec_secure_chan_destroy(mdev->macsec_unit, mdev->chan_id_decrypt);
    if (rc)
    {
        PR_ERR("xflow_macsec_secure_chan_destroy failed rc=%d\n", rc);
        goto exit;
    }
    mdev->chan_id_decrypt = 0;

exit:
    return rc;
}

int fl_macsec_mdo_add_rxsa(struct macsec_context *ctx, void *priv)
{
    int rc, i;
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    uint8_t *key;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];
    xflow_macsec_secure_assoc_info_t sa = {};

    if (ctx->prepare)
    {
        return !VALID_PORT(port_id) ||
            sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_DECRYPT);
    }

    printk("fl_macsec_mdo_add_rxsa macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    sa.an = sa_num;
    sa.enable = ctx->sa.rx_sa->active;
    sa.next_pkt_num = ctx->sa.rx_sa->next_pn_halves.lower;
    if (ctx->secy->xpn
#ifndef XPN_MACSEC_IMPLEMENTED
        || mdev->ssci_decrypt
#endif
       )
    {
        sa.next_pkt_num_upper = ctx->sa.rx_sa->next_pn_halves.upper;
        printk("RX SA PN lower 0x%X upper 0x%X ssci 0x%X\n", sa.next_pkt_num, sa.next_pkt_num_upper, ctx->sa.rx_sa->ssci);
#ifdef XPN_MACSEC_IMPLEMENTED
        for(i = 0; i < 4; i++)
            ((unsigned char *)&sa.ssci)[i] = ((unsigned char *)&ctx->sa.rx_sa->ssci)[3 - i];        
//        mdev->ssci_decrypt = sa.ssci = ctx->sa.rx_sa->ssci;
        mdev->ssci_decrypt = sa.ssci;
        for (i = 0; i < MACSEC_SALT_LEN; i++)
            mdev->salt_decrypt[i] = sa.salt[i] = ctx->sa.rx_sa->key.salt.bytes[MACSEC_SALT_LEN - 1 - i];
#else
        sa.ssci = mdev->ssci_decrypt;
        for (i = 0; i < MACSEC_SALT_LEN; i++)
            sa.salt[i] = mdev->salt_decrypt[MACSEC_SALT_LEN - 1 - i];
#endif
    }
    sa.flags = MACSEC_CONFIG_SA_NEXT_PKT_NUM_FLAG;

    if (ctx->secy->key_len == XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE)
        key = sa.aes.key;
    else
        key = sa.aes_256.key;
    for (i = 0; i < ctx->secy->key_len; i++)
        key[i] = ctx->sa.key[ctx->secy->key_len - 1 - i];
    rc = xflow_macsec_secure_assoc_create(mdev->macsec_unit, 0, mdev->chan_id_decrypt, &sa, &mdev->assoc_id_decrypt[sa_num]);
    if (rc != BCM_E_NONE)
        PR_ERR("xflow_macsec_secure_assoc_create(): FAILED! rc = %d\n", rc);

    return 0;
}

int fl_macsec_mdo_upd_rxsa(struct macsec_context *ctx, void *priv)
{
    int rc = 0;
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id) || sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_DECRYPT);

    printk("fl_macsec_mdo_upd_rxsa macsec_unit=%d, macsec_port=%d, port_id=%d, sa_num=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id, sa_num);

    return rc;
}

int fl_macsec_mdo_del_rxsa(struct macsec_context *ctx, void *priv)
{
    int rc = -1;
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        return !VALID_PORT(port_id) ||
            sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_DECRYPT);
    }

    printk("fl_macsec_mdo_del_rxsa macsec_unit=%d, macsec_port=%d, port_id=%d, sa_num=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id, sa_num);

    rc = xflow_macsec_secure_assoc_destroy(mdev->macsec_unit, mdev->assoc_id_decrypt[sa_num]);

    if (!rc)
        mdev->assoc_id_decrypt[sa_num] = 0;
    else
        PR_ERR("xflow_macsec_secure_assoc_destroy failed rc=%d\n", rc);

    return rc;
}

int fl_macsec_mdo_add_txsa(struct macsec_context *ctx, void *priv)
{
    int rc, i;
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    uint8_t *key;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];
    xflow_macsec_secure_assoc_info_t sa = {};

    if (ctx->prepare)
    {
        return !VALID_PORT(port_id) ||
            sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_ENCRYPT);
    }

    printk("fl_macsec_mdo_add_txsa macsec_unit=%d, macsec_port=%d, mdev->chan_id_encrypt=%d, port_id=%d\n", 
        mdev->macsec_unit, mdev->macsec_port, mdev->chan_id_encrypt, port_id);

    sa.an = sa_num;
    sa.enable = ctx->sa.tx_sa->active;
    sa.next_pkt_num = ctx->sa.tx_sa->next_pn_halves.lower;
    sa.flags = MACSEC_CONFIG_SA_NEXT_PKT_NUM_FLAG;
    if (ctx->secy->xpn
#ifndef XPN_MACSEC_IMPLEMENTED
        || mdev->ssci_encrypt
#endif
       )
    {
        sa.next_pkt_num_upper = ctx->sa.tx_sa->next_pn_halves.upper;
#ifdef XPN_MACSEC_IMPLEMENTED
        printk("xpn %u\n", ctx->sa.tx_sa->ssci);
        for(i = 0; i < 4; i++)
            ((unsigned char *)&sa.ssci)[i] = ((unsigned char *)&ctx->sa.tx_sa->ssci)[3 - i];        
//        mdev->ssci_decrypt = sa.ssci = ctx->sa.rx_sa->ssci;
        mdev->ssci_decrypt = sa.ssci;

        for (i = 0; i < MACSEC_SALT_LEN; i++)
             mdev->salt_encrypt[i] = sa.salt[i] = ctx->sa.tx_sa->key.salt.bytes[MACSEC_SALT_LEN - 1 - i];
#else
        sa.ssci = mdev->ssci_encrypt;
        for (i = 0; i < MACSEC_SALT_LEN; i++)
            sa.salt[i] = mdev->salt_encrypt[MACSEC_SALT_LEN - 1 - i];
#endif
    }
    if (ctx->secy->key_len == XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE)
        key = sa.aes.key;
    else
        key = sa.aes_256.key;
    for (i = 0; i < ctx->secy->key_len; i++)
        key[i] = ctx->sa.key[ctx->secy->key_len - 1 - i];

    rc = xflow_macsec_secure_assoc_create(mdev->macsec_unit, 0, mdev->chan_id_encrypt, &sa, &mdev->assoc_id_encrypt[sa_num]);
    if (rc != BCM_E_NONE)
        PR_ERR("xflow_macsec_secure_assoc_create(): FAILED! rc = %d\n", rc);

    return 0;
}

int fl_macsec_mdo_upd_txsa(struct macsec_context *ctx, void *priv)
{
    int rc = 0;
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        return !VALID_PORT(port_id) ||
            sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(FL_UNIT, XFLOW_MACSEC_ENCRYPT);
    }

    printk("fl_macsec_mdo_upd_txsa macsec_unit=%d, macsec_port=%d, port_id=%d, sa_num=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id, sa_num);

    return rc;
}

int fl_macsec_mdo_del_txsa(struct macsec_context *ctx, void *priv)
{
    int rc = -1;
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        return !VALID_PORT(port_id) ||
            sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(FL_UNIT, XFLOW_MACSEC_ENCRYPT);
    }

    printk("fl_macsec_mdo_del_txsa macsec_unit=%d, macsec_port=%d, port_id=%d, sa_num=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id, sa_num);

    rc = xflow_macsec_secure_assoc_destroy(mdev->macsec_unit, mdev->assoc_id_encrypt[sa_num]);

    if (!rc)
        mdev->assoc_id_encrypt[sa_num] = 0;
    else
        PR_ERR("xflow_macsec_secure_assoc_destroy failed rc=%d\n", rc);

    return rc;
}

int fl_macsec_mdo_get_dev_stats(struct macsec_context *ctx, void *priv)
{
    uint32_t port_id = (uintptr_t)priv;
    uint64_t val = 0;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id);

    printk("fl_macsec_mdo_get_dev_stats macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    _xflow_macsec_counters_collect(mdev->macsec_unit);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port_id), xflowMacsecSecyStatsTxUntaggedPkts, &val);
    ctx->stats.dev_stats->OutPktsUntagged = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port_id), xflowMacsecSecyStatsRxUntaggedPkts, &val);
    ctx->stats.dev_stats->InPktsUntagged = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_ENCRYPT, port_id), xflowMacsecSecyStatsTxTooLongPkts, &val);
    ctx->stats.dev_stats->OutPktsTooLong = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port_id), xflowMacsecSecyStatsRxNoTagPkts, &val);
    ctx->stats.dev_stats->InPktsNoTag = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port_id), xflowMacsecSecyStatsRxBadTagPkts, &val);
    ctx->stats.dev_stats->InPktsBadTag = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port_id), xflowMacsecSecyStatsRxUnknownSCIPkts, &val);
    ctx->stats.dev_stats->InPktsUnknownSCI = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port_id), xflowMacsecSecyStatsRxNoSCIPkts, &val);
    ctx->stats.dev_stats->InPktsNoSCI = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, XFLOW_MACSEC_SUBPORT_ID_CREATE(XFLOW_MACSEC_DECRYPT, port_id), xflowMacsecSecyStatsRxOverrunPkts, &val);
    ctx->stats.dev_stats->InPktsOverrun = val;
#if 0
    debug_dump_table(mdev, "ISEC_PORT_COUNTERS", 0);
    debug_dump_table(mdev, "ISEC_PORT_COUNTERS", 1);
    debug_dump_table(mdev, "ISEC_PORT_COUNTERS", 2);
    debug_dump_table(mdev, "ISEC_SCTCAM_HIT_COUNT", 0);
    debug_dump_table(mdev, "ISEC_SCTCAM_HIT_COUNT", 1);
    debug_dump_table(mdev, "ISEC_SCTCAM_HIT_COUNT", 2);
    debug_dump_table(mdev, "ISEC_SPTCAM_HIT_COUNT", 0);
    debug_dump_table(mdev, "ISEC_SPTCAM_HIT_COUNT", 1);
    debug_dump_table(mdev, "ISEC_SPTCAM_HIT_COUNT", 2);
//    xflow_macsec_debug_mode_handler(mdev, 1, "isec", port_id);
//    xflow_macsec_debug_mode_handler(mdev, 1, "esec", port_id);
//    xflow_macsec_debug_mode_handler(mdev, 2, "isec", port_id);
//    xflow_macsec_debug_mode_handler(mdev, 2, "esec", port_id);
#endif
    return 0;
}

int fl_macsec_mdo_get_tx_sc_stats(struct macsec_context *ctx, void *priv)
{
    uint32_t port_id = (uintptr_t)priv;
    uint64_t val = 0;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id);


    printk("fl_macsec_mdo_get_tx_sc_stats macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    _xflow_macsec_counters_collect(mdev->macsec_unit);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsProtectedPkts, &val);
    ctx->stats.tx_sc_stats->OutPktsProtected    = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsEncryptedPkts, &val);
    ctx->stats.tx_sc_stats->OutPktsEncrypted    = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsOctetsProtected, &val);
    ctx->stats.tx_sc_stats->OutOctetsProtected  = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_encrypt, xflowMacsecSecyTxSCStatsOctetsEncrypted, &val);
    ctx->stats.tx_sc_stats->OutOctetsEncrypted  = val;

    return 0;
}

int fl_macsec_mdo_get_tx_sa_stats(struct macsec_context *ctx, void *priv)
{
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    uint64_t val = 0;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
    {
        return !VALID_PORT(port_id) ||
            sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_ENCRYPT);
    }

    printk("fl_macsec_mdo_get_tx_sa_stats macsec_unit=%d, macsec_port=%d, port_id=%d sa_num=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id, sa_num);

    _xflow_macsec_counters_collect(mdev->macsec_unit);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_encrypt[sa_num], xflowMacsecSecyTxSAStatsProtectedPkts, &val);
    ctx->stats.tx_sa_stats->OutPktsProtected    = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_encrypt[sa_num], xflowMacsecSecyTxSAStatsEncryptedPkts, &val);
    ctx->stats.tx_sa_stats->OutPktsEncrypted    = val;

    return 0;
}

int fl_macsec_mdo_get_rx_sc_stats(struct macsec_context *ctx, void *priv)
{
    uint32_t port_id = (uintptr_t)priv;
    uint64_t val = 0;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id);


    printk("fl_macsec_mdo_get_rx_sc_stats macsec_unit=%d, macsec_port=%d, port_id=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id);

    _xflow_macsec_counters_collect(mdev->macsec_unit);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOctetsValidated, &val);
    ctx->stats.rx_sc_stats->InOctetsValidated   = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOctetsDecrypted, &val);
    ctx->stats.rx_sc_stats->InOctetsDecrypted   = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsUncheckedPkts, &val);
    ctx->stats.rx_sc_stats->InPktsUnchecked     = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsDelayedPkts, &val);
    ctx->stats.rx_sc_stats->InPktsDelayed       = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsOKPkts, &val);
    ctx->stats.rx_sc_stats->InPktsOK            = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsInvalidPkts, &val);
    ctx->stats.rx_sc_stats->InPktsInvalid       = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsLatePkts, &val);
    ctx->stats.rx_sc_stats->InPktsLate          = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsNotValidPkts, &val);
    ctx->stats.rx_sc_stats->InPktsNotValid      = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsNotUsingSAPkts, &val);
    ctx->stats.rx_sc_stats->InPktsNotUsingSA    = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->chan_id_decrypt, xflowMacsecSecyRxSCStatsUnusedSAPkts, &val);
    ctx->stats.rx_sc_stats->InPktsUnusedSA      = val;

    return 0;
}

int fl_macsec_mdo_get_rx_sa_stats(struct macsec_context *ctx, void *priv)
{
    uint32_t port_id = (uintptr_t)priv, sa_num = ctx->sa.assoc_num;
    uint64_t val = 0;
    macsec_dev_t *mdev = &msec_devs[FL_UNIT][port_id];

    if (ctx->prepare)
        return !VALID_PORT(port_id) || sa_num >= XFLOW_MACSEC_NUM_SA_PER_SC(mdev->macsec_unit, XFLOW_MACSEC_DECRYPT);

    printk("fl_macsec_mdo_get_rx_sa_stats macsec_unit=%d, macsec_port=%d, port_id=%d sa_num=%d\n", mdev->macsec_unit, mdev->macsec_port, port_id, sa_num);

    _xflow_macsec_counters_collect(mdev->macsec_unit);
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[sa_num], xflowMacsecSecyRxSAStatsOKPkts, &val);
    ctx->stats.rx_sa_stats->InPktsOK            = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[sa_num], xflowMacsecSecyRxSAStatsInvalidPkts, &val);
    ctx->stats.rx_sa_stats->InPktsInvalid       = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[sa_num], xflowMacsecSecyRxSAStatsNotValidPkts, &val);
    ctx->stats.rx_sa_stats->InPktsNotValid      = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[sa_num], xflowMacsecSecyRxSAStatsNotUsingSAPkts, &val);
    ctx->stats.rx_sa_stats->InPktsNotUsingSA    = val;
    val = 0;
    xflow_macsec_stat_get(mdev->macsec_unit, 0, mdev->assoc_id_decrypt[sa_num], xflowMacsecSecyRxSAStatsUnusedSAPkts, &val);
    ctx->stats.rx_sa_stats->InPktsUnusedSA      = val;

    return 0;
}

struct bcm_macsec_ops fl_macsec_ops = {
    /* Device wide */
    .bcm_mdo_dev_open = fl_macsec_mdo_dev_open,
    .bcm_mdo_dev_stop = fl_macsec_mdo_dev_stop,
    /* SecY */
    .bcm_mdo_add_secy = fl_macsec_mdo_add_secy,
    .bcm_mdo_upd_secy = fl_macsec_mdo_upd_secy,
    .bcm_mdo_del_secy = fl_macsec_mdo_del_secy,
    /* Security channels */
    .bcm_mdo_add_rxsc = fl_macsec_mdo_add_rxsc,
    .bcm_mdo_upd_rxsc = fl_macsec_mdo_upd_rxsc,
    .bcm_mdo_del_rxsc = fl_macsec_mdo_del_rxsc,
    /* Security associations */
    .bcm_mdo_add_rxsa = fl_macsec_mdo_add_rxsa,
    .bcm_mdo_upd_rxsa = fl_macsec_mdo_upd_rxsa,
    .bcm_mdo_del_rxsa = fl_macsec_mdo_del_rxsa,
    .bcm_mdo_add_txsa = fl_macsec_mdo_add_txsa,
    .bcm_mdo_upd_txsa = fl_macsec_mdo_upd_txsa,
    .bcm_mdo_del_txsa = fl_macsec_mdo_del_txsa,
    /* Statistics */
    .bcm_mdo_get_dev_stats = fl_macsec_mdo_get_dev_stats,
    .bcm_mdo_get_tx_sc_stats = fl_macsec_mdo_get_tx_sc_stats,
    .bcm_mdo_get_tx_sa_stats = fl_macsec_mdo_get_tx_sa_stats,
    .bcm_mdo_get_rx_sc_stats = fl_macsec_mdo_get_rx_sc_stats,
    .bcm_mdo_get_rx_sa_stats = fl_macsec_mdo_get_rx_sa_stats,
};

struct bcm_macsec_ops *bcm_macsec_get_ops(void)
{
    return &fl_macsec_ops;
}
EXPORT_SYMBOL(bcm_macsec_get_ops);
