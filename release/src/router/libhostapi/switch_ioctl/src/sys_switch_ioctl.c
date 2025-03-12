/*
 * sys_switch_ioctl.c: switch(ioctl) set API
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "sys_switch_ioctl.h"

int sys_cl22_mdio_read(uint8_t phyaddr, uint16_t reg, uint16_t *value)
{
	mdio_data data;
        int esw_fd = open("/dev/mxlswitch", O_RDONLY);

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.reg = reg;

		if (ioctl(esw_fd, 4, &data) < 0) {
			perror("mxlswitch ioctl");
		} else {
			*value = data.val;
		}

		close(esw_fd);
	}

	return (esw_fd != -1) ? : 0;
}

int sys_cl22_mdio_write(uint8_t phyaddr, uint16_t reg, uint16_t value)
{
	mdio_data data;
	int esw_fd = open("/dev/mxlswitch", O_RDONLY);

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.reg = reg;
		data.val = value;

		if (ioctl(esw_fd, 3, &data) < 0) {
			perror("mxlswitch ioctl");
		}

		close(esw_fd);
	}

	return 0;
}

int sys_cl45_mdio_read(uint8_t phyaddr, uint8_t mmd, uint16_t reg, uint16_t *value)
{
	mdio_data data;
	int esw_fd = open("/dev/mxlswitch", O_RDONLY);

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.dev = mmd;
		data.reg = reg;

		if (ioctl(esw_fd, 0, &data) < 0) {
			perror("mxlswitch ioctl");
		} else {
			*value = data.val;
		}

		close(esw_fd);
	}

	return (esw_fd != -1) ? : 0;
}

int sys_cl45_mdio_write(uint8_t phyaddr, uint8_t mmd, uint16_t reg, uint16_t value)
{
	mdio_data data;
	int esw_fd = open("/dev/mxlswitch", O_RDONLY);

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.dev = mmd;
		data.reg = reg;
		data.val = value;

		if (ioctl(esw_fd, 1, &data) < 0) {
			perror("mxlswitch ioctl");
		}

		close(esw_fd);
	}

	return 0;
}
