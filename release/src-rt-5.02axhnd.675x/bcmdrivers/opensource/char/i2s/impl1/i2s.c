/****************************************************************************
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
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
***************************************************************************/
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i3.cfg

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <board.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <pmc_drv.h>
#include <BPCM.h>
#ifdef __arm__
#include <mach/hardware.h>
#endif
#include "i2s.h"

/**************************************************************************** 
 *  i2s driver required data format:
 *  --------------------------------
 *  All samples that need to be sent out over the i2s bus need to be aligned
 *  to the MSB of a 32-bit word. The following diagrams show how 32, 24 and 
 *  16-bit samples need to be aligned ( X is a dummy byte, must be ZERO ):
 *
 *  32-bit LE data:
 *  0     1     2     3     4     5     6     7     8
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *  | LSB |byte1|byte2| MSB | LSB |byte1|byte2| MSB |
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *      Left Channel Data   |  Right Channel Data
 *  
 *  24-bit LE data:
 *  0     1     2     3     4     5     6     7     8
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *  |  X  | LSB |byte1| MSB |  X  | LSB |byte1| MSB |
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *      Left Channel Data   |  Right Channel Data
 *      
 *  16-bit LE data:
 *  0     1     2     3     4     5     6     7     8
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *  |  X  |  X  | LSB | MSB |  X  |  X  | LSB | MSB |
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *      Left Channel Data   |  Right Channel Data
 *
 *  32-bit BE data:
 *  0     1     2     3     4     5     6     7     8
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *  | MSB |byte2|byte2| LSB | MSB |byte2|byte2| LSB |
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *      Left Channel Data   |  Right Channel Data
 *  
 *  24-bit BE data:
 *  0     1     2     3     4     5     6     7     8
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *  | MSB |byte1| LSB |  X  | MSB |byte1| LSB |  X  |
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *      Left Channel Data   |  Right Channel Data
 *      
 *  16-bit BE data:
 *  0     1     2     3     4     5     6     7     8
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *  | MSB | LSB |  X  |  X  | MSB | LSB |  X  |  X  |
 *  +-----+-----+-----+-----+-----+-----+-----+-----+
 *      Left Channel Data   |  Right Channel Data
 *
 ***************************************************************************/
 
/****************************************************************************
* Macro Definitions / DEFINES
****************************************************************************/

/* Enable PLL clock source */
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856) 
#define I2S_TRY_PLL_CLK         1
#else
#define I2S_TRY_PLL_CLK         0
#endif  

/* Debug controls */
#define I2S_DEBUG_ENABLE        0 /* Master Debug control */
#define I2S_API_DEBUG           0
#define I2S_ISR_DEBUG           0
#define I2S_TASKLET_DEBUG       0
#define I2S_DESC_DEBUG          0
#define I2S_PLL_DEBUG           0

#undef I2S_DEBUG             /* undef it, just in case */
#if I2S_DEBUG_ENABLE		
#   define I2S_DEBUG_LEVEL	KERN_DEBUG
#   define I2S_DEBUG(fmt, args...) printk( I2S_DEBUG_LEVEL "i2s_drv: " fmt, ## args)
#else
#   define I2S_DEBUG(fmt, args...) /* not debugging: nothing */
#endif

/* Operational controls */
#define I2S_PROCESS_DESC_IN_ISR 0 
#define I2S_SINE_TEST   	0
#define I2S_MAX_WAIT_CNT        100

#if I2S_SINE_TEST
#define I2S_SINEPERIOD_NUM_SAMPLES   32
#endif

#define I2S_PROC_ENTRY_ROOT "driver/i2s"
#define I2S_PROC_ENTRY "sampling_freq"

struct i2s_dma_desc
{
   dma_addr_t     dma_addr;         /* DMA address to be passed to h/w */  
   char *         buffer_addr;      /* Buffer address */
   unsigned int   dma_len;          /* Length of dma transfer */ 
   struct list_head tx_queue_entry;   
};

struct i2s_sysclk_freq_map
{
   unsigned int freq;               /* Desired sampling frequency */
   unsigned int mclk_rate;          /* mclk/2*bclk = mclk_rate */
   unsigned int clk_sel;            /* The mclk frequency */
};

struct i2s_pllclk_freq_map
{
   unsigned int freq;               /* Desired sampling frequency */
   unsigned int mclk_rate;          /* mclk/2*bclk = mclk_rate */
#if defined(CONFIG_BCM94908)   
   unsigned int mclk_sel;           /* Divide ratio selector for mclk to i2s */
   unsigned int pad_mclk_sel;       /* Divide ratio selector for mclk to pad */
   unsigned int pdiv;               /* Pll config */
   unsigned int ndiv_int;           /* Pll config */        
   unsigned int ndiv_frac;          /* Pll config */ 
   unsigned int mdiv;               /* Pll config */
#endif
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856) 
   unsigned int prg_clk_div;        /* Programmable clock divider cfg */ 
   unsigned int prg_clk_denom;      /* Programmable clock divider cfg */ 
   unsigned int prg_clk_sel;        /* Programmable clock divider cfg */ 
#endif
};

typedef int (*i2s_pll_clksrc_init_funcptr)( struct i2s_pllclk_freq_map * freq_map_ptr );

/****************************************************************************
* Function Prototypes
****************************************************************************/
static long i2s_ioctl( struct file *flip, unsigned int command, unsigned long arg );
ssize_t i2s_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);    
static int i2s_open( struct inode *inode, struct file *filp );
static int i2s_close( struct inode *inode, struct file *filp );
static irqreturn_t i2s_dma_isr(int irq, void *dev_id);
struct i2s_sysclk_freq_map * i2s_get_sys_freq_map( unsigned int frequency );
#if I2S_TRY_PLL_CLK
static int init_i2s_pll_clksrc(int freq);
#if defined(CONFIG_BCM94908)   
static int init_i2s_pll( struct i2s_pllclk_freq_map * freq_map_ptr );
#endif
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856) 
static int init_i2s_prg_clk_div( struct i2s_pllclk_freq_map * freq_map_ptr );
#endif
#endif

static void deinit_tx_desc_queue(void);
static void init_tx_desc_queue(void);
void enqueue_pending_tx_desc(struct i2s_dma_desc * desc);
struct i2s_dma_desc * dequeue_pending_tx_desc(void);
struct i2s_dma_desc * get_free_dma_desc_buffer(int length);
void put_free_dma_desc_buffer( struct i2s_dma_desc * desc );
static int i2s_create_proc_entries( void );
static int i2s_remove_proc_entries( void );

/****************************************************************************
* Local Variables
****************************************************************************/
dev_t i2s_devId;
static struct cdev     i2s_cdev;
static struct device   *i2s_device       = NULL;
static struct class    *i2s_cl           = NULL;
static struct i2s_dma_desc tx_desc_queue_head;
struct semaphore desc_available_sem;
static DEFINE_SPINLOCK(tx_queue_lock);
static DEFINE_MUTEX(i2s_cfg_mutex);
static int i2s_opened = 0;
static int i2s_write_in_progress = 0;
static struct proc_dir_entry *i2s_proc_entry;

