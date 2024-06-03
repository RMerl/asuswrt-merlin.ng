/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Sukumar Ghorai <s-ghorai@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <i2c.h>
#if defined(CONFIG_OMAP54XX) || defined(CONFIG_OMAP44XX)
#include <palmas.h>
#endif
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#ifdef CONFIG_OMAP54XX
#include <asm/arch/mux_dra7xx.h>
#include <asm/arch/dra7xx_iodelay.h>
#endif
#if !defined(CONFIG_SOC_KEYSTONE)
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#endif
#ifdef CONFIG_MMC_OMAP36XX_PINS
#include <asm/arch/mux.h>
#endif
#include <dm.h>
#include <power/regulator.h>
#include <thermal.h>

DECLARE_GLOBAL_DATA_PTR;

/* simplify defines to OMAP_HSMMC_USE_GPIO */
#if (defined(CONFIG_OMAP_GPIO) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_GPIO_SUPPORT))
#define OMAP_HSMMC_USE_GPIO
#else
#undef OMAP_HSMMC_USE_GPIO
#endif

/* common definitions for all OMAPs */
#define SYSCTL_SRC	(1 << 25)
#define SYSCTL_SRD	(1 << 26)

#ifdef CONFIG_IODELAY_RECALIBRATION
struct omap_hsmmc_pinctrl_state {
	struct pad_conf_entry *padconf;
	int npads;
	struct iodelay_cfg_entry *iodelay;
	int niodelays;
};
#endif

struct omap_hsmmc_data {
	struct hsmmc *base_addr;
#if !CONFIG_IS_ENABLED(DM_MMC)
	struct mmc_config cfg;
#endif
	uint bus_width;
	uint clock;
	ushort last_cmd;
#ifdef OMAP_HSMMC_USE_GPIO
#if CONFIG_IS_ENABLED(DM_MMC)
	struct gpio_desc cd_gpio;	/* Change Detect GPIO */
	struct gpio_desc wp_gpio;	/* Write Protect GPIO */
#else
	int cd_gpio;
	int wp_gpio;
#endif
#endif
#if CONFIG_IS_ENABLED(DM_MMC)
	enum bus_mode mode;
#endif
	u8 controller_flags;
#ifdef CONFIG_MMC_OMAP_HS_ADMA
	struct omap_hsmmc_adma_desc *adma_desc_table;
	uint desc_slot;
#endif
	const char *hw_rev;
	struct udevice *pbias_supply;
	uint signal_voltage;
#ifdef CONFIG_IODELAY_RECALIBRATION
	struct omap_hsmmc_pinctrl_state *default_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *hs_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *hs200_1_8v_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *ddr_1_8v_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *sdr12_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *sdr25_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *ddr50_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *sdr50_pinctrl_state;
	struct omap_hsmmc_pinctrl_state *sdr104_pinctrl_state;
#endif
};

struct omap_mmc_of_data {
	u8 controller_flags;
};

#ifdef CONFIG_MMC_OMAP_HS_ADMA
struct omap_hsmmc_adma_desc {
	u8 attr;
	u8 reserved;
	u16 len;
	u32 addr;
};

#define ADMA_MAX_LEN	63488

/* Decriptor table defines */
#define ADMA_DESC_ATTR_VALID		BIT(0)
#define ADMA_DESC_ATTR_END		BIT(1)
#define ADMA_DESC_ATTR_INT		BIT(2)
#define ADMA_DESC_ATTR_ACT1		BIT(4)
#define ADMA_DESC_ATTR_ACT2		BIT(5)

#define ADMA_DESC_TRANSFER_DATA		ADMA_DESC_ATTR_ACT2
#define ADMA_DESC_LINK_DESC	(ADMA_DESC_ATTR_ACT1 | ADMA_DESC_ATTR_ACT2)
#endif

/* If we fail after 1 second wait, something is really bad */
#define MAX_RETRY_MS	1000
#define MMC_TIMEOUT_MS	20

/* DMA transfers can take a long time if a lot a data is transferred.
 * The timeout must take in account the amount of data. Let's assume
 * that the time will never exceed 333 ms per MB (in other word we assume
 * that the bandwidth is always above 3MB/s).
 */
#define DMA_TIMEOUT_PER_MB	333
#define OMAP_HSMMC_SUPPORTS_DUAL_VOLT		BIT(0)
#define OMAP_HSMMC_NO_1_8_V			BIT(1)
#define OMAP_HSMMC_USE_ADMA			BIT(2)
#define OMAP_HSMMC_REQUIRE_IODELAY		BIT(3)

static int mmc_read_data(struct hsmmc *mmc_base, char *buf, unsigned int size);
static int mmc_write_data(struct hsmmc *mmc_base, const char *buf,
			unsigned int siz);
static void omap_hsmmc_start_clock(struct hsmmc *mmc_base);
static void omap_hsmmc_stop_clock(struct hsmmc *mmc_base);
static void mmc_reset_controller_fsm(struct hsmmc *mmc_base, u32 bit);

static inline struct omap_hsmmc_data *omap_hsmmc_get_data(struct mmc *mmc)
{
#if CONFIG_IS_ENABLED(DM_MMC)
	return dev_get_priv(mmc->dev);
#else
	return (struct omap_hsmmc_data *)mmc->priv;
#endif
}
static inline struct mmc_config *omap_hsmmc_get_cfg(struct mmc *mmc)
{
#if CONFIG_IS_ENABLED(DM_MMC)
	struct omap_hsmmc_plat *plat = dev_get_platdata(mmc->dev);
	return &plat->cfg;
#else
	return &((struct omap_hsmmc_data *)mmc->priv)->cfg;
#endif
}

#if defined(OMAP_HSMMC_USE_GPIO) && !CONFIG_IS_ENABLED(DM_MMC)
static int omap_mmc_setup_gpio_in(int gpio, const char *label)
{
	int ret;

#ifndef CONFIG_DM_GPIO
	if (!gpio_is_valid(gpio))
		return -1;
#endif
	ret = gpio_request(gpio, label);
	if (ret)
		return ret;

	ret = gpio_direction_input(gpio);
	if (ret)
		return ret;

	return gpio;
}
#endif

static unsigned char mmc_board_init(struct mmc *mmc)
{
#if defined(CONFIG_OMAP34XX)
	struct mmc_config *cfg = omap_hsmmc_get_cfg(mmc);
	t2_t *t2_base = (t2_t *)T2_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	u32 pbias_lite;
#ifdef CONFIG_MMC_OMAP36XX_PINS
	u32 wkup_ctrl = readl(OMAP34XX_CTRL_WKUP_CTRL);
#endif

	pbias_lite = readl(&t2_base->pbias_lite);
	pbias_lite &= ~(PBIASLITEPWRDNZ1 | PBIASLITEPWRDNZ0);
#ifdef CONFIG_TARGET_OMAP3_CAIRO
	/* for cairo board, we need to set up 1.8 Volt bias level on MMC1 */
	pbias_lite &= ~PBIASLITEVMODE0;
#endif
#ifdef CONFIG_TARGET_OMAP3_LOGIC
	/* For Logic PD board, 1.8V bias to go enable gpio127 for mmc_cd */
	pbias_lite &= ~PBIASLITEVMODE1;
#endif
#ifdef CONFIG_MMC_OMAP36XX_PINS
	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Disable extended drain IO before changing PBIAS */
		wkup_ctrl &= ~OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ;
		writel(wkup_ctrl, OMAP34XX_CTRL_WKUP_CTRL);
	}
#endif
	writel(pbias_lite, &t2_base->pbias_lite);

	writel(pbias_lite | PBIASLITEPWRDNZ1 |
		PBIASSPEEDCTRL0 | PBIASLITEPWRDNZ0,
		&t2_base->pbias_lite);

#ifdef CONFIG_MMC_OMAP36XX_PINS
	if (get_cpu_family() == CPU_OMAP36XX)
		/* Enable extended drain IO after changing PBIAS */
		writel(wkup_ctrl |
				OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ,
				OMAP34XX_CTRL_WKUP_CTRL);
