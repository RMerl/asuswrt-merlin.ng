/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _LOOP_DETECT_CFG_H_
#define _LOOP_DETECT_CFG_H_

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

struct loop_detect_passive_cfg {
	/* port is in loop state if MAC violation counter between interval
	 * is equal to or larger than this threshold
	 * range 1~255
	 * 0 to maintain current value
	 */
	uint8_t threshold;
	/* polling interval in 100 milliseconds
	 * range 1~100 (0.1s ~ 10s)
	 * 0 to maintain current value
	 */
	uint8_t interval;
};

struct loop_detect_active_cfg {
	/* port is not in loop state if test packet is not received within
	 * this number of transmission cycles
	 * range 1~100
	 * 0 to maintain current value
	 */
	uint8_t refresh;
	/* interval in 100 milliseconds to send test packet
	 * range 1~100 (0.1s ~ 10s)
	 * 0 to maintain current value
	 */
	uint8_t interval;
};

struct loop_prevention_cfg {
	uint8_t res;
};

#pragma scalar_storage_order default
#pragma pack(pop)

/* start passive loop detection
 * default setting is applied if pcfg is NULL
 * this should be called in thread context only
 */
int loop_detect_passive_start(const GSW_Device_t *dummy,
			      struct loop_detect_passive_cfg *pcfg);
/* stop passive loop detection
 * this should be called in thread context only
 */
int loop_detect_passive_stop(const GSW_Device_t *dummy);

/* start active loop detection
 * default setting is applied if pcfg is NULL
 * this should be called in thread context only
 */
int loop_detect_active_start(const GSW_Device_t *dummy,
			     struct loop_detect_active_cfg *pcfg);
/* stop active loop detection
 * this should be called in thread context only
 */
int loop_detect_active_stop(const GSW_Device_t *dummy);

/* start loop prevention
 * default setting is applied if pcfg is NULL
 * this should be called in thread context only
 */
int loop_prevention_start(const GSW_Device_t *dummy,
			  struct loop_prevention_cfg *pcfg);
/* stop loop prevention
 * this should be called in thread context only
 */
int loop_prevention_stop(const GSW_Device_t *dummy);

#endif /* _LOOP_DETECT_CFG_H_ */
