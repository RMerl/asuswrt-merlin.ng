/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <linux/gfp.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include "pmc_drv.h"
#include "BPCM.h"
#include "bcm_map_part.h"
#include "bcm_ubus4.h"
#include <linux/delay.h>
#include <linux/irqflags.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>

struct bpcm_device {
        unsigned char *name;
        unsigned devno;
        unsigned zones;
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || \
    defined(CONFIG_BCM96856)
        unsigned ucbid;
#endif
};

#define UCB_HAS_MST_SLV     (1<<8)
#define UCBID_MASK          0xff

/* device blocks with their device and zone numbers */
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
        { "apm",               PMB_ADDR_APM,               PMB_ZONES_APM               },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA              },
#if defined(CONFIG_BCM963138)
        { "aip",               PMB_ADDR_AIP,               PMB_ZONES_AIP               },
#elif defined(CONFIG_BCM963148)
        { "urb",               PMB_ADDR_URB,               PMB_ZONES_URB               },
        { "b15_cpu0",          PMB_ADDR_B15_CPU0,          PMB_ZONES_B15_CPU0          },
        { "b15_cpu1",          PMB_ADDR_B15_CPU1,          PMB_ZONES_B15_CPU1          },
        { "b15_l2",            PMB_ADDR_B15_L2,            PMB_ZONES_B15_L2            },
        { "b15_pll",           PMB_ADDR_B15_PLL,           PMB_ZONES_B15_PLL           },
#endif
        { "dect_ubus",         PMB_ADDR_DECT_UBUS,         PMB_ZONES_DECT_UBUS         },
        { "sar",               PMB_ADDR_SAR,               PMB_ZONES_SAR               },
        { "rdp",               PMB_ADDR_RDP,               PMB_ZONES_RDP               },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "vdsl3_mips",        PMB_ADDR_VDSL3_MIPS,        PMB_ZONES_VDSL3_MIPS        },
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
        { "vdsl3_afepll",      AFEPLL_PMB_ADDR_VDSL3_CORE, AFEPLL_PMB_ZONES_VDSL3_CORE },
#elif defined(CONFIG_BCM963381)
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "mips",              PMB_ADDR_MIPS,              PMB_ZONES_MIPS              },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "sar",               PMB_ADDR_SAR,               PMB_ZONES_SAR               },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "syspll1",           PMB_ADDR_SYSPLL1,           PMB_ZONES_SYSPLL1           },
        { "usb2x",             PMB_ADDR_USB2X,             PMB_ZONES_USB2X             },
        { "usb30",             PMB_ADDR_USB30,             PMB_ZONES_USB30             },
        { "vdsl3_afepll",      AFEPLL_PMB_ADDR_VDSL3_CORE, AFEPLL_PMB_ZONES_VDSL3_CORE },
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
        { "vdsl3_mips",        PMB_ADDR_VDSL3_MIPS,        PMB_ZONES_VDSL3_MIPS        },
#elif defined(CONFIG_BCM96848)
        { "usb2x",             PMB_ADDR_USB2X,             PMB_ZONES_USB2X             },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "syspll0",           PMB_ADDR_SYSPLL0,           PMB_ZONES_SYSPLL0           },
        { "syspll1",           PMB_ADDR_SYSPLL1,           PMB_ZONES_SYSPLL1           },
        { "mips",              PMB_ADDR_MIPS,              PMB_ZONES_MIPS              },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "rdp",               PMB_ADDR_RDP,               PMB_ZONES_RDP               },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN               },
        { "sgmii",             PMB_ADDR_SGMII,             PMB_ZONES_SGMII             },
#elif defined CONFIG_BCM94908
        { "b53pll",            PMB_ADDR_B53PLL,            PMB_ZONES_B53PLL            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "crypto",            PMB_ADDR_CRYPTO,            PMB_ZONES_CRYPTO            },
        { "dqm",               PMB_ADDR_DQM,               PMB_ZONES_DQM               },
        { "fpm",               PMB_ADDR_FPM,               PMB_ZONES_FPM               },
        { "gmac",              PMB_ADDR_GMAC,              PMB_ZONES_GMAC              },
        { "i2spll",            PMB_ADDR_I2SPLL,            PMB_ZONES_I2SPLL            },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2             },
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "rdp",               PMB_ADDR_RDP,               PMB_ZONES_RDP               },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA              },
        { "sgmii",             PMB_ADDR_SGMII,             PMB_ZONES_SGMII             },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "swtpll",            PMB_ADDR_SWTPLL,            PMB_ZONES_SWTPLL            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "urb",               PMB_ADDR_URB,               PMB_ZONES_URB               },
        { "usb",               PMB_ADDR_USB,               PMB_ZONES_USB               },
