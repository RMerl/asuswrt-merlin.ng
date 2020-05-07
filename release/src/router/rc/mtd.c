/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/
/*

	Modified for Tomato Firmware
	Portions, Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"

#include <limits.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#ifdef LINUX26
#ifndef HND_ROUTER
#include <linux/compiler.h>
#endif
#include <mtd/mtd-user.h>
#else
#include <linux/mtd/mtd.h>
#endif
#include <stdint.h>
#ifdef RTCONFIG_REALTEK
#include <linux/fs.h>
#endif
#include <arpa/inet.h>

#include <trxhdr.h>
#include <bcmutils.h>

#ifdef RTCONFIG_BCMARM
#include <bcmendian.h>
#include <bcmnvram.h>
#include <shutils.h>
#endif
//	#define DEBUG_SIMULATE

#ifdef RTCONFIG_REALTEK
#include "../shared/sysdeps/realtek/realtek.h"
#endif

struct code_header {
	char magic[4];
	char res1[4];
	char fwdate[3];
	char fwvern[3];
	char id[4];
	char hw_ver;
	char res2;
	unsigned short flags;
	unsigned char res3[10];
} ;

// -----------------------------------------------------------------------------

#ifdef RTCONFIG_BCMARM
static int mtd_open_old(const char *mtdname, mtd_info_t *mi)
#else
static int mtd_open(const char *mtdname, mtd_info_t *mi)
#endif
{
	char path[256];
	int part;
	int size;
	int f;

	if (mtd_getinfo(mtdname, &part, &size)) {
		sprintf(path, MTD_DEV(%d), part);
		if ((f = open(path, O_RDWR|O_SYNC)) >= 0) {
			if ((mi) && ioctl(f, MEMGETINFO, mi) != 0) {
				close(f);
				return -1;
			}
			return f;
		}
	}
	return -1;
}

/* int _unlock_erase(const char *mtdname, int erase)
 *
 * mtdname: name of mtd partition want to do
 * erase  : 0      to Unlock
 *          others to Erase
 *
 * return:
 * 	 1: timeout
 * 	 0: success
 * 	-1: fail
 */

