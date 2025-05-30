#ifndef PMC_H
#define PMC_H 

/*
 * Power Management Control
 */
typedef union
{
    struct
    {
        uint32_t propagate_to_err  :  1; // [00:00] -+
        uint32_t propagate_slv_err :  1; // [01:01]  | - these are potentially dangerous and MAY cause a system crash
        uint32_t pmbus_reset_n     :  1; // [02:02] -+
        uint32_t reserved0         :  1; // [03:03]
        uint32_t maxPmbIdx         :  3; // [06:04] 0-based (0-7)
        uint32_t reserved1         :  1; // [07:07]
        uint32_t maxClientId       : 12; // [19:08] 0-based (theoreticaly 0-4095, but code limits this to 256 devices - 0-255)
        uint32_t numRegsPerClient  : 10; // [29:20] some power of 2 - number of 32-bit registers in each client (max = 512)
        uint32_t startDiscovery    :  1; // [30:30] kicks off H/W discovery of clients and fills in the map (see PMB_REGS below)
        uint32_t discoveryBusy     :  1; // [31:31] whether or not H/W discovery is still busy creating the map
    } Bits;
    uint32_t Reg32;
} PMB_CONFIG_REG;

#define kSSBWrite   0x01
#define kSSBRead    0x02
#define kSSBEn      (1 << 12)
#define kSSBStart   (1 << 15)

typedef struct SSBMaster {
    uint32_t    control;    /* 0x0060 */
    uint32_t    wr_data;    /* 0x0064 */
    uint32_t    rd_data;    /* 0x0068 */
    uint32_t    status;     /* 0x006c */
    uint32_t    thHiLo;     /* 0x0070 */
    uint32_t    swLock;     /* 0x0074 */
} SSBMaster;

#define SWR_FIRST 0
#define SWR_LAST 4

#if defined(CONFIG_SWREG_ADJUSTMENT)
static const char *swreg_names[] ={"1.0D", "1.8 ", "1.5 ", "1.0A"} ;
static inline const char* get_swreg_names(int index)
{
	if(index < sizeof(swreg_names))
	{
		return swreg_names[index];
	}
	return NULL;
}
#endif

typedef struct PmmReg {
    uint32_t memPowerCtrl;            /* 0x0000 */
    uint32_t regSecurityConfig;       /* 0x0004 */
} PmmReg;

typedef struct keyholeReg {
    uint32_t control;
#define PMC_PMBM_START                  (1 << 31)
#define PMC_PMBM_TIMEOUT                (1 << 30)
#define PMC_PMBM_SLAVE_ERR              (1 << 29)
#define PMC_PMBM_BUSY                   (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read                   (0 << 24)
#define PMC_PMBM_Write                  (1 << 24)
    uint32_t wr_data;
    uint32_t mutex;
    uint32_t rd_data;
} keyholeReg;

typedef struct PmbBus {
    uint32_t config;          /* 0x0100 */
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32_t arbiter;         /* 0x0104 */
    uint32_t timeout;         /* 0x0108 */
    uint32_t unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32_t unused2[44];     /* 0x0150-0x01ff */
    uint32_t map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct Pmc {
    SSBMaster ssbMaster;            /* 0x20060-0x20077 */
    uint32_t unused12[34];                /* 0x20078-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
} Pmc;

#endif
