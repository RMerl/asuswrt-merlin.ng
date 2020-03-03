/*
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Added support for a Unix98-style ptmx device.
 *    -- C. Scott Ananian <cananian@alumni.princeton.edu>, 14-Jan-1998
 *
 */

#include <linux/module.h>

#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/bitops.h>
#include <linux/devpts_fs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/poll.h>


#ifdef CONFIG_UNIX98_PTYS
static struct tty_driver *ptm_driver;
static struct tty_driver *pts_driver;
static DEFINE_MUTEX(devpts_mutex);
#endif

static void pty_close(struct tty_struct *tty, struct file *filp)
{
	BUG_ON(!tty);
	if (tty->driver->subtype == PTY_TYPE_MASTER)
		WARN_ON(tty->count > 1);
	else {
		if (test_bit(TTY_IO_ERROR, &tty->flags))
			return;
		if (tty->count > 2)
			return;
	}
	set_bit(TTY_IO_ERROR, &tty->flags);
	wake_up_interruptible(&tty->read_wait);
	wake_up_interruptible(&tty->write_wait);
	spin_lock_irq(&tty->ctrl_lock);
	tty->packet = 0;
	spin_unlock_irq(&tty->ctrl_lock);
	/* Review - krefs on tty_link ?? */
	if (!tty->link)
		return;
	set_bit(TTY_OTHER_CLOSED, &tty->link->flags);
	wake_up_interruptible(&tty->link->read_wait);
	wake_up_interruptible(&tty->link->write_wait);
	if (tty->driver->subtype == PTY_TYPE_MASTER) {
		set_bit(TTY_OTHER_CLOSED, &tty->flags);
#ifdef CONFIG_UNIX98_PTYS
		if (tty->driver == ptm_driver) {
			mutex_lock(&devpts_mutex);
			if (tty->link->driver_data)
				devpts_pty_kill(tty->link->driver_data);
			mutex_unlock(&devpts_mutex);
		}
#endif
		tty_vhangup(tty->link);
	}
}

/*
 * The unthrottle routine is called by the line discipline to signal
 * that it can receive more characters.  For PTY's, the TTY_THROTTLED
 * flag is always set, to force the line discipline to always call the
 * unthrottle routine when there are fewer than TTY_THRESHOLD_UNTHROTTLE
 * characters in the queue.  This is necessary since each time this
 * happens, we need to wake up any sleeping processes that could be
 * (1) trying to send data to the pty, or (2) waiting in wait_until_sent()
 * for the pty buffer to be drained.
 */
static void pty_unthrottle(struct tty_struct *tty)
{
	tty_wakeup(tty->link);
	set_bit(TTY_THROTTLED, &tty->flags);
}

/**
 *	pty_write		-	write to a pty
 *	@tty: the tty we write from
 *	@buf: kernel buffer of data
 *	@count: bytes to write
 *
 *	Our "hardware" write method. Data is coming from the ldisc which
 *	may be in a non sleeping state. We simply throw this at the other
 *	end of the link as if we were an IRQ handler receiving stuff for
 *	the other side of the pty/tty pair.
 */

static int pty_write(struct tty_struct *tty, const unsigned char *buf, int c)
{
	struct tty_struct *to = tty->link;

	if (tty->stopped)
		return 0;

	if (c > 0) {
		/* Stuff the data into the input queue of the other end */
		c = tty_insert_flip_string(to->port, buf, c);
		/* And shovel */
		if (c)
			tty_flip_buffer_push(to->port);
	}
	return c;
}

/**
 *	pty_write_room	-	write space
 *	@tty: tty we are writing from
 *
 *	Report how many bytes the ldisc can send into the queue for
 *	the other device.
 */

static int pty_write_room(struct tty_struct *tty)
{
	if (tty->stopped)
		return 0;
	return tty_buffer_space_avail(tty->link->port);
}

/**
 *	pty_chars_in_buffer	-	characters currently in our tx queue
 *	@tty: our tty
 *
 *	Report how much we have in the transmit queue. As everything is
 *	instantly at the other end this is easy to implement.
 */

static int pty_chars_in_buffer(struct tty_struct *tty)
{
	return 0;
}

