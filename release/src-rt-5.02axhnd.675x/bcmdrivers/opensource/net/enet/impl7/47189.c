/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
      All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */


/*
 * Dummy switch and DMA-port definitions for use in BCM47189.
 */


#include "port.h"
#include "enet.h"
#include <linux/of.h>
#include <linux/nbuff.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include "enet_dbg.h"
#include "bcmenet_common.h"
#include "bcmenet_dma.h"
#include "dma.h"


#define NUMBER_OF_CHANNELS 2
extern int chan_thread_handler(void *data);

/********** Public variables **********/

const struct of_device_id enetxapi_of_platform_enet_table[] = {
        { .compatible = "bcmenet0",   .data = (void *)0, },
        { .compatible = "bcmenet1",   .data = (void *)0, },
        { /* end of list */ },
};


/********** Private variables **********/

static int initialized_ports;

/*
 * IRQ number for DMA RX interrupt on GMAC0.
 * Taken from the Device Tree.
 */
static int rx_irq;

/*
 * References to the enet channels for global access.
 */
static enetx_channel *channels[NUMBER_OF_CHANNELS];

/********** External functions **********/
/* Driver general initialization function (enet.c) */
int __init bcm_enet_init(void);

/********** Function definitions **********/

/*
 * dma47189_rx_isr
 *
 * ISR for DMA RX interrupts.
 *
 * PARAMETERS
 * irq: IRQ number
 * param: GMAC core number (int: 0/1)
 *
 * NOTES
 * For ARM CPUs the interrupt is always rearmed after the ISR finishes.
 *
 * To maintain NAPI semantics, the ISR doesn't clean the I_RI bit of
 * intstatus.
 * Further interrupt (DMA RX) triggering is avoided by disabling GMAC
 * interrupts, which is done in enetxapi_queue_int_disable during NAPI RX
 * processing path.
 */
static irqreturn_t dma47189_rx_isr(int irq, void *param)
{
    int corenum = (int)param;
    enetx_channel *chan = channels[corenum];

    /*
     * Inspect the appropriate GMAC interrupt status depending on the
     * receiving channel.
     */
    if (corenum == 0) {
        if (ENET_CORE0_MISC->intstatus & I_RI)
            enetx_rx_isr(chan);
    }
    else if (corenum == 1) {
        if (ENET_CORE1_MISC->intstatus & I_RI)
            enetx_rx_isr(chan);
    }

    return IRQ_HANDLED;
}


/*
 * enetxapi_of_platform_enet_probe
 *
 * Driver entry point. Called once for each bcmenet device defined in
 * the device-tree.
 * When run for the first interface, it delegates driver initialization
 * to bcm_enet_init (enet.c).
 *
 * When run for the second interface (if available), the driver
 * is already initialized.
 * Simply enable the interrupt for the second interface and link its
 * enet channel to the channel of the first interface.
 */
int enetxapi_of_platform_enet_probe(struct platform_device *ofdev)
{
    const struct of_device_id *match;
    struct device_node *np = ofdev->dev.of_node;

    match = of_match_device(enetxapi_of_platform_enet_table, &ofdev->dev);
    if (!match)
        return -EINVAL;

    rx_irq = irq_of_parse_and_map(np, 0);

    if (strstr(ofdev->name, "enetcore0")) {
        bcm_enet_init();
    }
    else if (strstr(ofdev->name, "enetcore1") && (initialized_ports == 2)) {
        /*
         * Setup the Ethernet core 1 features that are still pending to
         * initialize:
         * - IRQ registering
         * - Link channel 1 to channel 0 for RX processing
         */
        channels[0]->next = channels[1];
        request_irq(rx_irq, dma47189_rx_isr, 0, "bcmenet1", (void *)1);
    }

    return 0;
}

int enetxapi_of_platform_enet_remove(struct platform_device *ofdev)
{
    return 0;
}


/*
 * enetxapi_fkb_databuf_recycle
 *
 * Recycles a BPM buffer.
 *
 * PARAMETERS
 * fkb: Pointer to the fkb containing the buffer to recycle
 * context: Unused
 *
 */
void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb, void *context)
{
    //printk(KERN_EMERG "===== enetxapi_fkb_databuf_recycle\n");
    /* fkb_databuf_recycle currently implemented in 47189_dma.c */
    fkb_databuf_recycle(fkb, context);
}


void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags)
{
    enet_dma_recycle(skb, context, flags);
    //dma_enet_buf_recycle(skb, context, flags);
}


/*
 * enetxapi_queue_int_enable
 *
 * Re-enables the GMAC DMA RX interrupt for a channel.
 *
 * PARAMETERS
 * chan: Pointer to the target channel
 * q_id: Channel queue (always 0 for 47189)
 */
