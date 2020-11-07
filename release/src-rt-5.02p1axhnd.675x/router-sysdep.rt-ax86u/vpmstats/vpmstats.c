/***************************************************************************
*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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
* :>
*
*****************************************************************************
*    Description:
*
*    Voice Performance Metrics
*
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <signal.h>


#define DEFAULT_POLLINTER 500 /* 500msec = 0.5sec */
#define STATS_MAX_ENTRY   200

/* ---- Structure  -------------------------- */

typedef enum 
{
   STATUS_STOP,
   STATUS_RUN,
} VpmStatusType;

typedef enum 
{
   VPM_ALL_STATS,
   VPM_PCM_STATS,
   VPM_DSP_STATS,
   VPM_DECT_STATS,
   VPM_NONE_STATS,
} VpmStatsType;

typedef enum 
{
   VPM_ALL,          // switch use to specify all the stats
   VPM_SPECIFIC,     // switch use to specify specific stats e,g,. pcm, dsp, dect
   VPM_NUM_ITER,     // switch use to specify the number of iterations to run the application
   VPM_POLL_INTER,   // switch use to specify the polling interval time in milliseconds
   VPM_LAST,
} VpmSwitchType;

typedef struct VPM_MAP
{
   VpmSwitchType type;
   const char *name;
} VPM_MAP;

static VPM_MAP VpmMap[] =
{
   {VPM_ALL,                       "a"    },
   {VPM_SPECIFIC,                  "s"    },
   {VPM_NUM_ITER,                  "n"    },
   {VPM_POLL_INTER,                "t"    },
   {VPM_LAST,                      "error"}
};

typedef struct
{
   // dspTaskRunCyc represents the processor 
   // cycle counts which are consumed by DSP Task
   unsigned long dspTaskRunCyc;
   
   // dspTaskIntRunCyc represents the processor 
   // cycle counts between two consecutive DSP Tasks 
   // calls
   unsigned long dspTaskIntRunCyc;

} DSPTASKSTATS;

typedef struct
{
   // interruptErrCnt represents invalid pcm dma interrupt state
   // since voice started
   unsigned int  interruptErrCnt;
 
   // minInterruptIntCyc represents the minimum processor 
   // cycle counts between two consecutive pcm dma interrupts 
   // since voice started
   unsigned int  minInterruptIntCyc;

   // maxInterruptIntCyc represents the maximum processor 
   // cycle counts between two consecutive pcm dma interrupts 
   // since voice started
   unsigned int  maxInterruptIntCyc;

   // avgInterruptIntCyc represents the average processor
   // cycle counts between two consecutive pcm dma interrupts
   // since  past one second
   unsigned int  avgInterruptIntCyc;


   // minIsrRunCyc represents the minimum processor cycle counts 
   // taken by ISR since voice started
   unsigned int  minIsrRunCyc;
   
   // maxIsrRunCyc represents the maximum processor cycle counts 
   // taken by ISR since voice started
   unsigned int  maxIsrRunCyc;

   // avgIsrRunCyc represents the average processor cycle counts 
   // taken by ISR since past one second
   unsigned int  avgIsrRunCyc;

} PCMDMASTATS;


typedef void (*SIGNAL_HANDLER)( int sig, siginfo_t* siginfo, void* notused );
unsigned status;
FILE *pcmStatsFileDesc  = NULL;
FILE *dspStatsFileDesc  = NULL;
FILE *dectStatsFileDesc = NULL;

/* ---- Private Function Prototypes -------------------------- */

static int vpm_process( int argc, char **argv );
static void signalHandler( int sig, siginfo_t* siginfo, void* notused);
int openFileDesc( unsigned int type );
void vpmAllStats();
void vpmPcmStats();
void vpmDspStats();
void vpmDectStats();
void closeFileDesc();

