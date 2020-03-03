#ifndef _VME_PIO2_H_
#define _VME_PIO2_H_

#define PIO2_CARDS_MAX			32

#define PIO2_VARIANT_LENGTH		5

#define PIO2_NUM_CHANNELS		32
#define PIO2_NUM_IRQS			11
#define PIO2_NUM_CNTRS			6

#define PIO2_REGS_SIZE			0x40

#define PIO2_REGS_DATA0			0x0
#define PIO2_REGS_DATA1			0x1
#define PIO2_REGS_DATA2			0x2
#define PIO2_REGS_DATA3			0x3

static const int PIO2_REGS_DATA[4] = { PIO2_REGS_DATA0, PIO2_REGS_DATA1,
					PIO2_REGS_DATA2, PIO2_REGS_DATA3 };

#define PIO2_REGS_INT_STAT0		0x8
#define PIO2_REGS_INT_STAT1		0x9
#define PIO2_REGS_INT_STAT2		0xa
#define PIO2_REGS_INT_STAT3		0xb

static const int PIO2_REGS_INT_STAT[4] = { PIO2_REGS_INT_STAT0,
					PIO2_REGS_INT_STAT1,
					PIO2_REGS_INT_STAT2,
					PIO2_REGS_INT_STAT3 };

#define PIO2_REGS_INT_STAT_CNTR		0xc
#define PIO2_REGS_INT_MASK0		0x10
#define PIO2_REGS_INT_MASK1		0x11
#define PIO2_REGS_INT_MASK2		0x12
#define PIO2_REGS_INT_MASK3		0x13
#define PIO2_REGS_INT_MASK4		0x14
#define PIO2_REGS_INT_MASK5		0x15
#define PIO2_REGS_INT_MASK6		0x16
#define PIO2_REGS_INT_MASK7		0x17

static const int PIO2_REGS_INT_MASK[8] = { PIO2_REGS_INT_MASK0,
					PIO2_REGS_INT_MASK1,
					PIO2_REGS_INT_MASK2,
					PIO2_REGS_INT_MASK3,
					PIO2_REGS_INT_MASK4,
					PIO2_REGS_INT_MASK5,
					PIO2_REGS_INT_MASK6,
					PIO2_REGS_INT_MASK7 };



#define PIO2_REGS_CTRL			0x18
#define PIO2_REGS_VME_VECTOR		0x19
#define PIO2_REGS_CNTR0			0x20
#define PIO2_REGS_CNTR1			0x22
#define PIO2_REGS_CNTR2			0x24
#define PIO2_REGS_CTRL_WRD0		0x26
#define PIO2_REGS_CNTR3			0x28
#define PIO2_REGS_CNTR4			0x2a
#define PIO2_REGS_CNTR5			0x2c
#define PIO2_REGS_CTRL_WRD1		0x2e

#define PIO2_REGS_ID			0x30


/* PIO2_REGS_DATAx (0x0 - 0x3) */

static const int PIO2_CHANNEL_BANK[32] = { 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2,
					3, 3, 3, 3, 3, 3, 3, 3 };

