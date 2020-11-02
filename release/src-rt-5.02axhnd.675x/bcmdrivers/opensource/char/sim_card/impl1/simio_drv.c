/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

/**
*
*   @file   simio_drv.c
*
*   @brief  This include file contains the functions required to interface to the SIM.
*
*   @Reference: ISO/IEC 7816-3
*               GSM 11.11, ETSI TS 102221
*               Protocol T=0, Asynchronous, half duplex char transmission protocol(3G)
*               Protocol T=1, Asynchronous, half duplex char transmission protocol(3G)
*
****************************************************************************/
#include <bcmtypes.h>
#include <linux/kthread.h>
#include <linux/bcm_log.h>
#include "bcm_OS_Deps.h"
#include "bcm_map_part.h"
#include "chal_types.h"
#include "chal_simio.h"
#include <simio_def_common.h>
#include "simio.h"
#include "simio_board.h"
#include "simio_atr.h"
#include "sim_export_defs.h"


#if !defined (_CAPRI_)
   #define SIM2DAT_PAD SSPDO_PAD 
#endif

//#define SIMIO_ALLOW_ID_OVERRIDE

SIMIO_ID_t simio_override_id = SIMIO_ID_0;

#define SIMIO_COUNT   2

//#define SIM_DEBUG
#if 1 /*defined(BSP_PLUS_BUILD_INCLUDED) && defined(SIM_DEBUG)*/

#define DBG_LI       5
#define DBG_LW       1
#define DBG_LE       0
#else
#define dprintf(a,...)
#endif

// Limit the number of bytes printed on each SIM_LOG_ARRAY() call to ensure all data is output. 
// This is needed since SIM_LOG_ARRAY() prints up to 42 bytes of data only. 
#define MAX_NUM_OF_BYTES_PER_DEBUG_LINE	40

//******************************************************************************
//                      Feature Definition
//******************************************************************************
// Define this if SIMIO want to use DMA channel
//#define SIMIO_DMA_TX
//#define SIMIO_DMA_RX


//******************************************************************************
//                      Constant and Macro Definition
//******************************************************************************
//#define SYNC_LISR_HISR

#define _DBG_SIM_DATA_(a) a     /* Enable SIM CMD/RSP logging */
//#define _DBG_SIM_DATA_(a)     /* Disable SIM CMD/RSP logging */

//#define _DBG_2133_(a) a           /* Enable 2133 debug */
#define _DBG_2133_(a)           /* Disable 2133 debug */


#define SIMIO_ASSERT(exp)     do { if (!(exp)) { Log_DebugPrintf(LOGID_SIM, "!!!ASSERT fail: file %s lines %d", __FILE__, __LINE__); while(1); } } while (0)


#if 0
/* SIM control register */
#define SCR_REG     (* (volatile UInt32 *) SIM_BASE_ADDR)
/* SIM status register */
#define SSR_REG     (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x04))
/* SIM data register */
#define SDR_REG     (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x08))
/* SIM interrupt enable register */
#define SIER_REG    (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x0C))
/* SIM FIFO control register */
#define SFCR_REG    (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x10))
/* SIM extra character guard time register */
#define SECGTR_REG  (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x14))
/* SIM turn around guard time register */
#define STGTR_REG   (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x18))
#define SGCCR_REG   (* (volatile UInt32 *) (SIM_BASE_ADDR + 0x1C ))        /* SIM generic counter compare register */
#define SGCVR_REG   (* (volatile const UInt32 *) (SIM_BASE_ADDR + 0x20 ))  /* SIM generic counter value register */
/* SIM debug register */
#define SIMDEBUG_REG    (* (volatile UInt32 *)(SIM_BASE_ADDR + 0x30))


void SIMREGDUMP(void)
{
	SIM_LOGV(" SCR_REG=", SCR_REG );
	SIM_LOGV(" SSR_REG=", SSR_REG );
	SIM_LOGV(" SDR_REG=", SDR_REG );
	SIM_LOGV(" SIER_REG=", SIER_REG );
	SIM_LOGV(" SFCR_REG=", SFCR_REG );
	SIM_LOGV(" SECGTR_REG=", SECGTR_REG );
	SIM_LOGV(" STGTR_REG=", STGTR_REG );
	SIM_LOGV(" SGCCR_REG=", SGCCR_REG );
	SIM_LOGV(" SGCVR_REG=", SGCVR_REG );
	SIM_LOGV(" SGCCR_REG=", SIMDEBUG_REG );
}

#endif

#define MAX_SIM_FIFO_SIZE               64  // The physical FIFO size
#define DEFAULT_TXFIFO_LENGTH           48  // The driver allowed max TX size
#define DEFAULT_RXFIFO_LENGTH           32  // The driver allowed max RX size

#define T0_CMD_HEADER_LENGTH            5   // 5 bytes T=0 header

// Set transmit or receive mode
#define SET_TX_MODE(dev) {   \
						chal_simio_enable_intr((dev)->chal_handle, CHAL_SIMIO_INT_TXDONE); \
						dev->WaitForReceiving_On = FALSE; \
						}

#define SET_RX_MODE(dev) {   \
						 chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_TXDONE); \
						 dev->WaitForReceiving_Counter = 0; dev->WaitForReceiving_On = TRUE; \
						}

#define IS_RX_MODE(dev)  ( dev->WaitForReceiving_On )

#if defined(DISABLE_SIM_EMERGENCY_SHUTDOWN) || defined(_RHEA_) || defined(_CAPRI_)
  #define SIMIO_ENABLE_ESD                 0 
  #define SIMIO_ENABLE_ESD_BATTERY_REMOVAL 0 
  #define SIMIO_ENABLE_ESD_SIM_REMOVAL     0 
  #define SIMIO_ENABLE_ESD_RESET_WDOG      0 
#else 
  #define SIMIO_ENABLE_ESD                 1 
  #define SIMIO_ENABLE_ESD_BATTERY_REMOVAL 1
  #if defined(DISABLE_SIM_HOTSWAP)
    #define SIMIO_ENABLE_ESD_SIM_REMOVAL   0
  #else
    #define SIMIO_ENABLE_ESD_SIM_REMOVAL   1
  #endif 
  #define SIMIO_ENABLE_ESD_RESET_WDOG      1 
#endif 

typedef enum
{
  SIMIO_TOTYPE_PPS,
  SIMIO_TOTYPE_WWT,
  SIMIO_TOTYPE_BWT_T1,
  SIMIO_TOTYPE_MAX,
  SIMIO_TOTYPE_NONE
} eSimioTimer_Type;

typedef struct
{
  eSimioTimer_Type      timer_type;
  UInt32                etu;
} etu_timer_t;

typedef enum
{
	SIMIO_Idle,         // Idle state
	SIMIO_SimReset,     // Sim reset
	SIMIO_TimeOut,      // Sim access timeout
	SIMIO_SimInsert,    // Sim insert state
	SIMIO_RspData,      // response data
	SIMIO_T1Parity,     // T1 parity error
	SIMIO_T1InvalidLength,      // T1 parity error
	SIMIO_T1BWTTimeout,     // T1 BWT time out
	SIMIO_SimRemoved,   // SIM Removed
    SIMIO_SimDetected   // SIM Detected
} SimioSt_t;

typedef enum
{
	SIM_ATR_ST,         // handle ATR
	SIM_PB_Rx_ST,       // handle procedure byte when in receive mode
	SIM_Read_One_ST,    // handle data reading
	SIM_Read_Remain_ST, // handle data reading
	SIM_PB_Tx_ST,       // handle procedure byte when in transmit mode
	SIM_Write_ST,       // handle data writing
	SIM_Dummy_ST        // this state will do nothing
} SimDriverSt_t;


typedef enum
{
	INIT_RESET,             // set all parameters to default value
	TRY_NEXT_VOLTAGE_RESET, // use higher voltage to reset
	VOLTAGE_CHANGE_RESET    // reset for voltage change
} RESET_TYPE_t;

typedef enum
{
    SIMIO_CLK_13MHZ_TO_3P25MHZ,  // SIM interface clock 13MHz, SIM clock 3.25MHz
    SIMIO_CLK_26MHZ_TO_3P25MHZ,  // SIM interface clock 26MHz, SIM clock 3.25MHz
    SIMIO_CLK_25MHZ_TO_4P16MHZ,   // SIM interface clock 25MHz, SIM clock 4.16MHz
    SIMIO_CLK_25MHZ_TO_3P12HZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_2P5MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_2P08MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_1P78MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_1P5MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_1P3MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_1P25MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_1P13MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_25MHZ_TO_1P04MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_48MHZ_TO_4P8MHZ,   // SIM interface clock 48MHz, SIM clock 4.8MHz    
} SIMIO_CLK_DIVISOR_t;

#define SIMIO_CLK_BASIC SIMIO_CLK_25MHZ_TO_3P12HZ

// Set/disable/reset "watch for receiving" on
#define WATCH_FOR_RECEIVING_ON(dev)        {dev->WaitForReceiving_Counter = 0; dev->WaitForReceiving_On = TRUE; }
#define WATCH_FOR_RECEIVING_OFF(dev)       (dev->WaitForReceiving_On = FALSE)
#define WATCH_FOR_RECEIVING_RESET(dev)     (dev->WaitForReceiving_Counter = 0)
#define WATCH_FOR_DMA_TX_DONE()         {WATCH_FOR_RECEIVING_ON(); }
#define WATCH_FOR_DMA_RX_DONE()         {WATCH_FOR_RECEIVING_ON(); }
#define WATCH_FOR_DMA_TIMER_OFF()       {WATCH_FOR_RECEIVING_OFF();}

// T=1 Start Timer, Block Waiting Time/Character Waiting Time
#define START_TIMER_BWT(dev)               { dev->WaitForReceiving_On = FALSE; dev->WaitForBWTx_Counter=0; dev->WaitForBWTx_On = TRUE; }
#define STOP_TIMER_BWT(dev)                { dev->WaitForBWTx_On = FALSE; }
#define SIMIO_Timer_Start(dev)             {   OSTIMER_Start(dev->Timer_Simio); \
											WATCH_FOR_RECEIVING_ON(dev); }
#define SIMIO_Timer_Reset(dev)             {   OSTIMER_Reset(dev->Timer_Simio); \
											WATCH_FOR_RECEIVING_OFF(dev); \
											}

#define MAX_WAIT_FOR_RECEIVE_PROTOCOL_T0    3       // 3 sec (3 x 1s)
#define MAX_WAIT_FOR_TRANSMIT_PROTOCOL_T0   3       // 3 sec (3 x 1s)


// One China SIM takes more than 21s to return the response for Terminal Profile with NULL byte to increase the timeout. 
// We increase the timeout value to work with this SIM 
#define MAX_LONG_WAIT_FOR_RECEIVE_PROTOCOL_T0 6     // 6 sec (6 x 1s) 


#define MIN_EXTRA_GUARD_TIME            4   // Extra Inter-Character Guard Time needed to work with some SIM's that need
											// guard time larger than 2 ETU. This value is added to default value of 2 ETU

#define OSSEMAPHORE_RESULT_FAIL         0
#define OSSEMAPHORE_RESULT_SUCCESS      1


#define SEND_SIGNAL_INSERT(dev)        dev->simio_callback(SIMIO_SIGNAL_SIMINSERT, sizeof(SIMIO_USIMAP_ATR_PARAM_t), (UInt8*)&dev->ATR_ParamData);
#define SEND_SIGNAL_RSPDATA(dev)       dev->simio_callback(SIMIO_SIGNAL_RSPDATA, dev->RspLength, dev->SimBuffer);
#define SEND_SIGNAL_REMOVE(dev)        dev->simio_callback(SIMIO_SIGNAL_SIMREMOVED, NULL, NULL);
#define SEND_SIGNAL_ATR_CORRUPTED(dev) dev->simio_callback(SIMIO_SIGNAL_ATRCORRUPTED, NULL, NULL);
#define SEND_SIGNAL_ATR_WRONG_VOLTAGE(dev) dev->simio_callback(SIMIO_SIGNAL_ATR_WRONG_VOLTAGE, NULL, NULL);
#define SEND_SIGNAL_T1PARITY(dev)      dev->simio_callback(SIMIO_SIGNAL_T1_PARITY, dev->RspLength, dev->SimBuffer);
#define SEND_SIGNAL_T1_BWT_TIMEOUT(dev) dev->simio_callback(SIMIO_SIGNAL_T1_BWT_TIME_OUT, NULL, NULL);
#define SEND_SIGNAL_T1_INVALID_LENGTH(dev) dev->simio_callback(SIMIO_SIGNAL_T1_INVALID_LENGTH, NULL, NULL);


/* MobC00205851: SIM IRQ flooding on UMTS platforms
 * use a higher precision TIMER_GetAccuValue() to detect the issue
 */
/* 32 ticks of accurate timer = 1ms => 400us = 32*.4 = 13 */
#define ACCURATE_TIMER_VALUE_FOR_400US (13)
/* trigger if 6 or more interrupts in 400us (1 interrupt per ~67us) */
#define MAX_INT_ALLOWED_IN_400US 5


/* Number of times to toggle Reset line high/low to activate the SIM */
#define NUM_OF_MAX_RESET_SWITCH	3
#if defined(PROJECT_BVC_CAPRI_A01)
#define DISABLE_SIM_HOTSWAP //CSP568079,BVC not support this function 
#endif
#if defined(DISABLE_SIM_HOTSWAP)
#else
    #define SIMIO_CARD_DETECTION_ENABLED
#endif
#ifdef SIMIO_CARD_DETECTION_ENABLED
#define SIMIO_CARD_INSERTED     1
#define SIMIO_CARD_REMOVED      0
#endif

#define SIMIO_SIMLDO_SIMVCC_ENABLED

#define ISR_HIST_SIZE 256
typedef struct
{
	/* entering conditions */
	UInt32 time;
	UInt32 scr_reg;
	UInt32 sfcr_reg;
	UInt32 sier_reg;
	UInt32 ssr_reg;
	UInt32 sdebug_reg;
	SimDriverSt_t Sim_St;
	/* exiting conditions */
	UInt32 time_exit;
	UInt32 scr_reg_exit;
	UInt32 sfcr_reg_exit;
	UInt32 sier_reg_exit;
	UInt32 ssr_reg_exit;
	UInt32 sdebug_reg_exit;
	SimDriverSt_t Sim_St_exit;
	Boolean trigger_hisr_exit;
} SIMIO_ISR_DATA_t;


typedef struct
{
	SIMIO_ID_t   id;
	UInt32 base;
	InterruptId_t irq;
	CHAL_HANDLE  chal_handle;
	SIMIO_ATR_t  *atr_handle;

	UInt8        sim_sleep_id;
	UInt8        sim_pedestal_id;

	SIMIO_USIMAP_ATR_PARAM_t ATR_ParamData;
	SimDriverSt_t Sim_St;            // Sim state
	SimioSt_t     Simio_St;

	SIMIO_DET_CB_t        simio_det_callback;
	SIMIO_CB_t            simio_callback;
	SIMIO_Recov_Stat_CB_t recov_stat_cb;

	Boolean warmReset;
	
	SimVoltageLevel_t Sim_Voltage;
	PROTOCOL_t  pref_protocol;

	Task_t  Task_Simio;
	Task_t  Task_GCNTTimer;
	Interrupt_t  SimioHisr;  // SIMIO HISR handler;
	Semaphore_t  Semaphore_Simio;    // for coordinating LISR and TASK
	Semaphore_t  Semaphore_Memlock;  // for coordinating WriteCmd and interrupt
	Semaphore_t  SemaphoreGCNTTimer;
        Semaphore_t  SemaphoreUser;
	UInt32       fifo_size;

	Boolean fClockStopMode_Settle;
	Boolean Clk_Stop_Skipped;
	SIMIO_CLOCK_STOP_MODE_t ClockStopMode;
	SIMIO_CLOCK_STOP_MODE_t UICC_ClockStopMode;

	UInt8        Cmd_Instruction;    // Instruction sent to interface.

	// Buffer used by send and receive (261 is maximum number used by SIMIO_WriteCmd)
	UInt8        SimBuffer[SIMIO_SIM_BUFFER_SIZE];
	UInt16       RxTxIndex;              // point to next byte tobe sent
	UInt16       TxLength;               // total chars to be sent
	UInt16       TxFifo_lenth;           // Tx FIFO Minimum length

	UInt8        RxBuffer[SIMIO_RCVD_BUFFER_SIZE]; // receiving stream buffer
	UInt16       RxChar_Head_Index;  // receiving stream buffer head
	UInt16       RxChar_Tail_Index;  // receiving stream buffer tail
	UInt16       RxBuffer_Count;     // Number of bytes received in Rx buffer
	UInt16       RspLength;          // Expected characters to receive in response to a command.
	UInt16       RxCount;            // data counter for receiving
	UInt8        ATRRxBuffer[SIMIO_RCVD_BUFFER_SIZE];
	UInt16       ATRRxCount;
	UInt16       ATRRxSuccess;
        int          RXExpectedByteCount;

	Boolean      PPS_Time_Out;
	Boolean      ATR_Time_Out;
	Boolean      Parity_Error;

	/* TRUE if any bytes in the response for USIM command using T=1 protocol
	 * contains parity error. If this happens, we need to send a R-block
	 * indicating such a condition.
	 */
	Boolean      T1ParityError;
	Boolean      T1_Length_Invalid;
	Boolean      GCNTI_timer_triggered;

	UInt8        Max_WaitFor_Receive;    // Use either in T=0 or T=1
	UInt8        Max_WaitFor_Transmit;   // Use either in T=0 or T=1

	UInt32       T1SIMIO_BWT;            // T=1 Parameter, Block Waiting Time in fine etu's
	UInt8        T1Param_BWT;            // T=1 Parameter, Block Waiting Time in rough seconds
	UInt8        T1Param_CWT;            // T=1 Parameter, Character Waiting Time
	//maximum interval between the start leading edge of any character, Work Waiting Time, see 131 101
	UInt32      SIMIO_WWT;


	Boolean      RemovedSignalSent;      // simio task need to know this before send removed signal
	Boolean      Reconfig_Done;          // TRUE: reconfig done, FALSE:not( for h/w reconfig )
	Boolean      Select_Done;            // TRUE: SELECT cmd done, FALSE:not

	// for monitoring SIM access
	etu_timer_t  etu_timer;
	Timer_t      Timer_Simio;                    // for monitor sim access
	Timer_t      Timer_User;
	UInt8        WaitForReceiving_Counter;
	UInt8        WaitForBWTx_Counter;
	Boolean      WaitForReceiving_On;
	Boolean      WaitForBWTx_On;
	Boolean      T1BWT_On;

	// statistics
	UInt32       ParityErrorCount;

	UInt32		 int_time; // Time when SIM IRQ is triggered
	UInt32		 int_count; // Number of SIM IRQ's triggered 

	UInt32		 cmd_rcvd_time;	// Time when a command is to be sent to SIM

    CLIENT_ID    prm_sim_id;

	Boolean      esd_det_triggered;
	
	// Workaround of missing TXDONE issue for T=0
	Boolean      expecting_txdone;

	// For debugging purpose, do not remove. 
	UInt32 spurious_int_count;
	UInt32 irq_flood_count;
#ifdef SIMIO_CARD_DETECTION_ENABLED
	UInt32 saved_sier_reg;
#endif

	SIMIO_ISR_DATA_t isr_hist[ISR_HIST_SIZE];
	UInt16 isr_hist_cnt; // also used as the ptr
} SIMIO_t;

//******************************************************************************
// Local Function Prototypes
//******************************************************************************
static sim_card_info sim_card_data;
static void sim_card_tasklet_handler(unsigned long sim_card_data);
static DECLARE_TASKLET(sim_card_tasklet_enable, sim_card_tasklet_handler, (unsigned long)&sim_card_data);
static void sim_card_tasklet_det_handler(unsigned long sim_card_data);
static DECLARE_TASKLET(sim_card_tasklet_det_enable, sim_card_tasklet_det_handler, (unsigned long)&sim_card_data);

// Wappers for SIMIO_ID_0
static int     task_SIMIO_entry_wrapper0( void *);
static int     SIMIO_GCNTTimer_Entry_wrapper0( void * );
static FN_HANDLER SIMIO_ISR_wrapper0(int irq, void *sim_card_prm);
static void     SIMIO_HISR_wrapper0( TimerID_t);
static void     SIMIO_TimeOutTimer_Entry_wrapper0( void );

