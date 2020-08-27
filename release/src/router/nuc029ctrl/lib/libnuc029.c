#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <syslog.h>

#include <libnuc029.h>

static int open_bus(int bus)
{
	int fd;
	char busName[64];

	snprintf(busName, 64, "%s%d", I2C_NODE, bus);
	if ((fd = open(busName, O_RDWR)) < 0)
		perror("Failed to open the i2c bus");

	return fd;
}

void print_buf(unsigned char *buf, int size)
{
	char str[BUFSIZE * 3] = {0};
	char *pstr = str;
	int i, n;

	for (i = 0; i < size; i++) {
		n = sprintf(pstr, "%02x ", buf[i]);
		pstr += n;
	}
	printf("%s\n", str);
}

static inline void udelay(volatile int us)
{
	volatile int i;
	while(us--)
		for(i = 30; i > 0; i--);
}

static inline void mdelay(volatile int ms)
{
	while(ms--) {
		udelay(700);
	}
}

int read_register(unsigned char *reg, int bytes, unsigned char *data)
{
	int ret = -1;
	int busfd;
	struct i2c_rdwr_ioctl_data i2cData;

	/* open i2c bus */
	if ( (busfd = open_bus(0)) < 0) {
		goto err;
	}

	ioctl(busfd,I2C_TIMEOUT, 1); /* set timeout */
	ioctl(busfd,I2C_RETRIES, 2); /* set retry */

	i2cData.nmsgs= 2;
	i2cData.msgs = (struct i2c_msg *) malloc (i2cData.nmsgs * sizeof(struct i2c_msg));
	if (i2cData.msgs == NULL) {
		perror("malloc error");
	}

	/* read data from register addr store into buf. */
	i2cData.msgs[0].len= 1;
	i2cData.msgs[0].addr= I2C_DEVICE;
	i2cData.msgs[0].flags= 0;     
	i2cData.msgs[0].buf= reg;
	i2cData.msgs[1].len= bytes;
	i2cData.msgs[1].addr= I2C_DEVICE;
	i2cData.msgs[1].flags= 1;     /* read */
	i2cData.msgs[1].buf= data;

	ret = ioctl(busfd, I2C_RDWR, (unsigned long)& i2cData);
	if (ret < 0) {
		perror("read error");
		goto err;
	}

err:
	free(i2cData.msgs);
	if (busfd >= 0) {
		close(busfd);
	}

	return ret;
}

int read_register2str(unsigned char *reg, int bytes, unsigned char *data, int bufsize)
{
	int ret = -1;
	int i, n;
	int busfd;
	char str[BUFSIZE*3] = {0};
	char *pstr = str;
	unsigned char buf[BUFSIZE] = {0};
	struct i2c_rdwr_ioctl_data i2cData;

	/* open i2c bus */
	if ( (busfd = open_bus(0)) < 0) {
		goto err;
	}

	ioctl(busfd,I2C_TIMEOUT, 1); /* set timeout */
	ioctl(busfd,I2C_RETRIES, 2); /* set retry */

	i2cData.nmsgs= 2;
	i2cData.msgs = (struct i2c_msg *) malloc (i2cData.nmsgs * sizeof(struct i2c_msg));
	if (i2cData.msgs == NULL) {
		perror("malloc error");
	}
	/* read data from register addr store into buf. */
	i2cData.msgs[0].len= 1;
	i2cData.msgs[0].addr= I2C_DEVICE;
	i2cData.msgs[0].flags= 0;     
	i2cData.msgs[0].buf= reg;
	i2cData.msgs[1].len= bytes;
	i2cData.msgs[1].addr= I2C_DEVICE;
	i2cData.msgs[1].flags= 1;     /* read */
	i2cData.msgs[1].buf= buf;

	ret = ioctl(busfd, I2C_RDWR, (unsigned long)& i2cData);
	if (ret < 0) {
		perror("read error");
		goto err;
	}

	for (i = 0; i < bytes; i++) {
		n = sprintf(pstr, "%02x", buf[i]);
		pstr += n;
	}
	strncpy(data, str, bufsize);

err:
	free(i2cData.msgs);
	if (busfd >= 0) {
		close(busfd);
	}

	return ret;
}

