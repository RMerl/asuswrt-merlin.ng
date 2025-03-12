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

typedef struct {
	unsigned short addr;
	unsigned short dev;
	unsigned short reg;
	unsigned short val;
} mdio_data;

static inline unsigned long int safe_strtoul(const char *str, char **endptr, int base)
{
	errno = 0;
	return strtoul(str, endptr, base);
}

#define STRTOUL_CHECK(v, s, ep) { \
	(v) = safe_strtoul((s), &(ep), 0); \
	if (errno || *(ep)) \
		goto print_error_and_return; \
}

#define VALUE_CHECK(s, v, lo, hi) { \
	if (v < lo || v > hi) { \
		fprintf(stderr, "Invalid " s " value %d; range: %d to %d\n", v, lo, hi); \
		goto print_error_and_return; \
	} \
}

static int esw_fd;

int switch_ioctl_init(void)
{
	esw_fd = open("/dev/mxlswitch", O_RDONLY);
	if (esw_fd == -1) {
		perror("open");
		return -EINVAL;
	}

	return 0;
}

void switch_ioctl_fin(void)
{
	close(esw_fd);
}

int sys_cl22_mdio_read(uint8_t phyaddr, uint16_t reg, uint16_t *value)
{
	mdio_data data;
	int cmd = 4;

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.reg = reg;

		if (ioctl(esw_fd, cmd, &data) < 0) {
			perror("mxlswitch ioctl");
		} else {
			*value = data.val;
		}
	}

	return 0;
}

int sys_cl22_mdio_write(uint8_t phyaddr, uint16_t reg, uint16_t value)
{
	mdio_data data;
	int cmd = 3;

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.reg = reg;
		data.val = value;

		if (ioctl(esw_fd, cmd, &data) < 0) {
			perror("mxlswitch ioctl");
		}
	}

	return 0;
}

int sys_cl45_mdio_read(uint8_t phyaddr, uint8_t mmd, uint16_t reg, uint16_t *value)
{
	mdio_data data;
	int cmd = 0;

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.dev = mmd;
		data.reg = reg;

		if (ioctl(esw_fd, cmd, &data) < 0) {
			perror("mxlswitch ioctl");
		} else {
			*value = data.val;
		}
	}

	return 0;
}

int sys_cl45_mdio_write(uint8_t phyaddr, uint8_t mmd, uint16_t reg, uint16_t value)
{
	mdio_data data;
	int cmd = 1;

	if (esw_fd != -1) {
		memset(&data, 0, sizeof(data));
		data.addr = phyaddr;
		data.dev = mmd;
		data.reg = reg;
		data.val = value;

		if (ioctl(esw_fd, cmd, &data) < 0) {
			perror("mxlswitch ioctl");
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	unsigned int addr = 0;
	unsigned int set = 0, dev = 0; 
	unsigned short reg = 0, val = 0;
	char *endp;
	int count = 0;
	int cl22 = 0;

	if (strstr(*argv, "cl22"))
		cl22 = 1;

	argv = argv+1;

	/* parse addr */
	if (*argv) {
		STRTOUL_CHECK(addr, *argv, endp);
		argv++;
		count++;
	}

	/* parse dev */
	if (!cl22 && *argv) {
		STRTOUL_CHECK(dev, *argv, endp);
		argv++;
		count++;
	}

	/* parse register */
	if (*argv) {
		STRTOUL_CHECK(reg, *argv, endp);
		argv++;
		count++;
	}

	/* parse value */
	if (*argv) {
		STRTOUL_CHECK(val, *argv, endp);
		set = 1;
		argv++;
		count++;
	}

	if (cl22)
	{
		if (count < 2) goto print_error_and_return;
	}
	else
	{
		if (count < 3) goto print_error_and_return;
	}

	switch_ioctl_init();
	if (set)
	{
		if (cl22)
			sys_cl22_mdio_write(addr, reg, val);
		else
			sys_cl45_mdio_write(addr, dev, reg, val);
	}
	else
	{
		if (cl22)
			sys_cl22_mdio_read(addr, reg, &val);
		else
			sys_cl45_mdio_read(addr, dev, reg, &val);
		printf("value: %x\n", val);
	}
	switch_ioctl_fin();

	return 0;

print_error_and_return:
	if (cl22)
		fprintf(stderr, "syntax: cl22 addr reg <value>\n");
	else
		fprintf(stderr, "syntax: cl45 addr dev reg <value>\n");

	return 0;
}
