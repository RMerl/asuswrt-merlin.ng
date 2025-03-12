/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

#ifndef LIF_API_H
#define LIF_API_H

int32_t lif_mdio_init(char *lib);
int32_t lif_mdio_deinit(char *lib);
int32_t lif_mdio_open(char *lib, uint8_t clk_pin, uint8_t data_pin);
int32_t lif_mdio_close(char *lib, uint8_t clk_pin, uint8_t data_pin);
int32_t lif_mdio_c22_read(uint8_t lif_id, uint8_t pad, uint8_t dad);
int32_t lif_mdio_c22_write(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t val);
int32_t lif_mdio_c45_read(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t reg);
int32_t lif_mdio_c45_write(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t reg, uint16_t val);
int32_t lif_scan(char *lib);
int32_t lif_get_cpin(uint8_t lif_id);
int32_t lif_get_dpin(uint8_t lif_id);
int32_t lif_get_nr_phys(uint8_t lif_id);
int32_t lif_get_phy_addr(uint8_t lif_id, uint8_t phy);
int32_t lif_get_phy_id(uint8_t lif_id, uint8_t phy);
int32_t lif_get_nr_lif(void);

/*
return codes
*/

#define LIF_API_RET_SUCCESS          0      /* No error               : Successful operation */
#define LIF_API_RET_LIB_ERROR       -1      /* Library error          : Invalid library operation: wrong library selection, library not or already initialized. */
#define LIF_API_RET_PINS_ERROR      -2      /* Pins validity error    : Pins not allowed or (clk , data) pins are identical */
#define LIF_API_RET_OPEN_ERROR      -3      /* Pins open error        : Invalid Pins open operation: open operation already done. */
#define LIF_API_RET_CLOSE_ERROR     -4      /* Pins Close error       : Invalid Pins close operation: close operation already done or open operation missing.	 */
#define LIF_API_RET_ACCESS_ERROR    -5      /* Pins RW Access error    : Pins are not available. */
#define LIF_API_RET_LCOMM_ERROR     -6      /* Link communication Error:Mdio communication error, cannot be determined, will never occurs */
#define LIF_API_RET_FAPI_PRM_ERROR  -7      /* Parameter Error         : FAPI Parameter error  */

/*
RPI Information
*/

#define PHY_MAX_VAL 32
#define COMB_MAX_VAL 110

#define LIF_NO_SCAN
#undef LIF_NO_SCAN

#ifdef LIF_NO_SCAN
#define MAX_LINKS 1
#define RASP_AVAILABLE_PINS 2

#define LIF_CLK_PIN 5
#define LIF_DATA_PIN 6

#else
#define MAX_LINKS 5
#define RASP_AVAILABLE_PINS 11
#endif

/* transfered to lif_api.c
   TO-DO: rework of the lif scan handling */
#if 0
static int32_t GPIO_PINS[RASP_AVAILABLE_PINS] = {4, 5, 6, 12, 13, 22, 23, 24, 25, 26, 27};
#endif

#endif /* LIF_API_H */
