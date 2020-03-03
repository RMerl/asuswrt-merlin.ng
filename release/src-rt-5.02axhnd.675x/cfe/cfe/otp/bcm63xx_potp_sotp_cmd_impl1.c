/*
 * <:label-BRCM:2012:NONE:standard
 *
 * :>
 * */

#include "bcm63xx_util.h"
#include "btrm_if.h"
#include "bcm_map.h"
#include "bcm_otp.h"
#include "lib_byteorder.h"
#include "lib_string.h"
#include "bcm63xx_sotp.h"
#include "bcm63xx_potp_sotp_util.h"

typedef enum cmd_otpcfg_conf{
    CMD_OTPCFG_MODE_BOOTROM=0,
    CMD_OTPCFG_MODE_OP,
    CMD_OTPCFG_MFG_MRKTID,
    CMD_OTPCFG_OP_MRKTID,
    CMD_OTPCFG_MAP_END,
/* below are the commands which are not mapped to strings*/
    CMD_OTPCFG_MODE_BRCM_BOOTROM,
    CMD_OTPCFG_MODE_CUST_BOOTROM,
    CMD_OTPCFG_MAX
} cmd_otpcfg_conf_t;

static char* otpcfg_opts[CMD_OTPCFG_MAX] = {"btrm_enable","op_enable","mid","oid"};

static cmd_otpcfg_conf_t opt_to_id(char* str)
{
    int i;
    for (i = 0; i < CMD_OTPCFG_MAP_END; i++) { 
        if (strcmp(str,otpcfg_opts[i])==0) {
            break;
        }
    }
    return (cmd_otpcfg_conf_t)i;
}

static void cmd_help(void)
{
    printf("\nUsage: btrmcfg <get|set> %s\n"
           "       otpcfg <get|set> %s\n"
           "       otpcfg <get|set> %s <16bit market identifier>\n"
           "       otpcfg <get|set> %s <16bit market identifier>\n"
           "       otpcfg --help\n",
         otpcfg_opts[CMD_OTPCFG_MODE_BOOTROM],
         otpcfg_opts[CMD_OTPCFG_MODE_OP],
         otpcfg_opts[CMD_OTPCFG_MFG_MRKTID],
         otpcfg_opts[CMD_OTPCFG_OP_MRKTID]);
}

static int get_val(cmd_otpcfg_conf_t mode, uint32_t *val)
{
    int rc = -1;
    uint32_t data,row,mask,shift;
    switch(mode) {
          case CMD_OTPCFG_MODE_BRCM_BOOTROM:
               row = OTP_BRCM_BTRM_BOOT_ENABLE_ROW;
               mask = OTP_BRCM_BTRM_BOOT_ENABLE_MASK;
               shift = OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT;
               break;
          case CMD_OTPCFG_MODE_CUST_BOOTROM:
               row = OTP_CUST_BTRM_BOOT_ENABLE_ROW;
               mask = OTP_CUST_BTRM_BOOT_ENABLE_MASK;
               shift = OTP_CUST_BTRM_BOOT_ENABLE_SHIFT;
               break;
          case CMD_OTPCFG_MODE_OP:
               row = OTP_CUST_OP_INUSE_ROW;
               mask = OTP_CUST_OP_INUSE_MASK;
               shift = OTP_CUST_OP_INUSE_SHIFT;
               break;
          case CMD_OTPCFG_MFG_MRKTID:
               row = OTP_CUST_MFG_MRKTID_ROW;
               mask = OTP_CUST_MFG_MRKTID_MASK;
               shift = OTP_CUST_MFG_MRKTID_SHIFT;
               break;
          case CMD_OTPCFG_OP_MRKTID:
               row = OTP_CUST_OP_MRKTID_ROW;
               mask = OTP_CUST_OP_MRKTID_MASK;
               shift = OTP_CUST_OP_MRKTID_SHIFT;
               break;
          default:
               goto err;
    }
    rc = read_potp(row, &data);
err:
    if (!rc) {
       *val = (data&mask)>>shift;
    } else {
        printf("ERROR: otp failure\n");
    }
    return rc; 
}

static int set_val(cmd_otpcfg_conf_t mode, uint32_t val)
{
    int rc = -1;
    uint32_t row,mask,shift;
    switch(mode) {
          case CMD_OTPCFG_MODE_CUST_BOOTROM:
               row = OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW;
               mask = OTP_CUST_BTRM_BOOT_ENABLE_MASK;
               shift = OTP_CUST_BTRM_BOOT_ENABLE_SHIFT;
               break;
          case CMD_OTPCFG_MODE_BRCM_BOOTROM:
               row = OTP_BRCM_BTRM_BOOT_ENABLE_ROW;
               mask = OTP_BRCM_BTRM_BOOT_ENABLE_MASK;
               shift = OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT;
               break;
          case CMD_OTPCFG_MODE_OP:
               row = OTP_CUST_OP_INUSE_FUSE_ROW;
               mask = OTP_CUST_OP_INUSE_MASK;
               shift = OTP_CUST_OP_INUSE_SHIFT;
               break;
          case CMD_OTPCFG_MFG_MRKTID:
               row = OTP_CUST_MFG_MRKTID_FUSE_ROW;
               mask = OTP_CUST_MFG_MRKTID_MASK;
               shift = OTP_CUST_MFG_MRKTID_SHIFT;
               break;
          case CMD_OTPCFG_OP_MRKTID:
               row = OTP_CUST_OP_MRKTID_FUSE_ROW;
               mask = OTP_CUST_OP_MRKTID_MASK;
               shift = OTP_CUST_OP_MRKTID_SHIFT;
               break;
          default:
               goto err;
    }

    printf("\nProgramming value 0x%x bits \n", val, ((val<<shift)&mask)); 

    rc = burn_potp(row, (val<<shift)&mask);
err:
    if (rc) {
        printf("ERROR: Failed setting mode\n");
    }
    return rc; 
}


