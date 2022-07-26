/***********************************************************************
 *
 *  Copyright (c) 2007-2016  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2016:DUAL/GPL:standard

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
 *
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <signal.h>
#include "bcm_sotp.h"

#define SOTP_DEVICE_NAME     "/dev/sotp"
#define SOTPCTL_APP_NAME     "sotpctl"
#define ROLLBACKCTL_APP_NAME "rollbackctl"
#define EXIT_SUCCESS 0  
#define EXIT_FAILURE 1  

int cmdSotpOps(SOTP_ELEMENT element, int isSet, int addr, uint32_t * bufp, int num_words, int raw)
{
    int sotpFd;
    int rc = 0;
    int i;

    SOTP_IOCTL_PARMS ioctlParms;
    ioctlParms.result = -1;

    sotpFd = open(SOTP_DEVICE_NAME, O_RDWR);

    if ( sotpFd != -1 )
    {
        ioctlParms.element = element;
        ioctlParms.inout_data = bufp;
        ioctlParms.data_len = sizeof(uint32_t)*num_words;
        ioctlParms.result = 0;
        ioctlParms.raw_access = raw;

        switch ( ioctlParms.element )
        {
            case SOTP_ROW:
                ioctlParms.row_addr = addr;
            break;

            case SOTP_REGION_FUSELOCK:
            case SOTP_REGION_READLOCK:
                ioctlParms.region_num = addr;
            break;

            case SOTP_KEYSLOT:
            case SOTP_KEYSLOT_READLOCK:
                ioctlParms.keyslot_section_num = addr;
            break;

            default:
            break;
        }

        if (isSet)
            rc = ioctl(sotpFd, SOTP_IOCTL_SET, &ioctlParms);
        else
            rc = ioctl(sotpFd, SOTP_IOCTL_GET, &ioctlParms);

        if (rc < 0)
        {
            printf("%s: sotp ioctl failure <%s %s addr:%d res:%d> rc:%d\n", __FUNCTION__,
                                                                        (isSet?"SET":"GET"),
                                                                        (element==SOTP_ROW?"ROW":element==SOTP_KEYSLOT?"KEY":"RDLOCK"),
                                                                        addr,
                                                                        ioctlParms.result,
                                                                        rc);
        }
        else
        {
            if( !isSet && ioctlParms.element != SOTP_MAP && ioctlParms.element != SOTP_ROLLBACK_LVL)
            {
                printf("SOTP Data:\n");
                for( i=0; i<num_words; i++ )
                {
                    printf("0x%08x\n", *(bufp+i));
                }
                printf("SOTP Result: %d\n", ioctlParms.result);
            }
            else if(ioctlParms.element == SOTP_ROLLBACK_LVL )
            {
                printf("Current Rollback Level: %d\n", *(bufp));
            }
        }
    }
    else
       printf("Unable to open device %s", SOTP_DEVICE_NAME);

    close(sotpFd);

    return ioctlParms.result;
}


void cmdSotpCtlHelp(char * app_name)
{
    if (strstr(app_name, SOTPCTL_APP_NAME))
    {
        fprintf(stdout,
         "\nUsage: sotpctl get map\n"
           "       sotpctl set row <row addr> <32-bit word>\n"
           "       sotpctl get row <row addr>\n"
           "       sotpctl set row_raw <row addr> <32-bit word>\n"
           "       sotpctl get row_raw <row addr>\n"
           "       sotpctl set region_fuselock <region#>\n"
           "       sotpctl get region_fuselock <region#>\n"
           "       sotpctl set region_rdlock   <region#>\n"
           "       sotpctl get region_rdlock   <region#>\n"
           "       sotpctl set keyslot <keyslot section#> <32-bit word 0>...<32-bit word 7>\n"
           "       sotpctl get keyslot <keyslot section#>\n"
           "       sotpctl set keyslot_rdlock <keyslot section#>\n"
           "       sotpctl get keyslot_rdlock <keyslot section#>\n"
           "       sotpctl --help\n"
           "Note:\n"
           "       - Credentials should be programmed using keyslot read/write functions\n"
           "         which perform CRC checks, write validation and write-lock fusing\n"
           "       - 'get map' will dump out the status of the entire SOTP array\n"
           "       - 'get row_raw' will ignore ECC and return SOTP data\n"
           "       - 'set row_raw' will ignore ECC and write to rows which already have valid data\n"
           "       - There are 4 rows in a region, and all locking is done on a region basis\n"
           "       - Fuse-locked regions can never be written to again\n"
           "       - Read-locked regions can only be read after a POR\n"
           "       - Refer to Broadcom Secure Boot documentation for row, region and section numbers\n");
    }
    else
    {
        fprintf(stdout,
         "\nUsage: rollbackctl get level\n"
           "       rollbackctl set level <Level#: 1-%d>\n" 
           "       rollbackctl set level <Level#: 1-%d> --commit\n" 
           "       rollbackctl --help\n"
           "Note:\n"
           "       - Rollback levels cannot be lowered after being set, they can only be increased\n"
           "       - To actually write the new Rollback level use the --commit option, otherwise only a dry run is attempted\n"
           "       - Refer to Broadcom Secure Boot documentation for row, region and section numbers\n", SOTP_MAX_ROLLBACK_LVL, SOTP_MAX_ROLLBACK_LVL) ;
    }

}

int main(int argc, char *argv[])
{
    int result = 0;
    uint32_t data[SOTP_NUM_WORDS_IN_KEYSLOT] = {0};
    int i=0;

    /* parse the command line and build the argument vector */
    if (argv[1] == NULL)
    {
        cmdSotpCtlHelp(argv[0]);
    }
    else if ((strcasecmp(argv[1], "get") == 0) && (argc > 2))
    {
        /* Handling rollbackctl */
        if (strstr(argv[0], ROLLBACKCTL_APP_NAME))
        {
            if(strcasecmp(argv[2], "level") == 0)  
            {
                result = cmdSotpOps(SOTP_ROLLBACK_LVL, 0, 0, &data[0], 1, 0);
            }
            else
                cmdSotpCtlHelp(argv[0]);
        }
        else
        {
            /* Handling sotpctl */
            if (strcasecmp(argv[2], "map") == 0)  
            {
                result = cmdSotpOps(SOTP_MAP, 0, 0, &data[0], 1, 0);
            }
            else if (argc > 3 )
            {
                if (strcasecmp(argv[2], "row_raw") == 0)  
                {
                    result = cmdSotpOps(SOTP_ROW, 0, atoi(argv[3]), &data[0], 1, 1);
                }
                else if (strcasecmp(argv[2], "row") == 0)  
                {
                    result = cmdSotpOps(SOTP_ROW, 0, atoi(argv[3]), &data[0], 1, 0);
                }
                else if (strcasecmp(argv[2], "keyslot") == 0)
                {
                    result = cmdSotpOps(SOTP_KEYSLOT, 0, atoi(argv[3]), &data[0], SOTP_NUM_WORDS_IN_KEYSLOT, 0);
                }
                else if (strcasecmp(argv[2], "region_rdlock") == 0)
                {
                    result = cmdSotpOps(SOTP_REGION_READLOCK, 0, atoi(argv[3]), &data[0], 0, 0);
                }
                else if (strcasecmp(argv[2], "region_fuselock") == 0)
                {
                    result = cmdSotpOps(SOTP_REGION_FUSELOCK, 0, atoi(argv[3]), &data[0], 0, 0);
                }
                else if (strcasecmp(argv[2], "keyslot_rdlock") == 0)
                {
                    result = cmdSotpOps(SOTP_KEYSLOT_READLOCK, 0, atoi(argv[3]), &data[0], 0, 0);
                }
                else
                    cmdSotpCtlHelp(argv[0]);
            }
            else
                cmdSotpCtlHelp(argv[0]);
        }
    }
    else if ((strcasecmp(argv[1], "set") == 0))
    {
        if(argc > 3)         
        {
            /* Handling rollbackctl */
            if (strstr(argv[0], ROLLBACKCTL_APP_NAME))
            {
                if( (strcasecmp(argv[2], "level") == 0 ) )
                {
                    if( (argc > 4) && (strcasecmp(argv[4], "--commit") == 0 ) )
                    {
                        data[0] = strtoul(argv[3], NULL, 10);
                        printf("Comitting new rollback level %d\n", data[0]);
                        result = cmdSotpOps(SOTP_ROLLBACK_LVL, 1, 0, &data[0], 1, 0);
                    }
                    else
                    {
                        /* Dry run */
                        uint32_t current_lvl, new_lvl;
                        new_lvl = strtoul(argv[3], NULL, 10);
                        result = cmdSotpOps(SOTP_ROLLBACK_LVL, 0, 0, &current_lvl, 1, 0);
                        if( (new_lvl > current_lvl) && (new_lvl <= SOTP_MAX_ROLLBACK_LVL) )
                        {
                            printf("Rollback level will increase %d -> %d\n", current_lvl, new_lvl);
                            printf("Not comitting new rollback revel! Use --commit option to force a commit\n");
                        }
                        else
                        {
                            printf("Invalid new rollback level %d! New level needs to be: %d < #lvl <= %d\n", new_lvl, current_lvl, SOTP_MAX_ROLLBACK_LVL);
                        }
                    }
                }
            }
            else
            {
                /* Handling sotpctl */
                if (strcasecmp(argv[2], "region_rdlock") == 0)
                {
                    result = cmdSotpOps(SOTP_REGION_READLOCK, 1, atoi(argv[3]), &data[0], i, 0);
                }
                else if (strcasecmp(argv[2], "region_fuselock") == 0)
                {
                    result = cmdSotpOps(SOTP_REGION_FUSELOCK, 1, atoi(argv[3]), &data[0], i, 0);
                }
                else if (strcasecmp(argv[2], "keyslot_rdlock") == 0)
                {
                    result = cmdSotpOps(SOTP_KEYSLOT_READLOCK, 1, atoi(argv[3]), &data[0], i, 0);
                }
                else if(argc > 4)
                {
                    for(i=0; (i<SOTP_NUM_WORDS_IN_KEYSLOT) && (i+4 < argc); i++)
                    {
                        data[i] = strtoul(argv[4+i], NULL, 16);
                    }

                    if (strcasecmp(argv[2], "row_raw") == 0)
                    {
                        result = cmdSotpOps(SOTP_ROW, 1, atoi(argv[3]), &data[0], i, 1);
                    }
                    else if (strcasecmp(argv[2], "row") == 0)
                    {
                        result = cmdSotpOps(SOTP_ROW, 1, atoi(argv[3]), &data[0], i, 0);
                    }
                    else if (strcasecmp(argv[2], "keyslot") == 0)
                    {
                        result = cmdSotpOps(SOTP_KEYSLOT, 1, atoi(argv[3]), &data[0], i, 0);
                    }
                    else
                        cmdSotpCtlHelp(argv[0]);
                }
                else
                    cmdSotpCtlHelp(argv[0]);
            }
        }
        else
            cmdSotpCtlHelp(argv[0]);
    }
    else
        cmdSotpCtlHelp(argv[0]);

    fprintf(stdout,"\n%s result: 0x%x\n", argv[0], result);
    
    return result;
}