static int _unlock_erase(const char *mtdname, int erase)
{
	int mf;
	mtd_info_t mi;
	erase_info_t ei;
	int r, ret, skipbb;

	if (!wait_action_idle(10)){
		return 1;	//timeout
	}
	set_action(ACT_ERASE_NVRAM);

	r = 0;
	skipbb = 0;

#ifdef RTCONFIG_BCMARM
	if ((mf = mtd_open_old(mtdname, &mi)) >= 0) {
#else
	if ((mf = mtd_open(mtdname, &mi)) >= 0) {
#endif
			r = 1;
#if 1
			ei.length = mi.erasesize;
			for (ei.start = 0; ei.start < mi.size; ei.start += mi.erasesize) {
				printf("%sing 0x%x - 0x%x\n", erase ? "Eras" : "Unlock", ei.start, (ei.start + ei.length) - 1);
				fflush(stdout);

				if (!skipbb) {
					loff_t offset = ei.start;

					if ((ret = ioctl(mf, MEMGETBADBLOCK, &offset)) > 0) {
						printf("Skipping bad block at 0x%08x\n", ei.start);
						continue;
					} else if (ret < 0) {
						if (errno == EOPNOTSUPP) {
							skipbb = 1;	// Not supported by this device
						} else {
							perror("MEMGETBADBLOCK");
							r = 0;
							break;
						}
					}
				}
				if (ioctl(mf, MEMUNLOCK, &ei) != 0) {
//					perror("MEMUNLOCK");
//					r = 0;
//					break;
				}
				if (erase) {
					if (ioctl(mf, MEMERASE, &ei) != 0) {
						perror("MEMERASE");
#if !defined(RTCONFIG_MTK_NAND)
						r = 0;
						break;
#endif
					}
				}
			}
#else
			ei.start = 0;
			ei.length = mi.size;

			printf("%sing 0x%x - 0x%x\n", erase ? "Eras" : "Unlock", ei.start, ei.length - 1);
			fflush(stdout);

			if (ioctl(mf, MEMUNLOCK, &ei) != 0) {
				perror("MEMUNLOCK");
				r = 0;
			}
			else if (erase) {
				if (ioctl(mf, MEMERASE, &ei) != 0) {
					perror("MEMERASE");
					r = 0;
				}
			}
#endif

			// checkme:
			char buf[2];
			read(mf, &buf, sizeof(buf));
			close(mf);
	}

	set_action(ACT_IDLE);

	if (r) printf("\"%s\" successfully %s.\n", mtdname, erase ? "erased" : "unlocked");
	else printf("\nError %sing MTD\n", erase ? "eras" : "unlock");

	sleep(1);
	if (r)
		return 0;	//success
	return -1;		//erase fail
}

int mtd_unlock(const char *mtdname)
{
	return _unlock_erase(mtdname, 0);
}

#ifdef RTCONFIG_BCMARM
int mtd_erase_old(const char *mtdname)
#else
int mtd_erase(const char *mtdname)
#endif
{
	return !_unlock_erase(mtdname, 1);
}

#ifdef RTCONFIG_BCMARM
int mtd_unlock_erase_main_old(int argc, char *argv[])
#else
int mtd_unlock_erase_main(int argc, char *argv[])
#endif
{
	int c;
	char *dev = NULL;

#ifdef RTCONFIG_LANTIQ
	nvram_set(ASUS_STOP_COMMIT, "1");
#endif
	while ((c = getopt(argc, argv, "d:")) != -1) {
		switch (c) {
		case 'd':
			dev = optarg;
			break;
		}
	}

	if (!dev) {
		usage_exit(argv[0], "-d part");
	}

	return _unlock_erase(dev, strstr(argv[0], "erase") ? 1 : 0);
}

#ifdef RTCONFIG_REALTEK
int asusimg2rtkimg(char *src_name)
{
		FILE *pSrcFile = NULL, *pDstFile = NULL;
		unsigned char buf[4096] = {0};
		long filesize, kernelsize;
		int first_read_kernel = 1, first_read_root = 1, last_read_kernel = 0, count;

		pSrcFile = fopen(src_name, "r");
		pDstFile = fopen("/tmp/linux.trx.tmp", "w");
		if (pSrcFile && pDstFile) {
			/* Get image size */
			fseek(pSrcFile, 0, SEEK_END);
			filesize = ftell(pSrcFile);

			/* Skip ASUS trx header */
			fseek(pSrcFile, 64, SEEK_SET);
			filesize -= 64;

			do {
				if (last_read_kernel) {
					count = fread(buf, 1, kernelsize, pSrcFile);
					last_read_kernel = 0;
				}
				else
					count = fread(buf, 1, sizeof(buf), pSrcFile);

				if (first_read_kernel) {
					/* Set Start address+3 byte first bit is 1 */
					*(buf+7) |= 1;

					/* Get kernel size */
					kernelsize = (*(buf+12) << 24) + (*(buf+13) << 16) + (*(buf+14) << 8) + *(buf+15);
					first_read_kernel = 0;
					kernelsize += 16; // Need to add kernel signature size.
				}
				if (kernelsize > 0) { // Write Kernel + Kernrl signature
					fwrite(buf, 1, count, pDstFile);
					kernelsize -= count;

					if (kernelsize > 0 && kernelsize <= sizeof(buf))
						last_read_kernel = 1;
				}
				else { // Write Rootfs
					if (first_read_root) {
						fwrite(buf+16, 1, count-16, pDstFile);
						first_read_root = 0;
					}
					else
						fwrite(buf, 1, count, pDstFile);
				}

				filesize -= count;

			} while(filesize > 0);

			fclose(pSrcFile);
			fclose(pDstFile);

			if (remove(src_name) == 0) { // Delete asus trx image
				if (rename("/tmp/linux.trx.tmp", "/tmp/linux.trx") == 0) // rename rtk trx image
					return 1;
				else {
					_dprintf("rename failed\n");
					return 0;
				}
			}
			else {
				_dprintf("Delete linux.trx failed\n");
				return 0;
			}
		}
		else {
			_dprintf("Open file failed\n");
			return 0;
		}

	return 0;
}
int copy_file2file(char * src_name,long src_offset, char *dst_name,long dst_offset,char* errorInfo)
{
	int fsrc=-1,fdst=-1;
	int retval=0,readSize=0,writeSize=0,tobeWriteSize=0;
	unsigned char buff[(4096 * 1024)]={0};
	int i=0;
	long readoffset=src_offset,writeoffset=dst_offset;
	if(!src_name || !dst_name || !errorInfo)
	{
		strcpy(errorInfo,"invalid input!");
		retval= -1;
		goto FAIL;
	}

	fsrc= open(src_name, O_RDONLY);
	if(fsrc<0){ sprintf(errorInfo,"open %s fail!",src_name); retval= -1;goto FAIL;}
	fdst= open(dst_name, O_WRONLY);
	if(fdst<0){ sprintf(errorInfo,"open %s fail!",dst_name); retval= -1;goto FAIL;}
	lseek(fsrc,src_offset,SEEK_SET);
	lseek(fdst,dst_offset,SEEK_SET);

	do
	{
		bzero(buff,sizeof(buff));
		readSize=read(fsrc,buff,sizeof(buff));
		readoffset+=readSize;
		tobeWriteSize=readSize;

		writeSize=write(fdst,buff,tobeWriteSize);
		writeoffset+=writeSize;

		sync();

//#ifdef KERNEL_3_10
		if(ioctl(fdst,BLKFLSBUF,NULL) < 0){
			_dprintf("flush mtd system cache error\n");
		}
//#endif
		//_dprintf("%s %d readoffset=%x readSize=%x writeoffset=%x writeSize=%x src_name=%s dst_name=%s\n", __FUNCTION__,__LINE__,readoffset,readSize,writeoffset,writeSize,src_name,dst_name);
		/*if(writeSize!=sizeof(buff))
		{
			 sprintf(errorInfo,"write %s fail!",dst_name); retval= -1;goto FAIL;
		}*/
	}while(readSize==sizeof(buff));

	FAIL:
		if(fsrc>0) close(fsrc);
		if(fdst>0) close(fdst);
		return retval;
}
#endif /* RTCONFIG_REALTEK */

#ifdef RTCONFIG_BCMARM
int mtd_write_main_old(int argc, char *argv[])
#else
int mtd_write_main(int argc, char *argv[])
#endif
{
	int mf = -1;
	mtd_info_t mi;
	erase_info_t ei;
	FILE *f;
	unsigned char *buf = NULL, *p, *bounce_buf = NULL;
	const char *error;
	long filelen = 0, n, wlen, unit_len;
	struct sysinfo si;
	uint32 ofs;
	int c;
	char *iname = NULL;
	char *dev = NULL;
	char msg_buf[2048];
	int alloc = 0, bounce = 0, fd;
#ifdef DEBUG_SIMULATE
	FILE *of;
#endif

	while ((c = getopt(argc, argv, "i:d:")) != -1) {
		switch (c) {
		case 'i':
			iname = optarg;
			break;
		case 'd':
			dev = optarg;
			break;
		}
	}

	if ((iname == NULL) || (dev == NULL)) {
		usage_exit(argv[0], "-i file -d part");
	}

	if (!wait_action_idle(10)) {
		printf("System is busy\n");
		return 1;
	}

	set_action(ACT_WEB_UPGRADE);
#ifdef RTCONFIG_REALTEK

		if (!asusimg2rtkimg(iname))
			goto ERROR;

		if(strcmp(iname,"/tmp/linux.trx")==0)
		{
#ifdef CONFIG_MTD_NAND
			if(mtd_erase("/dev/mtdblock2")){
				printf("%s %d\n", __FUNCTION__,__LINE__);
			}
			if(copy_file2file("/tmp/linux.trx",0,"/dev/mtdblock2",0x0,msg_buf)<0)
#else
			if(copy_file2file("/tmp/linux.trx",0,"/dev/mtdblock2",0x0,msg_buf)<0)
#endif
			{
			//	_dprintf("%s %d\n", __FUNCTION__,__LINE__);
				error=msg_buf;
			}
		//	_dprintf("%s %d\n", __FUNCTION__,__LINE__);
#ifdef CONFIG_ASUS_DUAL_IMAGE_ENABLE
#ifdef CONFIG_MTD_NAND
			if(mtd_erase("/dev/mtdblock4")){
				printf("%s %d\n", __FUNCTION__,__LINE__);
			}
			if(copy_file2file("/tmp/linux.trx",0,"/dev/mtdblock4",0x0,msg_buf)<0)
#else
			if(copy_file2file("/tmp/linux.trx",0,"/dev/mtdblock2",0x0,msg_buf)<0)
#endif
			{
			//	_dprintf("%s %d\n", __FUNCTION__,__LINE__);
				error=msg_buf;
			}
#endif
		}else
		if(strcmp(iname,"/tmp/root.trx")==0)
		{
			//_dprintf("%s %d\n", __FUNCTION__,__LINE__);
#ifdef CONFIG_MTD_NAND
			if(mtd_erase("/dev/mtdblock3")){
				printf("%s %d\n", __FUNCTION__,__LINE__);
			}
			if(copy_file2file("/tmp/root.trx",0,"/dev/mtdblock3",0,msg_buf)<0)
#else
			if(copy_file2file("/tmp/root.trx",0,"/dev/mtdblock3",0,msg_buf)<0)
#endif
			{
			//	_dprintf("%s %d\n", __FUNCTION__,__LINE__);
				error=msg_buf;
			}
			//_dprintf("%s %d\n", __FUNCTION__,__LINE__);
#ifdef CONFIG_ASUS_DUAL_IMAGE_ENABLE
#ifdef CONFIG_MTD_NAND
			if(mtd_erase("/dev/mtdblock5")){
				printf("%s %d\n", __FUNCTION__,__LINE__);
			}
			if(copy_file2file("/tmp/root.trx",0,"/dev/mtdblock5",0,msg_buf)<0)
#else
			if(copy_file2file("/tmp/root.trx",0,"/dev/mtdblock3",0,msg_buf)<0)
#endif
			{
			//	_dprintf("%s %d\n", __FUNCTION__,__LINE__);
				error=msg_buf;
			}
#endif
		}
		//_dprintf("%s %d\n", __FUNCTION__,__LINE__);
	goto RTK_FINISH;
#endif /* RTCONFIG_REALTEK */

	if ((f = fopen(iname, "r")) == NULL) {
		error = "Error opening input file";
		goto ERROR;
	}

	fd = fileno(f);
	fseek( f, 0, SEEK_END);
	filelen = ftell(f);
	fseek( f, 0, SEEK_SET);
	_dprintf("file len=0x%x\n", filelen);

#ifdef RTCONFIG_BCMARM
	if ((mf = mtd_open_old(dev, &mi)) < 0) {
#else
	if ((mf = mtd_open(dev, &mi)) < 0) {
#endif
		snprintf(msg_buf, sizeof(msg_buf), "Error opening MTD device. (errno %d (%s))", errno, strerror(errno));
		error = msg_buf;
		goto ERROR;
	}

	if (mi.erasesize < sizeof(struct trx_header)) {
		error = "Error obtaining MTD information";
		goto ERROR;
	}

	_dprintf("mtd size=%x, erasesize=%x, writesize=%x, type=%x\n", mi.size, mi.erasesize, mi.writesize, mi.type);

#if defined(RTCONFIG_MTK_NAND)
	unit_len = mi.erasesize;
#else
	unit_len = ROUNDUP(filelen, mi.erasesize);
#endif
	if (unit_len > mi.size) {
		error = "File is too big to fit in MTD";
		goto ERROR;
	}

	if ((buf = mmap(0, filelen, PROT_READ, MAP_SHARED, fd, 0)) == (unsigned char*)MAP_FAILED) {
		_dprintf("mmap %x bytes fail!. errno %d (%s).\n", filelen, errno, strerror(errno));
		alloc = 1;
	}

	sysinfo(&si);
	if (alloc) {
		if ((si.freeram * si.mem_unit) <= (unit_len + (4096 * 1024)))
			unit_len = mi.erasesize;
	}

#if !defined(RTCONFIG_MTK_NAND)
	if (mi.type == MTD_UBIVOLUME)
#endif
	{
		if (!(bounce_buf = malloc(mi.writesize))) {
			error = "Not enough memory";
			goto ERROR;
		}
	}
	_dprintf("freeram=%lx unit_len=%lx filelen=%lx mi.erasesize=%x mi.writesize=%x\n",
		si.freeram, unit_len, filelen, mi.erasesize, mi.writesize);

	if (alloc && !(buf = malloc(unit_len))) {
		error = "Not enough memory";
		goto ERROR;
	}

#ifdef DEBUG_SIMULATE
	if ((of = fopen("/mnt/out.bin", "w")) == NULL) {
		error = "Error creating test file";
		goto ERROR;
	}
#endif

	for (ei.start = ofs = 0, ei.length = unit_len, n = 0, error = NULL, p = buf;
	     ofs < filelen;
	     ofs += n, ei.start += unit_len)
	{
		wlen = n = MIN(unit_len, filelen - ofs);
#if !defined(RTCONFIG_MTK_NAND)
		if (mi.type == MTD_UBIVOLUME)
#endif
		{
			if (n >= mi.writesize) {
				n &= ~(mi.writesize - 1);
				wlen = n;
			} else {
				if (!alloc)
					memcpy(bounce_buf, p, n);
				bounce = 1;
				p = bounce_buf;
				wlen = ROUNDUP(n, mi.writesize);
			}
		}

		if (alloc && safe_fread(p, 1, n, f) != n) {
			error = "Error reading file";
			break;
		}

		_dprintf("ofs=%x n=%lx/%lx ei.start=%x ei.length=%x\n", ofs, n, wlen, ei.start, ei.length);

#ifdef DEBUG_SIMULATE
		if (fwrite(p, 1, wlen, of) != wlen) {
			fclose(of);
			error = "Error writing to test file";
			break;
		}
#else
		if (ei.start == ofs) {
			ioctl(mf, MEMUNLOCK, &ei);
			if (ioctl(mf, MEMERASE, &ei) != 0) {
				snprintf(msg_buf, sizeof(msg_buf), "Error erasing MTD block. (errno %d (%s))", errno, strerror(errno));
#if defined(RTCONFIG_MTK_NAND)
				//_dprintf("%s\n", msg_buf);
				filelen += n;
				continue;
#else
				error = msg_buf;
				break;
#endif
			}
		}
		if (write(mf, p, wlen) != wlen) {
			snprintf(msg_buf, sizeof(msg_buf), "Error writing to MTD device. (errno %d (%s))", errno, strerror(errno));
			error = msg_buf;
			break;
		}
#endif

		if (!(alloc || bounce))
			p += n;
	}

#ifdef DEBUG_SIMULATE
	fclose(of);
#endif

ERROR:
	if (!alloc)
		munmap((void*) buf, filelen);
	else
		free(buf);

	if (bounce_buf)
		free(bounce_buf);

	if (mf >= 0) {
		// dummy read to ensure chip(s) are out of lock/suspend state
		read(mf, &n, sizeof(n));
		close(mf);
	}
	if (f) fclose(f);

#ifdef RTCONFIG_REALTEK
RTK_FINISH:
#endif
	set_action(ACT_IDLE);

	_dprintf("%s\n",  error ? error : "Image successfully flashed");
	return (error ? 1 : 0);
}

#ifdef RTCONFIG_BCMARM

/*
 * Open an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @param       flags   open() flags
 * @return      return value of open()
 */
int
mtd_open(const char *mtd, int flags)
{
	FILE *fp;
	char dev[PATH_MAX];
	int i;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, mtd)) {
#ifdef LINUX26
				snprintf(dev, sizeof(dev), "/dev/mtd%d", i);
#else
				snprintf(dev, sizeof(dev), "/dev/mtd/%d", i);
#endif
				fclose(fp);
				return open(dev, flags);
			}
		}
		fclose(fp);
	}

	return open(mtd, flags);
}