#elif defined(CONFIG_BCM96858)
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH,        0                                       },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST,   0                                       },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL,        0                                       },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL,        0                                       },
        { "unipll",            PMB_ADDR_UNIPLL,            PMB_ZONES_UNIPLL,        0                                       },
        { "lport",             PMB_ADDR_LPORT,             PMB_ZONES_LPORT,         UCB_NODE_ID_SLV_LPORT                   },
        { "usb30",             PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X,      UCB_NODE_ID_MST_USB | UCB_HAS_MST_SLV   },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN,           UCB_NODE_ID_SLV_WAN                     },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP,          0                                       },
        { "xrdp_qm",           PMB_ADDR_XRDP_QM,           PMB_ZONES_XRDP_QM,       0                                       },
        { "xrdp_rc_quad0",     PMB_ADDR_XRDP_RC_QUAD0,     PMB_ZONES_XRDP_RC_QUAD0, 0                                       },
        { "xrdp_rc_quad1",     PMB_ADDR_XRDP_RC_QUAD1,     PMB_ZONES_XRDP_RC_QUAD1, 0                                       },
        { "xrdp_rc_quad2",     PMB_ADDR_XRDP_RC_QUAD2,     PMB_ZONES_XRDP_RC_QUAD2, 0                                       },
        { "xrdp_rc_quad3",     PMB_ADDR_XRDP_RC_QUAD3,     PMB_ZONES_XRDP_RC_QUAD3, 0                                       },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0,         UCB_NODE_ID_MST_PCIE0 | UCB_HAS_MST_SLV },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA,          UCB_NODE_ID_MST_SATA | UCB_HAS_MST_SLV  },
        { "pcie_ubus",         PMB_ADDR_PCIE_UBUS,         PMB_ZONES_PCIE_UBUS,     UCB_NODE_ID_MST_PCIE2 | UCB_HAS_MST_SLV },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0,    0                                       },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1,    0                                       },
        { "orion_cpu2",        PMB_ADDR_ORION_CPU2,        PMB_ZONES_ORION_CPU2,    0                                       },
        { "orion_cpu3",        PMB_ADDR_ORION_CPU3,        PMB_ZONES_ORION_CPU3,    0                                       },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU,  0                                       },
        { "orion_ars",         PMB_ADDR_ORION_ARS ,        PMB_ZONES_ORION_ARS,     0                                       },
        { "biupll",            PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL,       0                                       },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM,      0                                       },
#elif defined(CONFIG_BCM96836) || defined(CONFIG_BCM96856) 
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH,        0                                       },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST,   0                                       },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL,        0                                       },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL,        0                                       },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC,          UCB_NODE_ID_SLV_MEMC                    },
        { "usb30",             PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X,      UCB_NODE_ID_MST_USB | UCB_HAS_MST_SLV   },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN,           0                                       },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP,          0                                       },
        { "xrdp_rc0",          PMB_ADDR_XRDP_RC0,          PMB_ZONES_XRDP_RC0,      0                                       },
        { "xrdp_rc1",          PMB_ADDR_XRDP_RC1,          PMB_ZONES_XRDP_RC1,      0                                       },
        { "xrdp_rc2",          PMB_ADDR_XRDP_RC2,          PMB_ZONES_XRDP_RC2,      0                                       },
        { "xrdp_rc3",          PMB_ADDR_XRDP_RC3,          PMB_ZONES_XRDP_RC3,      0                                       },
        { "xrdp_rc4",          PMB_ADDR_XRDP_RC4,          PMB_ZONES_XRDP_RC4,      0                                       },
        { "xrdp_rc5",          PMB_ADDR_XRDP_RC5,          PMB_ZONES_XRDP_RC5,      0                                       },
#if defined(CONFIG_BCM96856)
        { "xrdp_rc6",          PMB_ADDR_XRDP_RC6,          PMB_ZONES_XRDP_RC6,      0                                       },
        { "xrdp_rc7",          PMB_ADDR_XRDP_RC7,          PMB_ZONES_XRDP_RC7,      0                                       },
#endif
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0,         0                                       },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1,         0                                       },
#if defined(CONFIG_BCM96836)
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA,          UCB_NODE_ID_MST_SATA | UCB_HAS_MST_SLV  },
        { "pcie_ubus",         PMB_ADDR_PCIE_UBUS,         PMB_ZONES_PCIE_UBUS,     UCB_NODE_ID_MST_SATA | UCB_HAS_MST_SLV  },
#else
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2,         0                                       },
#endif
#elif defined(CONFIG_BCM96846)
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH,        0                                       },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST,   0                                       },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL,        0                                       },
        { "pvtmon",            PMB_ADDR_PVTMON,            PMB_ZONES_PVTMON,        0                                       },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC,          UCB_NODE_ID_SLV_MEMC                    },
        { "usb20",             PMB_ADDR_USB20_2X,          PMB_ZONES_USB20_2X,      UCB_NODE_ID_MST_USB | UCB_HAS_MST_SLV   },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN,           0                                       },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP,          0                                       },
        { "xrdp_rc0",          PMB_ADDR_XRDP_RC0,          PMB_ZONES_XRDP_RC0,      0                                       },
        { "xrdp_rc1",          PMB_ADDR_XRDP_RC1,          PMB_ZONES_XRDP_RC1,      0                                       },
        { "xrdp_rc2",          PMB_ADDR_XRDP_RC2,          PMB_ZONES_XRDP_RC2,      0                                       },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0,         0                                       },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1,         0                                       },
        { "biupll",            PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL,       0                                       },
        { "biubpcm",           PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM,      0                                       },