// Wappers for SIMIO_ID_1
static int      task_SIMIO_entry_wrapper1(void *);
static int      SIMIO_GCNTTimer_Entry_wrapper1(void * );
static FN_HANDLER SIMIO_ISR_wrapper1(int irq, void *sim_card_prm);
static void     SIMIO_HISR_wrapper1( TimerID_t );
static void     SIMIO_TimeOutTimer_Entry_wrapper1( void );

// Real re-entrant functions
static void     task_SIMIO_entry(SIMIO_t* dev);
static void     SIMIO_GCNTTimer_Entry(SIMIO_t* dev);
static void     SIMIO_ISR(SIMIO_t* dev);
static void     SIMIO_HISR(SIMIO_t* dev);
static void     SIMIO_TimeOutTimer_Entry(SIMIO_t* dev);

static UInt16 SIMIO_GetSimData(SIMIO_t* dev, UInt8 *buffer, UInt16 buffer_size);
static void SIMIO_PutSimChar(SIMIO_t* dev, UInt8 *data, UInt16 data_len);
static void SIMIO_PurgeSimChar(SIMIO_t* dev);


static void     SIMIO_ProcessATR(SIMIO_t* dev, UInt8 rcvd_char, Boolean reset_recv);
static void     SIMIO_Read(SIMIO_t* dev, UInt8 rcvd_char, UInt8 remaining_num_of_char);
static void     SIMIO_Write(SIMIO_t* dev, UInt8 rcvd_char);

static void SIMIO_Active_SIM(SIMIO_t* dev);
static void SIMIO_Reset(SIMIO_t* dev, RESET_TYPE_t reset_type);

static void SIMIO_SendSimRemovalStatus(SIMIO_t* dev);

static void SIMIO_TimerStart(SIMIO_t* dev, UInt8 timer_type, UInt16 timer_val);
static void SIMIO_TimerStop(SIMIO_t* dev);
static void _SIMIO_DeactiveCard(SIMIO_ID_t id);
static void _SIMIO_ShutdownVcc(SIMIO_ID_t id);
static void _SIMIO_Delay(SIMIO_ID_t id, UInt8 etu);

static Boolean SIMIO_IsNullTimeExc(SIMIO_t* dev);

static Boolean SIMIO_Set_Clock(
    SIMIO_t* dev, 
    Boolean clk_on, 
    SIMIO_CLK_DIVISOR_t clk_divisor
);

#if defined(SIMIO_DMA_TX) || defined(SIMIO_DMA_RX)
static Boolean SIMIO_Dma_Mem2sim(UInt8 *src, UInt16 *dst, UInt16 size);
static Boolean SIMIO_Dma_Sim2mem(UInt16 *src, UInt8 *dst, UInt16 size);
static Boolean SIMIO_Dma_Start(UInt8 *mem_addr, UInt16 size, UInt8 direction, UInt8 dma_position, SimDriverSt_t next_sim_st, UInt16 next_rxtxindex);
static void SIMIO_Dma_NotifyCb(DMADRV_ERROR_t Err, DMADRV_SERVICE_TYPE_t Type);
static void SIMIO_Dma_End(UInt8 reason);
#endif


void SIMIO_WarmReset(void);
void SIMREGDUMP(void);

#ifdef SIMIO_CARD_DETECTION_ENABLED
#if 0 /*amir - this func is not used */
static void simio_enable_int(SIMIO_t* dev);
#endif
static void simio_disable_int(SIMIO_t* dev);
#endif


//******************************************************************************
//      Buffers and Variable
//******************************************************************************

static SIMIO_t simio_dev[SIMIO_COUNT];

const static struct
{
	InterruptId_t  irq;
	TEntry_t       task;
	TEntry_t       gcnt_task;
	isr_t          isr;  //&
	IEntry_t       hisr;  //&
	TimerEntry_t   os_timer_entry;
} simio_ro[SIMIO_COUNT] =
{
	{
		SIM_IRQ,  // IRQ4
		task_SIMIO_entry_wrapper0,
		SIMIO_GCNTTimer_Entry_wrapper0,
		SIMIO_ISR_wrapper0,
		SIMIO_HISR_wrapper0,
		(TimerEntry_t)SIMIO_TimeOutTimer_Entry_wrapper0
	}
	,
	{
        SIM2_IRQ,
		task_SIMIO_entry_wrapper1,
		SIMIO_GCNTTimer_Entry_wrapper1,
		SIMIO_ISR_wrapper1,
		SIMIO_HISR_wrapper1,
		(TimerEntry_t)SIMIO_TimeOutTimer_Entry_wrapper1
	}
};


#if defined(SIMIO_DMA_TX) || defined(SIMIO_DMA_RX)

/* DMA for SIMIO */
#define DMA_TRIGGER_1           1
#define DMA_TRIGGER_2           2
#define DMA_TRIGGER_3           3

typedef struct {
	UInt8       *mem_addr;
	UInt8       direction;
	UInt16      size;
	UInt8       position;
	UInt8       old_sim_st;
	UInt8       old_rxtxindex;
	UInt8       new_sim_st;
	UInt16      new_rxtxindex;
} DMA_START_INFO_t;

static DMA_START_INFO_t dma;

/* direction */
#define MEM2SIM_DIRECTION       0
#define SIM2MEM_DIRECTION       1

/* argument of SIMIO_Dma_End() */
#define DMA_END_NORMAL          0
#define DMA_END_NOTIFY_ERROR    1
#define DMA_END_TIMEOUT         2
#define DMA_END_STARTFAIL       3
#endif


static const UInt8 BWT_T1_Table[10] =   // second (f=3.25M, 372/1) 7816-3 9.5.3.2
{
	2,          // BWI=00
	2,          // 01
	2,          // 02
	2,          // 03
	2,          // 04=default
	4,          // 05
	8,          // 06
	16,         // 07
	32,         // 08
	64          // 09
};

static const UInt8 CWT_T1_Table[16] =   // second (f=3.25M, 372/1) 7816-3 9.5.3.1
{
	1,          // CWI=00
	1,          // 01
	1,          // 02
	1,          // 03
	1,          // 04
	1,          // 05
	1,          // 06
	1,          // 07
	1,          // 08
	1,          // 09
	1,          // 10
	1,          // 11
	1,          // 12
	1,          // 13=default
	2,          // 14
	4           // 15
};

/*****************************************************************************************************************
 * For debugging SIM loss problem.
 ****************************************************************************************************************/
//#define SIM_LOSS_DEBUG

#ifdef SIM_LOSS_DEBUG

#define _SIM_LOSS_DBG_(a) a

typedef struct
{
	UInt32  time;
	UInt16  byte;
	UInt8   sim_state;
	UInt8   action;
} SIMIO_Log_Data_t;

#define SIMIO_LOG_SIZE 300

static SIMIO_Log_Data_t Simio_Log_Data[SIMIO_LOG_SIZE];

static UInt16 Simio_Log_Index = 0;

static UInt32 Simio_Init_Time;

static void SIMIO_PrintSioLog(void)
{
	UInt16 i;
	SIMIO_Log_Data_t *log_data;

	if( (TIMER_GetValue() - Simio_Init_Time) > 3000 )
	{
		for(i = 0, log_data = Simio_Log_Data; i < SIMIO_LOG_SIZE; i++, log_data++)
		{
			SIM_LOGV4("SIMIO:", log_data->time, log_data->byte, log_data->sim_state, log_data->action);
			OSTASK_Sleep(10);
		}
	}
}

static void SIMIO_Log_Event(UInt16 word, UInt8 sim_state, UInt8 action)
{
	Simio_Log_Data[Simio_Log_Index].time = TIMER_GetValue();
	Simio_Log_Data[Simio_Log_Index].byte = word;
	Simio_Log_Data[Simio_Log_Index].sim_state = sim_state;
	Simio_Log_Data[Simio_Log_Index].action = action;

	Simio_Log_Index++;

	/* Wrap around */
	if(Simio_Log_Index >= SIMIO_LOG_SIZE)
	{
		Simio_Log_Index = 0;
	}
}

#else

#define _SIM_LOSS_DBG_(a)

#endif
#if 0 /*amir - this func is not used */
static void dump_isr_hist(SIMIO_t* dev)
{
	UInt32 i;
	SIMIO_ISR_DATA_t *isr_data;

	SIM_LOGV2("SIMIO: id|isr_hist_cnt", dev->id, dev->isr_hist_cnt);

	for (i = 0; i < ISR_HIST_SIZE; i++)
	{
		isr_data = &dev->isr_hist[i];

		SIM_LOGV6("SIMIO: cnt|time_enter|time_exit|st_enter|st_exit|hisr", 
			i, isr_data->time, isr_data->time_exit, isr_data->Sim_St,
			isr_data->Sim_St_exit, isr_data->trigger_hisr_exit);

		SIM_LOGV5("SIMIO: enter scr|sfcr|sier|ssr|debug",
			isr_data->scr_reg, isr_data->sfcr_reg, isr_data->sier_reg,
			isr_data->ssr_reg, isr_data->sdebug_reg);

		SIM_LOGV5("SIMIO: exit scr|sfcr|sier|ssr|debug",
			isr_data->scr_reg_exit, isr_data->sfcr_reg_exit, isr_data->sier_reg_exit,
			isr_data->ssr_reg_exit, isr_data->sdebug_reg_exit);
	}
}
#endif

static int task_SIMIO_entry_wrapper0( void * dev)
{
	dprintf(DBG_LI, "SIMIO Driver: task_SIMIO_entry_wrapper0\n");
	task_SIMIO_entry(&simio_dev[SIMIO_ID_0]);
        return 0;
}

static int SIMIO_GCNTTimer_Entry_wrapper0(void *dev)
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_GCNTTimer_Entry_wrapper0\n");
        SIMIO_GCNTTimer_Entry(&simio_dev[SIMIO_ID_0]);
        return 0;
}

static FN_HANDLER SIMIO_ISR_wrapper0(int irq, void *sim_card_prm)
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_ISR_wrapper0\n");
        tasklet_hi_schedule(&sim_card_tasklet_enable);
        return BCM_IRQ_NONE;
}

static void SIMIO_HISR_wrapper0( TimerID_t a )
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_HISR_wrapper0\n");
	SIMIO_HISR(&simio_dev[SIMIO_ID_0]);
}

static void SIMIO_TimeOutTimer_Entry_User( void )
{
    SIMIO_t*     dev = &simio_dev[SIMIO_ID_0];
    dprintf(DBG_LI, "SIMIO_TimeOutTimer_Entry_User\n");
    OSSEMAPHORE_Release(dev->SemaphoreUser);
}

static void SIMIO_TimeOutTimer_Entry_wrapper0( void )
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_TimeOutTimer_Entry_wrapper0\n");
	SIMIO_TimeOutTimer_Entry(&simio_dev[SIMIO_ID_0]);
}

static int task_SIMIO_entry_wrapper1(void *dev)
{
	dprintf(DBG_LI, "SIMIO Driver: task_SIMIO_entry_wrapper1\n");
	task_SIMIO_entry(&simio_dev[SIMIO_ID_1]);
        return 0;
}

static int SIMIO_GCNTTimer_Entry_wrapper1(void *dev)
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_GCNTTimer_Entry_wrapper1\n");
	SIMIO_GCNTTimer_Entry(&simio_dev[SIMIO_ID_1]);
        return 0;
}

static FN_HANDLER SIMIO_ISR_wrapper1(int irq, void *sim_card_prm)
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_ISR_wrapper1\n");
	SIMIO_ISR(&simio_dev[SIMIO_ID_1]);
        return BCM_IRQ_NONE;
}

static void SIMIO_HISR_wrapper1( TimerID_t b )
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_HISR_wrapper1\n");
	SIMIO_HISR(&simio_dev[SIMIO_ID_1]);
}

static void SIMIO_TimeOutTimer_Entry_wrapper1( void )
{
	dprintf(DBG_LI, "SIMIO Driver: SIMIO_TimeOutTimer_Entry_wrapper1\n");
	SIMIO_TimeOutTimer_Entry(&simio_dev[SIMIO_ID_1]);
}


//******************************************************************************
//
// Function Name: SIMIO_Set_Clock
//
// Description: This function set SIM clock
//              returns TRUE if the clock is set or FALSE otherwise.
//
// Notes: 
//
//******************************************************************************
static Boolean SIMIO_Set_Clock(
    SIMIO_t* dev, 
    Boolean clk_on, 
    SIMIO_CLK_DIVISOR_t clk_divisor
)
{
    UInt32 resource_id;
    Boolean clock_set = FALSE;

    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Set_Clock\n");

    SIM_LOGV2("SIMIO: SIMIO_Set_Clock: dev->id: , clk_on: ", dev->id, clk_on);

/* The following lines were added for the bring up to set the Policy Masks for SIM1 and SIM2.
   Then, they were removed as a part of code cleanup (CL#430685).
   This Policy mask setting was moved to the initialization for the SIM detection problem in the calibration mode
   while keeping the lines commented (MobC00241526).
    (*(volatile UInt32 *)0x35002000) = 0xA5A501;
    (*(volatile UInt32 *)0x3500201c) |= (3 << 19);
    (*(volatile UInt32 *)0x35002034) = 0x01;
    (*(volatile UInt32 *)0x3500200c) = 0x05;
*/
    if (dev->id == SIMIO_ID_0)
    {
        resource_id =  RESOURCE_SIM;                                  	
    }
    else
    {
        resource_id =  RESOURCE_SIM2;  	
    } 	

    if(clk_on)
    {
#if defined(FUSE_COMMS_PROCESSOR) && !defined(FPGA_VERSION)

         PRM_set_clock_state(dev->prm_sim_id, resource_id, CLOCK_ON);                                    	

#endif
        switch(clk_divisor)
        {
            case SIMIO_CLK_13MHZ_TO_3P25MHZ:
            // 13MHz/(2*(1+1)) = 3.25MHz
            // select 13MHz SIM clock
            // select crystal_clk and sim_pre_div = 1	    

#if defined(FUSE_COMMS_PROCESSOR) && !defined(FPGA_VERSION)

                PRM_set_clock_divider(dev->prm_sim_id, resource_id, 0, 1, 0, 0);               
#endif
                chal_simio_set_divisor(dev->chal_handle, TRUE, 1);
                clock_set = TRUE;
                break;        
            case SIMIO_CLK_26MHZ_TO_3P25MHZ:
            // 26MHz/(2*(3+1)) = 3.25MHz
            // select 26MHz SIM clock
#if defined(FUSE_COMMS_PROCESSOR) && !defined(FPGA_VERSION)

                PRM_set_clock_divider(dev->prm_sim_id, resource_id, 0, 0, 0, 0);           
#endif
                chal_simio_set_divisor(dev->chal_handle, TRUE, 3);
                clock_set = TRUE;
                break;        
            case SIMIO_CLK_25MHZ_TO_4P16MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 2);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_3P12HZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 3);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_2P5MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 4);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_2P08MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 5);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_1P78MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 6);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_1P5MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 7);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_1P3MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 8);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_1P25MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 9);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_1P13MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 10);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_25MHZ_TO_1P04MHZ:
                chal_simio_set_divisor(dev->chal_handle, TRUE, 11);
                clock_set = TRUE;
                break;
            case SIMIO_CLK_48MHZ_TO_4P8MHZ:
            // 48MHz/(2*(4+1)) = 4.8MHz
            // select 48MHz SIM clock
	   // select ref_96m_clk and sim_pre_div = 1
#if defined(FUSE_COMMS_PROCESSOR) && !defined(FPGA_VERSION)

                PRM_set_clock_divider(dev->prm_sim_id, resource_id, 3, 1, 0, 0);           
   
#endif
                // divisor = 4 and on
                chal_simio_set_divisor(dev->chal_handle, TRUE, 4);
                clock_set = TRUE;
                break;        
            default:
                clock_set = FALSE;
                break;
        }         
    }
    else
    {
#if defined(FUSE_COMMS_PROCESSOR) && !defined(FPGA_VERSION)
        if (dev->id == SIMIO_ID_0)
        {
            //PRM_set_clock_state(dev->prm_sim_id, RESOURCE_SIM, CLOCK_OFF);                                    	
        }
        else
        {
            //PRM_set_clock_state(dev->prm_sim_id, RESOURCE_SIM2, CLOCK_OFF);   	
        }
#endif  
        clock_set = TRUE;
    }
   
    return clock_set;
}

int SIMIO_set_frequency(SIMIO_ID_t id, SIMIO_DIVISOR_t freq)
{
    SIMIO_t*     dev = &simio_dev[id];
    SIMIO_CLK_DIVISOR_t divisor;

    switch(freq)
    {
    case SIMIO_CLK_4P16MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_4P16MHZ;
        break;
    case SIMIO_CLK_3P12HZ:
        divisor = SIMIO_CLK_25MHZ_TO_3P12HZ;
        break;
    case SIMIO_CLK_2P5MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_2P5MHZ;
        break;
    case SIMIO_CLK_2P08MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_2P08MHZ;
        break;
    case SIMIO_CLK_1P78MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_1P78MHZ;
        break;
    case SIMIO_CLK_1P5MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_1P5MHZ;
        break;
    case SIMIO_CLK_1P3MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_1P3MHZ;
        break;
    case SIMIO_CLK_1P25MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_1P25MHZ;
        break;
    case SIMIO_CLK_1P13MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_1P13MHZ;
        break;
    case SIMIO_CLK_1P04MHZ:
        divisor = SIMIO_CLK_25MHZ_TO_1P04MHZ;
        break;
    default:
        printk (KERN_EMERG "smart card err: wrong value %s:%d\n",__FILE__, __LINE__ );
        divisor = SIMIO_CLK_25MHZ_TO_3P12HZ;\
        break;
    }
    return SIMIO_Set_Clock(dev, TRUE, divisor);
}

int SIMIO_set_pps(SIMIO_ID_t id, CHAL_SIMIO_SPEED_t val, int divisor)
{
    SIMIO_t*     dev = &simio_dev[id];
    switch(val)
    {
    case CHAL_SIMIO_SPD_372_1 :
        memcpy(dev->SimBuffer, Pts_Req_F372_D1, sizeof(Pts_Req_F372_D1));
        dev->TxLength = sizeof(Pts_Req_F372_D1);
        break;
    case CHAL_SIMIO_SPD_512_8 :
        memcpy(dev->SimBuffer, Pts_Req_F512_D8, sizeof(Pts_Req_F512_D8));
        dev->TxLength = sizeof(Pts_Req_F512_D8);
        break;
    case CHAL_SIMIO_SPD_512_16 :
        memcpy(dev->SimBuffer, Pts_Req_F512_D16, sizeof(Pts_Req_F512_D16));
        dev->TxLength = sizeof(Pts_Req_F512_D16);
        break;
    case CHAL_SIMIO_SPD_512_32 :
        memcpy(dev->SimBuffer, Pts_Req_F512_D32, sizeof(Pts_Req_F512_D32));
        dev->TxLength = sizeof(Pts_Req_F512_D32);
        break;
    default:
        printk (KERN_EMERG "smart card err: wrong value %s:%d\n",__FILE__, __LINE__ );
        return SIM_CARD_INPUT_IS_INVALID;
    }

    memset(dev->ATRRxBuffer, 0, sizeof(dev->ATRRxBuffer));
    dev->ATRRxCount = 0;
    dev->RXExpectedByteCount = 0;
    
    dev->RxTxIndex += chal_simio_write_data(dev->chal_handle, dev->SimBuffer, dev->TxLength);
    
    OSTIMER_Start(dev->Timer_User);

    OSSEMAPHORE_Obtain(dev->SemaphoreUser,TICKS_FOREVER);

    OSTIMER_Stop(dev->Timer_User);

    if (memcmp(dev->SimBuffer, dev->ATRRxBuffer, 4))            
    {
        printk (KERN_EMERG "smart card err: NO ATR FEEDBACK!!! %s:%d\n",__FILE__, __LINE__ );            
        return SIM_CARD_UNSUCCESSFUL_PPS_EXCHANGE;
    }
    return 0;
}
#ifdef SIMIO_CARD_DETECTION_ENABLED
//******************************************************************************
//
// Function Name: simio_enable_int
//
// Description: This function 
//
// Notes: 
//
//******************************************************************************
#if 0 /*amir - this func is not used */
static void simio_enable_int(SIMIO_t* dev)
{
    //SIM_LOGV("SIMIO: simio_enable_int", dev->saved_sier_reg);

    chal_simio_enable_intr(dev->chal_handle, dev->saved_sier_reg);
}
#endif

//******************************************************************************
//
// Function Name: simio_disable_int
//
// Description: This function 
//
// Notes: 
//
//******************************************************************************
static void simio_disable_int(SIMIO_t* dev)
{
    chal_simio_read_reg(dev->chal_handle, NULL, &dev->saved_sier_reg, NULL, NULL, NULL);

    //SIM_LOGV("SIMIO: simio_disable_int", dev->saved_sier_reg);

    chal_simio_disable_intr(dev->chal_handle, dev->saved_sier_reg);
}
#endif


