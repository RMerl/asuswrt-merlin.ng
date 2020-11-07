/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
//#include <linux/devfs_fs_kernel.h>	
#include "bcmprocfs.h"
#include <linux/version.h>

#define VERSION "1.0"
#define BCMPROCFS_MAJOR 331

/* forward declarations for _fops */
static ssize_t bcm_read(struct file *file, char *buf, size_t count, loff_t *offset);
static ssize_t bcm_write(struct file *file, const char *buf, size_t count, loff_t *offset);
static int bcm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int bcm_open(struct inode *inode, struct file *file);
static int bcm_release(struct inode *inode, struct file *file);

#define STRINGLEN 32
#define MAXWAN 16

struct fb_data_t {
	char value0[STRINGLEN + 1];
	char value1[STRINGLEN + 1];
};


static struct proc_dir_entry *var_dir, *fyi_dir,*wan_dir,
	*ppp_dir[MAXWAN],
	*daemonstatus_file[MAXWAN],*showtimesecs_file[MAXWAN],*disconnreason_file[MAXWAN],*wanipaddr_file[MAXWAN],*servicename_file[MAXWAN],*status_file[MAXWAN],*wanup_file[MAXWAN],/**server_file[MAXWAN],*/ *pid_file[MAXWAN],*subnetmask_file[MAXWAN],*sessinfo_file[MAXWAN],*sys_dir,*dns_file,*gateway_file;


struct fb_data_t daemonstatus_data[MAXWAN], showtimesecs_data[MAXWAN], disconnreason_data[MAXWAN], wanipaddr_data[MAXWAN],servicename_data[MAXWAN];
struct fb_data_t status_data[MAXWAN],wanup_data[MAXWAN], server_data[MAXWAN], subnetmask_data[MAXWAN], sessinfo_data[MAXWAN], pid_data[MAXWAN];
struct fb_data_t dns_data,gateway_data;
static struct file_operations bcm_fops = {
	.owner		= THIS_MODULE,
	.read		= bcm_read,
	.write		= bcm_write,
	.ioctl		= bcm_ioctl,
	.open		= bcm_open,
	.release	= bcm_release
};

static int bcm_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	/* ln -s src dst */
	struct symlink buffer;
	struct proc_dir_entry *link;

	switch (cmd) {
		case CREATE_SYMLINK:
		copy_from_user(&buffer,(void *)arg,sizeof(struct symlink));
		// printk("buffer.src=%s buffer.dst=%s, size=%d \n",buffer.src,buffer.dst, sizeof(struct symlink));
		
		/* create symlink */
			
		link = proc_symlink(buffer.dst,wan_dir,buffer.src);
			if (link == NULL) {
			   printk("bcm_ioctl: proc_symlink failed \n");
			   return -ENOMEM;
			}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
			link->owner = THIS_MODULE;
#endif
			
			break;
		case CREATE_FILE:
			//copy_from_user(buffer,(char *)&arg,16);
			break;
		case RENAME_TELNETD:
			strncpy(current->comm,"telnetd",8);
			break;
		case RENAME_HTTPD:
			strncpy(current->comm,"httpd",6);
			break;
		case RENAME_SSHD:
			strncpy(current->comm,"sshd",5);
			break;
		case RENAME_SNMP:
			strncpy(current->comm,"snmpd",6);
			break;
		case RENAME_TR69C:
			strncpy(current->comm,"tr69c",6);
			break;
		case RENAME_CONSOLED:
			strncpy(current->comm,"consoled",9);
			break;
		case RENAME_TR64:
			strncpy(current->comm,"tr64",5);
			break;
		default: {
			printk("ioctl: no such command\n");
			return -ENOTTY;
		}
	}

	return 0;
}

static ssize_t bcm_read(struct file *file, char *buf, size_t count,
			  loff_t *offset)
{
    unsigned short minor;

    /* Select which minor device */
    minor = MINOR(file->f_dentry->d_inode->i_rdev);
	return 0;
}
static ssize_t bcm_write(struct file *file, const char *buf, 
				size_t count, loff_t *offset)
{
    unsigned short minor;

    /* Select which minor device */
    minor = MINOR(file->f_dentry->d_inode->i_rdev);
	return 0;
}

static int bcm_open(struct inode *inode, struct file *file)
{
	
	if (file->f_mode & FMODE_READ) {
		//printk("opened for reading\n");
	} else if (file->f_mode & FMODE_WRITE) {
		//printk("opened for writing \n");
	}

	//printk("major: %d minor: %d\n", MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	return 0;
}

static int bcm_release(struct inode *inode, struct file *file)
{
	//printk("bcm_release\n");
	return 0;
}




