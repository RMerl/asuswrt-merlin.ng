#ifndef _BCM_RSVMEM_H
#define _BCM_RSVMEM_H

#include <linux/types.h>

#if defined(CONFIG_BCM_ADSL) || defined(CONFIG_BCM_ADSL_MODULE)
#define ADSL_RESERVE_MEM_NUM            1
#else
#define ADSL_RESERVE_MEM_NUM            0
#endif

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#define RDPA_RESERVE_MEM_NUM            2
#else
#define RDPA_RESERVE_MEM_NUM            0
#endif

#if defined(CONFIG_BCM_DHD_RUNNER) || defined(CONFIG_BCM_DHD_RUNNER_MODULE)
#define DHD_RESERVE_MEM_NUM             3
#else
#define DHD_RESERVE_MEM_NUM             0
#endif

#if defined(CONFIG_BCM960333)
#define PLC_RESERVE_MEM_NUM             1
#else
#define PLC_RESERVE_MEM_NUM             0
#endif

#if defined(CONFIG_OPTEE)
#define OPTEE_RESERVE_MEM_NUM           2
#else
#define OPTEE_RESERVE_MEM_NUM           0
#endif

#if defined(CONFIG_BCM_B15_MEGA_BARRIER)
#define BARRIER_RESERVE_MEM_NUM         1
#else
#define BARRIER_RESERVE_MEM_NUM         0
#endif

/* one extra for CMA padding reserve memory */
#define TOTAL_RESERVE_MEM_NUM           (ADSL_RESERVE_MEM_NUM+RDPA_RESERVE_MEM_NUM+DHD_RESERVE_MEM_NUM+\
                                        PLC_RESERVE_MEM_NUM+BARRIER_RESERVE_MEM_NUM+\
                                        OPTEE_RESERVE_MEM_NUM+1)

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

extern phys_addr_t cma_phys_addr;
extern unsigned int cma_size;
extern char* dt_scan_mem_str[];
extern bool is_memory_reserved;
extern unsigned long reserved_mem_total;
extern int rsvd_mem_cnt;
extern reserve_mem_t reserve_mem[TOTAL_RESERVE_MEM_NUM];

#define BcmMemReserveVirtToPhys(vbase, pbase, virt)  \
        (phys_addr_t)((phys_addr_t)(pbase) + (phys_addr_t)((unsigned char*)(virt) - (unsigned char*)(vbase)))
#define BcmMemReservePhysToVirt(vbase, pbase, phys)  \
        (void*)((unsigned char*)(vbase) + ((phys_addr_t)(phys) - (phys_addr_t)(pbase)))
extern int BcmMemReserveGetByName(char *name, void **virt_addr, phys_addr_t* phys_addr, unsigned int *size);


#endif
