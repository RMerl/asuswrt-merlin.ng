/***************************************************************************
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
 ***************************************************************************/
#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "bcm63xx_httpd_web_var.h"

#if INC_EMMC_FLASH_DRIVER
#include "dev_emmcflash.h"
#endif

extern char ul_html[];
extern int ul_html_size;

typedef struct
{
    char web_var_name[WEB_VAR_NAME_MAX_LEN];
    char web_var_value[WEB_VAR_VALUE_MAX_LEN];
    cfe_web_var_getfunc getfunc;
} WEB_VAR_ENTRY;

static WEB_VAR_ENTRY web_var_table[] = 
{   /* var_name           dflt_value       getfunc */
#if INC_EMMC_FLASH_DRIVER
    { "isEmmcImg",          "1",            NULL},
    { "isImgupdRepart",     "",             &emmc_get_img_update_repart_web_var},
#else    
    { "isEmmcImg",          "0",            NULL},
    { "isImgupdRepart",     "0",            NULL},
#endif
    { "", "" }
};

static int get_web_var_value(char *var_value, char * var_name)
{
    int i=0;
    int length = WEB_VAR_VALUE_MAX_LEN;
    int ret = -1;
    while( strlen(web_var_table[i].web_var_name))
    {   
        /* Check if variable name matches */
        if( strncmp(web_var_table[i].web_var_name, var_name, strlen(web_var_table[i].web_var_name))==0)
        {
            /* Call getfunc if it exists */
            if( web_var_table[i].getfunc )
            {
                ret = web_var_table[i].getfunc(var_value, &length); 
                if(ret < 0 || length > WEB_VAR_VALUE_MAX_LEN)
                {
                    if( ret < 0 )
                        printf("Error: Variable %s's getfunction failed with ret:%d\n", var_name, ret);
                    else
                        printf("Error: Variable %s's value length %dexceeds maximum %d, memory corruption!\n"
                            , var_name, length, WEB_VAR_VALUE_MAX_LEN);
                    return -1;
                }
                return 0;
            }
            else
            {
                /* No getfunc, simply use default value */
                strncpy(var_value, web_var_table[i].web_var_value, strlen(web_var_table[i].web_var_value));
                return 0;
            }
        }
        i++;
    }
    return -1;
}

/***************************************************************************
 * Function Name: cfe_web_get_num_web_vars
 * Description  : Returns the number of web_vars that are compiled in
 * Args         : 
 * Returns      : number of web_vars that are compiled in 
 ***************************************************************************/
int cfe_web_get_num_web_vars(void)
{
    int i=0;
    while( strlen(web_var_table[i].web_var_name) )
    {
        i++;
    }
#ifdef CFE_WEB_VAR_DEBUG    
    printf("%s: %d\n", __FUNCTION__, i);
#endif    
    return i;
}

/***************************************************************************
 * Function Name: cfe_web_get_web_var
 * Description  : Returns the value of a web_var
 * Args         : (IN)web_var - Buffer pointing to the beginning of a web_var
 *              : (IN)length  - Length of the buffer containing the web_var
 *              : (OUT)length - Length of the web_var value
 * Returns      : Pointer to a buffer containing the web_var value. NULL if
 *                web_var is not found
 ***************************************************************************/
char * cfe_web_get_web_var( char * web_var, int * length )
{
    char * web_var_ptr = web_var;
    int var_name_length = 0;
    char * end_var_name = NULL;
    char * start_var_name = NULL;
    int i;
    static char var_value[WEB_VAR_VALUE_MAX_LEN] = {};
    
    if( !*length || *length < strlen(WEB_VAR_JS_PREFIX))
        goto web_var_notfound;

    /* Look for prefix */
    if (strncmp(web_var_ptr, WEB_VAR_JS_PREFIX, strlen(WEB_VAR_JS_PREFIX)) == 0) 
    {
        /* start of webvar name */
        web_var_ptr += strlen(WEB_VAR_JS_PREFIX);
        start_var_name = web_var_ptr;

        /* Find end of webvar name */
        for( i=0; (i<(WEB_VAR_NAME_MAX_LEN + strlen(WEB_VAR_JS_SUFFIX))) 
              &&  (i<(*length - strlen(WEB_VAR_JS_PREFIX))); i++ )
        {
            if( *web_var_ptr == ')')
            {
                end_var_name = web_var_ptr;
                var_name_length = end_var_name - start_var_name;
                break;
            }
            web_var_ptr++;
        }

        /* Quit if cant find end of entry */
        if( !end_var_name || !var_name_length || var_name_length > WEB_VAR_NAME_MAX_LEN )
            goto web_var_notfound;

        /* Check table for var_name */
        memset( var_value, '\0', WEB_VAR_VALUE_MAX_LEN);
        if (get_web_var_value(var_value, start_var_name) < 0 )
            goto web_var_notfound;

        *length = strlen(var_value);
        return &var_value[0];
    }

web_var_notfound:
    *length = 0;
    return NULL;
}

