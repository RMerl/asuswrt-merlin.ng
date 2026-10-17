// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <input.h>
#include <keyboard.h>
#include <key_matrix.h>
#include <log.h>
#include <stdio_dev.h>
#include <tegra-kbc.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch-tegra/timer.h>
#include <linux/delay.h>
#include <linux/input.h>

enum {
	KBC_MAX_GPIO		= 24,
	KBC_MAX_KPENT		= 8,	/* size of keypress entry queue */
};

#define KBC_FIFO_TH_CNT_SHIFT		14
#define KBC_DEBOUNCE_CNT_SHIFT		4
#define KBC_CONTROL_FIFO_CNT_INT_EN	(1 << 3)
#define KBC_CONTROL_KBC_EN		(1 << 0)
#define KBC_INT_FIFO_CNT_INT_STATUS	(1 << 2)
#define KBC_KPENT_VALID			(1 << 7)
#define KBC_ST_STATUS			(1 << 3)

enum {
	KBC_DEBOUNCE_COUNT	= 2,
	KBC_REPEAT_RATE_MS	= 30,
	KBC_REPEAT_DELAY_MS	= 240,
	KBC_CLOCK_KHZ		= 32,	/* Keyboard uses a 32KHz clock */
};

/* keyboard controller config and state */
struct tegra_kbd_priv {
	struct input_config *input;	/* The input layer */
	struct key_matrix matrix;	/* The key matrix layer */

	struct kbc_tegra *kbc;		/* tegra keyboard controller */
	unsigned char inited;		/* 1 if keyboard has been inited */
	unsigned char first_scan;	/* 1 if this is our first key scan */

	/*
	 * After init we must wait a short time before polling the keyboard.
	 * This gives the tegra keyboard controller time to react after reset
	 * and lets us grab keys pressed during reset.
	 */
	unsigned int init_dly_ms;	/* Delay before we can read keyboard */
	unsigned int start_time_ms;	/* Time that we inited (in ms) */
	unsigned int last_poll_ms;	/* Time we should last polled */
	unsigned int next_repeat_ms;	/* Next time we repeat a key */
};

/**
 * reads the keyboard fifo for current keypresses
 *
 * @param priv		Keyboard private data
 * @param fifo		Place to put fifo results
 * @param max_keycodes	Maximum number of key codes to put in the fifo
 * Return: number of items put into fifo
 */
static int tegra_kbc_find_keys(struct tegra_kbd_priv *priv, int *fifo,
			       int max_keycodes)
{
	struct key_matrix_key keys[KBC_MAX_KPENT], *key;
	u32 kp_ent = 0;
	int i;

	for (key = keys, i = 0; i < KBC_MAX_KPENT; i++, key++) {
		/* Get next word */
		if (!(i & 3))
			kp_ent = readl(&priv->kbc->kp_ent[i / 4]);

		key->valid = (kp_ent & KBC_KPENT_VALID) != 0;
		key->row = (kp_ent >> 3) & 0xf;
		key->col = kp_ent & 0x7;

		/* Shift to get next entry */
		kp_ent >>= 8;
	}
	return key_matrix_decode(&priv->matrix, keys, KBC_MAX_KPENT, fifo,
				 max_keycodes);
}

/**
 * Process all the keypress sequences in fifo and send key codes
 *
 * The fifo contains zero or more keypress sets. Each set
 * consists of from 1-8 keycodes, representing the keycodes which
 * were simultaneously pressed during that scan.
 *
 * This function works through each set and generates ASCII characters
 * for each. Not that one set may produce more than one ASCII characters -
 * for example holding down 'd' and 'f' at the same time will generate
 * two ASCII characters.
 *
 * Note: if fifo_cnt is 0, we will tell the input layer that no keys are
 * pressed.
 *
 * @param priv		Keyboard private data
 * @param fifo_cnt	Number of entries in the keyboard fifo
 */
