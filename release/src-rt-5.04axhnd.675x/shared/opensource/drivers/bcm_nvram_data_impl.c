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
#include <linux/fs.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <bcmTag.h>

#define ETHADDR "ethaddr"

enum
{
	INTEGER_STRING,
	INT8_STRING,
	INT16_STRING,
	SHORT_STRING,
	STRING,
	HEX_STRING,
	MAC_ID_STRING,
	COMMA_SEPARATED_INTEGER_STRING,
	COMMA_SEPARATED_HEX_INTEGER_STRING,
	COMMA_SEPARATED_SHORT_STRING,
	COMMA_SEPARATED_STRING,
	BINARY_DATA,
	MAX_TYPE
};

struct nvdata_params_struct
{
	char *param;
	void* offset;
	int size;
	int type;
};

static struct nvdata_params_struct nps[]=
{
	{NVRAM_ULVERSION, &((NVRAM_DATA*)0)->ulVersion, sizeof(((NVRAM_DATA*)0)->ulVersion), INTEGER_STRING},
	{NVRAM_SZBOOTLINE, &((NVRAM_DATA*)0)->szBootline, sizeof(((NVRAM_DATA*)0)->szBootline), STRING},
	{NVRAM_SZBOARDID, &((NVRAM_DATA*)0)->szBoardId, sizeof(((NVRAM_DATA*)0)->szBoardId), STRING},
	{NVRAM_ULMAINTPNUM, &((NVRAM_DATA*)0)->ulMainTpNum, sizeof(((NVRAM_DATA*)0)->ulMainTpNum), INTEGER_STRING },
	{NVRAM_ULPSISIZE, &((NVRAM_DATA*)0)->ulPsiSize, sizeof(((NVRAM_DATA*)0)->ulPsiSize), INTEGER_STRING },
	{NVRAM_ULNUMMACADDRS, &((NVRAM_DATA*)0)->ulNumMacAddrs, sizeof(((NVRAM_DATA*)0)->ulNumMacAddrs), INTEGER_STRING},
	{NVRAM_UCABASEMACADDR, &((NVRAM_DATA*)0)->ucaBaseMacAddr, sizeof(((NVRAM_DATA*)0)->ucaBaseMacAddr), MAC_ID_STRING},
	{NVRAM_PAD, &((NVRAM_DATA*)0)->pad, sizeof(((NVRAM_DATA*)0)->pad), INT8_STRING}, //unused
	{NVRAM_BACKUPPSI, &((NVRAM_DATA*)0)->backupPsi, sizeof(((NVRAM_DATA*)0)->backupPsi), INT8_STRING },
	{NVRAM_ULCHECKSUMV4, &((NVRAM_DATA*)0)->ulCheckSumV4, sizeof(((NVRAM_DATA*)0)->ulCheckSumV4), INTEGER_STRING },
	{NVRAM_GPONSERIALNUMBER, &((NVRAM_DATA*)0)->gponSerialNumber, sizeof(((NVRAM_DATA*)0)->gponSerialNumber), STRING }, //??
	{NVRAM_GPONPASSWORD, &((NVRAM_DATA*)0)->gponPassword, sizeof(((NVRAM_DATA*)0)->gponPassword), STRING}, //??
	{NVRAM_WPSDEVICEPIN, &((NVRAM_DATA*)0)->wpsDevicePin, sizeof(((NVRAM_DATA*)0)->wpsDevicePin), STRING}, //??
	{NVRAM_WLANPARAMS, &((NVRAM_DATA*)0)->wlanParams, sizeof(((NVRAM_DATA*)0)->wlanParams), HEX_STRING}, //??
	{NVRAM_ULSYSLOGSIZE, &((NVRAM_DATA*)0)->ulSyslogSize, sizeof(((NVRAM_DATA*)0)->ulSyslogSize), INTEGER_STRING },
	{NVRAM_ULNANDPARTOFSKB, &((NVRAM_DATA*)0)->ulNandPartOfsKb, sizeof(((NVRAM_DATA*)0)->ulNandPartOfsKb), COMMA_SEPARATED_INTEGER_STRING },// separated
	{NVRAM_ULNANDPARTSIZEKB, &((NVRAM_DATA*)0)->ulNandPartSizeKb, sizeof(((NVRAM_DATA*)0)->ulNandPartSizeKb), COMMA_SEPARATED_INTEGER_STRING },// separated
	{NVRAM_SZVOICEBOARDID, &((NVRAM_DATA*)0)->szVoiceBoardId, sizeof(((NVRAM_DATA*)0)->szVoiceBoardId), STRING},
	{NVRAM_AFEID, &((NVRAM_DATA*)0)->afeId, sizeof(((NVRAM_DATA*)0)->afeId), COMMA_SEPARATED_HEX_INTEGER_STRING},
	{NVRAM_OPTICRXPWRREADING, &((NVRAM_DATA*)0)->opticRxPwrReading, sizeof(((NVRAM_DATA*)0)->opticRxPwrReading), INT16_STRING},
	{NVRAM_OPTICRXPWROFFSET, &((NVRAM_DATA*)0)->opticRxPwrOffset, sizeof(((NVRAM_DATA*)0)->opticRxPwrOffset), INT16_STRING},
	{NVRAM_OPTICTXPWRREADING, &((NVRAM_DATA*)0)->opticTxPwrReading, sizeof(((NVRAM_DATA*)0)->opticTxPwrReading), INT16_STRING},
	{NVRAM_UCUNUSED2, &((NVRAM_DATA*)0)->ucUnused2, sizeof(((NVRAM_DATA*)0)->ucUnused2), INTEGER_STRING},
	{NVRAM_UCFLASHBLKSIZE, &((NVRAM_DATA*)0)->ucFlashBlkSize, sizeof(((NVRAM_DATA*)0)->ucFlashBlkSize), INT8_STRING},
	{NVRAM_UCAUXFSPERCENT, &((NVRAM_DATA*)0)->ucAuxFSPercent, sizeof(((NVRAM_DATA*)0)->ucAuxFSPercent), INT8_STRING},
	{NVRAM_UCUNUSED3, &((NVRAM_DATA*)0)->ucUnused3, sizeof(((NVRAM_DATA*)0)->ucUnused3), INTEGER_STRING},
	{NVRAM_ULBOARDSTUFFOPTION, &((NVRAM_DATA*)0)->ulBoardStuffOption, sizeof(((NVRAM_DATA*)0)->ulBoardStuffOption), INTEGER_STRING},
	{NVRAM_ALLOCS, &((NVRAM_DATA*)0)->allocs, sizeof(((NVRAM_DATA*)0)->allocs), HEX_STRING},
	{NVRAM_ULMEMORYCONFIG, &((NVRAM_DATA*)0)->ulMemoryConfig, sizeof(((NVRAM_DATA*)0)->ulMemoryConfig), INTEGER_STRING},
	{NVRAM_PART_INFO, &((NVRAM_DATA*)0)->part_info, sizeof(((NVRAM_DATA*)0)->part_info), COMMA_SEPARATED_SHORT_STRING},
	{NVRAM_ALLOC_DHD, &((NVRAM_DATA*)0)->alloc_dhd, sizeof(((NVRAM_DATA*)0)->alloc_dhd), HEX_STRING},
	{NVRAM_ULFEATURES, &((NVRAM_DATA*)0)->ulFeatures, sizeof(((NVRAM_DATA*)0)->ulFeatures), INTEGER_STRING},
	{NVRAM_CHUNUSED, &((NVRAM_DATA*)0)->chUnused, sizeof(((NVRAM_DATA*)0)->chUnused), INTEGER_STRING},
	{NVRAM_ULCHECKSUM, &((NVRAM_DATA*)0)->ulCheckSum, sizeof(((NVRAM_DATA*)0)->ulCheckSum), HEX_STRING},
	{NULL,NULL, 0, 0}
};

