/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include "host_adapt.h"
#include "host_api_impl.h"

int gsw_ss_sptag_get(const GSW_Device_t *dev, struct gsw_ss_sptag *pdata)
{
	return gsw_api_wrap(dev,
			    GSW_SS_SPTAG_GET,
			    pdata,
			    sizeof(*pdata),
			    0,
			    sizeof(*pdata));
}

int gsw_ss_sptag_set(const GSW_Device_t *dev, struct gsw_ss_sptag *pdata)
{
	return gsw_api_wrap(dev,
			    GSW_SS_SPTAG_SET,
			    pdata,
			    sizeof(*pdata),
			    GSW_SS_SPTAG_GET,
			    0);
}
