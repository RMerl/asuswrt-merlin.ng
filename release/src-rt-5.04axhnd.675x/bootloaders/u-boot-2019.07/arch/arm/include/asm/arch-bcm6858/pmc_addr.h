#define PMB_BUS_MAX              2
#define PMB_BUS_ID_SHIFT         8

#define PMB_BUS_PERIPH           0
#define PMB_ADDR_PERIPH          (0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         3

#define PMB_BUS_CHIP_CLKRST      0
#define PMB_ADDR_CHIP_CLKRST     (1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define PMB_BUS_SYSPLL           0
#define PMB_ADDR_SYSPLL          (2 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL         0

#define PMB_BUS_RDPPLL           0
#define PMB_ADDR_RDPPLL          (3 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL         0

#define PMB_BUS_UNIPLL           0
#define PMB_ADDR_UNIPLL          (5 | PMB_BUS_UNIPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_UNIPLL         0

#define PMB_BUS_CRYPTO           1
#define PMB_ADDR_CRYPTO          (6 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO         0

#define PMB_BUS_APM              0
#define PMB_ADDR_APM             (7 | PMB_BUS_APM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_APM            2

#define PMB_BUS_MEMC             0
#define PMB_ADDR_MEMC            (8 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_LPORT            1
#define PMB_ADDR_LPORT           (9 | PMB_BUS_LPORT << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_LPORT          3

#define PMB_BUS_USB30_2X         1
#define PMB_ADDR_USB30_2X        (10 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X       4

#define PMB_BUS_WAN              1
#define PMB_ADDR_WAN             (11 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            7

#define PMB_BUS_XRDP              1
#define PMB_ADDR_XRDP             (12 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP            3

#define PMB_BUS_XRDP_QM           1
#define PMB_ADDR_XRDP_QM          (13 | PMB_BUS_XRDP_QM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_QM         1

#define PMB_BUS_XRDP_RC_QUAD0     1
#define PMB_ADDR_XRDP_RC_QUAD0    (14 | PMB_BUS_XRDP_RC_QUAD0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD0   1

#define PMB_BUS_XRDP_RC_QUAD1     1
#define PMB_ADDR_XRDP_RC_QUAD1    (15 | PMB_BUS_XRDP_RC_QUAD1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD1   1

#define PMB_BUS_XRDP_RC_QUAD2     1
#define PMB_ADDR_XRDP_RC_QUAD2    (16 | PMB_BUS_XRDP_RC_QUAD2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD2   1

#define PMB_BUS_XRDP_RC_QUAD3     1
#define PMB_ADDR_XRDP_RC_QUAD3    (17 | PMB_BUS_XRDP_RC_QUAD3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD3   1

#define PMB_BUS_PCIE0              1
#define PMB_ADDR_PCIE0             (18 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0            1

#define PMB_BUS_PCIE1              1
#define PMB_ADDR_PCIE1             (19 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1            1

#define PMB_BUS_SATA               1
#define PMB_ADDR_SATA             (20 | PMB_BUS_SATA << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SATA             1

#define PMB_BUS_PCIE_UBUS          1
#define PMB_ADDR_PCIE_UBUS         (21 | PMB_BUS_PCIE_UBUS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE_UBUS        1

#define PMB_BUS_ORION_CPU0         0
#define PMB_ADDR_ORION_CPU0        (24 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0       1

#define PMB_BUS_ORION_CPU1         0
#define PMB_ADDR_ORION_CPU1        (25 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1       1

#define PMB_BUS_ORION_CPU2         0
#define PMB_ADDR_ORION_CPU2        (26 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2       1

#define PMB_BUS_ORION_CPU3         0
#define PMB_ADDR_ORION_CPU3        (27 | PMB_BUS_ORION_CPU3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU3       1

#define PMB_BUS_ORION_NONCPU       0
#define PMB_ADDR_ORION_NONCPU      (28 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU     1

#define PMB_BUS_ORION_ARS          0
#define PMB_ADDR_ORION_ARS         (29 | PMB_BUS_ORION_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_ARS        1

#define PMB_BUS_BIU_PLL            0
#define PMB_ADDR_BIU_PLL           (30 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL          1   // FIXMET

#define PMB_BUS_BIU_BPCM           0
#define PMB_ADDR_BIU_BPCM          (31 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM         1

#define PMB_BUS_PCM                0

#define PMB_ADDR_PCM               (0 | PMB_BUS_PCM << PMB_BUS_ID_SHIFT)

#define PMB_ZONES_PCM              2

enum {
    PCM_Zone_Main,
    PCM_Zone_PCM=3,
};

//--------- SOFT Reset bits for PCM ------------------------
#define   BPCM_PCM_SRESET_HARDRST_N   0x00000004
#define   BPCM_PCM_SRESET_PCM_N       0x00000040
#define   BPCM_PCM_SRESET_BUS_N       0x00000001
