/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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
/***************************************************************************/
#include <linux/module.h>  /* Specifically, a module */
#include <linux/kthread.h> /* For kthread_run */
#include <linux/delay.h>
#include <linux/debugfs.h> /* For debugfs_create_dir*/
#include <linux/uaccess.h> /* For copy_*_user in sysfs read/write*/
#include <linux/sort.h>
#include <linux/semaphore.h>
#include <linux/circ_buf.h> /* For CIRC_CNT */
#include <linux/dma-mapping.h>
#if defined(CONFIG_PMC_PCM_V1)
#include <pmc/pmc_pcm.h>
#endif
#include <pcm_regs.h>
#include <pcmhal_pcm.h>
#include <iudma.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/io.h>
#if !(defined(CONFIG_BRCM_SMC_BOOT))
#include "bcm_ubus4.h"
#endif

/*
   * There are a couple of fixes which have been applied to 6888 but these fixes break the PCM enable/disable fix that we had.
   * Vijay wants us to run 4 tests. 
   * Case1:  fix2_dis, pcm_dis,  pcm_en,  fix2_en
   * Case 2: pcm_dis,  fix2_dis, fix2_en, pcm_en
   * Case 3: fix2_dis, pcm_dis , fix2_en, pcm_en
   * Case 4: pcm_dis,  fix2_dis, pcm_en,  fix2_en 
      */
/* Non or only one of these can be set to 1 */
#define FIX2DIS_PCMDIS_PCMEN_FIX2EN 0
#define PCMDIS_FIX2DIS_FIX2EN_PCMEN 0
#define FIX2DIS_PCMDIS_FIX2EN_PCMEN 0
#define PCMDIS_FIX2DIS_PCMEN_FIX2EN 0

/****************************************************************************
* Local function declarations needed in initialization
****************************************************************************/
static void pcm_dma_read_cb(unsigned int chan, char *bufp, size_t size,
                            dma_addr_t bufp_dma);
static void pcm_dma_write_cb(unsigned int chan, char *bufp, size_t size,
                             dma_addr_t bufp_dma);
/****************************************************************************
* Variables
****************************************************************************/
int               iudma_get_irq_number=0;
static   int      initialized = 0;
void __iomem      *apm_reg;
unsigned int loglevel=7; /* Value should be between 0 and 7 */
/* Place holder for stats in case is needed in the future */
struct debug_stats debug_stats = {};
struct iudma_chan_config
{
   int             enabled;
   /* Channel interrupt callback function pointer */
   void            (*data_cb)(unsigned int chan, char *bufp, size_t size,
                              dma_addr_t bufp_dma);
   enum dma_data_direction   direction; /* dma direction */
   int             buffer_count; /* Number of buffers for a given channel */

   /* For some channels, we do not want to generate an interrupt (for example,
    * the TX portion of a duplex DMA may run at the same rate as the RX, so we
    * can do the TX and RX operations in the RX's interrupt). The value in this
    * field specifies which channel's interrupt will call the data callback. If
    * this is the same as our channel id, this channel will handle its own
    * interrupt. */
   int             interrupt_ch;

   /* The time in msec at which this channel interrupt generates an IUDMA tick.
    * If 0, the channel does not generate a tick */
   int             tick;

   /* Addresses & sizes for descriptors and buffers */
   struct dma_addr desc_addr;
   size_t          desc_size;
   struct dma_addr buf_addr;
   size_t          buf_size; /* size for 1 buffer. Total size is size * count */
   int             cur_buf_idx;
};
struct iudma_chan_config  iudma_chan_cfg[IUDMA_NUM_CHANNELS] = {{ 0 }};
struct dhdev
{
   struct device  *device;
   struct class   *class;
};
struct dhdev dhdev;
#define RX_CHAN_ID 4
#define TX_CHAN_ID 5
/* Maximum number of voice channels in the system.
** This represents the sum of all channels available on the devices in the system */
#define BP_MAX_VOICE_CHAN              8
struct med_buf
{
   char     *buf;
   int       size;
   atomic_t  head;
   atomic_t  tail;
   int       watermark;

};
enum dh_sample_rate
{
   DSPHAL_RATE_8KHZ,
   DSPHAL_RATE_16KHZ,
};
/* PCM Channel configuration structure */
struct phchancfg
{
   unsigned int        id;
   unsigned int        enabled;
   unsigned int        initialized;
   wait_queue_head_t   queue;
   spinlock_t          lock;

   /* internal channel configuration */
   enum dh_sample_rate rate;  

   /* index of channel in voice boardparms */
   int                 vp_dev;
   int                 vp_chan;

   /* Egress and ingress buffers */
   struct med_buf      eg_buf;
   struct med_buf      ing_buf;

   //void                (*direct_eg)(struct phchancfg *cfg);

   /* Interface-specific data */
   void               *priv;

};
struct phchancfg  chan_cfg[BP_MAX_VOICE_CHAN];
struct pcm_chan_priv
{
   const struct pcm_dma *rx_dma;
   const struct pcm_dma *tx_dma;
   unsigned int          ts_offset;
};

typedef struct
{
   int rxTimeslot[4];
   int txTimeslot[4];
} BP_PCM_TS_CFG;

static char temp_buf[PCM_BYTES_PER_SAMPLE_WB * PCM_FRAMES_PER_TICK];
volatile unsigned int pcm_chan_ctrl = 0;
static int pcm_chan_count = 0;
#define BUF_SIZE         (128 * sizeof(int16_t))
#define BUF_SIZE_WB      (128 * sizeof(int32_t))

typedef enum
{
   BP_VC_NONE = 0,
   BP_VC_ACTIVE,
   BP_VC_INACTIVE,
} BP_VC_STATUS;

typedef enum
{
   BP_VC_8KHZ,
   BP_VC_16KHZ
} BP_VC_SAMPLE_RATE;

typedef enum
{
   BP_VC_COMP_LINEAR,
   BP_VC_COMP_ALAW,
   BP_VC_COMP_ULAW
} BP_VC_SAMPLE_COMP;

typedef struct
{
   unsigned int id;          /* global voice channel index */
   int          status;      /* BP_VS_STATUS      - active/inactive */
   int          sampleRate;  /* BP_VC_SAMPLE_RATE - narrowband/wideband */
   BP_PCM_TS_CFG ts;        /* timeslot configuration */
} VIRTUAL_BOARDPARAM;

#define PCM_MAX_TS      32
#define PCM_MAX_CHAN PCM_MAX_TS/2   
#define PCM_MAP_MAX 0xffff
struct dma_ts_cfg
{
   int ts_map[PCM_MAX_TS/2]; // devide by two because we use ts16
   int pcm_map[PCM_MAX_CHAN];
};
static struct dma_ts_cfg ts_cfg_txrx = {};
VIRTUAL_BOARDPARAM virt_bp[BP_MAX_VOICE_CHAN];
static volatile int pcm_lpbcktype=0;
static volatile int pcm_disturb=0;
static volatile int pcm_loopback=0;
static volatile int pcm_shift_detected=0;
static volatile int pcm_startCompare=0;
static volatile int pcm_enable_disable=0;

static struct task_struct *pcm_task;
static struct dentry *pcmhal_dir;
static unsigned int pcm_enabled=0;
static unsigned int gbl_int_mask_cache=0;
static int majorNumber;
#define DEVICE_NAME "pcmhal"
#define TASK_NAME "kptsk" // Kernel PCM task
#define PROCFS_BUF_MAX_SIZE 64

#define CREATE_DEBUGFS_FILE(x,y) do { \
   junk = debugfs_create_file(x, S_IRUGO | S_IWUSR,\
         pcmhal_dir, NULL, y);                                        \
   if(!junk){                                                         \
      PHLOG_ERR("%s: failed to create /debug/pcm/%s\n", __func__, x); \
      goto err_full_cleanup;\
   }\
   } while(0)

#define DEFINE_SIMPLE_STRING(__fops, __read, __write)  \
static const struct file_operations __fops = {         \
   .owner    = THIS_MODULE,                            \
   .llseek   = pcm_debug_llseek,                       \
   .read     = __read,                                 \
   .write    = __write,                                \
};

atomic_t start_reqs = ATOMIC_INIT(0);
DEFINE_SPINLOCK(iudma_lock);
DEFINE_MUTEX(print_lock);
DEFINE_MUTEX(write_lock);
#define DBPRINTF_SETUP() \
   char tmp[PROCFS_BUF_MAX_SIZE]; \
   size_t size;          \
   ssize_t ret = 0;      \
   if(*f_pos) return 0;  \
   mutex_lock(&print_lock)

