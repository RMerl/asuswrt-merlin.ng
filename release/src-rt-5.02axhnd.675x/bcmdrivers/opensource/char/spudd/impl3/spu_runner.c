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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
#include <linux/kthread.h>
#include <linux/bcm_realtime.h>
#include <crypto/algapi.h>
#include <crypto/authenc.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/md5.h>
#include <rdpa_api.h>
#include <rdpa_ag_ipsec.h>
#include <rdpa_ipsec_helper.h>
#include <rdpa_mw_cpu_queue_ids.h>
#include <bcmspudrv.h>
#include <bcm_mm.h>
#include "spu.h"
#include "spu_runner.h"

static bdmf_object_handle ipsec_obj        = NULL;
static bdmf_object_handle spu_rdpa_cpu_obj = NULL;

static void spu_runner_recycle_handler(pNBuff_t pNBuf, uint32_t context, uint32_t free_flag)
{
   struct spu_trans_req *pTransReq;
   void                 *pBuf = PNBUFF_2_PBUF(pNBuf);
   int                   index;
   
   if (IS_SKBUFF_PTR(pNBuf))
   {
      if ( SKB_RECYCLE_NOFREE != free_flag )
      {
         SPU_TRACE(("Received unepxected recycle call\n"));
         /* cannot rely on context data so return */
         return;
      }
   }

   index = context - SPU_TRANS_REQ_BASE_IDX;
   if ( (index < 0) || (index >= SPU_NUM_TRANS_REQ) )
   {
      /* context is invalid */
      SPU_TRACE(("Received invalid context\n"));
      return;
   }

   pTransReq = spuinfo->transReqBase;
   pTransReq += index;
   if (SPU_DIRECTION_US == pTransReq->pSpuCtx->direction)
   {
       spuinfo->stats.encDrops++;
   }
   else
   {
       spuinfo->stats.decDrops++;
   }

   if (IS_SKBUFF_PTR(pNBuf))
   {
      struct sk_buff *skb = (struct sk_buff *)pBuf;
      skb->recycle_hook    = pTransReq->recycle_hook;
      skb->recycle_context = pTransReq->recycle_context;
      skb->recycle_flags   = pTransReq->recycle_flags;

      /* pass packet to the kernel so it can free its
         private context data */
      pTransReq->err = -ENOBUFS;
      pTransReq->callback(pTransReq);
   }
   else
   {
      FkBuff_t *fkb = (FkBuff_t *)pBuf;
      fkb->recycle_hook    = pTransReq->recycle_hook;
      fkb->recycle_context = pTransReq->recycle_context;

      /* call original recycle hook to free the fkb 
         fkb is master fkb with single reference
         no need to pass packet to kernel */
      fkb->recycle_hook(FKBUFF_2_PNBUFF(fkb), fkb->recycle_context, 0);
   }

   spu_free_trans_req(pTransReq);
   /* pTransReq is invalid at this point */
}

static void spu_runner_recycle_set(pNBuff_t pNBuf, struct spu_trans_req *pTransReq)
{
   void * pBuf = PNBUFF_2_PBUF(pNBuf);

   if (IS_SKBUFF_PTR(pNBuf))
   {
      struct sk_buff *skb = (struct sk_buff *)pBuf;

      pTransReq->recycle_hook    = skb->recycle_hook;
      pTransReq->recycle_context = skb->recycle_context;
      pTransReq->recycle_flags   = skb->recycle_flags;

      skb->recycle_hook    = (RecycleFuncP)spu_runner_recycle_handler;
      skb->recycle_context = pTransReq->index;
      skb->recycle_flags   = SKB_RECYCLE_NOFREE;
   }
   else
   {
      FkBuff_t *fkb = (FkBuff_t *)pBuf;

      pTransReq->recycle_hook    = fkb->recycle_hook;
      pTransReq->recycle_context = fkb->recycle_context;

      fkb->recycle_hook    = (RecycleFuncP)spu_runner_recycle_handler;
      fkb->recycle_context = pTransReq->index;
   }
}

struct spu_trans_req *spu_runner_recycle_restore(pNBuff_t pNBuf)
{
   struct spu_trans_req *pTransReq;
   void * pBuf = PNBUFF_2_PBUF(pNBuf);

