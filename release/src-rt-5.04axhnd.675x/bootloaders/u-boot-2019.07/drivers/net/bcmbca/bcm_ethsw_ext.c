/* SPDX-License-Identifier: GPL-2.0+
*  *
*   *  Copyright 2019 Broadcom Ltd.
*    */


#include <config.h>
#include <common.h>
#include <stdlib.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/ioport.h>

//#include "asm/arch/ethsw.h"
#include "mii_shared.h"
//#include "pmc_drv.h"
//#include "pmc_switch.h"
#include "bcm_ethsw.h"

#define BP_MAX_SWITCH_PORTS                     8
#define BP_MAX_ENET_MACS                        2
#if 0
#define ETHSW_MDIO_BUSY                       (1 << 29)
#define ETHSW_MDIO_FAIL                       (1 << 28)
#define ETHSW_MDIO_CMD_SHIFT                  26
#define ETHSW_MDIO_CMD_MASK                   (0x3<<ETHSW_MDIO_CMD_SHIFT) 
#define ETHSW_MDIO_CMD_C22_READ               2
#define ETHSW_MDIO_CMD_C22_WRITE              1
#define ETHSW_MDIO_C22_PHY_ADDR_SHIFT         21
#define ETHSW_MDIO_C22_PHY_ADDR_MASK          (0x1f<<ETHSW_MDIO_C22_PHY_ADDR_SHIFT)
#define ETHSW_MDIO_C22_PHY_REG_SHIFT          16
#define ETHSW_MDIO_C22_PHY_REG_MASK           (0x1f<<ETHSW_MDIO_C22_PHY_REG_SHIFT)
#define ETHSW_MDIO_PHY_DATA_SHIFT             0
#define ETHSW_MDIO_PHY_DATA_MASK              (0xffff<<ETHSW_MDIO_PHY_DATA_SHIFT)

typedef struct sw_mdio {
	uint32_t mdio_cmd;
	uint32_t mdio_cfg;
	uint32_t mdio_lvl_irq_clr;
	uint32_t mdio_lvl_irq_msk;

} sw_mdio;
#endif
struct bcmbca_extsw_priv {
	bcm_ethsw_ops_t ops;
	volatile uint32_t *serdes_cntrl;    // for 47622 only
	int ext_sw_sgmii;
	void *rgmii_ctrl;
	void *gpio_pad_ctrl;
	void *rgmii_select;
};

#if defined(CONFIG_BCM6756)
#define     RGMII_VER7  7   /* for 6756 device          rgmii_ctrl   gpio_pad_ctrl */
#endif
 
typedef struct {
    int delay_rx, delay_tx;
    int is_1p8v, is_3p3v;
    int pin_offset;
} rgmii_params;

#define RGMII_CTRL_REG                  (priv->rgmii_ctrl + 0)
#define RGMII_RX_CLOCK_DELAY_CNTRL      (priv->rgmii_ctrl + 8)
#define RGMII_SELECT                    (priv->rgmii_select)

#define GPIO_PAD_CTRL               (priv->gpio_pad_ctrl + 0x0040)
#define GPIO_TestPortBlkDataMsb     (priv->gpio_pad_ctrl + 0x0054)
#define GPIO_TestPortBlkDataLsb     (priv->gpio_pad_ctrl + 0x0058)
#define GPIO_TestPortCommand        (priv->gpio_pad_ctrl + 0x005c)

#define LOAD_PAD_CTRL_CMD           0x22

#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)(_a))
#define WRITE_32(a, r) writel(*(uint32_t *)&(r), DEVICE_ADDRESS(a))
#define READ_32(a, r) (*(volatile uint32_t *)&(r) = readl(DEVICE_ADDRESS(a)))

