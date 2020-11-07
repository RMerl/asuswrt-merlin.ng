/***************************************************************************
*    Copyright 2004  Broadcom Corporation
*    All Rights Reserved
*    No portions of this material may be reproduced in any form without the
*    written permission of:
*             Broadcom Corporation
*             16215 Alton Parkway
*             P.O. Box 57013
*             Irvine, California 92619-7013
*    All information contained in this document is Broadcom Corporation
*    company private, proprietary, and trade secret.
*
****************************************************************************
*
*    Filename: profdrv.c
*
****************************************************************************
*    Description:
*
*      Implementation of the profiler device driver in the kernel space.
*
****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/timex.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#include "profiler.h"

/* ---- Device driver init and cleanup -------------------------------- */

static int __init profdrv_init( void );
static void __exit profdrv_cleanup( void );

/* ---- Device driver entry points ------------------------------------ */

static int profdrv_open( struct inode *inode, struct file *filp );
static int profdrv_close( struct inode *inode, struct file *filp );
static int profdrv_ioctl( struct inode *inode, struct file *flip, 
                          unsigned int command, unsigned long arg );

static struct file_operations profdrv_device_driver_ops =
{
    ioctl:     profdrv_ioctl,
    open:      profdrv_open,
    release:   profdrv_close,
};

static unsigned profdrv_data_dump_flag;
static unsigned profdrv_data_clean_flag;
static unsigned profdrv_data_prof_ops;
static PROFILER_IOCTL_DATA profdrv_data;
static PROFILER_CPU_UTILIZATION profdrv_cpu;

#if defined( MODULE )

/***************************************************************************
 * Function Name: init_module
 * Description  : Initial function that is called if this driver is compiled
 *                as a module.  If it is not, profdrv_init is called in
 *                chr_dev_init() in drivers/char/mem.c.
 * Returns      : None.
 ***************************************************************************/

int init_module( void ) 
{
    return profdrv_init();
}

/***************************************************************************
 * Function Name: cleanup_module
 * Description  : Final function that is called if this driver is compiled
 *                as a module.
 * Returns      : None.
 ***************************************************************************/
void cleanup_module(void)
{
    profdrv_cleanup();
    return;
}

#else

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/

module_init( profdrv_init );
module_exit( profdrv_cleanup );

#endif /* MODULE */

/***************************************************************************
 * Function Name: profdrv_init
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init profdrv_init( void )
{
    printk( "=> profdrv_init ... ENTER\n" );

    profdrv_data_dump_flag = 0;
    profdrv_data_clean_flag = 0;
    profdrv_data_prof_ops = 0;
   memset( &profdrv_cpu, 0, sizeof(profdrv_cpu));
   memset( &profdrv_data, 0, sizeof(profdrv_data));

    register_chrdev( PROFDRV_DEVICE_DRIVER_MAJOR, 
                     "bcmprof", &profdrv_device_driver_ops );
    /*
        Initializes the profiler database.
    */
    profiler_init();

    printk( "=> profdrv_init ... EXIT\n" );

    return 0;
} 

/***************************************************************************
 * Function Name: profdrv_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit profdrv_cleanup( void )
{
    printk( "=> profdrv_cleanup\n" );
    unregister_chrdev( PROFDRV_DEVICE_DRIVER_MAJOR, "bcmprof" );
    return;
} 

/***************************************************************************
 * Function Name: profdrv_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int profdrv_open( struct inode *inode, struct file *filp )
{
    // printk( "=> profdrv_open\n" );
    /* Do nothing, but get rid of warnings */
    (void)inode;
    (void)filp;
    return 0;
}

/***************************************************************************
 * Function Name: profdrv_close
 * Description  : Called when an application closes this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int profdrv_close( struct inode *inode, struct file *filp )
{
    // printk( "=> profdrv_close\n" );
    /* Do nothing, but get rid of warnings */
    (void)inode;
    (void)filp;
    return 0;
}

/***************************************************************************
 * Function Name: profdrv_ioctl
 * Description  : Main entry point for an application ioctl commands.
 * Returns      : 0 - success or error
 ***************************************************************************/
