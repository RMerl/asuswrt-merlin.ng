/*
 * NVRAM variable manipulation (Linux user mode half)
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_linux.c,v 1.18 2005/05/16 12:35:03 honor Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <shared.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <nvram_convert.h>
#include <shutils.h>
#include <utils.h>
#ifdef RTCONFIG_REALTEK
#include <sysdeps/realtek/realtek.h>
#endif


#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
#define PATH_DEV_NVRAM "/dev/"MTD_OF_NVRAM
#else
#define PATH_DEV_NVRAM "/dev/nvram"
#endif

#define NVRAM_IOCTL_GET_SPACE	0x0001

/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;
#ifdef RTCONFIG_REALTEK
static char hw_mac[18];
#endif
int
nvram_init(void *unused)
{
	int ret;
	unsigned int nvram_space = MAX_NVRAM_SPACE;

	if (nvram_fd >= 0)
		return 0;

	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0)
		goto err;

	ret = ioctl(nvram_fd, NVRAM_IOCTL_GET_SPACE, &nvram_space);	// get the real nvram space size
	/* Map kernel string buffer into user space */
	nvram_buf = mmap(NULL, nvram_space, PROT_READ, MAP_SHARED, nvram_fd, 0);
	if (nvram_buf == MAP_FAILED) {
		close(nvram_fd);
		nvram_fd = -1;
		goto err;
	}

	fcntl(nvram_fd, F_SETFD, FD_CLOEXEC);

	return 0;

err:
	perror(PATH_DEV_NVRAM);
	return errno;
}

#ifdef RTCONFIG_REALTEK
int get_rtk_mac_offset(const char *name)
{
	int offset = HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T);
	if(strcmp(name,"rtk_mac_nic0")==0)
	{
		offset+=(int)(&((struct hw_setting *)0)->nic0Addr);
	}else
	if(strcmp(name,"rtk_mac_nic1")==0)
	{
		offset+=(int)(&((struct hw_setting *)0)->nic1Addr);
	}else
	if(strcmp(name,"rtk_mac_wl0")==0)
	{
		offset+=(int)(&((struct hw_setting *)0)->wlan);
		offset += sizeof(struct hw_wlan_setting);
		offset+=(int)(&((struct hw_wlan_setting *)0)->macAddr);
	}else
	if(strcmp(name,"rtk_mac_wl1")==0)
	{
		offset+=(int)(&((struct hw_setting *)0)->wlan);
		offset+=(int)(&((struct hw_wlan_setting *)0)->macAddr);
	}else
	{
		return 0;
	}
	return offset;
}
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}
#endif /* RTCONFIG_REALTEK */

char *nvram_get(const char *name)
{
	char tmp[100];
	char *value;
	size_t count = strlen(name) + 1;
	unsigned long *off = (unsigned long *)tmp;
#ifdef RTCONFIG_REALTEK
	if(strncmp(name,"rtk_mac",strlen("rtk_mac"))==0)
	{
		unsigned char tmpMac[6]={0};
		int offset=get_rtk_mac_offset(name);
		if(!offset)
			return NULL;
		bzero(hw_mac,sizeof(hw_mac));
		rtk_flash_read(tmpMac,offset,sizeof(tmpMac));
		sprintf(hw_mac,"%02x%02x%02x%02x%02x%02x",tmpMac[0],tmpMac[1],tmpMac[2],tmpMac[3],tmpMac[4],tmpMac[5]);
		return hw_mac;
	}
#endif /* RTCONFIG_REALTEK */

	if (nvram_fd < 0) {
		if (nvram_init(NULL) != 0) return NULL;
	}

	if (count > sizeof(tmp)) {
		if ((off = malloc(count)) == NULL) return NULL;
	}

	/* Get offset into mmap() space */
	strcpy((char *) off, name);
	count = read(nvram_fd, off, count);

	if (count == sizeof(*off)) {
		value = &nvram_buf[*off];
	}
	else {
		value = NULL;
		if (count < 0) perror(PATH_DEV_NVRAM);
	}

	if (off != (unsigned long *)tmp) free(off);
	return value;
}

