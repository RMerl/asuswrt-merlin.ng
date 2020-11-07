/*
<:copyright-BRCM:2015:GPL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 * BL Extra parms processor 
 */
#include <linux/types.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/of.h>

#include <board.h>

/*minimum payload length  of BRCM_EXTRA_KERN="<payload>" */
#define BLXPARMS_LEN_MIN(prim_token) (sizeof(char)*2 + sizeof((char)prim_token))
/* xparms parameter designator(keyword) */
#define BLXPARMS "BRCM_EXTRA_KERN="

#define BLXPARM_PROC "blxparms"

/*#define BLXPARMS_DEBUG*/
#ifdef BLXPARMS_DEBUG
#define xparms_print_dbg(parm,...) do { printk(KERN_DEBUG parm,__VA_ARGS__);}while(0);
#else
#define xparms_print_dbg(xargs,...) 
#endif

/* if defined  parsed bl parms data will be copied to
	to compiled time defined buffer otherwise, allocated
 */
/*   Procfs and processing */
static struct proc_dir_entry *bl_xparms_proc_fs;

static ssize_t bl_xparms_proc_read(struct file* file, char __user *buf,
				size_t size, loff_t *_pos)
{
 /* start by dragging the command into memory */
	char* rval = (char*)PDE_DATA(file_inode(file)); 
	return simple_read_from_buffer(buf,size,_pos, rval, strlen(rval)); 
} 

/* Procfs reflection */
struct file_operations bl_xparms_procfs_fops = {
        .read    = bl_xparms_proc_read,
};

/*main buffer to maintain parsed bl data for procfs access */
static unsigned char* bl_xparms_data;
/*Needed for iteration to add next tuple*/
static unsigned char* bl_xparms_data_ptr;
/* safety check*/
static unsigned char* bl_xparms_data_end;

static int bl_xparms_zalloc_buf(ssize_t size)
{
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
	static unsigned char rsrv_mem[CONFIG_BCM_CFE_XARGS_EARLY_SIZE];
	size = CONFIG_BCM_CFE_XARGS_EARLY_SIZE;
	bl_xparms_data = rsrv_mem;
#else
	bl_xparms_data = vzalloc(size);
	if (!bl_xparms_data) {
		printk(KERN_ERR "Unable to allocate buffer\n");
		return -ENOMEM;
	}
#endif
	bl_xparms_data_end = bl_xparms_data + size;
	bl_xparms_data_ptr = bl_xparms_data;
	return  0; 
}

static void bl_xparms_free_buf(void)
{
	if (bl_xparms_data) {
#ifndef CONFIG_BCM_CFE_XARGS_EARLY
		vfree(bl_xparms_data);
#endif /*CONFIG_BCM_CFE_XARGS_EARLY*/
		bl_xparms_data = bl_xparms_data_end = bl_xparms_data_ptr = NULL;
	}
}

/*
Returns pointer to the copied entity then sets 
xparms_data_ptr to the size of the copied + 1 
*/
static void* bl_xparms_copy_it(void* src, ssize_t size, unsigned char append_char)
{
	void* ptr = bl_xparms_data_ptr;
	if (bl_xparms_data_ptr+size+1 <= bl_xparms_data_end) { 	
		memcpy(bl_xparms_data_ptr, src, size);
		bl_xparms_data_ptr += size;
		*bl_xparms_data_ptr++ = append_char;
	}
	return ptr;
}

/* procfs initializer */
static int bl_xparms_procfs_add(struct proc_dir_entry *pentry, 
				char* lval, char* rval)
{
        if (!proc_create_data(lval, 0644, bl_xparms_proc_fs,
                                 &bl_xparms_procfs_fops, rval)) {
		printk(KERN_ERR "Unable to create BL entry");
		return -ENOMEM;
	}
	return	0; 
}

static int __init bl_xparms_add_to_proc(struct proc_dir_entry *pentry)
{
	unsigned char* p = bl_xparms_data,*lval,*rval;
	bl_xparms_proc_fs = proc_mkdir(BLXPARM_PROC, pentry);
	if (!bl_xparms_proc_fs) {
		return -EINVAL;
	}
	while(p < bl_xparms_data_ptr) {
		lval = p;
		rval = p + strlen(p) + sizeof((char)'\0');
		xparms_print_dbg("added lval=%s rval=%s\n",lval,rval);	
		bl_xparms_procfs_add(pentry, lval, rval);
		p = rval + strlen(rval) + sizeof((char)'\0');
	}
	return 0;
}