/******************************************************************************
** FUNCTION:   signalRegisterHandler
**
** PURPOSE:    registers signal handler for the application 
**
** PARAMETERS: signal handler passed by the application.
**
** RETURNS:
**
** NOTE:
** 
** *****************************************************************************/
void signalRegisterHandler( SIGNAL_HANDLER handler )
{
   struct sigaction sa;

   sa.sa_sigaction = handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;
   sigaction( SIGTSTP,  &sa,  NULL );     /* keyboard stop */
}

/*****************************************************************************
* FUNCTION:  cmd_usage 
*
* PURPOSE:    
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void cmd_usage()
{
   printf("Usage:\n");
   printf("------\n"); 
   printf("\tvpmstats a       :all stats\n");
   printf("\tvpmstats s pcm   :displays the pcm dma stats\n");
   printf("\tvpmstats s dsp   :displays the dsp task stats\n");
   printf("\tvpmstats s dect  :displays the dect mac interrupt\n");
   printf("\tvpmstats -h      :display command usage\n");
   printf("\tctrl+z           :stop the application\n");
   
   printf("\nOptional:\n"); 
   printf("---------\n"); 
   printf("\t'n'                :number of iterations to run the application\n");
   printf("\t't'                :polling interval time in millisecond\n");
   
   printf("\nFor example:\n");
   printf("------------\n"); 
   printf("\tvpmstats a n 10 t 500 \n\n");
}
/*****************************************************************************
* FUNCTION:  main function 
*
* PURPOSE:    main entry function to process the vpm command
*
* PARAMETERS:
*
* RETURNS:    return 0 if success else 1
*
*****************************************************************************/
int main(int argc, char *argv[])
{
   int err = 0;
   
   argv++;
   if ( (*argv != NULL ) && (strcmp(*argv, "-h") != 0) )
   {
      /* process command */
      err = vpm_process(argc, argv);
      if (err)
      {
         printf("Error: invalid command format \n");
      }
   }
   else
   {
      cmd_usage();
   }
   return err;
}
/*****************************************************************************
* FUNCTION:  vpm_process 
*
* PURPOSE:   process the vpm command
*
* PARAMETERS:
*
* RETURNS:    return 0 if success else 1
*
*****************************************************************************/
static int vpm_process( int argc __attribute__((unused)), char **argv )
{
   int i;
   int ret = 0;
   int inValid = 0;
   unsigned int numIter = 0;
   unsigned int pollInter = 0;
   VpmStatsType stats_type = VPM_NONE_STATS;
   VpmSwitchType cmd_switch = VPM_LAST;

   while ( *argv != NULL && !inValid)
   {

      for ( i = 0; VpmMap[i].type != VPM_LAST ; i++ )
      {
         if ( strcasecmp(argv[0], VpmMap[i].name) == 0 )
         {
            cmd_switch = VpmMap[i].type;
            break;
         }
         else
         {
            cmd_switch = VPM_LAST;

         }
      }//for

      switch (cmd_switch)
      {
         case VPM_ALL:
            /* all stats */
            stats_type = VPM_ALL_STATS;
            break;

         case VPM_SPECIFIC:
            /* specific stats */
            argv++;

            if( strcasecmp (argv[0], "pcm") == 0 )
            {
               /* pcm stats */
               stats_type = VPM_PCM_STATS;
            }
            else if( strcasecmp (argv[0], "dsp") == 0 )
            {
               /* dsp stats */
               stats_type = VPM_DSP_STATS;
            }
            else if( strcasecmp (argv[0], "dect") == 0 )
            {
               /* dect stats */
               stats_type = VPM_DECT_STATS;
            }
            else
            {
               /* invalid */
               inValid = 1;
               cmd_usage();
               ret = 1;
            }
            break;

         case VPM_NUM_ITER:
            /* number of iterations */
            argv++;
            
            if( *argv == NULL )
            {
               ret = 1;
               break;
            }
            
            numIter = atoi(argv[0]);
            if (numIter <= 0)
            {
               printf("Error: invalid value for 'n' \n");
               ret = 1;
            }
            break;

         case VPM_POLL_INTER: 
            /* polling interval in msec */
            argv++;
            
            if( *argv == NULL )
            {
               ret = 1;
               break;
            }
            
            pollInter = atoi(argv[0]);
            if (pollInter <= 0)
            {
               printf("Error: invalid value for 't' \n");
               ret = 1;
            }
            break;

         default:
            cmd_usage();
            inValid = 1;
            ret = 1;
            break;


      }//switch
      argv++;

   }//while

   if(ret)
   {
      return ret;
   }
   
   ret = openFileDesc( stats_type );
   if(ret)
   {
      return ret;
   }
   
   // register signal handler 
   signalRegisterHandler( signalHandler );
   status = STATUS_RUN;

   while(status)
   {
      switch( stats_type )
      {      
         case VPM_ALL_STATS:
            vpmAllStats();
            break;

         case VPM_PCM_STATS:
            vpmPcmStats();
            break;

         case VPM_DSP_STATS:
            vpmDspStats();
            break;

         case VPM_DECT_STATS:
            vpmDectStats();
            break;

         default:
            break;
      }


      if( pollInter )
      {
         usleep( pollInter*1000 );

      }else
      {
         /* default value is 500msec */
         usleep( DEFAULT_POLLINTER );
      }

      if( numIter )
      {
         numIter--;
         if ( numIter == 0 )
         {
            status = STATUS_STOP;
         }
      }

   }// while

   return ret;
}

