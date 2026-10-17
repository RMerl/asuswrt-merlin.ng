#include <common.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

void eth_init_board(void)
{
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
}
