/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/aio.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include <linux/delay.h>        /* For delay */

#include "bdmf_system.h"
#include "bdmf_session.h"
#include "bdmf_shell.h"
#include "bdmf_chrdev.h"

int bdmf_chrdev_major = 321; /* Should comply the value in targets/makeDev */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
module_param(bdmf_chrdev_major, int, S_IRUSR | S_IRGRP | S_IWGRP);
#else
module_param(bdmf_chrdev_major, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
MODULE_DESCRIPTION("BDMF shell module");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");

#define BDMF_CHRDEV_MAX_SESS      16

/* #define DEBUG */

#ifdef DEBUG
#define dprintk printk
#define CHRDEV_LOCK(lock) \
    ({ int __rc; \
        printk("%s:%d taking lock..\n", __FUNCTION__, __LINE__); \
        __rc = bdmf_mutex_lock(lock); \
        printk("%s:%d ..lock taken\n", __FUNCTION__, __LINE__);  \
        __rc; })
#define CHRDEV_UNLOCK(lock) \
    do { \
        printk("%s:%d releasing lock..\n", __FUNCTION__, __LINE__); \
        bdmf_mutex_unlock(lock); \
    } while (0)
#else
#define dprintk(...)
#define CHRDEV_LOCK(lock)           bdmf_mutex_lock(lock)
#define CHRDEV_UNLOCK(lock)         bdmf_mutex_unlock(lock)
#endif

#define CHRDEV_FIFO_SIZE            (32*1024)

struct bdmf_chrdev_fifo
{
    int rd;
    int wr;
    struct semaphore data_sem;
    char buf[CHRDEV_FIFO_SIZE];
};

struct bdmf_chdev_session
{
    int pid;
    bdmf_session_handle mon_session;
    bdmf_session_handle mon_session_le;
    struct bdmf_chrdev_fifo read_fifo;
    struct bdmf_chrdev_fifo write_fifo;
    bdmf_task read_task;
};

static struct bdmf_chdev_session dev_sessions[BDMF_CHRDEV_MAX_SESS];
static struct bdmf_chdev_session *line_edit_session;

static bdmf_mutex dev_lock;

/*
 * Session helpers
 */

/* Read task handler */
static int bdmf_chrdev_read_handler(void *arg)
{
    struct bdmf_chdev_session *sess = (struct bdmf_chdev_session *)arg;
    while (line_edit_session != NULL)
    {
        if (kthread_should_stop())
            break;
        bdmfmon_driver(sess->mon_session_le);
    }
    return 0;
}

/* Push character into FIFO and wake up reader if necessary
 * called under dev_lock */
static int bdmf_chrdev_fifo_push(struct bdmf_chrdev_fifo *fifo, int c)
{
    int cur_wr, next_wr;
    mb();
    cur_wr = fifo->wr;
    next_wr = fifo->wr + 1;
    if (next_wr == CHRDEV_FIFO_SIZE)
        next_wr = 0;
    if (next_wr == fifo->rd)
        return -ENOSPC; /* Overflow. Shouldn't happen */
    fifo->buf[cur_wr] = c;
    fifo->wr = next_wr;
    dprintk("%s:%d: PUSH: wr=%d rd=%d c=%x(%c) %s\n", __FUNCTION__, __LINE__,
        fifo->wr, fifo->rd, c, c, (cur_wr==fifo->rd) ? "posting sem": "");
    if (cur_wr == fifo->rd)
        up(&fifo->data_sem);
    return 0;
}

/* Pop character from FIFO. Wait if necessary.
 * called under dev_lock
 */
static int bdmf_chrdev_fifo_pop(struct bdmf_chrdev_fifo *fifo)
{
    int c;
    mb();
    dprintk("%s:%d: POP: wr=%d rd=%d %s\n", __FUNCTION__, __LINE__,
        fifo->wr, fifo->rd, (fifo->wr==fifo->rd) ? "will wait..": "");
    while (fifo->rd == fifo->wr)
    {
        CHRDEV_UNLOCK(&dev_lock);
        if (down_interruptible(&fifo->data_sem))
            return -ERESTARTSYS;
        if (!line_edit_session)
            return -ERESTARTSYS;
        dprintk("%s:%d: POP got sem: wr=%d rd=%d %s\n", __FUNCTION__, __LINE__,
            fifo->wr, fifo->rd, (fifo->wr==fifo->rd) ? "will wait..": "");
        CHRDEV_LOCK(&dev_lock);
        if (!line_edit_session)
        {
            CHRDEV_UNLOCK(&dev_lock);
            return -EBADF;
        }
    }
    c = fifo->buf[fifo->rd];
    if (fifo->rd == CHRDEV_FIFO_SIZE - 1)
        fifo->rd = 0;
    else
        ++fifo->rd;
    dprintk("%s:%d: POP: wr=%d rd=%d c=%x(%c)\n", __FUNCTION__, __LINE__,
         fifo->wr, fifo->rd, c, c);
    return c;
}


/* line-edit session write callback */
static int bdmf_chrdev_session_write_cb(bdmf_session_handle session, const char *buf, uint32_t size)
{
    int i;
    dprintk("%s:%d: size=%u\n", __FUNCTION__, __LINE__, size);
    for (i=0; i<size; i++)
        bdmf_putchar(buf[i]);
    return i;
}

/* Look for an empty slot in dev_sessions array,
 * and create a new session.
 * Returns unique pid or -EBUSY if no free skots
 */
static int bdmf_chrdev_sess_new(struct bdmf_chdev_session **p_sess)
{
    bdmf_session_parm_t mon_session_parm;
    struct bdmf_chdev_session *sess;
    int line_edit_mode = 0;
    int i;
    int rc;

#ifdef CONFIG_LINENOISE
    line_edit_mode = 1;
#endif
    for(i=0; i<BDMF_CHRDEV_MAX_SESS && dev_sessions[i].pid; i++)
        ;
    if (i >= BDMF_CHRDEV_MAX_SESS)
        return -EBUSY;

    /* Only 1 session with full featured line editing is supported */
    if (line_edit_session != NULL)
        line_edit_mode = 0;

    /* Create shell session */
    sess = &dev_sessions[i];
    memset(&mon_session_parm, 0, sizeof(mon_session_parm));
    mon_session_parm.access_right = BDMF_ACCESS_ADMIN;
    rc = bdmfmon_session_open(&mon_session_parm, &sess->mon_session);
    if (rc)
        return -ENOMEM;
    if (line_edit_mode)
    {
        mon_session_parm.write = bdmf_chrdev_session_write_cb;
        rc = bdmfmon_session_open(&mon_session_parm, &sess->mon_session_le);
        if (rc)
        {
            bdmfmon_session_close(sess->mon_session);
            return -ENOMEM;
        }
    }
    sess->pid = current->pid;

    /* If line editing mode - create read task and semaphores */
    if (line_edit_mode)
    {
        line_edit_session = sess;
        sema_init(&sess->read_fifo.data_sem, 0);
        sema_init(&sess->write_fifo.data_sem, 0);
        rc = bdmf_task_create("kbdmf_shell", BDMFSYS_DEFAULT_TASK_PRIORITY, BDMFSYS_DEFAULT_TASK_STACK,
            bdmf_chrdev_read_handler, sess, &sess->read_task);
        if (rc)
        {
            bdmfmon_session_close(sess->mon_session);
            bdmfmon_session_close(sess->mon_session_le);
            line_edit_session = NULL;
            return -EINVAL;
        }
    }
    rc = sess->pid;
    dprintk("%s: sess %d. le_sess=%p read_fifo=%p write_fifo=%p\n", __FUNCTION__, rc, sess->mon_session_le, &sess->read_fifo, &sess->write_fifo);
    dprintk("%s: line_edit_session=%p\n", __FUNCTION__, &line_edit_session);
    *p_sess = sess;

    return rc;
}

static bdmf_session_handle bdmf_chrdev_sess_get(int session_id)
{
    int i;
    bdmf_session_handle sess=NULL;

    for(i=0; i<BDMF_CHRDEV_MAX_SESS; i++)
    {
        if (dev_sessions[i].pid == session_id)
        {
            sess = dev_sessions[i].mon_session;
            break;
        }
    }
    if (!sess)
        bdmf_print("BDMF SHELL: session %d is not found\n", session_id);
    return sess;
}

static int bdmf_chrdev_sess_free(int session_id)
{
    int i;

    dprintk("%s:%d:\n", __FUNCTION__, __LINE__);
    for(i=0; i<BDMF_CHRDEV_MAX_SESS && dev_sessions[i].pid != session_id; i++)
        ;
    if (i >= BDMF_CHRDEV_MAX_SESS)
    {
        bdmf_print("BDMF SHELL: session %d is not found\n", session_id);
        return -ENOENT;
    }
    if (dev_sessions[i].mon_session_le)
    {
        struct bdmf_chrdev_fifo *fifo = &line_edit_session->read_fifo;
        dprintk("%s:%d: killing read task...\n", __FUNCTION__, __LINE__);
        line_edit_session = NULL;
        up(&fifo->data_sem);
        bdmf_task_destroy(dev_sessions[i].read_task);
        dprintk("%s:%d: read task killed\n", __FUNCTION__, __LINE__);
        bdmfmon_session_close(dev_sessions[i].mon_session_le);
    }
    bdmfmon_session_close(dev_sessions[i].mon_session);
    memset(&dev_sessions[i], 0, sizeof(struct bdmf_chdev_session));
    dprintk("%s:%d: OUT\n", __FUNCTION__, __LINE__);

    return 0;
}


/*
 * Open and close
 */
int bdmf_chrdev_open(struct inode *inode, struct file *filp)
{
    dprintk("%s:%d\n", __FUNCTION__, __LINE__);
    return 0;
}


int bdmf_chrdev_release(struct inode *inode, struct file *filp)
{
    dprintk("%s:%d\n", __FUNCTION__, __LINE__);
    return 0;
}

/* write user input.
 * This function is only called in interactive line-editing mode
 */
static ssize_t bdmf_chrdev_write(struct file *filp, const char __user *buf,
    size_t count, loff_t *f_pos)
{
    int i;
    char c;
    int rc;

    /* bdmf_shell application writes data. Push it into read FIFO */
    rc = CHRDEV_LOCK(&dev_lock);
    if (rc)
        return rc;
    if (!line_edit_session)
    {
        CHRDEV_UNLOCK(&dev_lock);
        return -EOPNOTSUPP;
    }
    for (i = 0; i < count; i++)
    {
        if (get_user(c, buf + i))
        {
            rc = -EFAULT;
            break;
        }
        rc = bdmf_chrdev_fifo_push(&line_edit_session->read_fifo, c);
        if (rc)
            break;
    }
    CHRDEV_UNLOCK(&dev_lock);
    dprintk("%s:%d: OUT: count=%u rc=%d\n", __FUNCTION__, __LINE__, count, rc);
    return rc ? rc : count;
}

/* Read CLI output.
 * This function is only called in interactive line-editing mode
 */
static ssize_t bdmf_chrdev_read(struct file *filp, char *buffer,
    size_t length, loff_t *offset)
{
    int i = 0;
    char c;
    int rc;

    if (!length)
        return length;

    rc = CHRDEV_LOCK(&dev_lock);
    if (rc)
        return rc;
    if (!line_edit_session)
    {
        CHRDEV_UNLOCK(&dev_lock);
        return -EOPNOTSUPP;
    }

    /* The 1st call to FIFO_POP can lock on semaphore.
     * Other calls will go right through because of write_fifo.rd != write_fifo.wr check
     */
    do {
        c = bdmf_chrdev_fifo_pop(&line_edit_session->write_fifo);
        put_user(c, &buffer[i++]);
    } while (i < length && c >= 0 &&
        line_edit_session->write_fifo.rd != line_edit_session->write_fifo.wr);

    CHRDEV_UNLOCK(&dev_lock);

    dprintk("%s:%d: size=%u\n", __FUNCTION__, __LINE__, length);

    return (c >= 0) ? i : c;
}

long bdmf_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct io_param *shell_io_param;
    int rc = 0;

    dprintk("%s:%d cmd=0x%x\n", __FUNCTION__, __LINE__, cmd);

    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != BDMF_CHRDEV_IOC_MAGIC)
        return -ENOTTY;

    if (_IOC_NR(cmd) > BDMF_CHRDEV_MAX_NR)
        return -ENOTTY;

    /*
     * the type is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. Note that the type is user-oriented, while
     * verify_area is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
        rc = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        rc =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (rc)
        return -EFAULT;

    shell_io_param = kmalloc(sizeof(*shell_io_param), GFP_KERNEL);
    if (!shell_io_param)
        return -ENOMEM;

    rc = CHRDEV_LOCK(&dev_lock);
    if (rc)
        return rc;

    switch(cmd) {

    case BDMF_CHRDEV_SESSION_INIT:
    {
        struct bdmf_chdev_session *sess = NULL;
        rc = copy_from_user((char *)shell_io_param, (char *)arg, offsetof(struct io_param, command));
        if (rc < 0)
            break;
        shell_io_param->rc = 0;
        /* On input shell_io_param->session_id contains line_editing support flag */
        rc = bdmf_chrdev_sess_new(&sess);
        if (rc < 0)
            break;
        shell_io_param->session_id = sess->pid;
        rc = copy_to_user((char *)arg, (char *)shell_io_param, sizeof(*shell_io_param)-
            sizeof(shell_io_param->command));
    }
    break;

    case BDMF_CHRDEV_SESSION_SEND: /* Tell: arg is the value */
    {
        bdmf_session_handle sess;
        rc = copy_from_user((char *)shell_io_param, (char *)arg, sizeof(*shell_io_param));
        if (rc < 0)
            break;
        sess = bdmf_chrdev_sess_get(shell_io_param->session_id);
        if (!sess)
        {
            rc = -ENOENT;
            break;
        }
        dprintk("%s:%d cmd=<%s>", __FUNCTION__, __LINE__, shell_io_param->command);
        shell_io_param->rc = bdmfmon_parse(sess, shell_io_param->command);
        rc = copy_to_user((char *)arg, (char *)shell_io_param, sizeof(*shell_io_param)-
            sizeof(shell_io_param->command));
        dprintk("%s:%d rc=%d io_rc=%d\n", __FUNCTION__, __LINE__, shell_io_param->rc, rc);
    }
    break;

    case BDMF_CHRDEV_SESSION_CLOSE:
    {
        rc = copy_from_user((char *)shell_io_param, (char *)arg, sizeof(*shell_io_param)-
            sizeof(shell_io_param->command));
        rc = rc ? rc : bdmf_chrdev_sess_free(shell_io_param->session_id);
    }
    break;

    default:  /* redundant, as cmd was checked against MAXNR */
        rc = -ENOTTY;
        break;
    }
    CHRDEV_UNLOCK(&dev_lock);
    dprintk("%s:%d rc=%d\n", __FUNCTION__, __LINE__, rc);
    kfree(shell_io_param);

    return rc;
}