#if !I2S_PROCESS_DESC_IN_ISR   
void i2s_dma_tasklet (unsigned long unused);
DECLARE_TASKLET(i2s_dma_tlet, i2s_dma_tasklet, 0);
#endif


static const struct file_operations i2s_fops = {
    .owner =    THIS_MODULE,
    .write =    i2s_write,
    .unlocked_ioctl = i2s_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = i2s_ioctl,
#endif
    .open =     i2s_open,
    .release =    i2s_close,
};

#if I2S_SINE_TEST
static unsigned int freq = 16000;
#else
static unsigned int freq = 44100;
#endif

#if I2S_TRY_PLL_CLK
struct i2s_pllclk_freq_map pll_freq_map[] =
{
#if defined(CONFIG_BCM94908)   
  /*  Fs      mclk_rate   clk_sel   pad_mclk_sel   pdiv   ndiv_int   ndiv_frac   mdiv */
  { 8000   ,      2,         2,         2,           1,     0x18,     0x15A08,   0x62}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 16000  ,      2,         1,         1,           1,     0x18,     0x3F91E,   0x94}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 32000  ,      2,         0,         0,           1,     0x18,     0x3F91E,   0x94}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 44100  ,      2,         0,         0,           1,     0x18,     0x62B28,   0x6C}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 48000  ,      2,         0,         0,           1,     0x18,     0x15A08,   0x62}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 88200  ,      2,         0,         0,           1,     0x18,     0x62B28,   0x36}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 96000  ,      2,         0,         0,           1,     0x18,     0x9374C,   0x32}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 176400 ,      2,         0,         0,           1,     0x19,     0x49E88,   0x1C}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 192000 ,      2,         0,         0,           1,     0x19,     0x8F1D4,   0x1A}, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 384000 ,      1,         0,         0,           1,     0x19,     0x8F1D4,   0x1A}, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
