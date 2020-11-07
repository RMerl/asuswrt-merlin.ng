/*
* <:copyright-BRCM:2008:proprietary:standard
* 
*    Copyright (c) 2008 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/
/* Includes. */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <bcmtypes.h>
#include <bcm_map.h>
#include <board.h>
#include "pwrmngt.h"
#include "pwrmngtavs.h"
#include "bcmpwrmngtcfg.h"
#include "bcmpwrmngtdrv.h"
#if defined(CONFIG_BCM_PMC)
#include "pmc_drv.h"
#endif

#if defined CONFIG_ARM || defined CONFIG_ARM64
extern void set_cpu_arm_wait(int enable);
extern int get_cpu_arm_wait(void);
#define set_cpu_wait set_cpu_arm_wait
#define get_cpu_wait get_cpu_arm_wait
#elif defined(CONFIG_MIPS)
extern void set_cpu_r4k_wait(int enable); /* exported in cpu-probe.c */
extern int get_cpu_r4k_wait(void); /* exported in cpu-probe.c */ 
#define set_cpu_wait set_cpu_r4k_wait
#define get_cpu_wait get_cpu_r4k_wait
#else
//#warning "set/get_cpu_wait not defined"
#endif
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
extern void BcmPwrMngtSetDRAMSelfRefresh(unsigned int enable);
extern int BcmPwrMngtGetDRAMSelfRefresh(void);
#endif
#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE
extern void BcmPwrMngtSetSCR(void);
extern void BcmPwrMngtSetASCR(unsigned int freq_div);
extern void BcmPwrMngtSetAutoClkDivide(unsigned int enable);
extern int BcmPwrMngtGetAutoClkDivide(void);
 
static unsigned int BcmPwrMngtCalculateCpuSpeed(void)
{
   unsigned int cpu_speed;
   unsigned int cp0_ascr_asc;

   if (BcmPwrMngtGetAutoClkDivide()) {
      PWR_TRC("RATIO_ONE_ASYNC\n"); // Default when Power Management is compiled in
      cpu_speed = 256;
   } else {
#if defined (CONFIG_BCM960333)
      asm("mfc0 %0,$22,4" : "=d"(cp0_ascr_asc) : );
#else
      asm("mfc0 %0,$22,5" : "=d"(cp0_ascr_asc) : );
#endif
      cp0_ascr_asc = ( cp0_ascr_asc & MASK_ASCR ) >> MASK_ASCR_SHFT;
      PWR_TRC("calculateCpuSpeed() cp0_ascr_asc = 0x%01x ->", cp0_ascr_asc);

      switch (cp0_ascr_asc) {
#if !defined (CONFIG_BCM960333)
         case RATIO_ONE_SYNC:
           PWR_TRC("RATIO_ONE_SYNC\n"); // System default when Power Management is not compiled in
           cpu_speed = 0;  // Using 0 to represent 1 in SYNC mode
           break;
#endif

         case RATIO_ONE_ASYNC:
           PWR_TRC("RATIO_ONE_ASYNC\n"); // Experimental
           cpu_speed = 1;
           break;
 
         case RATIO_ONE_HALF:
            PWR_TRC("RATIO_ONE_HALF\n"); // Experimental
            cpu_speed = 2;
            break;

         case RATIO_ONE_QUARTER:
            PWR_TRC("RATIO_ONE_QUARTER\n"); // Experimental
            cpu_speed = 4;
            break;
 
         case RATIO_ONE_EIGHTH:
            PWR_TRC("RATIO_ONE_EIGHTH\n"); // Experimental
            cpu_speed = 8;
            break;
 
         default: 
            printk("bad value cp0_ascr_asc (0x%01x)\n", cp0_ascr_asc);
            cpu_speed = 0;
       }
   }

   return cpu_speed ;
} /* BcmPwrMngtCalculateCpuSpeed */


