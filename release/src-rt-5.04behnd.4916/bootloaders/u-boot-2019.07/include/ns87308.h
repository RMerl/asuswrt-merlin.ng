/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 */

#ifndef _NS87308_H_
#define _NS87308_H_

#include <asm/pci_io.h>

/* Note: I couldn't find a full data sheet for the ns87308, but the ns87307 seems to be pretty
   functionally- (and pin-) equivalent to the 87308, but the 308 has better ir support. */

void initialise_ns87308(void);

/*
 * The following struct represents the GPIO registers on the NS87308/NS97307
 */
struct GPIO
{
  unsigned char dta1;  /* 0 data port 1 */
  unsigned char dir1;  /* 1 direction port 1 */
  unsigned char out1;  /* 2 output type port 1 */
  unsigned char puc1;  /* 3 pull-up control port 1 */
  unsigned char dta2;  /* 4 data port 2 */
  unsigned char dir2;  /* 5 direction port 2 */
  unsigned char out2;  /* 6 output type port 2 */
  unsigned char puc2;  /* 7 pull-up control port 2  */
};

/*
 * The following represents the power management registers on the NS87308/NS97307
 */
#define PWM_FER1 0  /* 0 function enable reg. 1 */
#define PWM_FER2 1  /* 1 function enable reg. 2 */
#define PWM_PMC1 2  /* 2 power mgmt. control 1 */
#define PWM_PMC2 3  /* 3 power mgmt. control 2 */
#define PWM_PMC3 4  /* 4 power mgmt. control 3 */
#define PWM_WDTO 5  /* 5 watchdog time-out */
#define PWM_WDCF 6  /* 6 watchdog config. */
#define PWM_WDST 7  /* 7 watchdog status  */

/*PNP config registers:
 * these depend on the stated of BADDR1 and BADDR0 on startup
 * so there's three versions here with the last two digits indicating
 * for which configuration their valid
 * the 1st of the two digits indicates the state of BADDR1
 * the 2st of the two digits indicates the state of BADDR0
 */


#define IO_INDEX_OFFSET_0x 0x0279  /* full PnP isa Mode */
#define IO_INDEX_OFFSET_10 0x015C  /* PnP motherboard mode */
#define IO_INDEX_OFFSET_11 0x002E  /* PnP motherboard mode */
#define IO_DATA_OFFSET_0x  0x0A79  /* full PnP isa Mode */
#define IO_DATA_OFFSET_10  0x015D  /* PnP motherboard mode */
#define IO_DATA_OFFSET_11  0x002F  /* PnP motherboard mode */

#if defined(CONFIG_SYS_NS87308_BADDR_0x)
#define IO_INDEX (CONFIG_SYS_ISA_IO + IO_INDEX_OFFSET_0x)
#define IO_DATA  (CONFIG_SYS_ISA_IO + IO_DATA_OFFSET_0x)
#elif defined(CONFIG_SYS_NS87308_BADDR_10)
#define IO_INDEX (CONFIG_SYS_ISA_IO + IO_INDEX_OFFSET_10)
#define IO_DATA  (CONFIG_SYS_ISA_IO + IO_DATA_OFFSET_10)
#elif defined(CONFIG_SYS_NS87308_BADDR_11)
#define IO_INDEX (CONFIG_SYS_ISA_IO + IO_INDEX_OFFSET_11)
#define IO_DATA  (CONFIG_SYS_ISA_IO + IO_DATA_OFFSET_11)
#endif

/* PnP register definitions */

#define SET_RD_DATA_PORT    0x00
#define SERIAL_ISOLATION    0x01
#define CONFIG_CONTROL      0x02
#define WAKE_CSN            0x03
#define RES_DATA            0x04
#define STATUS              0x05
#define SET_CSN             0x06
#define LOGICAL_DEVICE      0x07

/*vendor defined values */
#define SID_REG             0x20
#define SUPOERIO_CONF1      0x21
#define SUPOERIO_CONF2      0x22
#define PGCS_INDEX          0x23
#define PGCS_DATA           0x24

/* values above 30 are different for each logical device
   but I can't be arsed to enter them all. the ones here
   are pretty consistent between all logical devices
   feel free to correct the situation if you want.. ;)
   */
#define ACTIVATE            0x30
#define ACTIVATE_OFF        0x00
#define ACTIVATE_ON         0x01

#define BASE_ADDR_HIGH      0x60
#define BASE_ADDR_LOW       0x61
#define LUN_CONFIG_REG		0xF0
#define DBASE_HIGH			0x60	/* SIO KBC data base address, 15:8 */
#define DBASE_LOW			0x61	/* SIO KBC data base address,  7:0 */
#define CBASE_HIGH			0x62	/* SIO KBC command base addr, 15:8 */
#define CBASE_LOW			0x63	/* SIO KBC command base addr,  7:0 */

/* the logical devices*/
#define LDEV_KBC1           0x00	/* 2 devices for keyboard and mouse controller*/
#define LDEV_KBC2           0x01
#define LDEV_MOUSE          0x01
#define LDEV_RTC_APC        0x02	/*Real Time Clock and Advanced Power Control*/
#define LDEV_FDC            0x03	/*floppy disk controller*/
#define LDEV_PARP           0x04	/*Parallel port*/
#define LDEV_UART2          0x05
#define LDEV_UART1          0x06
#define LDEV_GPIO           0x07    /*General Purpose IO and chip select output signals*/
#define LDEV_POWRMAN        0x08    /*Power Managment*/

