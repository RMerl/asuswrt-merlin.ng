#ifndef DT_BINDINGS_STM32_SDRAM_H
#define DT_BINDINGS_STM32_SDRAM_H

#define NO_COL_8	0x0
#define NO_COL_9	0x1
#define NO_COL_10	0x2
#define NO_COL_11	0x3

#define NO_ROW_11	0x0
#define NO_ROW_12	0x1
#define NO_ROW_13	0x2

#define MWIDTH_8	0x0
#define MWIDTH_16	0x1
#define MWIDTH_32	0x2
#define BANKS_2		0x0
#define BANKS_4		0x1
#define CAS_1		0x1
#define CAS_2		0x2
#define CAS_3		0x3
#define SDCLK_DIS	0x0
#define SDCLK_2		0x2
#define SDCLK_3		0x3
#define RD_BURST_EN	0x1
#define RD_BURST_DIS	0x0
#define RD_PIPE_DL_0	0x0
#define RD_PIPE_DL_1	0x1
#define RD_PIPE_DL_2	0x2

/* Timing = value +1 cycles */
#define TMRD_1		(1 - 1)
#define TMRD_2		(2 - 1)
#define TMRD_3		(3 - 1)
#define TXSR_1		(1 - 1)
#define TXSR_6		(6 - 1)
#define TXSR_7		(7 - 1)
#define TRAS_1		(1 - 1)
#define TRAS_4		(4 - 1)
#define TRC_6		(6 - 1)
#define TWR_1		(1 - 1)
#define TWR_2		(2 - 1)
#define TRP_2		(2 - 1)
#define TRCD_1		(1 - 1)
#define TRCD_2		(2 - 1)

#endif