#elif defined(CONFIG_BCM963158)
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA              },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_cpu2",        PMB_ADDR_ORION_CPU2,        PMB_ZONES_ORION_CPU2        },
        { "orion_cpu3",        PMB_ADDR_ORION_CPU3,        PMB_ZONES_ORION_CPU3        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
        { "orion_ars",         PMB_ADDR_ORION_ARS,         PMB_ZONES_ORION_ARS         },
        { "orion_c0_ars",      PMB_ADDR_ORION_C0_ARS,      PMB_ADDR_ORION_C0_ARS       },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "ubus_pll",          PMB_ADDR_UBUSPLL,           PMB_ZONES_UBUSPLL           },
        { "sync_pll",          PMB_ADDR_SYNC_PLL,          PMB_ZONES_SYNC_PLL          },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2             },
        { "pcie3",             PMB_ADDR_PCIE3,             PMB_ZONES_PCIE3             },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN               },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP              },
        { "xrdp_rc0",          PMB_ADDR_XRDP_RC0,          PMB_ZONES_XRDP_RC0          },
        { "xrdp_rc1",          PMB_ADDR_XRDP_RC1,          PMB_ZONES_XRDP_RC1          },
        { "xrdp_rc2",          PMB_ADDR_XRDP_RC2,          PMB_ZONES_XRDP_RC2          },
        { "xrdp_rc3",          PMB_ADDR_XRDP_RC3,          PMB_ZONES_XRDP_RC3          },
        { "xrdp_rc4",          PMB_ADDR_XRDP_RC4,          PMB_ZONES_XRDP_RC4          },
        { "xrdp_rc5",          PMB_ADDR_XRDP_RC5,          PMB_ZONES_XRDP_RC5          },
        { "sgmii",             PMB_ADDR_SGMII,             PMB_ZONES_SGMII             },
        { "crypto",            PMB_ADDR_CRYPTO,            PMB_ZONES_CRYPTO            },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "vdsl3_pmd",         PMB_ADDR_VDSL3_PMD,         PMB_ZONES_VDSL3_PMD         },
        { "vdsl3_mips",        PMB_ADDR_VDSL3_MIPS,        PMB_ZONES_VDSL3_MIPS        },
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
#endif
};

/* block common register templates */
static const struct attribute blockreg_attrs[] = {
        { "id",                  0644 }, /* offset = 0x00, actual offset =  0 */
        { "capabilities",        0644 }, /* offset = 0x04, actual offset =  1 */
#if defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        { "rsvd2",               0644 }, /* offset = 0x08, actual offset = 2 */
        { "rsvd3",               0644 }, /* offset = 0x0c, actual offset = 3 */
        { "control",             0644 }, /* offset = 0x10, actual offset = 4 */
        { "sr_control",          0644 }, /* offset = 0x14, actual offset = 5 */
        { "rsvd6",               0644 }, /* offset = 0x18, actual offset = 6 */
        { "rsvd7",               0644 }, /* offset = 0x1c, actual offset = 7 */
        { "client_sp0",          0644 }, /* offset = 0x20, actual offset = 8 */
        { "client_sp1",          0644 }, /* offset = 0x24, actual offset = 9 */
        { "client_sp2",          0644 }, /* offset = 0x28, actual offset = 10 */
        { "client_sp3",          0644 }, /* offset = 0x2c, actual offset = 11 */
#else
        { "control",             0644 }, /* offset = 0x08, actual offset =  2 */
        { "status",              0644 }, /* offset = 0x0c, actual offset =  3 */
        { "rosc_control",        0644 }, /* offset = 0x10, actual offset =  4 */
        { "rosc_thresh_h",       0644 }, /* offset = 0x14, actual offset =  5 */
        { "rosc_thresh_s",       0644 }, /* offset = 0x18, actual offset =  6 */
        { "rosc_count",          0644 }, /* offset = 0x1c, actual offset =  7 */
        { "pwd_control",         0644 }, /* offset = 0x20, actual offset =  8 */
        { "pwd_accum_control",   0644 }, /* offset = 0x24, actual offset =  9 */
        { "sr_control",          0644 }, /* offset = 0x28, actual offset = 10 */
        { "global_control",      0644 }, /* offset = 0x2c, actual offset = 11 */
        { "misc_control",        0644 }, /* offset = 0x30, actual offset = 12 */
        { "rsvd13",              0644 }, /* offset = 0x34, actual offset = 13 */
        { "rsvd14",              0644 }, /* offset = 0x38, actual offset = 14 */
        { "rsvd15",              0644 }, /* offset = 0x3c, actual offset = 15 */
#endif
};

