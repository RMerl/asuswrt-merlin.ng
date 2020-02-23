/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Environment variable subroutines		File: env_subr.c
    *  
    *  This module contains routines to muck with environment variables
    *  (manage the list, read/write to nvram, etc.)
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */

#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "env_subr.h"
///#include "nvram_subr.h"

#include "cfe_error.h"
#include "cfe.h"

/*  *********************************************************************
    *  Types
    ********************************************************************* */

typedef struct cfe_envvar_s {
    queue_t qb;
    int flags;
    char *name;
    char *value;
    /* name and value go here */
} cfe_envvar_t;

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

queue_t env_envvars = {&env_envvars,&env_envvars};
extern unsigned int cfe_startflags;

/*  *********************************************************************
    *  env_findenv(name)
    *  
    *  Locate an environment variable in the in-memory list
    *  
    *  Input parameters: 
    *  	   name - name of env var to find
    *  	   
    *  Return value:
    *  	   cfe_envvar_t pointer, or NULL if not found
    ********************************************************************* */

static cfe_envvar_t *env_findenv(const char *name)
{
    queue_t *qb;
    cfe_envvar_t *env;

    for (qb = env_envvars.q_next; qb != &env_envvars; qb = qb->q_next) {
	env = (cfe_envvar_t *) qb;
	if (strcmp(env->name,name) == 0) break;
	}

    if (qb == &env_envvars) return NULL;

    return (cfe_envvar_t *) qb;

}

/*  *********************************************************************
    *  env_enum(idx,name,namelen,val,vallen)
    *  
    *  Enumerate environment variables.  This routine locates
    *  the nth environment variable and copies its name and value
    *  to user buffers.
    *
    *  The namelen and vallen variables must be preinitialized to 
    *  the maximum size of the output buffer.
    *  
    *  Input parameters: 
    *  	   idx - variable index to find (starting with zero)
    *  	   name,namelen - name buffer and length
    *  	   val,vallen - value buffer and length
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

int env_enum(int idx,char *name,int *namelen,char *val,int *vallen)
{
    queue_t *qb;
    cfe_envvar_t *env;

    for (qb = env_envvars.q_next; qb != &env_envvars; qb = qb->q_next) {
	if (idx == 0) break;
	idx--;
	}

    if (qb == &env_envvars) return CFE_ERR_ENVNOTFOUND;
    env = (cfe_envvar_t *) qb;

    *namelen = xstrncpy(name,env->name,*namelen);
    *vallen  = xstrncpy(val,env->value,*vallen);

    return 0;

}

/*  *********************************************************************
    *  env_envtype(name)
    *  
    *  Return the type of the environment variable
    *  
    *  Input parameters: 
    *  	   name - name of environment variable
    *  	   
    *  Return value:
    *  	   flags, or <0 if error occured
    ********************************************************************* */
int env_envtype(const char *name)
{
    cfe_envvar_t *env;

    env = env_findenv(name);

    if (env) {
	return env->flags;
	}

    return CFE_ERR_ENVNOTFOUND;
}



/*  *********************************************************************
    *  env_delenv(name)
    *  
    *  Delete an environment variable
    *  
    *  Input parameters: 
    *  	   name - environment variable to delete
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

int env_delenv(const char *name)
{
    cfe_envvar_t *env;

    env = env_findenv(name);

    if (!env) return 0;

    if (!(env->flags & ENV_FLG_READONLY)) {
	q_dequeue((queue_t *) env);
	KFREE(env);
	return 0;
	}

    return CFE_ERR_ENVNOTFOUND;
}

/*  *********************************************************************
    *  env_getenv(name)
    *  
    *  Retrieve the value of an environment variable
    *  
    *  Input parameters: 
    *  	   name - name of environment variable to find
    *  	   
    *  Return value:
    *  	   value, or NULL if variable is not found
    ********************************************************************* */

char *env_getenv(const char *name)
{
    cfe_envvar_t *env;

    env = env_findenv(name);

    if (env) {
	return env->value;
	}

    return NULL;
}


/*  *********************************************************************
    *  env_setenv(name,value,flags)
    *  
    *  Set the value of an environment variable
    *  
    *  Input parameters: 
    *  	   name - name of variable
    *  	   value - value of variable
    *  	   flags - flags for variable (ENV_FLG_xxx)
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

int env_setenv(const char *name,char *value,int flags)
{
    cfe_envvar_t *env;
    int namelen;

    env = env_findenv(name);
    if (env) {
	if (!(flags & ENV_FLG_ADMIN)) {
	    if (env->flags & ENV_FLG_READONLY) return CFE_ERR_ENVREADONLY;
	    }
	q_dequeue((queue_t *) env);
	KFREE(env);
	}

    namelen = strlen(name);

    env = KMALLOC(sizeof(cfe_envvar_t) + namelen + 1 + strlen(value) + 1,0);
    if (!env) return CFE_ERR_NOMEM;

    env->name = (char *) (env+1);
    env->value = env->name + namelen + 1;
    env->flags = (flags & ENV_FLG_MASK);

    strcpy(env->name,name);
    strcpy(env->value,value);

    q_enqueue(&env_envvars,(queue_t *) env);

    return 0;
}
int env_save(void)
{
    return 0;
}
