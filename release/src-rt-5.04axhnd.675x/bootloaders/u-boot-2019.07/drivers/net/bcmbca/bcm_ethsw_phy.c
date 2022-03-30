#include <linux/ioport.h>

#include "mii_shared.h"
#include "asm/arch/ethsw.h"
#include "bcm_ethsw.h"

static volatile struct sw_mdio *ETHSW_MDIO = NULL;

void phy_set_mdio_base(volatile sw_mdio *mdio_base)
{
	printf("switch set mdio_base %p\n", mdio_base);
	ETHSW_MDIO = (sw_mdio *)mdio_base;
}

uint16_t bcm_ethsw_phy_read_reg(int phy_id, int reg)
{
	int reg_in, reg_out = 0;
	int i = 0;

	phy_id &= BCM_PHY_ID_M;

	reg_in = ((phy_id << ETHSW_MDIO_C22_PHY_ADDR_SHIFT)&ETHSW_MDIO_C22_PHY_ADDR_MASK) |
	((reg << ETHSW_MDIO_C22_PHY_REG_SHIFT)&ETHSW_MDIO_C22_PHY_REG_MASK) |
	(ETHSW_MDIO_CMD_C22_READ << ETHSW_MDIO_CMD_SHIFT);
	ETHSW_MDIO->mdio_cmd = reg_in | ETHSW_MDIO_BUSY;
	do {
		if (++i >= 10)  {

			return 0;
		}
		udelay(1);
		reg_out = ETHSW_MDIO->mdio_cmd;
	} while (reg_out & ETHSW_MDIO_BUSY);

	/* Read a second time to ensure it is reliable */
	ETHSW_MDIO->mdio_cmd = reg_in | ETHSW_MDIO_BUSY;
	i = 0;
	do {
		if (++i >= 10)  {
		return 0;
		}
		udelay(1);
		reg_out = ETHSW_MDIO->mdio_cmd;
	} while (reg_out & ETHSW_MDIO_BUSY);

	return (uint16_t)reg_out;
}

void bcm_ethsw_phy_write_reg(int phy_id, int reg, uint16_t data)
{
	int reg_value = 0;
	int i = 0;

	phy_id &= BCM_PHY_ID_M;

	reg_value = ((phy_id << ETHSW_MDIO_C22_PHY_ADDR_SHIFT)&ETHSW_MDIO_C22_PHY_ADDR_MASK) |
		((reg << ETHSW_MDIO_C22_PHY_REG_SHIFT)& ETHSW_MDIO_C22_PHY_REG_MASK) |
		(ETHSW_MDIO_CMD_C22_WRITE << ETHSW_MDIO_CMD_SHIFT) | (data&ETHSW_MDIO_PHY_DATA_MASK);
		ETHSW_MDIO->mdio_cmd = reg_value | ETHSW_MDIO_BUSY;

	do {
		if (++i >= 10)  {
		return;
		}
		udelay(1);
		reg_value = ETHSW_MDIO->mdio_cmd;
	} while (reg_value & ETHSW_MDIO_BUSY);

	return;
}

uint16_t bcm_ethsw_phy_read_misc_reg(int phy_id, int reg, int chn)
{
	uint16_t temp;
	bcm_ethsw_phy_write_reg(phy_id, 0x18, 0x7);
	temp = bcm_ethsw_phy_read_reg(phy_id, 0x18);
	temp |= 0x800;
	bcm_ethsw_phy_write_reg(phy_id, 0x18, temp);

	temp = (chn << 13)|reg;
	bcm_ethsw_phy_write_reg(phy_id, 0x17, temp);
	return  bcm_ethsw_phy_read_reg(phy_id, 0x15);
}

void bcm_ethsw_phy_write_misc_reg(int phy_id, int reg, int chn, uint16_t data)
{
	uint16_t temp;
	bcm_ethsw_phy_write_reg(phy_id, 0x18, 0x7);
	temp = bcm_ethsw_phy_read_reg(phy_id, 0x18);
	temp |= 0x800;
	bcm_ethsw_phy_write_reg(phy_id, 0x18, temp);

	temp = (chn << 13)|reg;
	bcm_ethsw_phy_write_reg(phy_id, 0x17, temp);
	bcm_ethsw_phy_write_reg(phy_id, 0x15, data);

	return;
}