/* block common register templates */
static const struct attribute zonereg_attrs[] = {
        { "control",             0644 },
        { "config1",             0644 },
        { "config2",             0644 },
        { "freq_scalar_control", 0644 },
};

/* attribute subclass with device and zone */
struct bpcm_attribute {
        struct attribute attr;
        ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf);
        ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count);
        int devno;
        int zoneno;
        int islandno;
        int regno;
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        int ucbid;
#endif
};

static ssize_t power_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned zone = pmc->zoneno >= 0 ? pmc->zoneno : 0;
        BPCM_PWR_ZONE_N_CONTROL control;
        int rc;

        rc = ReadZoneRegister(pmc->devno, zone, BPCMZoneRegOffset(control), &control.Reg32);
        if (rc)
                return sprintf(buf, "name %s, dev %d, error %x\n", pmc->attr.name, pmc->devno, rc);
        if (control.Reg32 == 0xdeaddead)
                return sprintf(buf, "%x\n", (unsigned) control.Reg32);
        return sprintf(buf, "%d\n", control.Bits.pwr_on_state);
}

static ssize_t power_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned value;

        if (kstrtouint(buf, 0, &value)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        if (value) {
                if (pmc->zoneno < 0)
                {
                        PowerOnDevice(pmc->devno);
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
                        ubus_register_port(pmc->ucbid & UCBID_MASK);
                        if (pmc->ucbid & UCB_HAS_MST_SLV)
                            ubus_register_port((pmc->ucbid & UCBID_MASK)+1);
#endif
                }
                else
                        PowerOnZone(pmc->devno, pmc->zoneno);
        } else {
                if (pmc->zoneno < 0)
                {
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) ||defined(CONFIG_BCM96856)
                        ubus_deregister_port(pmc->ucbid);
                        if (pmc->ucbid & UCB_HAS_MST_SLV)
                            ubus_deregister_port((pmc->ucbid & UCBID_MASK)+1);
#endif
                        PowerOffDevice(pmc->devno, 0); // dev, repower
                }
                else
                        PowerOffZone(pmc->devno, pmc->zoneno);
        }
        return count;
}

static ssize_t reset_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned zone = pmc->zoneno >= 0 ? pmc->zoneno : 0;
        BPCM_PWR_ZONE_N_CONTROL control;
        int rc;

        rc = ReadZoneRegister(pmc->devno, zone, BPCMZoneRegOffset(control), &control.Reg32);
        if (rc)
                return sprintf(buf, "name %s, dev %d, error %x\n", pmc->attr.name, pmc->devno, rc);
        if (control.Reg32 == 0xdeaddead)
                return sprintf(buf, "%x\n", (unsigned) control.Reg32);
        return sprintf(buf, "%d\n", control.Bits.reset_state);
}

static ssize_t reset_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        unsigned value;

        if (kstrtouint(buf, 0, &value)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }
        if (value) {
                if (pmc->zoneno < 0)
                        ResetDevice(pmc->devno);
                else
                        ResetZone(pmc->devno, pmc->zoneno);
        }
        return count;
}

/* node templates */
static const struct bpcm_attribute pseudo_attrs[] = {
        { { "power",       0644, }, power_show,       power_store },
        { { "reset",       0644, }, reset_show,       reset_store },
};

static ssize_t regs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        uint32 value;
        int rc;

        if (pmc->zoneno < 0)
                rc = ReadBPCMRegister(pmc->devno, pmc->regno, &value);
        else
                rc = ReadZoneRegister(pmc->devno, pmc->zoneno, pmc->regno, &value);
        if (rc == 0)
                return sprintf(buf, "%x\n", (unsigned) value);
        else
                return sprintf(buf, "name %s, dev %d, error %x\n", pmc->attr.name, pmc->devno, rc);
}

static ssize_t regs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) attr;
        uint32 value;
        int rc;

        if (kstrtou32(buf, 0, (u32 *)&value)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        if (pmc->zoneno < 0)
                rc = WriteBPCMRegister(pmc->devno, pmc->regno, value);
        else
                rc = WriteZoneRegister(pmc->devno, pmc->zoneno, pmc->regno, value);
        if (rc)
                return 0;
        return count;
}