#endif
	writel(readl(&t2_base->devconf0) | MMCSDIO1ADPCLKISEL,
		&t2_base->devconf0);

	writel(readl(&t2_base->devconf1) | MMCSDIO2ADPCLKISEL,
		&t2_base->devconf1);

	/* Change from default of 52MHz to 26MHz if necessary */
	if (!(cfg->host_caps & MMC_MODE_HS_52MHz))
		writel(readl(&t2_base->ctl_prog_io1) & ~CTLPROGIO1SPEEDCTRL,
			&t2_base->ctl_prog_io1);

	writel(readl(&prcm_base->fclken1_core) |
		EN_MMC1 | EN_MMC2 | EN_MMC3,
		&prcm_base->fclken1_core);

	writel(readl(&prcm_base->iclken1_core) |
		EN_MMC1 | EN_MMC2 | EN_MMC3,
		&prcm_base->iclken1_core);
#endif

#if (defined(CONFIG_OMAP54XX) || defined(CONFIG_OMAP44XX)) &&\
	!CONFIG_IS_ENABLED(DM_REGULATOR)
	/* PBIAS config needed for MMC1 only */
	if (mmc_get_blk_desc(mmc)->devnum == 0)
		vmmc_pbias_config(LDO_VOLT_3V3);
#endif

	return 0;
}

void mmc_init_stream(struct hsmmc *mmc_base)
{
	ulong start;

	writel(readl(&mmc_base->con) | INIT_INITSTREAM, &mmc_base->con);

	writel(MMC_CMD0, &mmc_base->cmd);
	start = get_timer(0);
	while (!(readl(&mmc_base->stat) & CC_MASK)) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for cc!\n", __func__);
			return;
		}
	}
	writel(CC_MASK, &mmc_base->stat)
		;
	writel(MMC_CMD0, &mmc_base->cmd)
		;
	start = get_timer(0);
	while (!(readl(&mmc_base->stat) & CC_MASK)) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for cc2!\n", __func__);
			return;
		}
	}
	writel(readl(&mmc_base->con) & ~INIT_INITSTREAM, &mmc_base->con);
}

#if CONFIG_IS_ENABLED(DM_MMC)
#ifdef CONFIG_IODELAY_RECALIBRATION
static void omap_hsmmc_io_recalibrate(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct omap_hsmmc_pinctrl_state *pinctrl_state;

	switch (priv->mode) {
	case MMC_HS_200:
		pinctrl_state = priv->hs200_1_8v_pinctrl_state;
		break;
	case UHS_SDR104:
		pinctrl_state = priv->sdr104_pinctrl_state;
		break;
	case UHS_SDR50:
		pinctrl_state = priv->sdr50_pinctrl_state;
		break;
	case UHS_DDR50:
		pinctrl_state = priv->ddr50_pinctrl_state;
		break;
	case UHS_SDR25:
		pinctrl_state = priv->sdr25_pinctrl_state;
		break;
	case UHS_SDR12:
		pinctrl_state = priv->sdr12_pinctrl_state;
		break;
	case SD_HS:
	case MMC_HS:
	case MMC_HS_52:
		pinctrl_state = priv->hs_pinctrl_state;
		break;
	case MMC_DDR_52:
		pinctrl_state = priv->ddr_1_8v_pinctrl_state;
	default:
		pinctrl_state = priv->default_pinctrl_state;
		break;
	}

	if (!pinctrl_state)
		pinctrl_state = priv->default_pinctrl_state;

	if (priv->controller_flags & OMAP_HSMMC_REQUIRE_IODELAY) {
		if (pinctrl_state->iodelay)
			late_recalibrate_iodelay(pinctrl_state->padconf,
						 pinctrl_state->npads,
						 pinctrl_state->iodelay,
						 pinctrl_state->niodelays);
		else
			do_set_mux32((*ctrl)->control_padconf_core_base,
				     pinctrl_state->padconf,
				     pinctrl_state->npads);
	}
}
#endif
static void omap_hsmmc_set_timing(struct mmc *mmc)
{
	u32 val;
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);

	mmc_base = priv->base_addr;

	omap_hsmmc_stop_clock(mmc_base);
	val = readl(&mmc_base->ac12);
	val &= ~AC12_UHSMC_MASK;
	priv->mode = mmc->selected_mode;

	if (mmc_is_mode_ddr(priv->mode))
		writel(readl(&mmc_base->con) | DDR, &mmc_base->con);
	else
		writel(readl(&mmc_base->con) & ~DDR, &mmc_base->con);

	switch (priv->mode) {
	case MMC_HS_200:
	case UHS_SDR104:
		val |= AC12_UHSMC_SDR104;
		break;
	case UHS_SDR50:
		val |= AC12_UHSMC_SDR50;
		break;
	case MMC_DDR_52:
	case UHS_DDR50:
		val |= AC12_UHSMC_DDR50;
		break;
	case SD_HS:
	case MMC_HS_52:
	case UHS_SDR25:
		val |= AC12_UHSMC_SDR25;
		break;
	case MMC_LEGACY:
	case MMC_HS:
	case SD_LEGACY:
	case UHS_SDR12:
		val |= AC12_UHSMC_SDR12;
		break;
	default:
		val |= AC12_UHSMC_RES;
		break;
	}
	writel(val, &mmc_base->ac12);

#ifdef CONFIG_IODELAY_RECALIBRATION
	omap_hsmmc_io_recalibrate(mmc);
#endif
	omap_hsmmc_start_clock(mmc_base);
}

static void omap_hsmmc_conf_bus_power(struct mmc *mmc, uint signal_voltage)
{
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	u32 hctl, ac12;

	mmc_base = priv->base_addr;

	hctl = readl(&mmc_base->hctl) & ~SDVS_MASK;
	ac12 = readl(&mmc_base->ac12) & ~AC12_V1V8_SIGEN;

	switch (signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_330:
		hctl |= SDVS_3V3;
		break;
	case MMC_SIGNAL_VOLTAGE_180:
		hctl |= SDVS_1V8;
		ac12 |= AC12_V1V8_SIGEN;
		break;
	}

	writel(hctl, &mmc_base->hctl);
	writel(ac12, &mmc_base->ac12);
}

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
static int omap_hsmmc_wait_dat0(struct udevice *dev, int state, int timeout)
{
	int ret = -ETIMEDOUT;
	u32 con;
	bool dat0_high;
	bool target_dat0_high = !!state;
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	struct hsmmc *mmc_base = priv->base_addr;

	con = readl(&mmc_base->con);
	writel(con | CON_CLKEXTFREE | CON_PADEN, &mmc_base->con);

	timeout = DIV_ROUND_UP(timeout, 10); /* check every 10 us. */
	while (timeout--)	{
		dat0_high = !!(readl(&mmc_base->pstate) & PSTATE_DLEV_DAT0);
		if (dat0_high == target_dat0_high) {
			ret = 0;
			break;
		}
		udelay(10);
	}
	writel(con, &mmc_base->con);

	return ret;
}
#endif

#if CONFIG_IS_ENABLED(MMC_IO_VOLTAGE)
#if CONFIG_IS_ENABLED(DM_REGULATOR)
static int omap_hsmmc_set_io_regulator(struct mmc *mmc, int mV)
{
	int ret = 0;
	int uV = mV * 1000;

	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);

	if (!mmc->vqmmc_supply)
		return 0;

	/* Disable PBIAS */
	ret = regulator_set_enable_if_allowed(priv->pbias_supply, false);
	if (ret)
		return ret;

	/* Turn off IO voltage */
	ret = regulator_set_enable_if_allowed(mmc->vqmmc_supply, false);
	if (ret)
		return ret;
	/* Program a new IO voltage value */
	ret = regulator_set_value(mmc->vqmmc_supply, uV);
	if (ret)
		return ret;
	/* Turn on IO voltage */
	ret = regulator_set_enable_if_allowed(mmc->vqmmc_supply, true);
	if (ret)
		return ret;

	/* Program PBIAS voltage*/
	ret = regulator_set_value(priv->pbias_supply, uV);
	if (ret && ret != -ENOSYS)
		return ret;
	/* Enable PBIAS */
	ret = regulator_set_enable_if_allowed(priv->pbias_supply, true);
	if (ret)
		return ret;

	return 0;
}
#endif

