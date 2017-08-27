#if defined(CONFIG_BCM_KF_BOUNCE) && defined(CONFIG_BRCM_BOUNCE)
/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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
 *******************************************************************************
 * File Name   : bounce.c
 *******************************************************************************
 */

#include <asm/bounce.h>
#include <linux/sched.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/fs.h>

#ifdef BOUNCE_COLOR
#define _H_						"\e[0;36;44m"
#define _N_						"\e[0m"
#define _R_						"\e[0;31m"
#define _G_						"\e[0;32m"
#else
#define _H_
#define _N_
#define _R_
#define _G_
#endif

#undef  BOUNCE_DECL
#define BOUNCE_DECL(x)			#x,

/*----- typedefs -----*/
typedef char BounceFmt_t[BOUNCE_FMT_LENGTH];

typedef struct bounceDev
{

	BounceMode_t	mode;		/* mode of operation */
	BounceLog_t	  * log_p;

	uint32_t		wrap;		/* buffer wrapped at least once */
	uint32_t		run;		/* trace incarnation */
	uint32_t		count;		/* log count .. (not function count) */

	dev_t			dev;
	struct cdev		cdev;

    BounceFmt_t     evtfmt[ BOUNCE_MAX_EVENTS ];

	BounceLog_t		log[ BOUNCE_SIZE ];

} BounceDev_t;


/*----- Forward definition -----*/

static int  bounce_open(struct inode *inodep, struct file *filep)BOUNCE_NOINSTR;
static int  bounce_rel(struct inode *inodep, struct file *filep)	BOUNCE_NOINSTR;
static long bounce_unlocked_ioctl( struct file *  file, 
                                  unsigned int   cmd,
                                  unsigned long  arg)			BOUNCE_NOINSTR;
extern void bounce_up(BounceMode_t mode, uint32_t limit)		BOUNCE_NOINSTR;
extern asmlinkage void bounce_dn(void);
extern asmlinkage void bounce_panic(void);
extern void bounce_reg(uint32_t event, char * eventName)        BOUNCE_NOINSTR;
extern void bounce_dump(uint32_t last)							BOUNCE_NOINSTR;

extern void bounce0(uint32_t event)                             BOUNCE_NOINSTR;
extern void bounce1(uint32_t event, uint32_t arg1)              BOUNCE_NOINSTR;
extern void bounce2(uint32_t event, uint32_t arg1, uint32_t arg2)
                                                                BOUNCE_NOINSTR;
extern void bounce3(uint32_t event, uint32_t arg1, uint32_t arg2, uint32_t arg3)
                                                                BOUNCE_NOINSTR;

static int  __init bounce_init(void)							BOUNCE_NOINSTR;
static void __exit bounce_exit(void)							BOUNCE_NOINSTR;

/*----- Globals -----*/

BounceDev_t bounce_g = { .mode = BOUNCE_MODE_DISABLED };

static struct file_operations bounce_fops_g =
{
	.unlocked_ioctl =    bounce_unlocked_ioctl,
	.open =     bounce_open,
	.release =  bounce_rel,
	.owner =    THIS_MODULE
};

static const char * bounce_mode_str_g[] =
{
    BOUNCE_DECL(BOUNCE_MODE_DISABLED)
    BOUNCE_DECL(BOUNCE_MODE_LIMITED)    /* auto disable when count goes to 0 */
    BOUNCE_DECL(BOUNCE_MODE_CONTINUOUS) /* explicit disable via bounce_dn() */
    BOUNCE_DECL(BOUNCE_MODE_MAXIMUM)
};

#ifdef BOUNCE_DEBUG
static const char * bounce_ioctl_str_g[] =
{
    BOUNCE_DECL(BOUNCE_START_IOCTL)
    BOUNCE_DECL(BOUNCE_STOP_IOCTL)
    BOUNCE_DECL(BOUNCE_DUMP_IOCTL)
    BOUNCE_DECL(BOUNCE_INVLD_IOCTL)
};
#endif

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#include "linux/spinlock.h"
static DEFINE_SPINLOCK(bounce_lock_g);   /* FkBuff packet flow */
#define BOUNCE_LOCK(flags)       spin_lock_irqsave( &bounce_lock_g, flags )
#define BOUNCE_UNLOCK(flags)     spin_unlock_irqrestore( &bounce_lock_g, flags )
#else
#define BOUNCE_LOCK(flags)       local_irq_save(flags)
#define BOUNCE_UNLOCK(flags)     local_irq_restore(flags)
#endif

