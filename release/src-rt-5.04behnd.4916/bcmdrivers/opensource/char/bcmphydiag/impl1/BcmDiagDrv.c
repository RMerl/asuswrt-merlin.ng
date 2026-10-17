/*
   Copyright (c) 2002 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2002:DUAL/GPL:standard

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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include "linux/poll.h"

#include "DiagDef.h"
#include "BcmDiagDrv.h"
#include "BcmDiag.h"

/* typedefs. */
typedef void (*DIAG_FN_IOCTL) (unsigned long arg);

/* Prototypes. */
static int __init diag_init( void );
static void __exit diag_cleanup( void );
static int diag_open( struct inode *inode, struct file *filp );
int diag_ioctl( struct inode *inode, struct file *flip,
   unsigned int command, unsigned long arg );

static void DoDiagCommand ( unsigned long arg );
static void DoOpenProcIf( unsigned long arg );
static void DoCloseProcIf( unsigned long arg );
static void BcmDiagDrvInitProcIfCtrl(void);
static void BcmDiagDrvUnInitProcIfCtrl(void);
static BCMDIAG_STATUS BcmDiagDrvOpenProcIf(int procMsgType, char *pIfName);
static BCMDIAG_STATUS BcmDiagDrvCloseProcIf(int procMsgType, char *pIfName);
static int BcmDiagProcSkbMsgOpen(struct inode * inode, struct file * file);
static int BcmDiagProcMsgRelease(struct inode * inode, struct file * file);
static ssize_t BcmDiagProcSkbMsgRead(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t BcmDiagProcSkbMsgWrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static unsigned int BcmDiagProcSkbMsgPoll(struct file *file, poll_table *wait);


static DEFINE_MUTEX(ioctlMutex);

static long diag_unlocked_ioctl(struct file *filep,
   unsigned int cmd, unsigned long arg )
{
   struct inode *inode;
   long rt;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
   inode = filep->f_dentry->d_inode;
#else
   inode = file_inode(filep);
#endif
   mutex_lock(&ioctlMutex);
   rt = diag_ioctl( inode, filep, cmd, arg );
   mutex_unlock(&ioctlMutex);

   return rt;
}

/* Globals. */
static struct file_operations diag_fops =
{
   .unlocked_ioctl = diag_unlocked_ioctl,
 #if defined(CONFIG_COMPAT)
   .compat_ioctl   = diag_unlocked_ioctl,
 #endif
   .open           = diag_open,
};


typedef struct {
	char ifName[BCM_DIAG_MAX_PROC_IFNAME_LEN];
	int procMsgType;
	Bool fileOpened;
	wait_queue_head_t waitQue;
	struct proc_dir_entry *pProcEntry;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	struct proc_ops *pFileOps;
#else
	struct file_operations *pFileOps;
#endif
} ProcIfCtrl;

#define DIAG_KTHREAD
#ifdef DIAG_KTHREAD
struct task_struct * diagThreadTask = NULL;
static wait_queue_head_t diagThreadWaitQue;
void diagThreadWakeup(void);
void diagThreadCreate(void);
void diagThreadTerminate(void);
extern long BcmDiagProcessTimer(void);

static int diagThread(void * data)
{
   long timeOutInJiffies = HZ;
   printk("*** %s: HZ = %u jiffies_to_msecs(HZ) = %d ***\n", __FUNCTION__, HZ, jiffies_to_msecs(HZ));
   
   while (1) {
      timeOutInJiffies = wait_event_interruptible_timeout(
         diagThreadWaitQue,
         false,
         timeOutInJiffies);
      if (kthread_should_stop()) {
         printk(KERN_INFO "*** %s: kthread_should_stop detected ***\n", __FUNCTION__);
         break;
      }
      timeOutInJiffies = BcmDiagProcessTimer();
   }

   printk(KERN_INFO "*** %s: Exiting ***\n", __FUNCTION__);

   return 0;
}

void diagThreadWakeup(void)
{
    wake_up_interruptible(&diagThreadWaitQue);
}

void diagThreadTerminate(void)
{
   if(diagThreadTask != NULL) {
      printk("*** %s: Stopping diagThread pid=%d ***\n", __FUNCTION__,  diagThreadTask->pid);
      kthread_stop(diagThreadTask);
      diagThreadTask = NULL;
   }
   printk("*** %s: Exiting ***\n", __FUNCTION__);
}

void diagThreadCreate(void)
{
   typedef int (*FN_GENERIC_INT) (void *);

   if (NULL == diagThreadTask) {
      init_waitqueue_head(&diagThreadWaitQue);

      diagThreadTask = kthread_run( (FN_GENERIC_INT) diagThread, (void *) NULL, "bcmdiag" );
      if( IS_ERR(diagThreadTask))
         printk("*** FATAL: Create diagThread failed! ***\n");
      else
         printk("*** diagThread pid=%d ***\n", diagThreadTask->pid);
   }
}
#endif  /* DIAG_KTHREAD */


static int __init diag_init( void )
{
   printk( "diag: diag_init entry\n" );
   if ( register_chrdev( DIAGDRV_MAJOR, "diag", &diag_fops ) )
   {
      printk("adsldd: failed to create adsl\n");
      return( -1 );
   }
   BcmDiagDrvInitProcIfCtrl();
   BcmDiagDrvInit();
   diagThreadCreate();
   
   return 0;
}

static void __exit diag_cleanup( void )
{
   printk( "diag: diag_cleanup entry\n" );
   // TO DO: TONY - Make sure the Diag driver is clean up before exiting.
   diagThreadTerminate();
   BcmDiagDrvUnInitProcIfCtrl();
} /* diag_cleanup */

/***************************************************************************
 * Function Name: diag_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int diag_open( struct inode *inode, struct file *filp )
{
   unsigned char lineId = MINOR(inode->i_rdev);
   filp->private_data = (void *)(long)lineId;  // Should be 0. It is not used.
   return( 0 );
} /* diag_open */


/***************************************************************************
 * Function Name: diag_ioctl
 * Description  : Main entry point for an application sending Diag command
 * Returns      : 0 - success or error
 ***************************************************************************/
int diag_ioctl( struct inode *inode, struct file *flip,
   unsigned int command, unsigned long arg )
{
   int ret = 0;
   unsigned int cmdnr = _IOC_NR(command);

   DIAG_FN_IOCTL IoctlFuncs[] = { DoDiagCommand, DoOpenProcIf, DoCloseProcIf };

   if( cmdnr < MAX_DIAGDRV_IOCTL_COMMANDS ) {
      (*IoctlFuncs[cmdnr]) (arg);
   }
   else
      ret = -EINVAL;

   return( ret );
} /* diag_ioctl */

/***************************************************************************
 * Function Name: DoDiagCommand
 * Description  : Calls BcmDiagProcessCmd on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoDiagCommand ( unsigned long arg )
{
   DIAGDRV_DIAG   KArg;
   int diagCmd;
   static char   *pCmdData = NULL;

   KArg.bvStatus = BCMDIAG_STATUS_ERROR;

   do {
      if (copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0)
         break;
#if defined(CONFIG_COMPAT)
      if (is_compat_task()) {
         BCM_IOC_PTR_ZERO_EXT(KArg.diagMap);
         BCM_IOC_PTR_ZERO_EXT(KArg.skbModel);
      }
#endif
      diagCmd = KArg.diagCmd &  ~(DIAGS_LINE_MASK | DIAG_TYPE_MASK);
      if ((LOG_CMD_CONNECT != diagCmd) && (LOG_CMD_DISCONNECT != diagCmd) && (0 != KArg.diagMap)) {
         if(BcmDiagIsActive() && (0 != KArg.srvIpAddr) && ((KArg.srvIpAddr&0xFFFF) != (BcmDiagGetSrvIpAddr()&0xFFFF)))
            break;

         if(NULL == pCmdData) {
            if(NULL == (pCmdData = kmalloc(LOG_MAX_DATA_SIZE, GFP_ATOMIC)))
               break;
         }
         if (KArg.logTime > LOG_MAX_DATA_SIZE)
            break;
         if (copy_from_user(pCmdData, (char*)KArg.diagMap, KArg.logTime) != 0)
            break;
         if(KArg.logTime < LOG_MAX_DATA_SIZE)
            memset((void*)(pCmdData+KArg.logTime), 0, LOG_MAX_DATA_SIZE-KArg.logTime);
         KArg.diagMap = (unsigned long) pCmdData;
      }
      else if(LOG_CMD_DISCONNECT == diagCmd) {
         if(NULL != pCmdData) {
            kfree(pCmdData);
            pCmdData = NULL;
         }
      }
      else if((LOG_CMD_CONNECT != diagCmd) && BcmDiagIsActive() &&
         (0 != KArg.srvIpAddr) && ((KArg.srvIpAddr&0xFFFF) != (BcmDiagGetSrvIpAddr()&0xFFFF)))
         break;
      
      KArg.bvStatus = BcmDiagProcessCmd ( &KArg );
   } while (0);
   put_user( KArg.bvStatus, &((PDIAGDRV_DIAG) arg)->bvStatus );
} /* DoDiagCommand */

static void DoOpenProcIf ( unsigned long arg )
{
   DIAGDRV_PROC_IF   KArg;
   char              ifName[BCM_DIAG_MAX_PROC_IFNAME_LEN];
   
   KArg.bvStatus = BCMDIAG_STATUS_ERROR;

   do {
      if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
         break;
      }

#if defined(CONFIG_COMPAT)
      if (is_compat_task())
         BCM_IOC_PTR_ZERO_EXT(KArg.ifName);
#endif
      if ((NULL != KArg.ifName) && (KArg.ifNameLen)) {
         if (KArg.ifNameLen >= BCM_DIAG_MAX_PROC_IFNAME_LEN)
            break;
         if( copy_from_user( ifName, KArg.ifName, KArg.ifNameLen ) != 0 )
            break;
         ifName[KArg.ifNameLen] = 0;
         KArg.bvStatus = BcmDiagDrvOpenProcIf(KArg.procMsgType, ifName);
      }
   } while (0);
   put_user( KArg.bvStatus, &((PDIAGDRV_PROC_IF) arg)->bvStatus );
}

