/***********************************************************************
 *
 *  Copyright (c) 2005  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2005:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 ************************************************************************/

/*
 * Frontend command-line utility for Linux NVRAM layer
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <typedefs.h>
#include <wlcsm_lib_api.h>
#include <unistd.h>
#include <ctype.h>
#include <shared.h>

#include "nvram_mode.h"

#define TEMP_KERNEL_NVRM_FILE "/var/.temp.kernel.nvram"
#define PRE_COMMIT_KERNEL_NVRM_FILE "/var/.kernel_nvram.setting.prec"
#define TEMP_KERNEL_NVRAM_FILE_NAME "/var/.kernel_nvram.setting.temp"
#define MFG_NVRAM_FILE_NAME_PATH "/mnt/nvram/nvram.nvm"
#define NVRAM_LINE_MAX (1024)
#define STR_MAX_LEN(a,b) (a>b?a:b)
#define ARG_VAL (**p_argv)
char g_line_buffer[NVRAM_LINE_MAX];
char *g_buf=NULL;

#ifdef DUMP_PREV_OOPS_MSG
extern int dump_prev_oops(void);
#endif

static int
isnumber(const char *src)
{
    char *iter = (char *)src;
    while (*iter) {
        if (! isdigit(*iter))
            return 0;
        iter++;
    }
    return 1;
}

#define NEXT_ARG(argc, argv) do { (argv)++; if (--(argc) <= 0 || !*(argv)) usage(NULL); } while (0)
#define ADVANCE_ARGCV(argc, argv,step) do { (argv)+=step; (argc)-=step; } while (0)
#define NEXT_IS_NUMBER(argc, argv) do { if (((argc) - 1 <= 0) ||!*(argv) || !isnumber((argv)[1])) { \
		usage(NULL); } } while (0)
#define NEXT_IS_VALID(argc) do { if ((argc - 1 <= 0)) usage(NULL); }  while (0)

enum {
    NVRAM_NORMAL_CMD=0,
    NVRAM_HIDDEN_CMD
};

void usage(char *cmd);
typedef int (*NVRAM_CMD_FUNC)(int *argc,char ***p_argv);

typedef struct nvram_cmd_item {
    char *cmd;
    char *prompt;
    char is_hidden_cmd;
    NVRAM_CMD_FUNC func;
    char align_tab;
} NVRAM_CMD_ITEM;


#ifdef NAND_SYS

/**** following functions are for kernel nvram handling ****
* The defaul kernel file * will be /data/.kernel_nvram.setting, when build image, the
* file kernel_nvram.setting under nvram will be burned to image and be available under
* /data directory after board bootup. when "nvram set" is used, the variable will save
* to a PRE_COMMIT_KERNEL_NVRM_FILE and then saved back to KERNEL_NVRAM_FILE_NAME when
* "nvram commit" is issued.
*/

/* internal function to update nvram in the existing file  */
static int  _assign_value(FILE *fp,FILE *to_fp,char *nm,char*vl,int is_commit)
{
    int is_handled=0,has_change=0;
    char *name,*value;
    if(!vl) vl="*DEL*";
    while(fgets(g_line_buffer,NVRAM_LINE_MAX,fp)!=NULL) {
        if(g_line_buffer[0]=='#') continue;
        value=g_line_buffer;
        name=strsep(&value,"=");
        if(name && value) {
            name=wlcsm_trim_str(name);
            value=wlcsm_trim_str(value);
            if(!strncmp(name,nm,STR_MAX_LEN(strlen(name),strlen(nm)))) {
                is_handled=1;
                if(is_commit && !strncmp(vl,"*DEL*",5)) {
                    has_change=1;
                    if(is_commit) continue;
                } else if(strncmp(value,vl,STR_MAX_LEN(strlen(value),strlen(vl)))) {
                    has_change=1;
                    sprintf(g_line_buffer,"%s=%s\n",nm,vl);
                }
            } else
                sprintf(g_line_buffer,"%s=%s\n",name,value);
            fputs(g_line_buffer,to_fp);
        }
    }
    if(!is_handled) {
        if(!(is_commit && !strncmp(vl,"*DEL*",5))) {
            sprintf(g_line_buffer,"%s=%s\n",nm,vl);
            fputs(g_line_buffer,to_fp);
            has_change=1;
        }
    }
    return has_change;
}

/**
 * internal function to set nvram value into file, either permenant
 * file or a temporary file based on b_commit
 */
static int nvram_kset(char *nm,char *vl,int is_commit)
{
    FILE *to_fp,*fp;
    char *target_file;
    int has_change=0;
    if(!is_commit) target_file=PRE_COMMIT_KERNEL_NVRM_FILE;
    else target_file=TEMP_KERNEL_NVRAM_FILE_NAME;

    if(!(fp=fopen(target_file,"r")))
        fp=fopen(target_file,"a+");
    if(!fp) return -1;

    to_fp=fopen(TEMP_KERNEL_NVRM_FILE,"w+");
    if(!to_fp) {
        fclose(fp);
        return -1;
    }

    has_change=_assign_value(fp,to_fp,nm,vl,is_commit);

    fclose(to_fp);
    fclose(fp);
    if(has_change) {
        unlink(target_file);
        rename(TEMP_KERNEL_NVRM_FILE,target_file);
    } else
        unlink(TEMP_KERNEL_NVRM_FILE);
    return has_change;
}

static int copy_file(char *from_file, char *to_file)
{
    FILE *from_fp,*fp;
    from_fp=fopen(from_file,"r");
    if(!from_fp)
        return 0;
    fp=fopen(to_file,"w+");
    if(fp) {
        while(fgets(g_line_buffer,NVRAM_LINE_MAX,from_fp)!=NULL)
            fputs(g_line_buffer,fp);
        fclose(fp);
    } else {
        fprintf(stderr,"%s:%d  Could not open file:%s \r\n",__FUNCTION__,__LINE__,to_file );
        fclose(from_fp);
        return -1;
    }
    fclose(from_fp);
    return 0;
}
/**
 * move tempary set nvram to kernel nvram file,then it will be used
 * by kernel with next boot up
 */
