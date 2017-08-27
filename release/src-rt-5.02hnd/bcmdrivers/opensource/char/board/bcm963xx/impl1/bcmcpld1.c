/*
<:copyright-BRCM:2009:GPL/GPL:standard

   Copyright (c) 2009 Broadcom 
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
/***************************************************************************
* File Name  : bcmcpld1.c
*
* Description: This file contains driver code for a cpld used to place the
               system in standby mode and eventually for synchronizing NTR
*
***************************************************************************/

#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <bcmtypes.h>
#include <board.h>
#include "boardparms.h"
#include "bcmSpiRes.h"
#include "bcm_intr.h"

#define BCMCPLD1_SPI_CS                                  3
#define BCMCPLD1_SPI_CLK_FREQ                     12500000
#if defined(CONFIG_BCM96328)
#define BCMCPLD1_EXT_INT           INTERRUPT_ID_EXTERNAL_0
#else
#define BCMCPLD1_EXT_INT           INTERRUPT_ID_EXTERNAL_2
#endif
#define BCMCPLD1_WRITE                             (0 << 7)
#define BCMCPLD1_READ                              (1 << 7)

#define BCMCPLD1_STATUS_ADDR                             0
#define BCMCPLD1_CTRL_ADDR                               1
#define BCMCPLD1_TIMER_ADDR                              2
#define BCMCPLD1_MS_START_ADDR                           3
#define BCMCPLD1_LS_START_ADDR                           4
#define BCMCPLD1_INT_STATUS_ADDR                         8
#define BCMCPLD1_INT_MASK_ADDR                           9
#define BCMCPLD1_GLB_INT_MASK_ADDR                      10

#define BCMCPLD1_SHUTDOWN_NORMAL                         0
#define BCMCPLD1_SHUTDOWN_STANDBY                        1

#if defined(CONFIG_BCM96328) 
#define BCMCPLD1_SPI_BUS_NUM HS_SPI_BUS_NUM
#else
#define BCMCPLD1_SPI_BUS_NUM LEG_SPI_BUS_NUM
#endif

typedef struct {
    unsigned int shutdown_mode;
} CPLD_CTRL, *PCPLD_CTRL;

CPLD_CTRL g_CpldContext;
static struct work_struct EnterStandbyWork;

#if 0 // Unused for now
/***************************************************************************
 * Function Name: BcmCpld1SpiRd
 * Description  : Reads CPLD registers over SPI.
 * Returns      : value read over SPI
 ***************************************************************************/
static unsigned int BcmCpld1SpiRd(unsigned int addr, unsigned int length)
{
   unsigned char txbuf[4], rxbuf[4];
   unsigned int retval;

   txbuf[0] = BCMCPLD1_READ | addr;
   // There is 1 byte of padding to give time to the CPLD to respond, hence length+2 below
   BcmSpiSyncTrans(txbuf, rxbuf, 0, length+2, BCMCPLD1_SPI_BUS_NUM, BCMCPLD1_SPI_CS);
   if (length == 1) {
      retval = (unsigned int)rxbuf[2];
   }
   else { // Assume 2 bytes
      retval = (((unsigned int)rxbuf[2])<<8) | (unsigned int)rxbuf[3];
   }
   return retval;
} /* BcmCpld1SpiRd */
#endif

/***************************************************************************
 * Function Name: BcmCpld1SpiWr
 * Description  : Writes CPLD registers over SPI.
 * Returns      : void
 ***************************************************************************/
static void BcmCpld1SpiWr(unsigned int addr, unsigned int setval, unsigned int length)
{
   unsigned char buf[3];

   buf[0] = BCMCPLD1_WRITE | addr;
   if (length == 1) {
      buf[1] = (unsigned char)( setval        & 0x000000FF);
   }
   else { // Assume 2 bytes
      buf[1] = (unsigned char)((setval >>  8) & 0x000000FF);
      buf[2] = (unsigned char)( setval        & 0x000000FF);
   }
   BcmSpiSyncTrans(buf, NULL, 0, length+1, BCMCPLD1_SPI_BUS_NUM, BCMCPLD1_SPI_CS);
} /* BcmCpld1SpiWr */

