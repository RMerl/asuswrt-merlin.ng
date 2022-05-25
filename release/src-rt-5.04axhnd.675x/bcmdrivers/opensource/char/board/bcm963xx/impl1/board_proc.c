/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mtd/mtd.h>

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>

#include "board_proc.h"
#include "board_image.h"
#include "board_wl.h"
#include "bcm_mbox_map.h"

#if defined(CONFIG_BCM96858)
int proc_show_rdp_mem( struct file *file, char __user *buf, size_t len, loff_t *pos);
#endif

#if defined (WIRELESS)
static ssize_t proc_get_wl_nandmanufacture(struct file * file, char * buff, size_t len, loff_t *offset);
#ifdef BUILD_NAND
static ssize_t proc_get_wl_mtdname(struct file * file, char * buff, size_t len, loff_t *offset);
#endif
#endif

static ssize_t proc_get_param_string(struct file *, char *, size_t, loff_t *);
static ssize_t proc_set_param(struct file *, const char *, size_t, loff_t *);
static ssize_t proc_set_led(struct file *, const char *, size_t, loff_t *);

int add_proc_files(void);
int del_proc_files(void);

static ssize_t __proc_get_socinfo(char *buf, int cnt)
{
    char socname[15] = {0};
    int i;
    int n=0;

    kerSysGetChipName( socname, strlen(socname));

    for( i=0; i< strlen(socname); i++ )
    {
        if(socname[i] == '_')
        {
            socname[i]='\0';
            break;
        }
    }
            
    n += sprintf(buf,   "SoC Name        :BCM%s\n", socname);
    n += sprintf(buf+n, "Revision        :%s\n", &socname[i+1]);

    return n;
}
static ssize_t __proc_get_wan_type(char *buf)
{
    int n = 0;

#ifdef CONFIG_DT_SUPPORT_ONLY
    n=sprintf(buf, "none");
    return n;
#else
    unsigned int wan_type = 0, t;
    int i, j, len = 0;

    BpGetOpticalWan(&wan_type);
    if (wan_type == BP_NOT_DEFINED)
    {
        n=sprintf(buf, "none");
        return n;
    }

    for (i = 0, j = 0; wan_type; i++)
    {
        t = wan_type & (1 << i);
        if (!t)
            continue;

        wan_type &= ~(1 << i);
        if (j++)
        {
            sprintf(buf + len, "\n");
            len++;
        }

        switch (t)
        {
        case BP_OPTICAL_WAN_GPON:
            n+=sprintf(buf + len, "gpon");
            break;
        case BP_OPTICAL_WAN_EPON:
            n+=sprintf(buf + len, "epon");
            break;
        case BP_OPTICAL_WAN_AE:
            n+=sprintf(buf + len, "ae");
            break;
        default:
            n+=sprintf(buf + len, "unknown");
            break;
        }
        len += n;
    }

    return len;
#endif
}

static ssize_t proc_get_socinfo( struct file *file,
                                       char __user *buf,
                                       size_t len, loff_t *pos)
{
    char *kbuf;
    int ret=0;
    if(*pos == 0)
    {
        if (!(kbuf = kmalloc(len, GFP_KERNEL)))
            return -EFAULT;
        
        *pos=__proc_get_socinfo(kbuf, len);
        if(likely(*pos != 0)) //something went wrong
            ret = (copy_to_user(buf, kbuf, *pos)) ? -EFAULT : *pos;
        kfree(kbuf);
    }
   
    return ret;
}
static ssize_t proc_get_wan_type( struct file *file,
                                       char __user *buf,
                                       size_t len, loff_t *pos)
{
    char *kbuf;
    int ret=0;
    if(*pos == 0)
    {
        if (!(kbuf = kmalloc(len, GFP_KERNEL)))
            return -EFAULT;
       *pos=__proc_get_wan_type(kbuf);
       if(likely(*pos != 0)) //something went wrong
           ret = (copy_to_user(buf, kbuf, *pos)) ? -EFAULT : *pos;
       kfree(kbuf);
    }
    return ret;
}