uint16_t bcm_ethsw_phy_read_exp_reg(int phy_id, int reg)
{
	bcm_ethsw_phy_write_reg(phy_id, 0x17, reg|0xf00);
	return bcm_ethsw_phy_read_reg(phy_id, 0x15);
}
void bcm_ethsw_phy_write_exp_reg(int phy_id, int reg, uint16_t data)
{
	bcm_ethsw_phy_write_reg(phy_id, 0x17, reg|0xf00);
	bcm_ethsw_phy_write_reg(phy_id, 0x15, data);
	return;
}

/* FIXME - same code exists in robosw_reg.c */
void phy_advertise_caps(unsigned int phy_id)
{
	uint16_t cap_mask = 0;

	/* control advertising if boardparms says so */
	if (IsPhyConnected(phy_id) && IsPhyAdvCapConfigValid(phy_id)) {
		cap_mask = bcm_ethsw_phy_read_reg(phy_id, MII_ANAR);
		cap_mask &= ~(ANAR_TXFD | ANAR_TXHD | ANAR_10FD | ANAR_10HD);
		if (phy_id & ADVERTISE_10HD)
			cap_mask |= ANAR_10HD;
		if (phy_id & ADVERTISE_10FD)
			cap_mask |= ANAR_10FD;
		if (phy_id & ADVERTISE_100HD)
			cap_mask |= ANAR_TXHD;
		if (phy_id & ADVERTISE_100FD)
			cap_mask |= ANAR_TXFD;
		bcm_ethsw_phy_write_reg(phy_id, MII_ANAR, cap_mask);

		cap_mask = bcm_ethsw_phy_read_reg(phy_id, MII_K1CTL);
		cap_mask &= (~(K1CTL_1000BT_FDX | K1CTL_1000BT_HDX));
		if (phy_id & ADVERTISE_1000HD)
			cap_mask |= K1CTL_1000BT_HDX;
		if (phy_id & ADVERTISE_1000FD)
			cap_mask |= K1CTL_1000BT_FDX;
		bcm_ethsw_phy_write_reg(phy_id, MII_K1CTL, cap_mask);
	}

	/* Always enable repeater mode */
	cap_mask = bcm_ethsw_phy_read_reg(phy_id, MII_K1CTL);
	cap_mask |= K1CTL_REPEATER_DTE;
	bcm_ethsw_phy_write_reg(phy_id, MII_K1CTL, cap_mask);
}