static int omap_hsmmc_set_signal_voltage(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct hsmmc *mmc_base = priv->base_addr;
	int mv = mmc_voltage_to_mv(mmc->signal_voltage);
	u32 capa_mask;
	__maybe_unused u8 palmas_ldo_volt;
	u32 val;

	if (mv < 0)
		return -EINVAL;

	if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_330) {
		mv = 3300;
		capa_mask = VS33_3V3SUP;
		palmas_ldo_volt = LDO_VOLT_3V3;
	} else if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
		capa_mask = VS18_1V8SUP;
		palmas_ldo_volt = LDO_VOLT_1V8;
	} else {
		return -EOPNOTSUPP;
	}

	val = readl(&mmc_base->capa);
	if (!(val & capa_mask))
		return -EOPNOTSUPP;

	priv->signal_voltage = mmc->signal_voltage;

	omap_hsmmc_conf_bus_power(mmc, mmc->signal_voltage);

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	return omap_hsmmc_set_io_regulator(mmc, mv);
#elif (defined(CONFIG_OMAP54XX) || defined(CONFIG_OMAP44XX)) && \
	defined(CONFIG_PALMAS_POWER)
	if (mmc_get_blk_desc(mmc)->devnum == 0)
		vmmc_pbias_config(palmas_ldo_volt);
	return 0;
#else
	return 0;
#endif
}
#endif

static uint32_t omap_hsmmc_set_capabilities(struct mmc *mmc)
{
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	u32 val;

	mmc_base = priv->base_addr;
	val = readl(&mmc_base->capa);

	if (priv->controller_flags & OMAP_HSMMC_SUPPORTS_DUAL_VOLT) {
		val |= (VS33_3V3SUP | VS18_1V8SUP);
	} else if (priv->controller_flags & OMAP_HSMMC_NO_1_8_V) {
		val |= VS33_3V3SUP;
		val &= ~VS18_1V8SUP;
	} else {
		val |= VS18_1V8SUP;
		val &= ~VS33_3V3SUP;
	}

	writel(val, &mmc_base->capa);

	return val;
}

#ifdef MMC_SUPPORTS_TUNING
static void omap_hsmmc_disable_tuning(struct mmc *mmc)
{
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	u32 val;

	mmc_base = priv->base_addr;
	val = readl(&mmc_base->ac12);
	val &= ~(AC12_SCLK_SEL);
	writel(val, &mmc_base->ac12);

	val = readl(&mmc_base->dll);
	val &= ~(DLL_FORCE_VALUE | DLL_SWT);
	writel(val, &mmc_base->dll);
}

static void omap_hsmmc_set_dll(struct mmc *mmc, int count)
{
	int i;
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	u32 val;

	mmc_base = priv->base_addr;
	val = readl(&mmc_base->dll);
	val |= DLL_FORCE_VALUE;
	val &= ~(DLL_FORCE_SR_C_MASK << DLL_FORCE_SR_C_SHIFT);
	val |= (count << DLL_FORCE_SR_C_SHIFT);
	writel(val, &mmc_base->dll);

	val |= DLL_CALIB;
	writel(val, &mmc_base->dll);
	for (i = 0; i < 1000; i++) {
		if (readl(&mmc_base->dll) & DLL_CALIB)
			break;
	}
	val &= ~DLL_CALIB;
	writel(val, &mmc_base->dll);
}

static int omap_hsmmc_execute_tuning(struct udevice *dev, uint opcode)
{
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;
	struct hsmmc *mmc_base;
	u32 val;
	u8 cur_match, prev_match = 0;
	int ret;
	u32 phase_delay = 0;
	u32 start_window = 0, max_window = 0;
	u32 length = 0, max_len = 0;
	bool single_point_failure = false;
	struct udevice *thermal_dev;
	int temperature;
	int i;

	mmc_base = priv->base_addr;
	val = readl(&mmc_base->capa2);

	/* clock tuning is not needed for upto 52MHz */
	if (!((mmc->selected_mode == MMC_HS_200) ||
	      (mmc->selected_mode == UHS_SDR104) ||
	      ((mmc->selected_mode == UHS_SDR50) && (val & CAPA2_TSDR50))))
		return 0;

	ret = uclass_first_device(UCLASS_THERMAL, &thermal_dev);
	if (ret) {
		printf("Couldn't get thermal device for tuning\n");
		return ret;
	}
	ret = thermal_get_temp(thermal_dev, &temperature);
	if (ret) {
		printf("Couldn't get temperature for tuning\n");
		return ret;
	}
	val = readl(&mmc_base->dll);
	val |= DLL_SWT;
	writel(val, &mmc_base->dll);

	/*
	 * Stage 1: Search for a maximum pass window ignoring any
	 * any single point failures. If the tuning value ends up
	 * near it, move away from it in stage 2 below
	 */
	while (phase_delay <= MAX_PHASE_DELAY) {
		omap_hsmmc_set_dll(mmc, phase_delay);

		cur_match = !mmc_send_tuning(mmc, opcode, NULL);

		if (cur_match) {
			if (prev_match) {
				length++;
			} else if (single_point_failure) {
				/* ignore single point failure */
				length++;
				single_point_failure = false;
			} else {
				start_window = phase_delay;
				length = 1;
			}
		} else {
			single_point_failure = prev_match;
		}

		if (length > max_len) {
			max_window = start_window;
			max_len = length;
		}

		prev_match = cur_match;
		phase_delay += 4;
	}

	if (!max_len) {
		ret = -EIO;
		goto tuning_error;
	}

	val = readl(&mmc_base->ac12);
	if (!(val & AC12_SCLK_SEL)) {
		ret = -EIO;
		goto tuning_error;
	}
	/*
	 * Assign tuning value as a ratio of maximum pass window based
	 * on temperature
	 */
	if (temperature < -20000)
		phase_delay = min(max_window + 4 * max_len - 24,
				  max_window +
				  DIV_ROUND_UP(13 * max_len, 16) * 4);
	else if (temperature < 20000)
		phase_delay = max_window + DIV_ROUND_UP(9 * max_len, 16) * 4;
	else if (temperature < 40000)
		phase_delay = max_window + DIV_ROUND_UP(8 * max_len, 16) * 4;
	else if (temperature < 70000)
		phase_delay = max_window + DIV_ROUND_UP(7 * max_len, 16) * 4;
	else if (temperature < 90000)
		phase_delay = max_window + DIV_ROUND_UP(5 * max_len, 16) * 4;
	else if (temperature < 120000)
		phase_delay = max_window + DIV_ROUND_UP(4 * max_len, 16) * 4;
	else
		phase_delay = max_window + DIV_ROUND_UP(3 * max_len, 16) * 4;

	/*
	 * Stage 2: Search for a single point failure near the chosen tuning
	 * value in two steps. First in the +3 to +10 range and then in the
	 * +2 to -10 range. If found, move away from it in the appropriate
	 * direction by the appropriate amount depending on the temperature.
	 */
	for (i = 3; i <= 10; i++) {
		omap_hsmmc_set_dll(mmc, phase_delay + i);
		if (mmc_send_tuning(mmc, opcode, NULL)) {
			if (temperature < 10000)
				phase_delay += i + 6;
			else if (temperature < 20000)
				phase_delay += i - 12;
			else if (temperature < 70000)
				phase_delay += i - 8;
			else if (temperature < 90000)
				phase_delay += i - 6;
			else
				phase_delay += i - 6;

			goto single_failure_found;
		}
	}

	for (i = 2; i >= -10; i--) {
		omap_hsmmc_set_dll(mmc, phase_delay + i);
		if (mmc_send_tuning(mmc, opcode, NULL)) {
			if (temperature < 10000)
				phase_delay += i + 12;
			else if (temperature < 20000)
				phase_delay += i + 8;
			else if (temperature < 70000)
				phase_delay += i + 8;
			else if (temperature < 90000)
				phase_delay += i + 10;
			else
				phase_delay += i + 12;

			goto single_failure_found;
		}
	}

single_failure_found:

	omap_hsmmc_set_dll(mmc, phase_delay);

	mmc_reset_controller_fsm(mmc_base, SYSCTL_SRD);
	mmc_reset_controller_fsm(mmc_base, SYSCTL_SRC);

	return 0;

tuning_error:

	omap_hsmmc_disable_tuning(mmc);
	mmc_reset_controller_fsm(mmc_base, SYSCTL_SRD);
	mmc_reset_controller_fsm(mmc_base, SYSCTL_SRC);

	return ret;
}
#endif

