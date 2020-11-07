/*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
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
:>
*/

#include <stdlib.h>
#include <wlcsm_lib_api.h>
#include <string.h>
#include <wlcsm_linux.h>
#include <stdio.h>

void usage()
{
    printf("wltest [cmd] [parameters] \r\n");
    printf("wltest set varname=varvalue [subidx] [idx]\r\n");
    printf("wltest get varname [subidx] [idx]\r\n");
    printf("wltest dmset oid offset value [subidx] [idx]\r\n" );
    printf("wltest dmvalidate oid offset value [subidx] [idx]\r\n" );
    printf("wltest dmget oid offset [subidx] [idx]\r\n" );
	 printf("wltest dmevt reg/dereg  obj/sta\r\n" );

}
void dm_event(int argc,char **argv)
{

    if(argc<3) {
        WLCSM_TRACE(WLCSM_TRACE_DBG," argc:%d \r\n",argc );
        printf("dmevt reg/dereg obj/sta \r\n" );
    } else {
        int to_reg=0;
        int event=WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED;
        if (*++argv && (!strcmp(*argv,"reg"))) to_reg=1;
        if (*++argv) {
            if(!strcmp(*argv,"sta")) event=WLCSM_DM_MNGR_EVENT_STAS_CHANGED;
        }
        return  wlcsm_mngr_dm_register_event(event,to_reg);
    }
    return 0;
}
/* set or validate var by oid and offset */
void dm_set_validate(int argc,int b_set,char **argv)
{

    unsigned int oid=0;
    unsigned int offset=0;
    unsigned int idx=0;
    unsigned int subidx=0;
    char *varvalue="";
    if(argc<4) {

        WLCSM_TRACE(WLCSM_TRACE_DBG," argc:%d \r\n",argc );
        if(b_set)
            printf("dmset oid offset value [subidx] [idx]\r\n" );
        else
            printf("dmvalidate oid offset value [subidx] [idx]\r\n" );
        return 0;

    } else {
        if (*++argv) {
            sscanf(*argv,"%u",&oid);
            WLCSM_TRACE(WLCSM_TRACE_DBG," oid:%d \r\n",oid );

        }
        if (*++argv) {
            sscanf(*argv,"%u",&offset);
            WLCSM_TRACE(WLCSM_TRACE_DBG," offset:%d \r\n",offset );
        }
        if (*++argv) {
            varvalue=*argv;
        }
        if (*++argv) {
            sscanf(*argv,"%u",&subidx);
            WLCSM_TRACE(WLCSM_TRACE_DBG," subidx:%d \r\n",subidx);

        }
        if (*++argv) {
            sscanf(*argv,"%u",&idx);
            WLCSM_TRACE(WLCSM_TRACE_DBG," idx:%d \r\n",idx);

        }
        if(b_set) {
            if(!wlcsm_mngr_dm_set(idx,subidx,oid,offset,varvalue)) {
                WLCSM_TRACE(WLCSM_TRACE_DBG," dm set success \r\n" );
            } else
                WLCSM_TRACE(WLCSM_TRACE_DBG," dm set failure \r\n" );
        } else {

            if(!wlcsm_mngr_dm_validate(idx,subidx,oid,offset,varvalue)) {
                WLCSM_TRACE(WLCSM_TRACE_DBG," dm validate success \r\n" );
            } else
                WLCSM_TRACE(WLCSM_TRACE_DBG,"dm  validate failure \r\n" );
        }
    }
}
/** main function
 *
 */
int main ( int argc, char *argv[] )
{
    char *name,*value,buf[6024]= {0};
    int i=0;

    WLCSM_SET_TRACE("wltest");
    --argc;
    ++argv;

    if (!*argv)
        usage();

    for (; *argv; argv++) {
        if (!strncmp(*argv, "get", 3)) {
            if (*++argv) {

                int idx=0;
                int subidx=0;
                if(*(argv+1)) subidx=atoi(*(argv+1));
                if(*(argv+2)) idx=atoi(*(argv+2));

                if ((value=wlcsm_mngr_get(idx, subidx,*argv,buf))) {
                    printf("  %s's values is %s \r\n",(*argv),value);
                } else
                    printf(" %s's values is null \r\n",(*argv));
            }
        } else if (!strncmp(*argv, "tr69", 4)) {
            int times=1;
            if (*++argv) {
                sscanf(*argv,"%d",&times);
            }
            for ( i=0; i<times; i++ ) {
                WLCSM_TRACE(WLCSM_TRACE_DBG,"........... restart:%d \r\n",i );
                wlcsm_mngr_restart(0,WLCSM_MNGR_RESTART_TR69C,WLCSM_MNGR_RESTART_NOSAVEDM,0);
            }
        } else if (!strncmp(*argv, "dmvalidate", 10)) {
            dm_set_validate(argc,0,argv);
        } else if (!strncmp(*argv, "dmset", 5)) {
            dm_set_validate(argc,1,argv);
        } else if (!strncmp(*argv, "dmevt", 5)) {
            dm_event(argc,argv);
        } else if (!strncmp(*argv, "dmget", 5)) {

            unsigned int oid=0;
            unsigned int offset=0;
            unsigned int b_set=0;
            unsigned int idx=0;
            unsigned int subidx=0;
            if (*++argv) {
                sscanf(*argv,"%u",&oid);
                WLCSM_TRACE(WLCSM_TRACE_DBG," oid:%d \r\n",oid );

            }
            if (*++argv) {
                sscanf(*argv,"%u",&offset);
                WLCSM_TRACE(WLCSM_TRACE_DBG," offset:%d \r\n",offset );

            }
            if (*++argv) {
                sscanf(*argv,"%u",&subidx);
                WLCSM_TRACE(WLCSM_TRACE_DBG," subidx:%d \r\n",subidx);

            }
            if (*++argv) {
                sscanf(*argv,"%u",&idx);
                WLCSM_TRACE(WLCSM_TRACE_DBG," idx:%d \r\n",idx);

            }
            if(wlcsm_mngr_dm_get(idx,subidx,oid,offset,buf)) {

                WLCSM_TRACE(WLCSM_TRACE_DBG," not null \r\n" );

                WLCSM_TRACE(WLCSM_TRACE_DBG," get:%s \r\n",buf );

            } else
                WLCSM_TRACE(WLCSM_TRACE_DBG," is null \r\n" );


        } else if (!strncmp(*argv, "gget", 4)) {
            if (*++argv) {

                wlcsm_mngr_get(0, 0,"wlnphyrates 0",buf);

                printf(" value:%s \r\n",buf);
            }
        } else if (!strncmp(*argv, "set", 3)) {
            if (*++argv) {
                strncpy(value = buf, *argv, sizeof(buf));
                name = strsep(&value, "=");
                if(value) {
                    if(!wlcsm_mngr_set(0,0,name, value)) {
                        printf(" set :%s value:%s success \r\n",name,value );
                    } else
                        printf(" set :%s value:%s failed \r\n",name,value );
                }
            }
        }
    }
    return 0;
}

