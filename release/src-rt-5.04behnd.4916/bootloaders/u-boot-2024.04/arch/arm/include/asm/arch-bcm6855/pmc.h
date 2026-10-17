#ifndef PMC_H
#define PMC_H 

/*
 * Power Management Control
 */

typedef union
{
    struct {
        uint32_t swreg_th_lo : 8; // [07:00]
        uint32_t swreg_th_hi : 8; // [15:08]
        uint32_t reserved    :16; // [31:16]
    } Bits;
    uint32_t Reg32;
} SSBM_SWREG_th_hilo_reg;

typedef union
{
    struct {
        uint32_t ssb_lock_addr : 10; // [09:00]
        uint32_t lock_bit      :  1; // [10:10]
        uint32_t lock_mode     :  1; // [11:11]
        uint32_t reserved      : 20; // [31:12]
    } Bits;
    uint32_t Reg32;
} SSBM_SWREG_lock_reg;

#define kSSBWrite   0x01
#define kSSBRead    0x02
#define kSSBEn      (1 << 12)
#define kSSBStart   (1 << 15)

typedef struct SSBMaster {
    uint32_t control;    /* 0x0060 */
    uint32_t wr_data;    /* 0x0064 */
    uint32_t rd_data;    /* 0x0068 */
    uint32_t status;     /* 0x006c */
    SSBM_SWREG_th_hilo_reg  ssbmThHiLo;     /* 0x0070 */
    SSBM_SWREG_lock_reg     ssbmSwLock;     /* 0x0074 */
} SSBMaster;

#define SWR_FIRST 0
#define SWR_LAST 2

static const char *swreg_names[] ={"1.5 ", "1.0A "}; 
static inline const char* get_swreg_names(int index)
{
	if(index < sizeof(swreg_names))
	{
		return swreg_names[index];
	}
	return NULL;
}

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
