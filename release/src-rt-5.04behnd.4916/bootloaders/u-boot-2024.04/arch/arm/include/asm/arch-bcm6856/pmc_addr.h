#define PMB_BUS_MAX              2
#define PMB_BUS_ID_SHIFT         12

#define PMB_BUS_PERIPH           0
#define PMB_ADDR_PERIPH          (0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         4

#define PMB_BUS_MEMC             0
#define PMB_ADDR_MEMC            (1 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_PVTMON           0
#define PMB_ADDR_PVTMON          (2 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON         0

#define PMB_BUS_CHIP_CLKRST      0
#define PMB_ADDR_CHIP_CLKRST     (3 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define PMB_BUS_USB30_2X         0
#define PMB_ADDR_USB30_2X        (4 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X       4

#define PMB_BUS_SYSPLL           0
#define PMB_ADDR_SYSPLL          (5 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL         0

#define PMB_BUS_RDPPLL           0
#define PMB_ADDR_RDPPLL          (6 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL         0

#define PMB_BUS_PCIE0            1
#define PMB_ADDR_PCIE0           (7 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0          1

#define PMB_BUS_PCIE1            1
#define PMB_ADDR_PCIE1           (8 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1          1

#define PMB_BUS_PCIE2            1
#define PMB_ADDR_PCIE2           (9 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2          1

#define PMB_BUS_XRDP             1
#define PMB_ADDR_XRDP            (10 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP           3

#define PMB_BUS_XRDP_RC0         1
#define PMB_ADDR_XRDP_RC0        (11 | PMB_BUS_XRDP_RC0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC0       1

#define PMB_BUS_XRDP_RC1         1
#define PMB_ADDR_XRDP_RC1        (12 | PMB_BUS_XRDP_RC1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC1       1

#define PMB_BUS_XRDP_RC2         1
#define PMB_ADDR_XRDP_RC2        (13 | PMB_BUS_XRDP_RC2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC2       1

#define PMB_BUS_XRDP_RC3         1
#define PMB_ADDR_XRDP_RC3        (14 | PMB_BUS_XRDP_RC3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC3       1

#define PMB_BUS_XRDP_RC4         1
#define PMB_ADDR_XRDP_RC4        (15 | PMB_BUS_XRDP_RC4 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC4       1

#define PMB_BUS_XRDP_RC5         1
#define PMB_ADDR_XRDP_RC5        (16 | PMB_BUS_XRDP_RC5 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC5       1

#define PMB_BUS_XRDP_RC6         1
#define PMB_ADDR_XRDP_RC6        (17 | PMB_BUS_XRDP_RC6 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC6       1

#define PMB_BUS_XRDP_RC7         1
#define PMB_ADDR_XRDP_RC7        (18 | PMB_BUS_XRDP_RC7 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC7       1

#define PMB_BUS_WAN              1
#define PMB_ADDR_WAN             (19 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            6

#define PMB_BUS_ORION_CPU0         0
#define PMB_ADDR_ORION_CPU0        (32 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0       1

#define PMB_BUS_ORION_CPU1         0
#define PMB_ADDR_ORION_CPU1        (33 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1       1

#define PMB_BUS_ORION_NONCPU       0
#define PMB_ADDR_ORION_NONCPU      (36 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU     1

#define PMB_BUS_BIU_PLL            0
#define PMB_ADDR_BIU_PLL           (38 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL          1

#define PMB_BUS_BIU_BPCM           0
#define PMB_ADDR_BIU_BPCM          (39 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM         1