int nvram_getall(char *buf, int count)
{
	int r;
	
	if (count <= 0) return 0;

	*buf = 0;
	if (nvram_fd < 0) {
		if ((r = nvram_init(NULL)) != 0) return r;
	}
	r = read(nvram_fd, buf, count);
	if (r < 0) perror(PATH_DEV_NVRAM);
	return (r == count) ? 0 : r;
}

static char *nvram_xfr_buf = NULL; 

char * 
nvram_xfr(const char *buf)
{
	size_t count = strlen(buf)*2+1; // ham 1120
	int ret;
	char tmpbuf[1024];

	if(nvram_fd < 0)
		if ((ret = nvram_init(NULL)))
			return NULL;
	
	if(count > sizeof(tmpbuf))
		return NULL;

	strcpy(tmpbuf, buf);

	if(!nvram_xfr_buf)
		nvram_xfr_buf = (char *)malloc(1024+1);

	if(!nvram_xfr_buf) return NULL;

	ret = ioctl(nvram_fd, NVRAM_MAGIC, tmpbuf);

	if(ret<0) {
		return NULL;
	}
	else {
		strcpy(nvram_xfr_buf, tmpbuf);
		return nvram_xfr_buf;
	}
}


static int _nvram_set(const char *name, const char *value)
{
	size_t count = strlen(name) + 1;
	char tmp[100];
	char *buf = tmp;
	int ret;
#ifdef RTCONFIG_REALTEK
	if(strncmp(name,"rtk_mac",strlen("rtk_mac"))==0)
	{
		int offset=get_rtk_mac_offset(name);
		if(!value)
		{
			cprintf("format must be name=value!\n");
			return 0;
		}

		if(!offset|| strlen(value)!=12 || !string_to_hex(value, buf, 12))
		{
			cprintf("invalid mac addr,must be xxxxxxxxxxxx\n");
			return 0;
		}

		return rtk_flash_write(buf,offset,6);
	}
#endif /* RTCONFIG_REALTEK */

	if (nvram_fd < 0) {
		if ((ret = nvram_init(NULL)) != 0) return ret;
	}

	/* Unset if value is NULL */
	if (value) count += strlen(value) + 1;

	if (count > sizeof(tmp)) {
		if ((buf = malloc(count)) == NULL) return -ENOMEM;
	}

	if (value) {
		sprintf(buf, "%s=%s", name, value);
	}
	else {
		strcpy(buf, name);
	}

	ret = write(nvram_fd, buf, count);

	if (ret < 0) perror(PATH_DEV_NVRAM);

	if (buf != tmp) free(buf);

	return (ret == count) ? 0 : ret;
}

int nvram_set(const char *name, const char *value)
{
	return _nvram_set(name, value);
}

int nvram_unset(const char *name)
{
	return _nvram_set(name, NULL);
}

int nvram_commit(void)
{
	int r = 0;
	FILE *fp;

	if (nvram_get(ASUS_STOP_COMMIT) != NULL)
	{
		cprintf("# skip nvram commit #\n");
		return r;
	}

	fp = fopen("/var/log/commit_ret", "w");

	if (wait_action_idle(10)) {
		if (nvram_fd < 0) {
			if ((r = nvram_init(NULL)) != 0) goto finish;
		}
		set_action(ACT_NVRAM_COMMIT);
//		nvram_unset("dirty");
		r = ioctl(nvram_fd, NVRAM_MAGIC, NULL);
		set_action(ACT_IDLE);

		if (r < 0) {
			perror(PATH_DEV_NVRAM);
			cprintf("commit: error\n");
			if(fp!=NULL)
				fprintf(fp,"commit: error\n");
		}
		else {
			if(fp!=NULL)
                                fprintf(fp,"commit: OK\n");
		}
	}
	else {
		cprintf("commit: system busy\n");
		if(fp!=NULL)
                        fprintf(fp,"commit: system busy\n");
	}

finish:
	if(fp!=NULL) fclose(fp);

	return r;
}