static int proc_read_dns_string(char *page, char **start,
			    off_t off, int count, 
			    int *eof, void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;
	
	/*
	len = sprintf(page, "%s\n%s\n", 
		      fb_data->value0,fb_data->value1);
		      */
	len = sprintf(page, "%s%s", 
		      fb_data->value0,fb_data->value1);

	return len;
}

static int proc_write_dns_string(struct file *file,
			     const char *buffer,
			     unsigned long count, 
			     void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;


	if(count > STRINGLEN)
		len = STRINGLEN;
	else
		len = count;
	/*
	printk("fb_data->value0[0]=0x%x \n",fb_data->value0[0]);
	printk("buffer[0]=0x%x \n",buffer[0]);
	*/
	if ( buffer[0] == 0xa ) { /* clean the entries */
		fb_data->value0[0]='\0';
		fb_data->value1[1]='\0';
		return len;
	}
	if (fb_data->value0[0] == '\0') {
	  if(copy_from_user(fb_data->value0, buffer, len)) {
		return -EFAULT;
	  }

	  fb_data->value0[len] = '\0';
	}
	else {
	 if(copy_from_user(fb_data->value1, buffer, len)) {
		return -EFAULT;
	 }

	 fb_data->value1[len] = '\0';
	}

	return len;
}
static int proc_read_string(char *page, char **start,
			    off_t off, int count, 
			    int *eof, void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;
	
	len = sprintf(page, "%s\n", 
		      fb_data->value0);

	return len;
}


static int proc_write_string(struct file *file,
			     const char *buffer,
			     unsigned long count, 
			     void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;

	if(count > STRINGLEN)
		len = STRINGLEN;
	else
		len = count;

	if(copy_from_user(fb_data->value0, buffer, len)) {
		return -EFAULT;
	}

	fb_data->value0[len] = '\0';

	return len;
}