   if (IS_SKBUFF_PTR(pNBuf))
   {
      struct sk_buff *skb = (struct sk_buff *)pBuf;

      pTransReq  = spuinfo->transReqBase;
      pTransReq += (skb->recycle_context - SPU_TRANS_REQ_BASE_IDX);

      skb->recycle_hook    = pTransReq->recycle_hook;
      skb->recycle_context = pTransReq->recycle_context;
      skb->recycle_flags   = pTransReq->recycle_flags;
   }
   else
   {
      FkBuff_t *fkb = (FkBuff_t *)pBuf;

      pTransReq  = spuinfo->transReqBase;
      pTransReq += (fkb->recycle_context - SPU_TRANS_REQ_BASE_IDX);

      fkb->recycle_hook    = pTransReq->recycle_hook;
      fkb->recycle_context = pTransReq->recycle_context;
   }
   return pTransReq;
}

int spu_runner_descr_key_validate(int enckeylen, int authkeylen)
{
    if (enckeylen > IPSEC_CRYPT_KEY_SIZE_MAX)
    {
        return -1;
    }

    if (authkeylen > IPSEC_AUTH_KEY_SIZE_MAX)
    {
        return -1;
    }

    return 0;
}

struct spu_desc *spu_runner_descr_get(void)
{
    struct spu_desc *pDescr;

    spin_lock_bh(&spuinfo->spuListLock);
    if ( list_empty(&spuinfo->descList) )
    {
       SPU_TRACE(("spu_cra_init: exhausted descriptors\n"));
       spin_unlock_bh(&spuinfo->spuListLock);
       return NULL;
    }
    pDescr = list_first_entry(&spuinfo->descList, struct spu_desc, entry);
    list_del(&pDescr->entry);
    spin_unlock_bh(&spuinfo->spuListLock);

    return pDescr;
}

void spu_runner_descr_put(struct spu_desc *pDescr)
{
    spin_lock_bh(&spuinfo->spuListLock);
    list_add_tail(&pDescr->entry, &spuinfo->descList);
    spin_unlock_bh(&spuinfo->spuListLock);
}

int spu_runner_descr_config(struct spu_ctx *pCtx)
{
    rdpa_ipsec_auth_alg_e   auth_alg;
    rdpa_ipsec_crypt_mech_e crypt_mech;
    rdpa_ipsec_crypt_alg_e  crypt_alg;
    uint32_t                ekey_len;
    uint32_t                akey_len;
    rdpa_sa_desc_t         *pSaDescr;
    int                     rc;

    pSaDescr = &pCtx->pDescr->sa_descr;
    ekey_len = (pCtx->descAlg >> BCM_DESC_ENCR_KEYLEN_SHIFT) & BCM_DESC_ENCR_KEYLEN_MASK;
    akey_len = (pCtx->descAlg >> BCM_DESC_AUTH_KEYLEN_SHIFT) & BCM_DESC_AUTH_KEYLEN_MASK;
    if ((SPU_DIRECTION_DS == pCtx->direction) && (pCtx->descAlg & BCM_DESC_ENCR_ALG_AES))
    {
       memcpy (pSaDescr->crypt_key, &pCtx->decrypt_key[0], ekey_len);
    }
    else
    {
       memcpy (pSaDescr->crypt_key, &pCtx->encrypt_key[0], ekey_len);
    }
    memcpy (pSaDescr->auth_key, &pCtx->auth_key[0], akey_len);

    crypt_mech = rdpa_crypt_mech_cbc;
    if (pCtx->descAlg & BCM_DESC_AUTH_ALG_SHA1) {
        auth_alg = rdpa_auth_alg_sha1;
    }
    else if (pCtx->descAlg & BCM_DESC_AUTH_ALG_SHA256) {
        auth_alg = rdpa_auth_alg_sha2;
    }
    else if (pCtx->descAlg & BCM_DESC_AUTH_ALG_MD5) {
        auth_alg = rdpa_auth_alg_md5;
    }
    else {
        return -1;
    }

    if (pCtx->descAlg & BCM_DESC_ENCR_ALG_DES) {
        crypt_alg = rdpa_crypt_alg_des;
    }
    else if (pCtx->descAlg & BCM_DESC_ENCR_ALG_3DES) {
        crypt_alg = rdpa_crypt_alg_3des;
    }
    else if (pCtx->descAlg & BCM_DESC_ENCR_ALG_AES) {
        if (AES_KEYSIZE_128 == ekey_len) {
            crypt_alg = rdpa_crypt_alg_aes_128;
        }
        else if (AES_KEYSIZE_192 == ekey_len) {
            crypt_alg = rdpa_crypt_alg_aes_192;
        }
        else if (AES_KEYSIZE_256 == ekey_len) {
            crypt_alg = rdpa_crypt_alg_aes_256;
        }
        else {
            return -1;
        }
    }
    else if (pCtx->descAlg & BCM_DESC_ENCR_ALG_NULL) {
        crypt_mech = rdpa_crypt_mech_plain;
        crypt_alg = rdpa_crypt_alg_bypass;
    }
    else 
    {
        return -1;
    }

    rc = rdpa_ipsec_sa_desc_setting((pCtx->direction == SPU_DIRECTION_US) ? rdpa_dir_us : rdpa_dir_ds, 
                                    auth_alg,
                                    crypt_mech,
                                    crypt_alg,
                                    pCtx->next_hdr,
                                    &pSaDescr->auth_config,
                                    &pSaDescr->crypt_config,
                                    &pSaDescr->crypt_config2);
    if ( 0 == rc )
    {
       cache_flush_len((void *)&pSaDescr->auth_config,
                       sizeof(pSaDescr->auth_config) +
                       sizeof(pSaDescr->crypt_config) +
                       sizeof(pSaDescr->crypt_config2) +
                       IPSEC_AUTH_KEY_SIZE_MAX +
                       IPSEC_CRYPT_KEY_SIZE_MAX);
    }
    pSaDescr->spi = 0;
    return rc;
}

