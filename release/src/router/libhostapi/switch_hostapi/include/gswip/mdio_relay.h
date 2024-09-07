/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _MDIO_RELAY_H_
#define _MDIO_RELAY_H_

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

struct mdio_relay_data {
	/* data to be read or written */
	uint16_t data;
	/* PHY index (0~7) for internal PHY
	 * PHY address (0~31) for external PHY access via MDIO bus
	 */
	uint8_t phy;
	/* MMD device (0~31) */
	uint8_t mmd;
	/* Register Index
	 * 0~31 if mmd is 0 (CL22)
	 * 0~65535 otherwise (CL45)
	 */
	uint16_t reg;
};

struct mdio_relay_mod_data {
	/* data to be written with mask */
	uint16_t data;
	/* PHY index (0~7) for internal PHY
	 * PHY address (0~31) for external PHY access via MDIO bus
	 */
	uint8_t phy;
	/* MMD device (0~31) */
	uint8_t mmd;
	/* Register Index
	 * 0~31 if mmd is 0 (CL22)
	 * 0~65535 otherwise (CL45)
	 */
	uint16_t reg;
	/* mask of bit fields to be updated
	 * 1 to write the bit
	 * 0 to ignore
	 */
	uint16_t mask;
};

#pragma scalar_storage_order default
#pragma pack(pop)

/* read internal GPHY MDIO/MMD registers */
int int_gphy_read(const GSW_Device_t *dev, struct mdio_relay_data *pdata);
/* write internal GPHY MDIO/MMD registers */
int int_gphy_write(const GSW_Device_t *dev, struct mdio_relay_data *pdata);
/* modify internal GPHY MDIO/MMD registers */
int int_gphy_mod(const GSW_Device_t *dev, struct mdio_relay_mod_data *pdata);

/* read external GPHY MDIO/MMD registers via MDIO bus */
int ext_mdio_read(const GSW_Device_t *dev, struct mdio_relay_data *pdata);
/* write external GPHY MDIO/MMD registers via MDIO bus */
int ext_mdio_write(const GSW_Device_t *dev, struct mdio_relay_data *pdata);
/* modify external GPHY MDIO/MMD registers via MDIO bus */
int ext_mdio_mod(const GSW_Device_t *dev, struct mdio_relay_mod_data *pdata);

#endif /*  _MDIO_RELAY_H_ */
