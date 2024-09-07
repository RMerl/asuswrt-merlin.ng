/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_DEVICE_H_
#define _GSW_DEVICE_H_
/** \file gsw_device.h GSW Device Type */

/** \brief GSW Device Prototype.
    Used by all GSW APIs. */
typedef struct {
	void (*usleep)(unsigned long usec);

	void (*lock)(void *lock_data);
	void (*unlock)(void *lock_data);
	void *lock_data;

	int (*mdiobus_read)(void *mdiobus_data, uint8_t phyaddr, uint8_t mmd,
			    uint16_t reg);
	int (*mdiobus_write)(void *mdiobus_data, uint8_t phyaddr, uint8_t mmd,
			     uint16_t reg, uint16_t val);
	void *mdiobus_data;

	uint8_t phy_addr;
	uint8_t smdio_phy_addr;
} GSW_Device_t;

#endif /* _GSW_DEVICE_H_ */
