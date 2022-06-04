/*
 * flash_otp_write.c -- write One-Time-Program data
 */

#define PROGRAM_NAME "flash_otp_write"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <common.h>
#include <mtd/mtd-user.h>

static ssize_t xread(int fd, void *buf, size_t count)
{
	ssize_t ret, done = 0;

retry:
	ret = read(fd, buf + done, count - done);
	if (ret < 0)
		return ret;

	done += ret;

	if (ret == 0 /* EOF */ || done == count)
		return done;
	else
		goto retry;
}

int main(int argc,char *argv[])
{
	int fd, val, ret, size, wrote, len;
	mtd_info_t mtdInfo;
	off_t offset;
	char *p, buf[2048];

	if (argc != 4 || strcmp(argv[1], "-u")) {
		fprintf(stderr, "Usage: %s -u <device> <offset>\n", PROGRAM_NAME);
		fprintf(stderr, "the raw data to write should be provided on stdin\n");
		fprintf(stderr, "CAUTION! ONCE SET TO 0, OTP DATA BITS CAN'T BE ERASED!\n");
		return EINVAL;
	}

	fd = open(argv[2], O_WRONLY);
	if (fd < 0) {
		perror(argv[2]);
		return errno;
	}

	val = MTD_OTP_USER;
	ret = ioctl(fd, OTPSELECT, &val);
	if (ret < 0) {
		perror("OTPSELECT");
		return errno;
	}

	if (ioctl(fd, MEMGETINFO, &mtdInfo)) {
		perror("MEMGETINFO");
		return errno;
	}

	offset = (off_t)strtoull(argv[3], &p, 0);
	if (argv[3][0] == 0 || *p != 0) {
		fprintf(stderr, "%s: bad offset value\n", PROGRAM_NAME);
		return ERANGE;
	}

	if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
		perror("lseek()");
		return errno;
	}

	printf("Writing OTP user data on %s at offset 0x%llx\n", argv[2], (unsigned long long)offset);

	if (mtd_type_is_nand_user(&mtdInfo))
		len = mtdInfo.writesize;
	else
		len = 256;

	if (len > sizeof(buf)) {
		printf("huh, writesize (%d) bigger than buffer (%zu)\n",
				len, sizeof(buf));
		return ENOMEM;
	}

	wrote = 0;
	while ((size = xread(0, buf, len))) {
		if (size < 0) {
			perror("read()");
			return errno;
		}
		p = buf;
		while (size > 0) {
			if (mtd_type_is_nand_user(&mtdInfo)) {
				/* Fill remain buffers with 0xff */
				memset(buf + size, 0xff, mtdInfo.writesize - size);
				size = mtdInfo.writesize;
			}
			ret = write(fd, p, size);
			if (ret < 0) {
				perror("write()");
				return errno;
			}
			if (ret == 0) {
				printf("write() returned 0 after writing %d bytes\n", wrote);
				return 0;
			}
			p += ret;
			wrote += ret;
			size -= ret;
		}
	}

	printf("Wrote %d bytes of OTP user data\n", wrote);
	return 0;
}