#define PIO2_CHANNEL0_BIT		(1 << 0)
#define PIO2_CHANNEL1_BIT		(1 << 1)
#define PIO2_CHANNEL2_BIT		(1 << 2)
#define PIO2_CHANNEL3_BIT		(1 << 3)
#define PIO2_CHANNEL4_BIT		(1 << 4)
#define PIO2_CHANNEL5_BIT		(1 << 5)
#define PIO2_CHANNEL6_BIT		(1 << 6)
#define PIO2_CHANNEL7_BIT		(1 << 7)
#define PIO2_CHANNEL8_BIT		(1 << 0)
#define PIO2_CHANNEL9_BIT		(1 << 1)
#define PIO2_CHANNEL10_BIT		(1 << 2)
#define PIO2_CHANNEL11_BIT		(1 << 3)
#define PIO2_CHANNEL12_BIT		(1 << 4)
#define PIO2_CHANNEL13_BIT		(1 << 5)
#define PIO2_CHANNEL14_BIT		(1 << 6)
#define PIO2_CHANNEL15_BIT		(1 << 7)
#define PIO2_CHANNEL16_BIT		(1 << 0)
#define PIO2_CHANNEL17_BIT		(1 << 1)
#define PIO2_CHANNEL18_BIT		(1 << 2)
#define PIO2_CHANNEL19_BIT		(1 << 3)
#define PIO2_CHANNEL20_BIT		(1 << 4)
#define PIO2_CHANNEL21_BIT		(1 << 5)
#define PIO2_CHANNEL22_BIT		(1 << 6)
#define PIO2_CHANNEL23_BIT		(1 << 7)
#define PIO2_CHANNEL24_BIT		(1 << 0)
#define PIO2_CHANNEL25_BIT		(1 << 1)
#define PIO2_CHANNEL26_BIT		(1 << 2)
#define PIO2_CHANNEL27_BIT		(1 << 3)
#define PIO2_CHANNEL28_BIT		(1 << 4)
#define PIO2_CHANNEL29_BIT		(1 << 5)
#define PIO2_CHANNEL30_BIT		(1 << 6)
#define PIO2_CHANNEL31_BIT		(1 << 7)

static const int PIO2_CHANNEL_BIT[32] = { PIO2_CHANNEL0_BIT, PIO2_CHANNEL1_BIT,
					PIO2_CHANNEL2_BIT, PIO2_CHANNEL3_BIT,
					PIO2_CHANNEL4_BIT, PIO2_CHANNEL5_BIT,
					PIO2_CHANNEL6_BIT, PIO2_CHANNEL7_BIT,
					PIO2_CHANNEL8_BIT, PIO2_CHANNEL9_BIT,
					PIO2_CHANNEL10_BIT, PIO2_CHANNEL11_BIT,
					PIO2_CHANNEL12_BIT, PIO2_CHANNEL13_BIT,
					PIO2_CHANNEL14_BIT, PIO2_CHANNEL15_BIT,
					PIO2_CHANNEL16_BIT, PIO2_CHANNEL17_BIT,
					PIO2_CHANNEL18_BIT, PIO2_CHANNEL19_BIT,
					PIO2_CHANNEL20_BIT, PIO2_CHANNEL21_BIT,
					PIO2_CHANNEL22_BIT, PIO2_CHANNEL23_BIT,
					PIO2_CHANNEL24_BIT, PIO2_CHANNEL25_BIT,
					PIO2_CHANNEL26_BIT, PIO2_CHANNEL27_BIT,
					PIO2_CHANNEL28_BIT, PIO2_CHANNEL29_BIT,
					PIO2_CHANNEL30_BIT, PIO2_CHANNEL31_BIT
					};

/* PIO2_REGS_INT_STAT_CNTR (0xc) */
#define PIO2_COUNTER0			(1 << 0)
#define PIO2_COUNTER1			(1 << 1)
#define PIO2_COUNTER2			(1 << 2)
#define PIO2_COUNTER3			(1 << 3)
#define PIO2_COUNTER4			(1 << 4)
#define PIO2_COUNTER5			(1 << 5)

static const int PIO2_COUNTER[6] = { PIO2_COUNTER0, PIO2_COUNTER1,
					PIO2_COUNTER2, PIO2_COUNTER3,
					PIO2_COUNTER4, PIO2_COUNTER5 };

/* PIO2_REGS_CTRL (0x18) */
#define PIO2_VME_INT_MASK		0x7
#define PIO2_LED			(1 << 6)
#define PIO2_LOOP			(1 << 7)

/* PIO2_REGS_VME_VECTOR (0x19) */
#define PIO2_VME_VECTOR_SPUR		0x0
#define PIO2_VME_VECTOR_BANK0		0x1
#define PIO2_VME_VECTOR_BANK1		0x2
#define PIO2_VME_VECTOR_BANK2		0x3
#define PIO2_VME_VECTOR_BANK3		0x4
#define PIO2_VME_VECTOR_CNTR0		0x5
#define PIO2_VME_VECTOR_CNTR1		0x6
#define PIO2_VME_VECTOR_CNTR2		0x7
#define PIO2_VME_VECTOR_CNTR3		0x8
#define PIO2_VME_VECTOR_CNTR4		0x9
#define PIO2_VME_VECTOR_CNTR5		0xa