static char *uboot_env=NULL;
static int uboot_env_length=UBOOT_MAX_ENV_LEN;
static struct proc_dir_entry *proc_directory=NULL;
static struct proc_dir_entry *raw_proc_entry=NULL;
#define RAW_BINARY_FILE "raw"

static ssize_t proc_get_raw(struct file * file, char * buff, size_t len, loff_t *offset)
{

	if (uboot_env == NULL && *offset < UBOOT_MAX_ENV_LEN+UBOOT_HEADER_LEN)
		return 0;

	len=len < (UBOOT_MAX_ENV_LEN+UBOOT_HEADER_LEN-*offset)?len:(UBOOT_MAX_ENV_LEN+UBOOT_HEADER_LEN-*offset);
	if(copy_to_user(buff, uboot_env+*offset, len))
	{
		len=-EFAULT;
	}
	*offset += len;
	return len;
}


static struct file_operations raw_proc = {
	.read=proc_get_raw
};

unsigned char *get_uboot_env_area(void)
{
	if(uboot_env == NULL)
	{
	        uboot_env = kmalloc(UBOOT_MAX_ENV_LEN+UBOOT_HEADER_LEN, GFP_ATOMIC);
		uboot_env_length=UBOOT_MAX_ENV_LEN;
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
	static struct file_operations env_proc = {
		.read=proc_get_env_param,
		//.write=proc_set_param,
	};

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
		//printk("n->var %s var [%s] %d %d\n", n->var, var, var_length, r);
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
		uboot_env_length+=UBOOT_HEADER_LEN;//
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

//void updateInMemNvramData(const unsigned char *data, int len, int offset);
static NVRAM_DATA* inMemNvramData_ptr;

static void init_inMemNvramData(void)
{
	if(inMemNvramData_ptr == NULL)
		inMemNvramData_ptr=get_inMemNvramData();
}

int eNvramGet(char *param, char *value, int len)
{
struct nvdata_params_struct *nps_ptr=nps;
char *ptr=NULL, filler=' ';
int total_used=0,index, used, to_copy;

	if(is_cfe_boot())
	{

		init_inMemNvramData();

		while(nps_ptr->param != NULL)
		{
			if(param == nps_ptr->param || strcmp(param, nps_ptr->param) == 0)
			{
				// always assumed inMemNvramData is pre populated
				ptr=(char *)inMemNvramData_ptr + (long)(int*)nps_ptr->offset;
				total_used=0;
				index=0;
				to_copy=nps_ptr->size;
				switch(nps_ptr->type)
				{
					case INTEGER_STRING:
						total_used=snprintf(value, len, "%d", *(int*)ptr);
						break;
					case INT8_STRING:
						total_used=snprintf(value, len, "%hhd", *(char*)ptr);
						break;
					case INT16_STRING:
						total_used=snprintf(value, len, "%hd", *(short*)ptr);
						break;
					case STRING:
						total_used=snprintf(value, len, "%s", ptr);
						break;
					case MAC_ID_STRING:
						filler=':';
						to_copy=6;//mac_id size
					//intentionally left out a break statement so the rest of the code will execute
					case HEX_STRING:
						while(len > 3 && index < to_copy)
						{
							used=snprintf(value+total_used, len,"%02hhx%c", ptr[index], filler );
							index++;
							total_used+=used;
							len-=used;
							if(to_copy-index == 1) filler='\0';
						}
						break;
					case COMMA_SEPARATED_INTEGER_STRING:
						total_used=0;
						while(len > 1 && index < nps_ptr->size)
						{
							used=snprintf(value+total_used,len, "%d;", *((int*)(ptr+index)));
							index+=sizeof(int);
							total_used+=used;
							len-=used;
						}
						break;
					case COMMA_SEPARATED_HEX_INTEGER_STRING:
						total_used=0;
						while(len > 1 && index < nps_ptr->size)
						{
							used=snprintf(value+total_used,len, "%x;", *((int*)(ptr+index)));
							index+=sizeof(int);
							total_used+=used;
							len-=used;
						}
						break;
					case COMMA_SEPARATED_SHORT_STRING:
						total_used=0;
						while(len > 1 && index < nps_ptr->size)
						{
							used=snprintf(value+total_used,len, "%hi:", *((short*)(ptr+index)));
							index+=sizeof(short);
							total_used+=used;
							len-=used;
						}
						break;
				}
				break;
			}
			nps_ptr++;
		}
	}
	else
	{
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
		
	}
return total_used;
}

int eNvramSet(char *param, char *value)
{

struct nvdata_params_struct *nps_ptr=nps;
char *ptr=NULL;
int total_used=0,index,ret=0,temp, old_param_length;
char *old_value=NULL, *var_in_env=NULL;

	if(is_cfe_boot())
	{

		init_inMemNvramData();

		while(nps_ptr->param != NULL)
		{
			if(param == nps_ptr->param || strcmp(param, nps_ptr->param) == 0)
			{
				// always assumed inMemNvramData is pre populated
				ptr=(char *)inMemNvramData_ptr + (long)(int*)nps_ptr->offset;
				total_used=0;
				index=0;
				switch(nps_ptr->type)
				{
					case INTEGER_STRING:
						sscanf(value, "%u", (unsigned int*) ptr);
						break;
					case INT8_STRING:
						sscanf(value, "%d", &temp);
						if(temp <= 255)
						{
							ptr[0]=temp&0xff;
						}
						break;
					case INT16_STRING:
						sscanf(value, "%hi", (short*)ptr);
						break;
					case STRING:
						strncpy(ptr, value, nps_ptr->size);
						break;
					case MAC_ID_STRING:
						if(strchr(value, ':'))
							sscanf(value, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
								&ptr[0], &ptr[1],&ptr[2],&ptr[3],&ptr[4],&ptr[5]);
						else if(strchr(value, '-'))
							sscanf(value, "%02hhx-%02hhx-%02hhx-%02hhx-%02hhx-%02hhx",
								&ptr[0], &ptr[1],&ptr[2],&ptr[3],&ptr[4],&ptr[5]);

						else
							sscanf(value, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
								&ptr[0], &ptr[1],&ptr[2],&ptr[3],&ptr[4],&ptr[5]);
						total_used=6;

						break;
					case HEX_STRING:
						while(total_used < nps_ptr->size && index < strlen(value) )
						{
							sscanf(value+index, "%02hhx", ptr+total_used);
							index+=3;//skip 2 digits and a space
							total_used++;
						}
						break;
					case COMMA_SEPARATED_INTEGER_STRING:
						total_used=0;
						while(value != NULL && total_used < nps_ptr->size)
						{
							sscanf(value, "%d;", (int*)ptr+total_used);
							total_used+=sizeof(int);
							value=strchr(value,';');
						}
						break;
					case COMMA_SEPARATED_HEX_INTEGER_STRING:
						total_used=0;
						while(value != ((void *)0) && total_used < nps_ptr->size)
						{
							sscanf(value, "%x;", (int*)ptr+total_used);
							total_used+=sizeof(int);
							value=strchr(value,';');
						}
						break;
					case COMMA_SEPARATED_SHORT_STRING:
						total_used=0;
						while(value != NULL && total_used < nps_ptr->size)
						{
							sscanf(ptr+total_used, "%hi;", (short*)value+total_used);
							total_used+=sizeof(short);
							value=strchr(value,';');
						}
					break;

				}
				break;
			}

			nps_ptr++;
		}
		inMemNvramData_ptr->ulCheckSum=0;
		ret=total_used;
	}
	else
	{
		//print_tree(head);
		old_value=find_var(head,param, strlen(param), &var_in_env);
		//printk("old value %s %s var_in_env %s\n", old_value, param, var_in_env);
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
			memset(uboot_env+uboot_env_length+2, 0xff, UBOOT_MAX_ENV_LEN-uboot_env_length-2);
			uboot_env_init(uboot_env, uboot_env_length, 1);
			//print_tree(head);
			ret=strlen(value);
		}	
	}
return ret;
}

int uboot_env_add(char *param_value)
{
int rc=UBOOT_ENV_ADD_FAILED;
char *temp_ptr=NULL, *old_value=NULL, *var_in_env=NULL;

	if(!is_cfe_boot())
	{
		if((temp_ptr=strchr(param_value, '=')) != NULL)
		{
			temp_ptr[0]= '\0';
			old_value=find_var(head,param_value, strlen(param_value), &var_in_env);
			temp_ptr[0]= '=';
			if(old_value == NULL)
			{ 
				if(UBOOT_MAX_ENV_LEN-uboot_env_length-2 > strlen(param_value)+2)
				{
					//we have to skip 1 '\0'
					snprintf(uboot_env+uboot_env_length+1, strlen(param_value)+2, "%s", param_value);
					uboot_env_length = uboot_env_length+strlen(param_value) + 2 + 1;
					uboot_env[uboot_env_length]='\0';
					uboot_env[uboot_env_length+1]='\0';
					memset(uboot_env+uboot_env_length+2, 0xff, UBOOT_MAX_ENV_LEN-uboot_env_length-2);
					uboot_env_init(uboot_env, uboot_env_length, 1);
					//print_tree(head);
					rc=UBOOT_ENV_ADD_SUCCESS;
				}
				else
				{
					rc=UBOOT_ENV_NO_SPACE_LEFT;
					printk("uboot_env_add: Not enough space left in u-boot environment current[%d] max[%d] needed [%d] \n", uboot_env_length, UBOOT_MAX_ENV_LEN, strlen(param_value)+2);
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
	}
	else
	{
		printk("new nvram add feature not supported in CFE!\n");
		rc=UBOOT_ENV_ADD_NOT_SUPPORTED;
	}
return rc;
}
