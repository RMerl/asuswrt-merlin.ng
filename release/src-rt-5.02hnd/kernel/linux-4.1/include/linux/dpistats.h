#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
#ifndef __DPISTATS_H__
#define __DPISTATS_H__

#include <net/netfilter/nf_conntrack.h>
#include <linux/brcm_dll.h>

//#define CC_DPISTATS_SUPPORT_DEBUG
#ifndef DPISTATS_NULL_STMT
#define DPISTATS_NULL_STMT                   do { /* NULL BODY */ } while (0)
#endif

#define DPISTATS_HTABLE_SIZE 512
#define DPISTATS_MAX_ENTRIES 4096
#define DPISTATS_IX_INVALID 0
#define DPISTATS_NULL ((DpiStats_t*)NULL)

typedef struct ctk_stats_t {
    unsigned long pkts;
    unsigned long long bytes;
    unsigned long ts;
} CtkStats_t;

typedef struct dpistats_entry_t {
    uint32_t idx;
    dpi_info_t result;
    CtkStats_t upstream;
    CtkStats_t dnstream;
} DpiStatsEntry_t;

typedef struct dpistats_t {
    struct dll_t node;
    struct dpistats_t *chain_p;

    DpiStatsEntry_t entry;
    CtkStats_t evict_up;
    CtkStats_t evict_dn;
} __attribute__((packed)) DpiStats_t;

extern uint32_t dpistats_lookup( const dpi_info_t *res_p );
extern void dpistats_info( uint32_t idx, const DpiStatsEntry_t *stats_p );
extern void dpistats_update( uint32_t idx, const DpiStatsEntry_t *stats_p );
extern void dpistats_show( struct seq_file *s );
extern int dpistats_init( void );
#endif
#endif