#ifdef SIMIO_SIMLDO_SIMVCC_ENABLED
//******************************************************************************
//
// Function Name: simio_simldo_simvcc_on
//
// Description: this function controls SIMVCC by SIMLDO
//
// Notes: 
//
//******************************************************************************
static void simio_simldo_simvcc_on(SIMIO_t* dev, SimVoltageLevel_t sim_voltage)
{
    UInt32 pad;
    SIM_LOGV("SIMIO: simio_simldo_simvcc_on: sel voltage: ", sim_voltage);
    if (dev->id == SIMIO_ID_0)
    {
        pad = SIMDAT_PAD;
    }
    else /*if (dev->id == SIMIO_ID_1)*/
    {
        pad = SIM2DAT_PAD;
    }
    switch(sim_voltage)
    {
        case SIM_3V:
            chal_simio_simvcc_sel(dev->chal_handle, TRUE);
	    PMUXDRV_Config_Pad_Resistor(pad, PADCTRL_SIMDAT_PULLUP_7p5kOHM);
            break;
        case SIM_1P8V:
            chal_simio_simvcc_sel(dev->chal_handle, FALSE);
	    PMUXDRV_Config_Pad_Resistor(pad, PADCTRL_SIMDAT_PULLUP_4p5kOHM);
            break;
        case SIM_5V:
        default:
            SIM_LOGV("SIMIO: simio_simldo_simvcc_on: voltage not supported", sim_voltage);
            break;
    }

    SIM_LOG("SIMIO: simio_simldo_simvcc_on: turn on ldo");

    chal_simio_simldo_simvcc(dev->chal_handle, TRUE);

    SIM_LOG("SIMIO: simio_simldo_simvcc_on: end");
}
#endif



//******************************************************************************
//      Functions
//******************************************************************************

void SIMIO_SetLDO_CB(SIMIO_LDO_CB_t cb)
{
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_SetLDO_CB\n");
    SIMIO_Board_SetLDO_CB(cb);    
}

FN_HANDLER usim_det_isr_handle(int irq, void *sim_card_prm)
{
    tasklet_hi_schedule(&sim_card_tasklet_det_enable);

    return BCM_IRQ_NONE;
}

//******************************************************************************
//
// Function Name: SIMIO_Init
//
// Description: This function initializes the SIMIO component.
//
// Notes:
//
//******************************************************************************
Boolean SIMIO_Init(SIMIO_ID_t id, int vcc_polarity)
{
	SIMIO_t*     dev = &simio_dev[id];
        
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Init\n");
    SIM_LOGV("SIMIO: SIMIO_Init: id: ", id);

#ifdef SIMIO_ALLOW_ID_OVERRIDE
	_DBG_2133_( SIM_LOGV("SIMIO: SIMIO_Init:: ", id) );
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	_DBG_2133_( SIM_LOGV("SIMIO: SIMIO_Init: ", id) );

	if (dev->Task_Simio)
	{
		_DBG_2133_( SIM_LOG("SIMIO: SIMIO_Init: Busy!") );
		return FALSE;
	}

	// init vars
	dev->id = id;
    if (id == SIMIO_ID_0)
	    dev->base = OSDAL_SYSMAP_Get_BaseAddr(OSDAL_SYSMAP_SIM);
	else
	    dev->base = OSDAL_SYSMAP_Get_BaseAddr(OSDAL_SYSMAP_SIM2);	
	dev->pref_protocol = SIM_PROTOCOL_T1;
	dev->Clk_Stop_Skipped = FALSE;
	dev->ClockStopMode = SIMIO_CLOCK_STOP_NOT_ALLOWED;
	dev->UICC_ClockStopMode = SIMIO_CLOCK_STOP_NOT_ALLOWED;
	dev->TxFifo_lenth = DEFAULT_TXFIFO_LENGTH;

	dev->chal_handle = chal_simio_init(dev->base);
	
	if (!dev->chal_handle)
	{
		_DBG_2133_( SIM_LOG("SIMIO: SIMIO_Init: can't init chal!") );
		assert(0);
	}

	dev->fifo_size = chal_simio_get_fifo_size(dev->chal_handle);

	dev->atr_handle = SIMIO_ATR_Init(id);
	
	if (!dev->atr_handle)
	{
		_DBG_2133_( SIM_LOG("SIMIO: SIMIO_Init: can't init atr!") );
		assert(0);
	}

	dev->sim_sleep_id = SLEEP_AllocId();

	dev->sim_pedestal_id = PEDESTAL_AllocId();

	// create semaphore
	dev->Semaphore_Simio = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	OSSEMAPHORE_ChangeName(dev->Semaphore_Simio, "SimIO_S");
	dev->Semaphore_Memlock = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	OSSEMAPHORE_ChangeName(dev->Semaphore_Memlock, "SimMem_S");

	// release semaphore
	OSSEMAPHORE_Release(dev->Semaphore_Memlock);

	// create semaphore for 16 bit Timer
	dev->SemaphoreGCNTTimer = OSSEMAPHORE_Create( 0, OSSUSPEND_PRIORITY );
	OSSEMAPHORE_ChangeName(dev->SemaphoreGCNTTimer, "SimGCNTS");

        dev->SemaphoreUser = OSSEMAPHORE_Create(0, 0);
	// create HISR
	dev->SimioHisr = OSINTERRUPT_Create( (IEntry_t) simio_ro[id].hisr, HISRNAME_SIMIO,
									IPRIORITY_MIDDLE, HISRSTACKSIZE_SIMIO );

	// create timer for monitoring sim access
	dev->Timer_Simio = OSTIMER_Create(simio_ro[id].os_timer_entry,
		//(TimerEntry_t) SIMIO_TimeOutTimer_Entry, // timer task function entry point
		0,                              // timer task ID (optional)
		(Ticks_t)(TICKS_ONE_SECOND),    // timer task, first timeout
		(Ticks_t)(TICKS_ONE_SECOND)     // timer task interval
	);

        dev->Timer_User = OSTIMER_Create((TimerEntry_t)SIMIO_TimeOutTimer_Entry_User,
            0, (Ticks_t)(TICKS_ONE_SECOND*5), (Ticks_t)(TICKS_ONE_SECOND));

	// create GCNT timer for monitoring sim using generic 16 bit timer in term of ETU , not OS timer in term of Ticks(second).
	dev->Task_GCNTTimer = OSTASK_Create( simio_ro[id].gcnt_task/*SIMIO_GCNTTimer_Entry*/, (TName_t)"SIMTM",
			TASKPRI_SIMIO, STACKSIZE_SIMIO );

	// register SIM IRQ
	dev->irq = simio_ro[id].irq;
	IRQ_Register(dev->irq, simio_ro[id].isr);
	dev->isr_hist_cnt = 0;

        chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_OUT_ISR );
	chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_IN_ISR );
        
        chal_simio_set_detection(dev->chal_handle, TRUE, CHAL_SIMIO_DEBOUNCE_TIME_32, CHAL_SIMIO_DEBOUNCE_MODE_PULSEEXT, CHAL_SIMIO_PRESENCE_LOW/*CHAL_SIMIO_PRESENCE_HIGH*/);
	chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_OUT_IER | CHAL_SIMIO_INT_CARD_IN_IER);

        BcmHalMapInterrupt((FN_HANDLER)usim_det_isr_handle, (void*)NULL, INTERRUPT_ID_USIM_ESD);
        BcmHalInterruptEnable(INTERRUPT_ID_USIM_ESD);

	dev->etu_timer.timer_type = SIMIO_TOTYPE_NONE;

	dev->int_time = 0;
	dev->int_count = 0;

#if defined(FUSE_COMMS_PROCESSOR) && !defined(FPGA_VERSION)
    if (dev->id == SIMIO_ID_0)
    {
        dev->prm_sim_id = PRM_client_register("SIMIO DRIVER");        	
    }
    else
    {
        dev->prm_sim_id = PRM_client_register("SIMIO2 DRIVER");        	
    }
#endif
    SIMIO_Set_Clock(dev, FALSE, SIMIO_CLK_BASIC); 

	SIMIO_Board_Init(id);

	// Enable SIM emergency shutdown as early as possible
#if 0 /*defined(SIMIO_CARD_DETECTION_ENABLED)*/
    // enable card detection interrupt
	chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_OUT_ISR | CHAL_SIMIO_INT_CARD_IN_ISR);
	chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_OUT_IER | CHAL_SIMIO_INT_CARD_IN_IER);
#endif

    chal_simio_enable_eshutdown(dev->chal_handle,
                                SIMIO_ENABLE_ESD,
                                SIMIO_ENABLE_ESD_BATTERY_REMOVAL,
                                SIMIO_ENABLE_ESD_SIM_REMOVAL,
                                SIMIO_ENABLE_ESD_RESET_WDOG);

    /* polarity of the vcc should be active high - otherwise, some sim cards may short circuit the vcc and the gnd */
    chal_simio_usim_control_register(TRUE, vcc_polarity, TRUE);

#if 0   // Need PMU driver to control STAT1 pad to avoid power-on and power-off glitches
#ifdef SIMIO_SIMLDO_SIMVCC_ENABLED
    // configure SIMLDO
    if (id == SIMIO_ID_0)
    {
        PMUXDRV_Config_Pad_Ex(STAT1_PAD, PAD_ALT1, PAD_DRV_STRENGTH_08MA, PAD_PULL_UP);
    }
    else
    {
        PMUXDRV_Config_Pad_Ex(GPS_CALREQ_PAD, PAD_ALT0, PAD_DRV_STRENGTH_08MA, PAD_PULL_UP);
    }
#endif
#endif

    dev->Task_Simio = OSTASK_Create(simio_ro[id].task, TASKNAME_SIMIO, TASKPRI_SIMIO, STACKSIZE_SIMIO);

#ifdef SIMIO_CARD_DETECTION_ENABLED
	chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_ALL);
	IRQ_Clear(dev->irq);
	IRQ_Enable(dev->irq);
#endif

	_DBG_2133_( SIM_LOG("SIMIO: SIMIO_Init end") );
	return TRUE;
}


//******************************************************************************
//
// Function Name: SIMIO_Run
//
// Description: This function will start run simio task
//
// Notes:
//
//******************************************************************************
Boolean SIMIO_Run(SIMIO_ID_t id)
{
#if 0
	SIMIO_t*     dev = &simio_dev[id];
	Boolean ret = FALSE;

	// Create and Run SIMIO Task
	if (dev->Task_Simio == NULL)
	{
		dev->Task_Simio = OSTASK_Create(simio_ro[id].task, TASKNAME_SIMIO, TASKPRI_SIMIO, STACKSIZE_SIMIO);
		ret = TRUE;
	}

	_DBG_2133_( SIM_LOG("SIMIO: SIMIO_Run end") );
	return ret;
#else
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Run\n");
    return TRUE;
#endif
}


//******************************************************************************
//
// Function Name: SIMIO_Shutdown
//
// Description: This function will destroy all the resoure created by SIMIO_Init/Run
//
// Notes:
//
//******************************************************************************
Boolean SIMIO_Shutdown(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Shutdown\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	_SIMIO_DeactiveCard(id);
    _SIMIO_ShutdownVcc(id);

	if (dev->Task_Simio == NULL)
	{
		return FALSE;
	}

	// destroy SIMIO tasks
	OSTASK_Suspend(dev->Task_Simio);
	OSTASK_Destroy(dev->Task_Simio);
	dev->Task_Simio = NULL;

	OSTASK_Suspend(dev->Task_GCNTTimer);
	OSTASK_Destroy(dev->Task_GCNTTimer);
	dev->Task_GCNTTimer = NULL;

	// destory semaphores
	OSSEMAPHORE_Destroy(dev->Semaphore_Simio);
	OSSEMAPHORE_Destroy(dev->Semaphore_Memlock);
	OSSEMAPHORE_Destroy(dev->SemaphoreGCNTTimer);

	// SimioHisr
	OSINTERRUPT_Destroy(dev->SimioHisr);
	
	// Timer_Simio
	OSTIMER_Destroy(dev->Timer_Simio);

	chal_simio_deinit(dev->chal_handle);
	dev->chal_handle = NULL;

	SIMIO_ATR_Cleanup(dev->atr_handle);
	dev->atr_handle = NULL;

	// clean up everything
	memset(dev, 0, sizeof(SIMIO_t));
	return TRUE;
}

//******************************************************************************
//
// Function Name: SIMIO_Reset
//
// Description: This function will request reset the SIM
//
// Notes:
//
//******************************************************************************
static void SIMIO_Reset(SIMIO_t* dev, RESET_TYPE_t reset_type)
{

    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Reset\n");
	SIM_LOGV2("SIMIO_Reset type|warm: ", reset_type, dev->warmReset);

	SIMIO_ATR_Start(dev->atr_handle, dev->Sim_Voltage, dev->pref_protocol);

#if 0  //VCC should be off
	_SIMIO_DeactiveCard(dev->id);
    _SIMIO_ShutdownVcc(dev->id);
#endif

	// Set default speed for communication
	chal_simio_set_speed(dev->chal_handle, CHAL_SIMIO_SPD_372_1);

	// Check reset type
	switch(reset_type)
	{
		case INIT_RESET:                            // set all parameters to default value
		case TRY_NEXT_VOLTAGE_RESET:                // try next higher voltage
			dev->Reconfig_Done   = FALSE;    // TRUE: reconfig done, FALSE: not (for hw reconfig)
			dev->Select_Done     = FALSE;    // TRUE: SELECT cmd done, FALSE: not
			dev->Max_WaitFor_Receive = MAX_WAIT_FOR_RECEIVE_PROTOCOL_T0;
			dev->Max_WaitFor_Transmit = MAX_WAIT_FOR_TRANSMIT_PROTOCOL_T0;

			dev->fClockStopMode_Settle = FALSE;
			dev->Clk_Stop_Skipped = FALSE;
			dev->ClockStopMode = SIMIO_CLOCK_STOP_NOT_ALLOWED;

			memset(&dev->ATR_ParamData, 0, sizeof(SIMIO_USIMAP_ATR_PARAM_t));

			break;

		case VOLTAGE_CHANGE_RESET:                  // reset for voltage change
		default:
			break;
	}

	// setup simio state
	dev->Simio_St = SIMIO_SimReset;
	dev->cmd_rcvd_time = 0;

	// release semaphore
	OSSEMAPHORE_Release(dev->Semaphore_Simio);
}


//******************************************************************************
//
// Function Name: SIMIO_SendSimRemovalStatus
//
// Description: This function sends SIM Removed indication to SIMMAP.
//
//******************************************************************************
static void SIMIO_SendSimRemovalStatus(SIMIO_t* dev)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_SendSimRemovalStatus\n");

	if( dev->RemovedSignalSent == FALSE && dev->Simio_St != SIMIO_SimReset)
	{
		_SIM_LOSS_DBG_( SIMIO_PrintSioLog() );
		dev->RemovedSignalSent = TRUE;
	}

	SEND_SIGNAL_REMOVE(dev);

	SIM_LOGV4( "SIMIO_SendSimRemovalStatus: ",
				dev->spurious_int_count, dev->irq_flood_count, dev->ParityErrorCount, 0);

	SLEEP_EnableDeepSleep(dev->sim_sleep_id);
	PEDESTAL_EnablePedestalMode(dev->sim_pedestal_id);
}