void enetxapi_queue_int_enable(enetx_channel *chan, int q_id)
{
    if (chan->rx_q[q_id] == 0) {
        //ENET_CORE0_MISC->intstatus = I_RI;
        ENET_CORE0_MISC->intmask = I_RI;
    }
    else if (chan->rx_q[q_id] == 1) {
        //ENET_CORE1_MISC->intstatus = I_RI;
        ENET_CORE1_MISC->intmask = I_RI;
    }
}


/*
 * enetxapi_queue_int_disable
 *
 * Disable all GMAC interrupts for a channel.
 *
 * PARAMETERS
 * chan: Pointer to the target channel
 * q_id: Channel queue (always 0 for 47189)
 */
void enetxapi_queue_int_disable(enetx_channel *chan, int q_id)
{
    if (chan->rx_q[q_id] == 0) {
        ENET_CORE0_MISC->intstatus = I_RI;
        ENET_CORE0_MISC->intmask = 0;
    }
    else if (chan->rx_q[q_id] == 1) {
        ENET_CORE1_MISC->intstatus = I_RI;
        ENET_CORE1_MISC->intmask = 0;
    }
}


/*
 * enetxapi_queues_init
 *
 * Initialize Ethernet channels
 *
 * PARAMETERS
 * chan: Pointer to a enetx_channel array. To be allocated in this function.
 *
 * NOTES
 * This is called at the end of bcm_enet_init. All mac drivers, ports and
 * are initialized at this point.
 *
 * TODO
 * Consider requesting the IRQs somewhere else and encode the channels-GMAC
 * relationship differently.
 */
int enetxapi_queues_init(enetx_channel **_chan)
{
    /*
     * Configure two channels, one for each GMAC core (channel0 -> GMAC0,
     * channel1 -> GMAC1). Each channel contains only one queue and rx_q[0] is
     * used to save the queue number (0 for GMAC0, 1 for GMAC1).
     * Both channels use the same ISR for receiving.
     */

    enetx_channel *chan;
    int i;

    *_chan = kzalloc(sizeof(enetx_channel) * NUMBER_OF_CHANNELS, GFP_KERNEL);
    if (!*_chan) {
        enet_err("Failed to create queues\n");
        return 1;
    }

    for (i = 0; i < NUMBER_OF_CHANNELS; i++) {
        chan = &(*_chan)[i];
        /* Only one queue. Queue id = 0 */
        chan->rx_q_count = 1;
        chan->rx_q[0] = i;
        channels[i] = chan;
        chan->rx_thread = kthread_run(chan_thread_handler, chan, "enet_rx");
        init_waitqueue_head(&chan->rxq_wqh);
    }
    /*
     * TODO: If enet_open initializes all channels the first time it's called,
     * then we'd have to link the two channels here. But if the board has
     * only one interface, linking the channels would make enet_open call
     * enetxapi_queue_int_enable on the second channel (linked from the first)
     * and cause the 47189 to hang because it wouldn't be initialized.
     */
    request_irq(rx_irq, dma47189_rx_isr, 0, "bcmenet0", 0);

    return 0;
}


int enetxapi_queues_uninit(enetx_channel **_chan)
{
    enetx_channel *next, *chan = *_chan;

    while (chan) {
        if (chan->rx_thread) {
                kthread_stop(chan->rx_thread);
                chan->rx_thread = NULL;
        }

        next = chan->next;
        kfree(chan);
        chan = next;
    }
    return 0;
}


/*
 * enetxapi_queue_need_reschedule
 *
 * RETURN
 * 1: If there are new received packets to process and isr handler need to be rescheduled
 * 0: If there aren't any more received packets pending to be processed and isr hadler wil be waked up by irq.
 *
 * NOTES
 * It simply relies on the DMA is_rx_empty implementation.
 */
int enetxapi_queue_need_reschedule(enetx_channel *chan, int q_id)
{
/* If the queue got full during handling of packets, new packets will not cause
 * interrupt (they will be dropped without interrupt). In this case, no one
 * will wake up NAPI, ever. The solution is to schedule another NAPI round
 * if the queue is not empty. */

    return !is_rx_empty(chan->rx_q[q_id]);
}


/*
 * enetxapi_rx_pkt
 */
int enetxapi_rx_pkt(int hw_q_id, FkBuff_t **fkb, enetx_rx_info_t *rx_info)
{
    unsigned char *pBuf = 0;
    int len = 0;

    rx_info->src_port = hw_q_id;
    rx_info->data_offset = 0;
    rx_info->extra_skb_flags = 0;

    if (dma_rx(hw_q_id, &pBuf, &len) == -1)
        return -1;

    if (len == 0) {
        enet_err("Empty buffer received\n");
        return -1;
    }

    *fkb = fkb_init(pBuf, BCM_PKT_HEADROOM, pBuf, len);
    (*fkb)->recycle_hook = (RecycleFuncP)enet_dma_recycle;
    (*fkb)->recycle_context = hw_q_id;

    return 0;
}

