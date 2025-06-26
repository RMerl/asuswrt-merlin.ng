#ifndef __HAL_CHIPID_H__
#define __HAL_CHIPID_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif
#define ARG_UNUSED(x) (void)(x)

#ifndef BIT
#define BIT(n)  (1UL << (n))
#endif

/** @brief 64-bit unsigned integer with bit position @p _n set. */
#define BIT64(_n) (1ULL << (_n))

#ifndef _LOG_DEBUG_H_
#define _LOG_DEBUG_H_
#define LOG_ERR printf
#define LOG_INF printf
#define LOG_DBG printf
#endif

/* MAC Index used by modules */
typedef enum {
	PMAC_0 = 0,
	MAC_1,
	MAC_2,
	MAC_3,
	MAC_4,
	MAC_5,
	MAC_6,
	MAC_7,
	MAC_8,
	MAC_9,
	MAC_10,
	MAC_11,
	MAC_12,
	MAC_13,
	MAC_14,
	MAC_15,
	MAC_16,
	MAC_LAST,
} MAC_IDX;

typedef enum app_port_id {
	APP_PID0 = 0,
	APP_PID1,
	APP_PID2,
	APP_PID3,
	APP_PID4,
	APP_PID5,
	APP_PID6,
	APP_PID7,
	APP_PID8,
	APP_PID9,
	APP_PID10,
	APP_PID11,
	APP_PID12,
	APP_PID13,
	APP_PID14,
	APP_PID15,
	APP_PID16,
	APP_PID_NUM = APP_PID16
} app_port_id_t;

typedef enum chip_port_mode {
	PORT_MODE_8_PLUS_2	= 0, // Default mode, both two XPCS are enabled in MAC mode,
	PORT_MODE_5_PLUS_2	= 1, // Both XPCS are enabled, PM slice 0~4 are enabled.
	PORT_MODE_NUM
} chip_port_mode_t;

/* 16-port cascade mode */
typedef enum cascade_type {
	CT_SINGLE	 = 0,
	CT_CASCADED	 = 1
} cascade_type_t;

typedef enum sw_phy_mode_type {
	PHY_MODE    = 0,
	SWITCH_MODE = 1,
} sw_phy_mode_type_t;

#pragma pack(push, 1)

typedef struct ctp_info {
	uint8_t sub_if_id_group;
	uint8_t logical_port_id;
} ctp_info_t;

typedef struct chip_info {
	uint8_t port_mode;	// value defined in chip_port_mode_t
	uint8_t port_num;
	uint16_t valid_ports;	/* each bit indicate valid physical port
				 * bit 0 is MAC 1 ... bit 15 is XGMAC 16
				 */
} chip_info_t;

#pragma pack(pop)


/*****************************************************************
 * Get all the chip information
 * return value:
 *	struct chip_info, include all the chip basic information,
 *	include the chip id, location info, port mode
 *	and switch or PHY mode, etc.
 *****************************************************************/
struct chip_info get_chip_info(void);

/*****************************************************************
 * Get the chip port mode.
 * return value:
 *	PORT_MODE_8_PLUS_2 = 0:
 *		Default mode, both two XPCS/SerDes are enabled,
 *		chip is in MAC mode, all slices are enabled.
 *	PORT_MODE_5_PLUS_2 = 1:
 *		Both two XPCS/SerDes are enabled,
 *		slice 0~4 are enabled, slice 5~7 are disabled.
 *****************************************************************/
uint8_t get_chip_port_mode(void);

/*****************************************************************
 * Get the chip port number.
 * return value:
 *	10: for PORT_MODE_8_PLUS_2 mode. Default mode
 *	 7: for PORT_MODE_5_PLUS_2 mode.
 *****************************************************************/
uint8_t get_chip_port_num(void);

/*****************************************************************
 * Map the application port index to driver index
 *     Different port mode, the mapping is different.
 * para: app_port_idx, input, range [1, 16]
 * return: driver F48X_PORT index.
 *****************************************************************/
uint8_t map_port_idx(uint8_t app_port_idx);

/*****************************************************************
 * Map the driver port index to application port index
 *     Different port mode, the mapping is different.
 * para: drv_port_idx, input, range [0, 16]
 * return: application app_port_id_t index.
 *****************************************************************/
uint8_t map_drv_port_idx(uint8_t drv_port_idx);

/*****************************************************************
 * get the ctp port info according to the given application port index.
 * para: app_port_idx, input, range [1, 16],
 *       p_ctp_port, input/output, the app_port_idx related ctp port info
 * return: 0: success; non-0: fail.
 *****************************************************************/
uint8_t get_ctp_info(uint8_t app_port_idx, ctp_info_t *p_ctp_port);

/*****************************************************************
 * get the array of LPID based on the portmode
 * return: array based on the chip port mode and its size
 *****************************************************************/
const uint8_t *get_chip_lpid_array(uint8_t *size);

const char *get_chip_part_num(void);

#endif