//******************************************************************************
//
// Function Name: SIMIO_ISR
//
// Description: This function is the SIMs ISR.
//
// Notes:
//
//******************************************************************************
static void SIMIO_ISR(SIMIO_t* dev)
{
	UInt32 timer_value;
	UInt32 hist_index;
	SIMIO_ISR_DATA_t *isr_data;
	UInt32 mask;
	Boolean trigger_hisr = FALSE, spurious_int = FALSE;
    Boolean sim_irq_flood = FALSE;

    dprintf(DBG_LI, "SIMIO Driver: SIMIO_ISR\n");

	IRQ_Disable(dev->irq);

	// For debugging purpose, do not remove. 
	hist_index = dev->isr_hist_cnt % ISR_HIST_SIZE;
	isr_data = &(dev->isr_hist[hist_index]);

	isr_data->Sim_St = dev->Sim_St;
	chal_simio_read_reg(dev->chal_handle, &(isr_data->scr_reg), &(isr_data->sier_reg), &(isr_data->ssr_reg),
						&(isr_data->sfcr_reg), &(isr_data->sdebug_reg));
	isr_data->time = TIMER_GetAccuValue();


#ifdef SIMIO_CARD_DETECTION_ENABLED
	mask = chal_simio_read_esd_det_intr_status(dev->chal_handle);

#if 0
    if(mask & (CHAL_SIMIO_INT_WATDOG_ESD_ISR | CHAL_SIMIO_INT_BATRM_ESD_ISR | CHAL_SIMIO_INT_CARD_OUT_ESD_ISR))
    {
        SIM_LOG("SIMIO: SIMIO_ISR: Emergency shutdown");

        // Emergency shutdown happens
        //dev->Simio_St =  SIMIO_SimRemoved;
        dev->esd_det_triggered = TRUE;
        OSINTERRUPT_Trigger(dev->SimioHisr);
        //goto _exit_isr;
    }
#endif
    if(mask & CHAL_SIMIO_INT_CARD_OUT_ISR)
    {
        SIM_LOG("SIMIO: SIMIO_ISR: CHAL_SIMIO_INT_CARD_OUT_ISR");
 
        // Card removed
        //dev->Simio_St =  SIMIO_SimRemoved;
        dev->esd_det_triggered = TRUE;
        OSINTERRUPT_Trigger(dev->SimioHisr);
        goto _exit_isr;
    }
    else if(mask & CHAL_SIMIO_INT_CARD_IN_ISR)
    {
        SIM_LOG("SIMIO: SIMIO_ISR: CHAL_SIMIO_INT_CARD_IN_ISR");
 
        // Card inserted
        //dev->Simio_St =  SIMIO_SimDetected;
        dev->esd_det_triggered = TRUE;
        OSINTERRUPT_Trigger(dev->SimioHisr);
        goto _exit_isr;
    }
#endif

	if (dev->Sim_St == SIM_Dummy_ST)
	{
		// We are not in a communication session with the SIM, just ignore and clear the interrupt	
		SIMIO_PurgeSimChar(dev);
		chal_simio_read_intr_status(dev->chal_handle);
		chal_simio_mask_data_in(dev->chal_handle, TRUE);

		// For debugging purpose, do not remove. 
		dev->spurious_int_count++;
		spurious_int = TRUE;
		
		goto _exit_isr;
	}

	timer_value = TIMER_GetAccuValue();

	if (dev->int_time == 0)
	{
		// This is the first time that the SIM IRQ is triggered
		dev->int_time = timer_value;
		dev->int_count = 1;
	}
	else 
	{
		UInt32 time_elapsed = (timer_value >= dev->int_time) ? (timer_value - dev->int_time) : (0xFFFFFFFFUL - dev->int_time + timer_value);
        
		if (dev->int_count != 0xFFFFFFFFUL)
		{
			dev->int_count++;
		}

		if (dev->Sim_St == SIM_ATR_ST) {
			/* in ATR mode, there is 1 interrupt per character, so don't check for the overflow */
		} else {
			if ((time_elapsed <= ACCURATE_TIMER_VALUE_FOR_400US) && (dev->int_count > MAX_INT_ALLOWED_IN_400US)) {
				sim_irq_flood = TRUE;
			}
		}

		/* force recovery */
		if (isr_data->ssr_reg & CHAL_SIMIO_INT_ROVF) {
			sim_irq_flood = TRUE;
		}

		if (sim_irq_flood)
		{
			dev->int_count = 1;
			dev->int_time = timer_value;

	        /* We are getting a flood of interrupts from SIM controller within 0-4 ms indicating something is terrribly wrong. Just 
			 * Remove all the characters received so far from the receive buffer and disable interrupts so that SIMIO receiver timer times out 
			 * and SIM recovery logic is kicked off to recover communication with SIM. 
			 */
		    SIMIO_PurgeSimChar(dev);
			chal_simio_read_intr_status(dev->chal_handle);
			chal_simio_mask_data_in(dev->chal_handle, TRUE);

			// For debugging purpose, do not remove. 
			dev->irq_flood_count++;

			goto _exit_isr;
		}
	}

	mask = chal_simio_read_intr_status(dev->chal_handle);

	if (dev->expecting_txdone)
	{
		dev->expecting_txdone = FALSE;
		
		if (!(mask & CHAL_SIMIO_INT_TXDONE))
		{
			/* In rare cases, it was observed that TXDONE isn't set either due to the card sends too fast or noise.
			 * We workaround this by forcing the TXDONE bit which causes the state machine to burst read.
			 */
			mask |= CHAL_SIMIO_INT_TXDONE;
			SIM_LOG("SIMIO: ISR workaround missing TXDONE issue");
		}
	}

	if (!mask)
	{
		SIM_LOGV2("SIMIO: ISR invoked when no SIM event: SIER_REG|SSR_REG", isr_data->sier_reg, isr_data->ssr_reg);
		assert(0);
	}

	if(mask & CHAL_SIMIO_INT_PERR)
	{
		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)(SSR_REG), Sim_St, 0x00) );
		dev->Parity_Error = 1;
	}

	if(mask & (CHAL_SIMIO_INT_TXDONE | CHAL_SIMIO_INT_TXTHRE))
	{
		if(mask & CHAL_SIMIO_INT_TXDONE) 
		{
			chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_TXDONE);
			// JLIM : Check 0x200, TXEMPTY
			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)(SSR_REG), Sim_St, 0x01) );
		}

		if(mask & CHAL_SIMIO_INT_TXTHRE) 
		{
			chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_TXTHRE);
			// JLIM : Check 0x200, TXEMPTY
			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)(SSR_REG ), Sim_St, 0x02) );
		}

		if( IS_RX_MODE(dev))
		{
			SIM_LOGV2("SIMIO: Triggered by TX events when in RX mode: SIER_REG|SSR_REG", isr_data->sier_reg, isr_data->ssr_reg);
//			dump_isr_hist(dev);

//			assert(0);

			CAL_LOG("SIMIO TXDONE interrupt in Rx mode");
			_SIM_LOSS_DBG_( SIMIO_Log_Event(dev->RxTxIndex, Sim_St, 0x04) );
		}

		// handle the PTS transmission
		if(dev->Sim_St == SIM_ATR_ST)
		{
			if(dev->RxTxIndex < dev->TxLength)
			{
				_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dev->SimBuffer[RxTxIndex], dev->Sim_St, 0x05) );

				dev->RxTxIndex += chal_simio_write_data(dev->chal_handle, dev->SimBuffer+dev->RxTxIndex, dev->TxLength - dev->RxTxIndex);
				SET_TX_MODE(dev);
			}
			else
			{
				_SIM_LOSS_DBG_( SIMIO_Log_Event(0, dev->Sim_St, 0x06) );

				// Change to reception mode to receive the response for PTS
				_DBG_2133_( CAL_LOGV("SIMIO: Enable rx mode 0", dev->Max_WaitFor_Receive) );

				SIMIO_TimerStart(dev, SIMIO_TOTYPE_PPS, 9600);
			}

			goto _exit_isr;
		}

		_DBG_2133_( CAL_LOGV4("SIMIO Sim_St RxTxIndex RxTxIndex: ", dev->Sim_St,dev->RxTxIndex,dev->TxLength,0) );

		// check if we sent all cmd over
		if(dev->RxTxIndex < dev->TxLength)
		{
			// handle the the case of Cmd_Instruction^0xFF and (Cmd_Instruction + 1)^0xFF
			if(dev->Sim_St == SIM_PB_Tx_ST)
			{
				/* This is the case that ME send one char. after receive inverse INS */
				_SIM_LOSS_DBG_( SIMIO_Log_Event(0, dev->Sim_St, 0x07) );

				STOP_TIMER_BWT(dev);
				SET_RX_MODE(dev);
			}
			else
			{
				// check if header is over(5 chars)
				if((dev->RxTxIndex == T0_CMD_HEADER_LENGTH) && !dev->ATR_ParamData.is_T1_protocol )                    // T=0 Protocol
				{
					_SIM_LOSS_DBG_( SIMIO_Log_Event(0, dev->Sim_St, 0x08) );

					// change to Rx mode to receive Procedure Byte
					dev->Sim_St = SIM_PB_Tx_ST;

					STOP_TIMER_BWT(dev);
					SET_RX_MODE(dev);
				}
				else
				{
					_SIM_LOSS_DBG_( SIMIO_Log_Event(dev->RxTxIndex, dev->Sim_St, 0x09) );
					OSINTERRUPT_Trigger( dev->SimioHisr );
					trigger_hisr = TRUE;
				}
			}
		}
		else
		{
			_SIM_LOSS_DBG_( SIMIO_Log_Event(dev->RxTxIndex, dev->Sim_St, 0x0A) );

			// change to Rx mode to receive data after send all datas
			if (!dev->ATR_ParamData.is_T1_protocol)      // T=0 Protocol
			{
				dev->Sim_St = SIM_PB_Rx_ST;          // T=0
	            
	            // Done with TX
	            if (dev->TxLength == T0_CMD_HEADER_LENGTH)
	            {
    	            // For RX case, we starts burst in LISR
			        if (dev->RspLength <= DEFAULT_RXFIFO_LENGTH)
			            chal_simio_set_rx_threshold(dev->chal_handle, dev->RspLength);
                    else
        			    chal_simio_set_rx_threshold(dev->chal_handle, DEFAULT_RXFIFO_LENGTH);

 	                chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_RDR);

                    //SIM_LOGV("SIMIO_ISR: RX burst started", dev->RspLength);
	            }
			}
			else
			{
				dev->Sim_St = SIM_Read_Remain_ST;    // T=1
			}

			dev->RxTxIndex = 0;

			_DBG_2133_( CAL_LOG("SIMIO Enable rx mode 3") );

			STOP_TIMER_BWT(dev);
			SET_RX_MODE(dev);
		}
	}
	else if ( mask & (CHAL_SIMIO_INT_RXTOUT | CHAL_SIMIO_INT_RXTHRE | CHAL_SIMIO_INT_ROVF | CHAL_SIMIO_INT_RDR) )
	{
		UInt8  rcvd_data[MAX_SIM_FIFO_SIZE];
		UInt16  rcvd_raw_data[MAX_SIM_FIFO_SIZE];
		UInt16  rcvd_data_len;
		UInt16	rcvd_real_data_len;
		UInt16  i;
		UInt16	rcvd_word=0;

		rcvd_data_len = chal_simio_read_data(dev->chal_handle, rcvd_raw_data, MAX_SIM_FIFO_SIZE);

		if (rcvd_data_len > 0)
		{
			rcvd_real_data_len = 0;

			for (i = 0; i < rcvd_data_len; i++)
			{
				//rcvd_word = GET_ONE_RCVD_BYTE_FROM_FIFO();
				rcvd_word = rcvd_raw_data[i];

				if ( (rcvd_word & 0x0100) == 0 )
				{
					rcvd_data[rcvd_real_data_len++] = (rcvd_word & 0xff);
                                        dev->ATRRxBuffer[dev->ATRRxCount++] = (rcvd_word & 0xff);
				}
				else
				{
					// skip the byte with parity error
					if (dev->ATR_ParamData.is_T1_protocol)
					{
						dev->T1ParityError = TRUE;
						rcvd_data[rcvd_real_data_len++] = (rcvd_word & 0xff);
                                                dev->ATRRxBuffer[dev->ATRRxCount++] = (rcvd_word & 0xff);

						CAL_LOGV4("SIMIO: Parity Error index|rcvd_data_len|data", i, rcvd_data_len, rcvd_word, rcvd_real_data_len);
					}
					else
					{
						dev->ParityErrorCount++;
					    CAL_LOGV("SIMIO FIFO parity err: ", rcvd_word);
					}
				}

				// CAL_LOGV("SIMIO read data: ", rcvd_data[i]);
			}

			/* 102-230 6.1.1 T0/T1 inverse FIX */
			if ( (dev->ParityErrorCount != 0) && (dev->Sim_St == SIM_ATR_ST) )
			{
				//if ( (ATR_Index == SimATR_TS) && ((UInt8) rcvd_word == INVERSE) )
				if (SIMIO_ATR_Detect_Inverse(dev->atr_handle, rcvd_word))
				{
                    chal_simio_set_order_sense(dev->chal_handle, TRUE);
					//TS_Char = INVERSE;
					dev->ParityErrorCount = 0;

					CAL_LOGV("SIMIO SET_ODD_PARITY: ", rcvd_word);
					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_data_len, dev->Sim_St, 0x0B) );

					goto _exit_isr;
				}
				else
				{
					// Rhea sets these bits incorrectly. Clear them!
					chal_simio_set_order_sense(dev->chal_handle, FALSE);

				}
			}

			SIMIO_PutSimChar(dev, rcvd_data, rcvd_real_data_len);

			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)(rcvd_data[0]<<8+rcvd_data[1]), dev->Sim_St, 0x0C) );

			if (rcvd_real_data_len > 0)
			{
				OSINTERRUPT_Trigger(dev->SimioHisr);
				trigger_hisr = TRUE;
			}
		}
		else
		{
			_SIM_LOSS_DBG_( SIMIO_Log_Event(0, dev->Sim_St, 0x0D) );

			_DBG_2133_( CAL_LOG("SIMIO here 04") );
		}
	}
	else if(mask & CHAL_SIMIO_INT_GCNTI)
	{
		/*CAL_LOG(" intrrupted! #");
		CAL_LOGV(" SIMIO isr SGCCR_REG=", SGCCR_REG );
		CAL_LOGV(" SIMIO isr SGCVR_REG=", SGCVR_REG );*/

		chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_GCNTI);

		dev->GCNTI_timer_triggered = TRUE;
		OSINTERRUPT_Trigger( dev->SimioHisr  );
		trigger_hisr = TRUE;
	}
	else
	{
		/* Something is wrong */
		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)(SSR_REG), dev->Sim_St, 0x0E) );
	}

_exit_isr:
	IRQ_Clear(dev->irq);

#ifdef SYNC_LISR_HISR
	if (dev->Sim_St != SIM_Dummy_ST && trigger_hisr != TRUE && !sim_irq_flood) 
#else
    if (dev->Sim_St != SIM_Dummy_ST && !sim_irq_flood)
#endif
	{
		IRQ_Enable(dev->irq);
	}

//    SIM_LOGV6("SIMIO_ISR: ssr|hisr|st|irq_en|spur_int|flood", 
//        dev->ssr_reg, trigger_hisr, dev->Sim_St, IRQ_IsEnabled(dev->irq),
//        spurious_int, sim_irq_flood);

    if (isr_data->ssr_reg & CHAL_SIMIO_INT_ROVF)
    {
        SIM_LOGV4("SIMIO_ISR: RX oveflow! hisr|st|irq_en|flood", 
            trigger_hisr, dev->Sim_St, IRQ_IsEnabled(dev->irq), sim_irq_flood);
        //assert(0);
    }

	isr_data->Sim_St_exit = dev->Sim_St;
	isr_data->time_exit = TIMER_GetAccuValue();
	chal_simio_read_reg(dev->chal_handle, &(isr_data->scr_reg_exit), &(isr_data->sier_reg_exit), &(isr_data->ssr_reg_exit),
						&(isr_data->sfcr_reg_exit), &(isr_data->sdebug_reg_exit));
	isr_data->trigger_hisr_exit = trigger_hisr;
	dev->isr_hist_cnt++;

}

static void sim_card_tasklet_handler(unsigned long sim_card_data)
{
    SIMIO_t* dev = &simio_dev[0];  

    SIMIO_ISR(dev);
    IRQ_Enable(dev->irq);
}

static void sim_card_esd_reinit(SIMIO_t *dev)
{
    chal_simio_soft_reset_esd(dev->chal_handle);
    chal_simio_enable_eshutdown(dev->chal_handle,
        SIMIO_ENABLE_ESD,
        SIMIO_ENABLE_ESD_BATTERY_REMOVAL,
        SIMIO_ENABLE_ESD_SIM_REMOVAL,
        SIMIO_ENABLE_ESD_RESET_WDOG);
    chal_simio_set_detection(dev->chal_handle, TRUE, CHAL_SIMIO_DEBOUNCE_TIME_32, CHAL_SIMIO_DEBOUNCE_MODE_PULSEEXT, CHAL_SIMIO_PRESENCE_LOW);
    chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_OUT_IER | CHAL_SIMIO_INT_CARD_IN_IER);
}

static void sim_card_tasklet_det_handler(unsigned long sim_card_data)
{
    SIMIO_t* dev = &simio_dev[0];  
    cUInt32 mask;

    mask = chal_simio_read_esd_det_intr_status(dev->chal_handle);
    if(mask & CHAL_SIMIO_INT_CARD_OUT_ISR)
    {
        SIM_LOG("SIMIO: SIMIO_ISR: CHAL_SIMIO_INT_CARD_OUT_ISR");
        sim_card_esd_reinit(dev);
	chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_OUT_ISR );
        SEND_SIGNAL_REMOVE(dev);
    }
    else if(mask & CHAL_SIMIO_INT_CARD_IN_ISR)
    {
        SIM_LOG("SIMIO: SIMIO_ISR: CHAL_SIMIO_INT_CARD_IN_ISR");
        SEND_SIGNAL_INSERT(dev);
	chal_simio_enable_esd_det_intr(dev->chal_handle, CHAL_SIMIO_INT_CARD_IN_ISR);
    }

    BcmHalInterruptEnable(INTERRUPT_ID_USIM_ESD);
}

//******************************************************************************
//
// Function Name: SIMIO_GetSimData
//
// Description: This function gets received characters from Rxbuffer
//
// UInt8 *buffer - buffer to store received characters
// buffer_size - size of the above buffer
//
// Return: number of bytes put into buffer
//         (if 0, means that there is no received character in Rxbuffer)
//
//******************************************************************************
static UInt16 SIMIO_GetSimData(SIMIO_t* dev, UInt8 *buffer, UInt16 buffer_size)
{
	UInt16 data_size;
	UInt16 i;

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetSimData\n");
	/* Access the global Rx SIM data buffer (Rxbuffer). In the middle, SIMIO_ISR
	 * may interrupt us and write to the same buffer by calling SIMIO_PutSimChar().
	 * So we must protect this critical section by disabling SIM interrupt.
	 */
 
#ifndef SYNC_LISR_HISR
	IRQ_Disable(dev->irq);
#endif

	data_size = MIN(buffer_size, dev->RxBuffer_Count);

	for(i = 0; i < data_size; i++)
	{
		buffer[i] = dev->RxBuffer[dev->RxChar_Tail_Index++];

		if(dev->RxChar_Tail_Index == SIMIO_RCVD_BUFFER_SIZE)
		{
			dev->RxChar_Tail_Index = 0;
		}
		// SIM_LOGV("SIMIO SIMIO_GetSimData: ", buffer[i]);
	}

	dev->RxBuffer_Count -= data_size;

#ifndef SYNC_LISR_HISR
	IRQ_Enable(dev->irq);
#endif

	return data_size;
}


//******************************************************************************
//
// Function Name: SIMIO_PutSimChar
//
// Description: This function put received characters to Rxbuffer
//
//******************************************************************************
static void SIMIO_PutSimChar(SIMIO_t* dev, UInt8 *data, UInt16 data_len)
{
	UInt16 i;

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_PutSimChar\n");
	for(i = 0; i < data_len; i++)
	{
		//SIM_LOGV("SIMIO SIMIO_PutSimChar: ", *data);

		dev->RxBuffer[dev->RxChar_Head_Index++] = *data++;

		if(dev->RxChar_Head_Index == SIMIO_RCVD_BUFFER_SIZE)
		{
			dev->RxChar_Head_Index = 0;
		}
	}

	dev->RxBuffer_Count += data_len;
}


//******************************************************************************
//
// Function Name: SIMIO_PurgeSimChar
//
// Description: This function removes all the characters in the RxBuffer as well
//				as characters buffered in FIFO.
//
//				Before this function is called, SIM IRQ must be disabled.
//
//******************************************************************************
static void SIMIO_PurgeSimChar(SIMIO_t* dev)
{
	UInt16 rcvd_data[MAX_SIM_FIFO_SIZE];

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_PurgeSimChar\n");

	dev->RxChar_Head_Index = 0;
	dev->RxChar_Tail_Index = 0;
	dev->RxBuffer_Count = 0;

	(void) chal_simio_read_data(dev->chal_handle, rcvd_data, MAX_SIM_FIFO_SIZE);
}


//******************************************************************************
//
// Function Name: SIMIO_HISR
//
// Description: This function is the SIMIO HISR.  It simply releases the
//              a semaphore to wake-up any waiting tasks.
//
// Notes:
//
//******************************************************************************
static void SIMIO_HISR(SIMIO_t* dev)
{
	UInt8   rcvd_data[MAX_SIM_FIFO_SIZE];
	UInt16  rcvd_data_len;
	UInt16  i;
#ifdef SIMIO_CARD_DETECTION_ENABLED
    Boolean inserted = FALSE;
    UInt32 mask;
#endif

    dprintf(DBG_LI, "SIMIO Driver: SIMIO_HISR\n");

#ifdef SIMIO_CARD_DETECTION_ENABLED
    if (dev->esd_det_triggered)
    {
        OSSEMAPHORE_Release(dev->Semaphore_Simio);
        dev->esd_det_triggered = FALSE;

        mask = chal_simio_read_detection_status(dev->chal_handle);
        if(mask == SIMIO_CARD_REMOVED)
        {
		    SIM_LOGV("SIMIO: SIMIO_HISR: SIMIO_CARD_REMOVED ", mask);
            // Card removed
            inserted = FALSE;
        }
        else if(mask == SIMIO_CARD_INSERTED)
        {
		    SIM_LOGV("SIMIO: SIMIO_HISR: SIMIO_CARD_INSERTED ", mask);
            // Card inserted
            inserted = TRUE;
        }
        else
        {
		    SIM_LOGV("SIMIO: SIMIO_HISR: detection status error ", mask);
            IRQ_Enable(dev->irq);
            goto _exit_hisr;
        }
    
        SIM_LOGV2("SIMIO: SIMIO_DET_HISR: id|insert", dev->id, inserted);
    
        // call upper layer callback in HISR context
        if (dev->simio_det_callback)
        {
            dev->simio_det_callback(inserted);
            SIM_LOG("SIMIO: SIMIO_DET_HISR: called callback");
        }
        else
            SIM_LOG("SIMIO: SIMIO_DET_HISR: no callback");

        IRQ_Enable(dev->irq);
        goto _exit_hisr;
    }
#endif

	if (dev->GCNTI_timer_triggered)
	{
		// release semaphore and let GCNT Timer task handle
		OSSEMAPHORE_Release( dev->SemaphoreGCNTTimer );
		dev->GCNTI_timer_triggered = FALSE;
		
		goto _exit_hisr;
	}

	if( dev->Parity_Error )
	{
		dev->Parity_Error=0;
		SIM_LOG("SIMIO Parity Error ");

		//if (SimATR_PTSS==ATR_Index||SimATR_PTS0==ATR_Index||SimATR_PTS1==ATR_Index)
		//{
		//    SIM_LOG("SIMIO: ATR parity error, set ATR_Corrupted ") ;
		//    ATR_Corrupted = TRUE;
		//}
	}

#if defined(SIMIO_DMA_TX) || defined(SIMIO_DMA_RX)
	if ( (IS_DMADRV_RegisterSimioHISR() != NULL) && (!ATR_ParamData.is_T1_protocol) )                //T=0
	{
		_DBG_2133_( SIM_LOG("SIMIO HISR DMA #1: ") );
		SIMIO_Dma_End(DMA_END_NORMAL);
		SET_RX_MODE();
		
		return;
	}
	else if ( (IS_DMADRV_RegisterSimioHISR() != NULL) && ATR_ParamData.is_T1_protocol )              //T=1
	{
		SIMIO_Dma_End(DMA_END_NORMAL);
		
		if(MEM2SIM_DIRECTION == dma.direction)
		{
			_DBG_2133_( SIM_LOG("SIMIO HISR DMA #MEM2SIM: ") );
			SET_RX_MODE();
		}
		else if(SIM2MEM_DIRECTION == dma.direction)
		{
			SIM_LOG("SIMIO HISR DMA #SIM2MEM: ");
			OSSEMAPHORE_Release( Semaphore_Simio );                                         // release semaphore, in order to let task run
		}
		
		return;
	}
#endif

	if(dev->Sim_St == SIM_Write_ST)
	{
		/* We do not receive any character, pass 0 as don't care */
		SIMIO_Write(dev, 0);
		goto _exit_hisr;
	}

	while( (rcvd_data_len = SIMIO_GetSimData(dev, rcvd_data, sizeof(rcvd_data))) > 0 )
	{
		for(i = 0; i < rcvd_data_len; i++)
		{
			// Use state machine to handle different case
			switch(dev->Sim_St)
			{
				case SIM_ATR_ST:
					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_data[i], dev->Sim_St, 0x11) );

					if ( chal_simio_get_reset_level(dev->chal_handle) )
					{
						SIMIO_ProcessATR(dev, rcvd_data[i], TRUE);
					}
					else
					{
						SIM_LOGV("SIMIO HISR ignore byte", rcvd_data[i]);
					}

					break;

				case SIM_PB_Rx_ST:
				case SIM_Read_One_ST:
				case SIM_Read_Remain_ST:
					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_data[i], dev->Sim_St, 0x12) );
					SIMIO_Read(dev, rcvd_data[i], rcvd_data_len - i - 1);
					break;

				case SIM_PB_Tx_ST:
					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_data[i], dev->Sim_St, 0x13) );
					SIMIO_Write(dev, rcvd_data[i]);

					break;

				default:
					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_data[i]<<8+Simio_St, dev->Sim_St, 0x14) );
					SIM_LOGV("SIMIO HISR Wrong state: ", dev->Simio_St );

					break;
			}
		}
	}

    // Burst read
	if (dev->Sim_St == SIM_Read_Remain_ST)
	{
	    /* Adjust Rx FIFO threshold for better RX performance */
	    if (dev->RxCount > 1)
		{
			SIM_LOGV("SIMIO HISR: RxCount", dev->RxCount);
		    if (dev->RxCount <= DEFAULT_RXFIFO_LENGTH)
			    chal_simio_set_rx_threshold(dev->chal_handle, dev->RxCount);
			else
			    chal_simio_set_rx_threshold(dev->chal_handle, DEFAULT_RXFIFO_LENGTH);
	                        
	        // no more single-byte interrupt
	        chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_RDR);
	    }
	    else
	        chal_simio_enable_intr(dev->chal_handle, CHAL_SIMIO_INT_RDR);
	}