static void process_fifo(struct tegra_kbd_priv *priv, int fifo_cnt)
{
	int fifo[KBC_MAX_KPENT];
	int cnt = 0;

	/* Always call input_send_keycodes() at least once */
	do {
		if (fifo_cnt)
			cnt = tegra_kbc_find_keys(priv, fifo, KBC_MAX_KPENT);

		input_send_keycodes(priv->input, fifo, cnt);
	} while (--fifo_cnt > 0);
}

/**
 * Check the keyboard controller and emit ASCII characters for any keys that
 * are pressed.
 *
 * @param priv		Keyboard private data
 */
static void check_for_keys(struct tegra_kbd_priv *priv)
{
	int fifo_cnt;

	if (!priv->first_scan &&
	    get_timer(priv->last_poll_ms) < KBC_REPEAT_RATE_MS)
		return;
	priv->last_poll_ms = get_timer(0);
	priv->first_scan = 0;

	/*
	 * Once we get here we know the keyboard has been scanned. So if there
	 * scan waiting for us, we know that nothing is held down.
	 */
	fifo_cnt = (readl(&priv->kbc->interrupt) >> 4) & 0xf;
	process_fifo(priv, fifo_cnt);
}

/**
 * In order to detect keys pressed on boot, wait for the hardware to
 * complete scanning the keys. This includes time to transition from
 * Wkup mode to Continous polling mode and the repoll time. We can
 * deduct the time that's already elapsed.
 *
 * @param priv		Keyboard private data
 */
static void kbd_wait_for_fifo_init(struct tegra_kbd_priv *priv)
{
	if (!priv->inited) {
		unsigned long elapsed_time;
		long delay_ms;

		elapsed_time = get_timer(priv->start_time_ms);
		delay_ms = priv->init_dly_ms - elapsed_time;
		if (delay_ms > 0) {
			udelay(delay_ms * 1000);
			debug("%s: delay %ldms\n", __func__, delay_ms);
		}

		priv->inited = 1;
	}
}

/**
 * Check the tegra keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 * Return: 1, to indicate that we have something to look at
 */
static int tegra_kbc_check(struct input_config *input)
{
	struct tegra_kbd_priv *priv = dev_get_priv(input->dev);

	kbd_wait_for_fifo_init(priv);
	check_for_keys(priv);

	return 1;
}

/* configures keyboard GPIO registers to use the rows and columns */
static void config_kbc_gpio(struct tegra_kbd_priv *priv, struct kbc_tegra *kbc)
{
	int i;

	for (i = 0; i < KBC_MAX_GPIO; i++) {
		u32 row_cfg, col_cfg;
		u32 r_shift = 5 * (i % 6);
		u32 c_shift = 4 * (i % 8);
		u32 r_mask = 0x1f << r_shift;
		u32 c_mask = 0xf << c_shift;
		u32 r_offs = i / 6;
		u32 c_offs = i / 8;

		row_cfg = readl(&kbc->row_cfg[r_offs]);
		col_cfg = readl(&kbc->col_cfg[c_offs]);

		row_cfg &= ~r_mask;
		col_cfg &= ~c_mask;

		if (i < priv->matrix.num_rows) {
			row_cfg |= ((i << 1) | 1) << r_shift;
		} else {
			col_cfg |= (((i - priv->matrix.num_rows) << 1) | 1)
					<< c_shift;
		}

		writel(row_cfg, &kbc->row_cfg[r_offs]);
		writel(col_cfg, &kbc->col_cfg[c_offs]);
	}
}

/**
 * Start up the keyboard device
 */