int enetxapi_post_config(void)
{
    return 0;
}

/********************************************************************
 * DMA-based port implementation.
 * If this is generic enough it could be put in a separate file so it can be
 * used by all DMA-based network processors.
 ********************************************************************/

/*
 * port_dma_port_init
 *
 * Initializes a DMA port. Allocates a BcmEnet_devctrl structure as the priv
 * field and calls the dma initialization function.
 *
 * PARAMETERS
 * self: The port to initialize
 */
static int port_dma_port_init(enetx_port_t *self)
{
    BcmEnet_devctrl *devctrl = NULL;

    if (!(devctrl = kzalloc(sizeof(BcmEnet_devctrl), GFP_KERNEL))) {
        enet_err("failed to allocate BcmEnet_devctrl object\n");
        return -1;
    }
    self->priv = devctrl;
    self->n.blog_phy = BLOG_ENETPHY;
    self->n.blog_chnl = self->n.blog_chnl_rx = self->p.port_id;
    self->n.set_channel_in_mark = 0;
    self->n.port_netdev_role = PORT_NETDEV_ROLE_LAN;

    mux_set_rx_index(root_sw, self->p.port_id, self);
    initialized_ports++;

    return 0;
}

static void port_dma_port_open(enetx_port_t *self)
{
    dma_init(self);
    port_generic_open(self);
}

static int port_dma_port_uninit(enetx_port_t *self)
{
    return 0;
}


/*
 * dispatch_pkt_dma
 *
 * Programs a DMA transmission.
 *
 * PARAMETERS
 * pNBuff: Buffer to transmit
 * port: Target port
 * channel: Always 0 here
 * egress_queue: Always 0
 */
static int dispatch_pkt_dma(dispatch_info_t *dispatch_info)
{
    dma_tx(dispatch_info->port, dispatch_info->pNBuff, dispatch_info->channel);
    return 0;
}

static void port_dma_port_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    port_generic_stats_get(self, net_stats);

    /* TODO: Add FW dropped packets */
}

static void port_dma_port_stop(enetx_port_t *self)
{
    dma_uninit(self);
    port_generic_stop(self);
}


port_ops_t port_dma_port =
{
    .init         = port_dma_port_init,
    .uninit       = port_dma_port_uninit,
    .open         = port_dma_port_open,
    .dispatch_pkt = dispatch_pkt_dma,
    .stats_get    = port_dma_port_stats_get,
    .stop         = port_dma_port_stop,
};




/********************************************************************
 * 47189 Dummy switch implementation.
 * May be reused for other switchless targets
 ********************************************************************/

/********** Stub functions **********/

static int dummy_sw_init(enetx_port_t *self)
{
    return 0;
}

static int dummy_sw_uninit(enetx_port_t *self)
{
    return 0;
}


/*
 * dummy_port_id_on_sw
 *
 * Assigns a port_id and a port_type based on the fields of port_info.
 *
 * PARAMETERS
 * port_info (input): Port information retieved from the object model
 * port_id (output)
 * port_type (output)
 */
static int dummy_sw_port_id_on_sw(port_info_t *port_info, int *port_id,
                                  port_type_t *port_type)
{
    /*
     * A 47189 port is always a GENERIC_DMA port and its identifier is its
     * port number (taken from boardparms)
     */
    *port_id = port_info->port;
    *port_type = PORT_TYPE_GENERIC_DMA;

    return 0;
}

static int dummy_sw_hw_switching_state = HW_SWITCHING_DISABLED;

static int dummy_sw_hw_switching_set(enetx_port_t *sw, unsigned long state)
{
    dummy_sw_hw_switching_state = (int)state;
    return 0;
}

static int dummy_sw_hw_switching_get(enetx_port_t *sw)
{
    return dummy_sw_hw_switching_state;
}

int enetxapi_rx_pkt_dump_on_demux_err(enetx_rx_info_t *rx_info)
{
    return 1;
}

sw_ops_t port_dummy_sw =
{
    .init          = dummy_sw_init,
    .uninit        = dummy_sw_uninit,
    .mux_port_rx    = mux_get_rx_index,
    .mux_free      = mux_index_sw_free,
    .port_id_on_sw = dummy_sw_port_id_on_sw,
    .hw_sw_state_set = dummy_sw_hw_switching_set,
    .hw_sw_state_get = dummy_sw_hw_switching_get,
};
