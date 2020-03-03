#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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



#ifndef _BRCMNAND_PRIV_H_
#define _BRCMNAND_PRIV_H_

#include <linux/vmalloc.h>
#include <linux/mtd/brcmnand.h>

#ifdef CONFIG_MTD_BRCMNAND_USE_ISR
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>

//#include "edu.h"
#endif

#define BRCMNAND_CORRECTABLE_ECC_ERROR		(1)
#define BRCMNAND_SUCCESS						(0)
#define BRCMNAND_UNCORRECTABLE_ECC_ERROR	(-1)
#define BRCMNAND_FLASH_STATUS_ERROR			(-2)
#define BRCMNAND_TIMED_OUT					(-3)

#ifdef CONFIG_MTD_BRCMNAND_EDU
#define BRCMEDU_CORRECTABLE_ECC_ERROR        	(4)
#define BRCMEDU_UNCORRECTABLE_ECC_ERROR      (-4)

#define  BRCMEDU_MEM_BUS_ERROR				(-5)


#define BRCMNAND_malloc(size) kmalloc(size, GFP_DMA)
#define BRCMNAND_free(addr) kfree(addr)

#else
#define BRCMNAND_malloc(size) vmalloc(size)
#define BRCMNAND_free(addr) vfree(addr)
#endif

#if 0 /* TO */
typedef u8 uint8;
typedef u16 uint16;
typedef u32 uint32;
#endif

#define BRCMNAND_FCACHE_SIZE		512
#define ECCSIZE(chip)					BRCMNAND_FCACHE_SIZE	/* Always 512B for Brcm NAND controller */

#define MTD_OOB_NOT_WRITEABLE	0x8000
#define MTD_CAP_MLC_NANDFLASH	(MTD_WRITEABLE | MTD_OOB_NOT_WRITEABLE)
#define MTD_IS_MLC(mtd) ((((mtd)->flags & MTD_CAP_MLC_NANDFLASH) == MTD_CAP_MLC_NANDFLASH) &&\
			(((mtd)->flags & MTD_OOB_NOT_WRITEABLE) == MTD_OOB_NOT_WRITEABLE))


/* 
 * NUM_NAND_CS here is strictly based on the number of CS in the NAND registers
 * It does not have the same value as NUM_CS in brcmstb/setup.c
 * It is not the same as NAND_MAX_CS, the later being the bit fields found in NAND_CS_NAND_SELECT.
 */

/*
 * # number of CS supported by EBI
 */
#ifdef BCHP_NAND_CS_NAND_SELECT_EBI_CS_7_SEL_MASK
/* Version < 3 */
#define NAND_MAX_CS    8

#elif defined(BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_SEL_MASK)
/* 7420Cx */
#define NAND_MAX_CS    4
#else
/* 3548 */
#define NAND_MAX_CS 2
#endif

/* 
 * Number of CS seen by NAND
 */
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_3
#define NUM_NAND_CS			4

#else
#define NUM_NAND_CS			2
#endif

#ifdef CONFIG_MTD_BRCMNAND_USE_ISR

//#define BCM_BASE_ADDRESS				0xb0000000

/* CP0 hazard avoidance. */
#define BARRIER __asm__ __volatile__(".set noreorder\n\t" \
				     "nop; nop; nop; nop; nop; nop;\n\t" \
				     ".set reorder\n\t")

/* 
 * Right now we submit a full page Read for queueing, so with a 8KB page,
 * and an ECC step of 512B, the queue depth is 16. Add 2 for dummy elements
 * during EDU WAR
 */
#if CONFIG_MTD_BRCMNAND_VERSION <=  CONFIG_MTD_BRCMNAND_VERS_3_3
#define MAX_NAND_PAGE_SIZE	(4<<10)

#else
#define MAX_NAND_PAGE_SIZE	(8<<10)
#endif

/* Max queue size is (PageSize/512B_ECCSize)+2 spare for WAR */
#define MAX_JOB_QUEUE_SIZE	((MAX_NAND_PAGE_SIZE>>9))

typedef enum {
	ISR_OP_QUEUED = 0, 
	ISR_OP_SUBMITTED = 1, 
	ISR_OP_NEED_WAR = 2,
	ISR_OP_COMPLETED = 3, 
	ISR_OP_TIMEDOUT = 4,
	ISR_OP_COMP_WITH_ERROR = 5,
} isrOpStatus_t;