/* on success lval_beg/lval_end are valid pointers of lval  
*  	and returned pointer is at the secondary token or at the beginning of the string 
*  if failed rval_beg is NULL and returned pointer is at the last iterated char or 
*       at the beginning of the string 
*
*  Parses string of the following format:
*  <secondary_token of the beginning of the string is reached><contigious set of chars except for primary and secondary tokens>
*  Parse is ended if either secondary token is encountered or the beginning of
*  the string is reached (on success). Otherwise, if empty or equal primary token - failed   
*/
static __init char* bl_xparms_lvalue_lookup(char* str_beg, char* str, 
		char prim_token, 
		char sec_token, 
		char** lval_beg, 
		char** lval_end)
{
	char *end = NULL;	
	while (str_beg <= str) {
		if (*str == prim_token)
			break;
		
		if (!end) { /* end of the lval not yeat found*/
			if (*str != sec_token) {
				end = str;
			}
		} else if (*str == sec_token) {
			/*	end parsing since we only accept consequtive 
				sets of chars for valid lval	
			*/
			*lval_beg = str + 1;
			*lval_end = end;
		        break; 		
		} else if (str == str_beg) {
			
			*lval_beg = str;
			*lval_end = end;
			break;		
		}
		str--;
	}

	return str;
}

/* on success rval_beg/rval_end are valid pointers of rval 
*   and returned pointer is at the primary token 
*  if failed lval_beg is NULL and returned pointer is at the beginning of the string or
*   at the last iterated char
*
*  Parses string of the following format:
*  <primary_token><any char except primary token; beginning and ending spaces(secondary token) are ignored>
*/
static  __init char* bl_xparms_rvalue_lookup(char* str_beg, 
				char* str, 
				char prim_token, 
				char sec_token,
				char** rval_beg, 
				char** rval_end)
{
	char* beg = NULL,*end = NULL;
	while (str_beg <= str) {
		if (*str == prim_token) {
			if (end) { 
				if (!beg) {
					beg = str+1;
				} 
				*rval_beg = beg;
				*rval_end = end;
			}
			break;
		} else if (!end) {
				if (*str != sec_token) {
					end = str;
				}
			} else  if (*str != sec_token) {
				/* record last occurence of the non-primay 
					and non-secondary tokens 
				*/
					beg = str;
			}
		str--;
	}
	return str;
}

/*
* Stores/copies parsed lval/rval null-terminated tuples 
* into internal buffer (xparms_data)
* for procfs use
*/
static void __init bl_xparms_do(char* lval_beg, 
				char* lval_end ,
				char prim_token, 
				char* rval_beg, 
				char* rval_end) 
{
	/*Locate keyword where extra BCMPARAMETERS are stored*/
#ifdef BLXPARMS_DEBUG
	char* lval,*rval;
	lval = bl_xparms_copy_it(lval_beg, lval_end-lval_beg + sizeof((char)'\0'),'\0');
	rval = bl_xparms_copy_it(rval_beg, rval_end-rval_beg + sizeof((char)'\0'),'\0');
	xparms_print_dbg(" Parsed BL Parms [ %s %c %s ] \n",(char*)lval, prim_token, (char*)rval);
#else
	bl_xparms_copy_it(lval_beg, lval_end-lval_beg + sizeof((char)'\0'),'\0');
	bl_xparms_copy_it(rval_beg, rval_end-rval_beg + sizeof((char)'\0'),'\0');
#endif
}