static void BcmPwrMngtSetNewCpuSpeed(int newspeed)
{
   unsigned int freq_div = 0;

   switch(newspeed)
   {
      case 8:
         BcmPwrMngtSetAutoClkDivide(0);
         freq_div = RATIO_ONE_EIGHTH;
         PWR_TRC("%s- 1/8 of full speed\n",__FUNCTION__);
         break;

      case 4:
         BcmPwrMngtSetAutoClkDivide(0);
         freq_div = RATIO_ONE_QUARTER;
         PWR_TRC("%s- 1/4 of full speed\n",__FUNCTION__);
         break;

      case 2:
         BcmPwrMngtSetAutoClkDivide(0);
         freq_div = RATIO_ONE_HALF;
         PWR_TRC("%s-BcmPwrMngtSetNewCpuSpeed- 1/2 of full speed\n",__FUNCTION__);
         break;

      case 1:
         BcmPwrMngtSetAutoClkDivide(0);
         freq_div = RATIO_ONE_ASYNC;
         PWR_TRC("%s- fullspeed (async) \n",__FUNCTION__);
         break;

      case 0:
#if defined (CONFIG_BCM960333)
         printk("Unsupported\n");
#else
         BcmPwrMngtSetAutoClkDivide(0);
         PWR_TRC("%s- Back to SYNC mode\n",__FUNCTION__);
#endif
         break;

      case 256:
         BcmPwrMngtSetAutoClkDivide(1);
         break;

      default:
         printk("%s- %d wrong value, setting to fullspeed (async)\n",__FUNCTION__,newspeed);
         return;
   }

   if (newspeed != 256) {
       if (newspeed != 0) {
          // Switch back to RATIO_ONE_ASYNC first
          BcmPwrMngtSetASCR(RATIO_ONE_ASYNC);

          // Then set the new requested ratio
          BcmPwrMngtSetASCR(freq_div); 
       } else {
          // Switch back to SYNC mode
          BcmPwrMngtSetSCR();
       }
   }

   return;
} /* BcmPwrMngtSetNewCpuSpeed */

static int BcmPwrMngtHandlerForCPUSpeed(PWRMNGT_OPCODE op, int setval)
{
   int cpuSpeed;

   PWR_TRC ("pwrmngt procfs set/get handler for CPUSpeed.");

   cpuSpeed = BcmPwrMngtCalculateCpuSpeed();
   if (op == SET) {
      if(cpuSpeed != setval) {
         BcmPwrMngtSetNewCpuSpeed(setval);
      }
      return 0;
   }
   else {// get
      return cpuSpeed;
   }
} /* BcmPwrMngtHandlerForCPUSpeed */
#elif defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)

extern void bcm_arm_cpuidle_set_auto_clk_divide(unsigned int enabled);
extern int bcm_arm_cpuidle_get_auto_clk_divide(void);
extern void bcm_arm_cpuidle_set_clk_divider(unsigned int maxdiv);
extern int bcm_arm_cpuidle_get_clk_divider(void);

static int BcmPwrMngtHandlerForCPUSpeed(PWRMNGT_OPCODE op, int setval)
{
   int divider = 0;
   int enabled = 0;

   // Handle the CPU IDLE implementation
   if (op == SET) {
      switch (setval) {
         case 0:   bcm_arm_cpuidle_set_clk_divider(1); bcm_arm_cpuidle_set_auto_clk_divide(0); break;
         case 1:   bcm_arm_cpuidle_set_clk_divider(1); bcm_arm_cpuidle_set_auto_clk_divide(0); break;
         case 2:   bcm_arm_cpuidle_set_clk_divider(2); bcm_arm_cpuidle_set_auto_clk_divide(0); break;
         case 4:   bcm_arm_cpuidle_set_clk_divider(4); bcm_arm_cpuidle_set_auto_clk_divide(0); break;
         case 8:   bcm_arm_cpuidle_set_clk_divider(8); bcm_arm_cpuidle_set_auto_clk_divide(0); break;
         case 256: bcm_arm_cpuidle_set_clk_divider(1); bcm_arm_cpuidle_set_auto_clk_divide(1); break;
         default:  return -1;
      }
   }
   divider = bcm_arm_cpuidle_get_clk_divider();
   enabled = bcm_arm_cpuidle_get_auto_clk_divide();
   if (1 == divider)
   {
      // Special value "256" just mean "1, but with auto enabled"
      return (enabled ? 256 : 1);
   }
   // Note: that there are no special "but with auto enabled" values for 1, 2 and 4.
   //       So if auto is enabled, we must return values as if the auto is disabled.
   return divider;
}
#elif defined(CONFIG_BCM_CPU_FREQ_PWRSAVE)
#include <linux/cpufreq.h>

