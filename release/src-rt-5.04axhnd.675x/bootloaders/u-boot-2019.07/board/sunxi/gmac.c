#include <common.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>

void eth_init_board(void)
{
	int pin;
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* Set MII clock */
#ifdef CONFIG_RGMII
	setbits_le32(&ccm->gmac_clk_cfg, CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII |
		CCM_GMAC_CTRL_GPIT_RGMII);
	setbits_le32(&ccm->gmac_clk_cfg,
		     CCM_GMAC_CTRL_TX_CLK_DELAY(CONFIG_GMAC_TX_DELAY));
#else
	setbits_le32(&ccm->gmac_clk_cfg, CCM_GMAC_CTRL_TX_CLK_SRC_MII |
		CCM_GMAC_CTRL_GPIT_MII);
#endif

#ifndef CONFIG_MACH_SUN6I
	/* Configure pin mux settings for GMAC */
#ifdef CONFIG_SUN7I_GMAC_FORCE_TXERR
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(17); pin++) {
#else
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(16); pin++) {
#endif
#ifdef CONFIG_RGMII
		/* skip unused pins in RGMII mode */
		if (pin == SUNXI_GPA(9) || pin == SUNXI_GPA(14))
			continue;
#endif
		sunxi_gpio_set_cfgpin(pin, SUN7I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
#elif defined CONFIG_RGMII
	/* Configure sun6i RGMII mode pin mux settings */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(3); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
	for (pin = SUNXI_GPA(9); pin <= SUNXI_GPA(14); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
	for (pin = SUNXI_GPA(19); pin <= SUNXI_GPA(20); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
	for (pin = SUNXI_GPA(25); pin <= SUNXI_GPA(27); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
#elif defined CONFIG_GMII
	/* Configure sun6i GMII mode pin mux settings */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(27); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 2);
	}
#else
	/* Configure sun6i MII mode pin mux settings */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(3); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
	for (pin = SUNXI_GPA(8); pin <= SUNXI_GPA(9); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
	for (pin = SUNXI_GPA(11); pin <= SUNXI_GPA(14); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
	for (pin = SUNXI_GPA(19); pin <= SUNXI_GPA(24); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
	for (pin = SUNXI_GPA(26); pin <= SUNXI_GPA(27); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_GMAC);
#endif
}
