/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

#include <bcmtypes.h>
#include <linux/types.h>

#include "bcm_OS_Deps.h"

#include <linux/bcm_log.h>
#include "chal_types.h"
#include "chal_simio.h"

#include <simio_def_common.h>
#include "simio.h"
#include "simio_board.h"

#include "sim_export_defs.h"

#include "shared_utils.h"


static int sim_card_proc_handler(struct ctl_table *ctl, int write, 
    void __user *buffer, size_t *res, loff_t *ppos);

char sim_card_str[30];
static ctl_table sim_card_child_table[] = {
    {
        .procname	= "bcm_sim_card",
        .data           = &sim_card_str,
        .maxlen		= sizeof(sim_card_str),
        .mode		= 0644,
        .proc_handler	= &sim_card_proc_handler,
    },
    { }
};

static ctl_table sim_card_root_table[] = {
    {
        .procname	= "sim_card",
        .mode		= 0644,
        .child		= sim_card_child_table
    },
    { .procname = NULL }
};

int read_option;
struct ctl_table_header *sim_card_sysctl_header;
static void print_byte_array(UInt8 *data, UInt16 len)
{
    int i;

    for (i=0; i<len;)
    {
        printk( "0x%x ", data[i]);
        i++;
        if (i != len && i%8 == 0)
            printk("\r\n");            
    }
    printk("\r\n");            
}

static int parse_sim_card_cmd(char *buffer)
{
    int ret;
    int main_option;

    ret = sscanf(buffer, "%d", &main_option);
    if (!ret)
    {
        printk(KERN_ERR "%s: invalid value, commands can be either:\n1 - sim card activate\n \
            2 - sim card detection\n3 - sim card baud rate\n5 - sim card write\n6 - sim card read\n",
            /*            control reset protocol*/
            module_name(THIS_MODULE));
        return -1;
    }

    switch(main_option)
    {
    case 1: /*activation*/
        {
            UInt8 data[SIM_CARD_MAX_BUFFER_SIZE];
            int len=SIM_CARD_MAX_BUFFER_SIZE;
            int ret;
            ret=sim_active(data, len);
            printk("from user: atr received:ret=%d\n",ret);
            print_byte_array(data,20);
            return 0;
        }
    case 2: /*detection*/
        {
            int detection_status;
            detection_status = sim_online();
            printk("user: detection_status = %d\n",detection_status);
            return 0;
        }

    case 3: /*baud rate*/
        {
            int F, D;

            ret = sscanf(buffer, "%d %d %d", &main_option, (int *)&F, (int *)&D);
            if (ret < 3)
            {
                printk(KERN_ERR "%s: not enough parametrs. parameters should be :\
                    \nF = 372, 512 D = 1, 8, 16, 32, 64\n",
                    module_name(THIS_MODULE));
                return -1;
            }
            ret=sim_set_baud(F, D);
            printk("user: ret = %d\n",ret);
            return 0;
        }

    case 5: /*write*/
    case 6: /*read*/        
        {
            //  int command;
            UInt8 data[SIM_CARD_MAX_BUFFER_SIZE*2];
            int tx_len=0, rx_len;
            int i;

            ret = sscanf(buffer, "%d %d %s",&main_option, (unsigned int*)&tx_len, data);
            if (ret < 2)
            {
                printk(KERN_ERR "%s: not enough parametrs: parameters should be:\ncommand: 0 -rx 1 - tx\n \
                    \n", module_name(THIS_MODULE));
                return -1;
            }

            printk("\nfrom user: tx data\n"); 
            for (i=0;i<tx_len;i++)
            {
                data[i*2]=HEXOF(data[i*2]);
                data[i*2+1]=HEXOF(data[i*2+1]);
                data[i]=(data[i*2]<<4)+data[i*2+1];
                printk("%02x",data[i]);
            }
            printk("\n");

            if (main_option == 5)
                rx_len=sim_write(data, SIM_CARD_MAX_BUFFER_SIZE);
            else
                rx_len=sim_read(data, SIM_CARD_MAX_BUFFER_SIZE);

            printk("\nfrom user: rx data len = %d\n",rx_len); 
            for (i=0;i<rx_len;i++)
                printk("%02x",data[i]);
            printk("\n");
            return 0;
        }
    case 11: /*protocol*/
        {
            PROTOCOL_t protocol;
            ret = sscanf(buffer, "%d %d", &main_option, (int *)&protocol);
            if (ret < 2)
            {
                printk(KERN_ERR "%s: not enough parametrs: parameters should be:\n0 - protocol T0, 1 - protocol T1\n", \
                    module_name(THIS_MODULE));
                return -1;
            }
            sim_card_protocol(protocol);
            return 0;

        }
    case 12: /*control*/
        {
            int control;
            ret = sscanf(buffer, "%d %d", &main_option, &control);
            if (ret < 2)
            {
                printk(KERN_ERR "%s: not enough parametrs: parameters should be:\ncontrol 0 - disable, 1 - enable\n", \
                    module_name(THIS_MODULE));
                return -1;
            }

            sim_card_control(control);
            return 0;
        }

    case 13: /*reset*/
        {
            SIMIO_DIVISOR_t freq;
            int reset;
            SimVoltageLevel_t voltage;

            ret = sscanf(buffer, "%d %d %d %d", &main_option, (int *)&reset, (int *)&freq, (int *)&voltage);
            if (ret < 4)
            {
                printk(KERN_ERR "%s: not enough parametrs: parameters should be:\nreset: 0 - cold, 1 - warm, \
                    \nfrequency[MHz]: 0 - 4.16, 1 - 3.12, 2 - 2.5, 3 - 2.08, 4 - 1.78, 5 - 1.5, 6 - 1.3, 7 - 1.25, \
                    8 - 1.13, 9 - 1.04\nvoltage level: 0 - 5V, 1 - 3V, 2 - 1.8V\n", module_name(THIS_MODULE));
                return -1;
            }
            sim_card_reset(reset, voltage, freq);

            return 0;
        }

    case 100: /*read*/
        {
            ret = sscanf(buffer, "%d %d", &main_option, &read_option);
            return 0;
        }
    default:
        {
            printk(KERN_ERR "%s: invalid value, commands can be either:\n1 - sim card activate\n \
                2 - sim card detection\n3 - sim card baud rate\n5 - sim card write\n6 - sim card read\n",
                /*            control reset protocol*/
                module_name(THIS_MODULE));
            return 0;
        }
    }
}

