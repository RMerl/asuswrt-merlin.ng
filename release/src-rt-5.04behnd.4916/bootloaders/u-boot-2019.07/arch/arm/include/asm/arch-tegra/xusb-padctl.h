#ifndef _TEGRA_XUSB_PADCTL_H_
#define _TEGRA_XUSB_PADCTL_H_

struct tegra_xusb_phy;

/**
 * tegra_xusb_phy_get() - obtain a reference to a specified padctl PHY
 * @type: the type of PHY to obtain
 *
 * The type of PHY varies between SoC generations. Typically there are XUSB,
 * PCIe and SATA PHYs, though not all generations support all of them. The
 * value of type can usually be directly parsed from a device tree.
 *
 * Return: a pointer to the PHY or NULL if no such PHY exists
 */
struct tegra_xusb_phy *tegra_xusb_phy_get(unsigned int type);

void tegra_xusb_padctl_init(void);
int tegra_xusb_phy_prepare(struct tegra_xusb_phy *phy);
int tegra_xusb_phy_enable(struct tegra_xusb_phy *phy);
int tegra_xusb_phy_disable(struct tegra_xusb_phy *phy);
int tegra_xusb_phy_unprepare(struct tegra_xusb_phy *phy);

#endif