int spu_runner_process_ipsec(struct spu_trans_req *pReq, pNBuff_t pNBuf, uint32_t offset)
{
    int  ret;

    spu_runner_recycle_set(pNBuf, pReq);
    ret = rdpa_cpu_tx_ipsec_offload((bdmf_sysb)pNBuf,
                                    (pReq->pSpuCtx->direction == SPU_DIRECTION_US) ? rdpa_dir_us : rdpa_dir_ds,
                                    offset,
                                    pReq->pSpuCtx->pDescr->index,
                                    pReq->pSpuCtx->update,
                                    CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);
    if ( ret != 0 )
    {
        spu_runner_recycle_restore(pNBuf);
    }

    return ret;
}

static int spu_runner_rxq_thread(void *arg)
{
    int                   rc;
    rdpa_cpu_rx_info_t    info = {};
    pNBuff_t              pNBuf;
    struct spu_trans_req *pTransReq;
    int                   count;
    unsigned char        *pdata;
    int                   nbuflen;

    /* Main task loop */
    while (1)
    {
        /* Wait to be woken up by received packets */
        wait_event_interruptible(spuinfo->rx_thread_wqh, spuinfo->rx_work_avail || kthread_should_stop());

        /* Abort if we were woken up to terminate */
        if (kthread_should_stop())
        {
           SPU_TRACE(("kthread_should_stop detected on spu-rx\n"));
           break;
        }

        count = 0;
        while ( count < SPU_RX_BUDGET )
        {
            pNBuf = NULL;
            memset(&info, 0, sizeof(rdpa_cpu_rx_info_t));
            rc = rdpa_cpu_loopback_packet_get(rdpa_cpu_loopback_ipsec, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID,
                                              (bdmf_sysb *)&pNBuf, &info);
            if ( BDMF_ERR_NO_MORE == rc )
            {
                break;
            }
            count++;

            if ( (rc == 0) && (NULL != pNBuf) && (info.reason == rdpa_cpu_rx_reason_ipsec) )
            {
                /* data has been modified by IPSec engine, invalidate its cache lines */
                if ( IS_SKBUFF_PTR(pNBuf) ) {
                      struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
                      pdata = skb->data;
                      nbuflen = skb->len;
                } else {
                      FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuf);
                      pdata = fkb->data;
                      nbuflen = fkb->len;
                }
                cache_invalidate_len(pdata, nbuflen);

                pTransReq = spu_runner_recycle_restore(pNBuf);
                if ( IPSEC_ERROR(info.reason_data) )
                {
                    if ( SPU_DIRECTION_US == pTransReq->pSpuCtx->direction )
                    {
                        spuinfo->stats.encErrors++;
                    }
                    else
                    {
                        spuinfo->stats.decErrors++;
                    }
                    pTransReq->err = -EBADMSG;
                    SPU_TRACE(("Receive error %d\n", IPSEC_ERROR(info.reason_data)));
                }
                else
                {
                    pTransReq->err = 0;
                }
                pTransReq->callback(pTransReq);
                spu_free_trans_req(pTransReq);
            }
            else
            {
                SPU_TRACE(("spu_runner_rxq_thread: error in rdpa_cpu_loopback_packet_get()"
                           "(%d:%d)\n", rc, info.reason));
                if ( NULL != pNBuf )
                    nbuff_free(pNBuf);
            }

            /* budget was consumed */
            if ( SPU_RX_BUDGET == count )
            {
               /* Yield CPU to allow others to have a chance, then continue to
                  top of loop for more work.  */
               if ((current->policy == SCHED_FIFO) || (current->policy == SCHED_RR))
               {
                  count = 0;
                  cond_resched();
               }
            }
        }

        spuinfo->rx_work_avail = 0;
        rdpa_cpu_int_enable(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);
    }

    return 0;
}

