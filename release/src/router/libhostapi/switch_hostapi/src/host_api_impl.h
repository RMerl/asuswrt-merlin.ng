/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _HOST_API_IMPL_H_
#define _HOST_API_IMPL_H_

extern int gsw_api_wrap(const GSW_Device_t *dev, uint16_t cmd, void *pdata,
			uint16_t size, uint16_t cmd_r, uint16_t r_size);

#endif /* _HOST_API_IMPL_H_ */