static void DoCloseProcIf ( unsigned long arg )
{
   DIAGDRV_PROC_IF   KArg;
   char              ifName[BCM_DIAG_MAX_PROC_IFNAME_LEN];
   
   KArg.bvStatus = BCMDIAG_STATUS_ERROR;
   do {
      if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
         break;
      }

#if defined(CONFIG_COMPAT)
      if (is_compat_task())
         BCM_IOC_PTR_ZERO_EXT(KArg.ifName);
#endif
      if ((NULL != KArg.ifName) && (KArg.ifNameLen)) {
         if (KArg.ifNameLen >= BCM_DIAG_MAX_PROC_IFNAME_LEN)
            break;
         if( copy_from_user( ifName, KArg.ifName, KArg.ifNameLen ) != 0 )
            break;
         ifName[KArg.ifNameLen] = 0;
         KArg.bvStatus = BcmDiagDrvCloseProcIf(KArg.procMsgType, ifName);
      }
   } while (0);
   put_user( KArg.bvStatus, &((PDIAGDRV_PROC_IF) arg)->bvStatus );
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static struct proc_ops BcmDiagProcSkbFileOps = {
   .proc_read = BcmDiagProcSkbMsgRead,
   .proc_write = BcmDiagProcSkbMsgWrite,
   .proc_poll = BcmDiagProcSkbMsgPoll,
   .proc_open = BcmDiagProcSkbMsgOpen,
   .proc_release = BcmDiagProcMsgRelease,
   };