#endif
#if defined(CONFIG_BCM963158)   
  /*  fs       mclk_rate     div      denom         sel                */
  { 8000  ,       1,        15625,     64,      CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
  { 16000 ,       1,        15625,     128,     CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
  { 32000 ,       3,        15625,     768,     CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 384 * Fs */     
  { 44100 ,       2,        78125,     3528,    CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 48000 ,       2,        15625,     768,     CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 256 * Fs */     
  { 88200 ,       1,        78125,     3528,    CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
  { 96000 ,       1,        15625,     768,     CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
  { 176400,       1,        45274,     4089,    CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
  { 192000,       1,        15625,     1536,    CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */     
  { 384000,       1,        15625,     3072,    CLK_SEL_250MHZ_SYNCE_PLL }, /*  Req Bclk: 64 * Fs , Mclk: 128 * Fs */ 

#endif
#if defined (CONFIG_BCM96856)
  /*  fs       mclk_rate     div      denom         sel      */   
  { 8000  ,       1,        0x3d09,    0x80,      I2S_CLK_PLL },    
  { 11025 ,       1,        0x1312d,   0x372,     I2S_CLK_PLL },
  { 12000 ,       1,        0x3d09,    0xc0,      I2S_CLK_PLL },   
  { 16000 ,       1,        0x3d09,    0x100,     I2S_CLK_PLL },    
  { 22050 ,       1,        0x1312d,   0x6e4,     I2S_CLK_PLL },
  { 24000 ,       1,        0x3d09,    0x180,     I2S_CLK_PLL },
  { 32000 ,       1,        0x3d09,    0x200,     I2S_CLK_PLL },   
  { 44100 ,       1,        0x1312d,   0xdc8,     I2S_CLK_PLL },  
  { 48000 ,       1,        0x3d09,    0x300,     I2S_CLK_PLL },
  { 96000 ,       1,        0x3d09,    0x600,     I2S_CLK_PLL },  
  { 192000 ,      1,        0x3d09,    0xC00,     I2S_CLK_PLL },  
  { 384000 ,      1,        0x3d09,    0x1800,    I2S_CLK_PLL },        
#endif                                                      
  { 0      ,      0, },
};
#endif

struct i2s_sysclk_freq_map freq_map[] =
{  
  /*  Fs      mclk_rate       clk_sel  */
   { 16000  ,    12,      I2S_CLK_25MHZ }, /*  Req Bclk: 1,024,000  , Actual Bclk: 1,041,667  */
   { 32000  ,    12,      I2S_CLK_50MHZ }, /*  Req Bclk: 2,048,000  , Actual Bclk: 2,083,333  */
   { 44100  ,    9 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 2,822,400  , Actual Bclk: 2,777,778  */
   { 48000  ,    8 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 3,072,000  , Actual Bclk: 3,125,000  */
   { 96000  ,    4 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 6,144,000  , Actual Bclk: 6,250,000  */
   { 192000 ,    2 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 12,288,000 , Actual Bclk: 12,500,000 */
   { 384000 ,    1 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 24,576,000 , Actual Bclk: 25,000,000 */
   { 0      ,    0 ,      0             }, 
};                                                              

#if I2S_SINE_TEST
/* One, i2s-formatted, period of a sinewave sampled at 16-bits @ 16Khz Stereo */
unsigned int i2s_sine_16bit_16khz[I2S_SINEPERIOD_NUM_SAMPLES] = 
{      
   0x00000000, 0xFFFF0000, 0x188C0000, 0x188D0000, 0x2D5D0000, 0x2D5C0000, 0x3B440000, 0x3B440000, 
   0x40260000, 0x40270000, 0x3B440000, 0x3B440000, 0x2D5C0000, 0x2D5D0000, 0x188D0000, 0x188C0000, 
   0x00000000, 0xFFFF0000, 0xE7730000, 0xE7730000, 0xD2A30000, 0xD2A30000, 0xC4BC0000, 0xC4BC0000, 
   0xBFD90000, 0xBFD90000, 0xC4BC0000, 0xC4BC0000, 0xD2A30000, 0xD2A40000, 0xE7730000, 0xE7740000,
};
#endif


/****************************************************************************
* Static functions
****************************************************************************/

/* Get free tx descriptor */
struct i2s_dma_desc * get_free_dma_desc_buffer(int length)
{
   struct i2s_dma_desc * new_desc = NULL;
         
   new_desc = kzalloc( sizeof(struct i2s_dma_desc), GFP_NOWAIT );
   if( new_desc )
   {
      new_desc->buffer_addr = kzalloc( length, GFP_NOWAIT );
      if( new_desc->buffer_addr )
      {
         new_desc->dma_len = length;
#if I2S_DESC_DEBUG   
         I2S_DEBUG("%s: Allocated desc:%p buffer:%p\n", __FUNCTION__, 
                  new_desc, new_desc->buffer_addr);            
#endif	 
      }
      else
      {
         kfree(new_desc);
         new_desc = NULL;
      }
   }
   
   return new_desc;      
}

void put_free_dma_desc_buffer( struct i2s_dma_desc * desc )
{
   if( desc )
   {
#if I2S_DESC_DEBUG   
      I2S_DEBUG("%s: Freeing   desc:%p buffer:%p\n", __FUNCTION__, 
                  desc, desc->buffer_addr);                  
#endif
      kfree(desc->buffer_addr);
      kfree(desc);
   }         
}

/* De-queue TX desc */
struct i2s_dma_desc * dequeue_pending_tx_desc(void)
{
   struct i2s_dma_desc * tempDesc = NULL;
   unsigned long flags;
   
   spin_lock_irqsave(&tx_queue_lock, flags);   
      
   if( !list_empty(&tx_desc_queue_head.tx_queue_entry) )
   {
      /* Get first entry in list - Implements a queue */
      tempDesc = list_entry(tx_desc_queue_head.tx_queue_entry.next, struct i2s_dma_desc, tx_queue_entry);
      
      /* Delete entry from list */
      list_del(tx_desc_queue_head.tx_queue_entry.next);  
   }
   
   spin_unlock_irqrestore(&tx_queue_lock, flags);   
   
   return tempDesc;
}

/* Enqueue TX desc */
void enqueue_pending_tx_desc(struct i2s_dma_desc * desc)
{
   unsigned long flags;
   spin_lock_irqsave(&tx_queue_lock, flags);   
      
   /* Add entry at end of list - Implements a queue */
   list_add_tail( &desc->tx_queue_entry, &tx_desc_queue_head.tx_queue_entry );
   
   spin_unlock_irqrestore(&tx_queue_lock, flags);   
}

/* Initialize TX desc queue */
static void init_tx_desc_queue(void)
{
   unsigned long flags;
   
   spin_lock_irqsave(&tx_queue_lock, flags);   
   
   INIT_LIST_HEAD(&tx_desc_queue_head.tx_queue_entry);   
   
   spin_unlock_irqrestore(&tx_queue_lock, flags);   
}

/* De-Init TX desc queue */
static void deinit_tx_desc_queue(void)
{
   struct i2s_dma_desc * dma_request_desc = NULL;
   
   /* De-queue 1st descriptor */
   dma_request_desc = dequeue_pending_tx_desc();  
   
   while( dma_request_desc )
   {
      /* Free descriptor and dma buffer*/    
      dma_unmap_single(i2s_device, dma_request_desc->dma_addr, dma_request_desc->dma_len, DMA_TO_DEVICE); 
      put_free_dma_desc_buffer(dma_request_desc);           
            
      /* De-queue next descriptor */
      dma_request_desc = dequeue_pending_tx_desc();  
   }
}
      
/* Get frequency-parameter map */
struct i2s_sysclk_freq_map * i2s_get_sys_freq_map( unsigned int frequency )
{
   struct i2s_sysclk_freq_map * freq_map_ptr = NULL;
   int i;
   
   for( i=0; freq_map[i].freq; i++ )
   {
      if( frequency == freq_map[i].freq )
      {
         freq_map_ptr = &freq_map[i];        
         break;
      }
   }

   I2S_DEBUG("%s: freq:%d mclk_rate:%d clk_sel:%d\n", __FUNCTION__, (int)frequency,
               (int)freq_map_ptr->mclk_rate, (int)freq_map_ptr->clk_sel);                  
      
   return freq_map_ptr;         
}

#if I2S_SINE_TEST
void i2s_copy_sine_data( char * buffer, unsigned int length )
{
   int i,j;
   unsigned int * sample_buffer_ptr = (unsigned int * )buffer;
   int num_iterations = (length/sizeof(unsigned int))/I2S_SINEPERIOD_NUM_SAMPLES;
   
   for( i=0; i<num_iterations; i++ )
   {
      for( j=0; j<I2S_SINEPERIOD_NUM_SAMPLES; j++ )
      {
         *sample_buffer_ptr = i2s_sine_16bit_16khz[j];
         sample_buffer_ptr++;
      }
   }
   
}
#endif

/* I2S ISR */
static irqreturn_t i2s_dma_isr(int irq, void *dev_id)
{  
#if I2S_PROCESS_DESC_IN_ISR   
   dma_addr_t dma_addr  = 0;
   unsigned int dma_len = 0;
   int eop = 0;    
   struct i2s_dma_desc * done_tx_desc;
#endif
   unsigned int int_status = I2S->intr;
      
#if I2S_ISR_DEBUG   
   I2S_DEBUG("%s: Intstat: 0x%08x\n", __FUNCTION__, (unsigned int)I2S->intr);   
#endif   
   
   /* Check if we got the right interrupt */
   if( int_status & I2S_DESC_OFF_INTR )
   {      
#if I2S_PROCESS_DESC_IN_ISR      
      /* Retrieve descriptor */
      dma_addr = I2S->desc_off_addr;
      dma_len  = I2S->desc_off_len;  
      eop = dma_len & I2S_DESC_EOP;    
      dma_len &= ~I2S_DESC_EOP;
      
#if I2S_ISR_DEBUG      
      I2S_DEBUG("%s: pst-clr OFF_DESC_LEVEL:%d\n", __FUNCTION__,  (int)(I2S->intr >> I2S_DESC_OFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK );          
#endif      

      /* Unmap DMA memory */
      if( dma_addr )
         dma_unmap_single(i2s_device, dma_addr, dma_len, DMA_TO_DEVICE);            
      
      /* Free buffer */
      done_tx_desc = dequeue_pending_tx_desc();
      
      /* Release desc and buffer */
      put_free_dma_desc_buffer(done_tx_desc);
      
      /* Check for end of packet */
      if( eop )
      {
         if( !( (I2S->intr >> I2S_DESC_IFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK ) )
         {
            /* Disable I2S interface if no RX descriptors in FIFO */
            //I2S->cfg &= ~I2S_ENABLE;   
         }
      }
           
      /* Give semaphore to indicate available descriptor space */
      up(&desc_available_sem);             
#else
      /* Mask Interrupt - Will be unmasked by tasklet */
      I2S->intr_en &= ~I2S_DESC_OFF_INTR_EN;
      
     /* Schedule tasklet to handle used descriptors */
      tasklet_schedule(&i2s_dma_tlet);
#endif      
   }
   else if ( int_status & I2S_DESC_OFF_OVERRUN_INTR 
          || int_status & I2S_DESC_IFF_UNDERRUN_INTR )
   {
      printk(KERN_WARNING  "%s: Underrun/Overruns detected 0x%08x\n", __FUNCTION__, int_status);      
   }
   
   /* Clear interrupt by writing 0 */
   I2S->intr &= ~I2S_INTR_MASK;
      
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
   // Clear the interrupt
   BcmHalInterruptEnable (irq);
#endif
   
   return IRQ_HANDLED;   
}
 
#if !I2S_PROCESS_DESC_IN_ISR     
void i2s_dma_tasklet(unsigned long unused)
{
   dma_addr_t dma_addr  = 0;
   unsigned int dma_len = 0;
   int eop = 0;    
   int i=0;
   struct i2s_dma_desc * done_tx_desc;
   unsigned int int_status = I2S->intr;

   /* Loop until OFF fifo level drops to zero or for I2S_DESC_FIFO_DEPTH cycles */
   while( ( (int)(int_status >> I2S_DESC_OFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK ) && i < I2S_DESC_FIFO_DEPTH )         
   {
#if I2S_TASKLET_DEBUG      
      I2S_DEBUG("%s: pre-clr OFF_DESC_LEVEL:%d\n", __FUNCTION__,  (int)(I2S->intr >> I2S_DESC_OFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK );          
#endif    
  
      /* Retrieve descriptor */
      dma_addr = I2S->desc_off_addr;
      dma_len  = I2S->desc_off_len;  
      eop = dma_len & I2S_DESC_EOP;    
      dma_len &= ~I2S_DESC_EOP;
      
#if I2S_TASKLET_DEBUG      
      I2S_DEBUG("%s: pst-clr OFF_DESC_LEVEL:%d\n", __FUNCTION__,  (int)(int_status >> I2S_DESC_OFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK );          
#endif      
   
      /* Unmap DMA memory */
      if( dma_addr )
         dma_unmap_single(i2s_device, dma_addr, dma_len, DMA_TO_DEVICE);            
      
      /* Free buffer */
      done_tx_desc = dequeue_pending_tx_desc();
      
      /* Release desc and buffer */
      put_free_dma_desc_buffer(done_tx_desc);
      
      /* Check for end of packet */
      if( eop )
      {
         if( !( (int_status >> I2S_DESC_IFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK ) )
         {
            /* Disable I2S interface if no RX descriptors in FIFO */
            //I2S->cfg &= ~I2S_ENABLE;   
         }
      }
           
      /* Give semaphore to indicate available descriptor space */
      up(&desc_available_sem);                
      
      /* Increment loop counter */
      i++;
      
      /* Update interrupt status */
      int_status = I2S->intr;
   }
   
   /* Unmask interrupt */
   I2S->intr_en |= I2S_DESC_OFF_INTR_EN;
}
#endif

static int init_i2s_sys_based_clk(int freq)
{
   struct i2s_sysclk_freq_map * freq_map_ptr = NULL;

#if I2S_API_DEBUG   
   I2S_DEBUG("%s\n", __FUNCTION__);
#endif   

   /* Based on sampling frequency, choose MCLK and *
    * select divide ratio for required BCLK        */
   freq_map_ptr = i2s_get_sys_freq_map( freq );
   
   if( freq_map_ptr )
   {
      I2S->cfg &= I2S_MCLK_CLKSEL_CLR_MASK;
      I2S->cfg |= freq_map_ptr->mclk_rate << I2S_MCLK_RATE_SHIFT;
      I2S->cfg |= freq_map_ptr->clk_sel   << I2S_CLKSEL_SHIFT;     
      
      return(0);
   }

   return (-EINVAL);
}

#if I2S_TRY_PLL_CLK
#if defined(CONFIG_BCM963158)   
static int init_i2s_prg_clk_div( struct i2s_pllclk_freq_map * freq_map_ptr )
{
   /* Reset prg clk divider*/
   NTR_RESET->NtrResetReg &= ~CLK_DIV_RST_N;
   msleep(1);
   NTR_RESET->NtrResetReg = CLK_DIV_RST_N;

   /* Program div */
   NTR_CLK_PRG_SWCH->I2sPrgDivCfg1 &= ~CLK_SEL_MASK;
   NTR_CLK_PRG_SWCH->I2sPrgDivCfg1 = freq_map_ptr->prg_clk_sel << CLK_SEL_SHIFT;
   NTR_CLK_PRG_SWCH->I2sPrgDivCfg1 |= freq_map_ptr->prg_clk_div & CLK_DIV_MASK;

   /* Program denom */
   NTR_CLK_PRG_SWCH->I2sPrgDivCfg2 = freq_map_ptr->prg_clk_denom & CLK_DENOM_MASK;
   msleep(1);

   return 0;

}
#endif /* defined(CONFIG_BCM963158) */

#if defined(CONFIG_BCM96856) 
static int init_i2s_prg_clk_div( struct i2s_pllclk_freq_map * freq_map_ptr )
{
   unsigned long reg_val;
   unsigned long xrdp_base = bcm_io_block_address[XRDP_IDX];

   *((volatile uint32_t *)(xrdp_base + EPON_TOP_RESET_OFFSET)) |= 0x8; /* Active Low reset*/

    /* Program div */
   reg_val = ~CLK_SEL_MASK;
   *((volatile uint32_t *)(xrdp_base + EPON_CLK_PRG_CFG)) = reg_val;
   reg_val = (freq_map_ptr->prg_clk_div & CLK_DIV_MASK);
   *((volatile uint32_t *)(xrdp_base + EPON_CLK_PRG_CFG)) = reg_val;

   /* Program denom */
   reg_val = freq_map_ptr->prg_clk_denom & CLK_DENOM_MASK;
   *((volatile uint32_t *)(xrdp_base + EPON_CLK_PRG_CFG2)) = reg_val;
   msleep(1);

   return 0;
}
#endif /* defined(CONFIG_BCM96856)  */

#if defined(CONFIG_BCM94908)   
static int init_i2s_pll( struct i2s_pllclk_freq_map * freq_map_ptr )
{
   
   unsigned int val, fref;
   unsigned int PLL_CTRL_LOW  = 0x00C00000;
   unsigned int PLL_CTRL_HIGH = 0x00000000;

   unsigned int ki = 0x4; //Fref = 100 since pdiv = 0 & VCO low band [1.2, 2.2]
   unsigned int ka = 0;   //Not used
   unsigned int kp = 0x8; //Fref = 100 

   int numWaitLoop = 10;  //Number of times to wait for PLL_LOCK

   fref = ( (freq_map_ptr->pdiv == 0) ? 50*2 : 50/freq_map_ptr->pdiv);

   kp =  ((fref >= 50)  & (fref < 75) ) ? 0x8 :
         ((fref >= 75)  & (fref < 100)) ? 0x9 :
         ((fref >= 100) & (fref < 125)) ? 0xa  : 0x8;

#if I2S_PLL_DEBUG
   I2S_DEBUG("I2S_PLL Programing start\n");
   I2S_DEBUG("---------PLL SETTINGS-------\n"); 
   I2S_DEBUG("I2S_PLL freff           : %d \n", fref);
   I2S_DEBUG("I2S_PLL kp              : %d \n", kp);
   I2S_DEBUG("I2S_PLL freq            : %d \n", freq_map_ptr->freq);                
   I2S_DEBUG("I2S_PLL mclk_sel        : %d \n", freq_map_ptr->mclk_sel);   
   I2S_DEBUG("I2S_PLL pad_mclk_sel    : %d \n", freq_map_ptr->pad_mclk_sel);
   I2S_DEBUG("I2S_PLL mclk_rate       : %d \n", freq_map_ptr->mclk_rate);   
   I2S_DEBUG("I2S_PLL pdiv            : %d \n", freq_map_ptr->pdiv);        
   I2S_DEBUG("I2S_PLL ndiv_int        : 0x%02x \n", freq_map_ptr->ndiv_int);    
   I2S_DEBUG("I2S_PLL ndiv_frac       : 0x%04x \n", freq_map_ptr->ndiv_frac);   
   I2S_DEBUG("I2S_PLL mdiv            : 0x%02x \n", freq_map_ptr->mdiv);        
   I2S_DEBUG("----------------------------\n"); 
#endif   

   /* power on the pll */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), &val);
   val = val | I2SPLL_PLL_RESETS_PWR_ON_MASK | I2SPLL_PLL_RESETS_LDO_PWR_ON_MASK 
      | I2SPLL_PLL_RESETS_PWR_ON_BG_MASK;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), val);

   /* reset the PLL (reset both resetb and post_resetb) */
   /*  A write '1' will make the resetb & post_resetb to '0' in RTL ...Be Careful!!! */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), &val);
   val = val | I2SPLL_PLL_RESETS_RESETB_MASK | I2SPLL_PLL_RESETS_POST_RESETB_MASK;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), val);

   /* program misc control field */
   val = val & ~(I2SPLL_PLL_RESETS_PLL_MISC_CTRL_MASK);
   val = val | (((freq_map_ptr->pad_mclk_sel << 2) | freq_map_ptr->mclk_sel) 
      << I2SPLL_PLL_RESETS_PLL_MISC_CTRL_SHIFT);
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), val);

   /* program the override bit */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(pdiv), &val);
   val = val | 0x80000000;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(pdiv), val);

   /* program the freq_map_ptr->pdiv, ndiv and frac value */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(pdiv), &val);
   val = val & 0xFFFFFFF0;
   val = val | freq_map_ptr->pdiv;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(pdiv), val);

   /* program freq_map_ptr->ndiv_int and freq_map_ptr->ndiv_frac */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ndiv), &val);
   val = val & ~(I2SPLL_PLL_NDIV_NDIV_INT_MASK) & ~(I2SPLL_PLL_NDIV_NDIV_FRAC_MASK);
   val = val | (freq_map_ptr->ndiv_int << I2SPLL_PLL_NDIV_NDIV_INT_SHIFT) 
      | (freq_map_ptr->ndiv_frac << I2SPLL_PLL_NDIV_NDIV_FRAC_SHIFT);
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ndiv), val);

   /* override en for the freq_map_ptr->mdiv values */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch01_cfg), &val);
   val = val | 0x80008000;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch01_cfg), val);

   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch23_cfg), &val);
   val = val | 0x80008000;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch23_cfg), val);

   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch45_cfg), &val);
   val = val | 0x80008000;
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch45_cfg), val);

   /* set new freq_map_ptr->mdiv values */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch01_cfg), &val);
   val = val & 0xFF00FF00;
   val = val | freq_map_ptr->mdiv | ( (freq_map_ptr->mdiv+1) << 16); 
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch01_cfg), val);

   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch23_cfg), &val);
   val = val & 0xFF00FF00;
   val = val | (freq_map_ptr->mdiv+2) | ( (freq_map_ptr->mdiv+3) << 16); 
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch23_cfg), val);

   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch45_cfg), &val);
   val = val & 0xFF00FF00;
   val = val | (freq_map_ptr->mdiv+4) | ( (freq_map_ptr->mdiv+5) << 16); 
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch45_cfg), val);

   /* program the control register */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(cfg0), &val);
   val = val & 0x0;
   val = val | PLL_CTRL_LOW; 
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(cfg0), val);

   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(cfg1), &val);
   val = val & 0x0;
   val = val | PLL_CTRL_HIGH; 
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(cfg1), val);

   /* program the KI,KA,KP values */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(loop0), &val);
   val = val & 0xFFFF088F;
   val = val | ( ka << I2SPLL_SS_LOOP0_SS_KA_SHIFT) | ( ki << I2SPLL_SS_LOOP0_SS_KI_SHIFT) 
      | ( kp << I2SPLL_SS_LOOP0_SS_KP_SHIFT ); 
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(loop0), val);

   /* make master_reset '0' */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), &val);
   val = val & ~(I2SPLL_PLL_RESETS_MASTER_RESET_MASK);
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), val);

   /* release the resetb for PLL */
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), &val);
   val = val & ~(I2SPLL_PLL_RESETS_RESETB_MASK);
   WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), val);

