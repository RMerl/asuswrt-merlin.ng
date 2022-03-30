/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2011 Comelit Group SpA, Luca Ceresoli <luca.ceresoli@comelit.it>
 */

#ifndef _OMAP3_REGS_H
#define _OMAP3_REGS_H

/*
 * Register definitions for OMAP3 processors.
 */

/*
 * GPMC_CONFIG1 - GPMC_CONFIG7
 */

/* Values for GPMC_CONFIG1 - signal control parameters */
#define WRAPBURST                     (1 << 31)
#define READMULTIPLE                  (1 << 30)
#define READTYPE                      (1 << 29)
#define WRITEMULTIPLE                 (1 << 28)
#define WRITETYPE                     (1 << 27)
#define CLKACTIVATIONTIME(x)          (((x) & 3) << 25)
#define ATTACHEDDEVICEPAGELENGTH(x)   (((x) & 3) << 23)
#define WAITREADMONITORING            (1 << 22)
#define WAITWRITEMONITORING           (1 << 21)
#define WAITMONITORINGTIME(x)         (((x) & 3) << 18)
#define WAITPINSELECT(x)              (((x) & 3) << 16)
#define DEVICESIZE(x)                 (((x) & 3) << 12)
#define DEVICESIZE_8BIT               DEVICESIZE(0)
#define DEVICESIZE_16BIT              DEVICESIZE(1)
#define DEVICETYPE(x)                 (((x) & 3) << 10)
#define DEVICETYPE_NOR                DEVICETYPE(0)
#define DEVICETYPE_NAND               DEVICETYPE(2)
#define MUXADDDATA                    (1 << 9)
#define TIMEPARAGRANULARITY           (1 << 4)
#define GPMCFCLKDIVIDER(x)            (((x) & 3) << 0)

/* Values for GPMC_CONFIG2 - CS timing */
#define CSWROFFTIME(x)   (((x) & 0x1f) << 16)
#define CSRDOFFTIME(x)   (((x) & 0x1f) <<  8)
#define CSEXTRADELAY     (1 << 7)
#define CSONTIME(x)      (((x) &  0xf) <<  0)

/* Values for GPMC_CONFIG3 - nADV timing */
#define ADVWROFFTIME(x)  (((x) & 0x1f) << 16)
#define ADVRDOFFTIME(x)  (((x) & 0x1f) <<  8)
#define ADVEXTRADELAY    (1 << 7)
#define ADVONTIME(x)     (((x) &  0xf) <<  0)

/* Values for GPMC_CONFIG4 - nWE and nOE timing */
#define WEOFFTIME(x)     (((x) & 0x1f) << 24)
#define WEEXTRADELAY     (1 << 23)
#define WEONTIME(x)      (((x) &  0xf) << 16)
#define OEOFFTIME(x)     (((x) & 0x1f) <<  8)
#define OEEXTRADELAY     (1 << 7)
#define OEONTIME(x)      (((x) &  0xf) <<  0)

/* Values for GPMC_CONFIG5 - RdAccessTime and CycleTime timing */
#define PAGEBURSTACCESSTIME(x)  (((x) &  0xf) << 24)
#define RDACCESSTIME(x)         (((x) & 0x1f) << 16)
#define WRCYCLETIME(x)          (((x) & 0x1f) <<  8)
#define RDCYCLETIME(x)          (((x) & 0x1f) <<  0)

/* Values for GPMC_CONFIG6 - misc timings */
#define WRACCESSTIME(x)        (((x) & 0x1f) << 24)
#define WRDATAONADMUXBUS(x)    (((x) &  0xf) << 16)
#define CYCLE2CYCLEDELAY(x)    (((x) &  0xf) <<  8)
#define CYCLE2CYCLESAMECSEN    (1 << 7)
#define CYCLE2CYCLEDIFFCSEN    (1 << 6)
#define BUSTURNAROUND(x)       (((x) &  0xf) <<  0)

/* Values for GPMC_CONFIG7 - CS address mapping configuration */
#define MASKADDRESS(x)         (((x) &  0xf) <<  8)
#define CSVALID                (1 << 6)
#define BASEADDRESS(x)         (((x) & 0x3f) <<  0)

#endif /* _OMAP3_REGS_H */
