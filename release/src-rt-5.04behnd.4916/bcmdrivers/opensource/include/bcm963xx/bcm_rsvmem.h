#ifndef _BCM_RSVMEM_H
#define _BCM_RSVMEM_H

#include <linux/types.h>

#define ADSL_BASE_ADDR_STR          "adsl"
#define PARAM1_BASE_ADDR_STR        "rdp1"
#define PARAM2_BASE_ADDR_STR        "rdp2"
#define BUFMEM_BASE_ADDR_STR        "bufmem"
#define RNRMEM_BASE_ADDR_STR        "rnrmem"
#define TM_BASE_ADDR_STR            PARAM1_BASE_ADDR_STR
#define TM_MC_BASE_ADDR_STR         PARAM2_BASE_ADDR_STR
#if defined(CONFIG_BCM963146) || defined(_BCM963146_) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define FPMPOOL_BASE_ADDR_STR       BUFMEM_BASE_ADDR_STR
#define RNRTBLS_BASE_ADDR_STR       RNRMEM_BASE_ADDR_STR
#else
#define FPMPOOL_BASE_ADDR_STR       PARAM1_BASE_ADDR_STR
#define RNRTBLS_BASE_ADDR_STR       PARAM2_BASE_ADDR_STR
#endif
#define BUFFER_MEMORY_BASE_ADDR_STR PARAM1_BASE_ADDR_STR
#define FLOW_MEMORY_BASE_ADDR_STR   PARAM2_BASE_ADDR_STR
#define DHD_BASE_ADDR_STR           "dhd0"
#define DHD_BASE_ADDR_STR_1         "dhd1"
#define DHD_BASE_ADDR_STR_2         "dhd2"
#define DHD_BASE_ADDR_STR_3         "dhd3"
#define PLC_BASE_ADDR_STR           "plc"
#define CMA_BASE_ADDR_STR           "cma"
#define CMA_PAD_BASE_ADDR_STR       "pad0"
#define B15_MEGA_BARRIER            "b15_mega_br"
#define DT_RSVD_PREFIX_STR          "dt_reserved_"
#define DT_RSVD_NODE_STR            "reserved-memory"
#define DT_CMA_CACHED_NODE_STR      "plat_rsvmem_cached_device"
#define DT_CMA_UNCACHED_NODE_STR    "plat_rsvmem_uncached_device"
#define DT_CMA_RSVSIZE_PROP_STR     "rsvd-size"

#if defined(CONFIG_BCM_ADSL) || defined(CONFIG_BCM_ADSL_MODULE)
#define ADSL_RESERVE_MEM_NUM            1
#else
#define ADSL_RESERVE_MEM_NUM            0
#endif

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#define RDPA_RESERVE_MEM_NUM            2
#define MPM_RESERVE_MEM_NUM             0
#else
#define RDPA_RESERVE_MEM_NUM            0
/* there are non-RDP based chip that supports MPM */
#if defined(CONFIG_BCM_MPM) || defined(CONFIG_BCM_MPM_MODULE)
#define MPM_RESERVE_MEM_NUM             1
#else
#define MPM_RESERVE_MEM_NUM             0
#endif
#endif

#if defined(CONFIG_BCM_DHD_RUNNER) || defined(CONFIG_BCM_DHD_RUNNER_MODULE) || IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#define DHD_RESERVE_MEM_NUM             3
#else
#define DHD_RESERVE_MEM_NUM             0
#endif

#if defined(CONFIG_OPTEE)
#define OPTEE_RESERVE_MEM_NUM           2
#else
#define OPTEE_RESERVE_MEM_NUM           0
#endif

/* one extra for CMA padding reserve memory */
#define TOTAL_RESERVE_MEM_NUM           (ADSL_RESERVE_MEM_NUM+RDPA_RESERVE_MEM_NUM+DHD_RESERVE_MEM_NUM+\
                                        OPTEE_RESERVE_MEM_NUM+MPM_RESERVE_MEM_NUM+1)

#define MAX_RESREVE_MEM_NAME_SIZE       32

typedef struct _reserve_mem_t{
    char         name[MAX_RESREVE_MEM_NAME_SIZE];
    void*        virt_addr;
    phys_addr_t  phys_addr;
    unsigned int size;
    /* 1 - kernel automatically mapped as cached memory
       0 - bsp manually mapped as uncached memory 
    */
    int          mapped;
}reserve_mem_t;

#define BcmMemReserveVirtToPhys(vbase, pbase, virt)  \
        (phys_addr_t)((phys_addr_t)(pbase) + (phys_addr_t)((unsigned char*)(virt) - (unsigned char*)(vbase)))
#define BcmMemReservePhysToVirt(vbase, pbase, phys)  \
        (void*)((unsigned char*)(vbase) + ((phys_addr_t)(phys) - (phys_addr_t)(pbase)))
extern int BcmMemReserveGetByName(char *name, void **virt_addr, phys_addr_t* phys_addr, unsigned int *size);


#endif