static void bcm_set_padctrl(struct bcmbca_extsw_priv *priv, unsigned int pin_num, unsigned int pad_ctrl)
{
    unsigned int tp_blk_data_msb, tp_blk_data_lsb, tp_cmd;

    tp_cmd = LOAD_PAD_CTRL_CMD;
    tp_blk_data_msb = 0;
    tp_blk_data_lsb = 0;
    tp_blk_data_lsb |= pin_num;
    tp_blk_data_lsb |= pad_ctrl;

    WRITE_32(GPIO_TestPortBlkDataMsb, tp_blk_data_msb);
    WRITE_32(GPIO_TestPortBlkDataLsb, tp_blk_data_lsb);
    WRITE_32(GPIO_TestPortCommand, tp_cmd);
}

static void bcm_misc_hw_xmii_pads_init(struct bcmbca_extsw_priv *priv, rgmii_params *params)
{
    int num;
    uint32_t tp_data;
    uint32_t pad_ctrl;

    for (num = 0; num < 12; num++)
    {
        tp_data = 0;
#if defined(RGMII_VER7)
        tp_data |= (6 /*14mA*/ << 12);
        tp_data |= (((params->is_1p8v && (num < 6)) ? 1 : 0) << 15);    /* pad_amp_en */
        tp_data |= ((num < 6 ? 0 : 1) << 16);       /*pad_ind - 0 for RX pads, 1 for TX pads */
#else
        tp_data |= (6 /*14mA*/ << 12);
        tp_data |= ((params->is_1p8v ? 1 : 0) << 15);   /* pad_amp_en */
        tp_data |= ((params->is_3p3v ? 1 : 0) << 17);   /* pad_sel_gmii */
        tp_data |= ((num < 6 ? 0 : 1) << 16);       /*pad_ind - 0 for RX pads, 1 for TX pads */
#endif
        bcm_set_padctrl(priv, num+params->pin_offset, tp_data);
    }
}

static void rgmii_attach(struct udevice *dev, rgmii_params *params)
{
	struct bcmbca_extsw_priv *priv = dev_get_priv(dev);
    uint32_t val;

    READ_32(RGMII_CTRL_REG, val);
    val |= (1 << 0);  /* RGMII_MODE_EN=1 */
    val &= ~(7 << 2); /* Clear PORT_MODE */
    val |= (3 << 2);  /* RGMII mode */

    if (params->delay_tx)
        val &= ~(1 << 1); /* ID_MODE_DIS=0 */
    else
        val |= (1 << 1); /* ID_MODE_DIS=1 */
    WRITE_32(RGMII_CTRL_REG, val);

#if defined(RGMII_VER7)
    val = (params->delay_rx) ? 0xc8 : 0xe8 /*RXCLK_DLY_MODE_BYPASS*/;
#else
    val = (params->delay_rx) ? 0xc8 : 0xf8 /*ETHSW_RXCLK_IDDQ|ETHSW_RXCLK_BYPASS*/;
#endif
    WRITE_32(RGMII_RX_CLOCK_DELAY_CNTRL, val);

    READ_32(GPIO_PAD_CTRL, val);
#if defined(RGMII_VER7)
    if (params->is_1p8v)
        val &= ~(1 << 8);
    else
        val |= (1 << 8); /* rgmii_0_pad_modehv = 1 */
#else
    if (params->is_1p8v)
        val = (val & ~(1<<8)) | (1<<10);             // 1.8v: & ~MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_AMP_EN
    else if (params->is_3p3v)
        val = (val | (1<<8)) | (1<<9) & ~(1<<10);    // 3.3v: | MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN
    else
        val = (val | (1<<8)) & ~(1<<9) & ~(1<<10);   // 2.5v: | MISC_XMII_PAD_MODEHV & ~MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN
#endif
    WRITE_32(GPIO_PAD_CTRL, val);
    
    bcm_misc_hw_xmii_pads_init(priv, params);
    if (RGMII_SELECT) {
        val = 2;
        WRITE_32(RGMII_SELECT, val);      // set crossbar to RGMII
    }
}

//static volatile struct sw_mdio *ETHSW_MDIO = NULL;
static int ext_sw_id = 0;
static int ext_sw_sgmii = 0;

//// external switch defintion & operations
#define PBMAP_MIPS 0x100