static int nvram_kcommit(void)
{
    int ret=0,has_change=0;
    char *name,*value;
    FILE *fp;
    static char line_buffer[NVRAM_LINE_MAX];
    fp=fopen(PRE_COMMIT_KERNEL_NVRM_FILE,"r");
    /* if there is no new nvram in temporary file, quit */
    if(!fp) return 0;
    /* temporary copy kernel nvram from /data to /var for merging */
    if(copy_file(KERNEL_NVRAM_FILE_NAME,TEMP_KERNEL_NVRAM_FILE_NAME)) {
        fprintf(stderr,"%s:%d temporary file error  \r\n",__FUNCTION__,__LINE__ );
        fclose(fp);
        return -1;
    }

    while(fgets(line_buffer,NVRAM_LINE_MAX,fp)!=NULL) {
        if(line_buffer[0]=='#') continue;
        value=line_buffer;
        name=strsep(&value,"=");
        if(name && value) {
            has_change+=nvram_kset(wlcsm_trim_str(name),wlcsm_trim_str(value),1);
        }
    }
    fclose(fp);
    /*copy the final file to /data partition */
    if(has_change)
        ret=copy_file(TEMP_KERNEL_NVRAM_FILE_NAME,KERNEL_NVRAM_FILE_NAME);
    unlink(TEMP_KERNEL_NVRAM_FILE_NAME);
    unlink(PRE_COMMIT_KERNEL_NVRM_FILE);
    return ret;

}
#endif
/**
 * read from nvram files and populate it to the system
 */
void kernel_nvram_populate(char *file_name)
{
    char *name,*value;
    if(file_name) {
        FILE *fp=fopen(file_name,"r+");
        if(fp) {
            while(fgets(g_line_buffer,NVRAM_LINE_MAX,fp)!=NULL) {
                if(g_line_buffer[0]=='#') continue;
                value=g_line_buffer;
                name=strsep(&value,"=");
                if(name && value) {
                    name=wlcsm_trim_str(name);
                    value=wlcsm_trim_str(value);
                    wlcsm_nvram_set(name,value);
                }
            }
            fclose(fp);
        }
    }
}

/**************END OF KERNEL NVRAM HANDLING SECTION **************/



/******* NVRAM_CMDS implementations *********/

/** nvram set wl0_ssid="12345" **/
static int nvram_cmd_set(int *argc, char ***p_argv) {
    char  *name,*value=NULL;
    NEXT_ARG(*argc,*p_argv);
    strncpy(value = g_buf, ARG_VAL, strlen(ARG_VAL)+1);
    name = strsep(&value, "=");
#ifdef RTCONFIG_JFFS_NVRAM
    nvram_set(wlcsm_trim_str(name),wlcsm_trim_str(value));
#else
    wlcsm_nvram_set(wlcsm_trim_str(name),wlcsm_trim_str(value));
#endif

#ifdef NAND_SYS
    /*save to temporary file under var in
     *case it wil be commited */
    nvram_kset(wlcsm_trim_str(name),wlcsm_trim_str(value),0);
#endif
    return 0;
}

/** nvram setflag wl0_bititem 5=1 **/
static int nvram_cmd_setflag(int *argc, char ***p_argv) {
    char *bit_value,*bit_pos,*name,*value;
    char **argv;
    NEXT_ARG(*argc,*p_argv);
    NEXT_IS_VALID(*argc);
    argv=*p_argv;

    bit_value = argv[1];
    bit_pos = strsep(&bit_value, "=");
    if (bit_value && bit_pos && isnumber(bit_value) && isnumber(bit_pos)) {
        if ((value = wlcsm_nvram_get(argv[0])))
            printf("value:%s->",value);
        else
            printf("value:NULL->");

        wlcsm_nvram_set_bitflag(argv[0], atoi(bit_pos), atoi(bit_value));
        if ((name = wlcsm_nvram_get(argv[0])))
            printf("%s\n",name);
    }
    ADVANCE_ARGCV(*argc,*p_argv,1);
    return 0;
}

/** nvram unset wl0_ssid **/
static int nvram_cmd_unset(int *argc, char ***p_argv) {
    NEXT_ARG(*argc,*p_argv);
#ifdef RTCONFIG_JFFS_NVRAM
    nvram_unset(ARG_VAL);
#else
    wlcsm_nvram_unset(ARG_VAL);
#endif

#ifdef NAND_SYS
    nvram_kset((ARG_VAL), NULL,0);
#endif
    return 0;
}

/** nvram get wl0_ssid **/
static int nvram_cmd_get(int *argc, char ***p_argv) {
    char *value=NULL;
    NEXT_ARG(*argc,*p_argv);
#ifdef RTCONFIG_JFFS_NVRAM
    if ((value = nvram_get(ARG_VAL)))
#else
    if ((value = wlcsm_nvram_get(ARG_VAL)))
#endif
        puts(value);
    return 0;
}

/** nvram getflag wl0_ssid 6 */
static int nvram_cmd_getflag(int *argc, char ***p_argv) {
    char *value=NULL;
    char **argv;
    NEXT_ARG(*argc,*p_argv);
    NEXT_IS_NUMBER(*argc, *p_argv);
    argv=*p_argv;
    if ((value = wlcsm_nvram_get_bitflag(argv[0], atoi(argv[1]))))
        puts(value);
    ADVANCE_ARGCV(*argc,*p_argv,1);
    return 0;
}

/** nvram getall|dump|show **/
static int nvram_cmd_getall(int *argc, char ***p_argv) {
    char *name=NULL;
    int len;
    int size;
    ADVANCE_ARGCV(*argc,*p_argv,1);
#ifdef RTCONFIG_JFFS_NVRAM
    len = nvram_getall(g_buf, MAX_NVRAM_SPACE );
#else
    len = wlcsm_nvram_getall(g_buf, MAX_NVRAM_SPACE );
#endif
    for (name = g_buf; *name||(int)name-(int)g_buf<len; name += strlen(name) + 1)
    {
	if(!*name) {
		puts("XX ILLEGAL nvram");
		continue;
	}

        puts(name);
    }
    size = sizeof(struct nvram_header) + (int) name - (int)g_buf;
    fprintf(stderr, "size: %d bytes (%d left)\n", size, MAX_NVRAM_SPACE - size);
    return 0;
}
/** nvram loadfile filename **/
/** populate nvram from a file on board **/
int nvram_cmd_loadfile(int *argc, char ***p_argv) {
    char *file_name;
    ADVANCE_ARGCV(*argc,*p_argv,1);
    file_name=(ARG_VAL?ARG_VAL:"/data/nvramdefault.txt");
    kernel_nvram_populate(file_name);
    printf("popuplate nvram from %s done!\n",file_name);
    return 0;
}