static int sim_card_proc_handler(struct ctl_table *ctl, int write, 
    void __user *buffer, size_t *res, loff_t *ppos)
{
    char kbuffer[SIM_CARD_MAX_BUFFER_SIZE*2];

    if (copy_from_user(kbuffer, buffer, *res))
        return -EFAULT;

    if (write)
        parse_sim_card_cmd(kbuffer);

    else
    {
        switch (read_option)
        {
        case 1:
            strncpy(sim_card_str, "control", sizeof(sim_card_str));
            break;
        case 2:
            strncpy(sim_card_str, "reset", sizeof(sim_card_str));
            break;
        case 3:
            strncpy(sim_card_str, "pps", sizeof(sim_card_str));
            break;
        case 4:
            strncpy(sim_card_str, "command", sizeof(sim_card_str));
            break;

        }
    }

    return proc_dostring(ctl, write, buffer, res, ppos);
}

void simio_callback(SIMIO_SIGNAL_t simio_sig, UInt16 rsp_len, UInt8 *rsp_data)
{
    switch (simio_sig){
    case SIMIO_SIGNAL_ATRCORRUPTED:
        printk("user: atr corrupted\n");
        break;
    case SIMIO_SIGNAL_ATR_WRONG_VOLTAGE:
        printk("user: wrong voltage applied\n");
        break;
    case SIMIO_SIGNAL_SIMINSERT:
        printk("user: insert\n");
        break;
    case SIMIO_SIGNAL_RSPDATA:
        break;
    case SIMIO_SIGNAL_SIMREMOVED:
        printk("user: removed\n");
        break;
    case SIMIO_SIGNAL_TIMEOUT:
    case SIMIO_SIGNAL_IDLE:
    case SIMIO_SIGNAL_T1_PARITY:
    case SIMIO_SIGNAL_T1_INVALID_LENGTH:
    case SIMIO_SIGNAL_T1_BWT_TIME_OUT:
    case SIMIO_SIGNAL_SIMRESET:
    default:
        break;
    }
}

void simio_detect_callback(Boolean insert)
{
    if (insert == 0)
        printk("in user: card out\n");
    else
        printk("in user: card in\n");
}

void simio_user_register(void)
{
    sim_card_sysctl_header = register_sysctl_table(sim_card_root_table);
    if (!sim_card_sysctl_header)
        printk(KERN_ALERT "sim_card_root_table init");

    SIMIO_RegisterCB(SIMIO_ID_0, simio_callback, NULL);
    SIMIO_RegisterDetectionCB(SIMIO_ID_0, simio_detect_callback);
}

