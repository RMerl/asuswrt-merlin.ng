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


#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define PATH_DEV_NVRAM "/tmp/nvram.txt"
#define PATH_DEV_NVRAM_FLASH "/jffs/nvram.txt"
#define PATH_DEV_NVRAM_LOCK "/tmp/nv_lock.txt"

/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;

#define MAX_NVRAM_SPACE 0x10000
#define NVRAM_SPACE 	MAX_NVRAM_SPACE


#if !defined(LOCK_SH) 
    #define LOCK_SH             1       /* shared lock */
#endif
#if !defined(LOCK_EX)
    #define LOCK_EX             2       /* exclusive lock */
#endif
#if !defined(LOCK_NB)
    #define LOCK_NB             4       /* don't block when locking */
#endif
#if !defined(LOCK_UN)
    #define LOCK_UN             8       /* unlock */
#endif

int file_lock_tmp(const char *tag)
{
	return _file_lock("/tmp", tag);
}

void file_unlock_tmp(int lockfd)
{
	return _file_unlock(lockfd);
}

enum {EXLOCK, SHLOCK};

const int semkey=20140318;

int sync_file(const char *filename)
{
	FILE *pFile;
	char nvram_tmp[NVRAM_SPACE] = {0};

	memcpy(nvram_tmp, nvram_buf, NVRAM_SPACE);
	pFile = fopen(filename, "wb");
	fwrite(nvram_tmp, sizeof(char), NVRAM_SPACE, pFile);
	fclose(pFile);

	return 1;
}

int lock_shm(int op) {
#if 0
	struct sembuf lockop={0, 0, SEM_UNDO};
	int semid;
	ushort sem_val[] = {
		1 /*Init value of EXLOCK*/,
		0 /*Init value of SHLOCK*/ };

	if( (semid=semget(semkey,  2, IPC_CREAT|IPC_EXCL|0640)) >= 0) {
		if( semctl(semid, 0, SETALL, sem_val) < 0)
			return -1;
	}
	else {
		if((semid=semget(semkey,1,0)) < 0)
			return -1;
	}

	if( op & LOCK_NB )
		lockop.sem_flg |= IPC_NOWAIT;

	if( op & LOCK_EX ) {
		lockop.sem_num = EXLOCK;
		lockop.sem_op = -1;

		if( semop(semid,  &lockop, 1) < 0 )
			return -1;

		lockop.sem_num  = SHLOCK;
		lockop.sem_op = 0;
		return semop(semid, &lockop, 1);
	}
	else if( op &  LOCK_SH ) {
		lockop.sem_num = EXLOCK;
		lockop.sem_op = -1;
		if( semop(semid, &lockop,  1) < 0 )
			return -1;

		lockop.sem_num  = SHLOCK;
		lockop.sem_op = 1;
		if( semop(semid, &lockop, 1) < 0 )
			return -1;

		lockop.sem_num = EXLOCK;
		lockop.sem_op = 1;

		return semop(semid, &lockop, 1);
	}

	if( semctl(semid, 0, GETALL,  sem_val) < 0 )
		return -1;

	if( semctl(semid, SHLOCK, GETZCNT, 0) == 0 &&
		semctl(semid, EXLOCK, GETNCNT, 0)  == 0 )
	{
		if( sem_val[EXLOCK] <= 1 && sem_val[SHLOCK]  == 0) {
			return semctl(semid, 0, IPC_RMID, 0);
		}
	}

	if( sem_val[EXLOCK] >0 && sem_val[SHLOCK]  > 0 ) {
		lockop.sem_num = SHLOCK;
		lockop.sem_op = -1;

		return semop(semid, &lockop, 1);
	}
	else if( sem_val[EXLOCK]  <= 0 ) {
		if( semctl(semid, SHLOCK, GETZCNT, NULL) >0 ) {
			lockop.sem_num = SHLOCK;
			lockop.sem_op = -1;
		}
		else {
			lockop.sem_num = EXLOCK;
			lockop.sem_op = 1;
		}
		return semop(semid, &lockop,  1);
	}
	return 0;
#else
	return 1;
#endif
}

void
nv_token(char old, char new)
{
	char *a=NULL;

	for(a=strchr(nvram_buf, old); a; a=strchr(a, old))
		*a++ = new;
}

int nvram_init(void *unused)
{
	if (nvram_fd >= 0){
		return 0;
	}

	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0) {
		fprintf(stderr, "Open nvram failed\n");
		goto err;
	}

	/* Map kernel string buffer into user space */
	nvram_buf = mmap(NULL, MAX_NVRAM_SPACE, PROT_READ | PROT_WRITE, MAP_SHARED, nvram_fd, 0);
	if (nvram_buf == MAP_FAILED) {
		fprintf(stderr, "nv mmap failed\n");
		close(nvram_fd);
		nvram_fd = -1;
		goto err;
	}

	nv_token(0xa, 0);
	ftruncate(nvram_fd, NVRAM_SPACE);

	return 0;
err:
	perror(PATH_DEV_NVRAM);
	return errno;
}