#else
static struct file_operations BcmDiagProcSkbFileOps = {
   .read = BcmDiagProcSkbMsgRead,
   .write = BcmDiagProcSkbMsgWrite,
   .poll = BcmDiagProcSkbMsgPoll,
   .open = BcmDiagProcSkbMsgOpen,
   .release = BcmDiagProcMsgRelease,
   };
#endif


ProcIfCtrl  procSkbCtrl;

static int BcmDiagProcSkbMsgOpen(struct inode * inode, struct file * file)
{
   ProcIfCtrl *pProcIfCtrl = &procSkbCtrl;
   
   pProcIfCtrl->fileOpened = TRUE;
   pProcIfCtrl->procMsgType = BCM_DIAG_PROC_SKB_MSG;
   init_waitqueue_head(&pProcIfCtrl->waitQue);
   file->private_data = pProcIfCtrl;
   BcmDiagPendingSkbMsgQueueEnable(1);
   printk("%s: pProcIfCtrl=%px procMsgType 0x%X\n", __FUNCTION__, pProcIfCtrl, pProcIfCtrl->procMsgType);
   
   return 0;
}

static int BcmDiagProcMsgRelease(struct inode * inode, struct file * file)
{
   ProcIfCtrl *pProcIfCtrl = (ProcIfCtrl *)file->private_data;
   
   pProcIfCtrl->fileOpened = FALSE;
   if (pProcIfCtrl->procMsgType == BCM_DIAG_PROC_SKB_MSG) {
      BcmDiagPendingSkbMsgQueueEnable(0);
      BcmDiagPendingSkbMsgQueueClear();
   }

   printk("%s: pProcIfCtrl=%px procMsgType 0x%X\n", __FUNCTION__, pProcIfCtrl, pProcIfCtrl->procMsgType);
   return 0;
}