/* MACROS used by __cyg_profile_func_ enter() and exit() */
#define __BOUNCE_BGN(flags)												\
																	\
	if ( bounce_g.mode == BOUNCE_MODE_DISABLED )					\
		return;														\
																	\
    BOUNCE_LOCK(flags);                                                  \
	if ( bounce_g.mode == BOUNCE_MODE_LIMITED )						\
	{																\
		if ( bounce_g.count == 0 )									\
		{															\
			bounce_g.mode = BOUNCE_MODE_DISABLED;					\
            BOUNCE_UNLOCK(flags);                                        \
			return;													\
		}															\
		bounce_g.count--;											\
	}


#define __BOUNCE_END(flags)												\
																	\
	bounce_g.log_p++;												\
																	\
	if ( bounce_g.log_p == &bounce_g.log[ BOUNCE_SIZE ] )			\
	{																\
		bounce_g.wrap = 1;											\
		bounce_g.log_p = &bounce_g.log[0];							\
	}																\
																	\
    BOUNCE_UNLOCK(flags);


/* Function entry stub providied by -finstrument-functions */
void __cyg_profile_func_enter(void *ced, void *cer)
{
    unsigned long flags;
	__BOUNCE_BGN(flags);

    bounce_g.log_p->word0.u32 = (uint32_t)ced | (smp_processor_id() << 1) | 1;
	bounce_g.log_p->pid = (uint32_t)(current_thread_info()->task->pid);

	__BOUNCE_END(flags);
}

/* Function exit stub providied by -finstrument-functions */
void __cyg_profile_func_exit(void *ced, void *cer)
{
#if defined(CONFIG_BRCM_BOUNCE_EXIT)
    unsigned long flags;
	__BOUNCE_BGN(flags);

    bounce_g.log_p->word0.u32 = (uint32_t)ced | (smp_processor_id() << 1);
	bounce_g.log_p->pid = (uint32_t)(current_thread_info()->task->pid);

	__BOUNCE_END(flags);

#endif	/* defined(CONFIG_BRCM_BOUNCE_EXIT) */
}

void bounce0(uint32_t event)
{
    unsigned long flags;
    __BOUNCE_BGN(flags);

    bounce_g.log_p->word0.u32 = (event << 16) | (smp_processor_id() << 1);

    __BOUNCE_END(flags);
}

void bounce1(uint32_t event, uint32_t arg1)
{
    unsigned long flags;
    __BOUNCE_BGN(flags);

    bounce_g.log_p->word0.u32 = (event << 16)
                                | (1 << 2) | (smp_processor_id() << 1);
    bounce_g.log_p->arg1 = arg1;

    __BOUNCE_END(flags);
}

void bounce2(uint32_t event, uint32_t arg1, uint32_t arg2)
{
    unsigned long flags;
    __BOUNCE_BGN(flags);

    bounce_g.log_p->word0.u32 = (event << 16)
                                | (2 << 2) | (smp_processor_id() << 1);
    bounce_g.log_p->arg1 = arg1;
    bounce_g.log_p->arg2 = arg2;

    __BOUNCE_END(flags);
}

void bounce3(uint32_t event, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    unsigned long flags;
    __BOUNCE_BGN(flags);

    bounce_g.log_p->word0.u32 = (event << 16)
                                | (3 << 2) | (smp_processor_id() << 1);
    bounce_g.log_p->arg1 = arg1;
    bounce_g.log_p->arg2 = arg2;
    bounce_g.log_p->arg3 = arg3;

    __BOUNCE_END(flags);
}

static int bounce_panic_dump = 0;

/* Start tracing */
void bounce_up(BounceMode_t mode, uint32_t limit)
{
	bounce_g.wrap = 0;						/* setup trace buffer */
	bounce_g.log_p = &bounce_g.log[0];
	bounce_g.count = limit;					/* setup stop semantics */
	bounce_g.mode = mode;					/* tracing enabled now */

	bounce_panic_dump = 1;
}

/* Stop tracing */
void bounce_dn(void)
{
	BOUNCE_LOGK(bounce_dn);

    if ( bounce_g.mode != BOUNCE_MODE_DISABLED )
		bounce_g.mode = BOUNCE_MODE_LIMITED;/* initiate stop semantics */
}

/* Auto dump last BOUNCE_PANIC items on a panic/bug */
void bounce_panic(void)
{
	BOUNCE_LOGK(bounce_dn);

	if ( bounce_panic_dump ) {
		bounce_panic_dump = 0;
		bounce_g.mode = BOUNCE_MODE_DISABLED;
		bounce_dump( BOUNCE_PANIC );
	}
}

void bounce_reg(uint32_t event, char * eventName)
{
    if ( event < BOUNCE_MAX_EVENTS )
    {
        strncpy( bounce_g.evtfmt[event], eventName,
                 BOUNCE_FMT_LENGTH-1 );
    }
}