/* Set the lock flag on a pty */
static int pty_set_lock(struct tty_struct *tty, int __user *arg)
{
	int val;
	if (get_user(val, arg))
		return -EFAULT;
	if (val)
		set_bit(TTY_PTY_LOCK, &tty->flags);
	else
		clear_bit(TTY_PTY_LOCK, &tty->flags);
	return 0;
}

static int pty_get_lock(struct tty_struct *tty, int __user *arg)
{
	int locked = test_bit(TTY_PTY_LOCK, &tty->flags);
	return put_user(locked, arg);
}

/* Set the packet mode on a pty */
static int pty_set_pktmode(struct tty_struct *tty, int __user *arg)
{
	int pktmode;

	if (get_user(pktmode, arg))
		return -EFAULT;

	spin_lock_irq(&tty->ctrl_lock);
	if (pktmode) {
		if (!tty->packet) {
			tty->link->ctrl_status = 0;
			smp_mb();
			tty->packet = 1;
		}
	} else
		tty->packet = 0;
	spin_unlock_irq(&tty->ctrl_lock);

	return 0;
}

/* Get the packet mode of a pty */
static int pty_get_pktmode(struct tty_struct *tty, int __user *arg)
{
	int pktmode = tty->packet;
	return put_user(pktmode, arg);
}

/* Send a signal to the slave */
static int pty_signal(struct tty_struct *tty, int sig)
{
	struct pid *pgrp;

	if (sig != SIGINT && sig != SIGQUIT && sig != SIGTSTP)
		return -EINVAL;

	if (tty->link) {
		pgrp = tty_get_pgrp(tty->link);
		if (pgrp)
			kill_pgrp(pgrp, sig, 1);
		put_pid(pgrp);
	}
	return 0;
}

static void pty_flush_buffer(struct tty_struct *tty)
{
	struct tty_struct *to = tty->link;

	if (!to)
		return;

	tty_buffer_flush(to, NULL);
	if (to->packet) {
		spin_lock_irq(&tty->ctrl_lock);
		tty->ctrl_status |= TIOCPKT_FLUSHWRITE;
		wake_up_interruptible(&to->read_wait);
		spin_unlock_irq(&tty->ctrl_lock);
	}
}

static int pty_open(struct tty_struct *tty, struct file *filp)
{
	if (!tty || !tty->link)
		return -ENODEV;

	if (test_bit(TTY_OTHER_CLOSED, &tty->flags))
		goto out;
	if (test_bit(TTY_PTY_LOCK, &tty->link->flags))
		goto out;
	if (tty->driver->subtype == PTY_TYPE_SLAVE && tty->link->count != 1)
		goto out;

	clear_bit(TTY_IO_ERROR, &tty->flags);
	clear_bit(TTY_OTHER_CLOSED, &tty->link->flags);
	set_bit(TTY_THROTTLED, &tty->flags);
	return 0;

out:
	set_bit(TTY_IO_ERROR, &tty->flags);
	return -EIO;
}

static void pty_set_termios(struct tty_struct *tty,
					struct ktermios *old_termios)
{
	/* See if packet mode change of state. */
	if (tty->link && tty->link->packet) {
		int extproc = (old_termios->c_lflag & EXTPROC) |
				(tty->termios.c_lflag & EXTPROC);
		int old_flow = ((old_termios->c_iflag & IXON) &&
				(old_termios->c_cc[VSTOP] == '\023') &&
				(old_termios->c_cc[VSTART] == '\021'));
		int new_flow = (I_IXON(tty) &&
				STOP_CHAR(tty) == '\023' &&
				START_CHAR(tty) == '\021');
		if ((old_flow != new_flow) || extproc) {
			spin_lock_irq(&tty->ctrl_lock);
			if (old_flow != new_flow) {
				tty->ctrl_status &= ~(TIOCPKT_DOSTOP | TIOCPKT_NOSTOP);
				if (new_flow)
					tty->ctrl_status |= TIOCPKT_DOSTOP;
				else
					tty->ctrl_status |= TIOCPKT_NOSTOP;
			}
			if (extproc)
				tty->ctrl_status |= TIOCPKT_IOCTL;
			spin_unlock_irq(&tty->ctrl_lock);
			wake_up_interruptible(&tty->link->read_wait);
		}
	}

	tty->termios.c_cflag &= ~(CSIZE | PARENB);
	tty->termios.c_cflag |= (CS8 | CREAD);
}