static void omap_hsmmc_send_init_stream(struct udevice *dev)
{
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	struct hsmmc *mmc_base = priv->base_addr;

	mmc_init_stream(mmc_base);
}
#endif

static void mmc_enable_irq(struct mmc *mmc, struct mmc_cmd *cmd)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct hsmmc *mmc_base = priv->base_addr;
	u32 irq_mask = INT_EN_MASK;

	/*
	 * TODO: Errata i802 indicates only DCRC interrupts can occur during
	 * tuning procedure and DCRC should be disabled. But see occurences
	 * of DEB, CIE, CEB, CCRC interupts during tuning procedure. These
	 * interrupts occur along with BRR, so the data is actually in the
	 * buffer. It has to be debugged why these interrutps occur
	 */
	if (cmd && mmc_is_tuning_cmd(cmd->cmdidx))
		irq_mask &= ~(IE_DEB | IE_DCRC | IE_CIE | IE_CEB | IE_CCRC);

	writel(irq_mask, &mmc_base->ie);
}

static int omap_hsmmc_init_setup(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct hsmmc *mmc_base;
	unsigned int reg_val;
	unsigned int dsor;
	ulong start;

	mmc_base = priv->base_addr;
	mmc_board_init(mmc);

	writel(readl(&mmc_base->sysconfig) | MMC_SOFTRESET,
		&mmc_base->sysconfig);
	start = get_timer(0);
	while ((readl(&mmc_base->sysstatus) & RESETDONE) == 0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for cc2!\n", __func__);
			return -ETIMEDOUT;
		}
	}
	writel(readl(&mmc_base->sysctl) | SOFTRESETALL, &mmc_base->sysctl);
	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & SOFTRESETALL) != 0x0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for softresetall!\n",
				__func__);
			return -ETIMEDOUT;
		}
	}
#ifdef CONFIG_MMC_OMAP_HS_ADMA
	reg_val = readl(&mmc_base->hl_hwinfo);
	if (reg_val & MADMA_EN)
		priv->controller_flags |= OMAP_HSMMC_USE_ADMA;
#endif

#if CONFIG_IS_ENABLED(DM_MMC)
	reg_val = omap_hsmmc_set_capabilities(mmc);
	omap_hsmmc_conf_bus_power(mmc, (reg_val & VS33_3V3SUP) ?
			  MMC_SIGNAL_VOLTAGE_330 : MMC_SIGNAL_VOLTAGE_180);
#else
	writel(DTW_1_BITMODE | SDBP_PWROFF | SDVS_3V0, &mmc_base->hctl);
	writel(readl(&mmc_base->capa) | VS33_3V3SUP | VS18_1V8SUP,
		&mmc_base->capa);
#endif

	reg_val = readl(&mmc_base->con) & RESERVED_MASK;

	writel(CTPL_MMC_SD | reg_val | WPP_ACTIVEHIGH | CDP_ACTIVEHIGH |
		MIT_CTO | DW8_1_4BITMODE | MODE_FUNC | STR_BLOCK |
		HR_NOHOSTRESP | INIT_NOINIT | NOOPENDRAIN, &mmc_base->con);

	dsor = 240;
	mmc_reg_out(&mmc_base->sysctl, (ICE_MASK | DTO_MASK | CEN_MASK),
		(ICE_STOP | DTO_15THDTO));
	mmc_reg_out(&mmc_base->sysctl, ICE_MASK | CLKD_MASK,
		(dsor << CLKD_OFFSET) | ICE_OSCILLATE);
	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & ICS_MASK) == ICS_NOTREADY) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for ics!\n", __func__);
			return -ETIMEDOUT;
		}
	}
	writel(readl(&mmc_base->sysctl) | CEN_ENABLE, &mmc_base->sysctl);

	writel(readl(&mmc_base->hctl) | SDBP_PWRON, &mmc_base->hctl);

	mmc_enable_irq(mmc, NULL);

#if !CONFIG_IS_ENABLED(DM_MMC)
	mmc_init_stream(mmc_base);
#endif

	return 0;
}

/*
 * MMC controller internal finite state machine reset
 *
 * Used to reset command or data internal state machines, using respectively
 * SRC or SRD bit of SYSCTL register
 */
static void mmc_reset_controller_fsm(struct hsmmc *mmc_base, u32 bit)
{
	ulong start;

	mmc_reg_out(&mmc_base->sysctl, bit, bit);

	/*
	 * CMD(DAT) lines reset procedures are slightly different
	 * for OMAP3 and OMAP4(AM335x,OMAP5,DRA7xx).
	 * According to OMAP3 TRM:
	 * Set SRC(SRD) bit in MMCHS_SYSCTL register to 0x1 and wait until it
	 * returns to 0x0.
	 * According to OMAP4(AM335x,OMAP5,DRA7xx) TRMs, CMD(DATA) lines reset
	 * procedure steps must be as follows:
	 * 1. Initiate CMD(DAT) line reset by writing 0x1 to SRC(SRD) bit in
	 *    MMCHS_SYSCTL register (SD_SYSCTL for AM335x).
	 * 2. Poll the SRC(SRD) bit until it is set to 0x1.
	 * 3. Wait until the SRC (SRD) bit returns to 0x0
	 *    (reset procedure is completed).
	 */
#if defined(CONFIG_OMAP44XX) || defined(CONFIG_OMAP54XX) || \
	defined(CONFIG_AM33XX) || defined(CONFIG_AM43XX)
	if (!(readl(&mmc_base->sysctl) & bit)) {
		start = get_timer(0);
		while (!(readl(&mmc_base->sysctl) & bit)) {
			if (get_timer(0) - start > MMC_TIMEOUT_MS)
				return;
		}
	}
#endif
	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & bit) != 0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for sysctl %x to clear\n",
				__func__, bit);
			return;
		}
	}
}

#ifdef CONFIG_MMC_OMAP_HS_ADMA
static void omap_hsmmc_adma_desc(struct mmc *mmc, char *buf, u16 len, bool end)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct omap_hsmmc_adma_desc *desc;
	u8 attr;

	desc = &priv->adma_desc_table[priv->desc_slot];

	attr = ADMA_DESC_ATTR_VALID | ADMA_DESC_TRANSFER_DATA;
	if (!end)
		priv->desc_slot++;
	else
		attr |= ADMA_DESC_ATTR_END;

	desc->len = len;
	desc->addr = (u32)buf;
	desc->reserved = 0;
	desc->attr = attr;
}

static void omap_hsmmc_prepare_adma_table(struct mmc *mmc,
					  struct mmc_data *data)
{
	uint total_len = data->blocksize * data->blocks;
	uint desc_count = DIV_ROUND_UP(total_len, ADMA_MAX_LEN);
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	int i = desc_count;
	char *buf;

	priv->desc_slot = 0;
	priv->adma_desc_table = (struct omap_hsmmc_adma_desc *)
				memalign(ARCH_DMA_MINALIGN, desc_count *
				sizeof(struct omap_hsmmc_adma_desc));

	if (data->flags & MMC_DATA_READ)
		buf = data->dest;
	else
		buf = (char *)data->src;

	while (--i) {
		omap_hsmmc_adma_desc(mmc, buf, ADMA_MAX_LEN, false);
		buf += ADMA_MAX_LEN;
		total_len -= ADMA_MAX_LEN;
	}

	omap_hsmmc_adma_desc(mmc, buf, total_len, true);

	flush_dcache_range((long)priv->adma_desc_table,
			   (long)priv->adma_desc_table +
			   ROUND(desc_count *
			   sizeof(struct omap_hsmmc_adma_desc),
			   ARCH_DMA_MINALIGN));
}

static void omap_hsmmc_prepare_data(struct mmc *mmc, struct mmc_data *data)
{
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	u32 val;
	char *buf;

	mmc_base = priv->base_addr;
	omap_hsmmc_prepare_adma_table(mmc, data);

	if (data->flags & MMC_DATA_READ)
		buf = data->dest;
	else
		buf = (char *)data->src;

	val = readl(&mmc_base->hctl);
	val |= DMA_SELECT;
	writel(val, &mmc_base->hctl);

	val = readl(&mmc_base->con);
	val |= DMA_MASTER;
	writel(val, &mmc_base->con);

	writel((u32)priv->adma_desc_table, &mmc_base->admasal);

	flush_dcache_range((u32)buf,
			   (u32)buf +
			   ROUND(data->blocksize * data->blocks,
				 ARCH_DMA_MINALIGN));
}