/*
 * Erase an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int
mtd_erase(const char *mtd)
{
	int mtd_fd;
	mtd_info_t mtd_info;
	erase_info_t erase_info;
#ifdef RTAC87U
	char erase_err[255] = {0};
#endif

	/* Open MTD device */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0) {
		perror(mtd);
		return errno;
	}

	/* Get sector size */
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0) {
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	erase_info.length = mtd_info.erasesize;

	for (erase_info.start = 0;
	     erase_info.start < mtd_info.size;
	     erase_info.start += mtd_info.erasesize) {
		(void) ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			perror(mtd);
			close(mtd_fd);
#ifdef RTAC87U
						sprintf(erase_err, "logger -t ATE mtd_erase failed: [%d]", errno);
						system(erase_err);
#endif
			return errno;
		}
	}

	close(mtd_fd);
#ifdef RTAC87U
	sprintf(erase_err, "logger -t ATE mtd_erase OK:[%d]", errno);
	system(erase_err);
#endif
	return 0;
}

static char *
base64enc(const char *p, char *buf, int len)
{
	char al[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	char *s = buf;

	while (*p) {
		if (s >= buf+len-4)
			break;
		*(s++) = al[(*p >> 2) & 0x3F];
		*(s++) = al[((*p << 4) & 0x30) | ((*(p+1) >> 4) & 0x0F)];
		*s = *(s+1) = '=';
		*(s+2) = 0;
		if (! *(++p)) break;
		*(s++) = al[((*p << 2) & 0x3C) | ((*(p+1) >> 6) & 0x03)];
		if (! *(++p)) break;
		*(s++) = al[*(p++) & 0x3F];
	}

	return buf;
}

enum {
	METHOD_GET,
	METHOD_POST
};

static int
wget(int method, const char *server, char *buf, size_t count, off_t offset)
{
	char url[PATH_MAX] = { 0 }, *s;
	char *host = url, *path = "", auth[128] = { 0 }, line[512];
	unsigned short port = 80;
	int fd;
	FILE *fp;
	struct sockaddr_in sin;
	int chunked = 0, len = 0;

	if (server == NULL || !strcmp(server, "")) {
		_dprintf("wget: null server input\n");
		return (0);
	}

	strncpy(url, server, sizeof(url));

	/* Parse URL */
	if (!strncmp(url, "http://", 7)) {
		port = 80;
		host = url + 7;
	}
	if ((s = strchr(host, '/'))) {
		*s++ = '\0';
		path = s;
	}
	if ((s = strchr(host, '@'))) {
		*s++ = '\0';
		base64enc(host, auth, sizeof(auth));
		host = s;
	}
	if ((s = strchr(host, ':'))) {
		*s++ = '\0';
		port = atoi(s);
	}

	/* Open socket */
	if (!inet_aton(host, &sin.sin_addr))
		return 0;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	_dprintf("Connecting to %s:%u...\n", host, port);
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
	    connect(fd, (struct sockaddr *) &sin, sizeof(sin)) < 0 ||
	    !(fp = fdopen(fd, "r+"))) {
		perror(host);
		if (fd >= 0)
			close(fd);
		return 0;
	}
	_dprintf("connected!\n");

	/* Send HTTP request */
	fprintf(fp, "%s /%s HTTP/1.1\r\n", method == METHOD_POST ? "POST" : "GET", path);
	fprintf(fp, "Host: %s\r\n", host);
	fprintf(fp, "User-Agent: wget\r\n");
	if (strlen(auth))
		fprintf(fp, "Authorization: Basic %s\r\n", auth);
	if (offset)
		fprintf(fp, "Range: bytes=%ld-\r\n", offset);
	if (method == METHOD_POST) {
		fprintf(fp, "Content-Type: application/x-www-form-urlencoded\r\n");
		fprintf(fp, "Content-Length: %d\r\n\r\n", (int) strlen(buf));
		fputs(buf, fp);
	} else
		fprintf(fp, "Connection: close\r\n\r\n");

	/* Check HTTP response */
	_dprintf("HTTP request sent, awaiting response...\n");
	if (fgets(line, sizeof(line), fp)) {
		_dprintf("%s", line);
		for (s = line; *s && !isspace((int)*s); s++);
		for (; isspace((int)*s); s++);
		switch (atoi(s)) {
		case 200: if (offset) goto done; else break;
		case 206: if (offset) break; else goto done;
		default: goto done;
		}
	}
	/* Parse headers */
	while (fgets(line, sizeof(line), fp)) {
		_dprintf("%s", line);
		for (s = line; *s == '\r'; s++);
		if (*s == '\n')
			break;
		if (!strncasecmp(s, "Content-Length:", 15)) {
			for (s += 15; isblank(*s); s++);
			chomp(s);
			len = atoi(s);
		}
		else if (!strncasecmp(s, "Transfer-Encoding:", 18)) {
			for (s += 18; isblank(*s); s++);
			chomp(s);
			if (!strncasecmp(s, "chunked", 7))
				chunked = 1;
		}
	}

	if (chunked && fgets(line, sizeof(line), fp))
		len = strtol(line, NULL, 16);

	len = (len > count) ? count : len;
	len = fread(buf, 1, len, fp);

done:
	/* Close socket */
	fflush(fp);
	fclose(fp);
	return len;
}