/** nvram savefile filename **/
/** save nvram to a file on board **/
int nvram_cmd_savefile(int *argc, char ***p_argv) {
    char *name,*file_name;
    FILE *ofp;
    ADVANCE_ARGCV(*argc,*p_argv,1);
    file_name=(ARG_VAL?ARG_VAL:"/data/nvramdefault.txt");
    ofp =fopen(file_name,"w");
    if(!ofp) {
        fprintf(stderr,"%s:%d open configuration file error \r\n",__FUNCTION__,__LINE__ );
        return -1;
    }
#ifdef RTCONFIG_JFFS_NVRAM
    nvram_getall(g_buf, MAX_NVRAM_SPACE );
#else
    wlcsm_nvram_getall(g_buf, MAX_NVRAM_SPACE );
#endif
    for (name = g_buf; *name; name += strlen(name) + 1) {
        fputs(name,ofp);
        fputs("\n",ofp);
    }
    fclose(ofp);
    printf("save nvram to %s done!\n",file_name);
    return 0;
}

/** nvram restart **/
static int nvram_cmd_restart(int *argc, char ***p_argv) {
    ADVANCE_ARGCV(*argc,*p_argv,1);
    wlcsm_mngr_restart(0,WLCSM_MNGR_RESTART_NVRAM,WLCSM_MNGR_RESTART_NOSAVEDM,0);
    return 0;
}

#ifdef NAND_SYS
int nvram_cmd_kernelset(int *argc, char ***p_argv) {

    NEXT_ARG(*argc,*p_argv);
    kernel_nvram_populate(ARG_VAL);
    printf("restore done!\r\n");
    return 0;
}

/*
* -1 - error
*/
/*****************************************************************************
*  FUNCTION:  nvram_mfg_read_entry
*  PURPOSE:  Read next "name=value" entry from  from manufacturing NVRAM
*            file  (/mnt/nvram/nvram.nvm).
*  PARAMETERS:
*      f(IN) - manufacturing default NVRAM file handler.
*      ptr(IN) - buffer to read in.
*      len(IN) - max size of the buffer "ptr"
*  RETURNS:
*      strlen of string "name=value".
*      -1 if error occur.
*  NOTES:
*      Each couple "name=value" in /mnt/nvram/nvram.nvm is ending with '\0'.
*
*****************************************************************************/
static int nvram_mfg_read_entry(FILE *f, void* ptr, int len)
{
    int rv = -1;
    int i;
    char *p = (char*)ptr;

    for (i = 0; i < len; i++, p++) {
        if ((fread(p, 1, 1, f) != 1) ||
                (*p == '\0')) {
            break;
        }
    }

    if (i < len)
        rv = i;

    return rv;
}

/*****************************************************************************
*  FUNCTION:  nvram_mfg_restore_default
*  PURPOSE:  Restore kernel NVRAM file (/data/.KERNEL_NVRAM_FILE_NAME)
*            from manufacturing default NVRAM setting (/mnt/nvram/nvram.nvm).
*  PARAMETERS:
*      pathname(IN) - the string of kernel NVRAM file name.
*  RETURNS:
*      0 - succeeded.
*      -1 error
*  NOTES:
*      /mnt/nvram/nvram.nvm is in binary format. Each "name=value" ending with '\0'
*
*****************************************************************************/
static int nvram_mfg_restore_default(char *pathname)
{
    FILE *f_mfg, *f_usr,*f_macs;
    int err = -1;
    char append_mac=1;


    if ((f_mfg = fopen(MFG_NVRAM_FILE_NAME_PATH, "rb")) != NULL) {
        clearerr(f_mfg);
        if ((f_usr = fopen(pathname, "a")) != NULL) {

            while ((err = nvram_mfg_read_entry(f_mfg, (void*)&g_line_buffer[0],
                                               sizeof(g_line_buffer))) > 0) {
                /* check if et0macaddr entry is included */
                if(!strncmp(g_line_buffer,"et0macaddr=",11))
                    append_mac=0;
                fprintf(f_usr, "%s\n", g_line_buffer);
            }

            if(append_mac) {
                char macs[18]= {0};
                if ((f_macs = fopen("/proc/nvram/BaseMacAddr", "rb")) != NULL) {
                    if(fread(macs,1,18,f_macs)) {
                        fprintf(f_usr,"et0macaddr=%s\n",macs);
                    }
                    fclose(f_macs);
                }
            }

            fclose(f_usr);
        }

        fclose(f_mfg);
    }

    return err;
}

int nvram_cmd_restore_mfg(int *argc, char ***p_argv) {
    NEXT_ARG(*argc,*p_argv);
    printf("Restoring NVRAM to manufacturing default ... ");
    if (nvram_mfg_restore_default(ARG_VAL) == 0)
        printf("done.\r\n");
    else
        printf("fail.\r\n");
    return 0;
}

#endif

/** nvram commit [restart] **/
static int nvram_cmd_commit(int *argc, char ***p_argv) {
    ADVANCE_ARGCV(*argc,*p_argv,1);
    //sends message to wldaemo to covers all the work nvram_commit does.
    if(ARG_VAL && !strncmp(ARG_VAL, "restart", 7)) {
        wlcsm_mngr_restart(0,WLCSM_MNGR_RESTART_NVRAM,WLCSM_MNGR_RESTART_SAVEDM,0);
    }
    else
        wlcsm_nvram_commit(); //no need restart for all other cases
#if defined(NAND_SYS)
    nvram_kcommit();
#endif
    return 0;
}