char *_nvram_get(const char *name, int *err)
{
	char *var, *eq, *value;
	char var2[255] = {0};

	*err = 0;

	if(lock_shm(LOCK_EX) < 0) {
		fprintf(stderr, "%s: lock fail\n", __func__);
		*err = 1;
		return NULL;
	}

	if (nvram_fd < 0) {
		if (nvram_init(NULL) != 0) {
			if(lock_shm(LOCK_UN) < 0) {
				fprintf(stderr, "%s: unlock fail(fd) \n", __func__);
				return NULL;
			}
			return NULL;
		}
	}
	if(!name) {
		if(lock_shm(LOCK_UN) < 0) {
			fprintf(stderr, "%s: unlock fail(name) \n", __func__);
			return NULL;
		}
		return NULL;
	}

#if 1
	for(var=nvram_buf; *var; var=value+strlen(value)+1) {
		if (!(eq = strchr(var, '='))){
			break;
		}
		value = eq + 1;
		if ((eq - var) == strlen(name) &&
				strncmp(var, name, (eq - var)) == 0) {
			if(lock_shm(LOCK_UN) < 0) {
				fprintf(stderr, "%s: unlock fail(got) \n", __func__);
				return value;
			}
			return value;
		}
	}
#endif

	if(lock_shm(LOCK_UN) < 0) {
		fprintf(stderr, "%s: unlock fail(none) \n", __func__);
		return NULL;
	}

	return NULL;
}

int nvram_getall(char *buf, int count)
{
	int r;
	char tmp[count];
	int lockfd = file_lock_tmp("nvram");

	if (count <= 0){
		file_unlock_tmp(lockfd);
		return -1;
	}

	if(lock_shm(LOCK_EX) < 0) {
		fprintf(stderr, "%s: lock fail\n", __func__);
			file_unlock_tmp(lockfd);
			return -1;
	}

	memset(tmp, 0, sizeof(tmp));
	if (nvram_fd < 0) {
		if ((r = nvram_init(NULL)) != 0){
			file_unlock_tmp(lockfd);
			return r;
		}
	}

	r = read(nvram_fd, tmp, count);

	if(lock_shm(LOCK_UN) < 0) {
		fprintf(stderr, "%s: unlock fail\n", __FUNCTION__);
		file_unlock_tmp(lockfd);
		return -1;
	}

	if (r < 0) perror(PATH_DEV_NVRAM);

	memcpy(buf, tmp, count);

	file_unlock_tmp(lockfd);
	return (r == count) ? 0 : r;	
}

char *nvram_xfr(const char *buf)
{
#if 1	/* Segmentation fault */
	return "test";
#endif
}

char *get_nvname(const char *name)
{
	char *var=NULL, *value, *eq;

	for(var=nvram_buf; *var; var=value+strlen(value)+1) {
		if (!(eq = strchr(var, '=')))
			break;
		value = eq + 1;
		if ((eq - var) == strlen(name) && strncmp(var, name, (eq - var)) == 0){
			return var;
		}
	}
	return NULL;
}

