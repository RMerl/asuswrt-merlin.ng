/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include "host_adapt.h"
#include "host_api_impl.h"
#include "mdio_relay.h"

int int_gphy_read(const GSW_Device_t *dev, struct mdio_relay_data *parm)
{
	return gsw_api_wrap(dev,
			    INT_GPHY_READ,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(parm->data));
}

int int_gphy_write(const GSW_Device_t *dev, struct mdio_relay_data *parm)
{
	return gsw_api_wrap(dev,
			    INT_GPHY_WRITE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

int int_gphy_mod(const GSW_Device_t *dev, struct mdio_relay_mod_data *parm)
{
	return gsw_api_wrap(dev,
			    INT_GPHY_MOD,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

int ext_mdio_read(const GSW_Device_t *dev, struct mdio_relay_data *parm)
{
	return gsw_api_wrap(dev,
			    EXT_MDIO_READ,
			    parm,
			    sizeof(*parm),
			    0,
			    sizeof(parm->data));
}

int ext_mdio_write(const GSW_Device_t *dev, struct mdio_relay_data *parm)
{
	return gsw_api_wrap(dev,
			    EXT_MDIO_WRITE,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

int ext_mdio_mod(const GSW_Device_t *dev, struct mdio_relay_mod_data *parm)
{
	return gsw_api_wrap(dev,
			    EXT_MDIO_MOD,
			    parm,
			    sizeof(*parm),
			    0,
			    0);
}

