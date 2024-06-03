#ifndef ACCESS_MACROS_SIM
#define ACCESS_MACROS_SIM
#include "rdd_simulator.h"
#include "rdp_cpu_sim.h"
#include "rdpa_net_sim.h"
extern uint32_t g_runner_sim_connected;

/*
 * This is a woraround for SIM vs HW mismatch. SIM is not masking out variable context
 *  length field as happens in HW.
 * */

#if defined(USE_NATC_VAR_CONTEXT_LEN) 
#define NATC_VAR_CONTEXT_MARK  0x0000000f
#else 
#define NATC_VAR_CONTEXT_MARK  0x00000000
#endif
#define SOC_BASE_ADDRESS        0x82000000
extern uint8_t *soc_base_address;
#define DEVICE_ADDRESS(_a) \
    (((bdmf_phys_addr_t)(_a) < SOC_BASE_ADDRESS) ? \
        ((volatile uint8_t * const)(soc_base_address + RDP_BLOCK_SIZE + ((bdmf_phys_addr_t)(_a) & WAN_BLOCK_ADDRESS_MASK))) \
        : ((volatile uint8_t * const)(soc_base_address + ((bdmf_phys_addr_t)(_a) & (RDP_BLOCK_SIZE - 1)))))


#define MEMSET(a, v, sz)                memset(a, v, sz)
#define WMB()  
#define RMB()
#define DMA_WMB() do {} while(0)

#define VAL32(_a)               (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))
#define READ_8(a, r)            (*(volatile uint8_t *)&(r) = *(volatile uint8_t *)DEVICE_ADDRESS(a))
#define READ_16(a, r)           do { \
                                       uint16_t u16 = *(volatile uint16_t *)DEVICE_ADDRESS(a); \
                                       *(volatile uint16_t *)&(r) = swap2bytes(u16); \
                                } while (0)
#define READ_32(a, r)           do { \
                                       uint32_t u32 = *(volatile uint32_t *)DEVICE_ADDRESS(a); \
                                       *(volatile uint32_t *)&(r) = swap4bytes(u32); \
                                } while (0)

#define WRITE_8(a, r)           (*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r))
#define WRITE_16(a, r)          (*(volatile uint16_t *)DEVICE_ADDRESS(a) = swap2bytes(*(uint16_t *)&(r)))
#define WRITE_32(a, r)          (*(volatile uint32_t *)DEVICE_ADDRESS(a) = swap4bytes(*(uint32_t *)&(r)))

#define READ_I_8(a, i, r)       (*(volatile uint8_t *)&(r) = *((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_16(a, i, r)      do { \
                                    uint16_t u16 = *((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)); \
                                    *(volatile uint16_t *)&(r) = swap2bytes(u16); \
                                } while (0)
#define READ_I_32(a, i, r)      do { \
                                    uint32_t u32 = *((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)); \
                                    *(volatile uint32_t *)&(r) = swap4bytes(u32); \
                                } while (0)

#define WRITE_I_8(a, i, r)      (*((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)) = *(uint8_t *)&(r))
#define WRITE_I_16(a, i, r) (*((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)) = swap2bytes(*(uint16_t *)&(r)))
#define WRITE_I_32(a, i, r) (*((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)) = swap4bytes(*(uint32_t *)&(r)))

#define do_div(x,y)             (x = (x)/(y));

#define MREAD_BLK_8(d, s, sz) memcpy(d, s, sz)
#define MREAD_BLK_16(d, s, sz) memcpy(d, s, sz)
#define MREAD_BLK_32(d, s, sz) memcpy(d, s, sz)

#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#define configure_ubus_biu_fpm_decode() do {} while(0)
void drv_sbpm_default_val_init(void);
#define sim_wait(x) bdmf_usleep(x)
#define RDPA_EPON_CONTROL_QUEUE_ID             101
#endif /* ACCESS_MACROS_SIM */