static int init_procfs(void)
{
	int i = 0;
	int rv = 0;
	char path[64]="";

	/* create directory */
	var_dir = proc_mkdir("var", NULL);
	if(var_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	var_dir->owner = THIS_MODULE;
#endif		


	/* create fyi directory */
	fyi_dir = proc_mkdir("var/fyi", NULL);
	if(fyi_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	fyi_dir->owner = THIS_MODULE;
#endif
	
	sys_dir = proc_mkdir("var/fyi/sys", NULL);
	if(sys_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	sys_dir->owner = THIS_MODULE;
#endif

	dns_file = create_proc_entry("dns", 0644, sys_dir);
	if(dns_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	//strcpy(dns_data.value0, "nameserver 10.1.2.3");
	//strcpy(dns_data.value1, "nameserver 10.1.2.4");
	dns_file->data = &dns_data;
	dns_file->read_proc = proc_read_dns_string;
	dns_file->write_proc = proc_write_dns_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30 )
#else
	dns_file->owner = THIS_MODULE;
#endif

	gateway_file = create_proc_entry("gateway", 0644, sys_dir);
	if(gateway_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	strcpy(gateway_data.value0, "10.1.2.3");
	gateway_file->data = &gateway_data;
	gateway_file->read_proc = proc_read_string;
	gateway_file->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	gateway_file->owner = THIS_MODULE;
#endif

	/* create fyi directory */
	wan_dir = proc_mkdir("var/fyi/wan", NULL);
	if(wan_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	wan_dir->owner = THIS_MODULE;
#endif

	/* create ppp directory */
	for (i=0; i < MAXWAN; i++) {
	    sprintf(path, "var/fyi/wan/.ppp%d", i);
	    ppp_dir[i] = proc_mkdir(path, NULL);
	    if(ppp_dir[i] == NULL) {
		rv = -ENOMEM;
		goto out;
	    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	    ppp_dir[i]->owner = THIS_MODULE;
#endif
	}

#if 0
	/* create symlink */
	symlink = proc_symlink("ppp77", wan_dir, 
			       "ppp7");
	if(symlink == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	symlink->owner = THIS_MODULE;
#endif

	for (i=0; i < MAXWAN; i++) {

	daemonstatus_file[i] = create_proc_entry("daemonstatus", 0644, ppp_dir[i]);
	strcpy(daemonstatus_data[i].value0, "daemonstatus");
	daemonstatus_file[i]->data = &daemonstatus_data[i];
	daemonstatus_file[i]->read_proc = proc_read_string;
	daemonstatus_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	daemonstatus_file[i]->owner = THIS_MODULE;
#endif
//LGD_FOR_TR098
	showtimesecs_file[i] = create_proc_entry("showtimesecs", 0644, ppp_dir[i]);
	strcpy(showtimesecs_data[i].value0, "showtimesecs");
	showtimesecs_file[i]->data = &showtimesecs_data[i];
	showtimesecs_file[i]->read_proc = proc_read_string;
	showtimesecs_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	showtimesecs_file[i]->owner = THIS_MODULE;
#endif

	disconnreason_file[i] = create_proc_entry("disconnreason", 0644, ppp_dir[i]);
	strcpy(disconnreason_data[i].value0, "disconnreason");
	disconnreason_file[i]->data = &disconnreason_data[i];
	disconnreason_file[i]->read_proc = proc_read_string;
	disconnreason_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	disconnreason_file[i]->owner = THIS_MODULE;
#endif

	wanipaddr_file[i] = create_proc_entry("ipaddress", 0644, ppp_dir[i]);
	strcpy(wanipaddr_data[i].value0, "wanIpaddress");
	wanipaddr_file[i]->data = &wanipaddr_data[i];
	wanipaddr_file[i]->read_proc = proc_read_string;
	wanipaddr_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	wanipaddr_file[i]->owner = THIS_MODULE;
#endif

	servicename_file[i] = create_proc_entry("servicename", 0644, ppp_dir[i]);
	strcpy(servicename_data[i].value0, "servicename");
	servicename_file[i]->data = &servicename_data[i];
	servicename_file[i]->read_proc = proc_read_string;
	servicename_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	servicename_file[i]->owner = THIS_MODULE;
#endif

	status_file[i] = create_proc_entry("status", 0644, ppp_dir[i]);
	strcpy(status_data[i].value0, "status");
	status_file[i]->data = &status_data[i];
	status_file[i]->read_proc = proc_read_string;
	status_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	status_file[i]->owner = THIS_MODULE;
#endif

	wanup_file[i] = create_proc_entry("wanup", 0644, ppp_dir[i]);
	strcpy(wanup_data[i].value0, "wanup");
	wanup_file[i]->data = &wanup_data[i];
	wanup_file[i]->read_proc = proc_read_string;
	wanup_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	wanup_file[i]->owner = THIS_MODULE;
#endif
/*
	server_file[i] = create_proc_entry("server", 0644, ppp_dir[i]);
	strcpy(server_data[i].value0, "server");
	server_file[i]->data = &server_data[i];
	server_file[i]->read_proc = proc_read_string;
	server_file[i]->write_proc = proc_write_string;
	server_file[i]->owner = THIS_MODULE;
*/
	subnetmask_file[i] = create_proc_entry("subnetmask", 0644, ppp_dir[i]);
	strcpy(subnetmask_data[i].value0, "subnetmask");
	subnetmask_file[i]->data = &subnetmask_data[i];
	subnetmask_file[i]->read_proc = proc_read_string;
	subnetmask_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	subnetmask_file[i]->owner = THIS_MODULE;
#endif

	sessinfo_file[i] = create_proc_entry("sessinfo", 0644, ppp_dir[i]);
	strcpy(sessinfo_data[i].value0, "");
	sessinfo_file[i]->data = &sessinfo_data[i];
	sessinfo_file[i]->read_proc = proc_read_string;
	sessinfo_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	sessinfo_file[i]->owner = THIS_MODULE;
#endif

	pid_file[i] = create_proc_entry("pid", 0644, ppp_dir[i]);
	strcpy(pid_data[i].value0, "");
	pid_file[i]->data = &pid_data[i];
	pid_file[i]->read_proc = proc_read_string;
	pid_file[i]->write_proc = proc_write_string;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
	pid_file[i]->owner = THIS_MODULE;
#endif
	}


	//------------------------------------------------------------

	/* everything OK */
	printk(KERN_INFO "Broadcom BCMPROCFS %s%s initialized\n",
	       "v", VERSION);
	return 0;

out:
	return rv;
}


static void  cleanup_procfs(void)
{
	int i=0;
	char path[64]="";

	remove_proc_entry("var/fyi/sys", NULL);
	for (i=0; i < MAXWAN; i++) {
	    sprintf(path, "var/fyi/wan/.ppp%d", i);
	    remove_proc_entry(path, NULL);
	}
	remove_proc_entry("var/fyi/wan", NULL);
	remove_proc_entry("var/fyi", NULL);
	remove_proc_entry("var", NULL);

	printk(KERN_INFO "%s%s removed\n",
	       "v", VERSION);
}

static int __init bcmprocfs_init(void)
{
	int res;
	
	/* register device with kernel */
	res = register_chrdev(BCMPROCFS_MAJOR, "bcm", &bcm_fops);
	if (res) {
		printk("bcmprocfs_init: can't register device with kernel\n");
		return res;
	}
	init_procfs();
	return 0;
}

static void __exit bcmprocfs_cleanup(void)
{
        unregister_chrdev(BCMPROCFS_MAJOR, "bcm");
	cleanup_procfs();
}

module_init(bcmprocfs_init);
module_exit(bcmprocfs_cleanup);
MODULE_LICENSE("Proprietary");
MODULE_VERSION(VERSION);
