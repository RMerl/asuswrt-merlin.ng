#ifndef _LPC32XX_I2C_H
#define _LPC32XX_I2C_H

#include <asm/types.h>

/* i2c register set */
struct lpc32xx_i2c_base {
	union {
		u32 rx;
		u32 tx;
	};
	u32 stat;
	u32 ctrl;
	u32 clk_hi;
	u32 clk_lo;
	u32 adr;
	u32 rxfl;
	u32 txfl;
	u32 rxb;
	u32 txb;
	u32 stx;
	u32 stxfl;
};

#if CONFIG_IS_ENABLED(DM_I2C)
enum {
	I2C_0, I2C_1, I2C_2,
};

struct lpc32xx_i2c_dev {
	struct lpc32xx_i2c_base *base;
	int index;
	uint speed;
};
#endif /* CONFIG_DM_I2C */
#endif /* _LPC32XX_I2C_H */