/*	
	Input string is treated as a buffer of null terminated strings.	
	Func. searches for every occurence of the parm_id (BLXPARMS) then parses its payload. 
	On successful parse of lval/rval tuple do_xparms callback is called 
	The format of the xparm is expected to be as the following:
	<BLXPARMS><delimiter><payload><delimiter> BLXPARMS must be contigious string of non-space characters
	(predefined for this implementation)
	Payload is expected to be a set of triples such as :
	<secondary token or beginning of the string><lval><primary_token><rval><secondary_token>...<secondary_token><rval><primary_token><lval>
	example:
	BRCM_EXTRA_KERN="parm1=2 parm2 = 3 sl 1"
	parm_delim: "
	sec_token: space
	prim_token =
	lval=parm1 rval=2
	lval=parm2 rval=3 sl 1  
*/
static int __init bl_xparms_lookup(const char* parm_id,
				const char* str, unsigned int size, 
				char prim_token, char sec_token, char parm_delim,
				void (*do_xparms)(char*, char* , char , char*, char*))
{
	const char *substr = str;
	char *xparms = NULL, *xparms_end = NULL, *lval_beg = NULL, 
			*lval_end = NULL, *rval_beg = NULL, *rval_end = NULL;
	unsigned int xparms_len = 0;
	int rc = 0;
	/* if at least one parm tuple was added then true*/
	bool parm_added = false;
	rc = bl_xparms_zalloc_buf(size);
	if (rc) {
		goto done; 
	}
	while(substr < str+size) {
		xparms = strstr(substr, parm_id);
		if (xparms && do_xparms) {
			xparms_len =  strlen(xparms); 
		 	if (xparms_len >= strlen(parm_id) + sizeof(parm_delim)*2 + BLXPARMS_LEN_MIN(prim_token)) {
		        	xparms_end = xparms + xparms_len - 1;
				xparms += strlen(parm_id); 
				if ((*(xparms_end)&(*xparms)) != parm_delim) {
					printk(KERN_ERR "Invalid format string  - must be quoted . %c %c .\n", *(xparms_end), *xparms); 
					break;
				}
				xparms++; 
				xparms_end--; 
				while(xparms <= xparms_end) {
					xparms_end = bl_xparms_rvalue_lookup(xparms, xparms_end, 
								prim_token, sec_token, &rval_beg, &rval_end);
					if (rval_beg) {
						xparms_end  = bl_xparms_lvalue_lookup(xparms, xparms_end-1, 
								prim_token, sec_token, &lval_beg, &lval_end);
						if (lval_beg) {
							parm_added = true;
							bl_xparms_do(lval_beg, lval_end, prim_token, rval_beg, rval_end);
						}
					}
					rval_beg = NULL;
					lval_beg = NULL;
					xparms_end--;
				}
			} 
		}
		substr += strlen(substr)+1;
	}
	if (!parm_added) {
		bl_xparms_free_buf();
	}
done:
	return 0;
}

void __init bl_xparms_setup(const unsigned char* blparms, unsigned int size)
{
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
	bl_xparms_lookup(BLXPARMS, blparms, size, '=',' ','"', bl_xparms_do);
#endif
}

#ifdef CONFIG_BCM_CFE_XARGS_EARLY
EXPORT_SYMBOL(bl_xparms_setup);
#endif

static int __init bl_xparms_setup_proc(struct proc_dir_entry *pentry)
{
#ifndef CONFIG_BCM_CFE_XARGS_EARLY
	const unsigned char* blparms = NULL;
	unsigned int size = 0;

	blparms = bcm_get_blparms();
	size = bcm_get_blparms_size();
	if (blparms) {
		bl_xparms_lookup(BLXPARMS, blparms, size, '=',' ','"', bl_xparms_do);
	}
#endif
	return bl_xparms_add_to_proc(pentry);
}

#if defined(CONFIG_ARM64) || defined(CONFIG_ARM)

static void* proc_entry_deferred;

int __init bcm_blxparms_init(struct proc_dir_entry *pentry)
{
	proc_entry_deferred = pentry;
	return 0;
}

static int __init bcm_blxparms_init_late(void)
{
	//in case if the /proc/brcm entry is missing create one
	if (!proc_entry_deferred && (proc_entry_deferred = proc_mkdir("brcm", NULL)) == NULL) {
		printk(KERN_ERR "Proc entry is empty\n");
		return -1;
	}
	return 	bl_xparms_setup_proc((struct proc_dir_entry*)proc_entry_deferred);
}

late_initcall(bcm_blxparms_init_late);

#else

int __init bcm_blxparms_init(struct proc_dir_entry *pentry)
{
	return 	bl_xparms_setup_proc(pentry);
}

#endif

EXPORT_SYMBOL(bcm_blxparms_init);