/** nvram kcommit **/
static int nvram_cmd_kcommit(int *argc, char ***p_argv) {
    ADVANCE_ARGCV(*argc,*p_argv,1);
#if defined(NAND_SYS)
    nvram_kcommit();
#endif
    return 0;
}
/** nvram godefault **/
static int nvram_cmd_wl_godefault(int *argc, char ***p_argv) {
    ADVANCE_ARGCV(*argc,*p_argv,1);
    //sends message to wldaemo to covers all the work nvram_commit does.
    //and remove kernel nvram configu.
    unlink(KERNEL_NVRAM_FILE_NAME);
    sync();
    //wlcsm_mngr_wl_godefault();
    return 0;
}

/**************************************************/
/*** 			ASUS 			***/
/**************************************************/
#define PROFILE_HEADER          "HDR1"
#ifdef RTCONFIG_DSL
#define PROFILE_HEADER_NEW      "N55U"
#else
#define PROFILE_HEADER_NEW      "HDR2"
#endif

#define PROTECT_CHAR    'x'
#define DEFAULT_LOGIN_DATA	"xxxxxxxx"
/*******************************************************************
* NAME: _secure_romfile
* AUTHOR: Andy Chiu
* CREATE DATE: 2015/06/08
* DESCRIPTION: replace account /password by PROTECT_CHAR with the same string length
* INPUT:  path: the rom file path
* OUTPUT:
* RETURN:  0: success, -1:failed
* NOTE: Andy Chiu, 2015/12/18. Add new tokens.
*           Andy Chiu, 2015/12/24. Add new tokens.
*	     Andy Chiu, 2016/02/18. Add new token, wtf_username.
*******************************************************************/
/* memset list field.
 * member mask is bitmask: 0 - 1st, 2 - 2nd, ... 0x80000000 - 31th member */
static char *nvlist_memset(char *list, int c, unsigned int member_mask)
{
	char *item, *end, *next;
	char *mitem, *mend, *mnext;
	unsigned int mask;

	if (!list || member_mask == 0)
		return list;

	for (item = list; *item; ) {
		end = strchr(item, '<');
		if (end)
			next = end + 1;
		else	next = end = strchr(item, 0);
		for (mitem = item, mask = member_mask;
		     mitem < end && mask; mask >>= 1) {
		        mend = memchr(mitem, '>', end - mitem);
			if (mend)
				mnext = mend + 1;
			else	mnext = mend = end;
			if (mask & 1)
				memset(mitem, c, mend - mitem);
			mitem = mnext;
		}
		item = next;
	}

	return list;
}

