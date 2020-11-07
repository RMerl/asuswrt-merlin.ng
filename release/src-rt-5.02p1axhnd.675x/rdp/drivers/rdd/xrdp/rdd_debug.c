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

#if defined(DEBUG_PRINTS)

#include "rdd.h"
#include "rdd_debug.h"
#include "rdp_drv_cntr.h"
#include "rdd_debug_strings.h"
#include "rdp_mm.h"

static const char **rdd_debug_all_runners_strings[16];
static int          rdd_debug_all_runners_n_strings[16];

struct debug_prints_info rdd_debug_prints_info;

#ifndef RUNNER0_DEBUG_STRING
const char * runner0_debug_strings[] = {};
#endif
#ifndef RUNNER1_DEBUG_STRING
const char * runner1_debug_strings[] = {};
#endif
#ifndef RUNNER2_DEBUG_STRING
const char * runner2_debug_strings[] = {};
#endif
#ifndef RUNNER3_DEBUG_STRING
const char * runner3_debug_strings[] = {};
#endif
#ifndef RUNNER4_DEBUG_STRING
const char * runner4_debug_strings[] = {};
#endif
#ifndef RUNNER5_DEBUG_STRING
const char * runner5_debug_strings[] = {};
#endif
#ifndef RUNNER6_DEBUG_STRING
const char * runner6_debug_strings[] = {};
#endif


/* Initializing strings arrays - up to 6 images supported */
void rdd_debug_init_runners_strings(void)
{
  rdd_debug_all_runners_strings[0] = runner0_debug_strings;
  rdd_debug_all_runners_n_strings[0] = (int)sizeof(runner0_debug_strings)/sizeof(runner0_debug_strings[0]);
  rdd_debug_all_runners_strings[1] = runner1_debug_strings;
  rdd_debug_all_runners_n_strings[1] = (int)sizeof(runner1_debug_strings)/sizeof(runner1_debug_strings[0]);
  rdd_debug_all_runners_strings[2] = runner2_debug_strings;
  rdd_debug_all_runners_n_strings[2] = (int)sizeof(runner2_debug_strings)/sizeof(runner2_debug_strings[0]);
  rdd_debug_all_runners_strings[3] = runner3_debug_strings;
  rdd_debug_all_runners_n_strings[3] = (int)sizeof(runner3_debug_strings)/sizeof(runner3_debug_strings[0]);
  rdd_debug_all_runners_strings[4] = runner4_debug_strings;
  rdd_debug_all_runners_n_strings[4] = (int)sizeof(runner4_debug_strings)/sizeof(runner4_debug_strings[0]);
  rdd_debug_all_runners_strings[5] = runner5_debug_strings;
  rdd_debug_all_runners_n_strings[5] = (int)sizeof(runner5_debug_strings)/sizeof(runner5_debug_strings[0]);
  rdd_debug_all_runners_strings[6] = runner6_debug_strings;
  rdd_debug_all_runners_n_strings[6] = (int)sizeof(runner6_debug_strings)/sizeof(runner6_debug_strings[0]);
}

static void rdd_debug_prints_runner_init(uint32_t addr_hi, uint32_t addr_lo, uint32_t length)
{
    rdd_debug_prints_info.debug_print_buf_len = length*sizeof(uint32_t);
    rdd_debug_prints_info.double_buf_num = 0;
    rdd_debug_prints_info.priority = 0;
    rdd_debug_prints_info.num_of_messages_in_period = -1;
    rdd_debug_prints_info.perodicity_ms = 100;
        
    RDD_DEBUG_PRINT_INFO_TABLE_ID_WRITE_G(0, RDD_DEBUG_PRINT_TABLE_ADDRESS_ARR, 0);
    RDD_DEBUG_PRINT_INFO_ADDR_LOW_WRITE_G(addr_lo, RDD_DEBUG_PRINT_TABLE_ADDRESS_ARR, 0);
    RDD_DEBUG_PRINT_INFO_ADDR_HIGH_WRITE_G((addr_hi & 0xff), RDD_DEBUG_PRINT_TABLE_ADDRESS_ARR, 0);
    RDD_DEBUG_PRINT_INFO_SIZE_WRITE_G(length, RDD_DEBUG_PRINT_TABLE_ADDRESS_ARR, 0);
    RDD_BYTE_1_BITS_WRITE_G(0, RDD_DEBUG_PRINT_CORE_LOCK_ADDRESS_ARR, 0);
    
    /* reset wr_ptr  */
    drv_cntr_counter_clr(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_WRITE_PTR_0_DEBUG_PRINT);
    drv_cntr_counter_clr(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_WRITE_PTR_1_DEBUG_PRINT);

    rdd_debug_init_runners_strings();
}

void rdd_debug_prints_init(void)
{
    bdmf_phys_addr_t phy_addr;
    void* debug_prints_buffer_base;
    uint32_t addr_hi, addr_lo;
    
    /*allocate double buffer for debug prints - must be non-cacheable memory*/
    debug_prints_buffer_base = (void *)rdp_mm_aligned_alloc(DEBUG_PRINT_TABLE_SIZE*2, &phy_addr);
    
    rdd_debug_prints_info.debug_print_buf = debug_prints_buffer_base;
    
    if (debug_prints_buffer_base == NULL)
    {
        bdmf_trace("failed to allocate memory for debug prints buffer\n");
        rdd_debug_prints_runner_init(0, 0, 0);
    }
    else
    {
        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phy_addr);
        /* buffer size in fw in words */
        rdd_debug_prints_runner_init(addr_hi, addr_lo, DEBUG_PRINT_TABLE_SIZE/4);
        bdmf_trace("Done initializing debug prints double buffer Base=%p Size= %d RDD Base=0x%lx\n",
          debug_prints_buffer_base, DEBUG_PRINT_TABLE_SIZE*2, (unsigned long)phy_addr);
    }
}

