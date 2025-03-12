
#include <stddef.h>
#include "hal_chipid.h"

chip_info_t g_wsp_info;

typedef enum logic_port_id {
	LPID0	= 0,
	LPID1	= APP_PID1,
	LPID2	= APP_PID2,
	LPID3	= APP_PID3,
	LPID4	= APP_PID4,
	LPID5	= APP_PID5,
	LPID6	= APP_PID6,
	LPID7	= APP_PID7,
	LPID8	= APP_PID8,
	LPID9	= APP_PID9,
	LPID10	= APP_PID10,
	LPID11	= APP_PID11,
	LPID12	= APP_PID12,
	LPID13	= APP_PID13,
	LPID14	= APP_PID14,
	LPID15	= APP_PID15,
	LPID16	= APP_PID16,
	LPID_NUM
} logic_port_id_t;

typedef enum f48x_port_id {
	F48X_PORT_0_CPU		= LPID0,
	F48X_PORT_1_GMAC0	= LPID1,
	F48X_PORT_2_GMAC1	= LPID2,
	F48X_PORT_3_GMAC2	= LPID3,
	F48X_PORT_4_GMAC3	= LPID4,
	F48X_PORT_5_GMAC4	= LPID5,
	F48X_PORT_6_GMAC5	= LPID6,
	F48X_PORT_7_GMAC6	= LPID7,
	F48X_PORT_8_GMAC7	= LPID8,
	F48X_PORT_9_XGMAC0	= LPID9,
	F48X_PORT_10_XGMAC1	= LPID10,
	F48X_PORT_11_XGMAC2	= LPID11,
	F48X_PORT_12_XGMAC3	= LPID12,
	F48X_PORT_13_XGMAC4	= LPID13,
	F48X_PORT_14_XGMAC5	= LPID14,
	F48X_PORT_15_XGMAC6	= LPID15,
	F48X_PORT_16_XGMAC7	= LPID16,
	F48X_PORT_NUM
} f48x_port_id_t;

typedef enum f48x_slice_id {
	F48X_SLICE0	= 0,
	F48X_SLICE1	= 1,
	F48X_SLICE2	= 2,
	F48X_SLICE3	= 3,
	F48X_SLICE4	= 4,
	F48X_SLICE5	= 5,
	F48X_SLICE6	= 6,
	F48X_SLICE7	= 7,
	F48X_SLICE_NUM
} f48x_slice_id_t;

typedef enum f48x_serdes_id {
	F48X_SERDES0	= 0,
	F48X_SERDES1	= 1,
	F48X_SERDES_MAX
} f48x_serdes_id_t;

typedef enum serdes_disabled {
	SERDES_ENABLED	 = 0,
	SERDES_DISABLED	 = 1
} serdes_disabled_t;

typedef enum port_num {
	MODE_8_PLUS_2_PORT_NUM_10 = 11,
	MODE_5_PLUS_2_PORT_NUM_7  = 8,
} port_num_t;

const uint8_t g_lpid_mac_port_mapping_8_plus_2[MODE_8_PLUS_2_PORT_NUM_10] __attribute__((section(".text_array")))
= {
	[APP_PID0] = F48X_PORT_0_CPU,
	[APP_PID1] = F48X_PORT_1_GMAC0,
	[APP_PID2] = F48X_PORT_2_GMAC1,
	[APP_PID3] = F48X_PORT_3_GMAC2,
	[APP_PID4] = F48X_PORT_4_GMAC3,
	[APP_PID5] = F48X_PORT_5_GMAC4,
	[APP_PID6] = F48X_PORT_6_GMAC5,
	[APP_PID7] = F48X_PORT_7_GMAC6,
	[APP_PID8] = F48X_PORT_8_GMAC7,
	[APP_PID9] = F48X_PORT_9_XGMAC0,
	[APP_PID10] = F48X_PORT_13_XGMAC4
};

const uint8_t g_lpid_mac_port_mapping_5_plus_2[MODE_5_PLUS_2_PORT_NUM_7] __attribute__((section(".text_array")))
= {
	[APP_PID0] = F48X_PORT_0_CPU,
	[APP_PID1] = F48X_PORT_1_GMAC0,
	[APP_PID2] = F48X_PORT_2_GMAC1,
	[APP_PID3] = F48X_PORT_3_GMAC2,
	[APP_PID4] = F48X_PORT_4_GMAC3,
	[APP_PID5] = F48X_PORT_5_GMAC4,
	[APP_PID6] = F48X_PORT_9_XGMAC0,
	[APP_PID7] = F48X_PORT_13_XGMAC4
};