/***************************************************************************
 * Function Name: BcmCpld1EnterStandbyWork
 * Description  : Work thread to enter standby mode
 * Returns      : void
 ***************************************************************************/
static void BcmCpld1EnterStandbyWork(struct work_struct *work)
{
   // Temporarily Standby forever for now
   // Will be replaced with code that sends message using netlink to userspace
   // which then computes the amount of time to go into standby
   BcmCpld1SpiWr(BCMCPLD1_TIMER_ADDR, 0xFFFF, 2);

   // Go into standby
   BcmCpld1SetShutdownMode();
   //kerSysMipsSoftReset();

   kernel_restart(NULL);
   return;
}

/***************************************************************************
 * Function Name: BcmCpld1InterruptHandler
 * Description  : Handles Interrupts coming from CPLD
 * Returns      : void
 ***************************************************************************/
static int BcmCpld1InterruptHandler(unsigned int addr, unsigned int setval, unsigned int length)
{
   // Acknowledge the interrupt in CPLD
   BcmCpld1SpiWr(BCMCPLD1_INT_STATUS_ADDR, 1, 1);

   INIT_WORK(&EnterStandbyWork, BcmCpld1EnterStandbyWork);
   schedule_work(&EnterStandbyWork);

   return IRQ_HANDLED;
} /* BcmCpld1InterruptHandler */

/***************************************************************************
 * Function Name: BcmCpld1Initialize
 * Description  : Initialization code when external CPLD is used to control timer.
 * Returns      : 0 on success, -1 otherwise
 ***************************************************************************/
int BcmCpld1Initialize(void)
{
   // Reserve SPI bus to control external CPLD Standby Timer
   if ( SPI_STATUS_OK != BcmSpiReserveSlave(BCMCPLD1_SPI_BUS_NUM, BCMCPLD1_SPI_CS, BCMCPLD1_SPI_CLK_FREQ) )
      return( -1 ) ;

   BcmHalMapInterrupt((FN_HANDLER)BcmCpld1InterruptHandler, (void*)0, BCMCPLD1_EXT_INT);
#if !defined(CONFIG_ARM)
   BcmHalInterruptEnable (BCMCPLD1_EXT_INT);
#endif
 
   BcmCpld1SpiWr(BCMCPLD1_INT_MASK_ADDR, 1, 1);
   BcmCpld1SpiWr(BCMCPLD1_INT_STATUS_ADDR, 1, 1);
   BcmCpld1SpiWr(BCMCPLD1_GLB_INT_MASK_ADDR, 1, 1);

   g_CpldContext.shutdown_mode = BCMCPLD1_SHUTDOWN_NORMAL;

   return( 0 ) ;
} /* BcmCpld1Initialize */

/***************************************************************************
 * Function Name: BcmCpld1CheckShutdownMode
 * Description  : Final step of soft reset sequence determines if this
 *                a normal soft reset or if we are going into Standby mode
 * Returns      : void
 ***************************************************************************/
void BcmCpld1CheckShutdownMode(void)
{
   if (g_CpldContext.shutdown_mode == BCMCPLD1_SHUTDOWN_STANDBY)	{
      BcmCpld1SpiWr(BCMCPLD1_CTRL_ADDR, 0, 1);
      for (;;) {
		 printk("System entering Standby Mode...\n");
         msleep(100);
      }
   }

   return;
} /* BcmCpld1CheckShutdownMode */

/***************************************************************************
 * Function Name: BcmCpld1SetStandbyMode
 * Description  : Programs final step of soft reset sequence to be standby mode.
 *                Application still needs to intiate a reboot afterwards.
 * Returns      : void
 ***************************************************************************/
void BcmCpld1SetShutdownMode(void)
{
   g_CpldContext.shutdown_mode = BCMCPLD1_SHUTDOWN_STANDBY;

   return;
} /* BcmCpld1SetShutdownMode */

/***************************************************************************
 * Function Name: BcmCpld1SetStandbyTimer
 * Description  : Programs duration of standby in CPLD
 * Returns      : void
 ***************************************************************************/
void BcmCpld1SetStandbyTimer(unsigned int duration)
{
   BcmCpld1SpiWr(BCMCPLD1_TIMER_ADDR, duration, 2);

   return;
} /* BcmCpld1SetStandbyMode */

MODULE_LICENSE("GPL");