_exit_hisr:;
#ifdef SYNC_LISR_HISR
    if (dev->Sim_St != SIM_Dummy_ST)
    {
        IRQ_Enable(dev->irq);
    }
#endif
}

int SIMIO_read_detection_status(SIMIO_ID_t id, int *detection_status)
{
    SIMIO_t* dev = &simio_dev[id];

    *detection_status = chal_simio_read_detection_status(dev->chal_handle);
    return 0;
}

//******************************************************************************
//
// Function Name: SIMIO_Read
//
// Description: This function is will handle SIM data reading procedure
//
// Notes:
//          See ISO/IEC 7816/3 section 8.2.2 for description
//          of procedure byte processing.
//
//******************************************************************************
static void SIMIO_Read(SIMIO_t* dev, UInt8 rcvd_char, UInt8 remaining_num_of_char)
{
	Boolean reset_rcv_counter = TRUE;

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Read\n");

	switch(dev->Sim_St)
	{
		case SIM_PB_Rx_ST:
			if(rcvd_char != 0x60)
			{
				if( ((rcvd_char & 0xF0) == 0x90) || (( rcvd_char & 0xF0) == 0x60) ) // check SW1
				{
					if(dev->RxTxIndex == 0)
					{
						dev->RspLength = 2;  // Total number of bytes of response data
					}

					dev->SimBuffer[dev->RxTxIndex++] = rcvd_char;
					dev->RxCount = 1;    // for receiving SW2

					chal_simio_enable_vpp(dev->chal_handle, 0);
					dev->Sim_St = SIM_Read_Remain_ST;

					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x20) );
				}
				else
				{
					// Table 9 in ISO/IEC 7816 describes the following logic.
					if( (rcvd_char == dev->Cmd_Instruction) || (rcvd_char == (dev->Cmd_Instruction + 1)) )
					{
						if(rcvd_char == dev->Cmd_Instruction)
						{
							chal_simio_enable_vpp(dev->chal_handle, 0);
						}
						else
						{
							chal_simio_enable_vpp(dev->chal_handle, 1);
						}

						// adjust the RspLength
						dev->RxCount = dev->RspLength - dev->RxTxIndex;
						dev->Sim_St = SIM_Read_Remain_ST;

						reset_rcv_counter = FALSE;
						
						_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dev->RxTxIndex, dev->Sim_St, 0x21) );
					}
					else
					{
						if ( (rcvd_char == (dev->Cmd_Instruction ^ 0xFF)) || (rcvd_char == ((dev->Cmd_Instruction + 1) ^ 0xFF)) )
						{
							if ( rcvd_char == (dev->Cmd_Instruction ^ 0xFF) )
							{
								chal_simio_enable_vpp(dev->chal_handle, 0);
							}
							else
							{
								chal_simio_enable_vpp(dev->chal_handle, 1);
							}

							dev->Sim_St = SIM_Read_One_ST;

							reset_rcv_counter = FALSE;
						}
						else
						{
							/* Some Cingular SIM's sometimes do not send procedure byte for Get Response command
							 * (A0 C0 00 00 0F). Instead they just directly send the response back to ME or our
							 * ASIC HW fails to receive the procedure byte in this situation. Here we handle this
							 * situation by storing the response data.
							 */
							dev->SimBuffer[dev->RxTxIndex++] = rcvd_char;
							dev->RxCount = dev->RspLength - dev->RxTxIndex;
							dev->Sim_St = SIM_Read_Remain_ST;
						}

						_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dev->rcvd_char, dev->Sim_St, 0x22) );
					}
				}
			}
			else
			{
				/* Else: this is Procedure NULL byte, do nothing if received it */
				_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dev->rcvd_char, dev->Sim_St, 0x23) );

				reset_rcv_counter = !SIMIO_IsNullTimeExc(dev);
				dev->Max_WaitFor_Receive = MAX_LONG_WAIT_FOR_RECEIVE_PROTOCOL_T0;
				dev->ATRRxCount--;  /*skip the NULL byte*/
                                OSTIMER_Start(dev->Timer_User);
                        }

			break;

		case SIM_Read_One_ST:
			dev->SimBuffer[dev->RxTxIndex++] = rcvd_char;
			dev->Sim_St = SIM_PB_Rx_ST;

			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x24) );

			break;

		case SIM_Read_Remain_ST:
			if( !dev->ATR_ParamData.is_T1_protocol && (dev->RxCount == 2) && (rcvd_char == 0x60) )    //0x60 = NULL byte
			{
				/* We are expecting SW1, but receive NULL BYTE value instead, so ignore it */
				_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x25) );

				reset_rcv_counter = !SIMIO_IsNullTimeExc(dev);
			}
			else
			{
				if (dev->ATR_ParamData.is_T1_protocol)
				{
					dev->T1BWT_On = FALSE;

					// T1 : NAD + PCB + LEN + INF(0-254) + EDC
					// T1 : [0] + [1] + [2] + [::::::::] + [0 or 1]
					if ( dev->RxTxIndex == 2 )
					{
						// pass 102 230 7.3.5 IFSD Chaining - IFSD management
						if ( (rcvd_char == 0xFF && !dev->T1ParityError) ||(rcvd_char>dev->ATR_ParamData.atr_T1_ifsc && !dev->T1ParityError))
						{
							if (rcvd_char > dev->ATR_ParamData.atr_T1_ifsc)
							{
								SIM_LOGV4("SIMIO T1 Block size greater than IFSD length! Invalid! recv_len | ATR_T1_IFSC", rcvd_char, dev->ATR_ParamData.atr_T1_ifsc, 0,0);
							}
							else if (rcvd_char == 0xFF && !dev->T1ParityError)
							{
								SIM_LOG("SIMIO T1 length is 0xFF! Invalid!");
							}

							dev->T1_Length_Invalid = TRUE;
							dev->Sim_St = SIM_Dummy_ST;
							dev->Simio_St = SIMIO_T1InvalidLength;
							OSSEMAPHORE_Release(dev->Semaphore_Simio);
						}

						dev->RxCount =  rcvd_char + 2;                   // rcvd_char=LEN
						dev->RspLength = rcvd_char + 4 + dev->ATR_ParamData.atr_T1_edc;     // Calulate again based on received LEN

						_DBG_2133_( SIM_LOGV4("SIMIO Read(RxTxIndex == 2)", dev->RxTxIndex, dev->RxCount, dev->RspLength, rcvd_char) );

#ifdef SIMIO_DMA_RX
						SimBuffer[RxTxIndex++] = rcvd_char;                 //Do not move

						if(SIMIO_Dma_Start(SimBuffer + RxTxIndex, (UInt16)rcvd_char+1, SIM2MEM_DIRECTION, DMA_TRIGGER_3, SIM_Dummy_ST, NULL))
						{
							IRQ_Disable(dev->irq);
							WATCH_FOR_RECEIVING_RESET(dev);
							IRQ_Enable(dev->irq);

							return;
						}
						else
						{
							SIMIO_Dma_End(DMA_END_STARTFAIL);
							RxTxIndex -= 1;
							SET_RX_MODE();
						}
#endif
					}
				}

				if (dev->RxCount > 0)
				{
					dev->SimBuffer[dev->RxTxIndex++] = rcvd_char;

					if (--dev->RxCount == 0)
					{
						if (remaining_num_of_char == 0)
						{
							//SIM_LOG("SIMIO set to response data state or t1 parity error state...");
							dev->Simio_St = dev->T1ParityError ? SIMIO_T1Parity : SIMIO_RspData;

							dev->Sim_St = SIM_Dummy_ST;

							// disable time watch for receiving
							SIMIO_Timer_Reset(dev);

							// release semaphore, in order to let task run
							OSSEMAPHORE_Release( dev->Semaphore_Simio );
							_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x26) );
						}
						else
						{
							OSTIMER_Stop(dev->Timer_Simio);
							dev->Simio_St = SIMIO_TimeOut;

							OSSEMAPHORE_Release(dev->Semaphore_Simio);
						}
					}
				}
				else
				{
					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x27) );
				}
			}
			break;

		case SIM_Dummy_ST:
		default:
			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x28) );

			break;
	}

	if (reset_rcv_counter)
	{
		WATCH_FOR_RECEIVING_RESET(dev);
	}
}


//******************************************************************************
//
// Function Name: SIMIO_Write
//
// Description: This function handles SIM data transmiting procedure
//
// Notes:   See ISO/IEC 7816/3 section 8.2.2 for description
//          of procedure byte processing.
//
// UInt8 rcvd_char - the character received. This argument is only applicable
//                   when "Sim_St" is in state "SIM_PB_Tx_ST" in which we are
//                   waiting for the procedure byte. In other states, this
//                   argument is ignored.
//
//******************************************************************************
static void SIMIO_Write(SIMIO_t* dev, UInt8 rcvd_char)
{
	Boolean reset_rcv_counter = TRUE;
	Boolean tx_char_flag = FALSE;

#ifdef SIMIO_DMA_TX
	Boolean SendData_Fifo=FALSE;
#endif
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Write\n");

	switch(dev->Sim_St)
	{
		case SIM_PB_Tx_ST:
			if(rcvd_char == 0x60)
			{
				// this is Procedure NULL byte, do nothing if received it
				_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x29) );

				reset_rcv_counter = !SIMIO_IsNullTimeExc(dev);
                dev->Max_WaitFor_Receive = MAX_LONG_WAIT_FOR_RECEIVE_PROTOCOL_T0;
			}
			else
			{
				if( ((rcvd_char & 0xF0) == 0x90) || ((rcvd_char & 0xF0) == 0x60) ) // check SW1
				{
					dev->SimBuffer[0] = rcvd_char;
					dev->RxTxIndex = 1;
					dev->RxCount = 1;    // for receiving SW2
					dev->RspLength = 2;  // Total number of bytes of response data

					chal_simio_enable_vpp(dev->chal_handle, 0);
					dev->Sim_St = SIM_Read_Remain_ST;

					_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x2A) );
				}
				else
				{
					// Table 9 in ISO/IEC 7816-e: 1989 (E)
					// describes the following logic.
					if( (rcvd_char == dev->Cmd_Instruction) || (rcvd_char == (dev->Cmd_Instruction + 1)) )
					{
						// set VPP
						if(rcvd_char == dev->Cmd_Instruction)
						{
							chal_simio_enable_vpp(dev->chal_handle, 0);
						}
						else
						{
							chal_simio_enable_vpp(dev->chal_handle, 1);
						}

						dev->Sim_St = SIM_Write_ST;
						tx_char_flag = TRUE;
						
						_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x2B) );
					}
					else
					{
						if ( (rcvd_char == (dev->Cmd_Instruction ^ 0xFF)) || (rcvd_char == ((dev->Cmd_Instruction + 1) ^ 0xFF)) )
						{
							// set VPP
							if ( rcvd_char == (dev->Cmd_Instruction ^ 0xFF) )
							{
								chal_simio_enable_vpp(dev->chal_handle, 0);
							}
							else
							{
								chal_simio_enable_vpp(dev->chal_handle, 1);
							}

							tx_char_flag = TRUE;
						}
						else
						{
							reset_rcv_counter = FALSE;
						}

						_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x2C) );
					}
				}
			}
			
			break;

		case SIM_Write_ST:
			tx_char_flag = TRUE;
			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x2D) );

			break;

		case SIM_Dummy_ST:
		default:
			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)rcvd_char, dev->Sim_St, 0x2E) );

			break;
	}

	if (reset_rcv_counter)
	{
		WATCH_FOR_RECEIVING_RESET(dev);
	}

	if(tx_char_flag)
	{
		/* Some SIM cards (e.g. Gemplus SIM) do not like quick response.
		 * We delay some time before sending the response back (this is not
		 * specified in any spec, but needed just to make us work with
		 * those SIM cards.
		 */
		#define DELAY_LOOP_COUNTER 400

		volatile UInt32 delay_counter;
		UInt32 delay_loop_counter;

		if( (dev->ATR_ParamData.PTS_Required != PTS_NOT_REQ) && (dev->ATR_ParamData.PTS_Required != PTS_372_1) )
		{
			delay_loop_counter = DELAY_LOOP_COUNTER * 8;
		}
		else
		{
			delay_loop_counter = DELAY_LOOP_COUNTER;
		}

		delay_counter = 0;
		
		while(delay_counter < delay_loop_counter)
		{
			delay_counter++;
		}

		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)SimBuffer[RxTxIndex], dev->Sim_St, 0x2F) );

		_DBG_2133_( SIM_LOG("SIMIO Enable tx mode 1") );

		if(dev->Sim_St != SIM_Write_ST)		// receive inverse INS
		{
			dev->TxFifo_lenth = 1;           /* When (rcvd_char == (Cmd_Instruction ^ 0xFF) case */
		}
		else
		{
			dev->TxFifo_lenth = DEFAULT_TXFIFO_LENGTH;
		}

		chal_simio_set_tx_threshold(dev->chal_handle, MIN(dev->TxLength - dev->RxTxIndex, dev->TxFifo_lenth)); //MAX_SIM_FIFO_SIZE);

#ifdef SIMIO_DMA_TX
		if( !ATR_ParamData.is_T1_protocol && (Sim_St == SIM_Write_ST) && ((TxLength - RxTxIndex) > 2) )// T=0 & SIM_Write_ST & length>2
		{
			if(FALSE==SIMIO_Dma_Start(SimBuffer + RxTxIndex, TxLength - RxTxIndex, MEM2SIM_DIRECTION, DMA_TRIGGER_1, SIM_PB_Rx_ST, 0))
			{
				SendData_Fifo = TRUE;
				SIMIO_Dma_End(DMA_END_STARTFAIL);
			}
		}
		else 
		{	
			// T=1
			SendData_Fifo = TRUE;
		}

		if(SendData_Fifo==TRUE)
		{
			RxTxIndex += chal_simio_write_data(chal_handle, SimBuffer+RxTxIndex, TxLength - RxTxIndex);
			SET_TX_MODE();
		}
#else // no DMA
        IRQ_Disable(dev->irq);
	    dev->RxTxIndex += chal_simio_write_data(dev->chal_handle, dev->SimBuffer+dev->RxTxIndex, dev->TxLength - dev->RxTxIndex);
		SET_TX_MODE(dev);
        IRQ_Enable(dev->irq);
#endif
	}
}

//******************************************************************************
//
// Function Name: SIMIO_SetPrefProtocol
//
// Description: This function sets the preferred protocol
//
// Notes:
//
//******************************************************************************
void SIMIO_SetPrefProtocol(SIMIO_ID_t id, PROTOCOL_t protocol)
{
	SIMIO_t* dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_SetPrefProtocol\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif
	dev->pref_protocol = (PROTOCOL_t)protocol;
        chal_simio_set_protocol(dev, protocol);
}


//******************************************************************************
//
// Function Name: SIMIO_GetCurrVoltage
//
// Description: This function returns the current voltage used to activate with SIM card.
//
// Notes:
//
//******************************************************************************
SimVoltageLevel_t SIMIO_GetCurrVoltage(SIMIO_ID_t id)
{
	SIMIO_t* dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetCurrVoltage\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif
	return dev->Sim_Voltage;
}

//******************************************************************************
//
// Function Name: SIMIO_GetClockStop
//
// Description: This function returns the current clock stop mode
//
// Notes:
//
//******************************************************************************
UInt32 SIMIO_GetClockStop(SIMIO_ID_t id)
{
	SIMIO_t* dev = &simio_dev[id];

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetClockStop\n");
#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif
	return dev->ClockStopMode;
}

//******************************************************************************
//
// Function Name: SIMIO_UICCGetClockStop
//
// Description: This function returns the UICC clock stop mode
//
// Notes:
//
//******************************************************************************
UInt32 SIMIO_GetUICCClockStop(SIMIO_ID_t id)
{
	SIMIO_t* dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetUICCClockStop\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif
	return dev->UICC_ClockStopMode;
}
//******************************************************************************
//
// Function Name: SIMIO_ProcessATR
//
// Description: This function processes the Answer-To-Reset message and
//              returns TRUE if there were no errors or FALSE otherwise.
//
// Notes:
//
//******************************************************************************
static void SIMIO_ProcessATR(SIMIO_t* dev, UInt8 rcvd_char, Boolean reset_recv)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_ProcessATR\n");
	if (reset_recv)
	{
		WATCH_FOR_RECEIVING_RESET(dev);
	}

	switch (SIMIO_ATR_Process_A_Byte(dev->atr_handle, rcvd_char))
	{
		case SIMATR_ACTION_PTS_SEND:
			dev->TxLength = SIMIO_ATR_Get_PTS(dev->atr_handle, dev->SimBuffer, SIMIO_SIM_BUFFER_SIZE);

			SIM_LOGV("SIMIO sending PPS bytes", dev->TxLength);

			// New tx threshold
			chal_simio_set_tx_threshold(dev->chal_handle, dev->TxLength);
			
			// New rx threshold
			chal_simio_set_rx_threshold(dev->chal_handle, dev->TxLength-1);
	        // no more single-byte interrupt
	        chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_RDR);

			// Put bytes into the output buffer.
            IRQ_Disable(dev->irq);
			dev->RxTxIndex = 0;
#if 0 /*amir - dont want to change rate auto*/
			dev->RxTxIndex += chal_simio_write_data(dev->chal_handle, dev->SimBuffer, dev->TxLength);