#if I2S_PLL_DEBUG
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(id_reg), &val);
   I2S_DEBUG("I2S_PLL id_reg:       0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(capabilities), &val);
   I2S_DEBUG("I2S_PLL capabilities: 0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), &val);
   I2S_DEBUG("I2S_PLL resets:       0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(cfg0), &val);
   I2S_DEBUG("I2S_PLL cfg0:         0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(cfg1), &val);
   I2S_DEBUG("I2S_PLL cfg1:         0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ndiv), &val);
   I2S_DEBUG("I2S_PLL ndiv:         0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(pdiv), &val);
   I2S_DEBUG("I2S_PLL pdiv:         0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(loop0), &val);
   I2S_DEBUG("I2S_PLL loop0:        0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(loop1), &val);
   I2S_DEBUG("I2S_PLL loop1:        0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch01_cfg), &val);
   I2S_DEBUG("I2S_PLL ch01_cfg:     0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch23_cfg), &val);
   I2S_DEBUG("I2S_PLL ch23_cfg:     0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(ch45_cfg), &val);
   I2S_DEBUG("I2S_PLL ch45_cfg:     0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(octrl), &val);
   I2S_DEBUG("I2S_PLL octrl:        0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(stat), &val);
   I2S_DEBUG("I2S_PLL stat:         0x%08x\n", (unsigned int)val);
   ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(strap), &val);
   I2S_DEBUG("I2S_PLL strap:        0x%08x\n", (unsigned int)val);
#endif /* I2S_PLL_DEBUG */

   do
   {
      ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(stat), &val);
      val = val & 0x80000000;
      msleep(1);
      numWaitLoop--;
   }
   while (!(val) && numWaitLoop);
  
   if( val )
   {
      I2S_DEBUG("I2S_PLL Locked!! stat:0x%08x\n", (unsigned int)val);

      ReadBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), &val);
      val = val & ~(I2SPLL_PLL_RESETS_POST_RESETB_MASK);
      WriteBPCMRegister(PMB_ADDR_I2SPLL, PLLBPCMRegOffset(resets), val);

      return 0;
   }

   printk(KERN_ERR "I2S_PLL NOT Locked!! stat:0x%08x\n", (unsigned int)val);

   return (-EFAULT);

}
#endif /* defined(CONFIG_BCM94908) */

struct i2s_pllclk_freq_map * i2s_get_pll_freq_map( unsigned int frequency, i2s_pll_clksrc_init_funcptr * init_func )
{
   struct i2s_pllclk_freq_map * freq_map_ptr = NULL;
   int i;
   
   for( i=0; pll_freq_map[i].freq; i++ )
   {
      if( frequency == pll_freq_map[i].freq )
      {
         freq_map_ptr = &pll_freq_map[i];        
         break;
      }
   }

   /* Set the init function */
   if ( freq_map_ptr )
   {
      I2S_DEBUG("%s: freq:%d mclk_rate:%d\n", __FUNCTION__, (int)frequency,
                  (int)freq_map_ptr->mclk_rate);                  

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856)  
      /* Initilize the programmable clock divider which sources the clock */
      *init_func = init_i2s_prg_clk_div;
#endif      
#if defined(CONFIG_BCM94908)   
      /* Initilize the PLL which sources the clock */
      *init_func = init_i2s_pll;
#endif      
   }
   else
   {
      I2S_DEBUG("%s: Cannot find PLL mapping for freq:%d \n", __FUNCTION__, (int)frequency);
   }
     
   return freq_map_ptr;         
}

static int init_i2s_pll_clksrc(int freq)
{
   struct i2s_pllclk_freq_map * freq_map_ptr = NULL;
   i2s_pll_clksrc_init_funcptr i2s_pll_clksrc_init_func = NULL;

#if I2S_API_DEBUG   
   I2S_DEBUG("%s\n", __FUNCTION__);
#endif   

   /* Based on sampling frequency, choose MCLK and *
    * select divide ratio for required BCLK        */
   freq_map_ptr = i2s_get_pll_freq_map(freq, &i2s_pll_clksrc_init_func);
   
   if(freq_map_ptr && i2s_pll_clksrc_init_func && (i2s_pll_clksrc_init_func(freq_map_ptr) == 0))
   {
      I2S->cfg &= I2S_MCLK_CLKSEL_CLR_MASK;
      I2S->cfg |= freq_map_ptr->mclk_rate << I2S_MCLK_RATE_SHIFT;
      I2S->cfg |= I2S_CLK_PLL  << I2S_CLKSEL_SHIFT;     
      
      return(0);
   }

   return (-EINVAL);

}
#endif

static int i2s_close( struct inode *inode, struct file *filp )
{
   int i = 0;
#if I2S_API_DEBUG   
   I2S_DEBUG("%s\n", __FUNCTION__);
#endif   

   /* Wait till writes complete */
   while( ( (int)(I2S->intr >> I2S_DESC_OFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK ) ||
          ( (int)(I2S->intr >> I2S_DESC_IFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK ) )
   {
      msleep(10);
      i++;
      if( i == I2S_MAX_WAIT_CNT )
      {
         printk(KERN_ERR "%s: Timed out waiting for I2S writes to end!\n", __FUNCTION__);
         break;
      }
   }

   /* Aquire configuration mutex */
   mutex_lock(&i2s_cfg_mutex);
   
   /* Clear opened flag */
   i2s_opened = 0; 
   
   /* Release configuration mutex */
   mutex_unlock(&i2s_cfg_mutex);         

   return 0;
}

/* i2s_open: Basic register initialization */
static int i2s_open( struct inode *inode, struct file *filp )
{
   int ret = -EINVAL;
  
#if I2S_API_DEBUG   
   I2S_DEBUG("%s\n", __FUNCTION__);
#endif   

   /* Acquire configuration mutex */
   mutex_lock(&i2s_cfg_mutex);
   
   /* Driver not re-entrant */
   if( i2s_opened )
   {
      ret = -EBUSY;  
   }
   else
   {         
      /* Disable I2S interface */
      I2S->cfg &= ~I2S_ENABLE;   
      
      /* Clear and disable I2S interrupts ( by writing 0 ) */
      I2S->intr &= ~I2S_INTR_MASK;
      I2S->intr_en = 0;   
         
      /* Clear DMA interrupt thresholds */   
      I2S->intr_iff_thld = 0;   
      I2S->intr_off_thld = 0;
         
      /* Setup I2S as follows ( Fs = sampling frequency ):                    *
       * 64Fs BCLK, leftChannel=0, rightchannel=1, falling BCLK,LRCLK low for *
       * left, Data delayed by 1 BCLK from LRCLK transition, MSB justified    */
      I2S->cfg |= I2S_OUT_R;                                                       
      I2S->cfg &= ~I2S_OUT_L;                                                    
      I2S->cfg |= 2 << I2S_SCLKS_PER_1FS_DIV32_SHIFT;                            
      I2S->cfg |= I2S_BITS_PER_SAMPLE_32 << I2S_BITS_PER_SAMPLE_SHIFT;         
      I2S->cfg &= ~I2S_SCLK_POLARITY;                                         
      I2S->cfg &= ~I2S_LRCK_POLARITY;                                         
      I2S->cfg |= I2S_DATA_ALIGNMENT;                                         
      I2S->cfg &= ~I2S_DATA_JUSTIFICATION;                                    
      I2S->cfg |= I2S_DATA_ENABLE;                                            
      I2S->cfg |= I2S_CLOCK_ENABLE;                                              
          
      /* Set DMA interrupt thresholds */   
      I2S->intr_iff_thld = 0;   
      I2S->intr_off_thld = 0;
           
      /* Enable off interrupts - interrupt when output fifo level is over 0 */
      I2S->intr_en &= ~I2S_DESC_INTR_TYPE_SEL;      
      I2S->intr_en |= I2S_DESC_OFF_INTR_EN;

#if I2S_TRY_PLL_CLK
      /* Try to initialize i2s clocks using pll clock source */
      ret = init_i2s_pll_clksrc(freq);
#endif      

      /* If pll based clk not available, try predefined system clks */
      if( ret < 0 )
      {
         ret = init_i2s_sys_based_clk(freq);
      }
         
      if( ret == 0 )
      {
         /* Initialize semaphore */
         sema_init(&desc_available_sem, I2S_DESC_FIFO_DEPTH); 
                  
         /* Initialize pending descriptor queue */
         init_tx_desc_queue();      
      }
      else
      {
         printk(KERN_ERR "%s: Unsupported frequency %d\n", __FUNCTION__, freq);
      }
   }

   /* Set flag to indicate driver has been opened */
   i2s_opened = 1;
   
   /* Release configuration mutex */
   mutex_unlock(&i2s_cfg_mutex);      

   return ret;   
}
 
/* i2s_write: Send data via DMA */
ssize_t i2s_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)    
{
   char * user_buff = (char*)buf;
   unsigned int byte_count = count;
   struct i2s_dma_desc * new_desc;
   unsigned int xfer_length;
   int ret = 0;
               
   /* Acquire configuration mutex */
   mutex_lock(&i2s_cfg_mutex);
   
   if( i2s_write_in_progress )
      return -EBUSY;
   else
      i2s_write_in_progress = 1;
      
   /* Release configuration mutex */
   mutex_unlock(&i2s_cfg_mutex);         
              
#if I2S_SINE_TEST
   /* Adjust byte_count so that we can stuff the maximum number of sine wave periods in a dma buffer */
   byte_count =  I2S_DMA_BUFF_MAX_LEN - (I2S_DMA_BUFF_MAX_LEN%(I2S_SINEPERIOD_NUM_SAMPLES * sizeof(unsigned int)));                 
#endif   

   /* Submit dma transfers */
   while( byte_count )
   {
#if I2S_API_DEBUG      
      I2S_DEBUG("%s: Count:%d\n", __FUNCTION__, byte_count); 
#endif      
      
      /* Ensure we only continue if we have descriptor space */
      if(down_interruptible(&desc_available_sem))	 
      {
         ret = -ERESTARTSYS;   
         break;
      }
      
      /* Calculate transfer length */
      xfer_length = ((byte_count>I2S_DMA_BUFF_MAX_LEN)?I2S_DMA_BUFF_MAX_LEN:byte_count);
      new_desc = get_free_dma_desc_buffer(xfer_length);
      if( new_desc )
      {        
#if I2S_DESC_DEBUG   
         I2S_DEBUG("%s: Bytes Left cur:%d\n", __FUNCTION__, byte_count); 
#endif
      
#if I2S_SINE_TEST
         i2s_copy_sine_data(new_desc->buffer_addr, byte_count);
#else
         /* Copy over user data */
         copy_from_user (new_desc->buffer_addr, user_buff, new_desc->dma_len);
#endif   
         
         /* Increment data pointer */
         user_buff += new_desc->dma_len;
         
#if !I2S_SINE_TEST
         /* Adjust count */
         byte_count -= new_desc->dma_len;
#endif   

#if I2S_DESC_DEBUG   
         I2S_DEBUG("%s: Bytes Left rem:%d\n", __FUNCTION__, byte_count);          
#endif 
         
         /* Map dma buffer */
#if I2S_DESC_DEBUG   
         I2S_DEBUG("%s: First data word:0x%08x\n", __FUNCTION__, *(unsigned int*)new_desc->buffer_addr);          
#endif 
         new_desc->dma_addr = dma_map_single(i2s_device, new_desc->buffer_addr, new_desc->dma_len, DMA_TO_DEVICE);
         if( dma_mapping_error(i2s_device, new_desc->dma_addr) )
         {
            printk(KERN_ERR "%s:dma_map_single Tx failed. Buffer Address: %p, Length: %d, Err:%p\n", 
               __FUNCTION__, new_desc->buffer_addr, new_desc->dma_len, (void*)new_desc->dma_addr);
            put_free_dma_desc_buffer(new_desc);
            ret = -ENOMEM;
            break;
         }   
         
         /* Write descriptor */
         enqueue_pending_tx_desc(new_desc);
         I2S->desc_iff_len = (byte_count>0)? new_desc->dma_len : (new_desc->dma_len | I2S_DESC_EOP) ;
         I2S->desc_iff_addr = new_desc->dma_addr;      
                           
#if I2S_DESC_DEBUG   
         I2S_DEBUG("%s: IFF_DESC_LEVEL:%d\n", __FUNCTION__,  (int)(I2S->intr >> I2S_DESC_IFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK ); 
         I2S_DEBUG("%s: OFF_DESC_LEVEL:%d\n", __FUNCTION__,  (int)(I2S->intr >> I2S_DESC_OFF_LEVEL_SHIFT) & I2S_DESC_LEVEL_MASK );          
#endif	 
                  
         /* Acquire configuration mutex */
         mutex_lock(&i2s_cfg_mutex);
         
         /* Enable I2S interface */
         I2S->cfg |= I2S_ENABLE; 
         
         /* Release configuration mutex */
         mutex_unlock(&i2s_cfg_mutex);         
      }
      else
      {
         printk(KERN_ERR "%s:Couldnt get free tx descriptor\n", __FUNCTION__); 
         ret = -ENOMEM; 
         break;
      }
   }
   
   /* Acquire configuration mutex */
   mutex_lock(&i2s_cfg_mutex);
   
   /* Write complete */
   i2s_write_in_progress = 0;
      
   /* Release configuration mutex */
   mutex_unlock(&i2s_cfg_mutex);         
   
   if( ret )
   {
      return ret;
   }
   else
   {
      *f_pos += count;
      return count;
   }
}
   
static long i2s_ioctl( struct file *flip, unsigned int command, unsigned long arg )
{
   unsigned long target_freq = arg;
   int ret = -EINVAL;

#if I2S_API_DEBUG
   I2S_DEBUG("%s: Cmd: %d\n", __FUNCTION__, command);
#endif   

   /* Acquire configuration mutex */
   mutex_lock(&i2s_cfg_mutex);      
   
   if( !i2s_write_in_progress )
   {         
      if( command == I2S_SAMPLING_FREQ_SET_IOCTL )
      {            
         if( (I2S->cfg & I2S_DATA_ENABLE) && (I2S->cfg & I2S_CLOCK_ENABLE) )
         {
#if I2S_TRY_PLL_CLK
            /* Try to initialize i2s clocks using pll clock source */
            ret = init_i2s_pll_clksrc(target_freq);
#endif    
      
            /* If pll clk src not available, try predefined system clks */
            if( ret < 0 )
            {
               ret = init_i2s_sys_based_clk(target_freq);
            }
               
            if( ret == 0 )
            {
               freq = target_freq;
               I2S_DEBUG("%s: Setting Sampling Freq:%d\n", __FUNCTION__,  freq );       
            }
            else
            {
               printk(KERN_ERR "%s: Unsupported frequency %lu\n", __FUNCTION__, target_freq);
            }
         }
         else
         {
            printk(KERN_ERR "%s: Driver not initialized\n", __FUNCTION__);      
            ret = -EINVAL; 
         }     
      }
      else
      {
         //printk(KERN_ERR "%s: Unsupported command %d\n", __FUNCTION__, command);
         ret = -EINVAL; 
      } 
   }       
   else
   {
      ret = -EBUSY;       
   }
           
   /* Release configuration mutex */
   mutex_unlock(&i2s_cfg_mutex); 
                 
   return ret; 
}

static ssize_t i2s_write_freq(struct file *file, const char __user *buffer,
                                  size_t count, loff_t *ppos)
{
   char locald[16] = {0};
   unsigned long target_freq;
   char *endp;
   int i;

   if( i2s_write_in_progress )
   {
      printk(KERN_ERR "%s: Cannot set frequency, writes in progress in use\n", __FUNCTION__);
      return -EFAULT;
   }

   if ( count > (sizeof(locald) - 1) )
   {
      return -EINVAL;
   }

   if (copy_from_user(locald, buffer, count))
   {
      return -EFAULT;
   }

   target_freq = simple_strtoul(locald, &endp, 10);

   if (endp == locald)
   {
      printk(KERN_INFO "%s: Invalid parameter\n", __FUNCTION__);
      return count;
   }

#if I2S_TRY_PLL_CLK
   for( i=0; pll_freq_map[i].freq; i++ )
   {
      if( pll_freq_map[i].freq == target_freq )
         freq = target_freq;
   }
#endif
   
   for( i=0; freq_map[i].freq; i++ )
   {
      if( freq_map[i].freq == target_freq )
         freq = target_freq;
   }
   
   if( freq != target_freq )
   {
      printk(KERN_ERR "%s: Invalid freq selected: %d, Current freq: %d\n", __FUNCTION__, (int)target_freq, freq);
   }

   *ppos += count;
   return count;
}

static int i2s_show_freq(struct seq_file *seq, void *offset)
{
   int i;
#if I2S_TRY_PLL_CLK
   int j;
#endif
   seq_printf(seq,"Current Sampling Frequency: %d\n", freq);

   seq_printf(seq, "Available Frequencies:\n");

#if I2S_TRY_PLL_CLK
   for( i=0; pll_freq_map[i].freq; i++ )
   {
      seq_printf(seq,"%d\n", pll_freq_map[i].freq );
   }
#endif
   
   for( i=0; freq_map[i].freq; i++ )
   {
#if I2S_TRY_PLL_CLK
      /* Dont display frequencies already present in pll based table */
      for( j=0; pll_freq_map[j].freq; j++ )
      {
         if( freq_map[i].freq == pll_freq_map[j].freq )
            break;
      }
      if( pll_freq_map[j].freq )
         continue;
#endif

      seq_printf(seq,"%d\n", freq_map[i].freq );
   }

   return 0;
}

static int i2s_seq_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
    return single_open(file, i2s_show_freq, PDE_DATA(inode));
#else
    return single_open(file, i2s_show_freq, PDE(inode)->data);
#endif
}

static struct file_operations i2s_proc_fops = {
   .owner = THIS_MODULE,
   .open = i2s_seq_open,
   .read = seq_read,
   .write = i2s_write_freq,
   .llseek = seq_lseek,
   .release = single_release,
};

static int i2s_create_proc_entries( void )
{
   i2s_proc_entry = proc_mkdir(I2S_PROC_ENTRY_ROOT, NULL);
   if(i2s_proc_entry == NULL)
   {
      return -ENOMEM;
   }

   proc_create(I2S_PROC_ENTRY, S_IWUSR | S_IRUGO, i2s_proc_entry, &i2s_proc_fops);

   return 0;
}

/*
 * Remove proc entry
 */
static int i2s_remove_proc_entries( void )
{
   remove_proc_entry(I2S_PROC_ENTRY, i2s_proc_entry);
   return 0;
}

static int __init i2s_init(void) 
{
   int ret;
   
   /* Register char driver region */
   alloc_chrdev_region(&i2s_devId, 0, 1, "i2s");

   /* Create class and device ( /sys entries ) */
   i2s_cl = class_create(THIS_MODULE, "i2s");
   if(i2s_cl == NULL)
   {
      printk(KERN_ERR "Error creating device class\n");
      goto err_cdev_cleanup;
   }

   i2s_device = device_create(i2s_cl, NULL, i2s_devId, NULL, "i2s");
   if(i2s_device == NULL)
   {
      printk(KERN_ERR "Error creating device\n");
      goto err_class_cleanup;
   }

   /* Set the DMA masks for this device */
   dma_coerce_mask_and_coherent(i2s_device, DMA_BIT_MASK(32));
   
   /* Init the character device */
   cdev_init(&i2s_cdev, &i2s_fops);
   i2s_cdev.owner = THIS_MODULE;
   ret = cdev_add(&i2s_cdev, i2s_devId, 1);

   if( ret )
   {
      printk(KERN_ERR "Error %d adding i2s driver", ret);
      goto err_device_cleanup;
   }
   else
   {
      printk(KERN_ALERT "i2s registered\n");
   }
   
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
   /* for ARM it will always rearm!! */
   ret = BcmHalMapInterruptEx((FN_HANDLER)i2s_dma_isr,
                          (void*)0, 
                          INTERRUPT_ID_I2S,
                          "i2s_dma", INTR_REARM_YES,
                          INTR_AFFINITY_DEFAULT);
#else
   ret = BcmHalMapInterruptEx((FN_HANDLER)i2s_dma_isr,
                          (void*)0, 
                          INTERRUPT_ID_I2S,
                          "i2s_dma", INTR_REARM_NO,
                          INTR_AFFINITY_DEFAULT);
#endif    

   if (ret != 0)
   {
      printk(KERN_ERR "i2s_init: failed to register "
                          "intr %d rv=%d\n", INTERRUPT_ID_I2S, ret);
      goto err_device_cleanup;      
   }   
   
   /* Create proc entries */
   ret = i2s_create_proc_entries();

   if (ret != 0)
   {
      printk(KERN_ERR "i2s_init: failed to create proc entries"
                          "rv=%d\n", ret);
      goto err_device_cleanup;      
   }   
   
   return 0;
   
err_device_cleanup:
   device_destroy(i2s_cl, i2s_devId);
err_class_cleanup:
   class_destroy(i2s_cl);
err_cdev_cleanup:
   cdev_del(&i2s_cdev);
   return -1;
   
}
 
static void __exit i2s_exit(void) 
{
   /* deinit queue */
   deinit_tx_desc_queue();
   
   /* Delete cdev */
   cdev_del(&i2s_cdev);

   /* destroy the shim device and device class */
   device_destroy(i2s_cl, i2s_devId);
   class_destroy(i2s_cl);
   
   /* Unregister chrdev region */
   unregister_chrdev_region(i2s_devId, 1);

   /* Remove proc entries */
   i2s_remove_proc_entries();
   
   printk(KERN_ALERT "i2s unregistered\n");
}
 
module_init(i2s_init);
module_exit(i2s_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("I2S Driver");


         
         

      
            
      
      
      
     
      
