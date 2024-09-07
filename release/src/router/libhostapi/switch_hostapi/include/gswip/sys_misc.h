/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _SYS_MISC_H_
#define _SYS_MISC_H_

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

struct sys_fw_image_version {
	uint8_t major;
	uint8_t minor;
	uint16_t revision;
	uint32_t app_revision;
};

struct sys_delay {
	/* m_sec unit is 1ms, the accuracy is 10ms. */
	uint32_t m_sec;
};

struct sys_gpio_config {
	uint16_t enable_mask[3];
	uint16_t alt_sel_0[3];
	uint16_t alt_sel_1[3];
	uint16_t dir[3];
	uint16_t out_val[3];
	/**
	 * reserve 1 - reserved for open drain in future
	 * reserve 2 - reserved for pull up enable in future
	 * reserve 3 - reserved for pull up/down in future
	**/
	uint16_t reserve_1[3];
	uint16_t reserve_2[3];
	uint16_t reserve_3[3];
	/**
	 * unit is 1ms, the accuracy is 10ms.
	**/
	uint32_t timeout_val;
};

/**
 * @brief Representation of a sensor readout value.
 *
 * The value is represented as having an integer and a fractional part,
 * and can be obtained using the formula val1 + val2 * 10^(-6). Negative
 * values also adhere to the above formula, but may need special attention.
 * Here are some examples of the value representation:
 *
 *      0.5: val1 =  0, val2 =  500000
 *     -0.5: val1 =  0, val2 = -500000
 *     -1.0: val1 = -1, val2 =  0
 *     -1.5: val1 = -1, val2 = -500000
 */
struct sys_sensor_value {
	/** Integer part of the value. */
	int32_t val1;
	/** Fractional part of the value (in one-millionth parts). */
	int32_t val2;
};

/**
 * @brief Register read/write data structure
 */
struct sys_reg_rw {
	/** 32-bit register address */
	uint32_t addr;
	/** register value */
	uint32_t val;
};

/**
 * @brief Register read/write data structure
 */
struct sys_reg_mod {
	/** 32-bit register address */
	uint32_t addr;
	/** register value to write */
	uint32_t val;
	/** register value mask, bit value 1 to write, 0 to ignore */
	uint32_t mask;
};

enum {
	SYS_CML_CLK_50MHZ = 0,
	SYS_CML_CLK_156P25MHz = 1,
	SYS_CML_CLK_XO = 2,
	SYS_CML_CLK_SRC_MAX
};

/**
 * @brief Enable/Disable/Config CML Clock Output
 */
struct sys_cml_clk {
	/** select CML Clock Output (0 or 1) */
	uint32_t clk:		1;
	/** value 1 to enable or disable CML Clock Output */
	uint32_t en_val:	1;
	/** value 1 to enable CML Clock Output when en_val is 1 */
	uint32_t en:		1;
	/** value 1 to change clock source selection */
	uint32_t src_val:	1;
	/** select clock source when src_val is 1
	 *     0 - select LJPLL FOUT0 (50MHz)
	 *     1 - select LJPLL FOUT1 (156.25MHz)
	 *     2 - select XO (25MHz)
	 */
	uint32_t src_sel:	2;
};

/**
 * @brief Config SFP
 */
struct sys_sfp_cfg {
	/** Port id (0 or 1) */
	uint8_t	port_id: 4;
	/** config options
	 *    0 - SFP mode/speed/link-status
	 *    1 - flow control
	 */
	uint8_t option: 4;
	union {
		struct {
			/** select SFP mode
			 *	0 - auto
			 *	1 - fix
			 *	2 - disable
			 */
			uint8_t mode;
			/** select speed when mode is 1
			 *	0 - 10G Quad USXGMII
			 *	1 - 1000BaseX ANeg
			 *	2 - 10G	XFI
			 *	3 - 10G Single USXGMII
			 *	4 - 2.5G SGMII
			 *	5 - 2500 Single USXGMI
			 *	6 - 2500BaseX NonANeg
			 *	7 - 1000BaseX NonANeg
			 *	8 - 1G SGMI
			 */
			uint8_t speed;
			/* link status
			 *	0 - link down
			 *	1 - link up
			 */
			uint8_t link;
		};
		/* flow control
		 *   0 - disable
		 *   1 - enable
		 */
		uint8_t fc_en;
	};
};

#pragma scalar_storage_order default
#pragma pack(pop)

int sys_misc_fw_update(const GSW_Device_t *dummy);
int sys_misc_fw_version(const GSW_Device_t *dummy, struct sys_fw_image_version *sys_img_ver);
int sys_misc_pvt_temp(const GSW_Device_t *dev, struct sys_sensor_value *sys_temp_val);
int sys_misc_pvt_voltage(const GSW_Device_t *dev, struct sys_sensor_value *sys_voltage);
int sys_misc_delay(const GSW_Device_t *dummy, struct sys_delay *pdelay);
int sys_misc_gpio_configure(const GSW_Device_t *dummy, struct sys_gpio_config *sys_gpio_conf);
int sys_misc_reboot(const GSW_Device_t *dummy);
int sys_misc_reg_rd(const GSW_Device_t *dummy, struct sys_reg_rw *sys_reg);
int sys_misc_reg_wr(const GSW_Device_t *dummy, struct sys_reg_rw *sys_reg);
int sys_misc_reg_mod(const GSW_Device_t *dummy, struct sys_reg_mod *sys_reg);
int sys_misc_cml_clk_get(const GSW_Device_t *dummy, struct sys_cml_clk *clk);
int sys_misc_cml_clk_set(const GSW_Device_t *dummy, struct sys_cml_clk *clk);
int sys_misc_sfp_get(const GSW_Device_t *dummy, struct sys_sfp_cfg *cfg);
int sys_misc_sfp_set(const GSW_Device_t *dummy, struct sys_sfp_cfg *cfg);

#endif
