
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <sys/ioctl.h>
#include <lantiq.h>
#include <iwlib.h>
#include "utils.h"
#include "shutils.h"
#include <shared.h>
#include <trxhdr.h>
#include <bcmutils.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

uint32_t get_fullimage(char *imagefile, long offset, long size)
{
	FILE *fp;
	FILE *fp_fullimage;
	char buf[1024000];
	int nr;
	int ret = 0;
	long read_already;
	long bytes_to_read;

	fp_fullimage = fopen("/tmp/fullimage.img", "wb");
	if (fp_fullimage == NULL) return 0;

	if ((fp = fopen(imagefile, "r")) != NULL) {
		fseek (fp, offset, SEEK_SET );
		read_already = 0;
		bytes_to_read = sizeof(buf);
		while ((nr = fread(buf, 1, bytes_to_read , fp)) > 0){
			fwrite(buf, 1, nr, fp_fullimage);
			read_already = read_already + nr;
			if(read_already+ sizeof(buf) < size)
				bytes_to_read = sizeof(buf);
			else
				bytes_to_read = size - read_already;
		}
	}else{
		fclose(fp_fullimage);
		return 0;
	}
	return 1;
}

int update_trx(char *imagefile)
{
	int ifd = -1;
	int ret = 1;
	struct stat sbuf;
	unsigned char *ptr = NULL;
	struct trx_header *trx;
	long fullimage_offset, fullimage_size;

	ifd = open(imagefile, O_RDONLY);
	if (ifd < 0) {
		_dprintf("Can't open %s: %d\n", imagefile, strerror(errno));
		ret = 0;
		goto update_trx_fail;
	}

	(void)fdatasync(ifd);

	if (fstat(ifd, &sbuf) < 0) {
		_dprintf("Can't stat %s: %d\n", imagefile, strerror(errno));
		ret = 0;
		goto update_trx_fail;
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
				    PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		_dprintf("Can't map %s: %s\n", imagefile, strerror(errno));
		ret = 0;
		goto update_trx_fail;
	}

	trx = (struct trx_header *) ptr;

	fullimage_offset = __bswap32(trx->offsets[0]);
	fullimage_size = __bswap32(trx->offsets[1] - trx->offsets[0]);
#if 0
	_dprintf("fullimage_offset:[%08X][%d]\n", fullimage_offset, fullimage_offset);
	_dprintf("fullimage_size:[%08X][%d]\n", fullimage_size, fullimage_size);
#endif
	system("rm -rf /tmp/wireless/lantiq");
	get_fullimage(imagefile, fullimage_offset, fullimage_size);

	system("rm -f /tmp/linux.trx");
	system("echo 1 > /proc/sys/vm/drop_caches");
	system("/usr/sbin/upgrade /tmp/fullimage.img fullimage 0 1");

update_trx_fail:
	close(ifd);
	return ret;
}


uint32_t trx_crc(char *imagefile)
{
	FILE *fp;
	char buf[1024000];
	int nr;
	uint32_t checksum;

	checksum = 0xffffffff;
	if ((fp = fopen(imagefile, "r")) != NULL) {
		fseek (fp, 12, SEEK_SET );
		while ((nr = fread(buf, 1, sizeof(buf), fp)) > 0){
			// _dprintf("checkcrc(), read:[%d] bytes\n", nr);
			checksum = crc_calc(checksum ,
				buf, nr<1024000?nr:1024000);
		}
		fclose(fp);
	}else{
		return 0;
	}
	// _dprintf("final crc:[%08X]\n", checksum);
	return checksum;
	
}

int checkcrc(char *imagefile)
{
	int ifd = -1;
	int ret = 1;
	struct stat sbuf;
	unsigned char *ptr = NULL;
	uint32_t checksum;
	int len = 0;
	struct trx_header *trx;

	checksum  = trx_crc(imagefile);

	if(checksum == 0){
		_dprintf("Can't calculated TRX crc\n");
		return 0;
	}

	ifd = open(imagefile, O_RDONLY);
	if (ifd < 0) {
		_dprintf("Can't open %s: %d\n", imagefile, strerror(errno));
		ret = 0;
		goto checkcrc_end;
	}

	(void)fdatasync(ifd);

	if (fstat(ifd, &sbuf) < 0) {
		_dprintf("Can't stat %s: %d\n", imagefile, strerror(errno));
		ret = 0;
		goto checkcrc_fail;
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
				    PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		_dprintf("Can't map %s: %s\n", imagefile, strerror(errno));
		ret = 0;
		goto checkcrc_fail;
	}

	trx = (struct trx_header *) ptr;
	len = trx->len;

	if(trx->crc32 != __bswap_32(checksum)) ret = 0 ;

checkcrc_fail:
	if (ptr != NULL)
		munmap(ptr, sbuf.st_size);
	(void)fdatasync(ifd);

checkcrc_end:
	if (close(ifd)) {
		_dprintf("Read error on %s: %s\n", imagefile, strerror(errno));
		ret = -1;
	}
	return ret;
}