#if 0
static int _secure_conf(char* buf)
{
	char name[128], *value, *item;
	int len, i;

	//name contains token
	const char *keyword_token[] = {"http_username", "passwd", "password",
		NULL};	//Andy Chiu, 2015/12/18

	//name is token
	const char *token1[] = {"wan_pppoe_passwd", "modem_pass", "modem_pincode",
		"http_passwd", "wan0_pppoe_passwd", "dslx_pppoe_passwd", "ddns_passwd_x",
		"wl_wpa_psk",	"wlc_wpa_psk",  "wlc_wep_key",
		"wl0_wpa_psk", "wl0.1_wpa_psk", "wl0.2_wpa_psk", "wl0.3_wpa_psk",
		"wl1_wpa_psk", "wl1.1_wpa_psk", "wl1.2_wpa_psk", "wl1.3_wpa_psk",
		"wl0.1_key1", "wl0.1_key2", "wl0.1_key3", "wl0.1_key4",
		"wl0.2_key1", "wl0.2_key2", "wl0.2_key3", "wl0.2_key4",
		"wl0.3_key1", "wl0.3_key2", "wl0.3_key3", "wl0.3_key4",
		"wl0_key1", "wl0_key2", "wl0_key3", "wl0_key4",
		"wl1.1_key1", "wl1.1_key2", "wl1.1_key3", "wl1.1_key4",
		"wl1.2_key1", "wl1.2_key2", "wl1.2_key3", "wl1.2_key4",
		"wl1.3_key1", "wl1.3_key2", "wl1.3_key3", "wl1.3_key4",
		"wl_key1", "wl_key2", "wl_key3", "wl_key4",
		"wl1_key1", "wl1_key2", "wl1_key3", "wl1_key4",
		"wl0_phrase_x", "wl0.1_phrase_x", "wl0.2_phrase_x", "wl0.3_phrase_x",
		"wl1_phrase_x", "wl1.1_phrase_x", "wl1.2_phrase_x", "wl1.3_phrase_x",
		"wl_phrase_x", "vpnc_openvpn_pwd", "PM_SMTP_AUTH_USER", "PM_MY_EMAIL",
		"PM_SMTP_AUTH_PASS", "wtf_username", "ddns_hostname_x", "ddns_username_x",
		NULL};

	//name is token
	//value is [<]username>password<username...
	const char *token2[] = {"acc_list", "pptpd_clientlist", "vpn_serverx_clientlist",
		NULL};

	//name is token
	//valus is [<]desc>type>index>username>password<desc...
	const char vpnc_token[] = "vpnc_clientlist";

	//name is token
	//value is [<]type>username>password>url>rule>dir>enable<type...
	const char cloud_token[] = "cloud_sync";

	//name is token
	//value is username@domain.tld
	const char pppoe_username_token[] = "pppoe_username";

	if (!buf)
		return -1;

	for (item = buf; *item; item += strlen(item) + 1) {
		value = strchr(item, '=');
		if (!value)
			continue;
		len = value - item;
		if (len < 0 || len > sizeof(name) - 1)
			continue;

		strncpy(name, item, len);
		name[len] = '\0';
		value++;

		//check the password keyword token
		for (i = 0; keyword_token[i]; i++) {
			if (strstr(name, keyword_token[i]) != NULL) {
				memset(value, PROTECT_CHAR, strlen(value));
				goto next;
			}
		}

		//check the first token group
		for (i = 0; token1[i]; i++) {
			if (strcmp(name, token1[i]) == 0) {
				memset(value, PROTECT_CHAR, strlen(value));
				goto next;
			}
		}

		//check the 2nd token group
		//value is [<]username>password<username...
		for (i = 0; token2[i]; i++) {
			if (strcmp(name, token2[i]) == 0) {
				nvlist_memset(value, PROTECT_CHAR, (1u << 1));
				goto next;
			}
		}

		//check vpnc token
		//valus is [<]desc>type>index>username>password<desc...
		if (strcmp(name, vpnc_token) == 0) {
			nvlist_memset(value, PROTECT_CHAR, (1u << 4));
			goto next;
		}

		//check cloud sync token
		//value is [<]type>xxx1>xxx2>xxx3>xxx4>xxx5>xxx6<type...
		if (strcmp(name, cloud_token) == 0) {
			nvlist_memset(value, PROTECT_CHAR, (1u << 2)|(1u << 3)|(1u << 5));
			goto next;
		}

		//check
		//value is password@domain.tld
		if (strstr(name, pppoe_username_token) != NULL) {
			char *e = strchr(value, '@') ? : strchr(value, 0);
			memset(value, PROTECT_CHAR, e - value);
		}

	next:
		continue;
	}

	return 0;
}
#else
static int _convert_data(const char *name, char *value, size_t value_len)
{
	int i;

	//name contains token
	const char *http_token[] = {"http_username", "http_passwd", NULL};
	const char password_token[] = "password";

	//name is token
	const char *token1[] = {"wan_pppoe_passwd", "modem_pass", "modem_pincode",
		"http_passwd", "wan0_pppoe_passwd", "dslx_pppoe_passwd", "ddns_passwd_x",
		"wl_wpa_psk",	"wlc_wpa_psk",  "wlc_wep_key",
		"wl0_wpa_psk", "wl0.1_wpa_psk", "wl0.2_wpa_psk", "wl0.3_wpa_psk",
		"wl1_wpa_psk", "wl1.1_wpa_psk", "wl1.2_wpa_psk", "wl1.3_wpa_psk",
		"wl2_wpa_psk", "wl2.1_wpa_psk", "wl2.2_wpa_psk", "wl2.3_wpa_psk",
		"wl0.1_key1", "wl0.1_key2", "wl0.1_key3", "wl0.1_key4",
		"wl0.2_key1", "wl0.2_key2", "wl0.2_key3", "wl0.2_key4",
		"wl0.3_key1", "wl0.3_key2", "wl0.3_key3", "wl0.3_key4",
		"wl0_key1", "wl0_key2", "wl0_key3", "wl0_key4",
		"wl1.1_key1", "wl1.1_key2", "wl1.1_key3", "wl1.1_key4",
		"wl1.2_key1", "wl1.2_key2", "wl1.2_key3", "wl1.2_key4",
		"wl1.3_key1", "wl1.3_key2", "wl1.3_key3", "wl1.3_key4",
		"wl1_key1", "wl1_key2", "wl1_key3", "wl1_key4",
		"wl2.1_key1", "wl2.1_key2", "wl2.1_key3", "wl2.1_key4",
		"wl2.2_key1", "wl2.2_key2", "wl2.2_key3", "wl2.2_key4",
		"wl2.3_key1", "wl2.3_key2", "wl2.3_key3", "wl2.3_key4",
		"wl2_key1", "wl2_key2", "wl2_key3", "wl2_key4",
		"wl_key1", "wl_key2", "wl_key3", "wl_key4",
		"wl0_phrase_x", "wl0.1_phrase_x", "wl0.2_phrase_x", "wl0.3_phrase_x",
		"wl1_phrase_x", "wl1.1_phrase_x", "wl1.2_phrase_x", "wl1.3_phrase_x",
		"wl2_phrase_x", "wl2.1_phrase_x", "wl2.2_phrase_x", "wl2.3_phrase_x",
		"wl_phrase_x", "vpnc_openvpn_pwd", "PM_SMTP_AUTH_USER", "PM_MY_EMAIL",
		"PM_SMTP_AUTH_PASS", "wtf_username", "ddns_hostname_x", "ddns_username_x",
		NULL};

	//name is token
	//value is [<]username>password<username...
	const char *token2[] = {"acc_list", "pptpd_clientlist", "vpn_serverx_clientlist",
		NULL};

	//name is token
	//valus is [<]desc>type>index>username>password<desc...
	const char vpnc_token[] = "vpnc_clientlist";

	//name is token
	//value is [<]type>username>password>url>rule>dir>enable<type...
	const char cloud_token[] = "cloud_sync";

	//name is token
	//value is username@domain.tld
	const char pppoe_username_token[] = "pppoe_username";

	if(!value)
		return 0;

	//change http login username and password as xxxxxxxx
	for(i = 0; http_token[i]; ++i)
	{
		if(!strcmp(name, http_token[i]))
		{
			strlcpy(value, DEFAULT_LOGIN_DATA, value_len);
			return 1;
		}		
	}

	//check the password keyword token
	if (strstr(name, password_token) != NULL) 
	{
		memset(value, PROTECT_CHAR, strlen(value));
		return 1;
	}

	//check the first token group
	for (i = 0; token1[i]; i++) 
	{
		if (strcmp(name, token1[i]) == 0) 
		{
			memset(value, PROTECT_CHAR, strlen(value));
			return 1;
		}
	}

	//check the 2nd token group
	//value is [<]username>password<username...
	for (i = 0; token2[i]; i++)
	{
		if (strcmp(name, token2[i]) == 0)
		{
			nvlist_memset(value, PROTECT_CHAR, (1u << 1));
			return 1;
		}
	}

	//check vpnc token
	//valus is [<]desc>type>index>username>password<desc...
	if (strcmp(name, vpnc_token) == 0) 
	{
		nvlist_memset(value, PROTECT_CHAR, (1u << 4));
		return 1;
	}

	//check cloud sync token
	//value is [<]type>xxx1>xxx2>xxx3>xxx4>xxx5>xxx6<type...
	if (strcmp(name, cloud_token) == 0) 
	{
		nvlist_memset(value, PROTECT_CHAR, (1u << 2)|(1u << 3)|(1u << 5));
		return 1;
	}

	//check
	//value is password@domain.tld
	if (strstr(name, pppoe_username_token) != NULL) 
	{
		char *e = strchr(value, '@') ? : strchr(value, 0);
		memset(value, PROTECT_CHAR, e - value);
		return 1;
	}

	return 0;
}

