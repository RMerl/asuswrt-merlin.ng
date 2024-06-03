/*
<:copyright-BRCM:2017:DUAL/GPL:standard

   Copyright (c) 2017 Broadcom
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

/*
 ***************************************************************************
 * File Name  : bcm_nvram_data_impl.c
 *
 * Description: This file contains the nvram_data APIs for bcm63xx board.
 *
 *
 ***************************************************************************/

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/crc32.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <bcmtypes.h>
#include <board.h>

#include "board_nvram.h"

static char *uboot_env=NULL;
static int uboot_env_length=UBOOT_MAX_ENV_LEN;
static int uboot_env_max_length=UBOOT_MAX_ENV_LEN;
static struct proc_dir_entry *proc_directory=NULL;
static struct proc_dir_entry *raw_proc_entry=NULL;
#define RAW_BINARY_FILE "raw"

static ssize_t proc_get_raw(struct file * file, char * buff, size_t len, loff_t *offset)
{

	if (uboot_env == NULL && *offset < uboot_env_max_length+UBOOT_HEADER_LEN)
		return 0;

	len=len < (uboot_env_max_length+UBOOT_HEADER_LEN-*offset)?len:(uboot_env_max_length+UBOOT_HEADER_LEN-*offset);
	if(copy_to_user(buff, uboot_env+*offset, len))
	{
		len=-EFAULT;
	}
	*offset += len;
	return len;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
static struct file_operations raw_proc = {
	.read=proc_get_raw
};
#else
static struct proc_ops raw_proc = {
	.proc_read=proc_get_raw
};
#endif

int get_uboot_env_area_size(void)
{
	return uboot_env_max_length;
}

unsigned char *get_uboot_env_area(int current_size)
{
	if(uboot_env == NULL)
	{
		uboot_env_length=uboot_env_max_length=current_size;
	        uboot_env = kmalloc(current_size+UBOOT_HEADER_LEN, GFP_ATOMIC);
	}

	return uboot_env;

}

static int get_actual_uboot_env_length(char *uboot_env, int max_len)
{
    int len=0;
    while( len+1 < max_len)
    {
        if(uboot_env[len] == '\0' && uboot_env[len+1] == '\0')
            break;
        len++;
    }

    return len;
}

typedef struct tree_node
{
	char *var;
	char *var_in_env;
	char *val;
	int  var_len;
	int  val_len;
	int replaced;
	union
	{
		struct tree_node *left;// used for the tree structure
		struct tree_node *next;// used for linked list in the iterative functions
	};
	struct tree_node *right;

	struct proc_dir_entry *proc_entry;

}tree_node;

tree_node *head=NULL;


static ssize_t proc_get_env_param(struct file * file, char * buff, size_t len, loff_t *offset)
{
	size_t len_to_copy=0;
	char *value;
	tree_node *node = (tree_node*) PDE_DATA(file_inode(file));

	if (node == NULL || *offset >= strlen(node->val))
		return 0;

	value = (unsigned char *)kmalloc(len, GFP_KERNEL);
	if(value != NULL)
	{
		len_to_copy=strlen(node->val)-*offset < len ? strlen(node->val)-*offset:len;
		strncpy((char*)value, node->val+*offset, len_to_copy);
		*offset+=len;
		if(copy_to_user(buff, value, len_to_copy))
		{
			len=-EFAULT;
		}
		kfree(value);
	}
	return len_to_copy;
}

static tree_node *new_node(char *var, int var_len, char *val, int val_len)
{
tree_node *n=NULL;

	n=kmalloc(sizeof(tree_node), GFP_KERNEL);
	if(n)
	{
		n->var=kmalloc(var_len+1, GFP_KERNEL);
		if(n->var != NULL)
		{
			n->var_in_env=var;
			memset(n->var, '\0', var_len+1);
			memcpy(n->var, var, var_len);
			n->var_len=var_len;
			n->val=val;
			n->val_len=val_len;
			n->replaced=0;
			n->left=n->right=NULL;
		}
		else
		{
			kfree(n);
			n=NULL;
		}
	}
	//printk("new node [%s] %d [%s], %d\n", var, var_len, val, val_len);
	return n;
}

static tree_node* insert(tree_node **head, char *var, int var_len, char *val, int val_len)
{
	tree_node *n=NULL;
	int r;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	static struct file_operations env_proc = {
		.read=proc_get_env_param,
		//.write=proc_set_param,
	};
#else
	static struct proc_ops env_proc = {
		.proc_read=proc_get_env_param,
		//.proc_write=proc_set_param,
	};
#endif

	if(head != NULL && var != NULL && val != NULL)
	{
		if(head[0] == NULL)
		{
			n=head[0]=new_node(var, var_len, val, val_len);
			proc_directory = proc_mkdir("environment", NULL);
			if(n != NULL && proc_directory != NULL) 
				n->proc_entry=proc_create_data(n->var, S_IRUSR, proc_directory, &env_proc, n);
		}
		else
		{
			n=head[0];
			while (n)
			{
				r=strncmp(n->var, var, var_len);
				if(r==0)
				{
					//replace
					n->var_in_env=var;
					n->val=val;
					n->replaced=1;
					break;
				}
				else if (r > 0)
				{
					if(n->left == NULL)
					{
						n->left=new_node(var, var_len, val, val_len);
						n=n->left;
						break;
					}
					n=n->left;
				}
				else
				{
					if(n->right == NULL)
					{
						n->right=new_node(var, var_len, val, val_len);
						n=n->right;
						break;
					}
					n=n->right;
				}
			}
			if(n != NULL && n->replaced == 0)
			{
				n->proc_entry=proc_create_data(n->var, S_IRUSR, proc_directory, &env_proc, n);
			}
		}
	}
return n;
}

static void print_tree(tree_node *head)
{
	printk("-------------------\n");
	if(head != NULL)
	{
		printk("head %px [%s] = > [%s] \n", head, head->var, head->val);
		if(head->left != NULL)
		{
			print_tree(head->left);
		}
		if(head->right != NULL)
		{
			print_tree(head->right);
		}
	}
	printk("-------------------\n");
}

#if 0 
/*
non-recursive function to free the entire tree holding uboot env 
*/
static void free_tree(tree_node *head)
{
	tree_node *pool=NULL, *pool_tail;
	tree_node *temp;

	while( head != NULL)
	{
		//we are done with left side of the subtree,free the node and go to right side
		if(head->left == NULL)
		{
			temp=head;
			head=head->right;
			kfree(temp->var);
			kfree(temp);
			
		}
		// move current to the pool to visit later and move head to the left node
		else
		{
			temp=head;
			head=head->left;
			temp->left=NULL;
			if( pool == NULL)
			{
				pool=pool_tail=temp;	
			}
			else
			{
				pool_tail->next=temp;
				pool_tail=temp;
			}
		}

		// we have reached the end fof the subtree, retrieve saved subtree from the pool
		if(head == NULL && pool != NULL)
		{
			head = pool;
			pool=pool->next;
			head->next=NULL;
		}
	} 
}
#endif

/*
	search for the variable in the tree structure
*/
char *find_var(tree_node *head, char *var, char var_length, char ** var_in_env)
{
	int r=0;
	char *value=NULL;
	tree_node *n=head;

	while (n)
	{
		r=strncmp(n->var, var, var_length);
		if(r==0)
		{
			value=n->val;
			if(var_in_env != NULL)
				*var_in_env=n->var_in_env;
			break;
		}
		else if (r > 0)
			n=n->left;
		else
			n=n->right;
	}
return value;
}

/*
	search for next var=value pair
*/
static int get_next_var_val(char *input, char *last, int modify_in_place, char **var, int *var_len, char **val, int *val_len)
{
	int current_index=0;
	int rc=-1;
	int max_len=last-input;
	if(var != NULL && var_len != NULL && val != NULL && val_len != NULL)
	{
		if(*input == '\0' && *(input+1) == '\0')
			return rc;
		while(*input == '\0' && max_len)
		{
			input++;
			max_len--;
		}
		while(current_index < max_len)
		{
			if(input[current_index] ==  '=')
			{
			
				*var=input;
				*var_len=current_index;
				if(modify_in_place)
					input[current_index]='\0';
				*val=input+current_index+1;
				*val_len=strlen(input+current_index+1);
				//printk("found %s [%s] %d\n", *var, *val, *val_len);
				rc=0;
				break;
			}
			current_index++;
		}
	}
return rc;
}
/*
	populate the tree structure holding the uboot env
*/
void uboot_env_init(char *input, int max_len, int force)
{
	char *var=NULL, *val=NULL, *last=NULL;
	int var_len=0, val_len=0;
	static int init_me=0;


	if(init_me == 0 || force == 1)
	{

		//printk("uboot header %x\n", *(unsigned int*)uboot_env);
		uboot_env_length=get_actual_uboot_env_length(uboot_env+UBOOT_HEADER_LEN, max_len);
		uboot_env_length+=UBOOT_HEADER_LEN;
		if(max_len > uboot_env_length)
		{
			max_len=uboot_env_length;
		}
		input+=UBOOT_HEADER_LEN;
		last=input+max_len;


		while(input)
		{
			if(get_next_var_val(input, last, 0, &var, &var_len, &val, &val_len) == 0)
			{
				insert(&head, var, var_len, val, val_len);
				input=val+val_len;
			}
			else
				break;
		}
		if(raw_proc_entry == NULL)
		{
			raw_proc_entry=proc_create_data(RAW_BINARY_FILE, S_IRUSR, proc_directory, &raw_proc, NULL);
		}
	}
	init_me++;
}

int eNvramGet(char *param, char *value, int len)
{
char *ptr=NULL;
int total_used=0;

	uboot_env_init(uboot_env, uboot_env_length, 0); //skip 12 bytes header
	if(strncmp(param, "debug", 5) == 0)
		print_tree(head);
	ptr=find_var(head, param, strlen(param), NULL);
	if (ptr != NULL)
	{
		total_used=strlen(ptr)+1;
		total_used = total_used >  len ? len : total_used;
		strncpy(value, ptr, total_used);
		
	}
		
return total_used;
}
EXPORT_SYMBOL(eNvramGet);

int eNvramSet(char *param, char *value)
{

int ret=0, old_param_length;
char *old_value=NULL, *var_in_env=NULL;

	//print_tree(head);
	old_value=find_var(head,param, strlen(param), &var_in_env);
	//printk("after search %s old value %s %s var_in_env %s\n", param, old_value, param, var_in_env);
	// we have start of u-boot env, 
	// we know the place of the parameter in the uboot_env
	// we know the total lenth of the uboot env
	if(var_in_env != NULL && old_value != NULL)
	{

		old_param_length = strlen(param) + strlen(old_value) + 1 + 1; //= + '\0'
		if((var_in_env - uboot_env) + old_param_length != uboot_env_length)
		{
			//printk("old param length %d\n", old_param_length);
			//printk("---- next var is [%s] [%p]\n", var_in_env+old_param_length , var_in_env+old_param_length);
			//printk(" uboot_env_bin %p var_in_env %p \n\n uboot_env %p len %d\n" , uboot_env, var_in_env, uboot_env, uboot_env_length - (var_in_env - uboot_env) - old_param_length);
			//printk(" uboot_env_len %d \n" , uboot_env_length);
			
			memcpy(var_in_env, var_in_env+old_param_length, uboot_env_length - (var_in_env - uboot_env) - old_param_length);
			var_in_env += (uboot_env_length - (var_in_env - uboot_env) - old_param_length);
			//printk("end of the var before add %p [%s] [%s]\n", var_in_env, param, value); 
			
			memset(var_in_env, '\0', old_param_length);
			var_in_env +=1; //nedd to skip previous '\0';  
		}
		snprintf(var_in_env, strlen(param)+strlen(value)+2, "%s=%s", param, value);
		uboot_env_length=var_in_env - uboot_env + strlen(param) + 1 + strlen(value)+ 1;
		uboot_env[uboot_env_length]='\0';
		uboot_env[uboot_env_length+1]='\0';
		memset(uboot_env+uboot_env_length+2, 0xff, uboot_env_max_length-uboot_env_length-2);
		uboot_env_init(uboot_env, uboot_env_length, 1);
		//print_tree(head);
		ret=strlen(value);
	}	
return ret;
}

int uboot_env_add(char *param_value)
{
int rc=UBOOT_ENV_ADD_FAILED;
char *temp_ptr=NULL, *old_value=NULL, *var_in_env=NULL;

	if((temp_ptr=strchr(param_value, '=')) != NULL)
	{
		temp_ptr[0]= '\0';
		old_value=find_var(head,param_value, strlen(param_value), &var_in_env);
		temp_ptr[0]= '=';
		if(old_value == NULL)
		{ 
			//printk("var %s not found\n", param_value);
			if(uboot_env_max_length-uboot_env_length-2 > strlen(param_value)+2)
			{
				//printk("Adding new var %d\n", uboot_env_length);
				//we have to skip 1 '\0'
				snprintf(uboot_env+uboot_env_length+1, strlen(param_value)+2, "%s", param_value);
				uboot_env_length = uboot_env_length+strlen(param_value) + 1;
				uboot_env[uboot_env_length]='\0';
				uboot_env[uboot_env_length+1]='\0';
				memset(uboot_env+uboot_env_length+2, 0xff, uboot_env_max_length-uboot_env_length-2);
				uboot_env_init(uboot_env, uboot_env_length, 1);
				//printk("----------------------\n");
				//print_tree(head);
				//printk("----------------------\n");
				rc=UBOOT_ENV_ADD_SUCCESS;
			}
			else
			{
				rc=UBOOT_ENV_NO_SPACE_LEFT;
				printk("uboot_env_add: Not enough space left in u-boot environment current[%d] max[%d] needed [%d] \n", uboot_env_length, uboot_env_max_length, (int32_t)strlen(param_value)+2);
			}
		}
		else
		{
			temp_ptr[0]= '\0';
			eNvramSet(param_value, temp_ptr+1);
			rc=UBOOT_ENV_ADD_SUCCESS;
			temp_ptr[0]= '=';
		}
	}
	else
	{
		rc=UBOOT_ENV_ADD_IVALID_FORMAT;
		printk("Uboot env invalid format [%s]\n", param_value);
	}
	
return rc;
}