typedef struct eduIsrNode {
	struct list_head list;
	spinlock_t lock; // per Node update lock
	// int cmd;	// 1 == Read, 0 == Write

	// ISR stuffs
	uint32_t mask;	/* Clear status mask */
	uint32_t expect;	/* Status on success */
	uint32_t error;	/* Status on error */
	uint32_t intr;		/* Interrupt bits */
	uint32_t status; 	/* Status read during ISR.  There may be several interrupts before completion */
	isrOpStatus_t opComplete;	/* Completion status */

	/* Controller Level params (for queueing)  */
	struct mtd_info* mtd;
	void* 	buffer;
	u_char* 	oobarea;
	loff_t 	offset;
	int		ret;
	int		needBBT;

	/* EDU level params (for ISR) */
	uint32_t edu_ldw;
	uint32_t physAddr;
	uint32_t hif_intr2;
	uint32_t edu_status;

	int refCount;		/* Marked for re-use when refCount=0 */
	unsigned long expired; /* Time stamp for expiration, 3 secs from submission */
} eduIsrNode_t;

/*
 * Read/Write Job Q.
 * Process one page at a time, and queue 512B sector Read or Write EDU jobs.
 * ISR will wake up the process context thread iff
 * 1-EDU reports an error, in which case the process context thread need to be awaken
 *  		in order to do WAR
 * 2-Q is empty, in which case the page read/write op is complete.
 */
typedef struct jobQ_t {
	struct list_head 	jobQ;		/* Nodes queued for EDU jobs */
	struct list_head 	availList;	/* Free Nodes */
	spinlock_t		lock; 		/* Queues guarding spin lock */
	int 				needWakeUp;	/* Wake up Process context thread to do EDU WAR */
	int 				cmd; 		/* 1 == Read, 0 == Write */
	int				corrected;	/* Number of correctable errors */
} isrJobQ_t;

extern isrJobQ_t gJobQ; 

void ISR_init(void);

/*
 * Submit the first entry that is in queued state,
 * assuming queue lock has been held by caller.
 * 
 * @doubleBuffering indicates whether we need to submit just 1 job or until EDU is full (double buffering)
 * Return the number of job submitted for read.
 *
 * In current version (v3.3 controller), since EDU only have 1 register for EDU_ERR_STATUS,
 * we can't really do double-buffering without losing the returned status of the previous read-op.
 */
#undef EDU_DOUBLE_BUFFER_READ

int brcmnand_isr_submit_job(void);

eduIsrNode_t*  ISR_queue_read_request(struct mtd_info *mtd,
        void* buffer, u_char* oobarea, loff_t offset);
eduIsrNode_t* ISR_queue_write_request(struct mtd_info *mtd,
        const void* buffer, const u_char* oobarea, loff_t offset);
eduIsrNode_t*  ISR_push_request(struct mtd_info *mtd,
        void* buffer, u_char* oobarea, loff_t offset);


int brcmnand_edu_read_completion(struct mtd_info* mtd, 
        void* buffer, u_char* oobarea, loff_t offset, uint32_t intr_status);

int brcmnand_edu_read_comp_intr(struct mtd_info* mtd, 
        void* buffer, u_char* oobarea, loff_t offset, uint32_t intr_status);

#ifdef CONFIG_MTD_BRCMNAND_ISR_QUEUE
int brcmnand_edu_write_completion(struct mtd_info *mtd,
        const void* buffer, const u_char* oobarea, loff_t offset, uint32_t intr_status, 
        int needBBT);
int
brcmnand_edu_write_war(struct mtd_info *mtd,
        const void* buffer, const u_char* oobarea, loff_t offset, uint32_t intr_status, 
        int needBBT);
#endif
eduIsrNode_t* ISR_find_request( isrOpStatus_t opStatus);

uint32_t ISR_wait_for_completion(void);

/*
 *  wait for completion with read/write Queue
 */
int ISR_wait_for_queue_completion(void);

int ISR_cache_is_valid(void);

static __inline__ uint32_t ISR_volatileRead(uint32_t addr)
{
        
        
        return (uint32_t) BDEV_RD(addr);
}

static __inline__ void ISR_volatileWrite(uint32_t addr, uint32_t data)
{
        BDEV_WR(addr, data);
}

static __inline__ void ISR_enable_irq(eduIsrNode_t* req)
{
	//uint32_t intrMask; 
	//unsigned long flags;

	//spin_lock_irqsave(&gEduIsrData.lock, flags);
	
	// Clear status bits
	ISR_volatileWrite(BCHP_HIF_INTR2_CPU_CLEAR, req->mask);

	// Enable interrupt
	ISR_volatileWrite(BCHP_HIF_INTR2_CPU_MASK_CLEAR, req->intr);

	//spin_unlock_irqrestore(&gEduIsrData.lock, flags);
}

static __inline__ void ISR_disable_irq(uint32_t mask)
{

	/* Disable L2 interrupts */
	ISR_volatileWrite(BCHP_HIF_INTR2_CPU_MASK_SET, mask);

}

/*
 * For debugging
 */

#ifdef DEBUG_ISR