static void omap_hsmmc_dma_cleanup(struct mmc *mmc)
{
	struct hsmmc *mmc_base;
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	u32 val;

	mmc_base = priv->base_addr;

	val = readl(&mmc_base->con);
	val &= ~DMA_MASTER;
	writel(val, &mmc_base->con);

	val = readl(&mmc_base->hctl);
	val &= ~DMA_SELECT;
	writel(val, &mmc_base->hctl);

	kfree(priv->adma_desc_table);
}
#else
#define omap_hsmmc_adma_desc
#define omap_hsmmc_prepare_adma_table
#define omap_hsmmc_prepare_data
#define omap_hsmmc_dma_cleanup
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
static int omap_hsmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
#else
static int omap_hsmmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;
#endif
	struct hsmmc *mmc_base;
	unsigned int flags, mmc_stat;
	ulong start;
	priv->last_cmd = cmd->cmdidx;

	mmc_base = priv->base_addr;

	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		return 0;

	start = get_timer(0);
	while ((readl(&mmc_base->pstate) & (DATI_MASK | CMDI_MASK)) != 0) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting on cmd inhibit to clear\n",
					__func__);
			return -ETIMEDOUT;
		}
	}
	writel(0xFFFFFFFF, &mmc_base->stat);
	start = get_timer(0);
	while (readl(&mmc_base->stat)) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for STAT (%x) to clear\n",
				__func__, readl(&mmc_base->stat));
			return -ETIMEDOUT;
		}
	}
	/*
	 * CMDREG
	 * CMDIDX[13:8]	: Command index
	 * DATAPRNT[5]	: Data Present Select
	 * ENCMDIDX[4]	: Command Index Check Enable
	 * ENCMDCRC[3]	: Command CRC Check Enable
	 * RSPTYP[1:0]
	 *	00 = No Response
	 *	01 = Length 136
	 *	10 = Length 48
	 *	11 = Length 48 Check busy after response
	 */
	/* Delay added before checking the status of frq change
	 * retry not supported by mmc.c(core file)
	 */
	if (cmd->cmdidx == SD_CMD_APP_SEND_SCR)
		udelay(50000); /* wait 50 ms */

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = 0;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = RSP_TYPE_LGHT136 | CICE_NOCHECK;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = RSP_TYPE_LGHT48B;
	else
		flags = RSP_TYPE_LGHT48;

	/* enable default flags */
	flags =	flags | (CMD_TYPE_NORMAL | CICE_NOCHECK | CCCE_NOCHECK |
			MSBS_SGLEBLK);
	flags &= ~(ACEN_ENABLE | BCE_ENABLE | DE_ENABLE);

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= CCCE_CHECK;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= CICE_CHECK;

	if (data) {
		if ((cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK) ||
			 (cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK)) {
			flags |= (MSBS_MULTIBLK | BCE_ENABLE | ACEN_ENABLE);
			data->blocksize = 512;
			writel(data->blocksize | (data->blocks << 16),
							&mmc_base->blk);
		} else
			writel(data->blocksize | NBLK_STPCNT, &mmc_base->blk);

		if (data->flags & MMC_DATA_READ)
			flags |= (DP_DATA | DDIR_READ);
		else
			flags |= (DP_DATA | DDIR_WRITE);

#ifdef CONFIG_MMC_OMAP_HS_ADMA
		if ((priv->controller_flags & OMAP_HSMMC_USE_ADMA) &&
		    !mmc_is_tuning_cmd(cmd->cmdidx)) {
			omap_hsmmc_prepare_data(mmc, data);
			flags |= DE_ENABLE;
		}
#endif
	}

	mmc_enable_irq(mmc, cmd);

	writel(cmd->cmdarg, &mmc_base->arg);
	udelay(20);		/* To fix "No status update" error on eMMC */
	writel((cmd->cmdidx << 24) | flags, &mmc_base->cmd);

	start = get_timer(0);
	do {
		mmc_stat = readl(&mmc_base->stat);
		if (get_timer(start) > MAX_RETRY_MS) {
			printf("%s : timeout: No status update\n", __func__);
			return -ETIMEDOUT;
		}
	} while (!mmc_stat);

	if ((mmc_stat & IE_CTO) != 0) {
		mmc_reset_controller_fsm(mmc_base, SYSCTL_SRC);
		return -ETIMEDOUT;
	} else if ((mmc_stat & ERRI_MASK) != 0)
		return -1;

	if (mmc_stat & CC_MASK) {
		writel(CC_MASK, &mmc_base->stat);
		if (cmd->resp_type & MMC_RSP_PRESENT) {
			if (cmd->resp_type & MMC_RSP_136) {
				/* response type 2 */
				cmd->response[3] = readl(&mmc_base->rsp10);
				cmd->response[2] = readl(&mmc_base->rsp32);
				cmd->response[1] = readl(&mmc_base->rsp54);
				cmd->response[0] = readl(&mmc_base->rsp76);
			} else
				/* response types 1, 1b, 3, 4, 5, 6 */
				cmd->response[0] = readl(&mmc_base->rsp10);
		}
	}

#ifdef CONFIG_MMC_OMAP_HS_ADMA
	if ((priv->controller_flags & OMAP_HSMMC_USE_ADMA) && data &&
	    !mmc_is_tuning_cmd(cmd->cmdidx)) {
		u32 sz_mb, timeout;

		if (mmc_stat & IE_ADMAE) {
			omap_hsmmc_dma_cleanup(mmc);
			return -EIO;
		}

		sz_mb = DIV_ROUND_UP(data->blocksize *  data->blocks, 1 << 20);
		timeout = sz_mb * DMA_TIMEOUT_PER_MB;
		if (timeout < MAX_RETRY_MS)
			timeout = MAX_RETRY_MS;

		start = get_timer(0);
		do {
			mmc_stat = readl(&mmc_base->stat);
			if (mmc_stat & TC_MASK) {
				writel(readl(&mmc_base->stat) | TC_MASK,
				       &mmc_base->stat);
				break;
			}
			if (get_timer(start) > timeout) {
				printf("%s : DMA timeout: No status update\n",
				       __func__);
				return -ETIMEDOUT;
			}
		} while (1);

		omap_hsmmc_dma_cleanup(mmc);
		return 0;
	}
#endif

	if (data && (data->flags & MMC_DATA_READ)) {
		mmc_read_data(mmc_base,	data->dest,
				data->blocksize * data->blocks);
	} else if (data && (data->flags & MMC_DATA_WRITE)) {
		mmc_write_data(mmc_base, data->src,
				data->blocksize * data->blocks);
	}
	return 0;
}

static int mmc_read_data(struct hsmmc *mmc_base, char *buf, unsigned int size)
{
	unsigned int *output_buf = (unsigned int *)buf;
	unsigned int mmc_stat;
	unsigned int count;

	/*
	 * Start Polled Read
	 */
	count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
	count /= 4;

	while (size) {
		ulong start = get_timer(0);
		do {
			mmc_stat = readl(&mmc_base->stat);
			if (get_timer(0) - start > MAX_RETRY_MS) {
				printf("%s: timedout waiting for status!\n",
						__func__);
				return -ETIMEDOUT;
			}
		} while (mmc_stat == 0);

		if ((mmc_stat & (IE_DTO | IE_DCRC | IE_DEB)) != 0)
			mmc_reset_controller_fsm(mmc_base, SYSCTL_SRD);

		if ((mmc_stat & ERRI_MASK) != 0)
			return 1;

		if (mmc_stat & BRR_MASK) {
			unsigned int k;

			writel(readl(&mmc_base->stat) | BRR_MASK,
				&mmc_base->stat);
			for (k = 0; k < count; k++) {
				*output_buf = readl(&mmc_base->data);
				output_buf++;
			}
			size -= (count*4);
		}

		if (mmc_stat & BWR_MASK)
			writel(readl(&mmc_base->stat) | BWR_MASK,
				&mmc_base->stat);

		if (mmc_stat & TC_MASK) {
			writel(readl(&mmc_base->stat) | TC_MASK,
				&mmc_base->stat);
			break;
		}
	}
	return 0;
}

