#ifndef __SECO_COMMON_MX6_H
#define __SECO_COMMON_MX6_H

void seco_mx6_setup_uart_iomux(void);
void seco_mx6_setup_enet_iomux(void);
int seco_mx6_rgmii_rework(struct phy_device *phydev);
void seco_mx6_setup_usdhc_iomux(int id);

#endif /* __SECO_COMMON_MX6_H */