int _nvram_set(const char *name, const char *value1)
{
	int ret;

	char temp[1024], nvram_tmp[NVRAM_SPACE] = {0}, value[NVRAM_SPACE] = {0};
	char *offset_next = NULL, *offset_found = NULL, *a = NULL, *nv_end = NULL;
	unsigned int nvram_size, length_first, length_second;
	int ext_length;
	char shrink = 0;
	FILE *pFile;

	// nv_token(0xa, 0);
	// ftruncate(nvram_fd, NVRAM_SPACE);

	memset(value, 0, sizeof(value));
	if (value1){   /* not nvram_unset */
		memcpy(value, value1, strlen(value1));
	}

	if(lock_shm(LOCK_EX) < 0) {
		fprintf(stderr, "%s: lock fail\n", __FUNCTION__);
		return -1;
	}

	if (nvram_fd < 0) {
		if ((ret = nvram_init(NULL)) != 0) {
			return ret;
		}
	}

	memset(temp, 0, sizeof(temp));

	/* offset of existed item */
	offset_found = get_nvname(name);

	for(a=strchr(nvram_buf,0); a&&*(a+1); a=strchr(nv_end, 0))
		nv_end = a+1;
	nv_end = a+1;

	nvram_size = nv_end - nvram_buf;        /* 0x0 indicates the end of now nvram */

	if(offset_found != NULL){       /* existed item */
		memset(nvram_tmp, 0, NVRAM_SPACE);
		/* offset of next item */
		offset_next = strchr(offset_found, 0)+1;

		/* + 1 is length of chr(0x3d) */
		/* newname=newvalue - oldname=value+0x0 */
		if (value1){    /* besides nvram_unset */
			ext_length = ( strlen(name) + 1 + strlen(value) ) - (offset_next - offset_found - 1);
			if (ext_length > 0){
				lseek(nvram_fd, nvram_size, SEEK_SET);
				write(nvram_fd, "", ext_length);
			} else
				shrink = 1;
		}
		length_first = offset_found - nvram_buf;
		length_second = nvram_size - (offset_next - nvram_buf);

		/* copy 1st section to nvram_tmp */
		memcpy(nvram_tmp, nvram_buf, length_first);

		/* copy 2nd section to nvram_tmp */
		memcpy(nvram_tmp + length_first, offset_next, length_second);

		if (value1){    /* nvram_unset */
			/* copy new name=value to tmp */
			memcpy(temp, name, strlen(name) * sizeof(char));
			memcpy(temp + (strlen(name) * sizeof(char)), "=", sizeof(char));
			memcpy(temp + ( (strlen(name) + 1) * sizeof(char)), value, strlen(value) * sizeof(char));
			temp[((strlen(name) + 1 + strlen(value)) * sizeof(char))] = 0x0;

			/* include chr(0x3d) and chr(0x0)*/
			memcpy(nvram_tmp + length_first + length_second, temp, (strlen(name) + 1 + strlen(value) + 1) * sizeof(char));
			if (ext_length > 0)
				nvram_size += ext_length;
		}
		memset(nvram_buf, 0, nvram_size);
		if(!value1 || shrink){
			if(!value1)
				nvram_size = length_first + length_second;
			if(shrink)
				nvram_size += ext_length;
			// ftruncate(nvram_fd, nvram_size);
		}
		// memcpy(nvram_buf, nvram_tmp, nvram_size);
		memcpy(nvram_buf, nvram_tmp, NVRAM_SPACE);
	}else{  /* new item */
		if (value1){    /* nvram_unset check */
			/* newname=newvalue + 0x0 */
			ext_length = ( strlen(name) + 1 + strlen(value) ) + 1;
			if (ext_length > 0){
				lseek(nvram_fd, nvram_size, SEEK_SET);
				write(nvram_fd, "", ext_length);
			}
			/* copy new name=value to tmp */
			memcpy(temp, name, strlen(name) * sizeof(char));
			memcpy(temp + (strlen(name) * sizeof(char)), "=", sizeof(char));
			memcpy(temp + ( (strlen(name) + 1) * sizeof(char)), value, strlen(value) * sizeof(char));
			temp[((strlen(name) + 1 + strlen(value)) * sizeof(char))] = 0x0;

			/* append to nvram_buf */
			memcpy(nvram_buf + nvram_size, temp, strlen(name) + 1 + strlen(value) + 1);
		}
	}

	// sync_file(PATH_DEV_NVRAM);
	// fsync(nvram_fd);

	if(lock_shm(LOCK_UN) < 0){
		fprintf(stderr, "%s: unlock fail\n", __FUNCTION__);
	}

	return 0;
}

#define MAX_RETRY	100

int nvram_set(const char *name, const char *value)
{
	int ret=0, retry=0;
	int lockfd = file_lock_tmp("nvram");

	while((ret=_nvram_set(name, value))) {
		if(retry++ > MAX_RETRY){
			break;
		}
	};

	file_unlock_tmp(lockfd);
	return ret;
}

char *nvram_get(const char *name)
{
	char *ret = NULL;
	int retry = 0, lock_err = 0;
	int lockfd = file_lock_tmp("nvram");

	if(lock_shm(LOCK_EX) < 0) {
		fprintf(stderr, "%s: lock fail\n", __FUNCTION__);
		file_unlock_tmp(lockfd);
		return "err";
	}

	while(!(ret = _nvram_get(name, &lock_err))) {
		if(lock_err) {
			if(retry++ > MAX_RETRY)
				break;
		}
		else{
			break;
		}
	};
/*
	ret = _nvram_get(name);
        if(lock_shm(LOCK_UN) < 0) {
                fprintf(stderr, "%s: unlock fail \n", __FUNCTION__);
		return "err";
        }
*/
	// return ret?ret:"";
	file_unlock_tmp(lockfd);
	return ret; /* ret may be NULL */
}

int nvram_unset(const char *name)
{
	int ret;
	int lockfd = file_lock_tmp("nvram");

	ret = _nvram_set(name, NULL);

	file_unlock_tmp(lockfd);
	return ret;
}

int nvram_commit(void)
{
	int lockfd = file_lock_tmp("nvram");
	FILE *pFile;
	char nvram_tmp[NVRAM_SPACE] = {0};

	if(lock_shm(LOCK_EX) < 0) {
		fprintf(stderr, "%s: lock fail\n", __FUNCTION__);
		file_unlock_tmp(lockfd);
		return -1;
	}

	if (nvram_fd < 0){
		if (nvram_init(NULL) != 0){
			file_unlock_tmp(lockfd);
			return 0;
		}
	}

	// system("cp -f /tmp/nvram.txt /jffs/nvram.txt");
	sync_file(PATH_DEV_NVRAM);
	sync_file(PATH_DEV_NVRAM_FLASH);
	sync(); sync(); sync();

	if(lock_shm(LOCK_UN) < 0) {
		fprintf(stderr, "%s: unlock fail \n", __FUNCTION__);
		file_unlock_tmp(lockfd);
		return -1;
	}

	file_unlock_tmp(lockfd);
	return 1;
}

