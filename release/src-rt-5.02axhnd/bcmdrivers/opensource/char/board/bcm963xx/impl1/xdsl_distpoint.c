#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/tty.h>
#include <linux/sched.h>  // for current
#include <linux/seq_file.h>

#include "shared_utils.h"
#include <board.h>
#include <boardparms.h>
#include "bcm_gpio.h"

#define XDSL_DISTPOINT_PROC_ENTRY_NAME "xdsl_distpoint"

// Hardware related xdsl distpoint information (see board parameters)
XDSL_DISTPOINT_INFO xdslDistpointInfo;

#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];
int buffer_len;

static void _show_reset_status(void);
static void _show_gpio_value(void);
static int _set_gpio_value(unsigned short gpio, unsigned short value);

//{{{ tty
static int mtty_write(char *text)
{
    struct tty_struct *my_tty;
    static int null_tty = 0;

    my_tty = current->signal->tty;

    if (my_tty != NULL)
        tty_write_message(my_tty, text);
    else
        null_tty++;

    return strlen (text);
}

static int mtty_writeln(char *text)
{
    int r = 0;
    r += mtty_write(text);
    r += mtty_write ("\015\012");  /* crlf */
    return r;
}

static int mtty_writeln_buffer(void)
{
    int r = 0;
    int i = 0;

    buffer[buffer_len + 1] = '\0';
    r = mtty_writeln(buffer);
    for (i=0;i<BUFFER_SIZE;i++) {
        buffer[i] = ' ';
    }
    return r;
}
//}}}
//{{{ proc entry
static void _proc_show_supported_commands(void)
{
    mtty_writeln("supported commands:");
    mtty_writeln("    reset <enter|exit> <name|id>");
    mtty_writeln("    gpio [<id> <value>]");
    mtty_writeln("    status");
}

static int _proc_show(struct seq_file *f, void *v)
{
    seq_printf(f, "Usage: echo command > /proc/" XDSL_DISTPOINT_PROC_ENTRY_NAME "\n\n");
    seq_printf(f, "supported commands: \n");
    seq_printf(f, "    reset <enter|exit> <name|id>\n");
    seq_printf(f, "    gpio [<id> <value>]\n");
    seq_printf(f, "    status\n");

    return 0;
}

static int _proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, _proc_show, NULL);
}

static int _proc_control_reset(char * cmd, char * id_or_name)
{
    unsigned short i = 0;
    unsigned long j = 0;
    unsigned short gpio_num = 0;
    int is_user_gpio = 0;

    // search by id
    if (0 == kstrtoul(id_or_name, 10, &j)) {
        gpio_num = (unsigned short) j;
        for (i=0;i<xdslDistpointInfo.nbReset;i++) {
             if (gpio_num == (xdslDistpointInfo.reset[i].gpio & BP_GPIO_NUM_MASK)) {
                is_user_gpio = 1;
                break;
            }
        }
    }

    // search by name
    if (0 == is_user_gpio) {
        for (i=0;i<xdslDistpointInfo.nbReset;i++) {
            if (0 == strcmp(xdslDistpointInfo.reset[i].name, id_or_name)) {
                is_user_gpio = 1;
                gpio_num = xdslDistpointInfo.reset[i].gpio;
                break;
            }
        }
    }

    if (0 == is_user_gpio) {
        buffer_len = sprintf(buffer, "reset %s is not user controllable", id_or_name);
        mtty_writeln_buffer();
        return -EFAULT;
    }

    if (0 == strcmp(cmd, "enter")) {
        buffer_len = sprintf(
                buffer,
                "enter reset on %s (gpio %d)",
                xdslDistpointInfo.reset[i].name,
                xdslDistpointInfo.reset[i].gpio & BP_GPIO_NUM_MASK);
        mtty_writeln_buffer();
        kerSysSetGpioState(gpio_num, kGpioActive);
    }
    else if (0 == strcmp(cmd, "exit")) {
        buffer_len = sprintf(
                buffer,
                "exit reset on %s (gpio %d)",
                xdslDistpointInfo.reset[i].name,
                gpio_num & BP_GPIO_NUM_MASK);
        mtty_writeln_buffer();
        kerSysSetGpioState(gpio_num, kGpioInactive);
    }
    else {
        buffer_len = sprintf(buffer, "unknown reset command %s", cmd);
        mtty_writeln_buffer();
        return -EFAULT;
    }

    return 0;
}