static void tegra_kbc_open(struct tegra_kbd_priv *priv)
{
	struct kbc_tegra *kbc = priv->kbc;
	unsigned int scan_period;
	u32 val;

	/*
	 * We will scan at twice the keyboard repeat rate, so that there is
	 * always a scan ready when we check it in check_for_keys().
	 */
	scan_period = KBC_REPEAT_RATE_MS / 2;
	writel(scan_period * KBC_CLOCK_KHZ, &kbc->rpt_dly);
	writel(scan_period * KBC_CLOCK_KHZ, &kbc->init_dly);
	/*
	 * Before reading from the keyboard we must wait for the init_dly
	 * plus the rpt_delay, plus 2ms for the row scan time.
	 */
	priv->init_dly_ms = scan_period * 2 + 2;

	val = KBC_DEBOUNCE_COUNT << KBC_DEBOUNCE_CNT_SHIFT;
	val |= 1 << KBC_FIFO_TH_CNT_SHIFT;	/* fifo interrupt threshold */
	val |= KBC_CONTROL_KBC_EN;		/* enable */
	writel(val, &kbc->control);

	priv->start_time_ms = get_timer(0);
	priv->last_poll_ms = get_timer(0);
	priv->next_repeat_ms = priv->last_poll_ms;
	priv->first_scan = 1;
}

static int tegra_kbd_start(struct udevice *dev)
{
	struct tegra_kbd_priv *priv = dev_get_priv(dev);

	/* Set up pin mux and enable the clock */
	funcmux_select(PERIPH_ID_KBC, FUNCMUX_DEFAULT);
	clock_enable(PERIPH_ID_KBC);
	config_kbc_gpio(priv, priv->kbc);

	tegra_kbc_open(priv);
	debug("%s: Tegra keyboard ready\n", __func__);

	return 0;
}

/**
 * Set up the tegra keyboard. This is called by the stdio device handler
 *
 * We want to do this init when the keyboard is actually used rather than
 * at start-up, since keyboard input may not currently be selected.
 *
 * Once the keyboard starts there will be a period during which we must
 * wait for the keyboard to init. We do this only when a key is first
 * read - see kbd_wait_for_fifo_init().
 *
 * Return: 0 if ok, -ve on error
 */
static int tegra_kbd_probe(struct udevice *dev)
{
	struct tegra_kbd_priv *priv = dev_get_priv(dev);
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret;

	priv->kbc = dev_read_addr_ptr(dev);
	if ((fdt_addr_t)priv->kbc == FDT_ADDR_T_NONE) {
		debug("%s: No keyboard register found\n", __func__);
		return -EINVAL;
	}
	input_set_delays(input, KBC_REPEAT_DELAY_MS, KBC_REPEAT_RATE_MS);

	/* Decode the keyboard matrix information (16 rows, 8 columns) */
	ret = key_matrix_init(&priv->matrix, 16, 8, 1);
	if (ret) {
		debug("%s: Could not init key matrix: %d\n", __func__, ret);
		return ret;
	}
	ret = key_matrix_decode_fdt(dev, &priv->matrix);
	if (ret) {
		debug("%s: Could not decode key matrix from fdt: %d\n",
		      __func__, ret);
		return ret;
	}
	input_add_tables(input, false);
	if (priv->matrix.fn_keycode) {
		ret = input_add_table(input, KEY_FN, -1,
				      priv->matrix.fn_keycode,
				      priv->matrix.key_count);
		if (ret) {
			debug("%s: input_add_table() failed\n", __func__);
			return ret;
		}
	}

	/* Register the device. init_tegra_keyboard() will be called soon */
	priv->input = input;
	input->dev = dev;
	input->read_keys = tegra_kbc_check;
	strcpy(sdev->name, "tegra-kbc");
	ret = input_stdio_register(sdev);
	if (ret) {
		debug("%s: input_stdio_register() failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct keyboard_ops tegra_kbd_ops = {
	.start	= tegra_kbd_start,
};

static const struct udevice_id tegra_kbd_ids[] = {
	{ .compatible = "nvidia,tegra20-kbc" },
	{ }
};

U_BOOT_DRIVER(tegra_kbd) = {
	.name	= "tegra_kbd",
	.id	= UCLASS_KEYBOARD,
	.of_match = tegra_kbd_ids,
	.probe = tegra_kbd_probe,
	.ops	= &tegra_kbd_ops,
	.priv_auto	= sizeof(struct tegra_kbd_priv),
};
