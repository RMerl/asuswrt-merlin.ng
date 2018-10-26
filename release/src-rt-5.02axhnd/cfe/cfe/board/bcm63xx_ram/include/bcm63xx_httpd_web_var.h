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

#if !defined(__BCM63XX_HTTPD_WEB_VARS_H__)
#define __BCM63XX_HTTPD_WEB_VARS_H__


#define WEB_VAR_JS_PREFIX  "<%cfe_web_var_get("
#define WEB_VAR_JS_SUFFIX  ")%>"
#define WEB_VAR_NAME_MAX_LEN 15
#define WEB_VAR_VALUE_MAX_LEN 10

/* Function pointer for data retrieval function
 * Args: 
 * var_value(IN)      - pointer to where value should be copied
 * length   (INOUT)   - pointer to allowed length of var_value.
 *                      function will overwrite this with actual
 *                      length of value that is written to var_value
 * returns: 0 is successfull, -1 if not
 */
typedef int (*cfe_web_var_getfunc)(char * var_value, int * length);

/***************************************************************************
 * Function Name: cfe_web_get_num_web_vars
 * Description  : Returns the number of web_vars that are compiled in
 * Args         : 
 * Returns      : number of web_vars that are compiled in 
 ***************************************************************************/
int cfe_web_get_num_web_vars(void);

/***************************************************************************
 * Function Name: cfe_web_get_web_var
 * Description  : Returns the value of a web_var
 * Args         : (IN)web_var - Buffer pointing to the beginning of a web_var
 *              : (IN)length  - Length of the buffer containing the web_var
 *              : (OUT)length - Length of the web_var value
 * Returns      : Pointer to a buffer containing the web_var value. NULL if
 *                web_var is not found
 ***************************************************************************/
char * cfe_web_get_web_var( char * web_var, int * length );
#endif // __BCM63XX_HTTPD_WEB_VARS_H__
