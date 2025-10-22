#ifndef __POE_H__
#define __POE_H__

#define POE_PRIO_LOW            0
#define POE_PRIO_HIGH           1
#define POE_PRIO_CRITICAL       2

#ifdef EBG19P
#define POEMON_PORTS            8
#define MAX_TOTAL_POWER        	125	/* 15.4 * 8 = 123.2 (< 150 - 18), +2 tolerance */
#define ALERT_LED_ON            108
#define ALERT_LED_OFF           105
#define POWER_RECOVER           84
//#define MAX_PORT_POWER      	30
#define ICUT_ADJ		25
#define TOTAL_ADJ		15

#else
#define POEMON_PORTS            4
#define MAX_TOTAL_POWER        	53
#define ALERT_LED_ON            48
#define ALERT_LED_OFF           44
#define POWER_RECOVER           27      // half max
//#define MAX_PORT_POWER      	30
#endif

enum {
	POE_CAP_NONE,
	POE_CAP_8023_AF,
	POE_CAP_8023_AT,
	POE_CAP_8023_BT,
	POE_CAP_MAX,
};

#endif /*__POE_H__*/