/**
 *	pty_do_resize		-	resize event
 *	@tty: tty being resized
 *	@ws: window size being set.
 *
 *	Update the termios variables and send the necessary signals to
 *	peform a terminal resize correctly
 */

static int pty_resize(struct tty_struct *tty,  struct winsize *ws)
{
	struct pid *pgrp, *rpgrp;
	struct tty_struct *pty = tty->link;

	/* For a PTY we need to lock the tty side */
	mutex_lock(&tty->winsize_mutex);
	if (!memcmp(ws, &tty->winsize, sizeof(*ws)))
		goto done;

	/* Signal the foreground process group of both ptys */
	pgrp = tty_get_pgrp(tty);
	rpgrp = tty_get_pgrp(pty);

	if (pgrp)
		kill_pgrp(pgrp, SIGWINCH, 1);
	if (rpgrp != pgrp && rpgrp)
		kill_pgrp(rpgrp, SIGWINCH, 1);

	put_pid(pgrp);
	put_pid(rpgrp);

	tty->winsize = *ws;
	pty->winsize = *ws;	/* Never used so will go away soon */
done:
	mutex_unlock(&tty->winsize_mutex);
	return 0;
}

/**
 *	pty_start - start() handler
 *	pty_stop  - stop() handler
 *	@tty: tty being flow-controlled
 *
 *	Propagates the TIOCPKT status to the master pty.
 *
 *	NB: only the master pty can be in packet mode so only the slave
 *	    needs start()/stop() handlers
 */
static void pty_start(struct tty_struct *tty)
{
	unsigned long flags;

	if (tty->link && tty->link->packet) {
		spin_lock_irqsave(&tty->ctrl_lock, flags);
		tty->ctrl_status &= ~TIOCPKT_STOP;
		tty->ctrl_status |= TIOCPKT_START;
		spin_unlock_irqrestore(&tty->ctrl_lock, flags);
		wake_up_interruptible_poll(&tty->link->read_wait, POLLIN);
	}
}

static void pty_stop(struct tty_struct *tty)
{
	unsigned long flags;

	if (tty->link && tty->link->packet) {
		spin_lock_irqsave(&tty->ctrl_lock, flags);
		tty->ctrl_status &= ~TIOCPKT_START;
		tty->ctrl_status |= TIOCPKT_STOP;
		spin_unlock_irqrestore(&tty->ctrl_lock, flags);
		wake_up_interruptible_poll(&tty->link->read_wait, POLLIN);
	}
}

/**
 *	pty_common_install		-	set up the pty pair
 *	@driver: the pty driver
 *	@tty: the tty being instantiated
 *	@legacy: true if this is BSD style
 *
 *	Perform the initial set up for the tty/pty pair. Called from the
 *	tty layer when the port is first opened.
 *
 *	Locking: the caller must hold the tty_mutex
 */
static int pty_common_install(struct tty_driver *driver, struct tty_struct *tty,
		bool legacy)
{
	struct tty_struct *o_tty;
	struct tty_port *ports[2];
	int idx = tty->index;
	int retval = -ENOMEM;

	/* Opening the slave first has always returned -EIO */
	if (driver->subtype != PTY_TYPE_MASTER)
		return -EIO;

	ports[0] = kmalloc(sizeof **ports, GFP_KERNEL);
	ports[1] = kmalloc(sizeof **ports, GFP_KERNEL);
	if (!ports[0] || !ports[1])
		goto err;
	if (!try_module_get(driver->other->owner)) {
		/* This cannot in fact currently happen */
		goto err;
	}
	o_tty = alloc_tty_struct(driver->other, idx);
	if (!o_tty)
		goto err_put_module;

	tty_set_lock_subclass(o_tty);
	lockdep_set_subclass(&o_tty->termios_rwsem, TTY_LOCK_SLAVE);

	if (legacy) {
		/* We always use new tty termios data so we can do this
		   the easy way .. */
		retval = tty_init_termios(tty);
		if (retval)
			goto err_deinit_tty;

		retval = tty_init_termios(o_tty);
		if (retval)
			goto err_free_termios;

		driver->other->ttys[idx] = o_tty;
		driver->ttys[idx] = tty;
	} else {
		memset(&tty->termios_locked, 0, sizeof(tty->termios_locked));
		tty->termios = driver->init_termios;
		memset(&o_tty->termios_locked, 0, sizeof(tty->termios_locked));
		o_tty->termios = driver->other->init_termios;
	}

	/*
	 * Everything allocated ... set up the o_tty structure.
	 */
	tty_driver_kref_get(driver->other);
	/* Establish the links in both directions */
	tty->link   = o_tty;
	o_tty->link = tty;
	tty_port_init(ports[0]);
	tty_port_init(ports[1]);
	tty_buffer_set_limit(ports[0], 8192);
	tty_buffer_set_limit(ports[1], 8192);
	o_tty->port = ports[0];
	tty->port = ports[1];
	o_tty->port->itty = o_tty;

	tty_buffer_set_lock_subclass(o_tty->port);

	tty_driver_kref_get(driver);
	tty->count++;
	o_tty->count++;
	return 0;
err_free_termios:
	if (legacy)
		tty_free_termios(tty);
err_deinit_tty:
	deinitialize_tty_struct(o_tty);
	free_tty_struct(o_tty);
err_put_module:
	module_put(driver->other->owner);
err:
	kfree(ports[0]);
	kfree(ports[1]);
	return retval;
}