#if CONFIG_IS_ENABLED(MMC_WRITE)
static int mmc_write_data(struct hsmmc *mmc_base, const char *buf,
			  unsigned int size)
{
	unsigned int *input_buf = (unsigned int *)buf;
	unsigned int mmc_stat;
	unsigned int count;

	/*
	 * Start Polled Write
	 */
	count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
	count /= 4;

	while (size) {
		ulong start = get_timer(0);
		do {
			mmc_stat = readl(&mmc_base->stat);
			if (get_timer(0) - start > MAX_RETRY_MS) {
				printf("%s: timedout waiting for status!\n",
						__func__);
				return -ETIMEDOUT;
			}
		} while (mmc_stat == 0);

		if ((mmc_stat & (IE_DTO | IE_DCRC | IE_DEB)) != 0)
			mmc_reset_controller_fsm(mmc_base, SYSCTL_SRD);

		if ((mmc_stat & ERRI_MASK) != 0)
			return 1;

		if (mmc_stat & BWR_MASK) {
			unsigned int k;

			writel(readl(&mmc_base->stat) | BWR_MASK,
					&mmc_base->stat);
			for (k = 0; k < count; k++) {
				writel(*input_buf, &mmc_base->data);
				input_buf++;
			}
			size -= (count*4);
		}

		if (mmc_stat & BRR_MASK)
			writel(readl(&mmc_base->stat) | BRR_MASK,
				&mmc_base->stat);

		if (mmc_stat & TC_MASK) {
			writel(readl(&mmc_base->stat) | TC_MASK,
				&mmc_base->stat);
			break;
		}
	}
	return 0;
}
#else
static int mmc_write_data(struct hsmmc *mmc_base, const char *buf,
			  unsigned int size)
{
	return -ENOTSUPP;
}
#endif
static void omap_hsmmc_stop_clock(struct hsmmc *mmc_base)
{
	writel(readl(&mmc_base->sysctl) & ~CEN_ENABLE, &mmc_base->sysctl);
}

static void omap_hsmmc_start_clock(struct hsmmc *mmc_base)
{
	writel(readl(&mmc_base->sysctl) | CEN_ENABLE, &mmc_base->sysctl);
}

static void omap_hsmmc_set_clock(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct hsmmc *mmc_base;
	unsigned int dsor = 0;
	ulong start;

	mmc_base = priv->base_addr;
	omap_hsmmc_stop_clock(mmc_base);

	/* TODO: Is setting DTO required here? */
	mmc_reg_out(&mmc_base->sysctl, (ICE_MASK | DTO_MASK),
		    (ICE_STOP | DTO_15THDTO));

	if (mmc->clock != 0) {
		dsor = DIV_ROUND_UP(MMC_CLOCK_REFERENCE * 1000000, mmc->clock);
		if (dsor > CLKD_MAX)
			dsor = CLKD_MAX;
	} else {
		dsor = CLKD_MAX;
	}

	mmc_reg_out(&mmc_base->sysctl, ICE_MASK | CLKD_MASK,
		    (dsor << CLKD_OFFSET) | ICE_OSCILLATE);

	start = get_timer(0);
	while ((readl(&mmc_base->sysctl) & ICS_MASK) == ICS_NOTREADY) {
		if (get_timer(0) - start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for ics!\n", __func__);
			return;
		}
	}

	priv->clock = MMC_CLOCK_REFERENCE * 1000000 / dsor;
	mmc->clock = priv->clock;
	omap_hsmmc_start_clock(mmc_base);
}