static void sgphy_powerup(int phy_id, void *sphy_ctrl, void *phy_test_ctrl)
{
	uint32_t temp_data;
	volatile unsigned int *addr;


	addr = sphy_ctrl;

	temp_data = *addr;

	temp_data &= ~(ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_SPHY_CTRL_PHYAD_MASK);
	temp_data |= ETHSW_SPHY_CTRL_RESET_MASK|(phy_id<<ETHSW_SPHY_CTRL_PHYAD_SHIFT);
	*addr=temp_data;

	udelay(1);
#if defined(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
	temp_data &= ~(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
	*addr=temp_data;

	udelay(1000);

	temp_data = *addr;
	temp_data &= ~ETHSW_SPHY_CTRL_RESET_MASK;
	*addr = temp_data;

	udelay(1000);
}

static void qgphy_powerup(int phy_id, void *qphy_ctrl, void *phy_test_ctrl)
{

	uint32_t temp_data;
	volatile unsigned int *addr;
	addr = qphy_ctrl;

	temp_data = *addr;

	temp_data &= ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_QPHY_CTRL_PHYAD_BASE_MASK);
	temp_data |= ETHSW_QPHY_CTRL_RESET_MASK|(phy_id<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT);

#if defined(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
	temp_data &= ~(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
	*addr=temp_data;

	udelay(1000);
	temp_data = *addr;
	temp_data &= ~ETHSW_QPHY_CTRL_RESET_MASK;
	*addr=temp_data;

	udelay(1000);

}

// Note : work around only needed for 63158 / 63178 / 47622
static void qphy_init_power_workaround(int timeout, void *qphy_ctrl, void *phy_test_ctrl)
{
	unsigned int phy_ctrl;

	*((unsigned int*)qphy_ctrl) |= ETHSW_QPHY_CTRL_RESET_MASK;
	udelay(timeout);

	*((unsigned int*)phy_test_ctrl)=1;

	phy_ctrl = *((unsigned int*)qphy_ctrl);
	phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK);
#if defined(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
	phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
	*((unsigned int*)qphy_ctrl) = phy_ctrl;

	udelay(timeout);

	*((unsigned int*)qphy_ctrl) |= (ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK);
#if defined(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
	*((unsigned int*)qphy_ctrl) |= (ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
	udelay(timeout);

	phy_ctrl = *((unsigned int*)qphy_ctrl);
	phy_ctrl &= ~(ETHSW_QPHY_CTRL_RESET_MASK);
	*((unsigned int*)qphy_ctrl) = phy_ctrl;

	udelay(timeout);

	*((unsigned int*)phy_test_ctrl)=0;
}

static void sphy_init_power_workaround(int timeout, void *sphy_ctrl, void *phy_test_ctrl)
{

	unsigned int phy_ctrl;


	*((unsigned int*)sphy_ctrl) |= ETHSW_SPHY_CTRL_RESET_MASK;
	udelay(timeout);

	*((unsigned int*)phy_test_ctrl)=1;


	phy_ctrl = *((unsigned int*)sphy_ctrl);
	phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK);
#if defined(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
	phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
	*((unsigned int*)sphy_ctrl) = phy_ctrl;
	udelay(timeout);

	*((unsigned int*)sphy_ctrl) |= (ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK);
#if defined(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
	*((unsigned int*)sphy_ctrl) |= (ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
	udelay(timeout);

	phy_ctrl = *((unsigned int*)sphy_ctrl);
	phy_ctrl &= ~(ETHSW_SPHY_CTRL_RESET_MASK);
	*((unsigned int*)sphy_ctrl) = phy_ctrl;
	udelay(timeout);

	*((unsigned int*)phy_test_ctrl)=0;
}

extern void phy_adjust_afe(unsigned int phy_id_base, int is_quad);

static void qphy_fixup(void *qphy_ctrl, void *phy_test_ctrl)
{
	int phy_id;
	/* Internal QUAD PHY and Single PHY require some addition fixup on the PHY AFE */
	phy_id = (*((unsigned int*)qphy_ctrl)&ETHSW_QPHY_CTRL_PHYAD_BASE_MASK)>>ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT;
	phy_adjust_afe(phy_id, 1);
} 

static void sphy_fixup(void *sphy_ctrl, void *phy_test_ctrl)
{
	int phy_id;
	phy_id = (*((unsigned int*)sphy_ctrl)&ETHSW_SPHY_CTRL_PHYAD_MASK)>>ETHSW_SPHY_CTRL_PHYAD_SHIFT;
	phy_adjust_afe(phy_id, 0);
}

extern uint16_t bcm_ethsw_phy_read_reg(int phy_id, int reg);

void gphy_powerup(int phy_base, u32 wkard_timeout, void *sphy_ctrl, void *qphy_ctrl, void *phy_test_ctrl)
{
	int phy_id = phy_base;

	if(qphy_ctrl != NULL) {
		if (wkard_timeout != 0) {
			qphy_init_power_workaround(wkard_timeout, qphy_ctrl, phy_test_ctrl);
		}
		qgphy_powerup(phy_id, qphy_ctrl, phy_test_ctrl);
		phy_id += 4;
	}

	if(sphy_ctrl != NULL) {
		if (wkard_timeout != 0) {
			sphy_init_power_workaround(wkard_timeout, sphy_ctrl, phy_test_ctrl);
		}
		sgphy_powerup(phy_id, sphy_ctrl, phy_test_ctrl);
	}

	/* add dummy read to workaround first MDIO read/write issue after power on */
	bcm_ethsw_phy_read_reg(phy_base, 0x2);

	if (qphy_ctrl != NULL) {
		qphy_fixup(qphy_ctrl, phy_test_ctrl);
	}
	if (sphy_ctrl != NULL) {
		sphy_fixup(sphy_ctrl, phy_test_ctrl);
	}
}