static int profdrv_ioctl( struct inode *inode, struct file *filp, 
    unsigned int command, unsigned long arg )
{
    int err = 0;

    (void)inode;
    (void)filp;

    switch ( command )
    {
        case PROFILER_IOCTL_GET_DATA_DUMP:
      {
         PROFILER_COLLECTED_DATA *pData = profiler_get_data_dump();
         int index;

         for( index = 0 ; index < PROFILER_MAX_MONITORED_PROFILE ; index++ )
         {
            err = copy_to_user( (void *)((PROFILER_COLLECTED_DATA *)arg + index), 
                           pData + index, 
                           sizeof(PROFILER_COLLECTED_DATA) );  
         }  
        }
        break;

      case PROFILER_IOCTL_GET_RECSEQ_DATA_DUMP:
      {
         PROFILER_RECSEQ_DATA *pData = profiler_get_recseq_data_dump();
         err = copy_to_user( (void *)arg, pData, sizeof(PROFILER_RECSEQ_DATA) * PROFILER_MAX_RECSEQ );  
      }
      break;

      case PROFILER_IOCTL_GET_RECSEQ_DATA_INDEX:
      {         
         unsigned int index = profiler_get_recseq_data_index();
         err = copy_to_user( (void *)arg, &index, sizeof(index) );  
      }
      break;

        case PROFILER_IOCTL_SET_DATA_CLEAN:
            err = get_user( profdrv_data_clean_flag, (unsigned *)arg );
            if( err == 0 )
            {
                if( profdrv_data_clean_flag == 1 )
                {
                    /*
                        Re-initialize the data collected.
                    */
                    PROFILER_REINIT_COLLECTED();
                    profdrv_data_clean_flag = 0;
                }   
            }
        break;

        case PROFILER_IOCTL_SET_PROF_OPS:
            err = get_user( profdrv_data_prof_ops, (unsigned *)arg );
            if( err == 0 )
            {
                if( profdrv_data_prof_ops != 0 )
                {
                    /*
                        Start the data collection now.
                    */
                    PROFILER_START_COLLECT();
                }
                else
                {
                    /*
                        Stop the data collection now.
                    */
                    PROFILER_STOP_COLLECT();
                }   
            }
        break;

        case PROFILER_IOCTL_REGISTER_CALL:
            err = copy_from_user( &profdrv_data, (void *)arg, sizeof(profdrv_data) );
            if( err == 0 )
            {
                kernel_profiler_register( profdrv_data.name, PROFILER_SOURCE_USER );
            }
        break;

        case PROFILER_IOCTL_DEREGISTER_CALL:
            err = copy_from_user( &profdrv_data, (void *)arg, sizeof(profdrv_data) );
            if( err == 0 )
            {
                kernel_profiler_deregister( profdrv_data.name, PROFILER_SOURCE_USER );
            }
        break;

        case PROFILER_IOCTL_START_CALL:
            err = copy_from_user( &profdrv_data, (void *)arg, sizeof(profdrv_data) );
            if( err == 0 )
            {
                kernel_profiler_start( profdrv_data.name, PROFILER_SOURCE_USER );
            }
        break;

        case PROFILER_IOCTL_STOP_CALL:
            err = copy_from_user( &profdrv_data, (void *)arg, sizeof(profdrv_data) );
            if( err == 0 )
            {
                kernel_profiler_stop( profdrv_data.name, PROFILER_SOURCE_USER );
            }
        break;

      case PROFILER_IOCTL_PROFILER_STATUS_DATA:
      {
         PROFILER_STATUS status;
         profiler_get_status( &status );  
         err = copy_to_user( (void *)arg, &status, sizeof(PROFILER_STATUS) );
      }
      break;

      case PROFILER_IOCTL_SET_CPU_UTIL:
            err = copy_from_user( &profdrv_cpu, (void *)arg, 
                          sizeof(PROFILER_CPU_UTILIZATION) );
            profiler_set_cpu_util( &profdrv_cpu );
        break;

      case PROFILER_IOCTL_GET_CPU_UTIL:
            profiler_get_cpu_util( &profdrv_cpu );
         err = copy_to_user( (void *)arg, &profdrv_cpu, 
                        sizeof(PROFILER_CPU_UTILIZATION) );
        break;

      default:
            printk( "\n=> profdrv_ioctl: invalid command 0x%x\n", command );
            err = -ENOTTY;
        break;
   }

    if( err != 0 )
    {
        printk( "\n=> profdrv_ioctl: error executing 0x%x (%d)\n", 
            command, err );
    }

    return err ;
}