/*
 * Character IO
 */

/* Read character from STDIN */
int bdmf_getchar(void)
{
    int c;
    CHRDEV_LOCK(&dev_lock);
    if (!line_edit_session)
    {
        CHRDEV_UNLOCK(&dev_lock);
        return -EBADF;
    }
    c = bdmf_chrdev_fifo_pop(&line_edit_session->read_fifo);
    CHRDEV_UNLOCK(&dev_lock);
    return c;
}

/* Write character to STDOUT */
void bdmf_putchar(int c)
{
    CHRDEV_LOCK(&dev_lock);
    if (!line_edit_session)
        bdmf_print("%c", c);
    else
        bdmf_chrdev_fifo_push(&line_edit_session->write_fifo, c);
    CHRDEV_UNLOCK(&dev_lock);
}

#ifdef CONFIG_COMPAT
long bdmf_chrdev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    /* A place-holder for pointer conversion in case it is required */
    return bdmf_chrdev_ioctl(filp, cmd, arg);
}
#endif

/*
 * The fops
 */
struct file_operations bdmf_chrdev_fops = {
    .owner = THIS_MODULE,
    .open = bdmf_chrdev_open,
    .release = bdmf_chrdev_release,
    .write = bdmf_chrdev_write,
    .read = bdmf_chrdev_read,
    .unlocked_ioctl = bdmf_chrdev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = bdmf_chrdev_compat_ioctl,
#endif
};