static char* _get_attr(const char *buf, char *name, size_t name_len, char *value, size_t value_len)
{
	char *p, *e,  *v;
	if(!buf || !name || !value)
		return NULL;

	memset(name, 0, name_len);
	memset(value, 0, value_len);
	
	p = strchr(buf, '=');
	if(p)
	{
		strlcpy(name, buf, ((p - buf + 1) > name_len)? name_len:  (p - buf + 1));

		v = p + 1;
		e = strchr(v, '\0');
		if(e)
		{
			strlcpy(value, v, ((e - v + 1) > value_len)? value_len:  (e - v + 1));
			return e + 1;
		}
		else	//last line
		{
			strlcpy(value, v, value_len);
			return 0;
		}
		
	}
	return NULL;
}

static int _secure_conf(char* buf, size_t len)
{
	char *tmp, *p = buf, *b;
	char name[256], *value;
	int tmp_len;

	if(!buf || !len)
		return -1;

	tmp = malloc(len);
	if(!tmp)
	{
		fprintf(stderr, "Can NOT alloc memory!!!");
		return -1;
	}

	value = malloc(CKN_STR_MAX);
	if(!value)
	{
		fprintf(stderr, "Can NOT alloc memory!!!");
		free(tmp);
		return -1;
	}

	memset(tmp, 0, len);
	b = tmp;
	
	while(1)
	{
		p = _get_attr(p, name, sizeof(name), value, CKN_STR_MAX);

		if(name[0] != '\0')
		{
			//handle data
			_convert_data(name, value, CKN_STR_MAX);

			//write data in the new buffer
			if(value)
				tmp_len = snprintf(b, len - (b - tmp), "%s=%s", name, value);
			else
				tmp_len = snprintf(b, len - (b - tmp), "%s=", name);
			
			b += (tmp_len + 1);	//Add NULL at the end of the value
		}
		else
			break;
	}

	memcpy(buf, tmp, len);

	free(tmp);
	free(value);
	return 0;
}
#endif

unsigned char get_rand()
{
	unsigned char buf[1];
	FILE *fp;

	fp = fopen("/dev/urandom", "r");
	if (fp == NULL) {
#ifdef ASUS_DEBUG
		fprintf(stderr, "Could not open /dev/urandom.\n");
#endif
		return 0;
	}
	fread(buf, 1, 1, fp);
	fclose(fp);

	return buf[0];
}

#ifdef RTCONFIG_NVRAM_ENCRYPT
static int nvram_dec_all(char* buf_ap, char* buf)
{
	struct nvram_tuple *t;
	extern struct nvram_tuple router_defaults[];
	char *ptr, *item, *value;
	char name[128], nv[65535];
	int len;
	char output[NVRAM_ENC_MAXLEN];
	memset(output, 0, sizeof(output));

	if (!buf_ap || !buf)
		return -1;

	ptr = buf_ap;

	for (item = buf; *item; item += strlen(item) + 1) {
		value = strchr(item, '=');
		if (!value)
			continue;
		len = value - item;
		if (len < 0 || len > sizeof(name) - 1)
			continue;

		strncpy(name, item, len);
		name[len] = '\0';
		value++;

		for (t = router_defaults; t->name; t++)
		{
			if (strcmp(name, t->name) == 0 && t->enc == 1) {
				dec_nvram(t->name, value, output);
				value = output;
			}
		}

		snprintf(nv, sizeof(nv), "%s=%s", name, value);
		ptr = stpcpy(ptr, nv) + 1;
#ifdef ASUS_DEBUG
		puts(nv);
#endif
	}

    return 0;
}
#endif

int nvram_save_new(int *argc, char ***p_argv)
{
	FILE *fp;
	char *name, *file;
	char *buf, *nbuf;
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char *dbuf;
#endif
	unsigned long count, filelen, i;
	unsigned char rand = 0, temp;

	ADVANCE_ARGCV(*argc,*p_argv,1);
	file=(ARG_VAL?ARG_VAL:"/data/nvramdefault_asus.txt");
	if ((fp = fopen(file, "w")) == NULL) return -1;

	nbuf=malloc(MAX_NVRAM_SPACE);
	if(!nbuf) {
		fprintf(stderr,"Could not allocate memory\n");
		return -1;
	}
#ifdef RTCONFIG_JFFS_NVRAM
	nvram_getall(nbuf, MAX_NVRAM_SPACE );
#else
	wlcsm_nvram_getall(nbuf, MAX_NVRAM_SPACE);
#endif

#ifdef RTCONFIG_NVRAM_ENCRYPT
	dbuf=malloc(MAX_NVRAM_SPACE);
	if(!dbuf){
		fprintf(stderr,"Could not allocate memory\n");
		return -1;
	}

	nvram_dec_all(dbuf, nbuf);
	buf = dbuf;
#else
	buf = nbuf;
#endif

	count = 0;
	for (name = buf; *name; name += strlen(name) + 1)
	{
#ifdef ASUS_DEBUG
		puts(name);
#endif
		count = count + strlen(name) + 1;
	}

	filelen = count + (1024 - count % 1024);
	rand = get_rand() % 30;
#ifdef ASUS_DEBUG
	fprintf(stderr, "random number: %x\n", rand);
#endif
	fwrite(PROFILE_HEADER_NEW, 1, 4, fp);
	fwrite(&filelen, 1, 3, fp);
	fwrite(&rand, 1, 1, fp);
#ifdef ASUS_DEBUG
	for (i = 0; i < 4; i++)
	{
		fprintf(stderr, "%2x ", PROFILE_HEADER_NEW[i]);
	}
	for (i = 0; i < 3; i++)
	{
		fprintf(stderr, "%2x ", ((char *)&filelen)[i]);
	}
	fprintf(stderr, "%2x ", ((char *)&rand)[0]);
#endif
	for (i = 0; i < count; i++)
	{
		if (buf[i] == 0x0)
			buf[i] = 0xfd + get_rand() % 3;
		else
			buf[i] = 0xff - buf[i] + rand;
	}
	fwrite(buf, 1, count, fp);
#ifdef ASUS_DEBUG
	for (i = 0; i < count; i++)
	{
		if (i % 16 == 0) fprintf(stderr, "\n");
		fprintf(stderr, "%2x ", (unsigned char) buf[i]);
	}
#endif
	for (i = count; i < filelen; i++)
	{
		temp = 0xfd + get_rand() % 3;
		fwrite(&temp, 1, 1, fp);
#ifdef ASUS_DEBUG
		if (i % 16 == 0) fprintf(stderr, "\n");
		fprintf(stderr, "%2x ", (unsigned char) temp);
#endif
	}

	fclose(fp);
	free(buf);
#ifdef RTCONFIG_NVRAM_ENCRYPT
	free(nbuf);
#endif
	return 0;
}

