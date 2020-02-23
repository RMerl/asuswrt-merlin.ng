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
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>


#include "bcm_map_part.h"

static inline int get_led_number(char *filename, long *led_num)
{
	int ret=-1;
	
		
	if(strncmp(filename, "led", 3) == 0)
		ret=kstrtol(filename+3, 10, led_num);
return ret;
}


static int proc_led_hw_disable_write_string( struct file *file,
                                       const char *buffer,
                                       size_t count, loff_t *pos)
{
	long led_num=0;
	volatile unsigned int* hw_disable = (volatile unsigned int *)PDE_DATA(file_inode(file));

	//parent is between led0...led31
	if(get_led_number(file->f_path.dentry->d_parent->d_iname, &led_num) == 0)
	{
		led_num=1<<led_num;
		*hw_disable &= (unsigned int ) ~(led_num);
	}
	else
		count=-1;
return count; 
}

static int proc_set_bit( struct file *file,
                                       const char *buffer,
                                       size_t count, loff_t *pos)
{
	long led_num=0;
	volatile unsigned int* field = (volatile unsigned int *)PDE_DATA(file_inode(file));

	//parent is between led0...led31
	if(get_led_number(file->f_path.dentry->d_parent->d_iname, &led_num) == 0)
	{
		led_num=1<<led_num;
		*field |= (unsigned int ) (led_num);
	}
	else
		count=-1;
return count; 
}



static int proc_led_config_read_string( struct file *file,
                                       char __user *data,
                                       size_t count, loff_t *pos)
{
	int ret=0, len;
	volatile unsigned int* cfg_data = (volatile unsigned int *)PDE_DATA(file_inode(file));
	char user_value_str[11];// 0x + 8 nibbles

	len = sprintf(user_value_str, "%x\n", 
		      *cfg_data);
	if ( len > count)
		len=count; 
	if(copy_to_user(data, user_value_str, len) == 0)
	{	
		if(*pos == 0)
		{
			ret=len;
		}
		*pos=len;
	}
	else
	{
		ret=-EFAULT;
		*pos=-1;
	}

	return ret;
}


static int proc_led_config_write_string( struct file *file,
                                       const char *buffer,
                                       size_t count, loff_t *pos)
{
	volatile unsigned int *cfg_data = (volatile unsigned int * )PDE_DATA(file_inode(file));
	unsigned int user_value;
	char user_value_str[11];// 0x + 8 nibbles
	int len=0;

	len=sizeof(user_value_str);
	if(count < len)
		len=count;
	if(copy_from_user(user_value_str, buffer, len)) {
		return -EFAULT;
	}

	sscanf(user_value_str, "%x", &user_value);

	*cfg_data=user_value;

	return count;
}

static void  cleanup_procfs(void)
{
	int i=0;
	char path[64]="";

	remove_proc_entry("bcm_cled/swled_enable", NULL);
	remove_proc_entry("bcm_cled/hwled_enable", NULL);
	remove_proc_entry("bcm_cled/activate", NULL);
	for (i=0; i < 32; i++) {
	    sprintf(path, "bcm_cled/led%d", i);
	    remove_proc_entry(path, NULL);
	}
	remove_proc_entry("bcm_cled", NULL);

	printk(KERN_INFO "bcm_led removed\n");
	       
}

static int init_procfs(void)
{
	int i = 0,j;
	int rv = 0;
	char path[64]="";
	struct proc_dir_entry *root_dir=NULL, *led_dir=NULL;
	struct proc_dir_entry *cfg_file;
	static struct file_operations proc_fops = {
		.read = proc_led_config_read_string,
		.write = proc_led_config_write_string,
	};
	static struct file_operations sw_en_fops = {
		.write = proc_set_bit
	};
	static struct file_operations hw_dis_fops = {
		.write = proc_led_hw_disable_write_string
	};

	printk("init procfs\n");

	/* create directory */
	root_dir = proc_mkdir("bcm_cled", NULL);
	if(root_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	for(i=0; i < 32; i++)
	{
		/* create ledx directories */
	    	sprintf(path, "bcm_cled/led%d", i);
		led_dir = proc_mkdir(path, NULL);
		if(led_dir == NULL) {
			rv = -ENOMEM;
			goto out;
		}
		for(j=0;j<4;j++)
		{
			sprintf(path, "config%d", j);
			cfg_file = proc_create_data(path, S_IRUSR, led_dir, &proc_fops, (void*)&LED->LedCfg[i].config[j]);
			if(cfg_file == NULL) {
				rv = -ENOMEM;
				goto out;
			}
		}
		proc_create_data("swled_enable", S_IRUSR, led_dir, &sw_en_fops, (void*)&LED->SwData);
		proc_create_data("activate", S_IRUSR, led_dir, &sw_en_fops, (void*)&LED->ChnActive);
		proc_create_data("hwled_disable", S_IRUSR, led_dir, &hw_dis_fops, (void*)&LED->hWLedEn);

	}
	cfg_file = proc_create_data("activate", S_IRUSR, root_dir, &proc_fops, (void*)&LED->ChnActive);
	if(cfg_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	cfg_file = proc_create_data("swled_enable", S_IRUSR, root_dir, &proc_fops, (void*)&LED->SwData);
	if(cfg_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	cfg_file = proc_create_data("hwled_enable", S_IRUSR, root_dir, &proc_fops, (void*)&LED->hWLedEn);
	if(cfg_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	return rv;
out:
	printk("bcm_leds proc_fs creation error, Exiting...\n");
	cleanup_procfs();
	return rv;
}




static int __init bcm_cled_init(void)
{
	init_procfs();
	return 0;
}

static void __exit bcmcled_cleanup(void)
{
	cleanup_procfs();
}

module_init(bcm_cled_init);
module_exit(bcmcled_cleanup);
MODULE_LICENSE("Proprietary");
