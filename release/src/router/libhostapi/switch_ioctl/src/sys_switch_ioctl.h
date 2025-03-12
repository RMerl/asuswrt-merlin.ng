/*
 * sys_switch_ioctl.h: switch(ioctl) set API
 */

#ifndef SYS_SWITCH_IOCTL_H
#define SYS_SWITCH_IOCTL_H

typedef struct {
	unsigned short addr;
	unsigned short dev;
	unsigned short reg;
	unsigned short val;
} mdio_data;

int sys_cl22_mdio_read(uint8_t phyaddr, uint16_t reg, uint16_t *value);
int sys_cl22_mdio_write(uint8_t phyaddr, uint16_t reg, uint16_t value);
int sys_cl45_mdio_read(uint8_t phyaddr, uint8_t mmd, uint16_t reg, uint16_t *value);
int sys_cl45_mdio_write(uint8_t phyaddr, uint8_t mmd, uint16_t reg, uint16_t value);

#endif /* SYS_SWITCH_IOCTL_H */