/*****************************************************************************
* FUNCTION:  signalHandler
*
* PURPOSE:   handle the stop signal for the application
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void signalHandler( int sig, siginfo_t* siginfo __attribute__((unused)), void* notused __attribute__((unused)))
{
   if ( sig == SIGTSTP )
   {
      /* stop the application */ 
      status = STATUS_STOP;
      /* close the file descrip */
      closeFileDesc();
   }
}

/*****************************************************************************
* FUNCTION:  openFileDesc
*
* PURPOSE:   open proc file system entry
*
* PARAMETERS:
*
* RETURNS:   return 0 if success else 0
*
*****************************************************************************/
int openFileDesc( unsigned int type )
{
   int err = 0;

   switch (type)
   {
      case VPM_ALL_STATS:
         pcmStatsFileDesc  = fopen("/proc/voice/pcm_dma_stats","rb");
         dspStatsFileDesc  = fopen("/proc/voice/dsp_task_stats","rb");
         dectStatsFileDesc = fopen("/proc/dect/dect_mac_interrupt","r");

         if (pcmStatsFileDesc == NULL || dspStatsFileDesc == NULL || dectStatsFileDesc == NULL )
         {
            err = 1;
         }
         break;

      case VPM_PCM_STATS:
         pcmStatsFileDesc  = fopen("/proc/voice/pcm_dma_stats","rb");
         
         if ( pcmStatsFileDesc == NULL )
         {
            err = 1;
         }
         break;

      case VPM_DSP_STATS:
         dspStatsFileDesc  = fopen("/proc/voice/dsp_task_stats","rb");
         
         if ( dspStatsFileDesc == NULL )
         {
            err = 1;
         }
         break;

      case VPM_DECT_STATS:
         dectStatsFileDesc = fopen("/proc/dect/dect_mac_interrupt","r");
         
         if ( dectStatsFileDesc == NULL )
         {
            err = 1;
         }
         break;

      default:
         /* unspecified stats type */
         err = 1; 
         break;
   }

   return err;
}
/*****************************************************************************
* FUNCTION:  closeFileDesc
*
* PURPOSE:    
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void closeFileDesc( )
{
   if(pcmStatsFileDesc != NULL)
   {
      /* close pcmStatsFileDesc */
      fclose(pcmStatsFileDesc);
   }
   
   if(dspStatsFileDesc != NULL)
   {
      /* close dspStatsFileDesc */
      fclose(dspStatsFileDesc);
   }
   
   if(dectStatsFileDesc != NULL)
   {
      /* close dectStatsFileDesc */
      fclose(dectStatsFileDesc);
   }
}