static void pty_cleanup(struct tty_struct *tty)
{
	tty_port_put(tty->port);
}

/* Traditional BSD devices */
#ifdef CONFIG_LEGACY_PTYS

static int pty_install(struct tty_driver *driver, struct tty_struct *tty)
{
	return pty_common_install(driver, tty, true);
}

static void pty_remove(struct tty_driver *driver, struct tty_struct *tty)
{
	struct tty_struct *pair = tty->link;
	driver->ttys[tty->index] = NULL;
	if (pair)
		pair->driver->ttys[pair->index] = NULL;
}

static int pty_bsd_ioctl(struct tty_struct *tty,
			 unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case TIOCSPTLCK: /* Set PT Lock (disallow slave open) */
		return pty_set_lock(tty, (int __user *) arg);
	case TIOCGPTLCK: /* Get PT Lock status */
		return pty_get_lock(tty, (int __user *)arg);
	case TIOCPKT: /* Set PT packet mode */
		return pty_set_pktmode(tty, (int __user *)arg);
	case TIOCGPKT: /* Get PT packet mode */
		return pty_get_pktmode(tty, (int __user *)arg);
	case TIOCSIG:    /* Send signal to other side of pty */
		return pty_signal(tty, (int) arg);
	case TIOCGPTN: /* TTY returns ENOTTY, but glibc expects EINVAL here */
		return -EINVAL;
	}
	return -ENOIOCTLCMD;
}

static int legacy_count = CONFIG_LEGACY_PTY_COUNT;
module_param(legacy_count, int, 0);

/*
 * The master side of a pty can do TIOCSPTLCK and thus
 * has pty_bsd_ioctl.
 */
static const struct tty_operations master_pty_ops_bsd = {
	.install = pty_install,
	.open = pty_open,
	.close = pty_close,
	.write = pty_write,
	.write_room = pty_write_room,
	.flush_buffer = pty_flush_buffer,
	.chars_in_buffer = pty_chars_in_buffer,
	.unthrottle = pty_unthrottle,
	.ioctl = pty_bsd_ioctl,
	.cleanup = pty_cleanup,
	.resize = pty_resize,
	.remove = pty_remove
};

static const struct tty_operations slave_pty_ops_bsd = {
	.install = pty_install,
	.open = pty_open,
	.close = pty_close,
	.write = pty_write,
	.write_room = pty_write_room,
	.flush_buffer = pty_flush_buffer,
	.chars_in_buffer = pty_chars_in_buffer,
	.unthrottle = pty_unthrottle,
	.set_termios = pty_set_termios,
	.cleanup = pty_cleanup,
	.resize = pty_resize,
	.start = pty_start,
	.stop = pty_stop,
	.remove = pty_remove
};