static int pmc_sysfs_init_zones(const struct bpcm_device *dev, struct kobject *kobj)
{
        unsigned z;
        unsigned r;
        int error;

        for (z = 0; z < dev->zones; z++) {
                unsigned char name[8];
                struct kobject *zoneobj;
                struct bpcm_attribute *pmcattr;

                /* create subdirectory for each zone */
                snprintf(name, sizeof name, "%s%d", "zone", z);
                zoneobj = kobject_create_and_add(name, kobj);
                if (zoneobj == 0)
                        return -ENOENT;

                /* create zone's power and reset nodes */
                r = ARRAY_SIZE(pseudo_attrs);
                pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
                while (r--) {
                        sysfs_attr_init(&pmcattr[r].attr);
                        pmcattr[r] = pseudo_attrs[r];
                        pmcattr[r].devno = dev->devno;
                        pmcattr[r].zoneno = z;
                        pmcattr[r].regno = -1;
                        error = sysfs_create_file(zoneobj, (struct attribute *)&pmcattr[r]);
                }

                /* create some register nodes */
                r = ARRAY_SIZE(zonereg_attrs);
                pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
                while (r--) {
                        sysfs_attr_init(&pmcattr[r].attr);
                        pmcattr[r].attr = zonereg_attrs[r];
                        pmcattr[r].store = regs_store;
                        pmcattr[r].show = regs_show;
                        pmcattr[r].devno = dev->devno;
                        pmcattr[r].zoneno = z;
                        pmcattr[r].regno = r;
                        error = sysfs_create_file(zoneobj, (struct attribute *)&pmcattr[r]);
                }
        }

        return error;
}

static int pmc_sysfs_init_block(const struct bpcm_device *dev, struct kobject *kobj)
{
        struct bpcm_attribute *pmcattr;
        struct kobject *blkobj;
        unsigned r;
        int error;

        /* create subdirectory for each block */
        blkobj = kobject_create_and_add(dev->name, kobj);
        if (blkobj == 0)
                return -ENOMEM;

        /* create block's power and reset nodes */
        if (dev->zones) {
                r = ARRAY_SIZE(pseudo_attrs);
                pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
                while (r--) {
                        sysfs_attr_init(&pmcattr[r].attr);
                        pmcattr[r] = pseudo_attrs[r];
                        pmcattr[r].devno = dev->devno;
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
                        pmcattr[r].ucbid = dev->ucbid;
#endif
                        pmcattr[r].zoneno = -1;
                        pmcattr[r].regno = -1;
                        error = sysfs_create_file(blkobj, (struct attribute *)&pmcattr[r]);
                }
        }

        /* create some register nodes */
        r = ARRAY_SIZE(blockreg_attrs);
        pmcattr = kmalloc(r * sizeof *pmcattr, GFP_KERNEL);
        while (r--) {
                sysfs_attr_init(&pmcattr[r].attr);
                pmcattr[r].attr = blockreg_attrs[r];
                pmcattr[r].store = regs_store;
                pmcattr[r].show = regs_show;
                pmcattr[r].devno = dev->devno;
                pmcattr[r].zoneno = -1;
                pmcattr[r].regno = r;
                error = sysfs_create_file(blkobj, (struct attribute *)&pmcattr[r]);
        }

        return pmc_sysfs_init_zones(dev, blkobj);
}

static ssize_t revision_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
        struct attribute *attr = (struct attribute *)kattr;
        int revision, change;
        int rc;

        rc = GetRevision(&change, &revision);
        if (rc != kPMC_NO_ERROR)
                return sprintf(buf, "name %s, error %x\n", attr->name, rc);
        return sprintf(buf, "%x-%d\n", revision, change);
}

static ssize_t pvtmon_convert(char *buf, int sel, int value)
{
    int val, val0;
        switch (sel)  {
        case 0:
#if defined CONFIG_BCM963148 /*pvt2*/
        val = (38887551 - (466415 * value / 10)) / 10000;
        val0 = (38887551 - (466415 * value / 10)) % 10000;
#elif defined CONFIG_BCM96846
        val = ((41335000 - (49055 * value))/100000);
        val0 = ((41335000 - (49055 * value))%100000);
#else
        val = (41004000 - 48705 * value) / 100000;
        val0 = (41004000 - 48705 * value) % 100000;
#endif
            return sprintf(buf, "%s DieTemp: %d.%2dC\n", buf, val, val0);

        case 1: 
        case 2:
            val = (880 * value * 10) / (10 * 1024)/1000;
            val0 = (880 * value * 10) / (10 * 1024)%1000;
            return sprintf(buf, "%s V0.85_%d: %d.%03d V\n", buf, sel, val, val0 );
        case 3:  
            val = (880 * value * 10) / (7 * 1024)/1000;
            val0 = (880 * value * 10) / (7 * 1024)%1000;
            return sprintf(buf, "%s VIN: %d.%03d V\n", buf, val, val0);
        case 4:  
            val = (880 * value * 10) / (7 * 1024)/1000;
            val0 = (880 * value * 10) / (7 * 1024)%1000;
            return sprintf(buf, "%s V1.0: %d.%03d V\n", buf, val, val0);
        case 5:  
            val = (880 * value * 10) / (4 * 1024)/1000;
            val0 = (880 * value * 10) / (4 * 1024)%1000;
            return sprintf(buf, "%s V1.8: %d.%03d V\n", buf, val, val0);
        case 6:  
            val = (880 * value * 10) / (2 * 1024)/1000;
            val0 = (880 * value * 10) / (2 * 1024)%1000;
            return sprintf(buf, "%s V3.3: %d.%03d V\n", buf, val, val0);
        case 8:  return sprintf(buf, "%s kTEST: %d \n", buf,  value >> 16);
        default: return 0;
        }
}