#define DBPRINTF(fmt, ...)                                    \
   do {                                                       \
      size_t uncopied = 0;                                    \
      size = scnprintf(tmp, sizeof(tmp), fmt, ##__VA_ARGS__); \
      uncopied = copy_to_user(&buf[ret], tmp, size);          \
      ret += (size - uncopied);                               \
   } while(0)

#define DBPRINTF_END()        \
   *f_pos += ret;             \
   mutex_unlock(&print_lock); \
   return ret

#define validate_chan(chan)        \
   if(chan > IUDMA_NUM_CHANNELS)   \
      return -EINVAL;

/* Structure that defines the DMA configuration */
struct pcm_dma
{
   /* DMA channel ID */
   unsigned int chan_id;

   /* Pointer to data callback to call when interrupt fires */
   void         (*data_cb)(unsigned int chan, char *bufp, size_t size,
                           dma_addr_t bufp_dma);
   /* Direction of DMA */
   enum dma_data_direction direction;
   /* Burst size */
   int          burst_size;
   /* The id of the channel's interrupt that calls this channel's data
    * callback. Note that if this is the same as the channel id, the interrupt
    * will be automatically enabled for this channel, otherwise it will be
    * disabled. */
   int          interrupt_ch;
   /* The time in msec at which this channel interrupt generates an tick.
    * If 0, the channel does not generate a tick */
   int          tick;
   /* Buffer allocated for DMA for this channel */
   struct dma_addr buffer;
   /* Size of individual buffer */
   size_t       buffer_size;
   /* Number of buffers for a given channel */
   size_t       buffer_count;
};
/* Currently, we only have 1 PCM block, and hence only an RX and TX PCM DMA */
struct pcm_dma pcm_dma_cfg[NUM_PCM_DMA_CHAN] = {
   /* RX dma */
   {
      .chan_id     = PCM_DMA_CHANNEL_RX,
      .data_cb     = pcm_dma_read_cb,
      .direction   = DMA_FROM_DEVICE,
      .burst_size  = 1,
      .interrupt_ch = PCM_DMA_CHANNEL_RX,
      .tick        = PCM_MS_PER_TICK,
      .buffer      = { 0 },
   },

   /* TX dma */
   {
      .chan_id     = PCM_DMA_CHANNEL_TX,
      .data_cb     = pcm_dma_write_cb,
      .direction   = DMA_TO_DEVICE,
      .burst_size  = 1,
      .interrupt_ch = PCM_DMA_CHANNEL_RX,
      .tick        = 0,
      .buffer      = { 0 },
   },
};

typedef enum
{
   VC_8KHZ,
   VC_16KHZ
} VC_SAMPLE_RATE;

typedef enum
{
   VC_COMP_LINEAR,
   VC_COMP_ALAW,
   VC_COMP_ULAW
} VC_SAMPLE_COMP;

/* Macro that returns the Number of PCM timeslots in use by specificed channel
 * parameters */
#define PCM_TS_COUNT(rate) (rate == VC_16KHZ ? 4 : 2)

struct semaphore tsk_sched_sem;  /* Task scheduling semaphore */


static const short chan_pattern[] = {
   0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888
};
/****************************************************************************
* Local function declaration
****************************************************************************/
static void        iudma_clear(void);
static irqreturn_t iudma_isr(int irq, void *dev_id);
static void        iudma_channel_interrupt(unsigned int chan);
static void        iudma_handle_dma_owned(unsigned int chan, int item_idx);
static int         iudma_reset_chan(unsigned int chan);
static void        iudma_prep_bufs(unsigned int chan);
static int         iudma_release_chan(unsigned int chan);

static loff_t      pcm_debug_llseek(struct file *filp, loff_t offset, int origin);
static int         pcm_taskMain(void *unused);
static void        term_taskMain(void);
static int         pcm_init(void);
static int         pcm_deinit(void);
static void        pcm_nco_init(void);
static void        pcm_reg_cfg(void);
static int         pcm_config_channel(struct phchancfg *cfg);
static int         pcm_deconfig_channel(struct phchancfg *cfg);
static void        pcm_ts_alloc( int ts, int pcm_chan);
static void        pcm_ts_dealloc(const struct pcm_dma *dma, int ts);
static int         dh_configure_channel(int dev, int chan);
static void        pcm_ts_default(void);
static void        pcm_ts_init(void);
static void        pcm_ts_clear(void);
static int         compare (const void * elem1, const void * elem2);
static inline int  find_offset( const int offsets[], int size, int value);
static int         pcm_dma_init(void);
static int         pcm_dma_deinit(void);
static size_t      pcm_frame_size(void);
static int         pcm_start(void);
static int         pcm_stop(void);
static int         pcm_decompress_and_swap(struct phchancfg *chan_cfg, char *bufp,
                                    int size);
static int         pcm_swap_and_compress(struct phchancfg *chan_cfg, char *bufp,
                                    int size);
static void        dh_swap_byte (uint16_t* bufp, int size);
static void        dh_copy_egress(struct phchancfg *cfg, char *buf, size_t size);
static void        dh_copy_ingress(struct phchancfg *cfg, char *buf, size_t size);
static int         parse_chancfg(char* input, int count);
static int         validate_cfg(char* in, int count);
/****************************************************************************
* Function implementation
****************************************************************************/
/*****************************************************************************
*  FUNCTION:   iudma_request_chan
*
*  PURPOSE:    Request & initialize an IUDMA channel
*
*  PARAMETERS: chan  - DMA channel id
*              count - number of items to copy
*              size  - size of each item
*              bufp  - dma buffer to use
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      the buffer passed must be properly allocated DMA memory and must
*              be large enough to support (count * size) amount of data.
*
*****************************************************************************/
int iudma_request_chan(unsigned int chan, int count, size_t size,
                       struct dma_addr *bufp)
{
   unsigned long flags = 0;
   validate_chan(chan);

   spin_lock_irqsave(&iudma_lock, flags);

   if(iudma_chan_cfg[chan].desc_addr.cpu)
   {
      spin_unlock_irqrestore(&iudma_lock, flags);
      PHLOG_ERR("%s: iudma channel %u already configured\n", __func__, chan);
      return -EINVAL;
   }
   /* Allocate descriptor ring */
   memset(&iudma_chan_cfg[chan], 0, sizeof(iudma_chan_cfg[0]));
   iudma_chan_cfg[chan].desc_size     = count * sizeof(struct iudma_descr);
   iudma_chan_cfg[chan].desc_addr.cpu = dma_zalloc_coherent(dhdev.device,
         iudma_chan_cfg[chan].desc_size, &iudma_chan_cfg[chan].desc_addr.dma, GFP_ATOMIC | GFP_DMA);
   if(!iudma_chan_cfg[chan].desc_addr.cpu)
   {
      spin_unlock_irqrestore(&iudma_lock, flags);
      PHLOG_ERR("%s:dma_zalloc_coherent failed for iudma channel %u \n", __func__, chan);
      return -ENOMEM;
   }

   iudma_chan_cfg[chan].buffer_count = count;
   iudma_chan_cfg[chan].buf_addr.cpu = bufp->cpu;
   iudma_chan_cfg[chan].buf_addr.dma = bufp->dma;
   iudma_chan_cfg[chan].buf_size     = size;

   spin_unlock_irqrestore(&iudma_lock, flags);

   PHLOG_DEBUG("%s: iudma channel %u desc area 0x%p [0x%p], stram 0x%p\n",
               __func__, chan, iudma_chan_cfg[chan].desc_addr.cpu,
               (void*)iudma_chan_cfg[chan].desc_addr.dma, &IUDMA->stram[chan]);

   return 0;
}
/*****************************************************************************
*  FUNCTION:   iudma_config_chan
*
*  PURPOSE:    Configure an IUDMA channel
*
*  PARAMETERS: chan         - DMA channel id
*              burst_size   - DMA burst size in 32-bit words
*              direction    - direction of DMA channel
*              data_cb      - callback function to call when ISR is received
*              interrupt_ch - the id of the channel who's interrupt will call
*                             the data callback for this channel. If we are
*                             triggering our own interrupt, this will be the
*                             same as the passed channel id.
*              tick         - whether or not this channel's interrupt should
*                             generate an IUDMA tick event
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
int iudma_config_chan(unsigned int chan, int burst_size,
      enum dma_data_direction direction,
      void (*data_cb)(unsigned int chan, char *bufp, size_t size,
                      dma_addr_t bufp_dma),
      int interrupt_ch, int tick)
{
   unsigned long flags = 0;
   validate_chan(chan);

   /* Configure the channel */
   if(burst_size <= 0 || burst_size > IUDMA_MAXBURST_SIZE)
   {
      PHLOG_WARNING("%s: burst_size (%d) outside limits [0, %d] for iudma "
                    "channel %u\n", __func__, burst_size, IUDMA_MAXBURST_SIZE,
                    chan);
      burst_size = 1;
   }

   spin_lock_irqsave(&iudma_lock, flags);

   iudma_chan_cfg[chan].direction    = direction;
   iudma_chan_cfg[chan].data_cb      = data_cb;
   iudma_chan_cfg[chan].interrupt_ch = interrupt_ch;
   iudma_chan_cfg[chan].tick         = tick;

   writel(burst_size, &IUDMA->ctrl[chan].max_burst);

   spin_unlock_irqrestore(&iudma_lock, flags);
   return 0;
}
/*****************************************************************************
*  FUNCTION:   iudma_release_chan
*
*  PURPOSE:    Release an IUDMA channel
*
*  PARAMETERS: chan  - DMA channel id
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
static int iudma_release_chan(unsigned int chan)
{
   struct dma_addr desc;
   size_t          desc_size;
   unsigned long   flags = 0;

   validate_chan(chan);

   /* Set the burstHalt bit while clearing endma bit */
   do
   {
      writel(IUDMA_CONFIG_BURSTHALT, &IUDMA->ctrl[chan].config);
   } while(readl(&IUDMA->ctrl[chan].config) & IUDMA_CONFIG_ENDMA);

   spin_lock_irqsave(&iudma_lock, flags);

   /* Disable interrupt, and reset interrupt state and mask */
   gbl_int_mask_cache &= ~(1 << chan);
   writel(gbl_int_mask_cache, &IUDMA->regs.gbl_int_mask);
   writel(readl(&IUDMA->ctrl[chan].int_status), &IUDMA->ctrl[chan].int_status);
   writel(0, &IUDMA->ctrl[chan].int_mask);

   /* Cleanup the channel config - note that we have to do the
    * dma_free_coherent with irqs enabled since the CPU needs to send IPI to
    * invalidate TLBs. */
   desc      = iudma_chan_cfg[chan].desc_addr;
   desc_size = iudma_chan_cfg[chan].desc_size;
   memset(&iudma_chan_cfg[chan], 0, sizeof(iudma_chan_cfg[0]));

   spin_unlock_irqrestore(&iudma_lock, flags);

   dma_free_coherent(dhdev.device, desc_size, desc.cpu, desc.dma);
   return 0;
}
/*****************************************************************************
*  FUNCTION:   iudma_clear
*
*  PURPOSE:    Clear the IUDMA config
*
*  PARAMETERS: none
*
*  RETURNS:    nothing
*
*  NOTES:      This function assumes the iudma lock is held
*
*****************************************************************************/
static void iudma_clear(void)
{
   int i;

   for(i = 0; i < ARRAY_SIZE(IUDMA->ctrl); i++)
   {
      writel(0,  &IUDMA->ctrl[i].config);
      writel(~0, &IUDMA->ctrl[i].int_status);
      writel(0,  &IUDMA->ctrl[i].int_mask);
      writel(1,  &IUDMA->ctrl[i].max_burst);
      writel(0,  &IUDMA->stram[i].base_desc_pointer);
      writel(0,  &IUDMA->stram[i].state_bytes_done_ring_offset);
      writel(0,  &IUDMA->stram[i].flags_length_status);
      writel(0,  &IUDMA->stram[i].current_buffer_pointer);
      writel((1 << i), &IUDMA->regs.channel_reset);
   }
   writel(0, &IUDMA->regs.ctrl_config);
   writel(0, &IUDMA->regs.gbl_int_mask);
}
/*****************************************************************************
*  FUNCTION:   iudma_reset_chan
*
*  PURPOSE:    Reset an IUDMA channel config
*
*  PARAMETERS: chan - DMA channel id
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes the iudma lock is held
*
*****************************************************************************/
static int iudma_reset_chan(unsigned int chan)
{
   writel(0, &IUDMA->ctrl[chan].config);
   /* The burst halt on the channels may not properly reset the DMA logic.
    * Setting this ensures the state of the DMA is actually properly reset
    * and won't break when initialized the next time around */
   writel((1 << chan), &IUDMA->regs.channel_reset);
   return 0;
}
/*****************************************************************************
*  FUNCTION:   iudma_isr
*
*  PURPOSE:    Interrupt Service Routine handler - calls the handlers for each
*              channel that has the interrupt bit set and handler enabled.
*
*  PARAMETERS: irq    - interrupt number
*              dev_id - ISR cookie passed in request_irq
*
*  RETURNS:    irq return value
*
*****************************************************************************/
static irqreturn_t iudma_isr(int irq, void *dev_id)
{
   unsigned long flags = 0, status = 0, pend = 0;
   (void)irq;
   (void)dev_id;

   debug_stats.isr_counts++;
   spin_lock_irqsave(&iudma_lock, flags);
   status = readl(&IUDMA->regs.gbl_int_stat);
//=============================
   pend = readl(&IUDMA->ctrl[PCM_DMA_CHANNEL_RX].int_status);
   if (pend & IUDMA_INTSTAT_BDONE)
      debug_stats.bdone[0]++;
   if (pend & IUDMA_INTSTAT_PDONE)
      debug_stats.pdone[0]++;
   if (pend & IUDMA_INTSTAT_NOTVLD)
      debug_stats.notvld[0]++;
   if (pend & IUDMA_INTSTAT_RXDMAERROR)
      debug_stats.rxdma_error[0]++;
   if (pend & IUDMA_INTSTAT_REPIN_ERROR)
      debug_stats.repin_error[0]++;


   pend = readl(&IUDMA->ctrl[PCM_DMA_CHANNEL_TX].int_status);
   if (pend & IUDMA_INTSTAT_BDONE)
      debug_stats.bdone[1]++;
   if (pend & IUDMA_INTSTAT_PDONE)
      debug_stats.pdone[1]++;
   if (pend & IUDMA_INTSTAT_NOTVLD)
      debug_stats.notvld[1]++;
   if (pend & IUDMA_INTSTAT_RXDMAERROR)
      debug_stats.rxdma_error[1]++;
   if (pend & IUDMA_INTSTAT_REPIN_ERROR)
      debug_stats.repin_error[1]++;
//=============================
   pend = readl(&PCM->pcm_int_pending); 
   if (pend & (PCM_TX_UNDERFLOW | PCM_RX_OVERFLOW))
   {
      writel(readl(&PCM->pcm_int_pending), &PCM->pcm_int_pending);
      if (pend & PCM_TX_UNDERFLOW) 
         debug_stats.tx_underflow_counter++;
      if (pend & PCM_RX_OVERFLOW ) 
         debug_stats.rx_overflow_counter++;
   }
   iudma_channel_interrupt(RX_CHAN_ID);

   spin_unlock_irqrestore(&iudma_lock, flags);
   up(&tsk_sched_sem);

   return IRQ_HANDLED;
}
/*****************************************************************************
*  FUNCTION:   iudma_channel_interrupt
*
*  PURPOSE:    Handles the interrupt for a specific channel
*
*  PARAMETERS: chan - DMA channel id
*
*  RETURNS:    nothing
*
*  NOTES:      This function assumes we are holding the iudma lock
*
*****************************************************************************/
static void iudma_channel_interrupt(unsigned int chan)
{
   int item_idx;
   struct iudma_descr *desc;

   /* If DMA is not configured, skip processing this channel */
   if(!iudma_chan_cfg[chan].buffer_count)
      return;

   /* Get the index of the buffer item to process */
   item_idx = iudma_chan_cfg[chan].cur_buf_idx;

   /* Verify the buffer is valid for us to use. If it's not, there may be a
    * mismatch between us and the DMA. Try another buffer. If the second buffer
    * is also not working, reset the DMA.*/
   desc = &(((struct iudma_descr*)iudma_chan_cfg[chan].desc_addr.cpu)[item_idx]);
   if(desc->flags & IUDMA_STATUS_OWN)
   {
      debug_stats.secondDescUsed++;
      item_idx = (item_idx + 1) % iudma_chan_cfg[chan].buffer_count;
      desc = &(((struct iudma_descr*)iudma_chan_cfg[chan].desc_addr.cpu)[item_idx]);
      if(desc->flags & IUDMA_STATUS_OWN)
      {
         iudma_handle_dma_owned(chan, item_idx);
         goto ack_channel;
      }
   }

   /* Call the data callback if it is configured */
   if(iudma_chan_cfg[chan].enabled && iudma_chan_cfg[chan].data_cb)
   {
      iudma_chan_cfg[chan].data_cb(chan,
            iudma_chan_cfg[chan].buf_addr.cpu + (item_idx * iudma_chan_cfg[chan].buf_size),
            iudma_chan_cfg[chan].buf_size,
            iudma_chan_cfg[chan].buf_addr.dma + (item_idx * iudma_chan_cfg[chan].buf_size));
   }

   /* Move our current buffer index forward */
   iudma_chan_cfg[chan].cur_buf_idx++;
   if(iudma_chan_cfg[chan].cur_buf_idx >= iudma_chan_cfg[chan].buffer_count)
      iudma_chan_cfg[chan].cur_buf_idx = 0;

   /* Set the ownership bits on the buffers */
   desc->flags |= IUDMA_STATUS_OWN;
   wmb();

ack_channel:
   /* Ack the interrupt (if required) and re-enable the DMA */
   writel(readl(&IUDMA->ctrl[chan].int_status), &IUDMA->ctrl[chan].int_status);
   writel(IUDMA_CONFIG_ENDMA, &IUDMA->ctrl[chan].config);

   // Service TX if it has not been serviced yet
   if (chan != TX_CHAN_ID)
      iudma_channel_interrupt(TX_CHAN_ID);
}
static void iudma_handle_dma_owned(unsigned int chan, int item_idx)
{
   debug_stats.dma_realigns++;

   /* If this DMA channel has the "not valid" bit set, just restart the DMA */
   if(readl(&IUDMA->ctrl[chan].int_status) & IUDMA_INTSTAT_NOTVLD)
   {
      debug_stats.dma_restarts++;

      iudma_reset_chan(chan);
      iudma_prep_bufs(chan);
   }
}
/*****************************************************************************
*  FUNCTION:   iudma_prep_bufs
*
*  PURPOSE:    Prepare a channel's DMA buffers
*
*  PARAMETERS: chan - DMA channel id
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes the iudma lock is held
*
*****************************************************************************/
static void iudma_prep_bufs(unsigned int chan)
{
   unsigned int i;

   iudma_chan_cfg[chan].cur_buf_idx = 0;
   for(i = 0; i < iudma_chan_cfg[chan].buffer_count; i++)
   {
      unsigned int flags = 0;
      struct iudma_descr *desc =
         &(((struct iudma_descr*)iudma_chan_cfg[chan].desc_addr.cpu)[i]);

      /* Set the descriptor copy size */
      flags |= iudma_chan_cfg[chan].buf_size << IUDMA_FLAGS_LENGTH_SHIFT;

      /* Set the ownership bits for all the buffers */
      flags |= IUDMA_STATUS_OWN << IUDMA_FLAGS_STATUS_SHIFT;

      /* If this is the last descriptor, set the wrap bit */
      if(i + 1 == iudma_chan_cfg[chan].buffer_count)
         flags |= IUDMA_STATUS_WRAP << IUDMA_FLAGS_STATUS_SHIFT;

      /* Set the flags and buffer address for the descriptor */
      desc->flags &= ~IUDMA_FLAGS_STATUS_MASK | ~IUDMA_FLAGS_LENGTH_MASK;
      desc->flags |= flags;
      desc->addr   = iudma_chan_cfg[chan].buf_addr.dma + (i * iudma_chan_cfg[chan].buf_size);
      wmb();
   }

   /* Ensure the state ram is not in a bad state */
   writel(iudma_chan_cfg[chan].desc_addr.dma, &IUDMA->stram[chan].base_desc_pointer);
   writel(0, &IUDMA->stram[chan].state_bytes_done_ring_offset);
}
/*****************************************************************************
*  FUNCTION:   iudma_configure
*
*  PURPOSE:    Configures the IUDMA
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes we hold the iudma_lock
*
*****************************************************************************/
int iudma_configure(void)
{
   int i, ret = 0;

   /* Ensure the IUDMA is fully disabled before starting */
   iudma_clear();

   /* Register the ISR */
   ret = request_irq(iudma_get_irq_num(), iudma_isr, 0, "voip:iudma", iudma_chan_cfg);
   if(ret)
   {
      PHLOG_ERR("%s: unable to install interrupt %d (%d)\n", __func__,
                iudma_get_irq_num(), ret);
      goto out;
   }

   PHLOG_INFO("%s: installed irq handler for interrupt %d\n",
               __func__, iudma_get_irq_num());

   for(i = 0; i < ARRAY_SIZE(iudma_chan_cfg); i++)
   {
      if(!iudma_chan_cfg[i].buffer_count)
         continue;

      /* Reset the channel and prepare the buffers */
      iudma_prep_bufs(i);

      writel(readl(&IUDMA->ctrl[i].int_status), &IUDMA->ctrl[i].int_status);
      writel(IUDMA_INTMASK_BDONE | IUDMA_INTMASK_NOTVLD,
             &IUDMA->ctrl[i].int_mask);

      if(iudma_chan_cfg[i].interrupt_ch == i)
         gbl_int_mask_cache |= (1 << i);
   }
   /* Enable the DMA */
   writel(gbl_int_mask_cache, &IUDMA->regs.gbl_int_mask);

out:
   return ret;
}
/*****************************************************************************
*  FUNCTION:   iudma_enable_chan
*
*  PURPOSE:    Enable an IUDMA channel
*
*  PARAMETERS: chan - DMA channel id
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
int iudma_enable_chan(unsigned int chan)
{
   int ret = 0;
   unsigned long flags = 0;
   validate_chan(chan);

   spin_lock_irqsave(&iudma_lock, flags);
   if(iudma_chan_cfg[chan].enabled)
      goto out;

   iudma_chan_cfg[chan].enabled = 1;
   spin_unlock_irqrestore(&iudma_lock, flags);

   PHLOG_DEBUG("%s: enabling iudma channel %u\n", __func__, chan);
   /* Make sure the iudma is started */
   return ret;

