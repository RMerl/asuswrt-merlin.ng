#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
#ifndef __DEVINFO_H__
#define __DEVINFO_H__

#include <linux/brcm_dll.h>

//#define CC_DEVINFO_SUPPORT_DEBUG
#ifndef DEVINFO_NULL_STMT
#define DEVINFO_NULL_STMT                   do { /* NULL BODY */ } while (0)
#endif

#define DEVINFO_HTABLE_SIZE 64
#define DEVINFO_MAX_ENTRIES 256
#define DEVINFO_IX_INVALID 0
#define DEVINFO_NULL ((DevInfo_t*)NULL)
#define DEVINFO_DONE 1

#include <linux/if_ether.h>

typedef struct devinfo_entry_t {
    uint16_t idx;
    uint16_t flags;
    uint16_t vendor_id; //!< Vendor (e.g. "Microsoft")
    uint16_t os_id; //!< OS/Device name (e.g. "Windows 8", or "iPhone 4")
    uint16_t class_id; //!< OS Class (e.g. "Windows")
    uint16_t type_id; //!< Device Type (e.g. "Phone")
    uint32_t dev_id; //!< Device Name (e.g. "iPhone 4")
} DevInfoEntry_t;

typedef struct devinfo_t {
    struct dll_t node;
    struct devinfo_t *chain_p;

    DevInfoEntry_t entry;
    uint8_t mac[ETH_ALEN];
} __attribute__((packed)) DevInfo_t;


extern uint16_t devinfo_lookup( const uint8_t *mac );
extern void devinfo_get( uint16_t idx, DevInfoEntry_t *entry );
extern void devinfo_set( const DevInfoEntry_t *entry );
extern void devinfo_getmac( uint16_t idx, uint8_t *mac );
extern int devinfo_init( void );
#endif
#endif