static int BcmPwrMngtHandlerForCPUSpeed(PWRMNGT_OPCODE op, int setval)
{
   unsigned freq, max;

   if (op == SET) {
      if (setval == PWRMNGT_MIPS_CPU_SPEED_MAX_VAL) {
#if defined CONFIG_CPU_FREQ_GOV_INTERACTIVE
         cpufreq_set_speed("interactive", 0);
#else /* CONFIG_CPU_FREQ_GOV_ONDEMAND */
         cpufreq_set_speed("ondemand", 0);
#endif
      } else {
         // switch to userspace governor and set speed fraction
         cpufreq_set_speed("userspace", setval ?: 1);
      }
   }

   freq = cpufreq_get_freq_max(&max);
   return freq ? max / freq : PWRMNGT_MIPS_CPU_SPEED_MAX_VAL;
}
#endif

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
static int BcmPwrMngtHandlerForDRAMSelfRefresh(PWRMNGT_OPCODE op, int setval)
{
   int dramSelfRefreshEn;

   PWR_TRC("pwrmngt procfs set/get handler for Memory Self-Refresh when idle\n");

   dramSelfRefreshEn = BcmPwrMngtGetDRAMSelfRefresh();
   if (op == SET) {
      if(dramSelfRefreshEn != setval) {
         BcmPwrMngtSetDRAMSelfRefresh(setval);
      }
      return 0;
   }
   else {// get
      return dramSelfRefreshEn;
   }
} /* BcmPwrMngtHandlerForDRAMSelfRefresh */
#endif

static int BcmPwrMngtHandlerForCPUr4KWait(PWRMNGT_OPCODE op, int setval)
{
   int cpur4kWaitEn;

   PWR_TRC("pwrmngt procfs set/get handler for CPU r4K wait instruction use when idle\n");

   cpur4kWaitEn = get_cpu_wait();
   if (op == SET) {
      if(cpur4kWaitEn != setval) {
         set_cpu_wait(setval);
      }
      return 0;
   }
   else {// get
      return cpur4kWaitEn;
   }
} /* BcmPwrMngtHandlerForCPUr4KWait */

#if defined(CONFIG_BCM_PMC)
static int BcmPwrMngtHandlerForAvs(PWRMNGT_OPCODE op, int setval)
{
   int avsDisabled;

   PWR_TRC("pwrmngt procfs set/get handler for PMC based Adaptive Voltage Scaling\n"); 

   if (GetAvsDisableState(0, &avsDisabled)) {
      return -1;
   }

   if (op == SET) {
#if defined(CONFIG_BCM_MoCA_AVS)
      /* Force enable MoCA AVS if global AVS is enabled. Do not disable MoCA AVS ever */
      if(setval)
         BcmPwrMngtEnableAvs(setval);
#endif      
      return -1;
   }
   else {// get
     return avsDisabled?0:1;
   }
} /* BcmPwrMngtHandlerForAvs */
#else
#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
static int BcmPwrMngtHandlerForAvs(PWRMNGT_OPCODE op, int setval)
{
   int avsEn;

   PWR_TRC("pwrmngt procfs set/get handler for Adaptive Voltage Scaling\n"); 

   avsEn = BcmPwrMngtAvsEnabled();
   if (op == SET) {
      if(avsEn != setval) {
         BcmPwrMngtEnableAvs(setval);
      }
      return 0;
   }
   else {// get
      return avsEn;
   }
} /* BcmPwrMngtHandlerForAvs */
#endif
#endif

#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
static int BcmPwrMngtHandlerForAvsLog(PWRMNGT_OPCODE op, int setval)
{
   int avsLog;

   PWR_TRC("pwrmngt procfs set/get handler for Adaptive Voltage Scaling\n"); 

   avsLog = BcmPwrMngtAvsLogGet();
   if (op == SET) {
      if(avsLog != setval) {
         BcmPwrMngtAvsLogSet(setval);
      }
      return 0;
   }
   else {// get
      return avsLog;
   }
} /* BcmPwrMngtHandlerForAvsLog */
#endif

/***************************************************************************
 * Function Name: BcmPwrMngtInitialize
 * Description  : Power Management Initialization entry point function.
 * Returns      : PWRMNGTSTS_SUCCESS if successful or error status.
 ***************************************************************************/

PWRMNGT_STATUS BcmPwrMngtInitialize(PPWRMNGT_CONFIG_PARAMS  pInitParams )
{
   PWRMNGT_STATUS status = PWRMNGTSTS_SUCCESS;

   PWR_TRC ("PWRMNGT: %s entry \n", __FUNCTION__ );
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE) || defined(CONFIG_BCM_CPU_FREQ_PWRSAVE) || defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)
   BcmPwrMngtHandlerForCPUSpeed(SET, pInitParams->cpuspeed);
#endif
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
   BcmPwrMngtHandlerForDRAMSelfRefresh(SET, pInitParams->dramSelfRefresh);
#endif
   BcmPwrMngtHandlerForCPUr4KWait(SET, pInitParams->cpur4kwait);
#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
   BcmPwrMngtHandlerForAvs(SET, pInitParams->avs);
#endif
   PWR_TRC ("PWRMNGT: %s exit status %d \n", __FUNCTION__, status ) ;

   return( status) ;

} /* BcmPwrMngtInitialize */