int write_register(unsigned char *data, int datasize)
{
	int ret = -1;
	int busfd;
	struct i2c_rdwr_ioctl_data i2cData;

	/* open i2c bus */
	if ( (busfd = open_bus(0)) < 0) {
		goto err;
	}

	ioctl(busfd,I2C_TIMEOUT, 1); /* set timeout */
	ioctl(busfd,I2C_RETRIES, 2); /* set retry */

	i2cData.nmsgs= 1;
	i2cData.msgs = (struct i2c_msg *) malloc (i2cData.nmsgs * sizeof(struct i2c_msg));
	if(i2cData.msgs == NULL) {
		perror("malloc error");
	}

	/* write data into register addr */
	i2cData.msgs[0].len= datasize;
	i2cData.msgs[0].addr= I2C_DEVICE;
	i2cData.msgs[0].flags= 0;     /* write */
	i2cData.msgs[0].buf = data;   /* write reg+data to reg address */

	ret = ioctl(busfd, I2C_RDWR, (unsigned long)& i2cData);

	if (ret < 0) {
		perror("write data error");
		goto err;
	}

	if (datasize <= 2) {
		MyDBG("### Write [0x%02x] to addr:[%#x]\n", data[1], data[0]);
	} else {
		MyDBG("### Write [0x%02x 0x%02x] to addr:[%#x]\n", data[1], data[2], data[0]);
	}

err:
	free(i2cData.msgs);
	if (busfd >= 0) {
		close(busfd);
	}

	return ret;
}

int GetMcuVer(unsigned char *data, size_t size)
{
	unsigned char reg = REG_FW_VER;
	return read_register2str(&reg, 4, data, size);
}

int SetVolumeLED(int vol)
{
	unsigned char data[3] = {REG_LED, REG_LED_VOLUME, vol};
	return write_register(data, sizeof(data));
}

int SetMuteLED(int val)
{
	unsigned char data[2] = {REG_LED_MUTE, val};
	return write_register(data, sizeof(data));
}

int SetLED(int val)
{
	unsigned char data[2] = {REG_LED, val};
	return write_register(data, sizeof(data));
}

int SetRGB_LED(int R, int G, int B, int LED, int LED2)
{
	unsigned char data[7] = {REG_LED, REG_LED_RGB, R, G, B, LED, LED2};
	return write_register(data, sizeof(data));
}

static int Switch2LDROM(void)
{
	int ret = -1;
	unsigned char val;
	unsigned char reset[2] = {REG_RESET_LDROM, 0x00};
	unsigned char reg = REG_LDROM_CHECK;

	write_register(reset, sizeof(reset));
	ret = read_register(&reg, 1, &val);
	if (val) {
		printf("LDROM CHECK:[OK]\n");
		return ret;
	}
	printf("LDROM CHECK:[NOK]\n");
	return -1;
}

int UpgradeAPROM(const char *name)
{
	int cnt = 0;
	int ret = -1;
	FILE *fp;
	unsigned char boot[2] = {REG_LDROM_BOOT_APROM, 0xA5};

	if ((fp = fopen(name, "rb")) == NULL) {
		printf("open file:[%s] error\n", name);
		return ret;
	}

	if ( (ret = Switch2LDROM()) > 0) {
		printf("Upgrading...");
		while (!feof(fp)) {
			int i;
			unsigned char data[33];
			memset(data, 0, sizeof(data));
			fread(data, sizeof(unsigned char), 32, fp);

			for (i = 33; i > 0; i--) {
				data[i] = data[i-1];
			}
			data[0] = REG_LDROM_FW;

			cnt++;
			if (cnt <= 2048) {
				ret = write_register(data, sizeof(data));
			}
			mdelay(1);
		}
		mdelay(1);
		ret = write_register(boot, sizeof(boot));
	}

	fclose(fp);
	return ret;
}

