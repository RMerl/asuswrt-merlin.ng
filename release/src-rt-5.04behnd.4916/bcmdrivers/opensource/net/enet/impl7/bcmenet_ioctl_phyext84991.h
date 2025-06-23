#ifndef BCMENET_IOCTL_PHYEXT84991_H
#define BCMENET_IOCTL_PHYEXT84991_H

extern int32_t mdio_read_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val);
extern int32_t mdio_write_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val);

int phyext84991_power_get(uint32_t addr, int *enable);
int phyext84991_power_set(uint32_t addr, int enable);
int phyext84991_phy_reset(uint32_t addr);
int phyext84991_mediatype(struct ethctl_data *ethctl);
int phyext84991_phy_eee_get(uint32_t phy_addr, int *enable);
int phyext84991_phy_eee_set(uint32_t phy_addr, int enable);
int phyext84991_phy_eee_autogreeen_read(uint32_t phy_addr, int *autogreeen);
int phyext84991_phy_eee_mode_set(uint32_t phy_addr, int autogreeen);

#endif /* BCMENET_IOCTL_PHYEXT84991_H */