static ssize_t pvtmon_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
        struct bpcm_attribute *pmc = (struct bpcm_attribute *) kattr;
        int rc, value;

        sprintf(buf, "");
        rc = GetPVT(pmc->regno, pmc->islandno, &value);
        if (rc != kPMC_NO_ERROR)
                return sprintf(buf, "name %s, error %x\n", pmc->attr.name, rc);

        return pvtmon_convert(buf, pmc->regno, value);
}

static ssize_t clock_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
#if defined(__AARCH64EL__)
        unsigned long time1, time2, ctrl, flags = 0;
        /* Enable cycle counter */
        asm volatile("MRS %0, PMCNTENSET_EL0" : "=r" (ctrl));
        ctrl |= 0x80000000;
        asm volatile("MSR PMCNTENSET_EL0, %0" :: "r" (ctrl));

        /* Enable PMU */
        ctrl = 0x1;
        asm volatile("MSR PMCR_EL0, %0" :: "r" (ctrl));

        /* Count CPU cycle for 1 ms to get the CPU frequency */
        local_irq_save(flags);
        asm volatile("MRS %0, PMCCNTR_EL0" : "=r" (time1));
        mdelay(1);
        asm volatile("MRS %0, PMCCNTR_EL0" : "=r" (time2));
        local_irq_restore(flags);

        printk ("CPU Frequency %d MHz\n", (unsigned int)(time2 - time1) / 1000);
#endif
        return 0;
}

static ssize_t clock_set(struct kobject *kobj, struct kobj_attribute *kattr, const char *buf, size_t count)
{
#if defined (CONFIG_BCM963158)
        unsigned mdiv0;
        PLL_CHCFG_REG pll_ch01_cfg;

        if (kstrtouint(buf, 0, &mdiv0)) {
                printk("%s: not a number\n", buf);
                return -EEXIST;
        }

        ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch01_cfg), &pll_ch01_cfg.Reg32);

        if (mdiv0 == 0) {
                printk("CPU is running with clock divider %d\n", pll_ch01_cfg.Bits.mdiv0);
                return count;
        }

        if (pll_ch01_cfg.Bits.mdiv0 != mdiv0) {
                pll_ch01_cfg.Bits.mdiv0 = mdiv0;
                WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch01_cfg), pll_ch01_cfg.Reg32);
                mdelay(1);
                pll_ch01_cfg.Bits.mdiv_override0 = 1;
                WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch01_cfg), pll_ch01_cfg.Reg32);
                mdelay(1);
        }

        clock_show (NULL, NULL, NULL);
#else
        printk ("Not implemented\n");
#endif
        return count;
}


#ifdef PVTMON_REG
static ssize_t volts_show (void *pvtreg)
{
        unsigned int ix, accum, data, sel = 3;

        pvtmon_regs *pvtmon = (pvtmon_regs*)pvtreg;
        /* Disable auto scan */
        pvtmon->ascan_config &= ~1;

        /* Select voltage + clock_en + power_on */
        iowrite32(sel << 4 | 1 << 3 | 1 << 1, &pvtmon->control);
        udelay(50);
        /* Select voltage + clock_en + power_on + un-reset */
        iowrite32(sel << 4 | 1 << 3 | 1 << 1 | 1 << 0, &pvtmon->control);
        /* Read once to make stale */
        ioread32(&pvtmon->adc_data);
        /* Confirm, not to read stale data */
        for (ix = 0; ix < 10; ix++) {
              do {
                    data = ioread32(&pvtmon->adc_data);
              }while (data & (1 << 15));  /* wait for non-stale data */
        }
        /* Read 32 times and then average them up */
        accum = 0;
        for (ix = 0; ix < 8; ix++) {
              do {
                    data = ioread32(&pvtmon->adc_data);
              }while (data & (1 << 15));  /* wait for non-stale data */
              accum += data & 0x3FF;      /* accumulate */
        }
        printk("Input voltage %dmV\n", ((accum / 8) * 880 * 10) / (7 * 1024));
        /* Enable auto scan */
        pvtmon->ascan_config |= 1;
        return 0;
}
#endif

static ssize_t volts_set (struct kobject *kobj, struct kobj_attribute *kattr, const char *buf, size_t count)
{
#ifdef PVTMON_REG
        pvtmon_regs *pvtmon;
        unsigned int address, value, cfg;
        char s_addr[16];
        char s_val[16];

        sscanf(buf, "%s %s", s_addr, s_val);
        kstrtouint (s_addr, 0, &address);
        kstrtouint (s_val, 0, &value);
        pvtmon = (pvtmon_regs *)ioremap_nocache(address, 0x100);

        if (value) {
              /* Write the DAC code */
              iowrite32(value, &pvtmon->vref_data);
              /* Set PVTMON in AVS mode (3) to be able to control the DAC voltage */
              cfg = (ioread32(&pvtmon->cfg_lo) & ~(0x7 << 10)) | 3 << 10;
              iowrite32(cfg, &pvtmon->cfg_lo);
        }
        else{
              printk ("DAC value %d\n", ioread32(&pvtmon->vref_data));
        }

        volts_show((void*)pvtmon);
        iounmap((void*)pvtmon);
#endif
        return count;
}