static union query_set_str {
    char nvram_query_str[256];
    char uboot_set_str[1024];
}query_set_str;

static ssize_t proc_get_query(struct file *file,
				char __user *buf,
				size_t len, loff_t *pos)
{
    int ret=0;
    char *value=NULL;

    if(*pos == 0)
    {

        value = (unsigned char *)kmalloc(len+1, GFP_KERNEL);
        if(value != NULL)
        {
            memset(value, '\0', len+1);
            ret=envram_get_locked((char*)query_set_str.nvram_query_str, value, len);
            if(copy_to_user(buf, value, len) == 0)
            {
                *pos=strlen(value);
            }
            else
            {
                ret=-EFAULT;
                *pos=-1;
            }
            kfree(value);
        }
        else
            ret=-EFAULT;
    }
    return ret;

}

static ssize_t proc_set_query(struct file *file, const char *buff, size_t len, loff_t *offset)
{
char *temp_str=NULL;

    memset(query_set_str.nvram_query_str, '\0', sizeof(query_set_str.nvram_query_str));

    if ((len > sizeof(query_set_str.nvram_query_str)-1) || (copy_from_user(query_set_str.nvram_query_str, buff, len) != 0))
        return -EFAULT;
    if((temp_str=strchr(query_set_str.nvram_query_str, '\n')) != NULL)
    {
        *temp_str='\0';
    }
    if((temp_str=strchr(query_set_str.nvram_query_str, '\r')) != NULL)
    {
        *temp_str='\0';
    }
    return len;
}

static ssize_t proc_set_set(struct file *file, const char *buff, size_t len, loff_t *offset)
{
char *temp_str=NULL;
int r=-1;

    if(*offset != 0)
        return 0;
    memset(query_set_str.uboot_set_str, '\0', sizeof(query_set_str.uboot_set_str));

    if ((len > sizeof(query_set_str.uboot_set_str)-1) || (copy_from_user(query_set_str.uboot_set_str, buff, len) != 0))
        return -EFAULT;
    if((temp_str=strchr(query_set_str.uboot_set_str, '\n')) != NULL)
    {
        *temp_str='\0';
    }
    if((temp_str=strchr(query_set_str.uboot_set_str, '\r')) != NULL)
    {
        *temp_str='\0';
    }
    r=envram_add_locked((char*)query_set_str.uboot_set_str);
    if (r == 0)
    {
        sync_nvram_with_flash();
        r=len;
        *offset=len;
    }
    
    return r;
}

static ssize_t __proc_set_param(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    char input[32];

    int i = 0;
    int r = cnt;

    memset(input, '\0', sizeof(input));

    if ((cnt > 32) || (copy_from_user(input, buf, cnt) != 0))
        return -EFAULT;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]) && input[i] != ':')
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    r=envram_set_locked((char*)data, input);
    sync_nvram_with_flash();
    if(r != -1)
        r=cnt;
    return r;
}
ssize_t __proc_get_param_string(char *page, int cnt, void *data)
{
    int r = 0;
    char *value=NULL;

    if (data == NULL)
        return 0;

    value = (unsigned char *)kmalloc(cnt, GFP_KERNEL);
    if(value != NULL)
    {
        r=envram_get_locked((char*)data, value, cnt);
        if(copy_to_user(page, value, r > cnt ? cnt:r))
        {
            r=-EFAULT;
        }
        kfree(value);
    }
    return (r < cnt && r > 0)? r: 0;
}
static ssize_t __proc_set_led(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    // char leddata[16];
    unsigned int leddata;
    char input[32];
    int i;

    if (cnt > 31)
        cnt = 31;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;


    for (i = 0; i < cnt; i ++)
    {
        if (!isxdigit(input[i]))
        {
            input[i] = 0;
        }
    }
    input[i] = 0;

    if (0 != kstrtouint(input, 16, &leddata)) 
        return -EFAULT;

    kerSysLedCtrl ((leddata & 0xff00) >> 8, leddata & 0xff);
    return cnt;
}