static void __init legacy_pty_init(void)
{
	struct tty_driver *pty_driver, *pty_slave_driver;

	if (legacy_count <= 0)
		return;

	pty_driver = tty_alloc_driver(legacy_count,
			TTY_DRIVER_RESET_TERMIOS |
			TTY_DRIVER_REAL_RAW |
			TTY_DRIVER_DYNAMIC_ALLOC);
	if (IS_ERR(pty_driver))
		panic("Couldn't allocate pty driver");

	pty_slave_driver = tty_alloc_driver(legacy_count,
			TTY_DRIVER_RESET_TERMIOS |
			TTY_DRIVER_REAL_RAW |
			TTY_DRIVER_DYNAMIC_ALLOC);
	if (IS_ERR(pty_slave_driver))
		panic("Couldn't allocate pty slave driver");

	pty_driver->driver_name = "pty_master";
	pty_driver->name = "pty";
	pty_driver->major = PTY_MASTER_MAJOR;
	pty_driver->minor_start = 0;
	pty_driver->type = TTY_DRIVER_TYPE_PTY;
	pty_driver->subtype = PTY_TYPE_MASTER;
	pty_driver->init_termios = tty_std_termios;
	pty_driver->init_termios.c_iflag = 0;
	pty_driver->init_termios.c_oflag = 0;
	pty_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	pty_driver->init_termios.c_lflag = 0;
	pty_driver->init_termios.c_ispeed = 38400;
	pty_driver->init_termios.c_ospeed = 38400;
	pty_driver->other = pty_slave_driver;
	tty_set_operations(pty_driver, &master_pty_ops_bsd);

	pty_slave_driver->driver_name = "pty_slave";
	pty_slave_driver->name = "ttyp";
	pty_slave_driver->major = PTY_SLAVE_MAJOR;
	pty_slave_driver->minor_start = 0;
	pty_slave_driver->type = TTY_DRIVER_TYPE_PTY;
	pty_slave_driver->subtype = PTY_TYPE_SLAVE;
	pty_slave_driver->init_termios = tty_std_termios;
	pty_slave_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	pty_slave_driver->init_termios.c_ispeed = 38400;
	pty_slave_driver->init_termios.c_ospeed = 38400;
	pty_slave_driver->other = pty_driver;
	tty_set_operations(pty_slave_driver, &slave_pty_ops_bsd);

	if (tty_register_driver(pty_driver))
		panic("Couldn't register pty driver");
	if (tty_register_driver(pty_slave_driver))
		panic("Couldn't register pty slave driver");
}
#else
static inline void legacy_pty_init(void) { }
#endif

/* Unix98 devices */
#ifdef CONFIG_UNIX98_PTYS

static struct cdev ptmx_cdev;

static int pty_unix98_ioctl(struct tty_struct *tty,
			    unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case TIOCSPTLCK: /* Set PT Lock (disallow slave open) */
		return pty_set_lock(tty, (int __user *)arg);
	case TIOCGPTLCK: /* Get PT Lock status */
		return pty_get_lock(tty, (int __user *)arg);
	case TIOCPKT: /* Set PT packet mode */
		return pty_set_pktmode(tty, (int __user *)arg);
	case TIOCGPKT: /* Get PT packet mode */
		return pty_get_pktmode(tty, (int __user *)arg);
	case TIOCGPTN: /* Get PT Number */
		return put_user(tty->index, (unsigned int __user *)arg);
	case TIOCSIG:    /* Send signal to other side of pty */
		return pty_signal(tty, (int) arg);
	}

	return -ENOIOCTLCMD;
}

/**
 *	ptm_unix98_lookup	-	find a pty master
 *	@driver: ptm driver
 *	@idx: tty index
 *
 *	Look up a pty master device. Called under the tty_mutex for now.
 *	This provides our locking.
 */

static struct tty_struct *ptm_unix98_lookup(struct tty_driver *driver,
		struct inode *ptm_inode, int idx)
{
	/* Master must be open via /dev/ptmx */
	return ERR_PTR(-EIO);
}

/**
 *	pts_unix98_lookup	-	find a pty slave
 *	@driver: pts driver
 *	@idx: tty index
 *
 *	Look up a pty master device. Called under the tty_mutex for now.
 *	This provides our locking for the tty pointer.
 */

static struct tty_struct *pts_unix98_lookup(struct tty_driver *driver,
		struct inode *pts_inode, int idx)
{
	struct tty_struct *tty;

	mutex_lock(&devpts_mutex);
	tty = devpts_get_priv(pts_inode);
	mutex_unlock(&devpts_mutex);
	/* Master must be open before slave */
	if (!tty)
		return ERR_PTR(-EIO);
	return tty;
}