/*****************************************************************************
* FUNCTION:  vpmDectStats
*
* PURPOSE:    
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void vpmDectStats( )
{
   int i=0;
   char buf[500]={0x0};

   printf("###################\n");
   printf("*** DECT STATS *** \n");
   printf("###################\n");

   if ( dectStatsFileDesc )
   {
      if(! fread( buf, 500, 1, dectStatsFileDesc )) printf("fread error!\n");

      for(i=0;i<500;i++)
      {
         printf("%c", buf[i]);
      }
      rewind( dectStatsFileDesc );
   }
   printf("\n");
}

/*****************************************************************************
* FUNCTION:  vpmDspStats
*
* PURPOSE:    
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void vpmDspStats( )
{
   int i=0;
   int len=0;
   char buf[4096]={0x0};
   int size = sizeof(DSPTASKSTATS) * STATS_MAX_ENTRY;
   DSPTASKSTATS dspStats[STATS_MAX_ENTRY];
   
   printf("###################\n");
   printf("*** DSP STATS *** \n");
   printf("###################\n");

   if ( dspStatsFileDesc )
   {
      if(! fread( buf, 4096, 1, dspStatsFileDesc )) printf("fread error!\n");
      rewind( dspStatsFileDesc );
      memset( &dspStats[0], 0, size );
      memcpy( &len, buf, 4);
      memcpy( &dspStats[0], (buf+4), size );

      if( len <= STATS_MAX_ENTRY )
      {
         for(i=0;i<len;i++)
         {
            printf("DSP Task Run Cycle:%lu \t\t DSP Task Inter Interval Run Cycle:%lu \n", dspStats[i].dspTaskRunCyc, dspStats[i].dspTaskIntRunCyc);
         }
      }
   }
   printf("\n");

}
/*****************************************************************************
* FUNCTION:  vpmPcmStats
*
* PURPOSE:    
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void vpmPcmStats( )
{
   char buf[4096]={0x0};
   int size = sizeof(PCMDMASTATS);
   PCMDMASTATS pcmDmaStats;

   printf("###################\n");
   printf("*** PCM STATS *** \n");
   printf("###################\n");
   
   if ( pcmStatsFileDesc )
   {
      if (! fread( buf, 4096, 1, pcmStatsFileDesc ))  printf("fread error!\n");
      rewind( pcmStatsFileDesc );
      memset( &pcmDmaStats, 0, size );
      memcpy( &pcmDmaStats, buf, size );

      printf("Interrupt Error Count:%u\n", pcmDmaStats.interruptErrCnt);
      printf("Minimum Interrupt Interval Cycle:%u\n", pcmDmaStats.minInterruptIntCyc);
      printf("Maximum Interrupt Interval Cycle:%u\n", pcmDmaStats.maxInterruptIntCyc);
      printf("Average Interrupt Interval Cycle:%u\n", pcmDmaStats.avgInterruptIntCyc);
      printf("Minimum ISR Run Cycle:%u\n", pcmDmaStats.minIsrRunCyc);
      printf("Maximum ISR Run Cycle:%u\n", pcmDmaStats.maxIsrRunCyc);
      printf("Average ISR Run Cycle:%u\n", pcmDmaStats.avgIsrRunCyc);
   }
   printf("\n");
}

/*****************************************************************************
* FUNCTION:  vpmAllStats
*
* PURPOSE:    
*
* PARAMETERS:
*
* RETURNS:    nothing
*
*****************************************************************************/
void vpmAllStats( )
{
   vpmPcmStats();
   vpmDspStats();
   vpmDectStats();
}