int
http_get(const char *server, char *buf, size_t count, off_t offset)
{
	return wget(METHOD_GET, server, buf, count, offset);
}

/*
 * Write a file to an MTD device
 * @param       path    file to write or a URL
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int
mtd_write(const char *path, const char *mtd)
{
	int mtd_fd = -1;
	mtd_info_t mtd_info;
	erase_info_t erase_info;

	struct sysinfo info;
	struct trx_header trx;
	unsigned long crc;

	FILE *fp;
	char *buf = NULL;
	long count, len, off;
	int ret = -1;

	/* Examine TRX header */
	if ((fp = fopen(path, "r")))
		count = safe_fread(&trx, 1, sizeof(struct trx_header), fp);
	else
		count = http_get(path, (char *) &trx, sizeof(struct trx_header), 0);
	if (count < sizeof(struct trx_header)) {
		fprintf(stderr, "%s: File is too small (%ld bytes)\n", path, count);
		goto fail;
	}

	/* Open MTD device and get sector size */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0 ||
	    ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0 ||
	    mtd_info.erasesize < sizeof(struct trx_header)) {
		perror(mtd);
		goto fail;
	}

	if (trx.magic != TRX_MAGIC ||
	    trx.len > mtd_info.size ||
	    trx.len < sizeof(struct trx_header)) {
		fprintf(stderr, "%s: Bad trx header\n", path);
		goto fail;
	}

	/* Allocate temporary buffer */
	/* See if we have enough memory to store the whole file */
	sysinfo(&info);
	if (info.freeram >= trx.len) {
		erase_info.length = ROUNDUP(trx.len, mtd_info.erasesize);
		if (!(buf = malloc(erase_info.length)))
			erase_info.length = mtd_info.erasesize;
	}
	/* fallback to smaller buffer */
	else {
		erase_info.length = mtd_info.erasesize;
		buf = NULL;
	}
	if (!buf && (!(buf = malloc(erase_info.length)))) {
		perror("malloc");
		goto fail;
	}

	/* Calculate CRC over header */
	crc = hndcrc32((uint8 *) &trx.flag_version,
		       sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
		       CRC32_INIT_VALUE);

	if (trx.flag_version & TRX_NO_HEADER)
		trx.len -= sizeof(struct trx_header);

	/* Write file or URL to MTD device */
	for (erase_info.start = 0; erase_info.start < trx.len; erase_info.start += count) {
		len = MIN(erase_info.length, trx.len - erase_info.start);
		if ((trx.flag_version & TRX_NO_HEADER) || erase_info.start)
			count = off = 0;
		else {
			count = off = sizeof(struct trx_header);
			memcpy(buf, &trx, sizeof(struct trx_header));
		}
		if (fp)
			count += safe_fread(&buf[off], 1, len - off, fp);
		else
			count += http_get(path, &buf[off], len - off, erase_info.start + off);
		if (count < len) {
			fprintf(stderr, "%s: Truncated file (actual %ld expect %ld)\n", path,
				count - off, len - off);
			goto fail;
		}
		/* Update CRC */
		crc = hndcrc32((uint8 *)&buf[off], count - off, crc);
		/* Check CRC before writing if possible */
		if (count == trx.len) {
			if (crc != trx.crc32) {
				fprintf(stderr, "%s: Bad CRC\n", path);
				goto fail;
			}
		}
		/* Do it */
		(void) ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0 ||
		    write(mtd_fd, buf, count) != count) {
			perror(mtd);
			goto fail;
		}
	}