#endif
			SET_TX_MODE(dev);
            IRQ_Enable(dev->irq);
			SIMIO_PrintBytes("SIMIO PPS data: ", dev->SimBuffer, dev->TxLength);
                        dev->ATRRxSuccess = 1;
			break;

		case SIMATR_ACTION_PTS_RECEIVE:
			SIM_LOG("Stop the PPS timer");
			SIMIO_TimerStop(dev);
			dev->PPS_Time_Out = FALSE;  //reset the flag after receive the PPS byte from sim card correctly
			break;

		case SIMATR_ACTION_CONFIG_HW:
			// get ATM parameters
			SIMIO_ATR_Get_Params(dev->atr_handle, &dev->ATR_ParamData);

			// get native clock stop mode
			dev->ClockStopMode = SIMIO_ATR_Get_ClkStpMode(dev->atr_handle);

			chal_simio_set_extra_guard_time(dev->chal_handle,
				(dev->ATR_ParamData.ATR_ExtGuardTime > MIN_EXTRA_GUARD_TIME ?
				dev->ATR_ParamData.ATR_ExtGuardTime : MIN_EXTRA_GUARD_TIME));

			switch(dev->ATR_ParamData.PTS_Required)
			{
				case PTS_512_8:
					SIM_LOG("SIMIO speed: setting HW to F = 512, D = 8!!!!");
					chal_simio_set_speed(dev->chal_handle, CHAL_SIMIO_SPD_512_8);
					chal_simio_set_extra_sample(dev->chal_handle, 1, 16);
					break;

				case PTS_512_16:
					SIM_LOG("SIMIO speed: setting HW to F = 512, D = 16!!!!");
					chal_simio_set_speed(dev->chal_handle, CHAL_SIMIO_SPD_512_16);
					chal_simio_set_extra_sample(dev->chal_handle, 1, 8);
					break;

				case PTS_512_32:
					SIM_LOG("SIMIO speed: setting HW to F = 512, D = 32!!!!");
					chal_simio_set_speed(dev->chal_handle, CHAL_SIMIO_SPD_512_32);
					chal_simio_set_extra_sample(dev->chal_handle, 1, 4);
					break;

				case PTS_512_64:
					SIM_LOG("SIMIO speed: setting HW to F = 512, D = 64!!!!");
					chal_simio_set_speed(dev->chal_handle, CHAL_SIMIO_SPD_512_64);
					chal_simio_set_extra_sample(dev->chal_handle, 1, 2);
					break;

				case PTS_NOT_REQ:
				case PTS_372_1:
				default:
					SIM_LOG("SIMIO speed: setting HW to F = 372, D = 1!!!!");
					chal_simio_set_speed(dev->chal_handle, CHAL_SIMIO_SPD_372_1);
					chal_simio_set_extra_sample(dev->chal_handle, 1, 80);
					break;
			}

			if (dev->ATR_ParamData.is_T1_protocol)
			{
				chal_simio_set_protocol(dev->chal_handle, 1);

				// Set T1 timeout parameter
				dev->T1SIMIO_BWT = 11 + (1 << dev->ATR_ParamData.atr_T1_bwi) * 960 + 300;  // add 300 etu margin
				dev->T1Param_BWT = BWT_T1_Table[((dev->ATR_ParamData.atr_T1_bwi <= 9) ? dev->ATR_ParamData.atr_T1_bwi : 04 )];
				dev->T1Param_CWT = CWT_T1_Table[((dev->ATR_ParamData.atr_T1_cwi <= 15) ? dev->ATR_ParamData.atr_T1_cwi : 13 )];
				dev->Max_WaitFor_Receive = dev->T1Param_CWT;
				dev->Max_WaitFor_Transmit = dev->T1Param_BWT;

				SIM_LOGV4("SIMIO Set Protocol=1", dev->T1SIMIO_BWT, dev->T1Param_BWT, dev->T1Param_CWT,0);
			}
			else
			{
				dev->Max_WaitFor_Receive = MAX_WAIT_FOR_RECEIVE_PROTOCOL_T0;
			}

			chal_simio_set_rx_retry(dev->chal_handle, 3);
			chal_simio_set_tx_retry(dev->chal_handle, 3);

			/* Set Rx FIFO threshold for 2 bytes */
			chal_simio_set_rx_threshold(dev->chal_handle, 2);

			// Sim insert state
			dev->Simio_St = SIMIO_SimInsert;

			// release semaphore, in order to let task run
			OSSEMAPHORE_Release(dev->Semaphore_Simio);

			dev->Sim_St = SIM_Dummy_ST;
                        OSSEMAPHORE_Release(dev->SemaphoreUser);                        
			break;
		
		default:
			break;
	}
}


//******************************************************************************
//
// Function Name: SIMIO_GetRawATR
//
// Description: This function returns the Raw ATR message.
//
// Notes:
//
//******************************************************************************
SIMIO_RAW_ATR_INFO_t *SIMIO_GetRawATR(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetRawATR\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	return SIMIO_ATR_Get_Raw_ATR(dev->atr_handle);
}


//******************************************************************************
//
// Function Name: SIMIO_ResetCard
//
// Description: This function Reset the SIMCARD.
//
// Notes:   SIMAP use it
//
//******************************************************************************
void SIMIO_ResetCard(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_ResetCard\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	/* SIM reset with increased voltage */
	dev->warmReset = FALSE;
	SIMIO_Reset( dev, TRY_NEXT_VOLTAGE_RESET );
}

int simio_activesim(SIMIO_ID_t id, UInt8 *data, int *len)
{
    SIMIO_t*     dev = &simio_dev[id];

    dprintf(DBG_LI, "SIMIO Driver: SIMIO_ResetCard\n");

    memset(dev->ATRRxBuffer,0,sizeof(dev->ATRRxBuffer));
    dev->ATRRxCount = 0;
    dev->ATRRxSuccess = 0;
    dev->RXExpectedByteCount = 0;

    SIMIO_ResetCard(id);

    OSTIMER_Start(dev->Timer_User);

    OSSEMAPHORE_Obtain(dev->SemaphoreUser,TICKS_FOREVER);
    OSTIMER_Stop(dev->Timer_User);

    if (dev->ATRRxCount > *len)
        return SIM_CARD_INPUTER_BUFFER_IS_TOO_SHORT;

    if (!dev->ATRRxCount)
        return SIM_CARD_NO_OUTPUT_DATA_FROM_SIM_CARD;

    *len = dev->ATRRxCount;

    memcpy(data, dev->ATRRxBuffer, *len);

    return 0;
}
//******************************************************************************
//
// Function Name: SIMIO_WarmResetCard
//
// Description: This function Reset the SIMCARD.
//
// Notes:   SIMAP use it
//
//******************************************************************************
void SIMIO_WarmResetCard(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_WarmResetCard\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

    SIMIO_Set_Clock(dev, TRUE, SIMIO_CLK_BASIC); 
	chal_simio_enable_clock(dev->chal_handle, 1);
	OSTASK_Sleep(5);
	
	/* SIM reset with increased voltage */
	dev->warmReset = TRUE;
	SIMIO_Reset( dev, TRY_NEXT_VOLTAGE_RESET );
}

//******************************************************************************
//
// Function Name: SIMIO_WriteCmd
//
// Description: This function writes a command to the SIM.
//
// Notes:   SIMAP use it
//
//******************************************************************************
void SIMIO_WriteCmd(SIMIO_ID_t id, UInt32 sim_lgth, UInt8 *sim_cmdPtr)
{
	SIMIO_t*     dev = &simio_dev[id];
	Boolean clk_turned_on = FALSE;
	UInt32 ssr_reg, sfcr_reg; /*Added for Debug Message*/
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_WriteCmd\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	OSSEMAPHORE_Obtain(dev->Semaphore_Memlock, TICKS_FOREVER);


	dev->T1ParityError = FALSE;
	dev->RxTxIndex = 0;
	SIMIO_ASSERT(sim_lgth <= SIMIO_SIM_BUFFER_SIZE);
	SIMIO_ASSERT(sim_cmdPtr != NULL);

	_DBG_SIM_DATA_( SIMIO_PrintBytes("SIMIO CMD: ", sim_cmdPtr, sim_lgth) );

	SLEEP_DisableDeepSleep(dev->sim_sleep_id);
	PEDESTAL_DisablePedestalMode(dev->sim_pedestal_id);

	chal_simio_read_intr_status(dev->chal_handle);
	chal_simio_mask_data_in(dev->chal_handle, FALSE);
	IRQ_Enable(dev->irq);


    if ( !dev->ATR_ParamData.is_T1_protocol )
    {
        dev->Max_WaitFor_Receive = MAX_WAIT_FOR_RECEIVE_PROTOCOL_T0;
    }

	// If the clock currently isn't on then enable it
	if ( chal_simio_is_clock_off(dev->chal_handle) )
	{
        SIMIO_Set_Clock(dev, TRUE, SIMIO_CLK_BASIC); 
		chal_simio_enable_clock(dev->chal_handle, 1);
		clk_turned_on = TRUE;
	}

	/* Two cases:
	 *
	 * 1. If we just turned on the SIM CLK, Per GSM 11.11 5.6 the interface-device must
	 *    wait "at least" 744 clock cycles before sending the first character
	 *    For our SIM CLK frequency of 3.25 MHz, this translates to a waiting time
	 *    of about 0.229 MS. Considering the 1 MS resolution of our system time,
	 *    we sleep for 2 MS before sending data to SIM.
	 *    Add one more MS to handle China Mobile SIM card, also another test SIM card shows the same behavior.
	 * 2. Do not delay for T1 SIM card, because T1 send command has its own delay
	 */
	if ( !dev->ATR_ParamData.is_T1_protocol && clk_turned_on )
	{
		OSTASK_Sleep(3);
	}

	dev->TxLength = (UInt16) sim_lgth;   // total chars to be sent

	// Calculate the number of characters expected in the response to the command. And save
	// the instruction for the response processing.
	 if (!dev->ATR_ParamData.is_T1_protocol)     // T=0 Protocol
	 {
		dev->RspLength = dev->RXExpectedByteCount ? dev->RXExpectedByteCount - 1 : 2;

		// enable workaround. only needed by T=0 command header sending case
		dev->expecting_txdone = TRUE;

		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dev->TxLength, dev->Sim_St, 0x30) );
	}
	else                                                    // T=1 Protocol
	{
		// T1 : NAD + PCB + LEN + INF(0-254) + EDC
		// T1 : [0] + [1] + [2] + [3+LEN-1] + [3+LEN]
		dev->RspLength = sim_cmdPtr[2] + 4 + dev->ATR_ParamData.atr_T1_edc;         // ATR_T1_EDC = 0 or 1
		dev->RxCount = dev->RspLength;

		_DBG_2133_( SIM_LOGV4("SIMIO CMD:(T1) ", sim_cmdPtr[2],dev->ATR_ParamData.atr_T1_edc,dev->RspLength,sim_lgth) );
		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)sim_lgth, dev->Sim_St, 0x31) );

		dev->expecting_txdone = FALSE;
	}

	// Set Tx FIFO control register. T1 cmd will be less than 5. Need to adjust dynamically. This will make ORGA USIM T=1 card works
	//SET_TXTHRE_FIFO_THRESHOLD(MIN(TxLength, 5));
	if (!dev->ATR_ParamData.is_T1_protocol)      // T=0 Protocol
	{
		chal_simio_set_tx_threshold(dev->chal_handle, MIN(dev->TxLength, T0_CMD_HEADER_LENGTH));
	}
	else
	{
		chal_simio_set_tx_threshold(dev->chal_handle, 2);
	}

	dev->Cmd_Instruction = sim_cmdPtr[1];

	// send whole buffer to the SIM buffer
	memcpy(dev->SimBuffer, sim_cmdPtr, sim_lgth);

	// change to Tx mode
	SIM_LOG("SIMIO Enable tx mode 2");

	dev->Sim_St = SIM_Write_ST;
	dev->cmd_rcvd_time = TIMER_GetValue();

	//--- start simio timer to monitor the access
	OSTIMER_Start(dev->Timer_Simio);
	START_TIMER_BWT(dev);

	if (dev->ATR_ParamData.is_T1_protocol)
		dev->T1BWT_On = TRUE;

	/*Disable SIM IRQ before calling SIMIO_PurgeSimChar*/	
	IRQ_Disable(dev->irq);
	SIMIO_PurgeSimChar(dev);
	IRQ_Clear(dev->irq);
	IRQ_Enable(dev->irq);

#ifdef SIMIO_DMA_TX
	if(ATR_ParamData.is_T1_protocol)                                                              // T=1
	{
		if(FALSE == SIMIO_Dma_Start(SimBuffer, TxLength, MEM2SIM_DIRECTION, DMA_TRIGGER_2, SIM_Read_Remain_ST, 0))
		{
			_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)TxLength, Sim_St, 0x32) );

			SIMIO_Dma_End(DMA_END_STARTFAIL);
			RxTxIndex += chal_simio_write_data(chal_handle, SimBuffer, TxLength);
			SET_TX_MODE();
		}
	}
	else                                                                                        // T=0
	{
		// Send the command header 5 bytes
		RxTxIndex += chal_simio_write_data(chal_handle, SimBuffer, TxLength);
		SET_TX_MODE();
	}
#else
	 // First send the command header due to procedure byte procedure
	 //SIM_LOGV("SIMIO_WriteCmd: FIFOCNT", (SFCR_REG & SFCR_FIFOCNT_MASK)>>26);
     IRQ_Disable(dev->irq);
	 dev->RxTxIndex += chal_simio_write_data(dev->chal_handle, dev->SimBuffer, dev->TxLength);
	 SET_TX_MODE(dev);
     IRQ_Clear(dev->irq);
     IRQ_Enable(dev->irq);
	 SIM_LOGV4("SIMIO_WriteCmd", dev->RxTxIndex, dev->TxLength, 0, 0);
#endif

     /* Added here to print SIM SSR and SIM SFCR Register states, after the write operation. */
     chal_simio_read_reg(dev->chal_handle, NULL, NULL, &ssr_reg, &sfcr_reg, NULL);
     SIM_LOGV4("SIMIO_WriteCmd: SSR,SFCR,0,0:", ssr_reg, sfcr_reg, 0, 0 );

	/* Set initial Rx FIFO threshold for 2 bytes */
	chal_simio_set_rx_threshold(dev->chal_handle, 2);
#ifdef SIMIO_CARD_DETECTION_ENABLED
    chal_simio_enable_intr(dev->chal_handle, CHAL_SIMIO_INT_RDR | dev->saved_sier_reg);
#else
    chal_simio_enable_intr(dev->chal_handle, CHAL_SIMIO_INT_RDR);
#endif
	//Start the WWT timer for T=0 sim card only
	//Keep in mind that the timer is only 16 bit, will overflow if handle etu morn that 0xFFFF.
	//Solution is simple:  do not kick out WWWT. Use the rudimentary OS timer of 1 second
	if (!dev->ATR_ParamData.is_T1_protocol && (dev->SIMIO_WWT<=0xFFFF))
	{
		SIM_LOGV("SIMIO  start timer for WWT", dev->SIMIO_WWT );

		if (dev->etu_timer.timer_type == SIMIO_TOTYPE_NONE) //ony start this when nobody else use the timer...
		{
			SIMIO_TimerStart(dev, SIMIO_TOTYPE_WWT, dev->SIMIO_WWT);
		}
	}
	// Start the hardware-based BWT timer for T=1 sim card only when BWT etu is less than 0xFFFF
	else if (dev->ATR_ParamData.is_T1_protocol && (dev->T1SIMIO_BWT<=0xFFFF))
	{
		SIM_LOGV("SIMIO  start timer for T1 BWT", dev->T1SIMIO_BWT );

		if (dev->etu_timer.timer_type == SIMIO_TOTYPE_NONE) //ony start this when nobody else use the timer...
		{
			SIMIO_TimerStart(dev, SIMIO_TOTYPE_BWT_T1, dev->T1SIMIO_BWT);
		}
	}
}

int sim_card_recovery(SIMIO_ID_t id, UInt8 *buffer)
{
    SIMIO_t* dev = &simio_dev[id];
    int size = SIM_CARD_MAX_BUFFER_SIZE;

    NO_SIM_RET(id);

    printk (KERN_EMERG "smart card err: connection with sim card got lost, sw1 = %02x, sw2 = %02x.\n", dev->ATRRxBuffer[0], dev->ATRRxBuffer[1]);
    simio_activesim(id, buffer, &size);

    if (size)
    {
        printk (KERN_EMERG "smart card err: connection with sim card was recovered. Sim card was reset.\n");
        return SIM_CARD_CONNECTION_GOT_LOST_AND_RECOVERED;
    }
    else
    {
        printk (KERN_EMERG "smart card err: connection with sim card got lost, and was not able to get recovered.\n");
        return SIM_CARD_CONNECTION_GOT_LOST_AND_DIDNT_RECOVER;
    }
}

int SIMIO_command(SIMIO_ID_t id, UInt16 val, UInt8 *buffer, int len)
{
    SIMIO_t* dev = &simio_dev[id];
    int buffer_len;
    UInt8 *ATRRxBufferptr;

    memset(dev->ATRRxBuffer, 0, sizeof(dev->ATRRxBuffer));
    dev->ATRRxCount = 0;
    if (val)
    {
        dev->RXExpectedByteCount = 0;
        buffer_len = buffer[4] + 5;
    }
    else
    {
        dev->RXExpectedByteCount = buffer[4] + 3;
        if (!buffer[4])
            dev->RXExpectedByteCount += 256;

        buffer_len = 5;
    }

    OSSEMAPHORE_Release(dev->Semaphore_Memlock);
    SIMIO_WriteCmd(id, buffer_len, buffer);

    OSTIMER_Start(dev->Timer_User);
    OSSEMAPHORE_Obtain(dev->SemaphoreUser,TICKS_FOREVER);

    OSTIMER_Stop(dev->Timer_User);

    if (!buffer)
        return SIM_CARD_DATA_IS_INVALID;

    if (!dev->ATRRxCount)
        return SIM_CARD_NO_OUTPUT_DATA_FROM_SIM_CARD;

    if (dev->ATRRxCount == 1) /* got only the ins, data is missing*/
        return (sim_card_recovery(id, buffer));
    
    if (dev->ATRRxCount == 2) /* got back only sw1, sw2*/
        ATRRxBufferptr = &dev->ATRRxBuffer[0];      
    else
    {
        dev->ATRRxCount--;
        ATRRxBufferptr = &dev->ATRRxBuffer[1];
    }

    if (len < dev->ATRRxCount)
        return SIM_CARD_INPUTER_BUFFER_IS_TOO_SHORT;

    memcpy(buffer, ATRRxBufferptr, dev->ATRRxCount);

    return dev->ATRRxCount;
}