#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM96846) 
static ssize_t pmc_log_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    static unsigned long virt = 0;
    uint32_t phys;
    uint32_t i;

    if (virt == 0)
    {
        phys = 0x40fc000;
        virt = (unsigned long)phys_to_virt(phys);
    }

    if (*((uint32_t *)(virt - 4)) != 0xc0ffee55)
    {
        printk("Log is not enabled in PMC\n");
        return 0;
    }

    for (i = 0; i< 0x4000; i++)
    {
        char *ch = (char *)(virt + i);

        printk("%c", *ch);
    }

    return 0;
}
#endif

#if 0 /*defined(CONFIG_BCM96846)*/
#include <asm/mach/map.h>
extern void create_mapping(struct map_desc *md);

static ssize_t pmc_log_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    static struct map_desc desc = {};
    uint32_t i;
    char *ch;

    if (desc.virtual == 0)
    {
        desc.virtual = (unsigned long)phys_to_virt(0x4000000);
        desc.pfn = __phys_to_pfn(0x4000000);
        desc.length = 0x100000;
        desc.type = MT_MEMORY_RWX_NONCACHED;

        create_mapping(&desc);
    }

    ch = (char *)(desc.virtual + 0x0fc000);

    if (*((uint32_t *)(ch - 4)) != 0xc0ffee55)
    {
        printk("Log is not enabled in PMC\n");
        return 0;
    }

    for (i = 0; i< 0x4000; i++)
    {
        printk("%c", *ch);
        ch++;
    }

    return 0;
}
#endif

#if defined(CONFIG_BCM96856)
struct bpcm
{
    int addr;
    int bus;
};

char *common_csv_header = "Temp,VIN";
struct bpcm map[]={{37,0},{45,0},{48,0},{49,1},{50,1}, {51,1}, {52,1}, {53,1}, {54,1}, {55,1}, {56,1}, {57,1}, {58,1}, {59,1}, {60,0}, {61, 0}};
char *csv_header = "ORION_0(37)SVT,ORION_0(37)HVT,ORION_1(45)SVT,ORION_1(45)HVT,PERIPH(48)SVT,PERIPH(48)HVT,WAN(49)SVT,"            "WAN(49)HVT,XRDP(50)SVT,XRDP(50)HVT,XRDP_RC0(51)SVT,XRDP_RC0(51)HVT,XRDP_RC1(52)SVT,XRDP_RC1(52)HVT,"
            "XRDP_RC2(53)SVT,XRDP_RC2(53)HVT,XRDP_RC3(54)SVT,XRDP_RC3(54)HVT,XRDP_RC4(55)SVT,XRDP_RC4(55)HVT,"
            "XRDP_RC5(56)SVT,XRDP_RC5(56)HVT,XRDP_RC6(57)SVT,XRDP_RC6(57)HVT,XRDP_RC7(58)SVT,XRDP_RC7(58)HVT,"
            "PCIE(59)SVT,PCIE(59)HVT,USB(60)SVT,USB(60)HVT,MEMC(61)SVT,MEMC(61)HVT,"
            "SVT THRESH HI,SVT THRESH LO,HVT THRESH HI,HVT THRESH LO, LVT TRESH HI,LVT TRESH LO\n";