static ssize_t proc_get_param_string(struct file * file, char * buff, size_t len, loff_t *offset)
{

    int ret=0;
    if(*offset == 0)
    {
        *offset =__proc_get_param_string(buff, len, PDE_DATA(file_inode(file))); 
        if(likely(*offset != 0)) //something went wrong
            ret=*offset;
    }
    return ret;
}
static ssize_t proc_set_param(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    return __proc_set_param(file,buff,len,PDE_DATA(file_inode(file)));
}
static ssize_t proc_set_led(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    int ret=-1;
    if(*offset == 0)
    {
        *offset=__proc_set_led(file, buff, len, PDE_DATA(file_inode(file)));
        if(likely(*offset != 0)) //something went wrong
            ret=*offset;
    }    
    return ret;

}

#if defined(CONFIG_BCM_FAILSAFE_BOOT)
static ssize_t proc_set_failsafe_boot(struct file *f, const char *buf, size_t cnt, loff_t *data)
{
    char __in[8] = {0};
    unsigned int complete = 0;
   
    if (cnt < 1 ||  cnt > 4 || 
        copy_from_user(__in, buf, 1)) {
        return -EFAULT;
    }

    if (sscanf(__in, "%u", &complete) != 1) {
        printk("\nError format, it is as:\n");
        return -EFAULT;
    }
    if (complete) {
        if (BCM_MBOX1_STATUS()) {
            /* Done tracing failsafe*/
            unsigned int st = BCM_MBOX_MSG1_GET();
            BCM_BLR_BOOT_STATE_SET_ERR(st, 0x0);
            BCM_MBOX_MSG1_SET(st);
        }
    }
    return cnt;
}

static ssize_t proc_get_failsafe_boot(struct file *file, char __user *buff, size_t len, loff_t *offset)
{
    char kbuf[32];
    unsigned int st;

    if(*offset != 0)
        return 0;
    st = BCM_BLR_BOOT_STATE_GET_INFO(BCM_MBOX_MSG1_GET());
     
    *offset = sprintf(kbuf, "BOOTED  %s IMAGE\n",(st&CFE_BOOT_INFO_SECONDARY)?"FAILSAFE PREVIOUS":"ACTIVE");
    return (copy_to_user(buff, kbuf, *offset)) ? -EFAULT : *offset;
}

#endif

#if defined(WIRELESS)
/*  for higher version 4.1 kernel */
static ssize_t proc_get_wl_nandmanufacture(struct file * file, char * buff, size_t len, loff_t *pos)
{
    char kbuf[16];
    ssize_t ret=0;
    if(*pos == 0)
    {
        (*pos) = sprintf(kbuf, "%d", _get_wl_nandmanufacture());
        if(likely(*pos != 0))
            ret = (copy_to_user(buff, kbuf, *pos)) ? -EFAULT : *pos;
    }
    return ret;
}

#ifdef BUILD_NAND
static ssize_t proc_get_wl_mtdname(struct file * file, char * buff, size_t len, loff_t *pos)
{
    char kbuf[16];
    ssize_t ret=0;
    if(*pos == 0)
    {
        struct mtd_info *mtd = get_mtd_device_nm(WLAN_MFG_PARTITION_NAME);
        if( !IS_ERR_OR_NULL(mtd) ) {
           (*pos) = sprintf(kbuf, "mtd%d",mtd->index );
           if(likely(*pos != 0)) 
               ret = (copy_to_user(buff, kbuf, *pos)) ? -EFAULT : *pos;
         }
    }
    return ret;
}
#endif
#endif