static void __inline__
ISR_print_queue(void)
{
	eduIsrNode_t* req;
	//struct list_head* node;
	int i = 0;

	list_for_each_entry(req, &gJobQ.jobQ, list) {
		
		printk("i=%d, cmd=%d, offset=%08llx, flashAddr=%08x, opComp=%d, status=%08x\n",
			i, gJobQ.cmd, req->offset, req->edu_ldw,req->opComplete, req->status);
		i++;
	}	
}

static void __inline__
ISR_print_avail_list(void)
{
	eduIsrNode_t* req;
	//struct list_head* node;
	int i = 0;

	printk("AvailList=%p, next=%p\n", &gJobQ.availList, gJobQ.availList.next);
	list_for_each_entry(req, &gJobQ.availList, list) {
		printk("i=%d, req=%p, list=%p\n", i, req, &req->list);
		i++;
	}	
}
#else
#define IS_print_queue()
#define ISR_print_avail_list()
#endif // DEBUG_ISR


#endif // CONFIG_MTD_BRCMNAND_USE_ISR

static inline u_int64_t device_size(struct mtd_info *mtd) 
{
	//return mtd->size == 0 ? (u_int64_t) mtd->num_eraseblocks * mtd->erasesize : (u_int64_t) mtd->size;
	return mtd->size;
}

/**
 * brcmnand_scan - [BrcmNAND Interface] Scan for the BrcmNAND device
 * @param mtd		MTD device structure
 * @cs			  	Chip Select number
 * @param numchips	Number of chips  (from CFE or from nandcs= kernel arg)

 *
 * This fills out all the not initialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 *
 */
extern int brcmnand_scan(struct mtd_info *mtd , int cs, int maxchips);

/**
 * brcmnand_release - [BrcmNAND Interface] Free resources held by the BrcmNAND device
 * @param mtd		MTD device structure
 */
extern void brcmnand_release(struct mtd_info *mtd);

/* BrcmNAND BBT interface */

/* Auto-format scan layout for BCH-8 with 16B OOB */
#define BRCMNAND_BBT_AUTO_PLACE	0x80000000

extern uint8_t* brcmnand_transfer_oob(struct brcmnand_chip *chip, uint8_t *oob,
				  struct mtd_oob_ops *ops, int len);
extern uint8_t* brcmnand_fill_oob(struct brcmnand_chip *chip, uint8_t *oob, struct mtd_oob_ops *ops);

/* Read the OOB bytes and tell whether a block is bad without consulting the BBT */
extern int brcmnand_isbad_raw (struct mtd_info *mtd, loff_t offs);

extern int brcmnand_scan_bbt(struct mtd_info *mtd, struct nand_bbt_descr *bd);
extern int brcmnand_default_bbt(struct mtd_info *mtd);

extern int brcmnand_update_bbt (struct mtd_info *mtd, loff_t offs);

//extern void* get_brcmnand_handle(void);

extern void print_oobbuf(const unsigned char* buf, int len);
extern void print_databuf(const unsigned char* buf, int len);

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
extern int brcmnand_cet_update(struct mtd_info *mtd, loff_t from, int *status);
extern int brcmnand_cet_prepare_reboot(struct mtd_info *mtd);
extern int brcmnand_cet_erasecallback(struct mtd_info *mtd, u_int32_t addr);
extern int brcmnand_create_cet(struct mtd_info *mtd);
#endif

/*
 * Disable ECC, and return the original ACC register (for restore)
 */
uint32_t brcmnand_disable_read_ecc(int cs);

void brcmnand_restore_ecc(int cs, uint32_t orig_acc0);

void brcmnand_post_mortem_dump(struct mtd_info* mtd, loff_t offset);

static unsigned int __maybe_unused brcmnand_get_bbt_size(struct mtd_info* mtd)
{
	struct brcmnand_chip * chip = mtd->priv;
	
	// return ((device_size(mtd) > (512 << 20)) ? 4<<20 : 1<<20);
	return chip->bbtSize;
}

	
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_3
static  inline uintptr_t  bchp_nand_acc_control(int cs)
{
	switch (cs) {
	case 0: return BCHP_NAND_ACC_CONTROL;
	case 1: return BCHP_NAND_ACC_CONTROL_CS1;
#ifdef BCHP_NAND_ACC_CONTROL_CS2
	case 2: return BCHP_NAND_ACC_CONTROL_CS2;
#endif
#ifdef BCHP_NAND_ACC_CONTROL_CS3
	case 3: return BCHP_NAND_ACC_CONTROL_CS3;
#endif
	}
	return 0;
}

static  inline uintptr_t bchp_nand_config(int cs)
{
	switch (cs) {
	case 0: return BCHP_NAND_CONFIG;
	case 1: return BCHP_NAND_CONFIG_CS1;
#ifdef BCHP_NAND_CONFIG_CS2
	case 2: return BCHP_NAND_CONFIG_CS2;
#endif
#ifdef BCHP_NAND_CONFIG_CS3
	case 3: return BCHP_NAND_CONFIG_CS3;
#endif
	}
	return 0;
}