/***************************************************************************
 * Function Name: BcmPwrMngtUninitialize
 * Description  : Power Management uninitialization entry point function.
 * Returns      : PWRMNGTSTS_SUCCESS if successful or error status.
 ***************************************************************************/
PWRMNGT_STATUS BcmPwrMngtUninitialize( void )
{

   PWR_TRC ("PWRMNGT: %s entry \n", __FUNCTION__ );


   PWR_TRC ("PWRMNGT: %s exit. \n", __FUNCTION__) ;

   return( PWRMNGTSTS_SUCCESS ) ;
} /* BcmPwrMngtUninitialize */


/***************************************************************************
 * Function Name: BcmPwrMngtGetConfig
 * Description  : Returns the Power Mangement config parameters.
 * Returns      : PWRMNGTSTS_SUCCESS if successful or error status.
 ***************************************************************************/
PWRMNGT_STATUS BcmPwrMngtGetConfig( PPWRMNGT_CONFIG_PARAMS pCfgParams,
                                    UINT32 configMask )
{
   PWRMNGT_STATUS status = PWRMNGTSTS_SUCCESS;

   PWR_TRC ("PWRMNGT: %s entry \n", __FUNCTION__ );
   
   if(configMask & PWRMNGT_CFG_PARAM_CPUSPEED_MASK)
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE) || defined(CONFIG_BCM_CPU_FREQ_PWRSAVE) || defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)
      pCfgParams->cpuspeed = BcmPwrMngtHandlerForCPUSpeed(GET, 0);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   if(configMask & PWRMNGT_CFG_PARAM_MEM_SELF_REFRESH_MASK)
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
      pCfgParams->dramSelfRefresh = BcmPwrMngtHandlerForDRAMSelfRefresh(GET,0);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   if(configMask & PWRMNGT_CFG_PARAM_CPU_R4K_WAIT_MASK)
      pCfgParams->cpur4kwait = BcmPwrMngtHandlerForCPUr4KWait(GET, 0);

   if(configMask & PWRMNGT_CFG_PARAM_MEM_AVS_MASK)
#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS) || defined(CONFIG_BCM_PMC)
      pCfgParams->avs = BcmPwrMngtHandlerForAvs(GET,0);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   if(configMask & PWRMNGT_CFG_PARAM_MEM_AVS_LOG_MASK)
#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
      pCfgParams->avsLog = BcmPwrMngtHandlerForAvsLog(GET,0);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   PWR_TRC ("PWRMNGT: %s exit. status = %d \n", __FUNCTION__, status) ;

   return( status) ;
} /* BcmPwrMngtGetConfig */

/***************************************************************************
 * Function Name: BcmPwrMngtSetConfig
 * Description  : Sets the Power Mangement config parameters. 
 * Returns      : PWRMNGTSTS_SUCCESS if successful or error status.
 ***************************************************************************/
PWRMNGT_STATUS BcmPwrMngtSetConfig( PPWRMNGT_CONFIG_PARAMS pCfgParams, 
                                    UINT32 configMask )
{
   PWRMNGT_STATUS status = PWRMNGTSTS_SUCCESS;

   PWR_TRC ("PWRMNGT: %s entry \n", __FUNCTION__ );

   if(configMask & PWRMNGT_CFG_PARAM_CPUSPEED_MASK)
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE) || defined(CONFIG_BCM_CPU_FREQ_PWRSAVE) || defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)
      BcmPwrMngtHandlerForCPUSpeed(SET, pCfgParams->cpuspeed);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   if(configMask & PWRMNGT_CFG_PARAM_MEM_SELF_REFRESH_MASK)
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
      BcmPwrMngtHandlerForDRAMSelfRefresh(SET, pCfgParams->dramSelfRefresh);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   if(configMask & PWRMNGT_CFG_PARAM_CPU_R4K_WAIT_MASK)
      BcmPwrMngtHandlerForCPUr4KWait(SET, pCfgParams->cpur4kwait);

   if(configMask & PWRMNGT_CFG_PARAM_MEM_AVS_MASK)
#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
      BcmPwrMngtHandlerForAvs(SET, pCfgParams->avs);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   if(configMask & PWRMNGT_CFG_PARAM_MEM_AVS_LOG_MASK)
#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
      BcmPwrMngtHandlerForAvsLog(SET, pCfgParams->avsLog);
#else
      status = PWRMNGTSTS_NOT_SUPPORTED;
#endif

   PWR_TRC ( "PWRMNGT: %s exit. \n", __FUNCTION__) ;

   return( status) ;

} /* BcmPwrMngtSetConfig */