static uint32_t sw_rreg(int page, int reg, int len)
{
    uint16_t val;
    uint32_t data32 = 0;
    uint16_t *data = (uint16_t *)&data32;
    int i;

    val = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val);

    val = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val);

    for (i = 0; i < 20; i++) {
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        if ((val & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20) {
        printf("sf2_rreg: mdio timeout!\n");
        return data32;
    }

    switch (len) {
    case 1:
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
        data[0] = (uint8_t)val; break;
    case 2:
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
        data[0] = val; break;
    case 4:
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
        data[0] = val;
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25);
        data[1] = val; break;
    default:
        printf("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        break;
    }
    //printk("sf2_rreg(page=%x reg=%x len=%d %04x %04x %04x %04x)\n", page, reg, len, data[0], data[1], data[2],data[3]);
    return data32;
}

static void sw_wreg(int page, int reg, uint32_t data_in, int len)
{
    uint16_t val;
    uint16_t *data = (uint16_t *)&data_in;
    int i;

    val = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val);

    switch (len) {
    case 1:
        val = (uint8_t)(data[0]);
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val); break;
    case 2:
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]); break;
    case 4:
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]);
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, data[1]); break;
    default:
        printf("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        return;
    }

    val = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val);

    for (i = 0; i < 20; i++) {
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        if ((val & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20)
        printf("sf2_wreg: mdio timeout!\n");
}

static void sw_reset(unsigned short configType)
{
    //uint32_t val;
    
    // todo: check (configType == BP_ENET_CONFIG_MDIO), now assume MDIO
    ext_sw_id = sw_rreg(PAGE_MANAGEMENT, REG_DEVICE_ID, 4);
    printf("Software Resetting Switch (Id=%x) ... ", ext_sw_id);
    //val = sw_rreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, 1);
    //sw_wreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, val|SOFTWARE_RESET|EN_SW_RST, 1);
    sw_wreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, 0x83, 1);
    for (; sw_rreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, 1)&SOFTWARE_RESET;) udelay(100);
    printf("Done.\n");
    udelay(1000);
}

static void sw_hw_ready(void)
{
    int i;

	printf("switch id 0x%x\n", ext_sw_id);
    if (!ext_sw_id) return;
    printf("Waiting MAC port Rx/Tx to be enabled by hardware ...");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Wait until hardware enable the ports, or we will kill the hardware */
        for(;sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1) & PORT_CTRL_RX_DISABLE; udelay(100));
    }
}

