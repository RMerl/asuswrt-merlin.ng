/*  ASUS COMBINE FW  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "comfw.h"

char comimg_name[MAX_CF][MAX_NAMELEN];

size_t strlcpy(char *dst, const char *src, size_t size)
{
        size_t srclen, len;

        srclen = strlen(src);
        if (size <= 0)
                return srclen;

        len = (srclen < size) ? srclen : size - 1;
        memcpy(dst, src, len); /* should not overlap */
        dst[len] = '\0';

        return srclen;
}

int get_fw_type(char *filename)
{
	if(strstr(filename, ".trx"))
		return _TRX;
	else if(strstr(filename, ".w"))
		return _W;
	else if(strstr(filename, ".pkgtb"))
		return _PKGTB;

	return 0;
}

void add_cname(char *oname, int type)
{
	int nr = strlen(oname);

	/* replace .x to c.x */
	if(type == _TRX) {
		oname[nr-4] = 'c';
		oname[nr-3] = '.';
		oname[nr-2] = 't';
		oname[nr-1] = 'r';
		oname[nr] = 'x';
		oname[nr+1] = 0;
	} else if(type == _W) {
		oname[nr-2] = 'c';
		oname[nr-1] = '.';
		oname[nr] = 'w';
		oname[nr+1] = 0;
	} else if(type == _PKGTB) {
		oname[nr-6] = 'c';
		oname[nr-5] = '.';
		oname[nr-4] = 'p';
		oname[nr-3] = 'k';
		oname[nr-2] = 'g';
		oname[nr-1] = 't';
		oname[nr] = 'b';
		oname[nr+1] = 0;
	}
}

void usage(char *argv0)
{
	fprintf(stderr,
	"Usage: %s [-c(add c-tail)] [-d dumpname][-f cidx:0~3] file1 file2...,\n output stored in %s/[first_img_name]",
	argv0, OUTPUT_DIR);
}

int main(int argc, char *argv[]) 
{
	int i, c, ac=1, nr = 0;
	comfw_head cf_head, cf_head_r;
	FILE *fwp[MAX_CF], *fp=NULL;
	int cf_size = sizeof(comfw_head);
	int cf_num = 0;
	struct stat st;
	char out_name[MAX_NAMELEN], buf[BUFSIZE];
	int total_size = 0;
	int use_cname = 0;
	int ch;
	int dump_only = 0;
	char dump_name[128];
	int sp_fname_idx = 0;
	
	if(argc < 2)
		return -1;

	for (;;) {
		c = getopt( argc, argv, "cd:f:");
		if (c == EOF) break;
		switch (c) {
			case 'c':
				use_cname = 1;
				ac++;
				break;
			case 'd':
				strlcpy(dump_name, argv[ac+1], sizeof(dump_name));
				printf("dump_name=%s\n", dump_name);
				ac+=2;
				dump_only = 1;
				break;
			case 'f':
				sp_fname_idx = atoi(argv[ac+1]);
				printf("use comfw-idx%d as filename\n", sp_fname_idx);
				ac+=2;
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
	}

	memset(&cf_head, 0, sizeof(cf_head));
	memset(&cf_head_r, 0, sizeof(cf_head_r));
	for(i=0; i<MAX_CF; ++i) {
		fwp[i] = NULL;
		memset(&comimg_name[i], 0, sizeof(comimg_name[i]));
	}

	if(dump_only) {
		cf_num = 1;
	} else {
		cf_head.magic = COMFW_MAGIC;
		for(i=0; argv[ac+i] && i<MAX_CF; ++i) {
			strlcpy(comimg_name[i], argv[ac+i], MAX_NAMELEN);
			cf_head.fw_type[i] = get_fw_type(comimg_name[i]);
			fwp[i] = fopen(comimg_name[i], "r");
			if((stat(comimg_name[i], &st) == 0))
				cf_head.fw_size[i] = st.st_size;
		}
		cf_num = i;

		printf("comfw:(%d) [%lu][%ld]\n", cf_num, sizeof(comfw_head), sizeof(comimg_name[0]));
		for(i=0; i<cf_num; ++i) 
			printf("[%s]:(t:%d)(%d) \n", comimg_name[i], cf_head.fw_type[i], cf_head.fw_size[i]);
		printf("\n");
		mkdir(OUTPUT_DIR, 0755);
	}

	if(dump_only)
		sprintf(out_name, "%s", dump_name);
	else {
		if(sp_fname_idx > 0 && sp_fname_idx < MAX_CF && cf_head.fw_type[sp_fname_idx] && cf_head.fw_size[sp_fname_idx])
			sprintf(out_name, "%s/%s", OUTPUT_DIR, comimg_name[sp_fname_idx]);
		else
			sprintf(out_name, "%s/%s", OUTPUT_DIR, comimg_name[0]);
	}

	if(!dump_only) {
		/* add c name */
		if(use_cname)
			add_cname(out_name, cf_head.fw_type[0]);
		printf("output file:%s\n", out_name);

		/* write cf_head, img1, img2... */
		fp = fopen(out_name, "w");
		fwrite(&cf_head, 1, sizeof(cf_head), fp);	
		total_size += sizeof(cf_head);

		for(i=0; i<cf_num; ++i) {
			if(fwp[i] && cf_head.fw_type[i] && cf_head.fw_size[i]) {
                		while ((nr = fread(buf, 1, BUFSIZE , fwp[i])) > 0) {
                        		fwrite(buf, 1, nr, fp);
                		}
				total_size += cf_head.fw_size[i];
			}
		} 

		fclose(fp);
	}

	/* test read head */
	fp = fopen(out_name, "r");
	fread(&cf_head_r, 1, sizeof(cf_head_r), fp);	
	printf("\n\nread comfw: \nmagic is %x\n", cf_head_r.magic);
	if(!dump_only)
		printf("\ntotal size=%d\n", total_size);

	if(dump_only) {
		for(i=0; i<MAX_CF; ++i) 
			if(cf_head_r.fw_type[i])
				printf("[%d]:(t:%d)(%d) \n", i, cf_head_r.fw_type[i], cf_head_r.fw_size[i]);

	} else {
		for(i=0; i<cf_num; ++i) 
			printf("[%s]:(t:%d)(%d) \n", comimg_name[i], cf_head_r.fw_type[i], cf_head_r.fw_size[i]);
	}

	printf("\n");
	fclose(fp);

end:
	for(i=0; i<cf_num; ++i) 
		if(fwp[i])
			fclose(fwp[i]);

	return 0;
}