int issyspara(char *p)
{
	struct nvram_tuple *t;
	extern struct nvram_tuple router_defaults[];

	if ((strstr(p, "wl") && strncmp(p+3, "_failed", 7)) || strstr(p, "wan") || strstr(p, "lan")
		|| strstr(p, "vpn_server") || strstr(p, "vpn_client")
	)
		return 1;

	for (t = router_defaults; t->name; t++)
	{
		if (strstr(p, t->name))
		break;

	}

	if (t->name) return 1;
	else return 0;
}

int nvram_restore_new(int *argc, char ***p_argv)
{
	FILE *fp;
	char *file, header[8], *p, *v;
	char *buf;
	unsigned long count, filelen, *filelenptr, i;
	unsigned char rand, *randptr;

	ADVANCE_ARGCV(*argc,*p_argv,1);
	file=(ARG_VAL?ARG_VAL:"/data/nvramdefault_asus.txt");
	if ((fp = fopen(file, "r+")) == NULL) return -1;

	buf=malloc(MAX_NVRAM_SPACE);
	if(!buf) {
		fprintf(stderr,"Could not allocate memory\n");
		return -1;
	}

	count = fread(header, 1, 8, fp);
	if (count>=8 && strncmp(header, PROFILE_HEADER, 4) == 0)
	{
		filelenptr = (unsigned long *)(header + 4);
#ifdef ASUS_DEBUG
		fprintf(stderr, "restoring original text cfg of length %x\n", *filelenptr);
#endif
		fread(buf, 1, *filelenptr, fp);
	}
	else if (count>=8 && strncmp(header, PROFILE_HEADER_NEW, 4) == 0)
	{
		filelenptr = (unsigned long *)(header + 4);
		filelen = *filelenptr & 0xffffff;
#ifdef ASUS_DEBUG
		fprintf(stderr, "restoring non-text cfg of length %x\n", filelen);
#endif
		randptr = (unsigned char *)(header + 7);
		rand = *randptr;
#ifdef ASUS_DEBUG
		fprintf(stderr, "non-text cfg random number %x\n", rand);
#endif
		count = fread(buf, 1, filelen, fp);
#ifdef ASUS_DEBUG
		fprintf(stderr, "non-text cfg count %x\n", count);
#endif
		for (i = 0; i < count; i++)
		{
			if ((unsigned char) buf[i] > ( 0xfd - 0x1)){
				/* e.g.: to skip the case: 0x61 0x62 0x63 0x00 0x00 0x61 0x62 0x63 */
				if(i > 0 && buf[i-1] != 0x0)
					buf[i] = 0x0;
			}
			else
				buf[i] = 0xff + rand - buf[i];
		}
#ifdef ASUS_DEBUG
		for (i = 0; i < count; i++)
		{
			if (i % 16 == 0) fprintf(stderr, "\n");
			fprintf(stderr, "%2x ", (unsigned char) buf[i]);
		}

		for (i = 0; i < count; i++)
		{
			if (i % 16 == 0) fprintf(stderr, "\n");
				fprintf(stderr, "%c", buf[i]);
		}
#endif
	}
	else
	{
		fclose(fp);
		return 0;
	}
	fclose(fp);

	p = buf;

	while (*p || p-buf<=count)
	{
#if 1
		/* e.g.: to skip the case: 00 2e 30 2e 32 38 00 ff 77 61 6e */
		if(*p == NULL || *p < 32 || *p > 127 ){
			p = p + 1;
			continue;
		}
#endif
		v = strchr(p, '=');

		if (v != NULL)
		{
			*v++ = '\0';

			if (issyspara(p))
				nvram_set(p, v);

			p = v + strlen(v) + 1;
		}
		else
		{
			nvram_unset(p);
			p = p + 1;
		}
	}

	free(buf);
	return 0;
}

int nvram_cmd_erase(int *argc, char ***p_argv) {
	ADVANCE_ARGCV(*argc,*p_argv,1);
	system("hnd-erase nvram");
	return 0;
}

int nvram_fb_save(int *argc, char ***p_argv)
{
	FILE *fp;
	char *name, *file;
	char *buf;
	unsigned long count, filelen, i;
	unsigned char rand = 0, temp;

	ADVANCE_ARGCV(*argc,*p_argv,1);
	file=(ARG_VAL?ARG_VAL:"/tmp/fb_save_nvram.txt");
	if ((fp = fopen(file, "w")) == NULL) return -1;

	char *tmpbuf = malloc(MAX_NVRAM_SPACE);
	if (!tmpbuf) {
		fprintf(stderr, "Can NOT alloc memory!!!");
		return 0;
	}
	buf=malloc(MAX_NVRAM_SPACE);
	if(!buf) {
		fprintf(stderr,"Could not allocate memory\n");
		return -1;
	}
#ifdef RTCONFIG_JFFS_NVRAM
	nvram_getall(buf, MAX_NVRAM_SPACE );
#else
	wlcsm_nvram_getall(buf, MAX_NVRAM_SPACE);
#endif
#ifdef RTCONFIG_NVRAM_ENCRYPT
            memset(tmpbuf, 0, MAX_NVRAM_SPACE);
            nvram_dec_all(tmpbuf, buf);
#else
            memcpy(tmpbuf, buf, MAX_NVRAM_SPACE);
#endif
	//_secure_conf(tmpbuf);
	_secure_conf(tmpbuf, MAX_NVRAM_SPACE);

	count = 0;
	for (name = tmpbuf; *name; name += strlen(name) + 1)
	{
		count = count + strlen(name) + 1;
	}

	filelen = count + (1024 - count % 1024);
	rand = get_rand() % 30;
	fwrite(PROFILE_HEADER_NEW, 1, 4, fp);
	fwrite(&filelen, 1, 3, fp);
	fwrite(&rand, 1, 1, fp);
	for (i = 0; i < count; i++)
	{
		if (tmpbuf[i] == 0x0)
			tmpbuf[i] = 0xfd + get_rand() % 3;
		else
			tmpbuf[i] = 0xff - tmpbuf[i] + rand;
	}
	fwrite(tmpbuf, 1, count, fp);
	for (i = count; i < filelen; i++)
	{
		temp = 0xfd + get_rand() % 3;
		fwrite(&temp, 1, 1, fp);
	}
	fclose(fp);
	free(buf);
	free(tmpbuf);
	return 0;
}