/* We have no need to install and remove our tty objects as devpts does all
   the work for us */

static int pty_unix98_install(struct tty_driver *driver, struct tty_struct *tty)
{
	return pty_common_install(driver, tty, false);
}

static void pty_unix98_remove(struct tty_driver *driver, struct tty_struct *tty)
{
}

/* this is called once with whichever end is closed last */
static void pty_unix98_shutdown(struct tty_struct *tty)
{
	struct inode *ptmx_inode;

	if (tty->driver->subtype == PTY_TYPE_MASTER)
		ptmx_inode = tty->driver_data;
	else
		ptmx_inode = tty->link->driver_data;
	devpts_kill_index(ptmx_inode, tty->index);
	devpts_del_ref(ptmx_inode);
}

static const struct tty_operations ptm_unix98_ops = {
	.lookup = ptm_unix98_lookup,
	.install = pty_unix98_install,
	.remove = pty_unix98_remove,
	.open = pty_open,
	.close = pty_close,
	.write = pty_write,
	.write_room = pty_write_room,
	.flush_buffer = pty_flush_buffer,
	.chars_in_buffer = pty_chars_in_buffer,
	.unthrottle = pty_unthrottle,
	.ioctl = pty_unix98_ioctl,
	.resize = pty_resize,
	.shutdown = pty_unix98_shutdown,
	.cleanup = pty_cleanup
};

static const struct tty_operations pty_unix98_ops = {
	.lookup = pts_unix98_lookup,
	.install = pty_unix98_install,
	.remove = pty_unix98_remove,
	.open = pty_open,
	.close = pty_close,
	.write = pty_write,
	.write_room = pty_write_room,
	.flush_buffer = pty_flush_buffer,
	.chars_in_buffer = pty_chars_in_buffer,
	.unthrottle = pty_unthrottle,
	.set_termios = pty_set_termios,
	.start = pty_start,
	.stop = pty_stop,
	.shutdown = pty_unix98_shutdown,
	.cleanup = pty_cleanup,
};

/**
 *	ptmx_open		-	open a unix 98 pty master
 *	@inode: inode of device file
 *	@filp: file pointer to tty
 *
 *	Allocate a unix98 pty master device from the ptmx driver.
 *
 *	Locking: tty_mutex protects the init_dev work. tty->count should
 *		protect the rest.
 *		allocated_ptys_lock handles the list of free pty numbers
 */

static int ptmx_open(struct inode *inode, struct file *filp)
{
	struct tty_struct *tty;
	struct inode *slave_inode;
	int retval;
	int index;

	nonseekable_open(inode, filp);

	/* We refuse fsnotify events on ptmx, since it's a shared resource */
	filp->f_mode |= FMODE_NONOTIFY;

	retval = tty_alloc_file(filp);
	if (retval)
		return retval;

	/* find a device that is not in use. */
	mutex_lock(&devpts_mutex);
	index = devpts_new_index(inode);
	if (index < 0) {
		retval = index;
		mutex_unlock(&devpts_mutex);
		goto err_file;
	}

	mutex_unlock(&devpts_mutex);

	mutex_lock(&tty_mutex);
	tty = tty_init_dev(ptm_driver, index);

	if (IS_ERR(tty)) {
		retval = PTR_ERR(tty);
		goto out;
	}

	/* The tty returned here is locked so we can safely
	   drop the mutex */
	mutex_unlock(&tty_mutex);

	set_bit(TTY_PTY_LOCK, &tty->flags); /* LOCK THE SLAVE */
	tty->driver_data = inode;

	/*
	 * In the case where all references to ptmx inode are dropped and we
	 * still have /dev/tty opened pointing to the master/slave pair (ptmx
	 * is closed/released before /dev/tty), we must make sure that the inode
	 * is still valid when we call the final pty_unix98_shutdown, thus we
	 * hold an additional reference to the ptmx inode. For the same /dev/tty
	 * last close case, we also need to make sure the super_block isn't
	 * destroyed (devpts instance unmounted), before /dev/tty is closed and
	 * on its release devpts_kill_index is called.
	 */
	devpts_add_ref(inode);

	tty_add_file(tty, filp);

	slave_inode = devpts_pty_new(inode,
			MKDEV(UNIX98_PTY_SLAVE_MAJOR, index), index,
			tty->link);
	if (IS_ERR(slave_inode)) {
		retval = PTR_ERR(slave_inode);
		goto err_release;
	}
	tty->link->driver_data = slave_inode;

	retval = ptm_driver->ops->open(tty, filp);
	if (retval)
		goto err_release;

	tty_unlock(tty);
	return 0;
err_release:
	tty_unlock(tty);
	tty_release(inode, filp);
	return retval;
out:
	mutex_unlock(&tty_mutex);
	devpts_kill_index(inode, index);
err_file:
	tty_free_file(filp);
	return retval;
}