out:
   spin_unlock_irqrestore(&iudma_lock, flags);
   return ret;
}
/*****************************************************************************
*  FUNCTION:   iudma_start
*
*  PURPOSE:    Starts the IUDMA
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes we hold the iudma_lock
*
*****************************************************************************/
int iudma_start(void)
{
   int ret=0;
   debug_stats.isr_counts=0;
   debug_stats.tx_underflow_counter=0;
   debug_stats.rx_overflow_counter=0;

   debug_stats.repin_error[0]=0;
   debug_stats.rxdma_error[0]=0;
   debug_stats.notvld[0]=0;
   debug_stats.pdone[0]=0;
   debug_stats.bdone[0]=0;

   debug_stats.repin_error[1]=0;
   debug_stats.rxdma_error[1]=0;
   debug_stats.notvld[1]=0;
   debug_stats.pdone[1]=0;
   debug_stats.bdone[1]=0;

   if(atomic_inc_return(&start_reqs) != 1)
      return ret;

   iudma_configure();

   /* Enable DMA channels */
   writel(IUDMA_CONFIG_ENDMA, &IUDMA->ctrl[RX_CHAN_ID].config);
   writel(IUDMA_CONFIG_ENDMA, &IUDMA->ctrl[TX_CHAN_ID].config);

   /* Enable DMA controller */
   writel(IUDMA_REGS_CTRLCONFIG_MASTER_EN, &IUDMA->regs.ctrl_config);
   return ret;
}
/*****************************************************************************
*  FUNCTION:   pcm_start
*
*  PURPOSE:    Start PCM
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
static int pcm_start()
{
   /* Enable PCM operation and set the cached pcm_chan_ctrl register */
   PCM->pcm_ctrl |= PCM_ENABLE;
   return 0;
}
/*****************************************************************************
*  FUNCTION:   pcm_stop

*
*  PURPOSE:    Stop PCM
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
int pcm_stop()
{
   /* Stop PCM operation */
   PCM->pcm_ctrl &= ~PCM_ENABLE;

   /* Clear any pending interrupts */
   PCM->pcm_int_pending |= PCM->pcm_int_pending;

   return 0;
}
/*****************************************************************************
*  FUNCTION:   pcm_frame_size
*
*  PURPOSE:    Calculate the size of a single frame in the PCM DMA buffer.
*
*  PARAMETERS: none
*
*  RETURNS:    size, in bytes, of the pcm frame
*
*****************************************************************************/
static size_t pcm_frame_size(void)
{
   size_t frame_size;

   /* Frame size must always be a multiple of 64-bits */
   frame_size = PCM_FRAME_BUF_SIZE(pcm_chan_count * PCM_BYTES_PER_SAMPLE);
   frame_size = (frame_size + 7) & ~0x7;
   return frame_size;
}
/*****************************************************************************
*  FUNCTION:   iudma_stop
*
*  PURPOSE:    Stop the IUDMA
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes we hold the iudma_lock
*
*****************************************************************************/
int iudma_stop(void)
{
   if(atomic_dec_return(&start_reqs) == 0)
   {
      /* Free the ISR and stop the DMA */
      iudma_clear();
      free_irq(iudma_get_irq_num(), iudma_chan_cfg);

      PHLOG_DEBUG("%s: freed irq handler for interrupt %d\n",
                  __func__, iudma_get_irq_num());
   }
   return 0;
}
/*****************************************************************************
*  FUNCTION:   pcm_dma_deinit
*
*  PURPOSE:    Deinitialize the board's PCM DMA
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes it will only be called once to
*              deinitialize the DMA.
*
*****************************************************************************/
static int IsInitialized =0;
static int pcm_dma_deinit()
{
   /* pcm_dma should only be initialized at power up.
    * De-initialization is not allowed. */
  unsigned int i;

   if (!IsInitialized)
      return 0;
   IsInitialized = 0;
   /* Release all the channels and disable the DMA */
   for(i = 0; i < ARRAY_SIZE(pcm_dma_cfg); i++)
   {
      iudma_release_chan(pcm_dma_cfg[i].chan_id);

      /* Free the allocated DMA buffers */
      if(pcm_dma_cfg[i].buffer.cpu)
      {
         dma_unmap_single(dhdev.device, pcm_dma_cfg[i].buffer.dma,
               pcm_dma_cfg[i].buffer_size * pcm_dma_cfg[i].buffer_count,
               pcm_dma_cfg[i].direction);
         kfree(pcm_dma_cfg[i].buffer.cpu);
         pcm_dma_cfg[i].buffer.cpu = NULL;
         pcm_dma_cfg[i].buffer.dma = 0;
      }
   }

   iudma_stop();
   pcm_stop();
   return 0;
}
/*****************************************************************************
*  FUNCTION:   pcm_dma_init
*
*  PURPOSE:    Initialize the board's PCM DMA for TX and RX
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*  NOTES:      This function assumes it will only be called once to initialize
*              the DMA. Note that the enabled channels must be already set in
*              pcm_chan_ctrl.
*
*****************************************************************************/
static int pcm_dma_init()
{
   int i;
   int ret = -ENOMEM;
   unsigned long flags;
   if ( IsInitialized )
      return 0;
   IsInitialized = 1;

   /* Setup the DMA TX and RX channels.
   * pcm_dma_cfg has two members, the first for RX and the second for TX.
   * The following loop initializes RX first and then TX. */
   for(i = 0; i < ARRAY_SIZE(pcm_dma_cfg); i++)
   {
      struct pcm_dma *dma = &pcm_dma_cfg[i];

      /* Allocate all the required buffer area. */
      dma->buffer_size  = pcm_frame_size() * PCM_FRAMES_PER_TICK;
      dma->buffer_count = NUM_BUF_PER_CHAN;
      dma->buffer.cpu   = kmalloc(dma->buffer_size * dma->buffer_count,
                                  GFP_KERNEL | GFP_DMA);

      if(!dma->buffer.cpu)
      {
         PHLOG_ERR("%s: error allocating dma buffers\n", __func__);
         goto err_cleanup_iudma_chan;
      }

      dma->buffer.dma = dma_map_single(dhdev.device, dma->buffer.cpu,
                                        dma->buffer_size * dma->buffer_count,
                                        dma->direction);

      PHLOG_DEBUG("%s: chan %d buffer area 0x%p [0x%p] size %u\n", __func__, i,
            dma->buffer.cpu, (void*)dma->buffer.dma,
            (unsigned int)(dma->buffer_size * dma->buffer_count));

      /* Find the buffer address offset from the pre-allocated buffer */
      ret = iudma_request_chan(dma->chan_id, dma->buffer_count,
                               dma->buffer_size, &dma->buffer);
      if(ret)
      {
         PHLOG_ERR("%s: error while requesting iudma chan %d\n",
                __func__, dma->chan_id);
         goto err_cleanup_iudma_chan;
      }

      iudma_config_chan(dma->chan_id, dma->burst_size, dma->direction,
                        dma->data_cb, dma->interrupt_ch, dma->tick);

      iudma_enable_chan(dma->chan_id);
   }

   /* Make DMA and PCM atomic */
   local_irq_save(flags);
   iudma_start();
   pcm_start();
   local_irq_restore(flags);

   return 0;

err_cleanup_iudma_chan:
   /* Cleanup the current channel, if required */
   if(pcm_dma_cfg[i].buffer.cpu)
   {
      dma_unmap_single(dhdev.device, pcm_dma_cfg[i].buffer.dma,
            pcm_dma_cfg[i].buffer_size * pcm_dma_cfg[i].buffer_count,
            pcm_dma_cfg[i].direction);
      kfree(pcm_dma_cfg[i].buffer.cpu);
      pcm_dma_cfg[i].buffer.cpu = NULL;
      pcm_dma_cfg[i].buffer.dma = 0;
   }
   /* Cleanup the remaining configured channels */
   for(i--; i >= 0; i--)
   {
      iudma_release_chan(pcm_dma_cfg[i].chan_id);
      if(pcm_dma_cfg[i].buffer.cpu)
      {
         dma_unmap_single(dhdev.device, pcm_dma_cfg[i].buffer.dma,
               pcm_dma_cfg[i].buffer_size * pcm_dma_cfg[i].buffer_count,
               pcm_dma_cfg[i].direction);
         kfree(pcm_dma_cfg[i].buffer.cpu);
         pcm_dma_cfg[i].buffer.cpu = NULL;
         pcm_dma_cfg[i].buffer.dma = 0;
      }
   }
   return ret;
}
/*****************************************************************************
*  FUNCTION:   compare
*
*  PURPOSE:    find if arg1 is bigger, smaller, or equal to arg2. 
*              This is to be used by the sort function
*
*  PARAMETERS: elem1 and elem2
*
*  RETURNS:    1 if elem1 bigger
*              -1 if elem2 is bigger
*              0 if they are equal
*
*****************************************************************************/
static int compare (const void * elem1, const void * elem2) 
{
   int f = *((int*)elem1);
   int s = *((int*)elem2);
   if (f > s) return  1;
   if (f < s) return -1;
   return 0;
}
/*****************************************************************************
*  FUNCTION:   find_offset
*
*  PURPOSE:    Find the index/offset for a given value
*
*  PARAMETERS: offsets    - array of ts16 
*              value      - value to be found
*
*  RETURNS:    offset or -1 if not found
*
*****************************************************************************/