static  inline uintptr_t bchp_nand_timing1(int cs)
{
	switch (cs) {
	case 0: return BCHP_NAND_TIMING_1;
	case 1: return BCHP_NAND_TIMING_1_CS1;
#ifdef BCHP_NAND_TIMING_1_CS2
	case 2: return BCHP_NAND_TIMING_1_CS2;
#endif
#ifdef BCHP_NAND_TIMING_1_CS3
	case 3: return BCHP_NAND_TIMING_1_CS3;
#endif
	}
	return 0;
}
static  inline uintptr_t bchp_nand_timing2(int cs)
{
	switch (cs) {
	case 0: return BCHP_NAND_TIMING_2;
	case 1: return BCHP_NAND_TIMING_2_CS1;
#ifdef BCHP_NAND_TIMING_2_CS2
	case 2: return BCHP_NAND_TIMING_2_CS2;
#endif
#ifdef BCHP_NAND_TIMING_2_CS3
	case 3: return BCHP_NAND_TIMING_2_CS3;
#endif
	}
	return 0;
}

#else
#define bchp_nand_acc_control(cs) BCHP_NAND_ACC_CONTROL
#define bchp_nand_config(cs) BCHP_NAND_CONFIG
#define bchp_nand_timing1(cs) BCHP_NAND_TIMING_1
#define bchp_nand_timing2(cs) BCHP_NAND_TIMING_2
#endif
	
/***********************************************************************
 * Register access macros - sample usage:
 *
 * DEV_RD(0xb0404000)                       -> reads 0xb0404000
 * BDEV_RD(0x404000)                        -> reads 0xb0404000
 * BDEV_RD(BCHP_SUN_TOP_CTRL_PROD_REVISION) -> reads 0xb0404000
 *
 * _RB means read back after writing.
 ***********************************************************************/
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define BPHYSADDR(x)	(x)
#define BVIRTADDR(x)	(x)
#else
#define BPHYSADDR(x)	((x) | 0x10000000)
#define BVIRTADDR(x)	KSEG1ADDR(BPHYSADDR(x))
#endif

#define DEV_RD(x) (*((volatile unsigned int *)(x)))
#define DEV_WR(x, y) do { *((volatile unsigned int *)(x)) = (y); } while (0)
#define DEV_UNSET(x, y) do { DEV_WR((x), DEV_RD(x) & ~(y)); } while (0)
#define DEV_SET(x, y) do { DEV_WR((x), DEV_RD(x) | (y)); } while (0)

#define DEV_WR_RB(x, y) do { DEV_WR((x), (y)); DEV_RD(x); } while (0)
#define DEV_SET_RB(x, y) do { DEV_SET((x), (y)); DEV_RD(x); } while (0)
#define DEV_UNSET_RB(x, y) do { DEV_UNSET((x), (y)); DEV_RD(x); } while (0)

#define BDEV_RD(x) (DEV_RD(BVIRTADDR(x)))
#define BDEV_WR(x, y) do { DEV_WR(BVIRTADDR(x), (y)); } while (0)
#define BDEV_UNSET(x, y) do { BDEV_WR((x), BDEV_RD(x) & ~(y)); } while (0)
#define BDEV_SET(x, y) do { BDEV_WR((x), BDEV_RD(x) | (y)); } while (0)

#define BDEV_SET_RB(x, y) do { BDEV_SET((x), (y)); BDEV_RD(x); } while (0)
#define BDEV_UNSET_RB(x, y) do { BDEV_UNSET((x), (y)); BDEV_RD(x); } while (0)
#define BDEV_WR_RB(x, y) do { BDEV_WR((x), (y)); BDEV_RD(x); } while (0)

#define BDEV_RD_F(reg, field) \
	((BDEV_RD(BCHP_##reg) & BCHP_##reg##_##field##_MASK) >> \
	 BCHP_##reg##_##field##_SHIFT)
#define BDEV_WR_F(reg, field, val) do { \
	BDEV_WR(BCHP_##reg, \
	(BDEV_RD(BCHP_##reg) & ~BCHP_##reg##_##field##_MASK) | \
	(((val) << BCHP_##reg##_##field##_SHIFT) & \
	 BCHP_##reg##_##field##_MASK)); \
	} while (0)
#define BDEV_WR_F_RB(reg, field, val) do { \
	BDEV_WR(BCHP_##reg, \
	(BDEV_RD(BCHP_##reg) & ~BCHP_##reg##_##field##_MASK) | \
	(((val) << BCHP_##reg##_##field##_SHIFT) & \
	 BCHP_##reg##_##field##_MASK)); \
	BDEV_RD(BCHP_##reg); \
	} while (0)


#endif
#endif
