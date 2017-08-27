#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
#ifndef __URLINFO
#define __URLINFO

#include <linux/brcm_dll.h>

//#define CC_URLINFO_SUPPORT_DEBUG
#define DPI_URL_RECORD
#ifndef URLINFO_NULL_STMT
#define URLINFO_NULL_STMT                   do { /* NULL BODY */ } while (0)
#endif

#define URLINFO_HTABLE_SIZE 2048
#define URLINFO_MAX_ENTRIES 8192
#define URLINFO_MAX_HOST_LEN 64
#define URLINFO_IX_INVALID 0
#define URLINFO_NULL ((UrlInfo_t*)NULL)
#define URLINFO_DONE 1

typedef struct urlinfo_entry_t {
    uint16_t idx;
    uint16_t hostlen;
    char host[URLINFO_MAX_HOST_LEN];

    /* In the future, URI and refer may be needed */
} UrlInfoEntry_t;

typedef struct urlinfo_t {
    struct dll_t node;
    struct urlinfo_t *chain_p;

    UrlInfoEntry_t entry;
} __attribute__((packed)) UrlInfo_t;


extern uint16_t urlinfo_lookup( const UrlInfoEntry_t *url );
extern void urlinfo_get( uint16_t idx, UrlInfoEntry_t *entry );
extern void urlinfo_set( const UrlInfoEntry_t *entry );
extern int urlinfo_init( void );
#endif
#endif
