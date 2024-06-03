#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <board.h>
#include <button.h>

void btn_hook_rst_to_dflt(unsigned long timeInMs, void* param) 
{
#if !defined(CONFIG_BRCM_IKOS)
    char buf[256] = {};
    
    printk(" *** Restore to Factory Default Setting ***\n\n");
#if defined(WIRELESS)
    board_util_wl_godefault();
#endif
    kerSysPersistentSet( buf, sizeof(buf), 0 );
#endif
    kernel_restart(NULL);        
}

void btn_hook_reset(unsigned long timeInMs, void* param) 
{
    printk(" *** Restarting System ***\n\n");
    kernel_restart(NULL);
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