/*---------------------------------------------------------------------------
 * void cpu_rxq_isr(int queue_id)
 * Description:
 *    spu receive interrupt service routine
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void spu_runner_rx_queue_isr(int queue_id)
{
    /* disable and clear interrupts */
    rdpa_cpu_int_disable(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);
    rdpa_cpu_int_clear(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);
    
    if (0 == spuinfo->rx_work_avail)
    {
        spuinfo->rx_work_avail = 1;
        wake_up_interruptible(&spuinfo->rx_thread_wqh);
    }
}  /* spu_runner_rx_queue_isr() */

/*---------------------------------------------------------------------------
 * int cfg_cpu_rx_queue(int queue_id, UINT32_t queue_size, void *rx_isr)
 * Description:
 *    Set cpu rx queue configuration including cpu rx queue isr.
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
static int spu_runner_cfg_cpu_rx_queue(int queue_id, uint32_t queue_size, void *rx_isr)
{
   int rc;
   rdpa_cpu_rxq_cfg_t rxq_cfg = {};

   /* Read current configuration, set new drop threshold and ISR and write
    * back.
    */
   rc = rdpa_cpu_rxq_cfg_get(spu_rdpa_cpu_obj, queue_id, &rxq_cfg);
   if (rc == 0)
   {
      rxq_cfg.size     = queue_size;
      rxq_cfg.isr_priv = queue_id;
      rxq_cfg.rx_isr   = (rdpa_cpu_rxq_rx_isr_cb_t)rx_isr;
      rc = rdpa_cpu_rxq_cfg_set(spu_rdpa_cpu_obj, queue_id, &rxq_cfg);
   }

   return rc;
}  /* spu_runner_cfg_cpu_rx_queue() */

static int spu_runner_cfg_cpu_reason_to_queue(void)
{
   int                     rc;
   rdpa_cpu_reason_index_t reason_cfg_idx = {BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED};
   rdpa_cpu_reason_cfg_t   reason_cfg = {};

   while (!rdpa_cpu_reason_cfg_get_next(spu_rdpa_cpu_obj, &reason_cfg_idx))
   {
      if ( reason_cfg_idx.reason == rdpa_cpu_rx_reason_ipsec )
      {
         reason_cfg.queue = CPU_RX_IPSEC_LOOPBACK_QUEUE_ID;
         reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
      }
      else {
          continue;
      }

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
      if (reason_cfg_idx.dir == rdpa_dir_us) {
          reason_cfg_idx.table_index = CPU_REASON_LAN_TABLE_INDEX;
      }
      else {
          reason_cfg_idx.table_index = CPU_REASON_WAN0_TABLE_INDEX;
      }
#endif
      rc = rdpa_cpu_reason_cfg_set(spu_rdpa_cpu_obj, &reason_cfg_idx, &reason_cfg);
      if (rc < 0)
      {
         printk("spu_runner_cfg_cpu_reason_to_queue: Error (%d) configuraing CPU reason to queue \n", rc );
         return rc;
      }
   }
   return 0;
} /* spu_runner_cfg_cpu_reason_to_queue */

static int spu_runner_sa_desc_table_address_set(uintptr_t descrBase, uint16_t descrSize)
{
   int rc = 0;

   BDMF_MATTR(attrs, rdpa_ipsec_drv());

   /* set ipsec sa table ddr address */
   rc = rc ? rc : rdpa_ipsec_sa_table_ddr_addr_set(attrs, descrBase);
   
   /* set ipsec sa table ddr address */
   rc = rc ? rc : rdpa_ipsec_sa_entry_size_set(attrs, descrSize);
   
   /* Create the ipsec object */
   rc = rc ? rc : bdmf_new_and_set(rdpa_ipsec_drv(), NULL, attrs, &ipsec_obj);
   if (rc)
   {
      printk("spu_runner_sa_desc_table_address_set: Failed to create ipsec object, rc %d\n", rc);
      printk("spu_runner_sa_desc_table_address_set: sa_table_ddr_addr=%lu sa_size=%u\n", descrBase, descrSize);
      rc = -EFAULT;
   }

   return rc;
   
}  /* spu_runner_sa_desc_table_address_set */

static int spu_runner_sa_desc_table_address_destroy( void )
{
   int rc = 0;
   
   /* destroy the ipsec object */
   rc = bdmf_destroy(ipsec_obj);
   if (rc)
   {
      printk("spu_runner_sa_desc_table_address_set: Failed to destroy ipsec object, rc %d\n", rc);
      rc = -EFAULT;
   }

   return rc;

}  /* spu_runner_sa_desc_table_address_destroy */