static ProcIfCtrl *BcmDiagGetProcIfCtrl(int procMsgType)
{
   ProcIfCtrl *pProcIfCtrl = NULL;

   switch(procMsgType) {
      case BCM_DIAG_PROC_SKB_MSG:
         pProcIfCtrl = &procSkbCtrl;
         break;
      default:
         break;
   }

   return pProcIfCtrl;
}

void BcmDiagProcWakeup(int procMsgType)
{
   ProcIfCtrl *pProcIfCtrl = BcmDiagGetProcIfCtrl(procMsgType);
#if 0
   printk("%s: pProcIfCtrl=%px procMsgType 0x%X fileOpened=%d\n", __FUNCTION__, pProcIfCtrl, procMsgType, pProcIfCtrl->fileOpened);
#endif
   if(NULL != pProcIfCtrl) {
      if(pProcIfCtrl->fileOpened) 
         wake_up_interruptible(&pProcIfCtrl->waitQue);
      else {
         if(BCM_DIAG_PROC_SKB_MSG == procMsgType) {
            BcmDiagPendingSkbMsgQueueEnable(0);
            BcmDiagPendingSkbMsgQueueClear();
         }
         BcmDiagWriteString(0, DIAG_DSL_CLIENT, "BcmDiagProcWakeup: Discarding received/queued data");
      }
   }
}

static ssize_t BcmDiagProcSkbMsgRead(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
   ProcIfCtrl *pProcIfCtrl = (ProcIfCtrl*)file->private_data;

   while ( !BcmDiagPendingSkbMsgQueueSize() ) {
      if (file->f_flags & O_NONBLOCK) {
         return -EAGAIN;
      }
      else if (wait_event_interruptible(pProcIfCtrl->waitQue, BcmDiagPendingSkbMsgQueueSize() )) {
         return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
      }
   }

   BcmDiagPendingSkbMsgRead( buf , (long*)&count );

#ifdef XDSL_EOC_DBG
   printk("%s: pProcIfCtrl=%px read %d\n", __FUNCTION__, pProcIfCtrl, (int)count);
#endif

   return count;
}

static ssize_t BcmDiagProcSkbMsgWrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return -EFAULT;
}