struct chip_info get_chip_info(void)
{
	static bool s_init_flg = false;

	if (!s_init_flg) {
		g_wsp_info.port_mode = PORT_MODE_5_PLUS_2;	//PORT_MODE_8_PLUS_2
		g_wsp_info.port_num = 7;					//10
		s_init_flg = true;
	}

	return g_wsp_info;
}

uint8_t get_chip_port_mode(void)
{
	get_chip_info();

	return g_wsp_info.port_mode;
}

uint8_t get_chip_port_num(void)
{
	get_chip_info();

	return g_wsp_info.port_num;
}

uint8_t map_port_idx(uint8_t app_port_idx)
{
	uint8_t drv_port_idx = 0;
	uint8_t port_mode = PORT_MODE_8_PLUS_2;

	port_mode = get_chip_port_mode();

	switch (port_mode) {
	case PORT_MODE_8_PLUS_2:
		drv_port_idx = g_lpid_mac_port_mapping_8_plus_2[app_port_idx % MODE_8_PLUS_2_PORT_NUM_10];
		break;
	case PORT_MODE_5_PLUS_2:
		drv_port_idx = g_lpid_mac_port_mapping_5_plus_2[app_port_idx % MODE_5_PLUS_2_PORT_NUM_7];
		break;
	default:
		drv_port_idx = 0;
		break;
	}

	return drv_port_idx;
}

uint8_t map_drv_port_idx(uint8_t drv_port_idx)
{
	uint8_t port_mode = get_chip_port_mode();
	const uint8_t *tbl;
	size_t max;

	switch (port_mode) {
	case PORT_MODE_8_PLUS_2:
		tbl = g_lpid_mac_port_mapping_8_plus_2;
		max = ARRAY_SIZE(g_lpid_mac_port_mapping_8_plus_2);
		break;
	case PORT_MODE_5_PLUS_2:
		tbl = g_lpid_mac_port_mapping_5_plus_2;
		max = ARRAY_SIZE(g_lpid_mac_port_mapping_5_plus_2);
		break;
	default:
		return 0;
	}

	for (size_t i = 0; i < max; i++) {
		if (tbl[i] == drv_port_idx)
			return i;
	}

	return 0;
}

uint8_t get_ctp_info(uint8_t app_port_idx, ctp_info_t *p_ctp_port)
{
	uint8_t port_mode = PORT_MODE_8_PLUS_2;

	if (p_ctp_port == NULL)
		return 1;

	if (app_port_idx == APP_PID16) {
		p_ctp_port->sub_if_id_group = 16;
		p_ctp_port->logical_port_id = 0;
	} else {
		p_ctp_port->sub_if_id_group = 0;

		port_mode = get_chip_port_mode();
		switch (port_mode) {
		case PORT_MODE_8_PLUS_2:
			if (app_port_idx > APP_PID10) {
				LOG_ERR("Incorrect app port index: %u/%u", app_port_idx, APP_PID10);
			}
			p_ctp_port->logical_port_id = app_port_idx <= APP_PID9 ? app_port_idx : APP_PID13;
			break;
		case PORT_MODE_5_PLUS_2:
			if (app_port_idx > APP_PID7) {
				LOG_ERR("Incorrect app port index: %u/%u", app_port_idx, APP_PID7);
			}
			if (app_port_idx <= APP_PID5)
				p_ctp_port->logical_port_id = app_port_idx;
			else if (app_port_idx == APP_PID6)
				p_ctp_port->logical_port_id = APP_PID9;
			else if (app_port_idx == APP_PID7)
				p_ctp_port->logical_port_id = APP_PID13;
			break;
		}
	}

	return 0;
}

const uint8_t *get_chip_lpid_array(uint8_t *size)
{
	if (!size)
		return NULL;

	*size = 0;

	switch (get_chip_port_mode()) {
	case PORT_MODE_8_PLUS_2:
		*size = ARRAY_SIZE(g_lpid_mac_port_mapping_8_plus_2);
		return g_lpid_mac_port_mapping_8_plus_2;
	case PORT_MODE_5_PLUS_2:
		*size = ARRAY_SIZE(g_lpid_mac_port_mapping_5_plus_2);
		return g_lpid_mac_port_mapping_5_plus_2;
	default:
		return NULL;
	}

	return NULL;
}
