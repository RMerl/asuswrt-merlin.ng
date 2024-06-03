#ifndef ACCESS_MACROS_BOARD
#define ACCESS_MACROS_BOARD
#include "linux/types.h"
#include <asm/barrier.h>
#include "bcm_ubus4.h"

volatile void *_xrdp__memcpy(volatile void *dst, const volatile void *src, size_t len) ;
volatile void *_xrdp__memset(volatile void *dst, int val, size_t len);

#define NATC_VAR_CONTEXT_MARK  0x00000000
#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)((uintptr_t)(_a)))

#if CHIP_VER >= RDP_GEN_50
#define MEMSET(a, v, sz)                \
    do {\
        _xrdp__memset(a, v, sz); \
        ACCESS_LOG(ACCESS_LOG_OP_MEMSET, a, v, sz); \
    } while (0)
#else
#define MEMSET(a, v, sz)                memset_io(a, v, sz)
#endif

#define WMB()  wmb() /* memory barrier */
#define RMB()  rmb() /* memory barrier */
#define DMA_WMB dma_wmb
#define sim_wait() do { } while(0)

#define VAL32(_a)       (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))

#define READ_8(a, r)        (*(volatile uint8_t *)&(r) = *(volatile uint8_t *)DEVICE_ADDRESS(a))
#define READ_16(a, r)       (*(volatile uint16_t *)&(r) = *(volatile uint16_t *)DEVICE_ADDRESS(a))
#define READ_32(a, r)           (*(volatile uint32_t *)&(r) = *(volatile uint32_t *)DEVICE_ADDRESS(a))
#define READ_64(a, r)           (*(volatile uint64_t *)&(r) = *(volatile uint64_t *)DEVICE_ADDRESS(a))

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

#if CHIP_VER >= RDP_GEN_50
    #define MREAD_BLK_8(d, s, sz)   _xrdp__memcpy(d, s, sz)
#else /* CHIP_VER >= RDP_GEN_50 */
    #define MREAD_BLK_8(d, s, sz)  memcpy_fromio(d, s, sz)
#endif /* CHIP_VER >= RDP_GEN_50 */

#define MREAD_BLK_16(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz/sizeof(uint16_t)); i++) \
        { \
            val = *((volatile uint16_t *)(s) + (i)); \
            MREAD_I_16((uintptr_t)(d), i, val); \
        } \
    } while (0)

#define MREAD_BLK_32(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz/sizeof(uint32_t)); i++) \
        { \
            val = *((volatile uint32_t *)(s) + (i)); \
            MREAD_I_32((uintptr_t)(d), i, val); \
        } \
    } while (0)

#if CHIP_VER >= RDP_GEN_50
#define MWRITE_BLK_8(d, s, sz) _xrdp__memcpy(d, s, sz)
#else
#if defined(CONFIG_ARM64)
#define MWRITE_BLK_8(d, s, sz) memcpy_toio(d, s, sz)
#else
#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#endif  /*CONFIG_ARM64 */ 
#endif  /* CHIP_VER */
#define drv_sbpm_default_val_init() do {} while(0)
#endif /* ACCESS_MACROS_ARC */