/*---------------------------------------------------------------------------
 * void spu_runner_unregister(void)
 * Description:
 *    Called when the driver is stopped.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void spu_runner_unregister(void)
{
   /* Stop RX thread first so it won't touch anything being deallocated */
   kthread_stop(spuinfo->rx_thread);

   rdpa_cpu_int_disable(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);
   rdpa_cpu_rxq_flush_set(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID, 1);
   rdpa_cpu_int_clear(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);

   spu_runner_sa_desc_table_address_destroy();

   if (spuinfo->descrBase) kfree(spuinfo->descrBase);
   spuinfo->descrBase = NULL;
}


/*---------------------------------------------------------------------------
 * void spu_runner_register(void)
 * Description:
 *    Called when the driver is started.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static int spu_runner_register(void)
{
   int              size;
   struct spu_desc *pDescr;
   int              i;

   INIT_LIST_HEAD(&spuinfo->descList);

   /* allocate descriptors */
   size = (SPU_NUM_DESCR * sizeof(struct spu_desc)) + SPU_DESC_ALIGN;
   spuinfo->descrBase = kzalloc(size, GFP_KERNEL);
   if (NULL == spuinfo->descrBase) {
       printk("spu_register: Insufficient memory for descriptors\n");
       return -1;
   }
   spuinfo->alignedDescrBase = (void *)SPU_ALIGN(CACHE_TO_NONCACHE(spuinfo->descrBase));

   pDescr = (struct spu_desc *)spuinfo->alignedDescrBase;
   for (i = 0; i < SPU_NUM_DESCR; i++) {
       list_add_tail(&pDescr->entry, &spuinfo->descList);
       pDescr->index = i;
       pDescr++;
   }

   spu_runner_sa_desc_table_address_set((uintptr_t)spuinfo->alignedDescrBase, sizeof(struct spu_desc));

   spuinfo->rx_work_avail = 0;

   init_waitqueue_head(&spuinfo->rx_thread_wqh);
   spuinfo->rx_thread = kthread_create(spu_runner_rxq_thread, NULL, "spu_rx");
   wake_up_process(spuinfo->rx_thread);

   rdpa_cpu_int_enable(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);

   return 0;
}

/*---------------------------------------------------------------------------
 * void spu_runner_deinit(void)
 * Description:
 *    Called when the driver is unloaded.
 * Returns: void
 *---------------------------------------------------------------------------
 */
__exit void spu_runner_deinit(void)
{
   spu_runner_unregister();

   if (ipsec_obj != NULL)
      bdmf_destroy(ipsec_obj);

   spu_runner_cfg_cpu_rx_queue(CPU_RX_IPSEC_LOOPBACK_QUEUE_ID, 0, NULL);

   if (spu_rdpa_cpu_obj) bdmf_put(spu_rdpa_cpu_obj);
} /* spu_runner_deinit */

/*---------------------------------------------------------------------------
 * void spu_runner_init(void)
 * Description:
 *    Called when the driver is loaded.
 * Returns: void
 *---------------------------------------------------------------------------
 */
__init int spu_runner_init(void)
{
   int rc = 0;

   /* get rdpa cpu object */
   if (rdpa_cpu_get(rdpa_cpu_host, &spu_rdpa_cpu_obj))
   {
       printk("spu_runner_init: cpu get failed\n");
       return -ESRCH;
   }

   rdpa_cpu_int_disable(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);
   rdpa_cpu_int_clear(rdpa_cpu_host, CPU_RX_IPSEC_LOOPBACK_QUEUE_ID);

   rc = spu_runner_cfg_cpu_rx_queue(CPU_RX_IPSEC_LOOPBACK_QUEUE_ID, CPU_RX_IPSEC_LOOPBACK_QUEUE_SIZE, &spu_runner_rx_queue_isr);
   if (rc)
   {
      printk("spu_runner_init: Error(%d) cfg IPSEC offload rx queue\n", rc);
      bdmf_unlock();
      return -EFAULT;
   }

   bdmf_lock();
   rc = spu_runner_cfg_cpu_reason_to_queue( );
   if (rc)
   {
      printk("spu_runner_init: Error(%d) cfg IPSEC offload rx queue reason\n", rc);
      bdmf_unlock();
      return -EFAULT;
   }

   bdmf_unlock();

   rc = spu_runner_register();
   if (rc)
   {
      printk("spu_runner_register failed with error %d\n", rc);
      return -EFAULT;
   }
    
   return rc;
}  /* spu_runner_init() */