/* Dump the trace buffer via printk */
void bounce_dump(uint32_t last)
{
	BounceLog_t * log_p;
	uint32_t logs;
	uint32_t wrap;
	uint32_t count;
	BounceMode_t mode;

	count = bounce_g.count;
	bounce_g.count = 0;

	mode = bounce_g.mode;
	bounce_g.mode  = BOUNCE_MODE_DISABLED;

	printk(_H_ "BOUNCE DUMP BGN: FUNC_EXIT<%d> run<%u> wrap<%u> count<%u> %s\n"
	       "B[0x%08x] L[0x%08x] E[0x%08x], %u:%u bounce_dn[<0x%08x>]\n\n" _N_,
#if defined(CONFIG_BRCM_BOUNCE_EXIT)
			1,
#else
			0,
#endif
		    bounce_g.run, bounce_g.wrap, count, bounce_mode_str_g[mode],
            (int)&bounce_g.log[0],
            (int)bounce_g.log_p, (int)&bounce_g.log[BOUNCE_SIZE],
			(((uint32_t)bounce_g.log_p - (uint32_t)&bounce_g.log[0])
            / sizeof(BounceLog_t)),
			(((uint32_t)(&bounce_g.log[BOUNCE_SIZE])
            - (uint32_t)bounce_g.log_p) / sizeof(BounceLog_t)),
            (int)bounce_dn );

	/* Dump the last few records */
	if ( last != 0 )
	{
		uint32_t items;

		if ( last > BOUNCE_SIZE )
			last = BOUNCE_SIZE;

		items = (((uint32_t)bounce_g.log_p - (uint32_t)&bounce_g.log[0])
				 / sizeof(BounceLog_t));

		if ( items > last )
		{
			log_p = (BounceLog_t*)
				((uint32_t)bounce_g.log_p - (last * sizeof(BounceLog_t)));
			wrap = 0;
		}
		else
		{
			items = last - items; 	/* remaining items */
			log_p = (BounceLog_t*)
				((uint32_t)(&bounce_g.log[BOUNCE_SIZE]
				 - (items * sizeof(BounceLog_t))));
			wrap = 1;
		}
	}
	else
	{
		wrap = bounce_g.wrap;
		if ( bounce_g.wrap )
			log_p = bounce_g.log_p;
		else
			log_p = & bounce_g.log[0];
	}

	logs = 0;

    /* Start from current and until end */
	if ( wrap )
	{
		for ( ; log_p != & bounce_g.log[BOUNCE_SIZE]; logs++, log_p++ )
		{
            if ( BOUNCE_IS_FUNC_LOG(log_p->word0.u32) )
            {
			    printk( "%s %5u %pS" _N_ "\n",
					    (log_p->word0.site.type) ? _R_ "=>" : _G_ "<=",
					    log_p->pid, BOUNCE_GET_FUNCP(log_p->word0.u32) );
            }
            else
            {
                switch (log_p->word0.event.args)
                {
                    case 0:
                        printk(bounce_g.evtfmt[log_p->word0.event.evid]);
                        break;
                    case 1:
                        printk( bounce_g.evtfmt[log_p->word0.event.evid],
                                log_p->arg1);
                        break;
                    case 2:
                        printk( bounce_g.evtfmt[log_p->word0.event.evid],
                                log_p->arg1, log_p->arg2);
                        break;
                    case 3:
                        printk( bounce_g.evtfmt[log_p->word0.event.evid],
                                log_p->arg1, log_p->arg2, log_p->arg3);
                        break;
                }
                printk( " %s cpu<%u> %s evt<%6u>\n",
                        (log_p->word0.event.cpu0) ? _R_ : _G_,
                        log_p->word0.event.cpu0,  _N_,
                        log_p->word0.event.evid );
            }
		}

		log_p = & bounce_g.log[0];
	}

	for ( ; log_p != bounce_g.log_p; logs++, log_p++ )
	{
        if ( BOUNCE_IS_FUNC_LOG(log_p->word0.u32) )
        {
		    printk( "%s %5u %pS" _N_ "\n",
				    (log_p->word0.site.type) ? _R_ "=>" : _G_ "<=",
				    log_p->pid, BOUNCE_GET_FUNCP(log_p->word0.u32) );
        }
        else
        {
            switch (log_p->word0.event.args)
            {
                case 0:
                    printk(bounce_g.evtfmt[log_p->word0.event.evid]);
                    break;
                case 1:
                    printk( bounce_g.evtfmt[log_p->word0.event.evid],
                            log_p->arg1);
                    break;
                case 2:
                    printk( bounce_g.evtfmt[log_p->word0.event.evid],
                            log_p->arg1, log_p->arg2);
                    break;
                case 3:
                    printk( bounce_g.evtfmt[log_p->word0.event.evid],
                            log_p->arg1, log_p->arg2, log_p->arg3);
                    break;
            }
            printk( " %s cpu<%u> %s evt<%6u>\n",
                    (log_p->word0.event.cpu0) ? _R_ : _G_,
                    log_p->word0.event.cpu0,  _N_,
                    log_p->word0.event.evid );
        }
	}

	printk( _H_ "\nBOUNCE DUMP END: logs<%u>\n\n\n" _N_, logs );
}

