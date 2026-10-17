#ifndef _AT91_I2C_H
#define _AT91_I2C_H

#include <linux/bitops.h>
#define	TWI_CR_START		BIT(0)	/* Send a Start Condition */
#define	TWI_CR_MSEN		BIT(2)	/* Master Transfer Enable */
#define	TWI_CR_STOP		BIT(1)	/* Send a Stop Condition */
#define	TWI_CR_SVDIS		BIT(5)	/* Slave Transfer Disable */
#define	TWI_CR_SWRST		BIT(7)	/* Software Reset */
#define	TWI_CR_ACMEN		BIT(16) /* Alternative Command Mode Enable */
#define	TWI_CR_ACMDIS		BIT(17) /* Alternative Command Mode Disable */
#define	TWI_CR_LOCKCLR		BIT(26) /* Lock Clear */

#define	TWI_MMR_MREAD		BIT(12) /* Master Read Direction */
#define	TWI_MMR_IADRSZ_1	BIT(8)	/* Internal Device Address Size */

#define	TWI_SR_TXCOMP		BIT(0)	/* Transmission Complete */
#define	TWI_SR_RXRDY		BIT(1)	/* Receive Holding Register Ready */
#define	TWI_SR_TXRDY		BIT(2)	/* Transmit Holding Register Ready */
#define	TWI_SR_OVRE		BIT(6)	/* Overrun Error */
#define	TWI_SR_UNRE		BIT(7)	/* Underrun Error */
#define	TWI_SR_NACK		BIT(8)	/* Not Acknowledged */
#define	TWI_SR_LOCK		BIT(23) /* TWI Lock due to Frame Errors */

#define	TWI_ACR_DATAL(len)	((len) & 0xff)
#define	TWI_ACR_DIR_READ	BIT(8)

#define	TWI_CWGR_HOLD_MAX	0x1f
#define	TWI_CWGR_HOLD(x)	(((x) & TWI_CWGR_HOLD_MAX) << 24)

struct at91_i2c_regs {
	u32 cr;
	u32 mmr;
	u32 smr;
	u32 iadr;
	u32 cwgr;
	u32 rev_0[3];
	u32 sr;
	u32 ier;
	u32 idr;
	u32 imr;
	u32 rhr;
	u32 thr;
	u32 smbtr;
	u32 rev_1;
	u32 acr;
	u32 filtr;
	u32 rev_2;
	u32 swmr;
	u32 fmr;
	u32 flr;
	u32 rev_3;
	u32 fsr;
	u32 fier;
	u32 fidr;
	u32 fimr;
	u32 rev_4[29];
	u32 wpmr;
	u32 wpsr;
	u32 rev_5[6];
};

struct at91_i2c_pdata {
	unsigned clk_max_div;
	unsigned clk_offset;
};

struct at91_i2c_bus {
	struct at91_i2c_regs *regs;
	u32 status;
	ulong bus_clk_rate;
	u32 clock_frequency;
	u32 speed;
	u32 cwgr_val;
	const struct at91_i2c_pdata *pdata;
};

#endif