static inline int find_offset( const int offsets[], int size, int value)
{
   int i = 0;
   while ( i < size && offsets[i] != value ) 
      i++;
   return ( i == size ? -1 : i );
}
/*****************************************************************************
*  FUNCTION:   pcm_ts_clear
*
*  PURPOSE:    Clear the timeslot allocation tables
*
*  PARAMETERS: none
*
*  RETURNS:    nothing
*
*****************************************************************************/
static void pcm_ts_clear()
{
   int i, j;
   for(i = 0; i < PCM_MAX_TIMESLOT_REGS; i++)
   {
      for(j = 0; j < 8 * sizeof(PCM->pcm_slot_alloc_tbl[0]); j++)
      {
         /* PCM block slot alloc register can only change by 1 bit at a time...
          * so make it happy */
         PCM->pcm_slot_alloc_tbl[i] &= ~(1 << j);
      }
   }
   /* Reset the channel control and the cached value */
   pcm_chan_ctrl = 0;
   pcm_chan_count = 0;             
}
static void pcm_ts_init()
{
   int j;                       
   for(j = 0; j < ARRAY_SIZE(ts_cfg_txrx.ts_map); j++)
   {
      /* Reset all the timeslot-channel mappings to -1 */
      ts_cfg_txrx.ts_map[j] = -1;
      /* Set defaults to a bigger number than PCM_MAX_CHAN to prevent errors in sort functions */
      ts_cfg_txrx.pcm_map[j] = PCM_MAP_MAX;
   }
}
/*****************************************************************************
*  FUNCTION:   pcm_ts_default
*
*  PURPOSE:    Set all the timeslots to a default value.
*
*  PARAMETERS: none
*
*  RETURNS:    nothing
*
*****************************************************************************/
static void pcm_ts_default()
{
   memset(virt_bp, 0, sizeof(virt_bp));
   virt_bp[0].id=0;
   virt_bp[0].status=BP_VC_ACTIVE;
   virt_bp[0].sampleRate =BP_VC_16KHZ;
   virt_bp[0].ts= (BP_PCM_TS_CFG) {.rxTimeslot={0,1,2,3}, .txTimeslot={0,1,2,3}};
   
   virt_bp[1].id=1;
   virt_bp[1].status=BP_VC_ACTIVE;
   virt_bp[1].sampleRate =BP_VC_16KHZ;
   virt_bp[1].ts= (BP_PCM_TS_CFG) {.rxTimeslot={4,5,6,7}, .txTimeslot={4,5,6,7}};

   virt_bp[2].id=2;
   virt_bp[2].status=BP_VC_ACTIVE;
   virt_bp[2].sampleRate =BP_VC_16KHZ;
   virt_bp[2].ts= (BP_PCM_TS_CFG) {.rxTimeslot={8,9,10,11}, .txTimeslot={8,9,10,11}};

   virt_bp[3].id=3;
   virt_bp[3].status=BP_VC_ACTIVE;
   virt_bp[3].sampleRate =BP_VC_16KHZ;
   virt_bp[3].ts= (BP_PCM_TS_CFG) {.rxTimeslot={12,13,14,15}, .txTimeslot={12,13,14,15}};
}

/*****************************************************************************
*  FUNCTION:   
*
*  PURPOSE:    
*
*  PARAMETERS: none
*
*  RETURNS:    nothing
*
*****************************************************************************/
// We only support a pcm device so dev is always 0 
static int dh_configure_channel(int dev, int chan)
{
   struct phchancfg *cfg;
   /* Virtual board parameter for a specific channel */
   VIRTUAL_BOARDPARAM *vbpc = &virt_bp[chan];
   unsigned long flags = 0;

   /* If this voice channel is not active, don't configure anything */
   if(vbpc->status != BP_VC_ACTIVE)
   {
      PHLOG_DEBUG("%s: ignoring non-active device %d channel %d\n",
                  __func__, dev, chan);
      return 0;
   }
   if(vbpc->id >= ARRAY_SIZE(chan_cfg))
   {
      PHLOG_ALERT("%s: channel id %d is out of range (dev %d, chan %d)\n",
                  __func__, vbpc->id, dev, chan);
      return -EINVAL;
   }

   PHLOG_INFO("%s: device %d, channel %d, id %d, status %d, rate %d Hz\n",
          __func__, dev, chan, vbpc->id, vbpc->status,
          vbpc->sampleRate == BP_VC_8KHZ ? 8000 : 16000);

   spin_lock_irqsave(&chan_cfg[vbpc->id].lock, flags);

   cfg            = &chan_cfg[vbpc->id];
   cfg->id        = vbpc->id;
   cfg->vp_dev    = dev;
   cfg->vp_chan   = chan;

   /* Set buffer size and allocate */
   if(vbpc->sampleRate == BP_VC_16KHZ)
      cfg->eg_buf.size = cfg->ing_buf.size = BUF_SIZE_WB;
   else
      cfg->eg_buf.size = cfg->ing_buf.size = BUF_SIZE;

   cfg->eg_buf.buf  = kzalloc(cfg->eg_buf.size, (__GFP_IO | __GFP_FS));
   atomic_set(&cfg->eg_buf.head, 0);
   atomic_set(&cfg->eg_buf.tail, 0);

   cfg->ing_buf.buf = kzalloc(cfg->ing_buf.size, (__GFP_IO | __GFP_FS));
   atomic_set(&cfg->ing_buf.head, 0);
   atomic_set(&cfg->ing_buf.tail, 0);


   spin_unlock_irqrestore(&chan_cfg[vbpc->id].lock, flags);

   pcm_config_channel(cfg);

   return 0;
}
/*****************************************************************************
*  FUNCTION:   dh_deconfigure_channel
*
*  PURPOSE:    Deconfigures a media channel based on the boardparms ids
*
*  PARAMETERS: id - channel index
*
*  RETURNS:    0 on success, error code otherwise
*
*****************************************************************************/
int dh_deconfigure_channel(int id)
{
   struct phchancfg *cfg = &chan_cfg[id];
   unsigned long flags = 0;

   pcm_deconfig_channel(cfg);
   spin_lock_irqsave(&cfg->lock, flags);

   /* Cleanup the channel buffers */
   kfree(cfg->eg_buf.buf);
   kfree(cfg->ing_buf.buf);

   cfg->enabled     = 0;

   spin_unlock_irqrestore(&cfg->lock, flags);

   return 0;
}
/*****************************************************************************
*  FUNCTION:   dh_set_channel_enable
*
*  PURPOSE:    Enables or disables a media channel
*
*  PARAMETERS: id     - channel index
*              enable - 1 to enable, 0 to disable
*
*  RETURNS:    0 on success, error code otherwise
*
*  NOTES:      This function assumes the id given is valid. No checking is
*              performed on it.
*
*****************************************************************************/
static int dh_set_channel_enable(int id, int enable)
{
   unsigned long flags = 0;

   spin_lock_irqsave(&chan_cfg[id].lock, flags);

   /* If already in the right state, no need to do anything */
   if(chan_cfg[id].enabled != enable)
      chan_cfg[id].enabled = enable;

   spin_unlock_irqrestore(&chan_cfg[id].lock, flags);
   return 0;
}
/*****************************************************************************
*  FUNCTION:   dh_swap_byte
*
*  PURPOSE:    Swap byte within a 16bit sample
*
*  PARAMETERS: buf  - buffer pointing the samples
*              size - length of the buffer in bytes
*
*  RETURNS:    Nothing.
*
*****************************************************************************/
static void dh_swap_byte (uint16_t* bufp, int size)
{
   int i = 0;
   for(i = 0; i < size >> 1; i++)
      bufp[i] = ntohs(bufp[i]);
}

