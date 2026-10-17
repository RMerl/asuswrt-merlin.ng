#ifndef PMC_H
#define PMC_H 

typedef struct keyholeReg {
    uint32_t control;
#define PMC_PMBM_START		            (1 << 31)
#define PMC_PMBM_TIMEOUT	            (1 << 30)
#define PMC_PMBM_SLAVE_ERR	            (1 << 29)
#define PMC_PMBM_BUSY		            (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read		            (0 << 24)
#define PMC_PMBM_Write		            (1 << 24)
    uint32_t wr_data;
    uint32_t mutex;
    uint32_t rd_data;
} keyholeReg;

typedef struct PmbBus {
    uint32_t config;          /* 0x0100 */
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32_t mutex;           /* 0x0104 */
    uint32_t timeout;         /* 0x0108 */
    uint32_t arbiter;         /* 0x010c */
    keyholeReg keyhole[4];    /* 0x0110-0x014f */
    uint32_t unused2[44];     /* 0x0150-0x01ff */
    uint32_t map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct Pmc {
    PmbBus pmb;                         /* 0x20100 */
} Pmc;

#define PCMBUS_PHYS_BASE            0x83010A00

#endif