static struct file_operations ptmx_fops;

static void __init unix98_pty_init(void)
{
	ptm_driver = tty_alloc_driver(NR_UNIX98_PTY_MAX,
			TTY_DRIVER_RESET_TERMIOS |
			TTY_DRIVER_REAL_RAW |
			TTY_DRIVER_DYNAMIC_DEV |
			TTY_DRIVER_DEVPTS_MEM |
			TTY_DRIVER_DYNAMIC_ALLOC);
	if (IS_ERR(ptm_driver))
		panic("Couldn't allocate Unix98 ptm driver");
	pts_driver = tty_alloc_driver(NR_UNIX98_PTY_MAX,
			TTY_DRIVER_RESET_TERMIOS |
			TTY_DRIVER_REAL_RAW |
			TTY_DRIVER_DYNAMIC_DEV |
			TTY_DRIVER_DEVPTS_MEM |
			TTY_DRIVER_DYNAMIC_ALLOC);
	if (IS_ERR(pts_driver))
		panic("Couldn't allocate Unix98 pts driver");

	ptm_driver->driver_name = "pty_master";
	ptm_driver->name = "ptm";
	ptm_driver->major = UNIX98_PTY_MASTER_MAJOR;
	ptm_driver->minor_start = 0;
	ptm_driver->type = TTY_DRIVER_TYPE_PTY;
	ptm_driver->subtype = PTY_TYPE_MASTER;
	ptm_driver->init_termios = tty_std_termios;
	ptm_driver->init_termios.c_iflag = 0;
	ptm_driver->init_termios.c_oflag = 0;
	ptm_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	ptm_driver->init_termios.c_lflag = 0;
	ptm_driver->init_termios.c_ispeed = 38400;
	ptm_driver->init_termios.c_ospeed = 38400;
	ptm_driver->other = pts_driver;
	tty_set_operations(ptm_driver, &ptm_unix98_ops);

	pts_driver->driver_name = "pty_slave";
	pts_driver->name = "pts";
	pts_driver->major = UNIX98_PTY_SLAVE_MAJOR;
	pts_driver->minor_start = 0;
	pts_driver->type = TTY_DRIVER_TYPE_PTY;
	pts_driver->subtype = PTY_TYPE_SLAVE;
	pts_driver->init_termios = tty_std_termios;
	pts_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	pts_driver->init_termios.c_ispeed = 38400;
	pts_driver->init_termios.c_ospeed = 38400;
	pts_driver->other = ptm_driver;
	tty_set_operations(pts_driver, &pty_unix98_ops);

	if (tty_register_driver(ptm_driver))
		panic("Couldn't register Unix98 ptm driver");
	if (tty_register_driver(pts_driver))
		panic("Couldn't register Unix98 pts driver");

	/* Now create the /dev/ptmx special device */
	tty_default_fops(&ptmx_fops);
	ptmx_fops.open = ptmx_open;

	cdev_init(&ptmx_cdev, &ptmx_fops);
	if (cdev_add(&ptmx_cdev, MKDEV(TTYAUX_MAJOR, 2), 1) ||
	    register_chrdev_region(MKDEV(TTYAUX_MAJOR, 2), 1, "/dev/ptmx") < 0)
		panic("Couldn't register /dev/ptmx driver");
	device_create(tty_class, NULL, MKDEV(TTYAUX_MAJOR, 2), NULL, "ptmx");
}

#else
static inline void unix98_pty_init(void) { }
#endif

static int __init pty_init(void)
{
	legacy_pty_init();
	unix98_pty_init();
	return 0;
}
module_init(pty_init);