/*****************************************************************************
*  FUNCTION:   pcm_swap_and_compress
*
*  PURPOSE:    Handles byte swapping and/or compression of PCM channel data, if
*              required
*
*  PARAMETERS: chan_cfg - channel config struct
*              bufp     - buffer to decompress
*              size     - buffer size
*
*  RETURNS:    new size of buffer (post compression)
*
*****************************************************************************/
static int pcm_swap_and_compress(struct phchancfg *chan_cfg, char *bufp,
                                 int size)
{
   dh_swap_byte((uint16_t*)bufp, size);
   /* By default, don't bother with any decompression */
   return size;
}
/*****************************************************************************
*  FUNCTION:   pcm_decompress_and_swap
*
*  PURPOSE:    Handles decompression of PCM channel data and byte swapping, if
*              required
*
*  PARAMETERS: chan_cfg - channel config struct
*              bufp - buffer to decompress
*              size - buffer size
*
*  RETURNS:    new size after decompression
*
*****************************************************************************/
static int pcm_decompress_and_swap(struct phchancfg *chan_cfg, char *bufp,
                                   int size)
{
   dh_swap_byte((uint16_t*)bufp, size);
   /* By default, don't bother with any decompression */
   return size;
}
/*****************************************************************************
*  FUNCTION:   pcm_interleave_chan
*
*  PURPOSE:    Interleaves and copies audio data for a particular channel into
*              the PCM DMA buffer.
*
*  PARAMETERS: chan_cfg   - audio channel config struct
*              eg_buf     - egress audio buffer
*              size       - size of data to copy from egress buffer
*              dma_buf    - the dma buffer
*              frame_size - size of a PCM frame, in bytes
*
*  RETURNS:    none
*
*****************************************************************************/
void pcm_interleave_chan(struct phchancfg *chan_cfg, char *eg_buf, size_t size,
                         char *dma_buf)
{
   BP_PCM_TS_CFG *tscfg;
   unsigned int frame_size, sample_offset, frame_length, ts16, 
                ch_offset, j, k, ts_count, buf_idx;
   struct dma_ts_cfg *ts_cfg = &ts_cfg_txrx;
   ts16=-1;
   /* For each sample in the buffer */
   buf_idx = 0;
   /* get the frame size in bytes */
   frame_size = pcm_frame_size();
   tscfg = &virt_bp[chan_cfg->vp_chan].ts;

   /* Find the number of allocated timeslots */
   for(ts_count = 0; ts_count < ARRAY_SIZE(tscfg->txTimeslot) &&
         tscfg->txTimeslot[ts_count] >= 0; ts_count++);
   /* Convert frame_size in terms of bytes or short integers  */
   frame_length = frame_size >> ((PCM_BYTES_PER_SAMPLE == 1) ? 0 : 1);
   /* For each frame 'j', copy the sample from the egress buffer */
   for(j = 0; j < PCM_FRAMES_PER_TICK; j++)
   {
      sample_offset = (j * frame_length);
      /* Time slots are passed as 8-bit but we process as 16-bit so increment the K by 2*/ 
      for (k = 0 ; k < ts_count ; k+=2)
      {
         ts16 = tscfg->txTimeslot[k] >> 1; 
         /* Find the offset, where sample will be placed for chan_cfg->pcm_map */
         ch_offset = find_offset( ts_cfg->pcm_map, PCM_MAX_CHAN, ts16);
         if (ch_offset == -1 )
         {
            PHLOG_ERR("%s: ts %d offset not found.\n", __func__,
                  tscfg->txTimeslot[k]);
            break;
         }
 
         if (PCM_BYTES_PER_SAMPLE == 1)
           dma_buf[sample_offset + ch_offset] = eg_buf[buf_idx];
         else
            ((uint16_t*)dma_buf)[sample_offset + ch_offset] = ((uint16_t*)eg_buf)[buf_idx];
         buf_idx++;
      }
   }
}
/*****************************************************************************
*  FUNCTION:   pcm_deinterleave_chan
*
*  PURPOSE:    Deinterleaves and copies audio data for a particular channel
*              from the PCM DMA buffer to the channel's ingress buffer.
*
*  PARAMETERS: chan_cfg   - audio channel config struct
*              in_buf     - ingress audio buffer
*              max_size   - max size of ingress buffer
*              dma_buf    - the dma buffer
*
*  RETURNS:    size of data copied into egress buffer
*
*****************************************************************************/
int pcm_deinterleave_chan(struct phchancfg *chan_cfg, char *in_buf,
                          size_t max_size, char *dma_buf)
{
   BP_PCM_TS_CFG *tscfg;
   unsigned int frame_size, sample_offset, frame_length, ts16, 
                ch_offset, j, k, ts_count, buf_idx;
   struct dma_ts_cfg *ts_cfg = &ts_cfg_txrx;
   ts16 = -1;
   /* For each sample in the buffer */
   buf_idx = 0;
   /* get the frame size in bytes */
   frame_size = pcm_frame_size();
   tscfg = &virt_bp[chan_cfg->vp_chan].ts;

   /* Find the number of allocated timeslots */
   for(ts_count = 0; ts_count < ARRAY_SIZE(tscfg->txTimeslot) &&
         tscfg->txTimeslot[ts_count] >= 0; ts_count++);

   /* Convert frame_size in terms of bytes or short integers  */
   frame_length = frame_size >> ((PCM_BYTES_PER_SAMPLE == 1) ? 0 : 1);
   /* For each frame 'j', place the sample in the ingress buffer */
   for(j = 0; j < PCM_FRAMES_PER_TICK; j++)
   {
      sample_offset = (j * frame_length);
      /* Find the offset, where sample will be placed for chan_cfg->pcm_map */
      for (k = 0 ; k < ts_count ; k+=2)
      {
         ts16 = tscfg->txTimeslot[k] >> 1; 
         ch_offset = find_offset( ts_cfg->pcm_map, PCM_MAX_CHAN, ts16);
         if (ch_offset == -1 )
         {
            PHLOG_ERR("%s: ts %d offset not found.\n", __func__,
                  tscfg->txTimeslot[k]);
            break;
         }

         if (PCM_BYTES_PER_SAMPLE == 1)
            in_buf[buf_idx] = dma_buf[sample_offset + ch_offset];
         else
            ((uint16_t*)in_buf)[buf_idx] = ((uint16_t*)dma_buf)[sample_offset + ch_offset];
         buf_idx++;
      }
   }

   return (buf_idx * PCM_BYTES_PER_SAMPLE);
}
static int pcm_loopback_enable(int enable)
{
   pcm_loopback=enable;
   pcm_shift_detected=0;
   if (enable)
   {
#if (PCMDIS_FIX2DIS_FIX2EN_PCMEN|| FIX2DIS_PCMDIS_FIX2EN_PCMEN)
      PCM->pcm_pcm_misc |= (PCM_TIME_SLOT_SHIFT_FIX_2_EN);
      //PHLOG_CRIT("%d: enable fix 2 before dma. pcm_misc 0x%08x\n", __LINE__, PCM->pcm_pcm_misc);
#endif      
      pcm_dma_init();
#if (FIX2DIS_PCMDIS_PCMEN_FIX2EN||PCMDIS_FIX2DIS_PCMEN_FIX2EN)
      PCM->pcm_pcm_misc |= (PCM_TIME_SLOT_SHIFT_FIX_2_EN);
      //PHLOG_CRIT("%d: enable fix 2 after dma. pcm_misc 0x%08x\n", __LINE__, PCM->pcm_pcm_misc);
#endif
   }
   else
   {
      pcm_startCompare=0;
#if (FIX2DIS_PCMDIS_PCMEN_FIX2EN  || FIX2DIS_PCMDIS_FIX2EN_PCMEN)
      PCM->pcm_pcm_misc &= ~ (PCM_TIME_SLOT_SHIFT_FIX_2_EN);
      //PHLOG_CRIT("%d: disable fix 2 befoer dma. pcm_misc 0x%08x\n", __LINE__, PCM->pcm_pcm_misc);
#endif
      pcm_dma_deinit();
#if (PCMDIS_FIX2DIS_FIX2EN_PCMEN||PCMDIS_FIX2DIS_PCMEN_FIX2EN)
      PCM->pcm_pcm_misc &= ~ (PCM_TIME_SLOT_SHIFT_FIX_2_EN);
      //PHLOG_CRIT("%d: disable fix 2 after dma. pcm_misc 0x%08x\n", __LINE__, PCM->pcm_pcm_misc);
#endif
   }
   return 0;
}

/*****************************************************************************
*  FUNCTION:   dh_copy_ingress
*
*  PURPOSE:    Copies data from the DMAs into the channel's audio buffer and
*              signal anyone waiting for the data
*
*  PARAMETERS: cfg  - channel config
*              buf  - pointer to linear data buffer to copy from
*              size - size of data to copy
*
*  RETURNS:    none
*
*****************************************************************************/
volatile int printNextReadBuf=0;
volatile uint64_t printOnThisIsr=0;
static void dh_copy_ingress(struct phchancfg *cfg, char *buf, size_t size)
{
   unsigned int i;
   static int max_enabled_cfg=0;
   short *bufp = (short*)buf;

   max_enabled_cfg = (max_enabled_cfg > cfg->id) ? max_enabled_cfg : cfg->id;
   if(printOnThisIsr==debug_stats.isr_counts || pcm_disturb)
   {
       printk(KERN_CONT "Read buffer[ch_id=%d]=0x%x\n", cfg->id, bufp[0]);
   }
   for(i = 0; i < size >> 1; i++)
   {
      if((bufp[i] != chan_pattern[cfg->id]))
      {
         if (!pcm_shift_detected)
         {
            
            if (max_enabled_cfg == cfg->id)
            {
               pcm_shift_detected=1;
               pcm_disturb=0;
               pcm_enable_disable=0;
               PHLOG_ERR("%s: Timeslot shift is detected\n", __func__);
            }
         }
         return;
      }
      else
      {
         pcm_shift_detected=0;
      }
   }

}
/*****************************************************************************
*  FUNCTION:   dh_copy_egress
*
*  PURPOSE:    Copies fix pattern from the chan_pattern into the temporary
*              buffer passed
*
*  PARAMETERS: cfg  - channel config
*              buf  - pointer to linear data buffer to copy into
*              size - size of data to copy
*
*  RETURNS:    none
*
*****************************************************************************/
static void dh_copy_egress(struct phchancfg *cfg, char *buf, size_t size)
{
   unsigned int i;
   short *bufp = (short*)buf;
   for(i = 0; i < size >> 1; i++)
   {
      bufp[i] = chan_pattern[cfg->id];
      if (printNextReadBuf)
      {
         printOnThisIsr=debug_stats.isr_counts+1;
         printNextReadBuf=0;
      }
   }
}
/*****************************************************************************
*  FUNCTION:   pcm_dma_write_cb
*
*  PURPOSE:    Write callback for the PCM DMA. Interleaves and copies audio
*              data from each configured audio channel's buffer into the PCM
*              data buffer.
*
*  PARAMETERS: chan - DMA channel id
*              bufp - the DMA buffer
*              size - size of the buffer
*              bufp_dma - physical addr of bufp
*
*  RETURNS:    none
*
*  NOTES:      This function is called from the context of the DMA ISR
*
*****************************************************************************/
static void pcm_dma_write_cb(unsigned int chan, char *bufp, size_t size,
                             dma_addr_t bufp_dma)
{
   VIRTUAL_BOARDPARAM *bpch;
   struct pcm_chan_priv *priv;
   unsigned int i;
   size_t buf_size;

   /* Get the frame size from the channel array */
   for(i = 0; i < ARRAY_SIZE(pcm_dma_cfg); i++)
   {
      if(pcm_dma_cfg[i].chan_id == chan)
         break;
   }
   if(i >= ARRAY_SIZE(pcm_dma_cfg))
   {
      PHLOG_DEBUG("%s: Couldn't find PCM dma config matching DMA channel %d\n",
                  __func__,  chan);
      return;
   }

   /* Iterate through each audio channel 'i'. Check to see if the channel is
    * enabled, if is a PCM channel, and if has been setup for this DMA channel.
    * Then for each audio channel, copy any pending egress data into a
    * temporary buffer and interleave this data into the PCM DMA buffer.
    */
   for(i = 0; i < ARRAY_SIZE(chan_cfg); i++)
   {
      bpch = &virt_bp[chan_cfg[i].vp_chan];
      priv = (struct pcm_chan_priv*) chan_cfg[i].priv;

      /* Skip the channel if we shouldn't process it */
      if(!chan_cfg[i].enabled)
         continue;
      if(!priv || priv->tx_dma->chan_id != chan)
         continue;

      /* Copy the data from the audio channel into our temporary buffer */
      buf_size = ( bpch->sampleRate == BP_VC_16KHZ ? 4 : 2 ) *
                 PCM_FRAMES_PER_TICK;                 

      dh_copy_egress(&chan_cfg[i], temp_buf, buf_size);

      /* Compress the data and/or swap bytes, if required */
      buf_size = pcm_swap_and_compress(&chan_cfg[i], temp_buf, buf_size);

      pcm_interleave_chan(&chan_cfg[i], temp_buf, buf_size, bufp);

      dma_sync_single_for_device(dhdev.device, bufp_dma,
            priv->tx_dma->buffer_size, priv->tx_dma->direction);
   }
}

