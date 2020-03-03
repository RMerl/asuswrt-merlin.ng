#ifndef OLPC_DCON_H_
#define OLPC_DCON_H_

#include <linux/notifier.h>
#include <linux/workqueue.h>

/* DCON registers */

#define DCON_REG_ID		 0
#define DCON_REG_MODE		 1

#define MODE_PASSTHRU	(1<<0)
#define MODE_SLEEP	(1<<1)
#define MODE_SLEEP_AUTO	(1<<2)
#define MODE_BL_ENABLE	(1<<3)
#define MODE_BLANK	(1<<4)
#define MODE_CSWIZZLE	(1<<5)
#define MODE_COL_AA	(1<<6)
#define MODE_MONO_LUMA	(1<<7)
#define MODE_SCAN_INT	(1<<8)
#define MODE_CLOCKDIV	(1<<9)
#define MODE_DEBUG	(1<<14)
#define MODE_SELFTEST	(1<<15)

#define DCON_REG_HRES		0x2
#define DCON_REG_HTOTAL		0x3
#define DCON_REG_HSYNC_WIDTH	0x4
#define DCON_REG_VRES		0x5
#define DCON_REG_VTOTAL		0x6
#define DCON_REG_VSYNC_WIDTH	0x7
#define DCON_REG_TIMEOUT	0x8
#define DCON_REG_SCAN_INT	0x9
#define DCON_REG_BRIGHT		0xa
#define DCON_REG_MEM_OPT_A	0x41
#define DCON_REG_MEM_OPT_B	0x42

/* Load Delay Locked Loop (DLL) settings for clock delay */
#define MEM_DLL_CLOCK_DELAY	(1<<0)
/* Memory controller power down function */
#define MEM_POWER_DOWN		(1<<8)
/* Memory controller software reset */
#define MEM_SOFT_RESET		(1<<0)

/* Status values */

#define DCONSTAT_SCANINT	0
#define DCONSTAT_SCANINT_DCON	1
#define DCONSTAT_DISPLAYLOAD	2
#define DCONSTAT_MISSED		3

/* Source values */

#define DCON_SOURCE_DCON        0
#define DCON_SOURCE_CPU         1

/* Interrupt */
#define DCON_IRQ                6

struct dcon_priv {
	struct i2c_client *client;
	struct fb_info *fbinfo;
	struct backlight_device *bl_dev;

	wait_queue_head_t waitq;
	struct work_struct switch_source;
	struct notifier_block reboot_nb;

	/* Shadow register for the DCON_REG_MODE register */
	u8 disp_mode;

	/* The current backlight value - this saves us some smbus traffic */
	u8 bl_val;

	/* Current source, initialized at probe time */
	int curr_src;

	/* Desired source */
	int pending_src;

	/* Variables used during switches */
	bool switched;
	struct timespec irq_time;
	struct timespec load_time;

	/* Current output type; true == mono, false == color */
	bool mono;
	bool asleep;
	/* This get set while controlling fb blank state from the driver */
	bool ignore_fb_events;
};

struct dcon_platform_data {
	int (*init)(struct dcon_priv *);
	void (*bus_stabilize_wiggle)(void);
	void (*set_dconload)(int);
	int (*read_status)(u8 *);
};

#include <linux/interrupt.h>

extern irqreturn_t dcon_interrupt(int irq, void *id);

#ifdef CONFIG_FB_OLPC_DCON_1
extern struct dcon_platform_data dcon_pdata_xo_1;
#endif

#ifdef CONFIG_FB_OLPC_DCON_1_5
extern struct dcon_platform_data dcon_pdata_xo_1_5;
#endif

#endif
