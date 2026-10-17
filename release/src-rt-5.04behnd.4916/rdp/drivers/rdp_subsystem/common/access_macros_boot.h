#ifndef ACCESS_MACROS_BOOT
#define ACCESS_MACROS_BOOT

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#else 
#include <linux/types.h>
#endif

#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)((uintptr_t)(_a)))

#if defined(_CFE_)
#define MEMSET(a, v, sz)                \
    do {\
        lib_memset(a, v, sz); \
        ACCESS_LOG(ACCESS_LOG_OP_MEMSET, a, v, sz); \
    } while (0)

#else
/*uboot */
#define MEMSET(a, v, sz)                \
    do {\
        memset(a, v, sz); \
        ACCESS_LOG(ACCESS_LOG_OP_MEMSET, a, v, sz); \
    } while (0)

#endif

#define WMB()  /* */
#define RMB()  /* */

#define VAL32(_a)       (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))
#define READ_8(a, r)        (*(volatile uint8_t *)&(r) = *(volatile uint8_t *)DEVICE_ADDRESS(a))
#define READ_16(a, r)       (*(volatile uint16_t *)&(r) = *(volatile uint16_t *)DEVICE_ADDRESS(a))
#define READ_32(a, r)           (*(volatile uint32_t *)&(r) = *(volatile uint32_t *)DEVICE_ADDRESS(a))
#define READ_64(a, r)           (*(volatile uint64_t *)&(r) = *(volatile uint64_t *)DEVICE_ADDRESS(a))

#if defined(CONFIG_GPL_RDP_GEN)
/*this version use read modify write method to save writes in CFE GPL mode */
#define WRITE_8(a, r) \
                do {\
                	uint8_t temp; \
                	uint8_t temp1 = *(uint8_t *)&r; \
                	READ_8(a, temp); \
                	if ((temp != temp1) || (temp != 0))\
                	{ \
						(*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r)); \
						ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint8_t *)&(r), 1); \
                	} \
                } while (0)
#define WRITE_16(a, r) \
                do {\
                	uint16_t temp; \
                	uint16_t temp1 = *(uint16_t *)&r; \
                	READ_16(a, temp); \
                	if ((temp != temp1) || (temp != 0))\
                	{ \
						(*(volatile uint16_t *)DEVICE_ADDRESS(a) = *(uint16_t *)&(r)); \
						ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint16_t *)&(r), 2); \
						} \
                } while (0)
#define WRITE_32(a, r) \
                do {\
                	uint32_t temp; \
                	uint32_t temp1 = *(uint32_t *)&r; \
                	READ_32(a, temp); \
                	if ((temp != temp1) || (temp != 0))\
                	{ \
						(*(volatile uint32_t *)DEVICE_ADDRESS(a) = *(uint32_t *)&(r)); \
						ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint32_t *)&(r), 4); \
                	} \
                } while (0)
#define WRITE_64(a, r) \
                do {\
                	uint64_t temp; \
                	uint64_t temp1 = *(uint64_t *)&r; \
                	READ_64(a, temp); \
                	if ((temp != temp1) || (temp != 0))\
                	{ \
						(*(volatile uint64_t *)DEVICE_ADDRESS(a) = *(uint64_t *)&(r)); \
						ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint64_t *)&(r), 8); \
                	} \
                } while (0)

#else
#define WRITE_8(a, r) \
                do {\
                    (*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint8_t *)&(r), 1); \
                } while (0)
#define WRITE_16(a, r) \
                do {\
                    (*(volatile uint16_t *)DEVICE_ADDRESS(a) = *(uint16_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint16_t *)&(r), 2); \
                } while (0)
#define WRITE_32(a, r) \
                do {\
                    (*(volatile uint32_t *)DEVICE_ADDRESS(a) = *(uint32_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint32_t *)&(r), 4); \
                } while (0)
#define WRITE_64(a, r) \
                do {\
                    (*(volatile uint64_t *)DEVICE_ADDRESS(a) = *(uint64_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint64_t *)&(r), 8); \
                } while (0)

#endif

#define READ_I_8(a, i, r)       (*(volatile uint8_t *)&(r) = *((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_16(a, i, r)  (*(volatile uint16_t *)&(r) = *((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_32(a, i, r)  (*(volatile uint32_t *)&(r) = *((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)))

#define WRITE_I_8(a, i, r) \
                do {\
                    (*((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)) = *(uint8_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint8_t *)DEVICE_ADDRESS(a) + (i), *(uint8_t *)&(r), 1); \
                } while (0)
#define WRITE_I_16(a, i, r) \
                do {\
                    (*((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)) = *(uint16_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint16_t *)DEVICE_ADDRESS(a) + (i), *(uint16_t *)&(r), 2); \
                } while (0)
#define WRITE_I_32(a, i, r) \
                do {\
                    (*((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)) = *(uint32_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint32_t *)DEVICE_ADDRESS(a) + (i), *(uint32_t *)&(r), 4); \
                } while (0)

#define MREAD_BLK_8(d, s, sz) memcpy(d, s, sz)
#define MREAD_BLK_16(d, s, sz) memcpy(d, s, sz)
#define MREAD_BLK_32(d, s, sz) memcpy(d, s, sz)

#if defined(_CFE_)
#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#else /*__UBOOT__*/
#if CHIP_VER >= RDP_GEN_50
#define MWRITE_BLK_8(d, s, sz) _xrdp__memcpy(d, s, sz)
#else
#if defined(CONFIG_ARM64)
#define MWRITE_BLK_8(d, s, sz) memcpy_toio(d, s, sz)
#else
#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#endif /*CONFIG_ARM64*/
#endif /*CHIP_VER*/ 
#endif /*_CFE_ */

#endif /* ACCESS_MACROS_ARC */