/*****************************************************************************
*  FUNCTION:   pcm_dma_read_cb
*
*  PURPOSE:    Read callback for the PCM DMA. Deinterleaves and copies audio
*              data from the PCM buffer into each audio channel's buffer.
*
*  PARAMETERS: chan - DMA channel id
*              bufp - the DMA buffer
*              size - size of the buffer
*              bufp_dma - physical addr of bufp
*
*  RETURNS:    none
*
*  NOTES:      This function is called from the context of the DMA ISR
*
*****************************************************************************/
static void pcm_dma_read_cb(unsigned int chan, char *bufp, size_t size,
                            dma_addr_t bufp_dma)
{
   struct pcm_chan_priv* priv;
   unsigned int i, ret;

   /* On the first iteration, we do not read anything so we escape it*/
#define SKIP_FIRST_ITR 2
   if (pcm_startCompare < SKIP_FIRST_ITR)
   {
      pcm_startCompare++;
      return;
   }
   /* Get the frame size from the channel array */
   for(i = 0; i < ARRAY_SIZE(pcm_dma_cfg); i++)
   {
      if(pcm_dma_cfg[i].chan_id == chan)
         break;
   }
   if(i >= ARRAY_SIZE(pcm_dma_cfg))
   {
      PHLOG_DEBUG("%s: Couldn't find PCM dma config matching DMA channel %d\n",
                  __func__,  chan);
      return;
   }

   /* Iterate through each audio channel 'i'. Check to see if the channel is
    * enabled, if it is a PCM channel, and if has been setup for this DMA
    * channel. If it is, deinterleave audio data. Once we have processed all
    * timeslots in all frames, we copy the temporary buffer into the channel's
    * audio buffer and signal to any waiting applications that there is new
    * data for this channel.
    */
   for(i = 0; i < ARRAY_SIZE(chan_cfg); i++)
   {
      priv = (struct pcm_chan_priv*) chan_cfg[i].priv;

      /* Skip the channel if we shouldn't process it */
      if(!chan_cfg[i].enabled)
         continue;
      if(!priv || priv->rx_dma->chan_id != chan)
         continue;

      dma_sync_single_for_cpu(dhdev.device, bufp_dma,
            priv->rx_dma->buffer_size, priv->rx_dma->direction);

      ret = pcm_deinterleave_chan(&chan_cfg[i], temp_buf, sizeof(temp_buf),
                                  bufp);

      /* If the data is compressed, we will need to decompress it */
      ret = pcm_decompress_and_swap(&chan_cfg[i], temp_buf, ret);

      /* Copy the data from our temporary buffer to the audio channel */
      dh_copy_ingress(&chan_cfg[i], temp_buf, ret);

   }
}
/*****************************************************************************
*  FUNCTION:   pcm_ts_dealloc
*
*  PURPOSE:    Deallocate an 8-bit RX and TX PCM timeslots
*
*  PARAMETERS: dma  - pointer to PCM dma config
*              ts   - 8-bit timeslot id
*
*  RETURNS:    nothing
*
*****************************************************************************/
void pcm_ts_dealloc(const struct pcm_dma *dma, int ts)
{
   int ts16, ts_reg, ts_loc, chan_id;
   chan_id = 0;
   ts_loc = 0; 
   struct dma_ts_cfg *ts_cfg = &ts_cfg_txrx;

   if(ts >= PCM_MAX_TS)
   {
      PHLOG_ERR("%s: ts %d outside possible range [0, %d].\n", __func__,
                ts16, PCM_MAX_TS - 1);
      return;
   }

   /* The PCM deals with 16-bit timeslots, so we need to divide our input
    * channels by 2 */
   ts16 = ts >> 1;

   /* Find the register offset, where time slot bit can be set.
    * Each register can set 32 time slots */
   ts_reg = ts16 >> 5;
   /* Find the time slot position within the register */
   ts_loc = ts16 & 0x1F;

   /* Set the new PCM register setting */
   if(PCM->pcm_slot_alloc_tbl[ts_reg] & (1 << ts_loc))
   {
      pcm_chan_count--;
      PCM->pcm_slot_alloc_tbl[ts_reg] &= ~(1 << ts_loc);
      pcm_chan_ctrl                   &= ~(1 << ts_loc);
   }
   ts_cfg->ts_map[ts16] = -1;
   chan_id = find_offset( ts_cfg->pcm_map, PCM_MAX_CHAN, ts16);
   if (chan_id == -1 )
      PHLOG_ERR("%s: ts %d index not found.\n", __func__, ts);

   ts_cfg->pcm_map[chan_id] = PCM_MAP_MAX;
   sort (ts_cfg->pcm_map, sizeof(ts_cfg->pcm_map)/sizeof(ts_cfg->pcm_map[0]), sizeof(ts_cfg->pcm_map[0]), &compare, NULL);
 
   PHLOG_INFO("%s: DMA timeslot [8-bit %d, 16-bit %d] unassigned\n",
              __func__, ts, ts16);
}
/*****************************************************************************
*  FUNCTION:   pcm_ts_alloc
*
*  PURPOSE:    Allocate an 8-bit PCM timeslot for a PCM audio channel
*
*  PARAMETERS: dma        - pointer to PCM dma config
*              ts         - 8-bit timeslot id
*              pcm_chan   - PCM channel id
*
*  RETURNS:    nothing
*
*****************************************************************************/
static void pcm_ts_alloc(int ts, int pcm_chan)
{
   int ts16, ts_reg, ts_loc, chan_id, i;
   struct dma_ts_cfg *ts_cfg = &ts_cfg_txrx;

   if(ts >= PCM_MAX_TS)
   {
      PHLOG_ERR("%s: ts %d outside possible range [0, %d].\n", __func__,
                ts, PCM_MAX_TS - 1);
      return;
   }

   /* The PCM deals with 16-bit timeslots, so we need to divide our input
    * channels by 2 */
   ts16 = ts >> 1;

   /* Find the register offset, where time slot bit can be set.
    * Each register can set 32 time slots and there are in total 4 registers */
   ts_reg = ts16 >> 5;
   /* Find the time slot position within the register */
   ts_loc = ts16 & 0x1F;
  
   /* Allocate a new PCM DMA channel for the timeslot */
   if(ts_cfg->ts_map[ts16] == -1)
   {
      for(chan_id = 0; chan_id < ARRAY_SIZE(ts_cfg->pcm_map); chan_id++)
      {
         if(ts_cfg->pcm_map[chan_id] == PCM_MAP_MAX)
         {
            /* Assign both 8-bit timeslots to the same 16-bit channel */
            ts_cfg->ts_map[ts16] = chan_id;
            ts_cfg->pcm_map[chan_id] = ts16;
            sort (ts_cfg->pcm_map, ARRAY_SIZE(ts_cfg->pcm_map), sizeof(ts_cfg->pcm_map[0]), &compare, NULL);
            break;
         }
      }
      if(chan_id >= ARRAY_SIZE(ts_cfg->pcm_map))
      {
         PHLOG_ERR("%s: No more available channels. chan_id=%d arSize=%d\n", __func__,chan_id, ARRAY_SIZE(ts_cfg->pcm_map) );
         return;
      }
   }

   /* Set the new PCM register setting */
   if(!(PCM->pcm_slot_alloc_tbl[ts_reg] & (1 << ts_loc)))
   {
      pcm_chan_count++;
      PCM->pcm_slot_alloc_tbl[ts_reg] |= (1 << ts_loc);
   }
   /* Cache the pcm channel control - this register can only be written to once
    * the PCM control is enabled */
   pcm_chan_ctrl |= (1 << ts_loc);
   for( i = 0 ; i < ARRAY_SIZE(ts_cfg->pcm_map) && ts_cfg->pcm_map[i]!=PCM_MAP_MAX ; i++) 
      PHLOG_DEBUG("%s: Chan %d ts16 %d updated offset = %d\n", __func__, ts_cfg->ts_map[i], ts_cfg->pcm_map[i], i);

   PHLOG_INFO("%s: DMA timeslot [8-bit %d, 16-bit %d] assigned to "
              "PCM channel %d\n", __func__, ts, ts16, pcm_chan);
}
/*****************************************************************************
*  FUNCTION:   pcm_reg_cfg
*
*  PURPOSE:    Configure and enable PCM registers
*
*  PARAMETERS: none
*
*  RETURNS:    none
*
*****************************************************************************/
static void pcm_reg_cfg(void)
{
   int sample_size = (PCM_BYTES_PER_SAMPLE == 2) ? PCM_SAMPLE_SIZE_16: PCM_SAMPLE_SIZE_8;
   /* PCM Control */
   PCM->pcm_ctrl = (0 << PCM_SLAVE_SEL_SHIFT)
                 | (1 << PCM_CLOCK_INV_SHIFT)
                 | (0 << PCM_FS_INVERT_SHIFT)
                 | (0 << PCM_FS_LONG_SHIFT)
                 | (1 << PCM_FS_TRIG_SHIFT)
                 | (1 << PCM_DATA_OFF_SHIFT)
                 | (sample_size << PCM_SAMPLE_SIZE_SHIFT)
                 | (0 << PCM_LSB_FIRST_SHIFT)
                 | (pcm_lpbcktype << PCM_LOOPBACK_SHIFT) // Testing loopback 
                 | (2 << PCM_CLK_DIV_SHIFT); // Divide 8MHz clock by 4 to get 2MHz clock

   /* Program the clock divider to generate 2048KHz PCM clock */
   if (PCM->pcm_msif_intf & PCM_MSIF_ENABLE) {
     PCM->pcm_ctrl |= PCM_CLOCK_SEL_ISI << PCM_CLK_DIV_SHIFT;
   }

   /* We are assuming PCM will be running at 2048KHz and will be using 8KHz Frame Sync.
      So, each 8KHz frame will contain 256 bits.
    */
   switch ((PCM->pcm_ctrl & PCM_SAMPLE_SIZE) >> PCM_SAMPLE_SIZE_SHIFT) {
      case PCM_SAMPLE_SIZE_8:
         /* In 8KHz frame there will be 32 samples of size 8 bits*/
         PCM->pcm_ctrl |= 32 << PCM_FRAME_SIZE_SHIFT;
         break;
      case PCM_SAMPLE_SIZE_16:
         /* In 8KHz frame there will be 16 samples of size 16 bits*/
         PCM->pcm_ctrl |= 16 << PCM_FRAME_SIZE_SHIFT;
         break;
      case PCM_SAMPLE_SIZE_32:
         /* In 8KHz frame there will be 8 samples of size 32 bits*/
         PCM->pcm_ctrl |= 8 << PCM_FRAME_SIZE_SHIFT;
         break;
   }

   /* Wipe the PCM channel control register */
   PCM->pcm_chan_ctrl = 0;

   /* Make sure the PCM interrupts are cleared and disabled */
   PCM->pcm_int_mask = 0;
}


/*****************************************************************************
*  FUNCTION:   pcm_deconfig_channel
*
*  PURPOSE:    Handle configuring the PCM channel prior to opening/releasing
*
*  PARAMETERS: cfg    - channel configuration
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
static int pcm_deconfig_channel(struct phchancfg *cfg)
{
   int i, tsnum;
   VIRTUAL_BOARDPARAM *chan;

   PHLOG_DEBUG("%s: channel %d\n", __func__, cfg->id);

   if(!pcm_enabled)
      goto free_chan_data;

   chan  = &virt_bp[cfg->vp_chan];
   tsnum = PCM_TS_COUNT(chan->sampleRate);

   /* Deallocate RX and TX timeslots */
   for(i = 0; i < tsnum; i++)
   {
      pcm_ts_dealloc(&pcm_dma_cfg[0], chan->ts.rxTimeslot[i]);
   }

   cfg->initialized = 0;

free_chan_data:
   /* Free any private channel data */
   if(cfg->priv)
   {
      kfree(cfg->priv);
      cfg->priv = NULL;
   }

   return 0;
}
static int  pcm_config_channel(struct phchancfg *cfg)
{
   int i, tsnum;
   VIRTUAL_BOARDPARAM *chan;
   struct pcm_chan_priv *priv;

   PHLOG_DEBUG("%s: channel %d\n", __func__, cfg->id);

   /* Setup the channel's private data */
   priv = kzalloc(sizeof(*priv), GFP_KERNEL);
   if(!priv)
   {
      PHLOG_ERR("%s failed to allocate memory for priv\n", __func__);
      return -ENOMEM;
   }
   priv->rx_dma = &pcm_dma_cfg[0];
   priv->tx_dma = &pcm_dma_cfg[1];
   cfg->priv = priv;


   /* Add the channel to the timeslot map */
   chan  = &virt_bp[cfg->vp_chan];
   tsnum = PCM_TS_COUNT(chan->sampleRate);

   /* Allocate RX and TX timeslots */
   for(i = 0; i < tsnum; i++)
   {
      pcm_ts_alloc(chan->ts.rxTimeslot[i], chan->id);
   }

   cfg->initialized = 1;

   return 0;
}

/*****************************************************************************
*  FUNCTION:   pcm_nco_init
*
*  PURPOSE:    Initialize the PCM NCO
*
*  PARAMETERS: none
*
*  RETURNS:    none
*
*****************************************************************************/
static void pcm_nco_init()
{
   unsigned long nco_mux_cfg = P_PCM_NCO_MUX_CNTL_MISC;

   /* Load MISC register with target FCW from DPLL, and set NCO and enable bits
    * depending on the clock type */
   PCM->reg_pcm_clk_cntl_0 = P_NCO_FCW_MISC;
   PHLOG_NOTICE("%s: PCM clock in normal mode\n", __func__);
   PCM->reg_pcm_clk_cntl_1 = PCM_CLK_CNTL_1_PCM;

   /* Soft-init PCM NCO */
   PCM->reg_pcm_clk_cntl_2 = (P_PCM_NCO_SHIFT & PCM_NCO_SHIFT)
         | ((nco_mux_cfg << PCM_NCO_MUX_SHIFT) & PCM_NCO_MUX_CNTL)
         |  (PCM_NCO_SOFT_INIT);
   PCM->reg_pcm_clk_cntl_2 = (P_PCM_NCO_SHIFT & PCM_NCO_SHIFT)
         | ((nco_mux_cfg << PCM_NCO_MUX_SHIFT) & PCM_NCO_MUX_CNTL);

   if( nco_mux_cfg == P_PCM_NCO_MUX_CNTL_MISC )
   {
      /* Load the MISC FCW */
      PCM->reg_pcm_clk_cntl_2 = (P_PCM_NCO_SHIFT & PCM_NCO_SHIFT)
         | ((nco_mux_cfg << PCM_NCO_MUX_SHIFT) & PCM_NCO_MUX_CNTL)
         |  (PCM_NCO_LOAD_MISC);
      PCM->reg_pcm_clk_cntl_2 = (P_PCM_NCO_SHIFT & PCM_NCO_SHIFT)
         | ((nco_mux_cfg << PCM_NCO_MUX_SHIFT) & PCM_NCO_MUX_CNTL);

      PHLOG_DEBUG("%s: loading MISC FCW to PCM NCO: 0x%08x\n",
             __func__, (unsigned int)PCM->reg_pcm_clk_cntl_0 );
   }
}
/*****************************************************************************
*  FUNCTION:   pcm_clk_init
*
*  PURPOSE:    Configure the PCM clock
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
int pcm_clk_init( void )
{
   PHLOG_INFO("%s: starting PCM clock\n", __func__);

   /* Setup the PCM clock NCO */
   pcm_nco_init();

   /* Configure the PCM registers */
   pcm_reg_cfg();

   return 0;
}
/*****************************************************************************
*  FUNCTION:   pcm_init
*
*  PURPOSE:    Handle initialization of PCM block
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
static int pcm_init()
{
   if(pcm_enabled)
      return 0;
   pcm_enabled = 1;

   PHLOG_INFO("%s: initialize the PCM.\n", __FUNCTION__);
   /* Power up the block */
#if defined(CONFIG_PMC_PCM_V1)
   pmc_pcm_power_up();
#endif
   /* Configure the PCM clock */
   pcm_clk_init();

   /* Clear the PCM timeslots */
   pcm_ts_init();
   pcm_ts_clear();
   return 0;

}

/*****************************************************************************
*  FUNCTION:   pcm_deinit
*
*  PURPOSE:    Handle deinitialization of PCM block
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
static int pcm_deinit()
{
   int ret;
   ret=0;

   if(!pcm_enabled)
      return 0;

   PHLOG_INFO("%s:%d deinitialize the PCM.\n", __FUNCTION__, __LINE__);
   /* Stop PCM */
   pcm_stop();

   /* Ensure all the timeslots are cleared */
   pcm_ts_clear();

   /* Turn of the PCM block */
#if defined(CONFIG_PMC_PCM_V1)
   pmc_pcm_power_down();