#ifdef DUMP_PREV_OOPS_MSG
int nvram_dump_prev_oops(int *argc, char ***p_argv)
{
	dump_prev_oops();
	return 0;
}
#endif

NVRAM_CMD_ITEM  nvram_cmd_items[]= {
    { "set", 		"set name with value",NVRAM_NORMAL_CMD,nvram_cmd_set,2},
    { "setflag",	"set bit value",NVRAM_NORMAL_CMD,nvram_cmd_setflag,1},
    { "unset", 		"remove nvram entry",NVRAM_NORMAL_CMD,nvram_cmd_unset,2},
    { "get", 		"get nvram value with name",NVRAM_NORMAL_CMD,nvram_cmd_get,2},

    { "getflag",		"get bit value",NVRAM_NORMAL_CMD,nvram_cmd_getflag,1 },
    /* for cmd alias to use colon to seperate them */
    { "show:dump:getall", "show all nvrams",NVRAM_NORMAL_CMD,nvram_cmd_getall,0},
    { "loadfile", 	"populate nvram value from files",NVRAM_NORMAL_CMD,nvram_cmd_loadfile,1},
    { "savefile", 	"save all nvram value to file",NVRAM_NORMAL_CMD,nvram_cmd_savefile,1},
#ifdef NAND_SYS
    { "restore_mfg", "restore mfg nvrams",NVRAM_HIDDEN_CMD,nvram_cmd_restore_mfg,0},
    { "kernelset","populate nvram from kernel configuration file",NVRAM_HIDDEN_CMD,nvram_cmd_kernelset,0},
    { "kset", 		"set name with value in kernel nvram",NVRAM_NORMAL_CMD,nvram_cmd_set,2},
    { "kunset", 	"remove nvram entry from kernel nvram",NVRAM_NORMAL_CMD,nvram_cmd_unset,1},
    { "kget", 		"get nvram value with name",NVRAM_NORMAL_CMD,nvram_cmd_get,2},
#endif
    { "commit","save nvram [optional] to restart wlan when following restart", NVRAM_NORMAL_CMD, nvram_cmd_commit,1},
    { "restart"		,"restart wlan",NVRAM_NORMAL_CMD, nvram_cmd_restart,1},
    { "kcommit", 	"only save knvrams",NVRAM_HIDDEN_CMD,nvram_cmd_kcommit,0},
    { "godefault", 	"reset wireless nvram to default and restart",NVRAM_HIDDEN_CMD,nvram_cmd_wl_godefault,1},
    { "save",		"save all nvram value to file", NVRAM_NORMAL_CMD,nvram_save_new,2},
    { "restore",	"restore all nvram value from file", NVRAM_NORMAL_CMD,nvram_restore_new,1},
    { "erase",		"erase nvram partition", NVRAM_NORMAL_CMD, nvram_cmd_erase,2},
    { "fb_save",	"save the romfile for feedback", NVRAM_NORMAL_CMD, nvram_fb_save,1},
#ifdef DUMP_PREV_OOPS_MSG
    { "dump_prev_oops",	"dump previous oops log", NVRAM_NORMAL_CMD, nvram_dump_prev_oops,0}
#endif
};

#define NVRAM_CMDS_NUM  (sizeof(nvram_cmd_items)/sizeof(NVRAM_CMD_ITEM))
#define FM_RESET "\033[0m"
#define FM_BOLDBLACK "\033[1m\033[30m"

void usage(char *cmd) {

    int i=0,j=0,line=0;
    if(cmd)
        printf("\n=====\nInput cmd:%s is not supported \n======\n",cmd);
    printf("\n======== NVRAM CMDS ========\n");
    for(i=0; i<NVRAM_CMDS_NUM; i++) {
        if(line%2) printf(FM_BOLDBLACK);
        else printf(FM_RESET);
        if(!nvram_cmd_items[i].is_hidden_cmd) {
            printf("[%s]\t",nvram_cmd_items[i].cmd);
            for(j=0; j<nvram_cmd_items[i].align_tab; j++)
                printf("\t");
            printf(": %s\n",nvram_cmd_items[i].prompt);
            line++;
        }
    }
    printf(FM_RESET);
    printf("============================\n");
    if(g_buf) free(g_buf);
    exit(0);
}


/* Main function to go through cmd list and execute cmd handler */

int main(int argc, char ** argv)
{
    int i=0,ret=0;
    char cmd_found=0;
    char *next;
    --argc;
    ++argv;
    if (!*argv) {
        usage(NULL);
    } else {
        WLCSM_SET_TRACE("nvram");
        g_buf=malloc(MAX_NVRAM_SPACE);
        if(!g_buf) {
            fprintf(stderr,"Could not allocate memory\n");
            return -1;
        }
        do {
            for(i=0; i<NVRAM_CMDS_NUM; i++) {
                foreachcolon(g_line_buffer,nvram_cmd_items[i].cmd,next) {
                    if(!strncmp(g_line_buffer,*argv,strlen(g_line_buffer)+1)) {
                        cmd_found=1;
                    }
                }
                if(cmd_found) {
                    ret=nvram_cmd_items[i].func(&argc,&argv);
                    break;
                }
            }
            if(!cmd_found)
                usage(*argv);
            else
                cmd_found=0;
            if(!*argv) break;
            ADVANCE_ARGCV(argc,argv,1);
        } while(*argv);
    }
    free(g_buf);
    return ret;
}