static void sw_setup(void)
{
    int i;
    uint32_t val;

    if (!ext_sw_id) return;

    // check switch SGMII/RGMII boardparam matching 53134 strap value, if not display warning
    val = sw_rreg(PAGE_STATUS,REG_STRAP_VAL,4);
    if (val&REG_STRAP_P8_SEL_SGMII)
    {
        if (!ext_sw_sgmii) printk("\n\e[0;93;41m!!!! Error: 53134 P8_SEL_SGMII is strapped high, but boardId selected is using RGMII interconnect.!!!!\e[0m\n\n");
    }
    else
    {
        if (ext_sw_sgmii) printk("\n\e[0;93;41m!!!! Error: 53134 P8_SEL_SGMII is strapped low, but boardId selected is using SGMII interconnect.!!!!\e[0m\n\n");
    }

    printk("Disable Switch All MAC port Rx/Tx\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        val = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1);
        sw_wreg(PAGE_CONTROL, PORT_CTRL_PORT+i, val|PORT_CTRL_RXTX_DISABLE, 1);
    }

    /* Set switch to unmanaged mode and enable forwarding */
    val = sw_rreg(PAGE_CONTROL,REG_SWITCH_MODE,1) | REG_SWITCH_MODE_SW_FWDG_EN | REG_SWITCH_MODE_RETRY_LIMIT_DIS;
    sw_wreg(PAGE_CONTROL,REG_SWITCH_MODE, val & ~REG_SWITCH_MODE_FRAME_MANAGE_MODE, 1);
    sw_wreg(PAGE_MANAGEMENT,REG_BRCM_HDR_CTRL,0,1);
    val = sw_rreg(PAGE_CONTROL,REG_SWITCH_CONTROL,2) | REG_SWITCH_CONTROL_MII_DUMP_FWD_EN;
    sw_wreg(PAGE_CONTROL,REG_SWITCH_CONTROL,val,2);
    val = REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_FLOW_CONTROL|REG_CONTROL_MPSO_SPEED1000|REG_CONTROL_MPSO_FDX|REG_CONTROL_MPSO_LINKPASS;
    sw_wreg(PAGE_CONTROL,REG_CONTROL_MII1_PORT_STATE_OVERRIDE,val,1);

    if (ext_sw_sgmii)
    {
        sw_wreg(0xe6, 0x00, 0x0001, 1);
        sw_wreg(0x14, 0x3e, 0x8000, 2);  // BLK0 Block Address
        sw_wreg(0x14, 0x20, 0x0c2f, 2);  // disable pll start sequencer
        sw_wreg(0x14, 0x3e, 0x8300, 2);  // Digital Block Address
        sw_wreg(0x14, 0x20, 0x010d, 2);  // enable fiber mode
        sw_wreg(0x14, 0x30, 0xc010, 2);  // force 2.5G fiber enable, 50Mhz refclk

        sw_wreg(0x14, 0x3e, 0x8340, 2);  // Digital5 Block Addres
        sw_wreg(0x14, 0x34, 0x0001, 2);  // set os2 mode
        sw_wreg(0x14, 0x3e, 0x8000, 2);  // BLK0 Block Address
        sw_wreg(0x14, 0x00, 0x0140, 2);  // disable AN, set 1G mode
        sw_wreg(0x14, 0x20, 0x2c2f, 2);  // enable pll start sequencer

        sw_wreg(PAGE_CONTROL, 0x5d, 0x004a, 1); // port 5 override  no override
        sw_wreg(PAGE_CONTROL, 0x0e, 0x008b, 1); // imp port override 2.5g duplex link up
    }
}

static int saved = 0; 
static uint32_t portCtrl[BP_MAX_SWITCH_PORTS], pbvlan[BP_MAX_SWITCH_PORTS];

static void sw_reg_save(void)
{
    int i;

    if (saved) return;
    saved = 1;
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        portCtrl[i] = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1);
        pbvlan[i] = sw_rreg(PAGE_PORT_BASED_VLAN,REG_VLAN_CTRL_P0+i*2,2);
    }
}

static void sw_reg_restore(void)
{
    int i;
    uint32_t val;

    if (!saved) return;
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        val = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1) & PORT_CTRL_SWITCH_RESERVE;
        val |= portCtrl[i] & ~PORT_CTRL_SWITCH_RESERVE;
        sw_wreg(PAGE_CONTROL,PORT_CTRL_PORT+i,val,1);
        sw_wreg(PAGE_PORT_BASED_VLAN,REG_VLAN_CTRL_P0+i*2, pbvlan[i],2);
    }
}

static void sw_open(void)
{
    int i;
    uint32_t val;

    if (!ext_sw_id) return;
    sw_reg_save();

    printk ("Enable Switch MAC Port Rx/Tx, set PBVLAN to FAN out, set switch to NO-STP.\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Set Port VLAN to allow CPU traffic only */
        sw_wreg(PAGE_PORT_BASED_VLAN,REG_VLAN_CTRL_P0+i*2,PBMAP_MIPS,2);

        /* Setting switch to NO-STP mode; enable port TX/RX. */
        val = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1) & (~(PORT_CTRL_RXTX_DISABLE|PORT_CTRL_PORT_STATUS_M));
        sw_wreg(PAGE_CONTROL,PORT_CTRL_PORT+i,val|PORT_CTRL_NO_STP,1);
    }
}

static void sw_close(void)
{
    if (!ext_sw_id) return;
    //sw_reg_restore();
}