static void omap_hsmmc_set_bus_width(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct hsmmc *mmc_base;

	mmc_base = priv->base_addr;
	/* configue bus width */
	switch (mmc->bus_width) {
	case 8:
		writel(readl(&mmc_base->con) | DTW_8_BITMODE,
			&mmc_base->con);
		break;

	case 4:
		writel(readl(&mmc_base->con) & ~DTW_8_BITMODE,
			&mmc_base->con);
		writel(readl(&mmc_base->hctl) | DTW_4_BITMODE,
			&mmc_base->hctl);
		break;

	case 1:
	default:
		writel(readl(&mmc_base->con) & ~DTW_8_BITMODE,
			&mmc_base->con);
		writel(readl(&mmc_base->hctl) & ~DTW_4_BITMODE,
			&mmc_base->hctl);
		break;
	}

	priv->bus_width = mmc->bus_width;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int omap_hsmmc_set_ios(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
#else
static int omap_hsmmc_set_ios(struct udevice *dev)
{
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;
#endif
	struct hsmmc *mmc_base = priv->base_addr;
	int ret = 0;

	if (priv->bus_width != mmc->bus_width)
		omap_hsmmc_set_bus_width(mmc);

	if (priv->clock != mmc->clock)
		omap_hsmmc_set_clock(mmc);

	if (mmc->clk_disable)
		omap_hsmmc_stop_clock(mmc_base);
	else
		omap_hsmmc_start_clock(mmc_base);

#if CONFIG_IS_ENABLED(DM_MMC)
	if (priv->mode != mmc->selected_mode)
		omap_hsmmc_set_timing(mmc);

#if CONFIG_IS_ENABLED(MMC_IO_VOLTAGE)
	if (priv->signal_voltage != mmc->signal_voltage)
		ret = omap_hsmmc_set_signal_voltage(mmc);
#endif
#endif
	return ret;
}

#ifdef OMAP_HSMMC_USE_GPIO
#if CONFIG_IS_ENABLED(DM_MMC)
static int omap_hsmmc_getcd(struct udevice *dev)
{
	int value = -1;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	value = dm_gpio_get_value(&priv->cd_gpio);
#endif
	/* if no CD return as 1 */
	if (value < 0)
		return 1;

	return value;
}

static int omap_hsmmc_getwp(struct udevice *dev)
{
	int value = 0;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	value = dm_gpio_get_value(&priv->wp_gpio);
#endif
	/* if no WP return as 0 */
	if (value < 0)
		return 0;
	return value;
}
#else
static int omap_hsmmc_getcd(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	int cd_gpio;

	/* if no CD return as 1 */
	cd_gpio = priv->cd_gpio;
	if (cd_gpio < 0)
		return 1;

	/* NOTE: assumes card detect signal is active-low */
	return !gpio_get_value(cd_gpio);
}

static int omap_hsmmc_getwp(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	int wp_gpio;

	/* if no WP return as 0 */
	wp_gpio = priv->wp_gpio;
	if (wp_gpio < 0)
		return 0;

	/* NOTE: assumes write protect signal is active-high */
	return gpio_get_value(wp_gpio);
}
#endif
#endif

#if CONFIG_IS_ENABLED(DM_MMC)
static const struct dm_mmc_ops omap_hsmmc_ops = {
	.send_cmd	= omap_hsmmc_send_cmd,
	.set_ios	= omap_hsmmc_set_ios,
#ifdef OMAP_HSMMC_USE_GPIO
	.get_cd		= omap_hsmmc_getcd,
	.get_wp		= omap_hsmmc_getwp,
#endif
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning = omap_hsmmc_execute_tuning,
#endif
	.send_init_stream	= omap_hsmmc_send_init_stream,
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	.wait_dat0	= omap_hsmmc_wait_dat0,
#endif
};
#else
static const struct mmc_ops omap_hsmmc_ops = {
	.send_cmd	= omap_hsmmc_send_cmd,
	.set_ios	= omap_hsmmc_set_ios,
	.init		= omap_hsmmc_init_setup,
#ifdef OMAP_HSMMC_USE_GPIO
	.getcd		= omap_hsmmc_getcd,
	.getwp		= omap_hsmmc_getwp,
#endif
};
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
int omap_mmc_init(int dev_index, uint host_caps_mask, uint f_max, int cd_gpio,
		int wp_gpio)
{
	struct mmc *mmc;
	struct omap_hsmmc_data *priv;
	struct mmc_config *cfg;
	uint host_caps_val;

	priv = calloc(1, sizeof(*priv));
	if (priv == NULL)
		return -1;

	host_caps_val = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;

	switch (dev_index) {
	case 0:
		priv->base_addr = (struct hsmmc *)OMAP_HSMMC1_BASE;
		break;
#ifdef OMAP_HSMMC2_BASE
	case 1:
		priv->base_addr = (struct hsmmc *)OMAP_HSMMC2_BASE;
#if (defined(CONFIG_OMAP44XX) || defined(CONFIG_OMAP54XX) || \
	defined(CONFIG_DRA7XX) || defined(CONFIG_AM33XX) || \
	defined(CONFIG_AM43XX) || defined(CONFIG_SOC_KEYSTONE)) && \
		defined(CONFIG_HSMMC2_8BIT)
		/* Enable 8-bit interface for eMMC on OMAP4/5 or DRA7XX */
		host_caps_val |= MMC_MODE_8BIT;
#endif
		break;
#endif
#ifdef OMAP_HSMMC3_BASE
	case 2:
		priv->base_addr = (struct hsmmc *)OMAP_HSMMC3_BASE;
#if defined(CONFIG_DRA7XX) && defined(CONFIG_HSMMC3_8BIT)
		/* Enable 8-bit interface for eMMC on DRA7XX */
		host_caps_val |= MMC_MODE_8BIT;
#endif
		break;
#endif
	default:
		priv->base_addr = (struct hsmmc *)OMAP_HSMMC1_BASE;
		return 1;
	}
#ifdef OMAP_HSMMC_USE_GPIO
	/* on error gpio values are set to -1, which is what we want */
	priv->cd_gpio = omap_mmc_setup_gpio_in(cd_gpio, "mmc_cd");
	priv->wp_gpio = omap_mmc_setup_gpio_in(wp_gpio, "mmc_wp");
#endif

	cfg = &priv->cfg;

	cfg->name = "OMAP SD/MMC";
	cfg->ops = &omap_hsmmc_ops;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	cfg->host_caps = host_caps_val & ~host_caps_mask;

	cfg->f_min = 400000;

	if (f_max != 0)
		cfg->f_max = f_max;
	else {
		if (cfg->host_caps & MMC_MODE_HS) {
			if (cfg->host_caps & MMC_MODE_HS_52MHz)
				cfg->f_max = 52000000;
			else
				cfg->f_max = 26000000;
		} else
			cfg->f_max = 20000000;
	}

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

#if defined(CONFIG_OMAP34XX)
	/*
	 * Silicon revs 2.1 and older do not support multiblock transfers.
	 */
	if ((get_cpu_family() == CPU_OMAP34XX) && (get_cpu_rev() <= CPU_3XX_ES21))
		cfg->b_max = 1;
#endif

	mmc = mmc_create(cfg, priv);
	if (mmc == NULL)
		return -1;

	return 0;
}
#else

#ifdef CONFIG_IODELAY_RECALIBRATION
static struct pad_conf_entry *
omap_hsmmc_get_pad_conf_entry(const fdt32_t *pinctrl, int count)
{
	int index = 0;
	struct pad_conf_entry *padconf;

	padconf = (struct pad_conf_entry *)malloc(sizeof(*padconf) * count);
	if (!padconf) {
		debug("failed to allocate memory\n");
		return 0;
	}

	while (index < count) {
		padconf[index].offset = fdt32_to_cpu(pinctrl[2 * index]);
		padconf[index].val = fdt32_to_cpu(pinctrl[2 * index + 1]);
		index++;
	}

	return padconf;
}

static struct iodelay_cfg_entry *
omap_hsmmc_get_iodelay_cfg_entry(const fdt32_t *pinctrl, int count)
{
	int index = 0;
	struct iodelay_cfg_entry *iodelay;

	iodelay = (struct iodelay_cfg_entry *)malloc(sizeof(*iodelay) * count);
	if (!iodelay) {
		debug("failed to allocate memory\n");
		return 0;
	}

	while (index < count) {
		iodelay[index].offset = fdt32_to_cpu(pinctrl[3 * index]);
		iodelay[index].a_delay = fdt32_to_cpu(pinctrl[3 * index + 1]);
		iodelay[index].g_delay = fdt32_to_cpu(pinctrl[3 * index + 2]);
		index++;
	}

	return iodelay;
}

static const fdt32_t *omap_hsmmc_get_pinctrl_entry(u32  phandle,
						   const char *name, int *len)
{
	const void *fdt = gd->fdt_blob;
	int offset;
	const fdt32_t *pinctrl;

	offset = fdt_node_offset_by_phandle(fdt, phandle);
	if (offset < 0) {
		debug("failed to get pinctrl node %s.\n",
		      fdt_strerror(offset));
		return 0;
	}

	pinctrl = fdt_getprop(fdt, offset, name, len);
	if (!pinctrl) {
		debug("failed to get property %s\n", name);
		return 0;
	}

	return pinctrl;
}

static uint32_t omap_hsmmc_get_pad_conf_phandle(struct mmc *mmc,
						char *prop_name)
{
	const void *fdt = gd->fdt_blob;
	const __be32 *phandle;
	int node = dev_of_offset(mmc->dev);

	phandle = fdt_getprop(fdt, node, prop_name, NULL);
	if (!phandle) {
		debug("failed to get property %s\n", prop_name);
		return 0;
	}

	return fdt32_to_cpu(*phandle);
}

static uint32_t omap_hsmmc_get_iodelay_phandle(struct mmc *mmc,
					       char *prop_name)
{
	const void *fdt = gd->fdt_blob;
	const __be32 *phandle;
	int len;
	int count;
	int node = dev_of_offset(mmc->dev);

	phandle = fdt_getprop(fdt, node, prop_name, &len);
	if (!phandle) {
		debug("failed to get property %s\n", prop_name);
		return 0;
	}

	/* No manual mode iodelay values if count < 2 */
	count = len / sizeof(*phandle);
	if (count < 2)
		return 0;

	return fdt32_to_cpu(*(phandle + 1));
}

static struct pad_conf_entry *
omap_hsmmc_get_pad_conf(struct mmc *mmc, char *prop_name, int *npads)
{
	int len;
	int count;
	struct pad_conf_entry *padconf;
	u32 phandle;
	const fdt32_t *pinctrl;

	phandle = omap_hsmmc_get_pad_conf_phandle(mmc, prop_name);
	if (!phandle)
		return ERR_PTR(-EINVAL);

	pinctrl = omap_hsmmc_get_pinctrl_entry(phandle, "pinctrl-single,pins",
					       &len);
	if (!pinctrl)
		return ERR_PTR(-EINVAL);

	count = (len / sizeof(*pinctrl)) / 2;
	padconf = omap_hsmmc_get_pad_conf_entry(pinctrl, count);
	if (!padconf)
		return ERR_PTR(-EINVAL);

	*npads = count;

	return padconf;
}

static struct iodelay_cfg_entry *
omap_hsmmc_get_iodelay(struct mmc *mmc, char *prop_name, int *niodelay)
{
	int len;
	int count;
	struct iodelay_cfg_entry *iodelay;
	u32 phandle;
	const fdt32_t *pinctrl;

	phandle = omap_hsmmc_get_iodelay_phandle(mmc, prop_name);
	/* Not all modes have manual mode iodelay values. So its not fatal */
	if (!phandle)
		return 0;

	pinctrl = omap_hsmmc_get_pinctrl_entry(phandle, "pinctrl-pin-array",
					       &len);
	if (!pinctrl)
		return ERR_PTR(-EINVAL);

	count = (len / sizeof(*pinctrl)) / 3;
	iodelay = omap_hsmmc_get_iodelay_cfg_entry(pinctrl, count);
	if (!iodelay)
		return ERR_PTR(-EINVAL);

	*niodelay = count;

	return iodelay;
}

static struct omap_hsmmc_pinctrl_state *
omap_hsmmc_get_pinctrl_by_mode(struct mmc *mmc, char *mode)
{
	int index;
	int npads = 0;
	int niodelays = 0;
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(mmc->dev);
	char prop_name[11];
	struct omap_hsmmc_pinctrl_state *pinctrl_state;

	pinctrl_state = (struct omap_hsmmc_pinctrl_state *)
			 malloc(sizeof(*pinctrl_state));
	if (!pinctrl_state) {
		debug("failed to allocate memory\n");
		return 0;
	}

	index = fdt_stringlist_search(fdt, node, "pinctrl-names", mode);
	if (index < 0) {
		debug("fail to find %s mode %s\n", mode, fdt_strerror(index));
		goto err_pinctrl_state;
	}

	sprintf(prop_name, "pinctrl-%d", index);

	pinctrl_state->padconf = omap_hsmmc_get_pad_conf(mmc, prop_name,
							 &npads);
	if (IS_ERR(pinctrl_state->padconf))
		goto err_pinctrl_state;
	pinctrl_state->npads = npads;

	pinctrl_state->iodelay = omap_hsmmc_get_iodelay(mmc, prop_name,
							&niodelays);
	if (IS_ERR(pinctrl_state->iodelay))
		goto err_padconf;
	pinctrl_state->niodelays = niodelays;

	return pinctrl_state;

err_padconf:
	kfree(pinctrl_state->padconf);

err_pinctrl_state:
	kfree(pinctrl_state);
	return 0;
}

#define OMAP_HSMMC_SETUP_PINCTRL(capmask, mode, optional)		\
	do {								\
		struct omap_hsmmc_pinctrl_state *s = NULL;		\
		char str[20];						\
		if (!(cfg->host_caps & capmask))			\
			break;						\
									\
		if (priv->hw_rev) {					\
			sprintf(str, "%s-%s", #mode, priv->hw_rev);	\
			s = omap_hsmmc_get_pinctrl_by_mode(mmc, str);	\
		}							\
									\
		if (!s)							\
			s = omap_hsmmc_get_pinctrl_by_mode(mmc, #mode);	\
									\
		if (!s && !optional) {					\
			debug("%s: no pinctrl for %s\n",		\
			      mmc->dev->name, #mode);			\
			cfg->host_caps &= ~(capmask);			\
		} else {						\
			priv->mode##_pinctrl_state = s;			\
		}							\
	} while (0)

static int omap_hsmmc_get_pinctrl_state(struct mmc *mmc)
{
	struct omap_hsmmc_data *priv = omap_hsmmc_get_data(mmc);
	struct mmc_config *cfg = omap_hsmmc_get_cfg(mmc);
	struct omap_hsmmc_pinctrl_state *default_pinctrl;

	if (!(priv->controller_flags & OMAP_HSMMC_REQUIRE_IODELAY))
		return 0;

	default_pinctrl = omap_hsmmc_get_pinctrl_by_mode(mmc, "default");
	if (!default_pinctrl) {
		printf("no pinctrl state for default mode\n");
		return -EINVAL;
	}

	priv->default_pinctrl_state = default_pinctrl;

	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(UHS_SDR104), sdr104, false);
	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(UHS_SDR50), sdr50, false);
	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(UHS_DDR50), ddr50, false);
	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(UHS_SDR25), sdr25, false);
	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(UHS_SDR12), sdr12, false);

	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(MMC_HS_200), hs200_1_8v, false);
	OMAP_HSMMC_SETUP_PINCTRL(MMC_CAP(MMC_DDR_52), ddr_1_8v, false);
	OMAP_HSMMC_SETUP_PINCTRL(MMC_MODE_HS, hs, true);

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
#ifdef CONFIG_OMAP54XX
__weak const struct mmc_platform_fixups *platform_fixups_mmc(uint32_t addr)
{
	return NULL;
}
#endif