#define CONFIG_SYS_NS87308_KBC1	(1 << LDEV_KBC1)
#define CONFIG_SYS_NS87308_KBC2	(1 << LDEV_KBC2)
#define CONFIG_SYS_NS87308_MOUSE	(1 << LDEV_MOUSE)
#define CONFIG_SYS_NS87308_RTC_APC	(1 << LDEV_RTC_APC)
#define CONFIG_SYS_NS87308_FDC		(1 << LDEV_FDC)
#define CONFIG_SYS_NS87308_PARP	(1 << LDEV_PARP)
#define CONFIG_SYS_NS87308_UART2	(1 << LDEV_UART2)
#define CONFIG_SYS_NS87308_UART1	(1 << LDEV_UART1)
#define CONFIG_SYS_NS87308_GPIO	(1 << LDEV_GPIO)
#define CONFIG_SYS_NS87308_POWRMAN	(1 << LDEV_POWRMAN)

/*some functions and macro's for doing configuration */

static inline void read_pnp_config(unsigned char index, unsigned char *data)
{
    pci_writeb(index,IO_INDEX);
    pci_readb(IO_DATA, *data);
}

static inline void write_pnp_config(unsigned char index, unsigned char data)
{
    pci_writeb(index,IO_INDEX);
    pci_writeb(data, IO_DATA);
}

static inline void pnp_set_device(unsigned char dev)
{
    write_pnp_config(LOGICAL_DEVICE, dev);
}

static inline void write_pm_reg(unsigned short base, unsigned char index, unsigned char data)
{
    pci_writeb(index, CONFIG_SYS_ISA_IO + base);
    eieio();
    pci_writeb(data, CONFIG_SYS_ISA_IO + base + 1);
}

/*void write_pnp_config(unsigned char index, unsigned char data);
void pnp_set_device(unsigned char dev);
*/

#define PNP_SET_DEVICE_BASE(dev,base) \
   pnp_set_device(dev); \
   write_pnp_config(ACTIVATE, ACTIVATE_OFF); \
   write_pnp_config(BASE_ADDR_HIGH, ((base) >> 8) & 0xff ); \
   write_pnp_config(BASE_ADDR_LOW, (base) &0xff); \
   write_pnp_config(ACTIVATE, ACTIVATE_ON);

#define PNP_ACTIVATE_DEVICE(dev) \
   pnp_set_device(dev); \
   write_pnp_config(ACTIVATE, ACTIVATE_ON);

#define PNP_DEACTIVATE_DEVICE(dev) \
   pnp_set_device(dev); \
   write_pnp_config(ACTIVATE, ACTIVATE_OFF);


static inline void write_pgcs_config(unsigned char index, unsigned char data)
{
    write_pnp_config(PGCS_INDEX, index);
    write_pnp_config(PGCS_DATA, data);
}

/* these macrose configure the 3 CS lines
   on the sandpoint board these controll NVRAM
   CS0 is connected to NVRAMCS
   CS1 is connected to NVRAMAS0
   CS2 is connected to NVRAMAS1
   */
#define PGCS_CS_ASSERT_ON_WRITE 0x10
#define PGCS_CS_ASSERT_ON_READ  0x20

#define PNP_PGCS_CSLINE_BASE(cs, base) \
  write_pgcs_config((cs) << 2, ((base) >> 8) & 0xff ); \
  write_pgcs_config(((cs) << 2) + 1, (base) & 0xff );

#define PNP_PGCS_CSLINE_CONF(cs, conf) \
  write_pgcs_config(((cs) << 2) + 2, (conf) );


/* The following sections are for 87308 extensions to the standard compoents it emulates */

/* extensions to 16550*/

#define MCR_MDSL_MSK    0xe0 /*mode select mask*/
#define MCR_MDSL_UART   0x00 /*uart, default*/
#define MCR_MDSL_SHRPIR 0x02 /*Sharp IR*/
#define MCR_MDSL_SIR    0x03 /*SIR*/
#define MCR_MDSL_CIR    0x06 /*Consumer IR*/

#define FCR_TXFTH0      0x10    /* these bits control threshod of data level in fifo */
#define FCR_TXFTH1      0x20    /* for interrupt trigger */

/*
 * Default NS87308 configuration
 */
#ifndef CONFIG_SYS_NS87308_KBC1_BASE
#define CONFIG_SYS_NS87308_KBC1_BASE	0x0060
#endif
#ifndef CONFIG_SYS_NS87308_RTC_BASE
#define CONFIG_SYS_NS87308_RTC_BASE	0x0070
#endif
#ifndef CONFIG_SYS_NS87308_FDC_BASE
#define CONFIG_SYS_NS87308_FDC_BASE	0x03F0
#endif
#ifndef CONFIG_SYS_NS87308_LPT_BASE
#define CONFIG_SYS_NS87308_LPT_BASE	0x0278
#endif
#ifndef CONFIG_SYS_NS87308_UART1_BASE
#define CONFIG_SYS_NS87308_UART1_BASE	0x03F8
#endif
#ifndef CONFIG_SYS_NS87308_UART2_BASE
#define CONFIG_SYS_NS87308_UART2_BASE	0x02F8
#endif

#endif /*_NS87308_H_*/
