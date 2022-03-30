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

typedef union
{
    struct {
        uint32_t data      : 16; // [15:00]
        uint32_t reserved1 : 16; // [31:16]
    } Bits;
    uint32_t Reg32;
} SSBM_data_reg;

typedef union
{
    struct {
        uint32_t  ssb_addr    : 10; // [09:00]
        uint32_t  ssb_cmd     :  2; // [11:10]
        uint32_t  ssb_en      :  1; // [12:12]
        uint32_t  ssb_add_pre :  1; // [13:13]
        uint32_t  reserved2   :  1; // [14:14]
        uint32_t  ssb_start   :  1; // [15:15]
        uint32_t  reserved1   : 16; // [31:16]
    } Bits;
    uint32_t Reg32;
} SSBM_control_reg;

typedef union
{
    struct {
        uint32_t busy      :  1; // [00:00]
        uint32_t reserved1 : 31; // [31:01]
    } Bits;
    uint32_t Reg32;
} SSBM_status_reg;

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

typedef struct PMSSBMasterControl {
    uint32_t control;     /* 0x0060 */
    uint32_t wr_data;      /* 0x0064 */
    uint32_t rd_data;      /* 0x0068 */
    uint32_t status;      /* 0x006c */
    SSBM_SWREG_th_hilo_reg  ssbmThHiLo;     /* 0x0070 */
    SSBM_SWREG_lock_reg     ssbmSwLock;     /* 0x0074 */
} PMSSBMasterControl;

#define SWR_FIRST 0
#define SWR_LAST 2
#define SWR_READ_CMD_P 0xB800
#define SWR_WR_CMD_P   0xB400
#define SWR_EN         0x1000
#define SET_ADDR(ps, reg)  (((ps) << 5 | ((reg) & 0x1f)) & 0x2ff)

#define SR_TEST(x)  {\
                     int num;\
                     for(num=1000;(((PROCMON->SSBMaster.control) & 0x8000) && (num > 0)) ; num--) ;\
                         if(!num) \
                         {\
                             printf("Error num %d timeout num = %d!!!", (x),  num);\
                         }\
}

static const char *swreg_names[] ={"1.5 ", "1.0A "}; 

typedef struct PmmReg {
    uint32_t memPowerCtrl;            /* 0x0000 */
    uint32_t regSecurityConfig;       /* 0x0004 */
} PmmReg;

typedef struct keyholeReg {
    uint32_t ctrlSts;
    uint32_t wrData;
    uint32_t mutex;
    uint32_t rdData;
} keyholeReg;

typedef struct PmbBus {
    PMB_CONFIG_REG config;          /* 0x0100 */
    uint32_t arbiter;         /* 0x0104 */
    uint32_t timeout;         /* 0x0108 */
    uint32_t unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32_t unused2[44];     /* 0x0150-0x01ff */
    uint32_t map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct Procmon {
    PmmReg  pmm;                        /* 0x20000 */
    uint32_t unused11[22];                /* 0x20008-0x2005f */
    PMSSBMasterControl SSBMaster;            /* 0x20060-0x20077 */
    uint32_t unused12[34];                /* 0x20078-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
} Procmon;
#define PROC_MON_BASE        0xffb20000
#define PROCMON ((volatile Procmon * const) PROC_MON_BASE)

typedef struct
{
    uint32_t  control;
#define PMC_PMBM_START                  (1 << 31)
#define PMC_PMBM_TIMEOUT                (1 << 30)
#define PMC_PMBM_SLAVE_ERR              (1 << 29)
#define PMC_PMBM_BUSY                   (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read                   (0 << 24)
#define PMC_PMBM_Write                  (1 << 24)
    uint32_t  wr_data;
    uint32_t  mutex;
    uint32_t  rd_data;
} PMB_keyhole_reg;

typedef struct PMBMaster {
    uint32_t config;
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32_t arbitger;
    uint32_t timeout;
    uint32_t reserved;
    PMB_keyhole_reg keyhole[4];
    uint32_t reserved1[44];
    uint32_t map[64];
} PMBMaster;
#define PMB_BASE        0xffb20100
#define PMB ((volatile PMBMaster * const) PMB_BASE)

#endif