static int omap_hsmmc_ofdata_to_platdata(struct udevice *dev)
{
	struct omap_hsmmc_plat *plat = dev_get_platdata(dev);
	struct omap_mmc_of_data *of_data = (void *)dev_get_driver_data(dev);

	struct mmc_config *cfg = &plat->cfg;
#ifdef CONFIG_OMAP54XX
	const struct mmc_platform_fixups *fixups;
#endif
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	plat->base_addr = map_physmem(devfdt_get_addr(dev),
				      sizeof(struct hsmmc *),
				      MAP_NOCACHE);

	ret = mmc_of_parse(dev, cfg);
	if (ret < 0)
		return ret;

	if (!cfg->f_max)
		cfg->f_max = 52000000;
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->f_min = 400000;
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
	if (fdtdec_get_bool(fdt, node, "ti,dual-volt"))
		plat->controller_flags |= OMAP_HSMMC_SUPPORTS_DUAL_VOLT;
	if (fdtdec_get_bool(fdt, node, "no-1-8-v"))
		plat->controller_flags |= OMAP_HSMMC_NO_1_8_V;
	if (of_data)
		plat->controller_flags |= of_data->controller_flags;

#ifdef CONFIG_OMAP54XX
	fixups = platform_fixups_mmc(devfdt_get_addr(dev));
	if (fixups) {
		plat->hw_rev = fixups->hw_rev;
		cfg->host_caps &= ~fixups->unsupported_caps;
		cfg->f_max = fixups->max_freq;
	}
#endif

	return 0;
}
#endif

#ifdef CONFIG_BLK

static int omap_hsmmc_bind(struct udevice *dev)
{
	struct omap_hsmmc_plat *plat = dev_get_platdata(dev);
	plat->mmc = calloc(1, sizeof(struct mmc));
	return mmc_bind(dev, plat->mmc, &plat->cfg);
}
#endif
static int omap_hsmmc_probe(struct udevice *dev)
{
	struct omap_hsmmc_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct omap_hsmmc_data *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct mmc *mmc;
#ifdef CONFIG_IODELAY_RECALIBRATION
	int ret;
#endif

	cfg->name = "OMAP SD/MMC";
	priv->base_addr = plat->base_addr;
	priv->controller_flags = plat->controller_flags;
	priv->hw_rev = plat->hw_rev;

#ifdef CONFIG_BLK
	mmc = plat->mmc;
#else
	mmc = mmc_create(cfg, priv);
	if (mmc == NULL)
		return -1;
#endif
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	device_get_supply_regulator(dev, "pbias-supply",
				    &priv->pbias_supply);
#endif
#if defined(OMAP_HSMMC_USE_GPIO)
#if CONFIG_IS_ENABLED(OF_CONTROL) && CONFIG_IS_ENABLED(DM_GPIO)
	gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio, GPIOD_IS_IN);
	gpio_request_by_name(dev, "wp-gpios", 0, &priv->wp_gpio, GPIOD_IS_IN);
#endif
#endif

	mmc->dev = dev;
	upriv->mmc = mmc;

#ifdef CONFIG_IODELAY_RECALIBRATION
	ret = omap_hsmmc_get_pinctrl_state(mmc);
	/*
	 * disable high speed modes for the platforms that require IO delay
	 * and for which we don't have this information
	 */
	if ((ret < 0) &&
	    (priv->controller_flags & OMAP_HSMMC_REQUIRE_IODELAY)) {
		priv->controller_flags &= ~OMAP_HSMMC_REQUIRE_IODELAY;
		cfg->host_caps &= ~(MMC_CAP(MMC_HS_200) | MMC_CAP(MMC_DDR_52) |
				    UHS_CAPS);
	}
#endif

	return omap_hsmmc_init_setup(mmc);
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)

static const struct omap_mmc_of_data dra7_mmc_of_data = {
	.controller_flags = OMAP_HSMMC_REQUIRE_IODELAY,
};

static const struct udevice_id omap_hsmmc_ids[] = {
	{ .compatible = "ti,omap3-hsmmc" },
	{ .compatible = "ti,omap4-hsmmc" },
	{ .compatible = "ti,am33xx-hsmmc" },
	{ .compatible = "ti,dra7-hsmmc", .data = (ulong)&dra7_mmc_of_data },
	{ }
};
#endif

U_BOOT_DRIVER(omap_hsmmc) = {
	.name	= "omap_hsmmc",
	.id	= UCLASS_MMC,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = omap_hsmmc_ids,
	.ofdata_to_platdata = omap_hsmmc_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct omap_hsmmc_plat),
#endif
#ifdef CONFIG_BLK
	.bind = omap_hsmmc_bind,
#endif
	.ops = &omap_hsmmc_ops,
	.probe	= omap_hsmmc_probe,
	.priv_auto_alloc_size = sizeof(struct omap_hsmmc_data),
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
};
#endif