#endif
   pcm_enabled = 0;
   return ret;
}
static void term_taskMain()
{
   if (pcm_task)
   { 
      PHLOG_DEBUG("%s: Terminating kptsk thread.\n", __FUNCTION__);
      kthread_stop(pcm_task);
      pcm_task = NULL;
   }
}
/*****************************************************************************
*  FUNCTION:   pcm_taskMain
*
*  PURPOSE:    Run the pcm library
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error code otherwise
*
*****************************************************************************/
DEFINE_SPINLOCK(toggle_lock);
static int pcm_taskMain(void *unused)
{
   static int tmpCounter=0;
   int toggler=1;
   PHLOG_DEBUG("%s:%d Started....\n", __FUNCTION__, __LINE__);

   while (!kthread_should_stop())
   {
      msleep(1000);
      if(pcm_enable_disable && !pcm_shift_detected)
      {
         if (toggler)
         {
            PHLOG_CRIT("en_dis test itr %d \n", tmpCounter);
            tmpCounter++;
         }
         pcm_loopback_enable(toggler);
         toggler=(toggler)? 0 : 1;
      }
      else
      {
         tmpCounter=0;
      }
   }
   PHLOG_DEBUG("%s:%d Exiting kptsk.\n", __FUNCTION__, __LINE__);
   return 0;
}

/*****************************************************************************
*  FUNCTION:   pcm_debug_llseek
*
*  PURPOSE:    place holder. This does not do anything
*
*  PARAMETERS: none
*
*  RETURNS:    -ESPIPE 
*
*****************************************************************************/
static loff_t pcm_debug_llseek(struct file *filp, loff_t offset, int origin)
{
   return -ESPIPE;
}

/*****************************************************************************
*  FUNCTION:   pcm_lpbcktype_read
*
*  PURPOSE:    Prints the loopback type
*
*  PARAMETERS: 
*
*  RETURNS:    
*
*****************************************************************************/
static ssize_t pcm_lpbcktype_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   DBPRINTF_SETUP();
   DBPRINTF("loopback type is %s loopback.\n", (pcm_lpbcktype)? "internal": "external" );
   DBPRINTF_END();
}

/*****************************************************************************
*  FUNCTION:   pcm_lpbcktype_write
*
*  PURPOSE:    Set the loopback type 0 for external and 1 for internal
*
*  PARAMETERS: none
*
*  RETURNS:    copy count on success, error code otherwise
*
*****************************************************************************/
static ssize_t pcm_lpbcktype_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   int buff_size=0;
   char tmp[PROCFS_BUF_MAX_SIZE];
   if ( count != 2)
   {
      PHLOG_ERR("%s:%d Wrong number of inputs. Please look at the help menu\n", __func__, __LINE__);
      return -1;
   }

   mutex_lock(&write_lock);

   memset (tmp, 0, sizeof(tmp));

   buff_size=(count > PROCFS_BUF_MAX_SIZE)? PROCFS_BUF_MAX_SIZE:count;

   if ( copy_from_user(tmp, user_buf, buff_size))
   {
      mutex_unlock(&write_lock);
      return -EFAULT;
   }
   pcm_lpbcktype=(tmp[0]-'0')? 1:0;
   PHLOG_INFO("loopback type is set to %s loopback\n", (pcm_lpbcktype)? "internal" : "external" );
   mutex_unlock(&write_lock);
   return count;
}

DEFINE_SIMPLE_STRING(pcm_lpbcktype_fops, pcm_lpbcktype_read,
                     pcm_lpbcktype_write);
DEFINE_SIMPLE_STRING(pcmhal_fops, NULL, NULL);

/*****************************************************************************
*  FUNCTION:   pcm_chancfg_read
*
*  PURPOSE:    Prints the PCM channel configuration
*
*  PARAMETERS: 
*
*  RETURNS:    
*
*****************************************************************************/
static ssize_t pcm_chancfg_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   int i, j;
   DBPRINTF_SETUP();
   DBPRINTF("pcm channel config. Max supported channel is 8.\n");

   for(i = 0; i < ARRAY_SIZE(virt_bp); i++)
   {
      DBPRINTF("chan=%d, id=%d",i ,virt_bp[i].id);
      DBPRINTF(", status=%s",(virt_bp[i].status)?"active":"idle");
      if (!virt_bp[i].status)
      {
         DBPRINTF("\n");
         continue;
      }
      DBPRINTF(", Band config=%sband",(virt_bp[i].sampleRate)?"Wide":"Narrow");
      DBPRINTF("ts={");
      for (j=0;j<4;j++)
      {
         DBPRINTF("%d", virt_bp[i].ts.rxTimeslot[j]);
         if (j!=3)
            DBPRINTF(" ,");
      }
      DBPRINTF("}\n");
   }
   DBPRINTF_END();
}
static int timeslotCounter=0;
#define ISWIDEBAND(b)   ((b) == 'W' || (b) == 'w')
static int set_cfg(char in, char band)
{
   int i,index;
   index=in-'0';
   virt_bp[index].id=index;
   virt_bp[index].status=BP_VC_ACTIVE;
   if (ISWIDEBAND(band))
   {
      virt_bp[index].sampleRate = BP_VC_16KHZ;
      for(i=0; i < 4; i++)
      {
         virt_bp[index].ts.rxTimeslot[i]= timeslotCounter;
         virt_bp[index].ts.txTimeslot[i]= timeslotCounter;
         timeslotCounter++;
      }
   }
   else
   {
      virt_bp[index].sampleRate = BP_VC_8KHZ;
      for(i=0; i < 2; i++)
      {
         virt_bp[index].ts.rxTimeslot[i]= timeslotCounter;
         virt_bp[index].ts.txTimeslot[i]= timeslotCounter;
         timeslotCounter++;
      }
      for(; i < 4; i++)
      {
         virt_bp[index].ts.rxTimeslot[i]= -1;
         virt_bp[index].ts.txTimeslot[i]= -1;
      }

   }
   return 0;
}
/*
   * Return 0 on success and 1 on failure 
 */
#define ISDIGIT(c)      ((c) >= '0' && (c) <= '9')
#define ISSEMICOLMN(s)  ((s) == ':')
#define ISWN_OR_wn(b)   ((b) == 'W' || (b) == 'w' || (b) == 'N' || (b) == 'n' )
#define VALIDATE_INPUT_STR(c) (ISDIGIT((c)[0]) && ISSEMICOLMN((c)[1]) && ISWN_OR_wn((c)[2]))
static int validate_cfg(char* in, int count)
{
   int i, ret;
   ret=0;
   for(i=0; i < count; i+=4)
   {
      if(!VALIDATE_INPUT_STR(&in[i]))
      {
         PHLOG_ERR("validation failed for chan=%c. Try again.\n", in[i]);
         ret =1;
         break;
      }
      PHLOG_INFO("chan=%c, ", in[i]);
      PHLOG_INFO("wide/narrow band=%c\n\n", in[i+2]);
   }

   return ret;
}
static int parse_chancfg(char* input, int count)
{
   int i;
   int minCount, maxCount;
   minCount=3;
   maxCount=32;
   timeslotCounter=0;
   if (count < minCount || count > maxCount)
   {
      PHLOG_INFO("Input input length. Chancfg is set to default:\"0:w,1:w,2:w,3:w\"\n");
      pcm_ts_default();
   }
   else
   {
      if (validate_cfg(input, count))
         return -1;
      else
         memset(virt_bp, 0, sizeof(virt_bp));

      for(i=0; i < count; i+=4)
      {
         set_cfg(input[i], input[i+2]);
      }
   }

   pcm_init();

   for(i = 0; i < ARRAY_SIZE(virt_bp); i++)
   {
      if (virt_bp[i].status!=BP_VC_ACTIVE)
         continue;
      dh_configure_channel(0, i);
      dh_set_channel_enable(i, 1);
   }
   return 0;
}
/*****************************************************************************
*  FUNCTION:   pcm_chancfg_write
*
*  PURPOSE:    Set the PCM channel configurations
*
*  PARAMETERS: none
*
*  RETURNS:    copy count on success, error code otherwise
*
*****************************************************************************/
static ssize_t pcm_chancfg_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   int buff_size=0;
   char tmp[PROCFS_BUF_MAX_SIZE];
   if ( count >= PROCFS_BUF_MAX_SIZE)
   {
      PHLOG_ERR("%s:%d Wrong number of inputs. Please look at the help menu\n", __func__, __LINE__);
      return -1;
   }

   mutex_lock(&write_lock);

   memset (tmp, 0, sizeof(tmp));

   buff_size=(count > PROCFS_BUF_MAX_SIZE)? PROCFS_BUF_MAX_SIZE:count;

   if ( copy_from_user(tmp, user_buf, buff_size))
   {
      mutex_unlock(&write_lock);
      return -EFAULT;
   }
   PHLOG_INFO("chancfg=%s,count=%d\n", tmp, count);
   if (parse_chancfg(tmp, count))
   {
      PHLOG_ERR("Wrong PCM channel configuration. Valid input example:\n\"0:N,1:N,3:W,4:N,5:N\"\n");
      count=-1;
   }
   mutex_unlock(&write_lock);
   return count;
}

DEFINE_SIMPLE_STRING(pcm_chancfg_fops, pcm_chancfg_read,
                     pcm_chancfg_write);

/*****************************************************************************
*  FUNCTION:   pcm_loopback_read
*
*  PURPOSE:    PCM loopback test status
*
*  PARAMETERS: 
*
*  RETURNS:    
*
*****************************************************************************/
static ssize_t pcm_loopback_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   DBPRINTF_SETUP();
   DBPRINTF("pcm_loopback feature is %s.\n", (pcm_loopback)? "on" : "off");
   DBPRINTF_END();
}

/*****************************************************************************
*  FUNCTION:   pcm_loopback_write
*
*  PURPOSE:    Set th PCM loopback test status. o for off and 1 for on
*
*  PARAMETERS: none
*
*  RETURNS:    copy count on success, error code otherwise
*
*****************************************************************************/
static ssize_t pcm_loopback_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   int buff_size=0;
   char tmp[PROCFS_BUF_MAX_SIZE];
   if ( count != 2)
   {
      PHLOG_ERR("%s:%d Wrong number of inputs. Please look at the help menu\n", __func__, __LINE__);
      return -1;
   }

   mutex_lock(&write_lock);

   memset (tmp, 0, sizeof(tmp));

   buff_size=(count > PROCFS_BUF_MAX_SIZE)? PROCFS_BUF_MAX_SIZE:count;

   if ( copy_from_user(tmp, user_buf, buff_size))
   {
      mutex_unlock(&write_lock);
      return -EFAULT;
   }
   pcm_loopback=(tmp[0]-'0')? 1:0;
   PHLOG_INFO("pcm_loopback updated to %s\n",(pcm_loopback)? "on" : "off" );
   mutex_unlock(&write_lock);
   pcm_loopback_enable(pcm_loopback);
   return count;
}

DEFINE_SIMPLE_STRING(pcm_loopback_fops, pcm_loopback_read,
                     pcm_loopback_write);

/*****************************************************************************
*  FUNCTION:   pcm_disturb_read
*
*  PURPOSE:    Prints the status of distrun function
*
*  PARAMETERS: 
*
*  RETURNS:    
*
*****************************************************************************/
static ssize_t pcm_disturb_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   DBPRINTF_SETUP();
   DBPRINTF("pcm_disturb feature is %s.\n", (pcm_disturb)? "on" : "off");
   DBPRINTF_END();
}

/*****************************************************************************
*  FUNCTION:   pcm_disturb_write
*
*  PURPOSE:    Set the disturb status to on with 1 or off with 0
*
*  PARAMETERS: none
*
*  RETURNS:    copy count on success, error code otherwise
*
*****************************************************************************/
static ssize_t pcm_disturb_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   int buff_size=0;
   char tmp[PROCFS_BUF_MAX_SIZE];
   if ( count != 2)
   {
      PHLOG_ERR("%s:%d Wrong number of inputs. Please look at the help menu\n", __func__, __LINE__);
      return -1;
   }

   mutex_lock(&write_lock);

   memset (tmp, 0, sizeof(tmp));

   buff_size=(count > PROCFS_BUF_MAX_SIZE)? PROCFS_BUF_MAX_SIZE:count;

   if ( copy_from_user(tmp, user_buf, buff_size))
   {
      mutex_unlock(&write_lock);
      return -EFAULT;
   }
   pcm_disturb=(tmp[0]-'0')? 1:0;
   PHLOG_INFO("%s:%d pcm_disturb updated to %s\n", __func__, __LINE__, (pcm_disturb)? "on" : "off" );
   mutex_unlock(&write_lock);

   return count;
}

DEFINE_SIMPLE_STRING(pcm_disturb_fops, pcm_disturb_read,
                     pcm_disturb_write);

/*****************************************************************************
*  FUNCTION:   pcm_help_read
*
*  PURPOSE:    Help menu for this driver 
*
*  PARAMETERS: 
*
*  RETURNS:    
*
*****************************************************************************/
static ssize_t pcm_en_dis_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   DBPRINTF_SETUP();
   DBPRINTF("pcm_enable_disable is %s\n", (pcm_enable_disable)? "on" : "off" );
   DBPRINTF_END();
}