#ifdef WIRELESS
   static struct file_operations wl_get_nandmanufacture_proc = {
       .read=proc_get_wl_nandmanufacture,
       .write=NULL,
    };
#ifdef BUILD_NAND
   static struct file_operations wl_get_mtdname_proc = {
       .read=proc_get_wl_mtdname,
       .write=NULL,
    };
#endif
#endif
   static struct file_operations base_mac_add_proc = {
       .read=proc_get_param_string,
       .write=proc_set_param,
    };
   static struct file_operations bootline_proc = {
       .read=proc_get_param_string,
       .write=NULL,
    };
    static struct file_operations led_proc = {
       .write=proc_set_led,
    };
    static struct file_operations supp_optical_wan_types_proc = {
       .read=proc_get_wan_type,
    };
#if defined(CONFIG_BCM_FAILSAFE_BOOT)
    static struct file_operations failsafe_boot_proc = {
       .write = proc_set_failsafe_boot,
       .read = proc_get_failsafe_boot,
    };
#endif
    static struct file_operations boardid_proc = {
       .read=proc_get_param_string,
    };
    static struct file_operations socinfo_proc = {
       .read=proc_get_socinfo,
    };
    static struct file_operations base_query_proc = {
       .read=proc_get_query,
       .write=proc_set_query
    };
    static struct file_operations base_set_proc = {
       .write=proc_set_set
    };

int add_proc_files(void)
{
#define offset(type, elem) ((size_t)&((type *)0)->elem)

    char *mi=NVRAM_UCABASEMACADDR;
    char *bootline=NVRAM_SZBOOTLINE;
    char *boardid=NVRAM_SZBOARDID;

    struct proc_dir_entry *p0;
    struct proc_dir_entry *p1;
    struct proc_dir_entry *p2;
    struct proc_dir_entry *p4;
    struct proc_dir_entry *p5;

    p0 = proc_mkdir("nvram", NULL);

    if (p0 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

#if defined(WIRELESS)
    p1 = proc_create("wl_nand_manufacturer", S_IRUSR, p0,&wl_get_nandmanufacture_proc);
    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

#ifdef BUILD_NAND

    p1 = proc_create("wl_nand_mtdname", S_IRUSR, p0,&wl_get_mtdname_proc);
    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }
#endif
#endif
     p1 = proc_create_data("BaseMacAddr", S_IRUSR, p0, &base_mac_add_proc, mi);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

     p1 = proc_create_data("bootline", S_IRUSR, p0, &bootline_proc, bootline);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1 = proc_create("led", S_IWUSR | S_IRUSR, NULL, &led_proc);
    if (p1 == NULL)
        return -1;


    p2 = proc_create("supported_optical_wan_types", S_IRUSR, p0, &supp_optical_wan_types_proc);
    if (p2 == NULL)
        return -1;

    p4 = proc_create_data("boardid", S_IRUSR, p0, &boardid_proc, boardid);
    if (p4 == NULL)
        return -1;

    p5 = proc_create("socinfo", S_IRUSR, NULL, &socinfo_proc);
    if (p5 == NULL)
        return -1;

    p1 = proc_create("query", S_IRUSR, p0, &base_query_proc);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }
    p1 = proc_create("set", S_IRUSR, p0, &base_set_proc);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

#if defined(CONFIG_BCM_FAILSAFE_BOOT)
    p0 = proc_mkdir("boot", NULL);
    if (p0 == NULL) {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }
    p2 = proc_create("failsafe", S_IRUSR|S_IWUSR, p0, &failsafe_boot_proc );
    if (p2 == NULL) {
        printk("add_proc_files: failed to create failsafe proc file!\n");
        return -1;
    }
#endif
    return 0;
}

int del_proc_files(void)
{
    remove_proc_entry("nvram", NULL);
    remove_proc_entry("led", NULL);

#if defined(CONFIG_BCM_FAILSAFE_BOOT)
    remove_proc_entry("failsafe", NULL);
#endif
    return 0;
}