static int warning_confirm(void)
{
     printf("\n This step is irreversible. Do you want to proceed?\n");
     if (yesno()) {
        return 1;
     }
     return 0;
}

static int otpcfg(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int rc = -1;
    uint32_t res = 0;
    char* arg[3] = {0};
    cmd_otpcfg_conf_t opt_id;
    if (argc < 2 ) {
        goto err_help;
    }
    switch(argc) {
         case 3:
             arg[2] = cmd_getarg(cmd, 2);
         case 2:
             arg[1] = cmd_getarg(cmd, 1);
         case 1:
             arg[0] = cmd_getarg(cmd, 0);
             break;
         case 0:
         default:
             goto err_help;
    }


    opt_id = opt_to_id(arg[1]);
 
    if ((strcmp(arg[0], "get") == 0)) {
        if (opt_id == CMD_OTPCFG_MAP_END) {
            goto err_help;
        }
        if (opt_id == CMD_OTPCFG_MODE_BOOTROM) {
            opt_id = CMD_OTPCFG_MODE_CUST_BOOTROM;
        }
        rc = get_val(opt_id, &res);
        if (rc) {
            goto err;
        }
        switch(opt_id) {
                case CMD_OTPCFG_OP_MRKTID:
                case CMD_OTPCFG_MFG_MRKTID:
    	             /*res = htons(res);*/
                case CMD_OTPCFG_MODE_OP: {
	             if (res) {
                         printf("\nThe customer %s otp bits are fused to 0x%x\n", otpcfg_opts[opt_id],res);
                     } else {
                         printf("\nThe customer %s otp bits are not fused \n", otpcfg_opts[opt_id]);
                     }
                     break;
                }
                case CMD_OTPCFG_MODE_CUST_BOOTROM: {
	             if (res) {
                         printf("\nThe customer %s otp bits are fused to 0x%x\n", otpcfg_opts[CMD_OTPCFG_MODE_BOOTROM],res);
                     } else {
                         printf("\nThe customer %s otp bits are not fused \n", otpcfg_opts[CMD_OTPCFG_MODE_BOOTROM]);
                     }
                     break;
                }
                default:
                     break;
       }

    } else if (strcmp(arg[0], "set") == 0) {
         switch(opt_id) { 
                case CMD_OTPCFG_MODE_BOOTROM: 
                     printf("\nThe SoC is about to be configured in a Bootrom/Secure mode. \n"
                            "\nThe SoC will not boot to other than a Bootrom/Secure mode once \n"
                            "\nchanges are made permanent.\n");
                     printf("\nFusing customer %s otp bit ... ", otpcfg_opts[CMD_OTPCFG_MODE_BOOTROM]);
                     if (warning_confirm()) {
                         return 0;
                     }
                     res = 7; 
                     opt_id = CMD_OTPCFG_MODE_CUST_BOOTROM;
                     rc = set_val(opt_id, res);
                     if (rc) {
                         goto err;
                     }
                     printf("OK \n");
                     res = 1;
                     printf("\nFusing brcm %s otp bit ... ", otpcfg_opts[CMD_OTPCFG_MODE_BOOTROM]);
                     opt_id = CMD_OTPCFG_MODE_BRCM_BOOTROM;
                     break;
                case CMD_OTPCFG_MODE_OP: {
                     if (warning_confirm()) {
                         return 0;
                     }
                     printf("\nFusing customer %s otp bit ... ", otpcfg_opts[opt_id]);
                     res  = 1;
                     break;
                }
                case CMD_OTPCFG_OP_MRKTID: 
                case CMD_OTPCFG_MFG_MRKTID: {
                     if (argc < 3) {
                       	 goto err_help; 
                     }
		     res = atoi(arg[2]);
		     if (warning_confirm()) {
                         return 0;
                     }

    		     /*res = htons(res);*/

                     printf("\nFusing  customer %s 0x%x ... ", otpcfg_opts[opt_id], res);
                     break;
                }
                default:
                     goto err_help;
       }
       rc = set_val(opt_id, res);
       if (rc) {
           goto err;
       }
       printf("OK \n");
   }  else 
       goto err_help;

   return rc;
err_help:
      cmd_help();
err:
    return rc;
}

/**********************************************************************
 *  int ui_init_otp_cmds(void) 
 *  
 *  Input parameters: 
 *      none
 *      
 *  Return value:
 *      0 - everything worked as expected
 *
 ********************************************************************* */
int ui_init_otp_cmds() {

    cmd_addcmd("otpcfg",
           otpcfg,
           NULL,
           "Reading/fusing potp/sotp bits ",
           "otpcfg",
           "");

    return( 0 );
}