/*****************************************************************************
*  FUNCTION:   pcm_help_write
*
*  PURPOSE:    Place holder
*
*  PARAMETERS: 
*
*  RETURNS:    
*
*****************************************************************************/
static ssize_t pcm_en_dis_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   int buff_size=0;
   char tmp[PROCFS_BUF_MAX_SIZE];

   mutex_lock(&write_lock);

   memset (tmp, 0, sizeof(tmp));

   buff_size=(count > PROCFS_BUF_MAX_SIZE)? PROCFS_BUF_MAX_SIZE:count;

   if ( copy_from_user(tmp, user_buf, buff_size))
   {
      mutex_unlock(&write_lock);
      return -EFAULT;
   }
   pcm_enable_disable=(tmp[0]-'0')? 1:0;

   PHLOG_INFO("loopbackpcm_enable_disable is set to %s\n", (pcm_enable_disable)? "on" : "off" );
   mutex_unlock(&write_lock);
   return count;
}

DEFINE_SIMPLE_STRING(pcm_en_dis_fops, pcm_en_dis_read,
                     pcm_en_dis_write);

static ssize_t pcm_stats_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   DBPRINTF_SETUP();

   DBPRINTF("\t PCM HAL STATISTICS\n");
   DBPRINTF("\t----------------\n");
   DBPRINTF("iudma interrupts    : %llu\n",
            debug_stats.isr_counts);
   DBPRINTF("iudma realigns      : %llu\n",
            debug_stats.dma_realigns);
   DBPRINTF("iudma restarts      : %llu\n",
            debug_stats.dma_restarts);
   DBPRINTF("pcm tx underflow    : %d\n", debug_stats.tx_underflow_counter);
   DBPRINTF("pcm rx overflow     : %d\n", debug_stats.rx_overflow_counter);
   DBPRINTF("PCM shift detected  : %s\n", (pcm_shift_detected)?"yes":"no");

   DBPRINTF("loopback type       : %s\n", (pcm_lpbcktype)? "internal": "external" );
   DBPRINTF("pcm_disturb         : %s\n", (pcm_disturb)? "on" : "off");
   DBPRINTF("pcm_loopback        : %s\n", (pcm_loopback)? "on" : "off");
   DBPRINTF("pcm_enable_disable  : %s\n", (pcm_enable_disable)? "on" : "off" );
   DBPRINTF("Secondary Dsc used  : %d\n", debug_stats.secondDescUsed);
   DBPRINTF("tx underflow        : %d\n", debug_stats.tx_underflow_counter);
   DBPRINTF("tx repin_error      : %d\n", debug_stats.repin_error[1]);
   DBPRINTF("tx rxdma_error      : %d\n", debug_stats.rxdma_error[1]);
   DBPRINTF("tx notvld           : %d\n", debug_stats.notvld[1]);
   DBPRINTF("tx pdone            : %d\n", debug_stats.pdone[1]);
   DBPRINTF("tx bdone            : %d\n", debug_stats.bdone[1]);

   DBPRINTF("rx overflow         : %d\n", debug_stats.rx_overflow_counter);
   DBPRINTF("rx repin_error      : %d\n", debug_stats.repin_error[0]);
   DBPRINTF("rx rxdma_error      : %d\n", debug_stats.rxdma_error[0]);
   DBPRINTF("rx notvld           : %d\n", debug_stats.notvld[0]);
   DBPRINTF("rx pdone            : %d\n", debug_stats.pdone[0]);
   DBPRINTF("rx bdone            : %d\n", debug_stats.bdone[0]);

   DBPRINTF_END();
}

static ssize_t pcm_stats_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   if (count) {
      debug_stats.isr_counts=0;
      debug_stats.dma_realigns=0;
      debug_stats.dma_restarts=0;
      debug_stats.tx_underflow_counter= 0;
      debug_stats.rx_overflow_counter = 0;
      pcm_shift_detected=0;
      PHLOG_NOTICE("Cleared PCM HAL stats\n");
   }
   return count;
}
DEFINE_SIMPLE_STRING(pcm_stats_fops, pcm_stats_read,
                     pcm_stats_write);
static ssize_t pcm_dumpReadBuf_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   printNextReadBuf=1;
   DBPRINTF_SETUP();
   DBPRINTF("Dumping the next buffer\n");
   DBPRINTF_END();
}

static ssize_t pcm_dumpReadBuf_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{

   printNextReadBuf=1;
   return count;
}
DEFINE_SIMPLE_STRING(pcm_dumpReadBuf_fops, pcm_dumpReadBuf_read,
                     pcm_dumpReadBuf_write);

static ssize_t pcm_loglevel_read(struct file *filp, char __user *buf,
                                 size_t count, loff_t *f_pos)
{
   DBPRINTF_SETUP();
   DBPRINTF("loglevel=%d\n", loglevel);
   DBPRINTF_END();
}

static ssize_t pcm_loglevel_write(struct file *file,
      const char __user *user_buf, size_t count, loff_t *ppos)
{
   int buff_size=0;
   char tmp[PROCFS_BUF_MAX_SIZE];

   mutex_lock(&write_lock);

   memset (tmp, 0, sizeof(tmp));

   buff_size=(count > PROCFS_BUF_MAX_SIZE)? PROCFS_BUF_MAX_SIZE:count;

   if ( copy_from_user(tmp, user_buf, buff_size))
   {
      mutex_unlock(&write_lock);
      return -EFAULT;
   }
   if( (tmp[0]-'0')> 7 ||(tmp[0]-'0') < 0)
      PHLOG_ERR("%s:%d loglevel should be between 0 and 7.\n", __func__, __LINE__);
   else
   {
      loglevel=(tmp[0]-'0');
      PHLOG_INFO("%s:%d loglevel updated to %d\n", __func__, __LINE__, loglevel );
   }
   mutex_unlock(&write_lock);
   return count;
}
DEFINE_SIMPLE_STRING(pcm_loglevel_fops, pcm_loglevel_read,
                     pcm_loglevel_write);

/*****************************************************************************
*  FUNCTION:   pcm_set_endianness
*
*  PURPOSE:    Configure the UBUS endianness to PCM
*
*  PARAMETERS: none
*
*  RETURNS:    none
*
*****************************************************************************/
#if !(defined(CONFIG_BRCM_SMC_BOOT))
static void pcm_set_endianness(void)
{
#if defined(UBUS_PORT_ID_PER)
   ubus_master_decode_cfg_set_ctrl(UBUS_PORT_ID_PER, 5);
#endif
}
#endif

static struct of_device_id const bcm_pcm_match[] = {
  { .compatible = "brcm,bcm63xx-apm-pcm" },
  {}
};
MODULE_DEVICE_TABLE(of, bcm_pcm_match);

static int bcm_pcm_probe(struct platform_device *pdev)
{
   int ret = 0;
   struct resource *reg_base;
   if(initialized)
      return -1;

   if (!of_match_device(bcm_pcm_match, &pdev->dev))
   {
      PHLOG_ALERT("%s bcm_pcm dev: Failed to match.\n", __func__);
      return -ENODEV;
   }
   iudma_get_irq_number = platform_get_irq(pdev, 0);
   if (iudma_get_irq_number < 0)
   {
      PHLOG_ALERT("%s Failed to get iudma_get_irq_number.\n", __func__);
      return -ENXIO;
   }
   
   reg_base = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (!reg_base) 
   {
      PHLOG_ALERT("%s Unable to get register resource.\n", __func__);
   	return -ENODEV;
   }
   apm_reg = devm_ioremap_resource(&pdev->dev, reg_base);
   if (IS_ERR(apm_reg))
   {
      PHLOG_ALERT("%s Failed to map the pcm resource.\n", __func__);
      return -ENXIO;
   }
   initialized = 1;
   return ret;
}

static struct platform_driver bcm_pcm_driver = {
   .driver = {
      .name = "brcm,bcm63xx-apm-pcm",
      .of_match_table = bcm_pcm_match,
   },
   .probe = bcm_pcm_probe,
};
/*****************************************************************************
*  FUNCTION:   pcmhal_init
*
*  PURPOSE:    Handle the module initialization.
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error code otherwise
*
*****************************************************************************/
static int __init pcmhal_init(void)
{
   int chan, ret = -ENOMEM;
   struct dentry* junk;
   PHLOG_INFO("Entering PCMHAL module!\n");
   if(platform_driver_register(&bcm_pcm_driver))
   {
      PHLOG_ALERT("%s: bcm_pcm_driver initialization failed.\n", __func__);
      return -EINVAL;
   }
   else
      PHLOG_NOTICE("%s: bcm_pcm_driver initialization succedded.\n", __func__);

   memset(virt_bp, 0, sizeof(virt_bp));
   /* Initialize the device variables */
   memset(&chan_cfg, 0, sizeof(chan_cfg));
   for(chan = 0; chan < ARRAY_SIZE(chan_cfg); chan++)
   {
      init_waitqueue_head( &chan_cfg[chan].queue );
      spin_lock_init( &chan_cfg[chan].lock );
   }

   sema_init(&tsk_sched_sem, 0);
   /* create kptsk thread */
   if(!pcm_task)
   {
      pcm_task = kthread_run(pcm_taskMain, NULL, TASK_NAME);
      if(!pcm_task)
      {
         PHLOG_ERR("%s: kptsk creation failed\n", __FUNCTION__);
         return ret;
      }
   }
   /* The entry will appear at /dev/pcmhal */
   majorNumber = register_chrdev(0, DEVICE_NAME, &pcmhal_fops);
   if (majorNumber<0){
      PHLOG_ERR("%s failed to register a major number\n", __func__);
      ret = majorNumber;
      goto err_task_cleanup;
   }
   PHLOG_DEBUG("%s registered correctly with major number %d\n", DEVICE_NAME, majorNumber);
   dhdev.class = class_create(THIS_MODULE, DEVICE_NAME);
   if (IS_ERR(dhdev.class)) 
   {
      PHLOG_ERR("%s: error creating pcm device class\n", __func__);
      ret = PTR_ERR(dhdev.class); // Correct way to return on a pointer
      goto err_class_cleanup;
   }
   PHLOG_DEBUG(DEVICE_NAME "device class registered correctly\n");
   
   // Register the device driver
   dhdev.device = device_create(dhdev.class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(dhdev.device)){               // Clean up if there is an error
      PHLOG_ERR(DEVICE_NAME "Failed to create the device\n");
      ret = PTR_ERR(dhdev.device);
      goto err_cdev_cleanup;
   }
   PHLOG_DEBUG( DEVICE_NAME "device class created correctly.\n");

   arch_setup_dma_ops(dhdev.device, 0, 0, NULL, false);

   if (dma_coerce_mask_and_coherent(dhdev.device, DMA_BIT_MASK(DMA_COHERENT_BITS))) {
      PHLOG_ERR("%s: error creating device mask\n", __func__);
      goto err_full_cleanup;
   }

   /* The entry will appear at /debug/pcmhal */
   pcmhal_dir = debugfs_create_dir(DEVICE_NAME, NULL);
   if(!pcmhal_dir)
   {
      PHLOG_ERR("%s: failed to create /debug/pcm\n", __func__);
      goto err_full_cleanup;
   }
   CREATE_DEBUGFS_FILE("lpbcktype", &pcm_lpbcktype_fops);
   CREATE_DEBUGFS_FILE("chancfg"  , &pcm_chancfg_fops);
   CREATE_DEBUGFS_FILE("loopback" , &pcm_loopback_fops);
   CREATE_DEBUGFS_FILE("disturb"  , &pcm_disturb_fops);
   CREATE_DEBUGFS_FILE("en_dis"     , &pcm_en_dis_fops);
   CREATE_DEBUGFS_FILE("stats"    , &pcm_stats_fops);
   CREATE_DEBUGFS_FILE("dumpReadBuf", &pcm_dumpReadBuf_fops);
   CREATE_DEBUGFS_FILE("loglevel" , &pcm_loglevel_fops);

#if !(defined(CONFIG_BRCM_SMC_BOOT))
   pcm_set_endianness();
#endif

   return 0;
err_full_cleanup:
   device_destroy(dhdev.class, MKDEV(majorNumber, 0));
err_cdev_cleanup:
   class_unregister(dhdev.class);
   class_destroy(dhdev.class);
err_class_cleanup:
   unregister_chrdev(majorNumber, DEVICE_NAME);
err_task_cleanup:
   term_taskMain();
   return ret;
}

static void __exit pcmhal_deinit(void)
{
   int i;
   term_taskMain();
   /* Disable all active channels and cleanup */
   for(i = 0; i < ARRAY_SIZE(chan_cfg); i++)
   {
      dh_set_channel_enable(i, 0);
      dh_deconfigure_channel(i);
   }
   pcm_deinit();
   device_destroy(dhdev.class, MKDEV(majorNumber, 0));
   class_unregister(dhdev.class);
   class_destroy(dhdev.class);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   PHLOG_NOTICE("%s:%d pcmhal exited!\x1b[0m\n",  __FUNCTION__ , __LINE__);
}
module_init(pcmhal_init);
module_exit(pcmhal_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BRCM PCM Verification driver");