/*
 * Write a file to an NVRAM variable.
 * @param	name	name of variable to get
 * @param	filenname	name of file to write
 * @return	return code
 *
 * Preserve mode (permissions) of the file.
 * Create the output directory.
 */
#define MAX_FS 4096
#define MAGICNUM 0x12161770	/* Ludwig van Beethoven's birthdate. */
int nvram_file2nvram(const char *varname, const char *filename)
{
	FILE *fp;
	int c,count;
	int i=0,j=0;
	struct stat stbuf;
	unsigned char mem[MAX_FS], buf[3 * MAX_FS];

	if ( !(fp=fopen(filename,"rb") )) {
		perror("");
		return 1;
	}

	stat(filename, &stbuf);
	*((mode_t *)mem) = stbuf.st_mode;
	*((mode_t *)mem+1) = MAGICNUM;
   
	count=fread(mem + 2*sizeof(mode_t), 1, sizeof(mem) - 2*sizeof(mode_t), fp);
	if (!feof(fp)) {
		fclose(fp);
		fprintf(stderr, "File too big.\n");
		return(1);
	}
	fclose(fp);
	count += 2*sizeof(mode_t);
	for (j = 0; j < count; j++) {
		if  (i > sizeof(buf)-3 )
			break;
		c=mem[j];
		if (c >= 32 && c <= 126 && c != '\\' && c != '~')  {
			buf[i++]=(unsigned char) c;
		} else if (c==0) {
			buf[i++]='~';
		} else {
			buf[i++]='\\';
			sprintf(buf+i,"%02X",c);
			i+=2;
		}
	}
	buf[i]=0;
	nvram_set(varname,buf);
	return 0;
}

/*
 * Get the value of an NVRAM variable and write it to a file.
 * It must have been written with nvram_file2nvram.
 * Directory path(s) are created, and permissions are preserved.
 * @param	name	name of variable to get
 * @param	filenname	name of file to write
 * @return	return code
 */
int nvram_nvram2file(const char *varname, const char *filename)
{
	int fnum;
	int c,tmp;
	int i=0,j=0;
	unsigned char *cp;
	unsigned char mem[MAX_FS], buf[3 * MAX_FS];

	cp = nvram_get(varname);
	if (cp == NULL) {
		printf("Key does not exist: %s\n", varname);
		return(1);
	}
	strcpy(buf, cp);
	while (buf[i] && j < sizeof(mem)-3 ) {
		if (buf[i] == '\\')  {
			i++;
			tmp=buf[i+2];
			buf[i+2]=0;
			sscanf(buf+i,"%02X",&c);
			buf[i+2]=tmp;
			i+=2;
			mem[j]=c;j++;
		} else if (buf[i] == '~') {
			mem[j++]=0;
			i++;
		} else {
			mem[j++]=buf[i++];
		}
	}
   
	if (j<=0)
		return j;
	if (*((mode_t *)mem+1) != MAGICNUM) {
		printf("Error: '%s' not created by nvram setfile.\n", varname);
		return(-1);
	}

	/* Create the directories to the path, as necessary. */
	strcpy(buf, filename);
	cp = strrchr(buf, '/');
	if (cp && cp > buf) {
		*cp = 0;
		eval("mkdir", "-m", "0777", "-p", buf);
	}
   
	if ( (fnum=open(filename, O_WRONLY | O_CREAT | O_TRUNC, *((mode_t *)mem))) < 0) {
		printf("failed. errno: %d\n", errno);
		perror(filename);
		return (-1);
	}
	i = write(fnum, mem + 2*sizeof(mode_t), j- 2* sizeof(mode_t));
	if (i != j- 2* sizeof(mode_t))
		perror(filename);
	close(fnum);
	return (i != (j- 2* sizeof(mode_t)));
}