static uint16_t serdesRef50mVco6p25 [] =
{
    0x8000, 0x0c2f,
    0x8308, 0xc000,
    0x8050, 0x5740,
    0x8051, 0x01d0,
    0x8052, 0x19f0,
    0x8053, 0xaab0,
    0x8054, 0x8821,
    0x8055, 0x0044,
    0x8056, 0x8000,
    0x8057, 0x0872,
    0x8058, 0x0000,

    0x8106, 0x0020,
    0x8054, 0x8021,
    0x8054, 0x8821,
};

static uint16_t serdesSet2p5GFiber [] =
{
    0x0010, 0x0C2F,       /* disable pll start sequencer */
    0x8066, 0x0009,       /* Set AFE for 2.5G */
    0x8065, 0x1620,       
    0x8300, 0x0149,       /* enable fiber mode, also depend board parameters */
    0x8308, 0xC010,       /* Force 2.5G Fiber, enable 50MHz refclk */
    0x834a, 0x0001,       /* Set os2 mode */
    0x0000, 0x0140,       /* disable AN, set 1G mode */
    0x0010, 0x2C2F,       /* enable pll start sequencer */
};

uint16_t phy_read_ext_bank_reg(int phy_id, int reg)
{
    uint16_t bank = reg & BRCM_MIIEXT_BANK_MASK;;
    uint16_t offset = (reg & BRCM_MIIEXT_OFF_MASK) + BRCM_MIIEXT_OFFSET;
    uint16_t val;
    
    bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, bank);
    val = bcm_ethsw_phy_read_reg(phy_id, offset);
    if (bank != BRCM_MIIEXT_DEF_BANK || offset == BRCM_MIIEXT_OFFSET)
        bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, BRCM_MIIEXT_DEF_BANK);
        
    return val;
}

void phy_write_ext_bank_reg(int phy_id, uint16_t reg, uint16_t data)
{
    uint16_t bank = reg & BRCM_MIIEXT_BANK_MASK;;
    uint16_t offset = (reg & BRCM_MIIEXT_OFF_MASK) + BRCM_MIIEXT_OFFSET;

    bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, bank);
    bcm_ethsw_phy_write_reg(phy_id, offset, data);
    if (bank != BRCM_MIIEXT_DEF_BANK || offset == BRCM_MIIEXT_OFFSET)
        bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, BRCM_MIIEXT_DEF_BANK);
}

static void config_serdes(int phy_addr, uint16_t seq[], int seqSize)
{
    int i;
    seqSize /= sizeof(seq[0]);
    for (i=0; i<seqSize; i+=2)
        if (seq[i] < 0x20)  // CL22 space
            bcm_ethsw_phy_write_reg(phy_addr, seq[i], seq[i+1]);
        else
            phy_write_ext_bank_reg(phy_addr, seq[i], seq[i+1]);
}

#define SWITCH_REG_SERDES_IDDQ       (1<<0)
#define SWITCH_REG_SERDES_PWRDWN     (1<<1)
#define SWITCH_REG_SERDES_RESETPLL   (1<<3)
#define SWITCH_REG_SERDES_RESETMDIO  (1<<4)
#define SWITCH_REG_SERDES_RESET      (1<<5)