#define ROSC_COUNT_OFF 11
#define ROSC_CONFIG_OFF 8
static int event_counters_enabled = 0;
static ssize_t get_roscs(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    int i;
    int rc;
    int temperature;
    int vin;
    int val, val0;
    int valin, valin0;
    uint32_t target;
    uint32_t num_bpcms = sizeof(map)/sizeof(struct bpcm);
    uint32_t thresh_s, thresh_h, thresh_l;

    if(!event_counters_enabled)
    {
        for (i=0; i < num_bpcms; i++)
        {
            target = 0xffff00ff;
            rc = WriteBPCMRegister((map[i].bus<<PMB_BUS_ID_SHIFT) | map[i].addr, ROSC_CONFIG_OFF, target);
        }
        event_counters_enabled = 1;

        sprintf(buf,"%s,%s",common_csv_header, csv_header);
    }
    else
    {
        sprintf(buf, "\n");
    }

    rc = ReadBPCMRegister((map[1].bus<<PMB_BUS_ID_SHIFT) | map[1].addr,  9, &thresh_h);
    rc = ReadBPCMRegister((map[0].bus<<PMB_BUS_ID_SHIFT) | map[0].addr, 10, &thresh_s);
    rc = ReadBPCMRegister((map[0].bus<<PMB_BUS_ID_SHIFT) | map[0].addr,  9, &thresh_l);
    rc = GetPVT(0, 0, &temperature);

    val = ((41335000 - (49055 * temperature))/100000);
    val0 = ((41335000 - (49055 * temperature))%100000);

    rc = GetPVT(3, 0, &vin);
    valin = (880 * vin * 10) / (7 * 1024)/1000;
    valin0 = (880 * vin * 10) / (7 * 1024)%1000;

    sprintf(buf, "%s%d.%03d,%d.%03d", buf,val, val0, valin, valin0);

    for (i=0; i < num_bpcms; i++)
    {
        target = 0;
        rc = ReadBPCMRegister((map[i].bus<<PMB_BUS_ID_SHIFT) | map[i].addr, ROSC_COUNT_OFF, &target);

        sprintf(buf, "%s,%04d,%04d", buf, target&0xffff,(target&0xffff0000)>>16 );
    }
    return sprintf(buf, "%s,%d,%d,%d,%d,%d,%d\n",buf,(thresh_s & 0xffff0000)>>16, thresh_s &
        0xffff, (thresh_h & 0xffff0000)>>16, thresh_h & 0xffff, (thresh_l & 0xffff0000)>>16, thresh_l & 0xffff);
}

static ssize_t get_all_roscs(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
    uint32_t ix, sh_ix;
    uint32_t phys = 0x4ffbc00;
    uint32_t *shmem = phys_to_virt(phys);
    uint32_t rc;

    rc = GetAllROs(phys);
    if (rc != kPMC_NO_ERROR)
    {
        return rc;  
    }

    sprintf(buf, "PVTMON: \n");
    for (sh_ix = 0; sh_ix < 7; sh_ix++)
    {
        pvtmon_convert(buf, sh_ix, shmem[sh_ix]);
    }

    sprintf(buf, "%s Central ROs: \n", buf);
    for (ix = 0; ix < 36; ix++, sh_ix++)
    {
        sprintf(buf, "%s %d) %u;\n", buf, ix, shmem[sh_ix]);
    }

    sprintf(buf, "%s ARS ROs: \n", buf);
    ix = 0;
    while(shmem[sh_ix] != 0xfffffffe)
    {
        sprintf(buf, "%s %d) SVT %d, HVT %d;\n", buf, ix++, shmem[sh_ix], shmem[sh_ix + 1]);
        sh_ix +=2;
    }
    return sprintf(buf, "%s DONE\n", buf);
}

#endif
static const struct bpcm_attribute root_attrs[] = {
        { { "revision",    0444, }, revision_show,    NULL          },
        { { "select0",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 0 },
        { { "select1",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 1 },
        { { "select2",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 2 },
        { { "select3",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 3 },
        { { "select4",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 4 },
        { { "select5",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 5 },
        { { "select6",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 6 },
        { { "select8",     0444, }, pvtmon_show,      NULL, 0, 0, 0, 8 },
        { { "select9",     0444, }, pvtmon_show,      NULL, 0, 0, 1, 3 },
        { { "select10",    0644, }, clock_show,       clock_set, 0, 0, 0, 0 },
        { { "select11",    0644, }, NULL,             volts_set, 0, 0, 0, 0 },
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        { { "pmc_log",     0444, }, pmc_log_show,      NULL, 0, 0, 1, 3 },
#endif
#if defined(CONFIG_BCM96856)
        { { "get_roscs",   0444, }, get_roscs ,        NULL, 0, 0, 0, 0 },
        { { "get_all_roscs",   0444, }, get_all_roscs , NULL, 0, 0, 0, 0 },
#endif
};

#ifdef CONFIG_PM
extern struct kobject *power_kobj; 
#else
struct kobject *power_kobj = NULL;
#endif

static int __init pmc_sysfs_init(void)
{
        struct bpcm_attribute *pmcattr;
        struct kobject *bpcmkobj;
        unsigned d;
        int error;

        /* create power object ourselves? */
        if (!power_kobj) {
                power_kobj = kobject_create_and_add("power", NULL);
                if (!power_kobj)
                        return -ENOENT;
        }

        /* create bpcm subdirectory */
        bpcmkobj = kobject_create_and_add("bpcm", power_kobj);
        if (bpcmkobj == 0)
                return -ENOMEM;

        /* create root nodes */
        d = ARRAY_SIZE(root_attrs);
        pmcattr = kmalloc(d * sizeof *pmcattr, GFP_KERNEL);
        while (d--) {
                sysfs_attr_init(&pmcattr[d].attr);
                pmcattr[d] = root_attrs[d];
                error = sysfs_create_file(bpcmkobj, (struct attribute *)&pmcattr[d]);
        }

        for (d = 0; d < ARRAY_SIZE(bpcm_devs); d++)
                pmc_sysfs_init_block(&bpcm_devs[d], bpcmkobj);
        return 0;
}

late_initcall(pmc_sysfs_init);