static DEFINE_MUTEX(ioctlMutex);

/* ioctl fileops */
long bounce_unlocked_ioctl( struct file *  file, 
                            unsigned int   command,
                            unsigned long  arg)
{
	BounceIoctl_t cmd;
	long ret = -EINVAL;

	mutex_lock(&ioctlMutex);

	if ( command > BOUNCE_INVLD_IOCTL )
		cmd = BOUNCE_INVLD_IOCTL;
	else
		cmd = (BounceIoctl_t)command;

	BDBG( printk(KERN_DEBUG "BOUNCE DEV: ioctl cmd[%d,%s] arg[%lu 0x%08x]\n",
		         command, bounce_ioctl_str_g[cmd], arg, (int)arg ); );

	switch ( cmd )
	{
		case BOUNCE_START_IOCTL:
			{
				BounceMode_t mode = (BounceMode_t) ( arg & 7 );
				uint32_t limit = ( arg >> 3 );

				bounce_up( mode, limit );
				ret = 0;
				break;
			}

		case BOUNCE_STOP_IOCTL:
			bounce_dn(); 
			ret = 0;
			break;

		case BOUNCE_DUMP_IOCTL:
			bounce_dump(arg);
			ret = 0;
			break;

		default:
			printk( KERN_ERR "BOUNCE DEV: invalid ioctl <%u>\n", command );
	}
	mutex_unlock(&ioctlMutex);
	return ret;
}

/* open fileops */
int bounce_open(struct inode *inodep, struct file *filep)
{
	int minor = MINOR(inodep->i_rdev) & 0xf;    /* fetch minor */

	if (minor > 0)
	{
		printk(KERN_WARNING "BOUNCE DEV: multiple open " BOUNCE_DEV_NAME);
		return -ENODEV;
	}
	return 0;
}

/* release fileops */
int bounce_rel(struct inode *inodep, struct file *filep)
{
	return 0;
}

/* module init: register character device */
int __init bounce_init(void)
{
	int i, ret;
	memset(&bounce_g, 0, sizeof(BounceDev_t));
	bounce_g.mode  = BOUNCE_MODE_DISABLED;
	bounce_g.count = BOUNCE_SIZE;
	bounce_g.log_p = &bounce_g.log[0];

	bounce_g.dev = MKDEV(BOUNCE_DEV_MAJ, 0);

	cdev_init(&bounce_g.cdev, &bounce_fops_g);
	bounce_g.cdev.ops = &bounce_fops_g;

	ret = cdev_add(&bounce_g.cdev, bounce_g.dev, 1);

    for (i=0; i<BOUNCE_MAX_EVENTS; i++)
        sprintf(bounce_g.evtfmt[i], "INVALID EVENT");

	if (ret) {
		printk( KERN_ERR _R_ "BOUNCE DEV: Error %d adding device "
				BOUNCE_DEV_NAME " [%d,%d] added.\n" _N_,
				ret, MAJOR(bounce_g.dev), MINOR(bounce_g.dev));
		return ret;
	} else {
		printk( KERN_DEBUG _G_ "BOUNCE DEV: "
				BOUNCE_DEV_NAME " [%d,%d] added.\n" _N_,
				MAJOR(bounce_g.dev), MINOR(bounce_g.dev));
	}

	return ret;
}

/* cleanup : did not bother with char device de-registration */
void __exit bounce_exit(void)
{
	cdev_del(&bounce_g.cdev);
	memset(&bounce_g, 0, sizeof(BounceDev_t));
}

module_init(bounce_init);
module_exit(bounce_exit);

EXPORT_SYMBOL(__cyg_profile_func_enter);
EXPORT_SYMBOL(__cyg_profile_func_exit);

EXPORT_SYMBOL(bounce_up);
EXPORT_SYMBOL(bounce_dn);
EXPORT_SYMBOL(bounce_reg);
EXPORT_SYMBOL(bounce0);
EXPORT_SYMBOL(bounce1);
EXPORT_SYMBOL(bounce2);
EXPORT_SYMBOL(bounce3);
EXPORT_SYMBOL(bounce_dump);
EXPORT_SYMBOL(bounce_panic);

#endif