static ssize_t _proc_write(struct file *f, const char __user *buf, size_t cnt,
                           loff_t *pos)
{
#define MAX_ARGS 3
#define MAX_ARG_SIZE 32
#define KBUF_SIZE (MAX_ARGS*MAX_ARG_SIZE)
    int i;
    int argc = 0;
    char arg[MAX_ARGS][MAX_ARG_SIZE];
    char kbuf[KBUF_SIZE];
    unsigned long gpio_num = 0;
    unsigned long gpio_value = 0;

    if (cnt > KBUF_SIZE)
        cnt = KBUF_SIZE;
    if (copy_from_user(kbuf, buf, cnt))
        return -EFAULT;
    kbuf[cnt]='\0';

    argc = sscanf(kbuf, "%s %s %s", arg[0], arg[1], arg[2]);
    if (argc < 1) {
        mtty_writeln("Need at-least 1 argument");
        return -EFAULT;
    }
    for (i=0; i<MAX_ARGS; ++i) {
        arg[i][MAX_ARG_SIZE-1] = '\0';
    }

    if (0 == strcmp(arg[0], "reset")) {
        if (3 != argc) {
            mtty_writeln("Invalid number of arguments for reset command.");
            mtty_writeln("reset command: reset <enter|exit> <name>");
            return -EFAULT;
        }
        if (_proc_control_reset(arg[1], arg[2])) {
            return -EFAULT;
        }
    }
    else if (0 == strcmp(arg[0], "gpio")) {
        switch (argc) {
            case 1:
                _show_gpio_value();
                break;
            case 3:
                if (kstrtoul(arg[1], 10, &gpio_num)) {
                    buffer_len = sprintf(buffer, "invalid gpio id %s", arg[1]);
                    mtty_writeln_buffer();
                    return -EFAULT;
                }
                if (kstrtoul(arg[2], 10, &gpio_value)) {
                    buffer_len = sprintf(buffer, "invalid gpio value %s", arg[2]);
                    mtty_writeln_buffer();
                    return -EFAULT;
                }
                if (_set_gpio_value((unsigned short) gpio_num, (unsigned short) gpio_value)) {
                    return -EFAULT;
                }
                break;
            default:
                mtty_writeln("Invalid number of arguments for gpio command.");
                mtty_writeln("gpio command: gpio [<id> [value]]");
                return -EFAULT;
        }
    }
    else if (0 == strcmp(arg[0], "status")) {
        _show_reset_status();
        _show_gpio_value();
    }
    else {
        mtty_writeln("Invalid command.");
        _proc_show_supported_commands();
        return -EFAULT;
    }

    return cnt;
}

static void _show_reset_status(void)
{
    unsigned short i = 0;
    unsigned short gpio;
    int max_name_len = 0;
    int name_len = 0;
    int is_active_low = 0;

    for (i=0;i<xdslDistpointInfo.nbReset;i++) {
        name_len = strlen(xdslDistpointInfo.reset[i].name);
        if (name_len > max_name_len)
            max_name_len = name_len;
    }

    mtty_writeln("+-------------------------+");
    mtty_writeln("| status of reset outputs |");
    mtty_writeln("+-------------------------+");
    buffer_len = sprintf(buffer, "%-*s\tvalue\tgpio", max_name_len, "name");
    mtty_writeln_buffer();

    for (i=0;i<xdslDistpointInfo.nbReset;i++) {
        buffer_len = sprintf(buffer, "%-*s\t", max_name_len, xdslDistpointInfo.reset[i].name);
        gpio = xdslDistpointInfo.reset[i].gpio & BP_GPIO_NUM_MASK;
        buffer_len += sprintf(buffer + buffer_len, "%-5d\t", kerSysGetGpioValue(gpio));
        buffer_len += sprintf(buffer + buffer_len, "%d", gpio);
        is_active_low = xdslDistpointInfo.reset[i].gpio & BP_ACTIVE_MASK;
        buffer_len += sprintf(buffer + buffer_len, " (%s)", is_active_low? "active low":"active high");
        mtty_writeln_buffer();
    }
    mtty_writeln("");
}

static void _show_gpio_value(void)
{
    unsigned short i = 0;
    unsigned short gpio;
    int max_info_len = 0;
    int info_len = 0;
    unsigned short value = 0;

    for (i=0;i<xdslDistpointInfo.nbGpio;i++) {
        info_len = strlen(xdslDistpointInfo.gpio[i].info);
        if (info_len > max_info_len)
            max_info_len = info_len;
    }

    mtty_writeln("+-----------------------------------------------------+");
    mtty_writeln("| status of user controllable general purpose outputs |");
    mtty_writeln("+-----------------------------------------------------+");
    buffer_len = mtty_write("gpio\t");
    buffer_len += sprintf(buffer + buffer_len, "%-*s\t", max_info_len, "info");
    buffer_len += sprintf(buffer + buffer_len, "value");
    mtty_writeln_buffer();

    for (i=0;i<xdslDistpointInfo.nbGpio;i++) {
        gpio = xdslDistpointInfo.gpio[i].gpio & BP_GPIO_NUM_MASK;
        buffer_len = sprintf(buffer, "%d\t", gpio);
        value = kerSysGetGpioValue(gpio);
        buffer_len += sprintf(buffer + buffer_len, "%-*s\t", max_info_len, xdslDistpointInfo.gpio[i].info);
        buffer_len += sprintf(buffer + buffer_len, "%d", value);
        buffer_len += sprintf(buffer + buffer_len, " (%s)", value ? xdslDistpointInfo.gpio[i].infoValue1:xdslDistpointInfo.gpio[i].infoValue0);
        mtty_writeln_buffer();
    }
    mtty_writeln("");
}

