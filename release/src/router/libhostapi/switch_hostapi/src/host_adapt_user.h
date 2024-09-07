/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _HOST_ADAPT_H_
#define _HOST_ADAPT_H_


/* endianess*/
#undef CONFIG_LITTLE_ENDIAN			/* big-endian  */
#define CONFIG_LITTLE_ENDIAN 1		/* little-endian */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#include "gsw_device.h"
#include "mmd_apis.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#endif

#define BITS_PER_LONG   (__CHAR_BIT__ * __SIZEOF_LONG__)

#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#undef BIT
#define BIT(x)		(1UL << (x))


/* This is to enable optimization of host <-> F48X communication.
 * It's based on a frequent use case that host get the configuration,
 * modify a field, then set it back to F48X (get-modify-set).
 * A set of shadow value has been saved for each GET API so that
 * only changed values are sent to F48X from host in following SET API.
 *
 * Note:
 *   GET and following SET should be atomic. So, a mutex should be used
 *   in Multithread environment/application to ensure no other GSW API
 *   called between GET and SET.
 *   Otherwise, please change this option to 0 to avoid malfunction.
 */
#define ENABLE_GETSET_OPT	1



#ifdef CONFIG_LITTLE_ENDIAN
#define sys_le16_to_cpu(val) (val)
#define sys_cpu_to_le16(val) (val)
#define sys_le24_to_cpu(val) (val)
#define sys_cpu_to_le24(val) (val)
#define sys_le32_to_cpu(val) (val)
#define sys_cpu_to_le32(val) (val)
#define sys_le48_to_cpu(val) (val)
#define sys_cpu_to_le48(val) (val)
#define sys_le64_to_cpu(val) (val)
#define sys_cpu_to_le64(val) (val)
#define sys_be16_to_cpu(val) __bswap_16(val)
#define sys_cpu_to_be16(val) __bswap_16(val)
#define sys_be24_to_cpu(val) __bswap_24(val)
#define sys_cpu_to_be24(val) __bswap_24(val)
#define sys_be32_to_cpu(val) __bswap_32(val)
#define sys_cpu_to_be32(val) __bswap_32(val)
#define sys_be48_to_cpu(val) __bswap_48(val)
#define sys_cpu_to_be48(val) __bswap_48(val)
#define sys_be64_to_cpu(val) __bswap_64(val)
#define sys_cpu_to_be64(val) __bswap_64(val)
#else
#define sys_le16_to_cpu(val) __bswap_16(val)
#define sys_cpu_to_le16(val) __bswap_16(val)
#define sys_le24_to_cpu(val) __bswap_24(val)
#define sys_cpu_to_le24(val) __bswap_24(val)
#define sys_le32_to_cpu(val) __bswap_32(val)
#define sys_cpu_to_le32(val) __bswap_32(val)
#define sys_le48_to_cpu(val) __bswap_48(val)
#define sys_cpu_to_le48(val) __bswap_48(val)
#define sys_le64_to_cpu(val) __bswap_64(val)
#define sys_cpu_to_le64(val) __bswap_64(val)
#define sys_be16_to_cpu(val) (val)
#define sys_cpu_to_be16(val) (val)
#define sys_be24_to_cpu(val) (val)
#define sys_cpu_to_be24(val) (val)
#define sys_be32_to_cpu(val) (val)
#define sys_cpu_to_be32(val) (val)
#define sys_be48_to_cpu(val) (val)
#define sys_cpu_to_be48(val) (val)
#define sys_be64_to_cpu(val) (val)
#define sys_cpu_to_be64(val) (val)
#endif

/* Clause 22 MDIO read and write functions to customize */
#ifndef CL22_MDIO_READ
#define CL22_MDIO_READ(...) 0
#endif

#ifndef CL22_MDIO_WRITE
#define CL22_MDIO_WRITE(...) 0
#endif

/* Clause 45 MDIO read and write functions to customize */
#ifndef CL45_MDIO_READ
#define CL45_MDIO_READ(...) 0
#endif

#ifndef CL45_MDIO_WRITE
#define CL45_MDIO_WRITE(...) 0
#endif


/*prototype global functions */
int32_t api_gsw_get_links(char* lib);
int gsw_read(const GSW_Device_t *dev, uint32_t regaddr);
int gsw_write(const GSW_Device_t *dev, uint32_t regaddr, uint16_t data);
GSW_Device_t* gsw_get_struc(uint8_t lif_id,uint8_t phy_id);


#endif /* _HOST_ADAPT_H_ */