//******************************************************************************
//
// Function Name: task_SIMIO_entry
//
// Description: This is SIMIO task entry
//
// Notes:
//
//******************************************************************************
static void task_SIMIO_entry(SIMIO_t* dev)
{
	static int  ossema_status;
	
	UInt8 num_of_reset_switch;
	UInt8 num_of_max_reset_switch;
        dprintf(DBG_LI, "SIMIO Driver: task_SIMIO_entry\n");

	_SIM_LOSS_DBG_( Simio_Init_Time = TIMER_GetValue() );

	_DBG_2133_( SIM_LOG("SIMIO Task_SIMIO_entry start") );

	while(TRUE)
	{
		// obtain semaphore first and task will be held on second obtain
		ossema_status = OSSEMAPHORE_Obtain(dev->Semaphore_Simio, TICKS_FOREVER);

		if(OSSEMAPHORE_RESULT_SUCCESS != ossema_status)
		{
			_DBG_2133_( SIM_LOGV("SIMIO Fail at Semaphore_Obtain", ossema_status) );
			SIMIO_ASSERT(FALSE);
		}
		else
		{
			_DBG_2133_( SIM_LOGV("SIMIO Simio_St = ", dev->Simio_St) );
		}

		switch(dev->Simio_St)
		{
			case SIMIO_SimReset:
				if (!dev->warmReset)
				    // Wait 20MS for voltage shut down if voltage switched
				    OSTASK_Sleep(TICKS_ONE_SECOND / 50);

				SIMIO_ProcessATR(dev, 0, FALSE);
				dev->Sim_St = SIM_ATR_ST;
				dev->PPS_Time_Out = FALSE;
				dev->ATR_Time_Out = FALSE;

				// active the SIM interface
				SIMIO_Active_SIM(dev);

				// need to change this to support dual SIM.
				num_of_max_reset_switch = (dev->recov_stat_cb == NULL ? FALSE : dev->recov_stat_cb()) ? NUM_OF_MAX_RESET_SWITCH : 1;
				num_of_reset_switch = 0;

				/* We try to switch the reset line a few times until ATR is received to maximize our chance. However switching the reset line more than once
				 * will cause us to fail GCF TC 5.1.5.6 in ETSI 102 230, so we do it only in SIM recovery mode. 
				 */
				while (num_of_reset_switch < num_of_max_reset_switch)
				{
					/* Section 5.2 of the year 1992 version of the ISO 7816-3 spec requires us to maintain RESET line at state L for at least 40000 clock cycles
					 * which is equivalent to 40000 / 3250 = 12.3 ms considering the 3.25 MHz clock frequency. but Section 5.3.2 of the year 1997 version of the ISO 
					 * 7816-3 spec requires just 400 clock cycles (0.12 ms). Here we sleep for 7 ms to take care of both sides and also to fix the problem that an Italian 
					 * INCARD SIM sends ATR within 10.6 ms after CLK signal is applied even though RESET line is still low.
					 */ 
					OSTASK_Sleep(TICKS_ONE_SECOND * 7 / 1000);

					// Check if ATR is coming
					if ( !SIMIO_ATR_Started(dev->atr_handle) )
					{
						// force RESET HIGH
						chal_simio_set_reset_level(dev->chal_handle, 1);
						SIM_LOG("SIMIO: RESET(H=1)");

						// set receiving watch timer
						SIMIO_Timer_Start(dev);

						_DBG_2133_( SIM_LOG("SIMIO: WATCH_FOR_RECEIVING_ON 0") );

						// per ISO7816-3 reset high need keep (40000/f), if SIM clock f=4M then need 10ms
						// Wait 20MS for ATR start
						OSTASK_Sleep(TICKS_ONE_SECOND / 50);
					}

					// Check again if ATR is coming
					if ( !SIMIO_ATR_Started(dev->atr_handle) )
					{
						if (++num_of_reset_switch < num_of_max_reset_switch)
						{
							SIM_LOG("SIMIO: reset low");
							chal_simio_set_reset_level(dev->chal_handle, 0);
							
							// disable time watch for receiving
							SIMIO_Timer_Reset(dev);
						}
					}
					else
					{
						break;
					}
				}

				if ( !SIMIO_ATR_Started(dev->atr_handle) )
				{
					// set ATR timeout flag
					SIM_LOG("SIMIO: set ATR timeout " );						
					dev->ATR_Time_Out = TRUE;
				}

				// max wait 2 second for ATR complete
				while(TRUE)
				{
					SIMATR_ERROR_t err;

					// check if ATR complete
					if(dev->Simio_St == SIMIO_SimInsert)
					{
						// for next reset use
						dev->RemovedSignalSent = FALSE;

						// stop watch the receiving
						_DBG_2133_( SIM_LOG("SIMIO: WATCH_FOR_RECEIVING_OFF 1") );
						WATCH_FOR_RECEIVING_OFF(dev);
						break;
					}

					err = SIMIO_ATR_Check_Error(dev->atr_handle);
					
					if(err==SIMATR_ERROR_CORRUPTED || err==SIMATR_ERROR_WRONG_VOLTAGE)
					{
						SIM_LOG("SIMIO(TASK): ATR_Corrupted/WrongVoltage Signal");

						WATCH_FOR_RECEIVING_OFF(dev);

						if( err==SIMATR_ERROR_CORRUPTED ) 
						{
							SEND_SIGNAL_ATR_CORRUPTED(dev);
						}
						else 
						{
							SEND_SIGNAL_ATR_WRONG_VOLTAGE(dev);
						}

						break;  // out of while(TRUE)-loop
					}

					//check if PPS timeout
					if (dev->PPS_Time_Out)
					{
						SIM_LOG("SIMIO: task pps tmout..." );
						WATCH_FOR_RECEIVING_OFF(dev);

						dev->PPS_Time_Out = FALSE;
						dev->etu_timer.timer_type = SIMIO_TOTYPE_NONE;
						SEND_SIGNAL_ATR_CORRUPTED(dev);

						break;
					}

					// check if time out
					if(dev->ATR_Time_Out)
					{
						SIM_LOG("SIMIO: ATR_Time_Out/Send Removal Signal");

						WATCH_FOR_RECEIVING_OFF(dev);
						SIMIO_SendSimRemovalStatus(dev);
						break;  // out of while(TRUE)-loop
					}
                                        if(dev->ATRRxSuccess)
                                        {
                                            OSSEMAPHORE_Release(dev->SemaphoreUser);                            
                                            break;
                                        }

					OSTASK_Sleep(TICKS_ONE_SECOND / 50);                    // sleep 20ms
				}

				// reset receiving watch timer
				SIMIO_Timer_Reset(dev);

				continue;

			case SIMIO_SimRemoved:                                          

				SIM_LOG("SIMIO: send removal status here 2") ;

				SIMIO_Timer_Reset(dev);

				SIMIO_SendSimRemovalStatus(dev);

				OSSEMAPHORE_Release(dev->Semaphore_Memlock);

				continue;

			case SIMIO_TimeOut:
				// report SIM removed
				SIM_LOG("SIMIO: send removal status here 3") ;
				SIMIO_SendSimRemovalStatus(dev);
				OSSEMAPHORE_Release(dev->Semaphore_Memlock);

#if defined(SIMIO_DMA_TX) || defined(SIMIO_DMA_RX)
				if(IS_DMADRV_RegisterSimioHISR() != NULL)
				{
					SIMIO_Dma_End(DMA_END_TIMEOUT);
				}
#endif
				continue;
				//break;

			case SIMIO_SimInsert:

				_DBG_2133_( SIM_LOG("SIMIO SIMIO_SimInsert state") );

				dev->Simio_St = SIMIO_Idle;

				SIM_LOG("SIMIO: SIMIO_SimInsert send inserted status");

				// Send signal indicating SIM inserted
				SEND_SIGNAL_INSERT(dev);

				break;

			case SIMIO_RspData:
#ifdef SIMIO_CARD_DETECTION_ENABLED
                simio_disable_int(dev);
#else
				IRQ_Disable(dev->irq);
#endif
				chal_simio_read_intr_status(dev->chal_handle);
				chal_simio_mask_data_in(dev->chal_handle, TRUE);


				if ((dev->etu_timer.timer_type == SIMIO_TOTYPE_WWT) && !dev->ATR_ParamData.is_T1_protocol)
				{
					SIM_LOG("SIMIO: Got rep. Stop the wwt timer");
					SIMIO_TimerStop(dev);
				}
				else if ((dev->etu_timer.timer_type == SIMIO_TOTYPE_BWT_T1) && dev->ATR_ParamData.is_T1_protocol)
				{
					SIM_LOG("SIMIO: Got rep. Stop the t1 bwt timer");
					SIMIO_TimerStop(dev);
				}

				if ( (dev->ClockStopMode == SIMIO_CLOCK_STOP_ALLOWED) || (dev->ClockStopMode == SIMIO_NO_PREFERRED_LEVEL) ||
					 (dev->ClockStopMode == SIMIO_HIGH_LEVEL_PREFERRED) || (dev->ClockStopMode == SIMIO_LOW_LEVEL_PREFERRED) ||
					 (dev->ClockStopMode == SIMIO_CLOCK_STOP_ONLY_HIGH) || (dev->ClockStopMode == SIMIO_CLOCK_STOP_ONLY_LOW) )
				{
					// Per GSM 11.11 5.6 the interface-device must wait "at least"
					// 1860 clock cycles after receiving the last character to stop the clock
					// so just shut it down. For our SIM CLK frequency of 3.25 MHz, this
					// translates to a waiting time of about 0.572 MS. Considering the 1 MS
					// resolution of our system time, we sleep for 2 MS before turnning off
					// the SIM CLK.
					//
					// GSM 11.10 test case 27.12.1 requires the clock not to be turned off
					// during the whole session: we can not turn off clock between receiving
					// status words with SW1=0x9F and sending the Get Response command.

					//do not turn off the clock when the system just start up. Wait until settle down indicated by USIM-ADF
					if (dev->fClockStopMode_Settle && !dev->Clk_Stop_Skipped)
					{
						if (((dev->SimBuffer[dev->RspLength - 2] != 0x9F) && !dev->ATR_ParamData.is_T1_protocol) || dev->ATR_ParamData.is_T1_protocol )
						{
							OSTASK_Sleep(TICKS_ONE_SECOND / 500);

							if( (dev->ClockStopMode == SIMIO_HIGH_LEVEL_PREFERRED) || (dev->ClockStopMode == SIMIO_CLOCK_STOP_ONLY_HIGH) )
							{
								chal_simio_set_clockstop_level(dev->chal_handle, 1);

								SIM_LOGV4("SIMIO stop CLK high: ", dev->irq_flood_count, dev->spurious_int_count, dev->ParityErrorCount, 0);
							}
							else
							{
								chal_simio_set_clockstop_level(dev->chal_handle, 0);

								SIM_LOGV4("SIMIO stop CLK low: ", dev->irq_flood_count, dev->spurious_int_count, dev->ParityErrorCount, 0);
							}

							chal_simio_enable_clock(dev->chal_handle, 0);
                            SIMIO_Set_Clock(dev, FALSE, SIMIO_CLK_BASIC); 

							_SIMIO_Delay(dev->id, 10);

							SLEEP_EnableDeepSleep(dev->sim_sleep_id);
							PEDESTAL_EnablePedestalMode(dev->sim_pedestal_id);
						}
						else
						{
							SIM_LOGV7("SIMIO don't stop CLk: ", dev->irq_flood_count, dev->spurious_int_count, dev->irq_flood_count, dev->spurious_int_count, dev->ParityErrorCount, 0, 0);
						}
					}
					else
					{
						SIM_LOGV7("SIMIO stop CLK skipped: ", dev->fClockStopMode_Settle, dev->Clk_Stop_Skipped, dev->irq_flood_count, dev->spurious_int_count, dev->ParityErrorCount, 0, 0);
					}
				}
				else
				{
					SIM_LOGV4("SIMIO card doesn't allow stop clock: ClockStopMode", dev->ClockStopMode, dev->irq_flood_count, dev->spurious_int_count, dev->ParityErrorCount);
				}

				dev->Simio_St = SIMIO_Idle;

				// Send data to SIMAP
				SEND_SIGNAL_RSPDATA(dev);

				OSSEMAPHORE_Release(dev->Semaphore_Memlock);

				if (dev->RXExpectedByteCount == 0)
				{                            
					dev->RXExpectedByteCount = -1;
					OSSEMAPHORE_Release(dev->SemaphoreUser);      
				}
				else if (dev->RXExpectedByteCount > 0)
				{
					if (((dev->ATRRxBuffer[0] & 0xF0) == 0x90) || 
					    ((dev->ATRRxBuffer[0] & 0xF0) == 0x60)) // check SW1
					{
						dev->RXExpectedByteCount = -1;
						OSSEMAPHORE_Release(dev->SemaphoreUser);      
					}
					else if (dev->RXExpectedByteCount == dev->ATRRxCount)
					{
						dev->RXExpectedByteCount = -1;
						OSSEMAPHORE_Release(dev->SemaphoreUser);      
					}
				}
				break;

			case SIMIO_T1Parity:
				dev->Simio_St = SIMIO_Idle;
				SIM_LOG("SIMIO T1 parity error state, call back signal send data to USIMAP...");
				dev->RspLength = 6;
				OSTASK_Sleep(50);
				SEND_SIGNAL_T1PARITY(dev);
				OSSEMAPHORE_Release(dev->Semaphore_Memlock);
				break;

			case SIMIO_T1BWTTimeout:
				dev->Simio_St = SIMIO_Idle;
				
				if (dev->etu_timer.timer_type == SIMIO_TOTYPE_BWT_T1)
				{
					SIM_LOG("SIMIO T1 BWT timeout: Stop the t1 bwt timer");
					SIMIO_TimerStop(dev);
				}
				
				SIM_LOG("SIMIO T1 BWT timeout: call back signal send data to USIMAP...");
				SEND_SIGNAL_T1_BWT_TIMEOUT(dev);
				OSSEMAPHORE_Release(dev->Semaphore_Memlock);
				break;

			case SIMIO_T1InvalidLength:
				if (dev->etu_timer.timer_type == SIMIO_TOTYPE_BWT_T1)
                {
                    SIM_LOG("SIMIO T1 invalidLength: Stop the t1 bwt timer");
                    SIMIO_TimerStop(dev);
                }
				OSTASK_Sleep(200);
				dev->T1_Length_Invalid = FALSE;
				dev->Simio_St = SIMIO_Idle;
				SIM_LOG("SIMIO T1 SIMIO_T1InvalidLength, call back signal send data to USIMAP...");
				SEND_SIGNAL_T1_INVALID_LENGTH(dev);
				OSSEMAPHORE_Release(dev->Semaphore_Memlock);
				break;

#if 0
            case SIMIO_SimDetected:
                SIM_LOG("SIMIO: send detected status") ;
                if(dev->simio_det_callback)
                {
                    dev->simio_det_callback;
                }
                break;
#endif
			default:
				dev->Simio_St = SIMIO_Idle;

				break;
		}

		// handling any task processing error here if needed
		continue;

	}  // end of task_SIMIO_entry taks while(TRUE)-loop
}

static void SIMIO_GCNTTimer_Entry(SIMIO_t* dev)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GCNTTimer_Entry\n");
	while( TRUE )
	{
		// obtain semaphore
		OSSEMAPHORE_Obtain( dev->SemaphoreGCNTTimer, TICKS_FOREVER );
		SIM_LOG("SIMIO:  Got GCNT timer sema," );

		if (dev->etu_timer.timer_type == SIMIO_TOTYPE_PPS)
		{
			dev->PPS_Time_Out = TRUE;
			WATCH_FOR_RECEIVING_OFF(dev);
			SIM_LOG("SIMIO: PPS timout, reset sim card..." );
	        SIMIO_Reset( dev, TRY_NEXT_VOLTAGE_RESET );
		}
		else if(dev->etu_timer.timer_type == SIMIO_TOTYPE_WWT)
		{
			SIM_LOG("SIMIO WWT time out" );
			_SIMIO_DeactiveCard(dev->id);      // sustain Vcc for NFC operation
		}
		else if(dev->etu_timer.timer_type == SIMIO_TOTYPE_BWT_T1)
		{
			SIM_LOG("SIMIO T1 BWT time out" );

			dev->T1BWT_On = FALSE;
			// stop the OS timer
			OSTIMER_Stop(dev->Timer_Simio);
			dev->WaitForBWTx_Counter = 0;

			dev->Sim_St = SIM_Dummy_ST;
			dev->Simio_St = SIMIO_T1BWTTimeout;

			// let task handle it
			OSSEMAPHORE_Release(dev->Semaphore_Simio);
		}
	}
}

//******************************************************************************
//
// Function Name:   SIMIO_TimeOutTimer_Entry
//
// Description: this is the entry pointer for simio timer out
//
// Notes: this function will handle the simio time out
//      Maximum delay between start leading edge is 9600 ETU
//      If f=3.25Mhz the ETU will be 372*(1/f)=114us
//      9600ETU will be 1000ms
//******************************************************************************
static void SIMIO_TimeOutTimer_Entry(SIMIO_t* dev)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_TimeOutTimer_Entry\n");
	SIM_LOG("SIMIO: _TimeOutTimer_Entry");

	//if one second time out happen, check whether it is due to T1 parity error, need to process accordingly instead of time out.
	if (dev->ATR_ParamData.is_T1_protocol && dev->T1ParityError)
	{
		SIM_LOG("SIMIO 1 second timeout for T1 parity");
		dev->Sim_St = SIM_Dummy_ST;
		dev->Simio_St = SIMIO_T1Parity;
		OSSEMAPHORE_Release(dev->Semaphore_Simio);
		return;
	}

	if(dev->T1BWT_On)                            // T=1 Protocol, Tx Timeout(BWT Timeout)
	{
		// check the counter
		SIM_LOG("SIMIO: _TimeOutTimer_Entry T1BWT_On");

		if(++dev->WaitForBWTx_Counter >= dev->Max_WaitFor_Transmit )
		{
			dev->T1BWT_On = FALSE;
			// stop the timer
			SIM_LOGV("SIMIO timout due to T1 Transmitting", dev->Max_WaitFor_Transmit);
			OSTIMER_Stop(dev->Timer_Simio);
			dev->WaitForBWTx_Counter = 0;

			dev->Sim_St = SIM_Dummy_ST;
			dev->Simio_St = SIMIO_T1BWTTimeout;

			// let task handle it
			OSSEMAPHORE_Release(dev->Semaphore_Simio);
		}

		return;
	}

	SIM_LOGV4("SIMIO: WaitForReceiving_Counter|Max_WaitFor_Receive", dev->WaitForReceiving_Counter,dev->Max_WaitFor_Receive,0,0);

	if(++dev->WaitForReceiving_Counter >= dev->Max_WaitFor_Receive)            //Max_WaitFor_Receive=CWT in T=1 case
	{
		SIM_LOG("SIMIO: max time-out reached!");

		// stop the timer
		SIM_LOG("SIMIO: stop timer");
		OSTIMER_Stop(dev->Timer_Simio);
		dev->WaitForReceiving_Counter = 0;

		if (!dev->ATR_ParamData.is_T1_protocol)                              // T=0 Protocol & Rx Timeout
		{
			SIM_LOG("SIMIO: T0 SIM time out");

			if (SIMIO_ATR_Passed(dev->atr_handle))
			//if(atr_handle->ATR_Index == SimATR_PASSED)
			{
				SIM_LOG("SIMIO: T0. 1. max time out reached; 2.max re-send reached, now have to send removal signal");

				// set time out state
				SIM_LOG("SIMIO: set time out");
				dev->Simio_St = SIMIO_TimeOut;

				// let task handle it
				OSSEMAPHORE_Release(dev->Semaphore_Simio);
			}
			else
			{
				SIM_LOG("SIMIO: _TimeOutTimer_Entry: set ATR time out");
				dev->ATR_Time_Out = TRUE;
			}
		}
		else                                                        // T=1 Protocol, Rx Timeout(CWT Timeout)
		{
			// set time out state
			SIM_LOGV("SIMIO timout due to T1 Receiving", dev->Max_WaitFor_Receive);

			dev->Simio_St = SIMIO_TimeOut;

			// let task handle it
			OSSEMAPHORE_Release(dev->Semaphore_Simio);
		}
	}
}

//******************************************************************************
//
// Function Name:   SIMIO_Active_SIM
//
// Description: this is the function which does SIM card active procedure
//
// Notes:
//          1. set RST to Low
//          2. power VCC
//          3. set I/O to reception mode
//          4. VPP should be in IDLE mode (ME should not touch it, GSM 11.11.5.3)
//          5. Enable CLK
//
//******************************************************************************
static void SIMIO_Active_SIM(SIMIO_t* dev)
{
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Active_SIM\n");
	_DBG_2133_( SIM_LOG("SIMIO: _Active_SIM") );

    IRQ_Disable(dev->irq);
    chal_simio_soft_reset(dev->chal_handle);

	// Set the index for receive buffer
	dev->RxChar_Head_Index = 0;
	dev->RxChar_Tail_Index = 0;
	dev->RxBuffer_Count = 0;

	dev->expecting_txdone = FALSE;

	chal_simio_mask_data_in(dev->chal_handle, FALSE);

    if (!dev->warmReset)
    {
#ifdef SIMIO_SIMLDO_SIMVCC_ENABLED
        simio_simldo_simvcc_on(dev, dev->Sim_Voltage);
#endif
	    // we turn on Vcc earlier
	    SIMIO_Board_Voltage_On(dev->id, dev->Sim_Voltage);
    }

	// for old SET_RX_MODE() artifact
	dev->WaitForReceiving_Counter = 0;
	dev->WaitForReceiving_On = TRUE;

    SIMIO_Set_Clock(dev, TRUE, SIMIO_CLK_BASIC);
    if (dev->warmReset)
        chal_simio_start(dev->chal_handle, TRUE);
	else
	    chal_simio_start(dev->chal_handle, FALSE);

	// enable rx ints
#ifdef SIMIO_CARD_DETECTION_ENABLED
	chal_simio_enable_intr(dev->chal_handle, CHAL_SIMIO_INT_RXTOUT | CHAL_SIMIO_INT_RXTHRE |
					   CHAL_SIMIO_INT_ROVF | CHAL_SIMIO_INT_PERR | dev->saved_sier_reg);
#else
	chal_simio_enable_intr(dev->chal_handle, CHAL_SIMIO_INT_RXTOUT | CHAL_SIMIO_INT_RXTHRE |
					   CHAL_SIMIO_INT_ROVF | CHAL_SIMIO_INT_PERR);
#endif

	// disable tx ints
	chal_simio_disable_intr(dev->chal_handle, CHAL_SIMIO_INT_TXDONE);

	// Shouldn't do this here!
	SLEEP_DisableDeepSleep(dev->sim_sleep_id);
	PEDESTAL_DisablePedestalMode(dev->sim_pedestal_id);
	IRQ_Clear(dev->irq);
	IRQ_Enable(dev->irq);
}
int SIMIO_activeCard(SIMIO_ID_t id)
{
    SIMIO_t*     dev = &simio_dev[id];
    SIMIO_Active_SIM(dev);
    return 0;
}