static void phy_sgmii_init(volatile uint32_t *serdes_regp)
{
    int phy_addr = ext_sw_sgmii & BCM_PHY_ID_M;
    uint32_t val32 = *serdes_regp;
    
    val32 |= SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET;
    val32 &= ~(SWITCH_REG_SERDES_IDDQ|SWITCH_REG_SERDES_PWRDWN);
    *serdes_regp = val32;
    udelay(1000);
    val32 &= ~(SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET);
    *serdes_regp = val32;
    udelay(1000);

    // do dummy MDIO read to workaround ASIC problem
    bcm_ethsw_phy_read_reg(phy_addr, 0);

    config_serdes(phy_addr, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    udelay(1000);
    // serdesSet2p5GFiber
    config_serdes(phy_addr, serdesSet2p5GFiber, sizeof(serdesSet2p5GFiber));
}

static void bcm_ethsw_ext_init (struct udevice *dev)
{
	struct bcmbca_extsw_priv *priv = dev_get_priv(dev);
    rgmii_params params = {};

	sw_setup();
	if (priv->rgmii_ctrl) {
	    params.delay_rx = 1; params.delay_tx = 1;
	    params.is_3p3v = 1;
	    params.pin_offset = 56;
	    rgmii_attach(dev, &params);
	}
}

static void bcm_ethsw_ext_open (struct udevice *dev)
{
	struct bcmbca_extsw_priv *priv = dev_get_priv(dev);

	sw_open();
	// When using the SGMII for communicating with external switch
	if (ext_sw_sgmii && priv->serdes_cntrl) 
	{
    	uint32_t val32 = priv->serdes_cntrl[0];
		printf("serdes_cntrl %p reads 0x%x\n", priv->serdes_cntrl, val32);
		phy_sgmii_init(priv->serdes_cntrl);
	}
}

static void bcm_ethsw_ext_close (struct udevice *dev)
{
	printk("Restore Switch's MAC port Rx/Tx, PBVLAN back.\n");
	sw_close();
}

#include <asm-generic/gpio.h>

static int extsw_probe(struct udevice *dev)
{
	int ret;
	struct resource res;
	struct bcmbca_extsw_priv *priv = dev_get_priv(dev);
	struct gpio_desc reset_gpiod;

	priv->ops.init  = bcm_ethsw_ext_init;
	priv->ops.open  = bcm_ethsw_ext_open;
	priv->ops.close = bcm_ethsw_ext_close;

	ret = dev_read_resource_byname(dev, "systemport-serdes-cntrl", &res);
	priv->serdes_cntrl = NULL;
	if (!ret) {
		priv->serdes_cntrl = devm_ioremap(dev, res.start, resource_size(&res));
	}

	ret = dev_read_resource_byname(dev, "parent-rgmii-ctrl", &res);
	priv->rgmii_ctrl = NULL;
	if (!ret) {
		priv->rgmii_ctrl = devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "parent-gpio-pad-ctrl", &res);
	priv->gpio_pad_ctrl = NULL;
	if (!ret) {
		priv->gpio_pad_ctrl = devm_ioremap(dev, res.start, resource_size(&res));
	}
	ret = dev_read_resource_byname(dev, "parent-rgmii-select", &res);
	priv->rgmii_select = NULL;
	if (!ret) {
		priv->rgmii_select = devm_ioremap(dev, res.start, resource_size(&res));
	}


    if (gpio_request_by_name(dev, "switch-reset", 0, &reset_gpiod, GPIOD_IS_OUT) != -ENOENT) {
        printk("Using GPIO %d to lift external switch out of Reset\n", reset_gpiod.offset);
        dm_gpio_set_dir(&reset_gpiod);
        dm_gpio_set_value(&reset_gpiod, 1);   /* reset active */
        mdelay(100);
        dm_gpio_set_value(&reset_gpiod, 0);   /* reset clear */
        mdelay(100);
    }
	sw_reset(0);

	sw_hw_ready();
	// FIXME: skipping RGMII setup
	priv->ext_sw_sgmii = 0;
	priv->ext_sw_sgmii = dev_read_u32_default(dev, "extswsgmii_addr", 0);
	ext_sw_sgmii = priv->ext_sw_sgmii;



	return 0;
}


static const struct udevice_id bcmbca_extsw_match_ids[] = {
	{ .compatible = "brcm,bcmbca-extsw"},
	{ }
};

#if defined(CONFIG_BCM6756)
U_BOOT_DRIVER(ethsw_ext) = {
#else
U_BOOT_DRIVER(ethsw) = {
#endif
	.name = "brcm,ethsw_ext",
	.id = UCLASS_NOP,
	.of_match = bcmbca_extsw_match_ids,
	.flags  = DM_REMOVE_ACTIVE_ALL,
	.probe = extsw_probe,
	.priv_auto_alloc_size = sizeof(struct bcmbca_extsw_priv),
};