static int _set_gpio_value(unsigned short gpio, unsigned short value)
{
    unsigned short i;
    int is_user_gpio = 0;

    for (i=0;i<xdslDistpointInfo.nbGpio;i++) {
        if ((xdslDistpointInfo.gpio[i].gpio & BP_GPIO_NUM_MASK) == gpio) {
            is_user_gpio = 1;
            break;
        }
    }
    if (0 == is_user_gpio) {
        buffer_len = sprintf(buffer, "gpio %d is not user controllable", gpio);
        mtty_writeln_buffer();
        return -EFAULT;
    }
    if ((0 != value) && (1 != value)) {
        buffer_len = sprintf(buffer, "invalid gpio value %d", value);
        mtty_writeln_buffer();
        return -EFAULT;
    }

    buffer_len = sprintf(buffer, "set gpio %d (%s) to value %d (%s)",
                         gpio,
                         xdslDistpointInfo.gpio[i].info,
                         value,
                         value ? xdslDistpointInfo.gpio[i].infoValue1:
                                 xdslDistpointInfo.gpio[i].infoValue0);
    mtty_writeln_buffer();
    kerSysSetGpioState(gpio & BP_GPIO_NUM_MASK, value);

    return 0;
}

static int _create_xdsl_distpoint_proc_entry(void)
{
    struct proc_dir_entry *p0;
    static const struct file_operations proc_file_fops = {
        .owner      = THIS_MODULE,
        .open       = _proc_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = seq_release,
        .write      = _proc_write
    };

    printk(KERN_INFO "Create xdsl_distpoint proc entry.\n");
    p0 = proc_create(XDSL_DISTPOINT_PROC_ENTRY_NAME, 0644, NULL, &proc_file_fops);
    if (p0 == NULL) {
        printk(KERN_WARNING "Failed to create xdsl_distpoint proc entry.\n");
        return -1;
    }

    return 0;
}

static int _del_proc_files(void)
{
    remove_proc_entry(XDSL_DISTPOINT_PROC_ENTRY_NAME, NULL);
    return 0;
}
//}}}

static void _init_gpio(void)
{
    unsigned short i;

    for (i=0;i<xdslDistpointInfo.nbGpio;i++) {
        kerSysSetGpioState(xdslDistpointInfo.gpio[i].gpio & BP_GPIO_NUM_MASK,
                       xdslDistpointInfo.gpio[i].initValue);
        kerSysSetGpioDir(xdslDistpointInfo.gpio[i].gpio & BP_GPIO_NUM_MASK);
        set_pinmux(xdslDistpointInfo.gpio[i].gpio & BP_GPIO_NUM_MASK, 5);
    }
}

static void _init_reset(void)
{
    unsigned short i;
    char boardid[NVRAM_BOARD_ID_STRING_LEN];

    kerSysNvRamGetBoardId(boardid);
    if (!strcmp(boardid, "BCM955045DPU"))
    {
        kerSysSetGpioState(12, kGpioInactive);
        kerSysSetGpioDir(1);
        set_pinmux(12, 5);
    }

    for (i=0;i<xdslDistpointInfo.nbReset;i++) {
        kerSysSetGpioState(xdslDistpointInfo.reset[i].gpio, kGpioActive);
        kerSysSetGpioDir(xdslDistpointInfo.reset[i].gpio);
        set_pinmux(xdslDistpointInfo.reset[i].gpio & BP_GPIO_NUM_MASK, 5);
        printk(KERN_INFO "[XDSL_DISTPOINT] init reset on %d\n", xdslDistpointInfo.reset[i].gpio & BP_GPIO_NUM_MASK);
    }
}

static void _release_reset(void)
{
    unsigned short i;

    printk(KERN_INFO "[XDSL_DISTPOINT] check releaseOnInit on %d signals.\n", xdslDistpointInfo.nbReset);

    for (i=0;i<xdslDistpointInfo.nbReset;i++) {
        if (xdslDistpointInfo.reset[i].releaseOnInit) {
            printk(KERN_INFO "[XDSL_DISTPOINT] release %s\n", xdslDistpointInfo.reset[i].name);
            kerSysSetGpioState(xdslDistpointInfo.reset[i].gpio, kGpioInactive);
        } else {
            printk(KERN_INFO "[XDSL_DISTPOINT] no release on %s\n", xdslDistpointInfo.reset[i].name);
        }
    }
}

static int __init _xdsl_distpoint_init(void)
{
    BpGetXdslDistpointInfo(&xdslDistpointInfo);
    _init_reset();
    _init_gpio();
    _release_reset();
    _create_xdsl_distpoint_proc_entry();

    return 0;
}

static void __exit _xdsl_distpoint_exit(void)
{
    _del_proc_files();
}

module_init(_xdsl_distpoint_init);
module_exit(_xdsl_distpoint_exit);