int SIMIO_reset_sim(SIMIO_ID_t id, UInt8 *buf, int len)
{
    SIMIO_t*     dev = &simio_dev[id];
    SIMIO_Active_SIM(dev);
    SIMIO_GetSimData(dev, buf, len);
    return 0;
}
//******************************************************************************
//
// Function Name:   SIMIO_DeactiveCard
//
// Description: this is the function which does SIM card deactive procedure
//
// Notes:   SIMAP use it. deactive procedure
//
//          1. set RST to Low
//          2. set CLK to Low
//          3. VPP inactive( should not touch it by ME, GSM 11.11.5.3 )
//          5. set IO to A( Low )
//          6. VCC inactive
//
//******************************************************************************
static void _SIMIO_DeactiveCard(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
    dprintf(DBG_LI, "SIMIO Driver: _SIMIO_DeactiveCard\n");

	SIM_LOG("SIMIO: _DEACTIVE_SIM");

#ifdef SIMIO_CARD_DETECTION_ENABLED
    simio_disable_int(dev);
#else
    IRQ_Disable(dev->irq);
#endif

	// added to solve power off problem
	dev->Simio_St = SIMIO_Idle;
	dev->Sim_St = SIM_Dummy_ST;

#if defined(SIMIO_DMA_TX) || defined(SIMIO_DMA_RX)
	DMADRV_RegisterSimioHISR(NULL);
#endif

    // Stop SIM Interface but leave the SIM CLK ON
    chal_simio_stop(dev->chal_handle, TRUE);

    // Clock stop and Vcc off functions are now in the _SIMIO_ShutdownVcc() below to support a new option "Sustain Vcc" and 
    // SIMIO_DeactiveCard() calls both functions.
}

static void _SIMIO_ShutdownVcc(SIMIO_ID_t id)
{

	SIMIO_t*     dev = &simio_dev[id];

    // Stop SIM Interface and stop the clock
    chal_simio_stop(dev->chal_handle, FALSE);

    SIMIO_Set_Clock(dev, FALSE, SIMIO_CLK_BASIC);

#ifdef SIMIO_SIMLDO_SIMVCC_ENABLED
    chal_simio_simldo_simvcc(dev->chal_handle, FALSE);
#endif

	SIMIO_Board_Voltage_Off(id);
}

void SIMIO_DeactiveCard(SIMIO_ID_t id)
{
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_DeactiveCard\n");
#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
#endif
    _SIMIO_DeactiveCard(id);
    _SIMIO_ShutdownVcc(id);
}

void SIMIO_DeactiveCard_SustainVcc(SIMIO_ID_t id)
{
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_DeactiveCard_SustainVcc\n");
#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
#endif
    _SIMIO_DeactiveCard(id);

    // No Vcc shutdown
}


//******************************************************************************
//
// Function Name:   SIMIO_Delay
//
// Description: This function makes sim i/o delay with the unit of etu
//
// Notes:
//
//******************************************************************************
static void _SIMIO_Delay(SIMIO_ID_t id, UInt8 etu)
{
	SIMIO_t*     dev = &simio_dev[id];

        dprintf(DBG_LI, "SIMIO Driver: _SIMIO_Delay\n");

	//SIM_LOGV4("FDratio|etu|0", FDratio, etu,0,0 );
	OSTASK_Sleep(dev->ATR_ParamData.FDratio);    // wait ms in term of etu unit
	/*amir - linux kernel doesnt permit division OSTASK_Sleep( ((TICKS_ONE_SECOND/1000)*(((1/3.25)*dev->ATR_ParamData.FDratio*etu)/1000)) );    // wait ms in term of etu unit*/
}
void SIMIO_Delay(SIMIO_ID_t id, UInt8 etu)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Delay\n");
#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
#endif
    _SIMIO_Delay(id, etu);
}


//******************************************************************************
//
// Function Name:   SIMIO_RegisterDetectionCB
//
// Description: This function set SIM detection callback function
//
//******************************************************************************
void SIMIO_RegisterDetectionCB(SIMIO_ID_t id, SIMIO_DET_CB_t cb)
{
    SIMIO_t* dev = &simio_dev[id];

    dev->simio_det_callback = cb;
}


//******************************************************************************
//
// Function Name:   SIMIO_RegisterCB
//
// Description: This function set callback function
//
//******************************************************************************
void SIMIO_RegisterCB(SIMIO_ID_t id, SIMIO_CB_t cb, SIMIO_Recov_Stat_CB_t recov_cb)
{
	SIMIO_t*     dev = &simio_dev[id];

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_RegisterCB\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	dev->simio_callback = cb;
	dev->recov_stat_cb = recov_cb;
}


//******************************************************************************
//
// Function Name:   SIMIO_GetPts
//
// Description: This function returns the PTS value in PPS procedure.
//
//******************************************************************************
PTS_t SIMIO_GetPts(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetPts\n");
#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	return dev->ATR_ParamData.PTS_Required;
}


//******************************************************************************
// Function Name:   SIMIO_GetATRParam
//
// Description: This function gets the key ATR parameters
//
//*****************************************************************************/
SIMIO_USIMAP_ATR_PARAM_t *SIMIO_GetATRParam(SIMIO_ID_t id)
{
	SIMIO_t*     dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetATRParam\n");
    return &dev->ATR_ParamData;    
}

//******************************************************************************
//
// Function Name:   SIMIO_SetSkipClkStop
//
// Description: This function sets the clock stop skip mode. It is used to disable
//				the clock stop during SIM initialization so that we can reduce
//				SIM Access time.
//
//******************************************************************************
void SIMIO_SetSkipClkStop(SIMIO_ID_t id,Boolean clk_stop_skipped)
{
	SIMIO_t*     dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_SetSkipClkStop\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif
	dev->Clk_Stop_Skipped = clk_stop_skipped;
}


//******************************************************************************
//
// Function Name:   SIMIO_GetCmdByte
//
// Description: This function returns the instruction byte of the 
//				current SIM command. 
//
//******************************************************************************
UInt8 SIMIO_GetCmdByte(SIMIO_ID_t id)
{
	SIMIO_t* dev = &simio_dev[id];
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_GetCmdByte\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif
	return dev->Cmd_Instruction;
}


//******************************************************************************
//
// Function Name:   SIMIO_SetParam
//
// Description: This function transfer information from USIMAP, make the final judgment for the clock stop mode
//
// Notes:
//
//******************************************************************************
void SIMIO_SetParam( SIMIO_ID_t id, UInt8 type,  UInt32 value32 )
{
	SIMIO_t*     dev = &simio_dev[id];

	SimVoltageLevel_t voltage;
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_SetParam\n");

#ifdef SIMIO_ALLOW_ID_OVERRIDE
    id = simio_override_id;
	dev = &simio_dev[id];
#endif

	SIM_LOGV4("SIMIO: SIMIO_SetParam",type, value32, 0, 0);

	switch(type)
	{
		case SIMIO_PARAM_CLOCKSTOP:         // value = Clock Stop Mode Definition

			dev->UICC_ClockStopMode = (SIMIO_CLOCK_STOP_MODE_t)value32;
			dev->ClockStopMode = SIMIO_ATR_Change_ClkStpMode(dev->atr_handle, (SIMIO_CLOCK_STOP_MODE_t)value32);

			dev->fClockStopMode_Settle = TRUE;
			SIM_LOGV("ClockStopMode: ", dev->ClockStopMode);

			break;

		case SIMIO_PARAM_VOLTAGE:
			voltage = (SimVoltageLevel_t)value32;
			if(voltage == SIM_3V || voltage == SIM_1P8V)
			{
				dev->Sim_Voltage = voltage;
			}
			else
			{
				SIM_LOGV("SIMIO: We support two voltages only: ", voltage) ;
				SIMIO_ASSERT(FALSE);
			}

			break;

		case SIMIO_PARAM_T0_WWT:
			dev->SIMIO_WWT = value32;
			SIM_LOGV("USIM_PARAM_T0_WWT: ",value32);
			break;

		default:
			break;
	}
}


/*******************************************************************************
* Function:... SIMIO_PrintBytes
*
* Description: Print the passed SIM data bytes by limiting the number of characters
*			   printed in each SIM_LOG_ARRAY() call. This is needed since SIM_LOG_ARRAY()
*			   prints up to 42 bytes of data only. 
*
*******************************************************************************/
void SIMIO_PrintBytes(const char* msg, const UInt8* buffer, UInt16 numOfBytes)
{
	UInt16 temp;

	while (numOfBytes != 0)
	{
		temp = (numOfBytes <= MAX_NUM_OF_BYTES_PER_DEBUG_LINE) ? numOfBytes : MAX_NUM_OF_BYTES_PER_DEBUG_LINE;
		SIM_LOG_ARRAY(msg, buffer, temp);

		buffer += temp;
		numOfBytes -= temp;
	}
} 



#if defined(SIMIO_DMA_TX) || defined(SIMIO_DMA_RX)

static Boolean SIMIO_Dma_Mem2sim(UInt8 *src, UInt16 *dst, UInt16 size)
{
	DMADRV_SERVICE_REQUEST_t Rq;
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Dma_Mem2sim\n");

	Rq.Type     = DMADRV_SERVICE_TYPE_MEM2SIM;
	Rq.SrcAddr  = (UInt32)src;
	Rq.DestAddr = (UInt32)dst;
	Rq.Size     = (UInt32)size;
	Rq.Notify   = SIMIO_Dma_NotifyCb;
	Rq.Repeat   = FALSE;

	if(DMADRV_Service(Rq) == DMADRV_ERROR_BUSY)
	{
		return FALSE;
	}

	return TRUE;
}

static Boolean SIMIO_Dma_Sim2mem(UInt16 *src, UInt8 *dst, UInt16 size)
{
	DMADRV_SERVICE_REQUEST_t Rq;
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Dma_Sim2mem\n");

	Rq.Type     = DMADRV_SERVICE_TYPE_SIM2MEM;
	Rq.SrcAddr  = (UInt32)src;
	Rq.DestAddr = (UInt32)dst;
	Rq.Size     = (UInt32)size;
	Rq.Notify   = SIMIO_Dma_NotifyCb;
	Rq.Repeat   = FALSE;

	if(DMADRV_Service(Rq) == DMADRV_ERROR_BUSY)
	{
		return FALSE;
	}

	return TRUE;
}

static void SIMIO_Dma_NotifyCb(DMADRV_ERROR_t Err, DMADRV_SERVICE_TYPE_t Type)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Dma_NotifyCb\n");
	if(DMADRV_CHANNEL_BUSY == Err)                                          //Error
	{
		SIMIO_Dma_End(DMA_END_NOTIFY_ERROR);
		_DBG_SIM_DATA_( SIM_LOG("SIMIO: _Dma_NotifyCb Error") );

	}

	_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)Err, Sim_St, 0x40) );
}

static Boolean SIMIO_Dma_Start(UInt8 *mem_addr, UInt16 size, UInt8 direction, UInt8 dma_position, SimDriverSt_t next_sim_st, UInt16 next_rxtxindex)
{
	Boolean res;
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Dma_Start\n");

	dma.mem_addr        = mem_addr;
	dma.size            = size;
	dma.direction       = direction;
	dma.position        = dma_position;
	dma.old_sim_st      = Sim_St;
	dma.old_rxtxindex   = RxTxIndex;

	dma.new_sim_st      = next_sim_st;
	dma.new_rxtxindex   = next_rxtxindex;
	RxTxIndex           += size;

	//DMA IRQ will treats SIMIO_HISR
	IRQ_Disable(dev->irq);

	SET_SIM_DMA_ENABLE();

	//WATCH_FOR_DMA_TX_DONE();

	DMADRV_RegisterSimioHISR(SimioHisr);

	if(MEM2SIM_DIRECTION == dma.direction)
	{
		res = SIMIO_Dma_Mem2sim(mem_addr, (UInt16 *)(SIM_BASE_REG + 0x08 + 0x03), size);
	}
	else
	{
		res = SIMIO_Dma_Sim2mem((UInt16 *)(SIM_BASE_REG + 0x08 + 0x02), mem_addr, size);
	}

	_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dma_position, Sim_St, 0x41) );
	_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)*(UInt8 *)mem_addr, Sim_St, 0x42) );
	_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)*(UInt8 *)(mem_addr+1), Sim_St, 0x43) );

	return res;
}

static void SIMIO_Dma_End(UInt8 reason)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_Dma_End\n");
	DMADRV_RegisterSimioHISR(NULL);
	SET_SIM_DMA_DISABLE();
//  WATCH_FOR_DMA_TIMER_OFF();

	if(reason == DMA_END_NORMAL)
	{
		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dma.position, Sim_St, 0x45) );

		CLEAR_TXDONE_INERRUPT();
		CLEAR_TXTHRE_INERRUPT();

		switch(dma.position) 
		{
			case DMA_TRIGGER_1:                                             //T=0 DMA(tx) after recv. INS char.
			case DMA_TRIGGER_2:                                             //T=1 DMA(tx) send T=1 command
				Sim_St      = dma.new_sim_st;
				RxTxIndex   = dma.new_rxtxindex;
				break;

			case DMA_TRIGGER_3:                                             //T=1 DMA(rx) receive response
				Sim_St      = dma.new_sim_st;
				Simio_St = SIMIO_RspData;
				//RxTxIndex indicates total received char.
				CLEAR_RXTOUT_INERRUPT();
				CLEAR_RXTHRE_INERRUPT();
				break;

			default:
				break;
		}

		IRQ_Enable(dev->irq);
	}
	else if(reason == DMA_END_NOTIFY_ERROR)
	{
		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)dma.position, Sim_St, 0x46) );

		CLEAR_TXDONE_INERRUPT();
		CLEAR_TXTHRE_INERRUPT();

		switch(dma.position) 
		{
			case DMA_TRIGGER_1:                                             //T=0 DMA(tx) after recv. INS char.
				Sim_St      = dma.old_sim_st;
				RxTxIndex   = dma.old_rxtxindex;

				IRQ_Enable(dev->irq);
				RxTxIndex += chal_simio_write_data(chal_handle, SimBuffer+RxTxIndex, TxLength - RxTxIndex);
				SET_TX_MODE();
				break;
		
			case DMA_TRIGGER_2:                                             //T=1 DMA(tx) send T=1 command
				Sim_St      = dma.old_sim_st;

				IRQ_Enable(dev->irq);
				RxTxIndex = 0;
				RxTxIndex += chal_simio_write_data(chal_handle, SimBuffer, TxLength);
				SET_TX_MODE();
				break;
			
			case DMA_TRIGGER_3:                                             //T=1 DMA(rx) receive response
				Sim_St      = dma.old_sim_st;
				RxTxIndex   = dma.old_rxtxindex;

				IRQ_Enable(dev->irq);
				SET_RX_MODE();
				break;

			default:
				break;
		}
	}
	else if((reason == DMA_END_TIMEOUT) || (reason == DMA_END_STARTFAIL))
	{
		_SIM_LOSS_DBG_( SIMIO_Log_Event((UInt16)reason, Sim_St, 0x47) );

		Sim_St      = dma.old_sim_st;
		RxTxIndex   = dma.old_rxtxindex;
		IRQ_Enable(dev->irq);
	}

	dma.old_sim_st = 0xff;
}

#endif



/****************************************************************************************************\
 * Function name        : SIMIO_TimerStart
 * Function Description : 16 bit timer start and return
 * Author(s)            :
 * Comments             : No suspended in any tasks.
 ****************************************************************************************************/

// No logging in this function: called from ISR
static void SIMIO_TimerStart(SIMIO_t* dev, UInt8 timer_type, UInt16 timer_val)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_TimerStart\n");
	chal_simio_enable_counter(dev->chal_handle, timer_val);

	dev->etu_timer.timer_type = (eSimioTimer_Type)timer_type;
	dev->etu_timer.etu = timer_val;
}

/****************************************************************************************************\
 * Function name        : SIMIO_TimerStop
 * Function Description : Stop the 16 bit timer
 * Author(s)            :
 * Comments             :
\****************************************************************************************************/
static void SIMIO_TimerStop(SIMIO_t* dev)
{
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_TimerStop\n");
	SIM_LOG("SIMIO_TimerStop called" );

	dev->etu_timer.timer_type = SIMIO_TOTYPE_NONE;
	dev->etu_timer.etu = 0;

	chal_simio_disable_counter(dev->chal_handle);
}


/****************************************************************************************************\
 *
 * Function Name: SIMIO_IsNullTimeExc
 *
 * Description: This function returns TRUE if the time has exceeded the maximum allowed
 *				for us to reset the receive timeout counter after receiving 0x60 NULL byte 
 *				from the SIM card. 
 * 
 * We need to take care of two cases:
 *
 *		 1. Some SIM's mal-function and keep sending 0x60 NULL byte indefinitely. We can not 
 *			keep resetting the receive counter otherwise SIMIO driver is dead-locked. 
 *		 
 *		 2. An Italian Vodafone SIM takes 8.3s to reply to a SMS PP Data Download
 *			Envelope command by sending 0x60 NULL byte in between to extend the phone's 
 *			wait time. 
 *
  *		 We allow a total of 60 seconds for NULL byte receiving.
 *       Note that it was 15 seconds and changed to 60 seconds per customer's request. 
 *
\****************************************************************************************************/
static Boolean SIMIO_IsNullTimeExc(SIMIO_t* dev)
{
	UInt32 timer_value = TIMER_GetValue();
        dprintf(DBG_LI, "SIMIO Driver: SIMIO_IsNullTimeExc\n");

	if (dev->cmd_rcvd_time == 0)
	{
		return FALSE;
	}
	else
	{
		UInt32 time_diff = (timer_value >= dev->cmd_rcvd_time) ? (timer_value - dev->cmd_rcvd_time) : (0xFFFFFFFF - dev->cmd_rcvd_time + timer_value); 

		return (time_diff >= (TICKS_ONE_SECOND * 60));	// Was 15 seconds
	}
}


/****************************************************************************************************\
 * Function name       : SIMIO_WarmReset
 * Function Description: Execute SIM warm reset based on 78163 5.3.3
 * Author(s)           :
 * Comments            :
\****************************************************************************************************/
void SIMIO_WarmReset(void)
{
	SIMIO_ID_t   id = SIMIO_ID_0; // Jian: move to argument
	SIMIO_t*     dev = &simio_dev[id];

        dprintf(DBG_LI, "SIMIO Driver: SIMIO_WarmReset\n");
	if( chal_simio_is_clock_off(dev->chal_handle) )
	{
        SIMIO_Set_Clock(dev, TRUE, SIMIO_CLK_BASIC); 
		chal_simio_enable_clock(dev->chal_handle, 1);
	}

	// set RST to L
	chal_simio_set_reset_level(dev->chal_handle, 0);

	//delay 10 etu
	_SIMIO_Delay(id, 10);

	// set RST to H
	chal_simio_set_reset_level(dev->chal_handle, 1);
}

/****************************************************************************************************\
 * Function name       : SIMIO_ATR_Cleanup_ById
 * Function Description: 
 * Author(s)           :
 * Comments            :
\****************************************************************************************************/
void SIMIO_ATR_Init_ById(SIMIO_ID_t id)
{
    SIMIO_t* dev = &simio_dev[id];
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_ATR_Init_ById\n");

    dev->atr_handle = SIMIO_ATR_Init(SIMIO_ID_0);
}

/****************************************************************************************************\
 * Function name       : SIMIO_ATR_Cleanup_ById
 * Function Description: 
 * Author(s)           :
 * Comments            :
\****************************************************************************************************/
void SIMIO_ATR_Cleanup_ById(SIMIO_ID_t id)
{
    SIMIO_t* dev = &simio_dev[id];
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_ATR_Cleanup_ById\n");

    SIMIO_ATR_Cleanup(dev->atr_handle);
}

//***************************************************************************
//
// Function Name: SIMIO_Get_Version
//
// Description:   return SIMIO driver version information.
//
// param:    none
//
// return:   driver version.
//
// note:  !!!!
// increase major when major driver change
// increase minor when minor driver change
// increase fix   when bug fix made
//
//***************************************************************************
UInt32 SIMIO_Get_Version(void)
{
    const UInt8 reserved = 0;
//Please maintain driver version number HERE
//===============================
    const UInt8 major = 1;
    const UInt8 minor = 0;
    const UInt8 fix   = 0;
//================================
    
    UInt32 version;
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_Get_Version\n");
    
    version = (major<<24) | (minor<<16) | (fix<<8) | reserved; 
	return version;
}

//****************************************************************************** 
// 
// Function Name: SIMIO_IsUiccClfInterfaceSupported 
// 
// Description: This function checks if UICC-CLF Interface is Supported. 
// 
// Boolean : Return   1 => UICC-CLF Supported. 
//                    0 => UICC-CLF Not Supported. 
//****************************************************************************** 
Boolean SIMIO_IsUiccClfInterfaceSupported(SIMIO_ID_t id) 
{ 
    SIMIO_t* dev = &simio_dev[id]; 
 
    dprintf(DBG_LI, "SIMIO Driver: SIMIO_IsUiccClfInterfaceSupported\n"); 
     
    // Get the information from SIMIO ATR 
    return (SIMIO_ATR_Is_UICC_CLF_Interface_Support(dev->atr_handle)); 
}