void rdd_debug_prints_update_params(int32_t max_prints_per_period, uint32_t period, uint32_t priority)
{
    rdd_debug_prints_info.num_of_messages_in_period = max_prints_per_period;
    rdd_debug_prints_info.perodicity_ms = period;
    rdd_debug_prints_info.priority = priority;
}

const char *rdd_debug_get_debug_string(int image_id, int ind, uint32_t params_num)
{
	static char undef_print[300];
	char tmp[30];
	uint32_t i;
	if( ind >= rdd_debug_all_runners_n_strings[image_id] )
	{
		strcpy( undef_print, "Undefined PRINT: " );
		for( i = 0; i < params_num; i++ )
		{
			sprintf( tmp, "param%d: 0x%%08X ", i );
			strcat( undef_print, tmp );
		}
		strcat( undef_print, "\n" );
		return undef_print;
	}
	return rdd_debug_all_runners_strings[image_id][ind];
}

char runner_debug_string_pre[] = "PRINTR: core_id=%d, task_id=%d || ";

void rdd_debug_prints_handle()
{
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    uint32_t *buf_start_addr, *buf_end_addr;
    uint32_t  mark, prio, msg_len_words, core_id, task_id, string_id, val, write_idx, i;
    uint32_t params[8], params_num, msg_num;
    char runner_debug_string_format[512];
  
    if (rdd_debug_prints_info.debug_print_buf == NULL)
      return;
            
    /* Update write index from counter */
    drv_cntr_counter_read(CNTR_GROUP_DHD_CTR, (DHD_CTR_GROUP_WRITE_PTR_0_DEBUG_PRINT+rdd_debug_prints_info.double_buf_num), cntr_arr);
    write_idx = cntr_arr[0];
    
    if (write_idx==0)
      return;
      
    // toggle buffer first
    RDD_DEBUG_PRINT_INFO_TABLE_ID_WRITE_G(1-rdd_debug_prints_info.double_buf_num, RDD_DEBUG_PRINT_TABLE_ADDRESS_ARR, 0);

    buf_start_addr = (uint32_t*)(((uintptr_t)rdd_debug_prints_info.debug_print_buf) +
             rdd_debug_prints_info.debug_print_buf_len * (rdd_debug_prints_info.double_buf_num));
             
    buf_end_addr = (uint32_t*)(((uintptr_t)buf_start_addr) + (write_idx*sizeof(uint32_t)));
    
    /* reset wr_ptr  */
    drv_cntr_counter_clr(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_WRITE_PTR_0_DEBUG_PRINT+rdd_debug_prints_info.double_buf_num);
    

    //bdmf_print("print info: buf_num:%d, write_idx:%d, buf_start_addr:%p, buf_end_addr:%p\n", 
    //            rdd_debug_prints_info.double_buf_num, write_idx, (void*)buf_start_addr, (void*)buf_end_addr);
              
    // switch internally
    rdd_debug_prints_info.double_buf_num = 1 - rdd_debug_prints_info.double_buf_num;

    msg_num = 0;
    
    while (buf_start_addr < buf_end_addr)
    {
      
      /* Limit displayed number of prints */
      if ((rdd_debug_prints_info.num_of_messages_in_period != -1) && (msg_num >= rdd_debug_prints_info.num_of_messages_in_period))
        return;
     
      val = __swap4bytes(*buf_start_addr++);
      
      mark = RDD_DEBUG_PRINT_HEADER_PRINT_MARK_L_READ(val);
      prio = RDD_DEBUG_PRINT_HEADER_PRINT_PRIO_L_READ(val);
      msg_len_words = RDD_DEBUG_PRINT_HEADER_MSG_LEN_L_READ(val);
      core_id= RDD_DEBUG_PRINT_HEADER_CORE_ID_L_READ(val);
      task_id = RDD_DEBUG_PRINT_HEADER_TASK_ID_L_READ(val);
      string_id = RDD_DEBUG_PRINT_HEADER_STRING_ID_L_READ(val);
      
      params_num = (msg_len_words <= 8) ? msg_len_words-1 : 7;
      if (mark != 0xAB)
      {
        bdmf_trace("corrupted runner prints ...\n");
        return;
      }

      for (i = 0; i < params_num; i++)
      {
        params[i] = __swap4bytes(*buf_start_addr++);
      }
      
      if (prio < rdd_debug_prints_info.priority)
        continue;
        
      msg_num++;
      
      
      strcpy(runner_debug_string_format, runner_debug_string_pre);
      strcat(runner_debug_string_format, rdd_debug_get_debug_string(rdp_core_to_image_map[core_id], string_id, params_num));
      
      switch(params_num) 
      {

       case 0  :
          bdmf_trace(runner_debug_string_format, core_id, task_id);
          break; 

       case 1  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[0]);
          break; 
       
       case 2  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[1], params[0]);
          break;
       
       case 3  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[2], params[1], params[0]);
          break;
       
       case 4  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[3], params[2], params[1], params[0]);
          break;
       
       case 5  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[4], params[3], params[2], params[1], params[0]);
          break;
       
       case 6  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[5], params[4], params[3], params[2], params[1], params[0]);
          break;
       
       case 7  :
          bdmf_trace(runner_debug_string_format,
                        core_id, task_id, params[6], params[5], params[4], params[3], params[2], params[1], params[0]);
          break;
       default : 
          break;
      }
      
    }
}

#endif
