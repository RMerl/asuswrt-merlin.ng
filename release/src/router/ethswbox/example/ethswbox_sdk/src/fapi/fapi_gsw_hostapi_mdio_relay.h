#ifndef _FAPI_GPY_HOST_H_

/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

#include <mmd_apis.h>

int fapi_int_gphy_read(int prmc, char *prmv[]);
/* write internal GPHY MDIO/MMD registers */
int fapi_int_gphy_write(int prmc, char *prmv[]);
/* modify internal GPHY MDIO/MMD registers */
int fapi_int_gphy_mod(int prmc, char *prmv[]);

/* read external GPHY MDIO/MMD registers via MDIO bus */
int fapi_ext_mdio_read(int prmc, char *prmv[]);
/* write external GPHY MDIO/MMD registers via MDIO bus */
int fapi_ext_mdio_write(int prmc, char *prmv[]);
/* modify external GPHY MDIO/MMD registers via MDIO bus */
int fapi_ext_mdio_mod(int prmc, char *prmv[]);

#endif /* _FAPI_GPY_HOST_H_ */