static unsigned int BcmDiagProcSkbMsgPoll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	ProcIfCtrl *pProcIfCtrl = (ProcIfCtrl*)file->private_data;
	
	if( BcmDiagPendingSkbMsgQueueSize() )
		mask |= POLLIN | POLLRDNORM;    /* readable */
	else
		poll_wait(file, &pProcIfCtrl->waitQue, wait);
	if( BcmDiagPendingSkbMsgQueueSize() )
		mask |= POLLIN | POLLRDNORM;    /* readable */

#ifdef XDSL_EOC_DBG
	if(0 != mask)
		printk("%s: pProcIfCtrl=%px mask %08x\n", __FUNCTION__, pProcIfCtrl, mask);
#endif
	
	return mask;
}


static void BcmDiagDrvInitProcIfCtrl(void)
{
   memset((void*)&procSkbCtrl, 0, sizeof(procSkbCtrl));
}

static void BcmDiagDrvUnInitProcIfCtrl(void)
{
   ProcIfCtrl *pProcIfCtrl = &procSkbCtrl;
   BcmDiagDrvCloseProcIf(pProcIfCtrl->procMsgType, pProcIfCtrl->ifName);
}

static BCMDIAG_STATUS BcmDiagDrvOpenProcIf(int procMsgType, char *pIfName)
{
   ProcIfCtrl *pProcIfCtrl;
   BCMDIAG_STATUS ret = BCMDIAG_STATUS_SUCCESS;

   printk("%s: ifName %s procMsgType 0x%X\n", __FUNCTION__, pIfName, procMsgType);

   switch(procMsgType) {
      case BCM_DIAG_PROC_SKB_MSG:
         pProcIfCtrl = &procSkbCtrl;
         pProcIfCtrl->procMsgType = procMsgType;
         pProcIfCtrl->fileOpened = FALSE;
         strncpy(pProcIfCtrl->ifName, pIfName, sizeof(pProcIfCtrl->ifName)-1);
         pProcIfCtrl->pFileOps = &BcmDiagProcSkbFileOps;
         break;
      default:
         printk("%s: Unkown procMsgType 0x%X\n", __FUNCTION__, procMsgType);
         return BCMDIAG_STATUS_ERROR;
   }

   if ( pProcIfCtrl->pProcEntry == NULL )
      pProcIfCtrl->pProcEntry = proc_create(pProcIfCtrl->ifName, S_IRUSR, NULL, pProcIfCtrl->pFileOps);
   else
      printk("%s: pointer to struct proc_dir_entry not empty\n", __FUNCTION__);

   if(NULL == pProcIfCtrl->pProcEntry) {
      ret = BCMDIAG_STATUS_ERROR;
      printk("%s: proc_create(%s} failed!\n", __FUNCTION__, pProcIfCtrl->ifName);
   }

   return ret;
}

static BCMDIAG_STATUS BcmDiagDrvCloseProcIf(int procMsgType, char *pIfName)
{
   ProcIfCtrl *pProcIfCtrl;
   BCMDIAG_STATUS ret = BCMDIAG_STATUS_SUCCESS;

   switch(procMsgType) {
      case BCM_DIAG_PROC_SKB_MSG:
         pProcIfCtrl = &procSkbCtrl;
         break;
      default:
         printk("BcmDiagDrvCloseProcIf: Unkown procMsgType 0x%X\n", procMsgType);
         return BCMDIAG_STATUS_ERROR;
   }

   if(NULL != pProcIfCtrl->pProcEntry) {
      remove_proc_entry(pProcIfCtrl->ifName, NULL);
      pProcIfCtrl->pProcEntry = NULL;
      printk("%s: procEntry for %s is removed\n", __FUNCTION__, pProcIfCtrl->ifName);
   }
   else {
      printk("%s: procEntry for %s doesn't exist!\n", __FUNCTION__, pIfName);
      ret = BCMDIAG_STATUS_ERROR;
   }

   return ret;
}

module_init( diag_init );
module_exit( diag_cleanup );
MODULE_LICENSE("Proprietary");