#define PIO2_VME_VECTOR_MASK		0xf0

static const int PIO2_VECTOR_BANK[4] = { PIO2_VME_VECTOR_BANK0,
					PIO2_VME_VECTOR_BANK1,
					PIO2_VME_VECTOR_BANK2,
					PIO2_VME_VECTOR_BANK3 };

static const int PIO2_VECTOR_CNTR[6] = { PIO2_VME_VECTOR_CNTR0,
					PIO2_VME_VECTOR_CNTR1,
					PIO2_VME_VECTOR_CNTR2,
					PIO2_VME_VECTOR_CNTR3,
					PIO2_VME_VECTOR_CNTR4,
					PIO2_VME_VECTOR_CNTR5 };

/* PIO2_REGS_CNTRx (0x20 - 0x24 & 0x28 - 0x2c) */

static const int PIO2_CNTR_DATA[6] = { PIO2_REGS_CNTR0, PIO2_REGS_CNTR1,
					PIO2_REGS_CNTR2, PIO2_REGS_CNTR3,
					PIO2_REGS_CNTR4, PIO2_REGS_CNTR5 };

/* PIO2_REGS_CTRL_WRDx (0x26 & 0x2e) */

static const int PIO2_CNTR_CTRL[6] = { PIO2_REGS_CTRL_WRD0,
					PIO2_REGS_CTRL_WRD0,
					PIO2_REGS_CTRL_WRD0,
					PIO2_REGS_CTRL_WRD1,
					PIO2_REGS_CTRL_WRD1,
					PIO2_REGS_CTRL_WRD1 };

#define PIO2_CNTR_SC_DEV0		0
#define PIO2_CNTR_SC_DEV1		(1 << 6)
#define PIO2_CNTR_SC_DEV2		(2 << 6)
#define PIO2_CNTR_SC_RDBACK		(3 << 6)

static const int PIO2_CNTR_SC_DEV[6] = { PIO2_CNTR_SC_DEV0, PIO2_CNTR_SC_DEV1,
					PIO2_CNTR_SC_DEV2, PIO2_CNTR_SC_DEV0,
					PIO2_CNTR_SC_DEV1, PIO2_CNTR_SC_DEV2 };

#define PIO2_CNTR_RW_LATCH		0
#define PIO2_CNTR_RW_LSB		(1 << 4)
#define PIO2_CNTR_RW_MSB		(2 << 4)
#define PIO2_CNTR_RW_BOTH		(3 << 4)

#define PIO2_CNTR_MODE0			0
#define PIO2_CNTR_MODE1			(1 << 1)
#define PIO2_CNTR_MODE2			(2 << 1)
#define PIO2_CNTR_MODE3			(3 << 1)
#define PIO2_CNTR_MODE4			(4 << 1)
#define PIO2_CNTR_MODE5			(5 << 1)

#define PIO2_CNTR_BCD			1



enum pio2_bank_config { NOFIT, INPUT, OUTPUT, BOTH };
enum pio2_int_config { NONE = 0, LOW2HIGH = 1, HIGH2LOW = 2, EITHER = 4 };

/* Bank configuration structure */
struct pio2_io_bank {
	enum pio2_bank_config config;
	u8 value;
	enum pio2_int_config irq[8];
};

/* Counter configuration structure */
struct pio2_cntr {
	int mode;
	int count;
};

struct pio2_card {
	int id;
	int bus;
	long base;
	int irq_vector;
	int irq_level;
	char variant[6];
	int led;

	struct vme_dev *vdev;
	struct vme_resource *window;

	struct gpio_chip gc;
	struct pio2_io_bank bank[4];

	struct pio2_cntr cntr[6];
};

int pio2_cntr_reset(struct pio2_card *);

int pio2_gpio_reset(struct pio2_card *);
int pio2_gpio_init(struct pio2_card *);
void pio2_gpio_exit(struct pio2_card *);

#endif /* _VME_PIO2_H_ */