#ifdef PLC
  eval("gigle_util restart");
  nvram_set("plc_pconfig_state", "2");
  nvram_commit();
#endif

	printf("%s: CRC OK\n", mtd);
	ret = 0;

fail:
	if (buf) {
		/* Dummy read to ensure chip(s) are out of lock/suspend state */
		(void) read(mtd_fd, buf, 2);
		free(buf);
	}

	if (mtd_fd >= 0)
		close(mtd_fd);
	if (fp)
		fclose(fp);
	return ret;
}

#endif

#ifdef HND_ROUTER
#include <bcm_hwdefs.h>

#define TEMP_KERNEL_NVRM_FILE "/var/.temp.kernel.nvram"
#define PRE_COMMIT_KERNEL_NVRM_FILE "/var/.kernel_nvram.setting.prec"
#define TEMP_KERNEL_NVRAM_FILE_NAME "/var/.kernel_nvram.setting.temp"
#define KERNEL_NVRAM_FILE_NAME "/data/.kernel_nvram.setting"

int hnd_nvram_erase()
{
	int err = 0;

	if (access(PRE_COMMIT_KERNEL_NVRM_FILE, F_OK) != -1 &&
		unlink(PRE_COMMIT_KERNEL_NVRM_FILE) < 0) {
		_dprintf("*** Failed to delete file %s. Error: %s\n",
			PRE_COMMIT_KERNEL_NVRM_FILE, strerror(errno));
		err = errno;
	}

	if (access(TEMP_KERNEL_NVRAM_FILE_NAME, F_OK) != -1 &&
		unlink(TEMP_KERNEL_NVRAM_FILE_NAME) < 0) {
		_dprintf("*** Failed to delete file %s. Error: %s\n",
			TEMP_KERNEL_NVRAM_FILE_NAME, strerror(errno));
		err = errno;
	}

	if (access(TEMP_KERNEL_NVRM_FILE, F_OK) != -1 &&
		unlink(TEMP_KERNEL_NVRM_FILE) < 0) {
		_dprintf("*** Failed to delete file %s. Error: %s\n",
			TEMP_KERNEL_NVRM_FILE, strerror(errno));
		err = errno;
	}

	if (access(KERNEL_NVRAM_FILE_NAME, F_OK) != -1 &&
		unlink(KERNEL_NVRAM_FILE_NAME) < 0) {
		_dprintf("*** Failed to delete file %s. Error: %s\n",
			KERNEL_NVRAM_FILE_NAME, strerror(errno));
		err = errno;
	}

	sync();
	_dprintf("Erasing nvram done\n");
	return err;
}

