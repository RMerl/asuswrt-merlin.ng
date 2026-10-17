#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <board.h>
#include <button.h>

typedef enum
{
    BTN_ACT_RESTORE, /* restoredefault */
    BTN_ACT_RESET
} BtnAction_t;

void board_util_restoredefault(BtnAction_t act) {
    char *envp[]= {"HOME=/", "PWD=/bin", NULL};
    char *argv[]= {"/bin/bcm_restoredefault", "", "-s \"reset button\"", NULL};

    if (act == BTN_ACT_RESTORE)
        argv[1] = "-A";
    else if(act == BTN_ACT_RESET)
        argv[1] = "-r";
    else
        return;

    call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}

void btn_hook_rst_to_dflt(unsigned long timeInMs, void* param) 
{
#if !defined(CONFIG_BRCM_IKOS)
    printk(" *** Restore to Factory Default Setting ***\n\n");
    board_util_restoredefault(BTN_ACT_RESTORE);
#endif
}

void btn_hook_reset(unsigned long timeInMs, void* param) 
{
    printk(" *** Restarting System ***\n\n");
    board_util_restoredefault(BTN_ACT_RESET);
}

static int __init reset_button_init(void)
{
    int ret;

    ret = register_button_action("reset_button", "rst_to_dflt", btn_hook_rst_to_dflt);
    if (ret < 0 && ret != -ENODEV)
    {
        printk("reset_button_init: failed to register reset to default action for reset_button\n");
        return -1;
    }

    ret = register_button_action("reset_button", "reset", btn_hook_reset);
    if (ret < 0 && ret != -ENODEV)
    {
        printk("reset_button_init: failed to register reset action for reset_button\n");
        return -1;
    }

    return 0;
}

module_init(reset_button_init);

MODULE_AUTHOR("Vladimir Neyelov (vladimir.neyelov@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA Board Reset Button driver");
MODULE_LICENSE("GPL v2");