static struct cdev bdmf_chrdev_cdev;
static int is_chrdev_reg, is_cdev_add;

int bdmf_chrdev_init(void)
{
    dev_t dev = MKDEV(bdmf_chrdev_major, 0);
    int rc;

    is_chrdev_reg = 0;
    is_cdev_add = 0;
    /*
     * Register your major, and accept a dynamic number.
     */
    if (!bdmf_chrdev_major)
        return -1;
    rc = register_chrdev_region(dev, 0, "bdmf_shell");
    if (rc < 0)
        return rc;
    is_chrdev_reg = 1;

    bdmf_mutex_init(&dev_lock);
    cdev_init(&bdmf_chrdev_cdev, &bdmf_chrdev_fops);
    rc = cdev_add(&bdmf_chrdev_cdev, dev, 1);
    if (rc < 0)
        return rc;
    is_cdev_add = 1;

    return 0;
}


void bdmf_chrdev_exit(void)
{
    int i;
    if (is_cdev_add)
        cdev_del(&bdmf_chrdev_cdev);
    if (is_chrdev_reg)
        unregister_chrdev_region(MKDEV(bdmf_chrdev_major, 0), 1);
    for(i=0; i<BDMF_CHRDEV_MAX_SESS; i++)
    {
        if (dev_sessions[i].pid)
            bdmf_chrdev_sess_free(dev_sessions[i].pid);
    }
}