/*****************************************************************************
 * Image interface handler. Used for accessing/writing flash device.
 *****************************************************************************/
IMGIF_HANDLE imgifHandle = NULL;

CmsImageFormat parseImgHdr(UINT8 *bufP, UINT32 bufLen)
{
   int result = CMS_IMAGE_FORMAT_FLASH;

   return result;
}

/*****************************************************************************
 *  FUNCTION:  bca_sys_upgrade
 *  PURPOSE:   Receiving an image content from httpd and writing to NAND flash.
 *  PARAMETERS:
 *      path (IN) - pipe file path.
 *  RETURNS:
 *      0 - succeeded.
 *      errno - failed operation.
 *  NOTES:
 *       The calling sequence:
 *  This function is called from the context of the /sbin/init programm process.
 *  The /sbin/init process is "forked" by HTTPD process and it is the child of
 *  HTTPD process.
 *  The communication between HTTPD (parent) and bca_sys_upgrade is hold via pipe.
 *****************************************************************************/

int
bca_sys_upgrade(const char *path)
{
	int ret = 0;
	pid_t pid = getpid();
	int imgsz, ulimgsz = 0;
#if 0
	int spsz;
#endif
	int r_count, w_count;
	FILE *fp = NULL;
	char *buf = NULL;
	uint bufsz = 0;
	imgif_flash_info_t flash_info;
	uint blknum = 0;

	/* Opening communication pipe between HTTPD parent process */
	if ((fp = fopen(path, "r")) == NULL) {
		_dprintf("*** Filed open a file %s. Error: %s. Aborting \n",
				path, strerror(errno));
		ret = errno;
		goto fail;
	}

#ifdef RTCONFIG_PIPEFW
	/* HTTPD parent process supposed to send the image size. reading it. */
	r_count = safe_fread((void*)&imgsz, 1, sizeof(imgsz), fp);
	if (r_count < sizeof(imgsz)) {
		_dprintf("*** Error(pid:%d): %s@%d Pipe read failed. Expected:%d,read:%d. Aborting\n",
			pid, __FUNCTION__, __LINE__, sizeof(imgsz), r_count);
		ret = EPIPE;
		goto fail;
	}
	_dprintf("%s@%d(pid:%d): image size=%d.\n", __FUNCTION__, __LINE__, pid, imgsz);
#else
	fseek(fp, 0, SEEK_END);
	imgsz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
#endif

	/* Initialize IMGIF context */
	imgifHandle = imgif_open(parseImgHdr, NULL);
	if (imgifHandle == NULL) {
		_dprintf("*** Error(pid:%d): %s@%d Failed to create image ifc context. Aborting\n",
			pid, __FUNCTION__, __LINE__);
		ret = EIO;
		goto fail;
	}

#if defined(IMGIF_API_VERSION)
	/* query flash device information */
	if (imgif_get_flash_info(&flash_info) < 0) {
		_dprintf("*** Error(pid:%d): %s@%d Failed to get flash info. Aborting\n",
			getpid(), __FUNCTION__, __LINE__);
		ret = EIO;
	goto fail;
	}
#else
	/* query flash device information */
	if (imgif_get_flash_info(imgifHandle, &flash_info) < 0) {
		_dprintf("*** Error(pid:%d): %s@%d Failed to get flash info. Aborting\n",
			getpid(), __FUNCTION__, __LINE__);
		ret = EIO;
		goto fail;
	}
#endif

	/* evaluate image size */
	if (((imgsz + CMS_IMAGE_OVERHEAD) > flash_info.flashSize) ||
	    (imgsz < CMS_IMAGE_MIN_LEN)) {
		ret = EINVAL;
		goto fail;
	}

	/* setting image upload buf size equals to flash block size */
	bufsz = flash_info.eraseSize;

	/* Allocating image upload buffer */
	if ((buf = malloc(bufsz)) == NULL) {
		_dprintf("*** Error(pid:%d) %s@%d malloc failed. %s\n",
				getpid(), __FUNCTION__, __LINE__, strerror(errno));
		ret = errno;
		goto fail;
	}

	printf("\nUpgrading: ");
	/* uploading entire image by chunks */
	for (ulimgsz = 0, blknum = 1; ulimgsz < imgsz; ulimgsz += r_count, blknum++) {
		r_count = safe_fread((void*)buf, 1, bufsz, fp);
		if ((r_count < bufsz) && ((r_count + ulimgsz) != imgsz)) {
			/* This must be the last chunk, othrwise fail */
			_dprintf("*** Error(pid:%d): %s@%d Pipe read failed. Expected:%d,read:%d. Aborting\n",
			pid, __FUNCTION__, __LINE__, bufsz, r_count);
			ret = EPIPE;
			goto fail;
		}
		/* Write chunk to the flash. */
		w_count = imgif_write(imgifHandle, (UINT8*)buf, r_count);
		if ((w_count < 0) || (w_count != r_count)) {
			_dprintf("\nimgif_write() failed, towrite=%d, ret=%d",
				     w_count, r_count);
			ret = EIO;
			goto fail;
		}
		_dprintf(".");
	}

 fail:
	if (buf != NULL)
		free(buf);
	if (fp != NULL)
		fclose(fp);
	if (imgifHandle != NULL) {
		if (imgif_close(imgifHandle, (ret != 0)) == 0) {
			if (ret == 0) {
				_dprintf("\nDone. (written %d bytes, %d blocks with size %d\n",
					ulimgsz, blknum, flash_info.eraseSize);
				setBootImageState(BOOT_SET_NEW_IMAGE);
			}
		} else {
			if (ret == 0) {
				_dprintf("\n*** Fail to write the image\n");
			}
			ret = EIO;
		}
	}

	return ret;
}

#endif
