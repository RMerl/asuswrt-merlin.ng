// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#include <common.h>
#include <console.h>
#include <debug_uart.h>
#include <dm.h>
#include <stdarg.h>
#include <iomux.h>
#include <malloc.h>
#include <mapmem.h>
#include <os.h>
#include <serial.h>
#include <stdio_dev.h>
#include <exports.h>
#include <environment.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

static int on_console(const char *name, const char *value, enum env_op op,
	int flags)
{
	int console = -1;

	/* Check for console redirection */
	if (strcmp(name, "stdin") == 0)
		console = stdin;
	else if (strcmp(name, "stdout") == 0)
		console = stdout;
	else if (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	if (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		return 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

#if CONFIG_IS_ENABLED(CONSOLE_MUX)
		if (iomux_doenv(console, value))
			return 1;
#else
		/* Try assigning specified device */
		if (console_assign(console, value) < 0)
			return 1;
#endif
		return 0;

	case env_op_delete:
		if ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		return 1;

	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static int on_silent(const char *name, const char *value, enum env_op op,
	int flags)
{
#if !CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET)
	if (flags & H_INTERACTIVE)
		return 0;
#endif
#if !CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC)
	if ((flags & H_INTERACTIVE) == 0)
		return 0;
#endif

	if (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	else
		gd->flags &= ~GD_FLG_SILENT;

	return 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#if CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#else
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static int console_setfile(int file, struct stdio_dev * dev)
{
	int error = 0;

	if (dev == NULL)
		return -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		/* Start new device */
		if (dev->start) {
			error = dev->start(dev);
			/* If it's not started dont use it */
			if (error < 0)
				break;
		}

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		switch (file) {
		case stdin:
			gd->jt->getc = getc;
			gd->jt->tstc = tstc;
			break;
		case stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			gd->jt->printf = printf;
			break;
		}
		break;

	default:		/* Invalid file ID */
		error = -1;
	}
	return error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * @return true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
static bool console_dev_is_serial(struct stdio_dev *sdev)
{
	bool is_serial;

#ifdef CONFIG_DM_SERIAL
	if (sdev->flags & DEV_FLAGS_DM) {
		struct udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} else
#endif
	is_serial = !strcmp(sdev->name, "serial");

	return is_serial;
}

#if CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
int cd_count[MAX_FILES];

/*
 * This depends on tstc() always being called before getc().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static int console_getc(int file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	return ret;
}

static int console_tstc(int file)
{
	int i, ret;
	struct stdio_dev *dev;
	int prev;

	prev = disable_ctrlc(1);
	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			if (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				return ret;
			}
		}
	}
	disable_ctrlc(prev);

	return 0;
}

static void console_putc(int file, const char c)
{
	int i;
	struct stdio_dev *dev;

	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->putc != NULL)
			dev->putc(dev, c);
	}
}

static void console_puts_noserial(int file, const char *s)
{
	int i;
	struct stdio_dev *dev;

	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->puts != NULL && !console_dev_is_serial(dev))
			dev->puts(dev, s);
	}
}

static void console_puts(int file, const char *s)
{
	int i;
	struct stdio_dev *dev;

	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

static inline void console_doenv(int file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#else
static inline int console_getc(int file)
{
	return stdio_devices[file]->getc(stdio_devices[file]);
}

static inline int console_tstc(int file)
{
	return stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(int file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

static inline void console_puts_noserial(int file, const char *s)
{
	if (!console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(int file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_doenv(int file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

int serial_printf(const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	return i;
}

int fgetc(int file)
{
	if (file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		for (;;) {
			WATCHDOG_RESET();
#if CONFIG_IS_ENABLED(CONSOLE_MUX)
			/*
			 * Upper layer may have already called tstc() so
			 * check for that first.
			 */
			if (tstcdev != NULL)
				return console_getc(file);
			console_tstc(file);
#else
			if (console_tstc(file))
				return console_getc(file);
#endif
#ifdef CONFIG_WATCHDOG
			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			 udelay(1);
#endif
		}
	}

	return -1;
}

int ftstc(int file)
{
	if (file < MAX_FILES)
		return console_tstc(file);

	return -1;
}

void fputc(int file, const char c)
{
	if (file < MAX_FILES)
		console_putc(file, c);
}

void fputs(int file, const char *s)
{
	if (file < MAX_FILES)
		console_puts(file, s);
}

int fprintf(int file, const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	return i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

int getc(void)
{
#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return 0;
#endif

	if (!gd->have_console)
		return 0;

#ifdef CONFIG_CONSOLE_RECORD
	if (gd->console_in.start) {
		int ch;

		ch = membuff_getbyte(&gd->console_in);
		if (ch != -1)
			return 1;
	}
#endif
	if (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		return fgetc(stdin);
	}

	/* Send directly to the handler */
	return serial_getc();
}

int tstc(void)
{
#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return 0;
#endif

	if (!gd->have_console)
		return 0;
#ifdef CONFIG_CONSOLE_RECORD
	if (gd->console_in.start) {
		if (membuff_peekbyte(&gd->console_in) != -1)
			return 1;
	}
#endif
	if (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		return ftstc(stdin);
	}

	/* Send directly to the handler */
	return serial_tstc();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#if CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_PRE_CON_BUF_SZ)

static void pre_console_putc(const char c)
{
	char *buffer;

	buffer = map_sysmem(CONFIG_PRE_CON_BUF_ADDR, CONFIG_PRE_CON_BUF_SZ);

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	while (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(int flushpoint)
{
	unsigned long in = 0, out = 0;
	char buf_out[CONFIG_PRE_CON_BUF_SZ + 1];
	char *buf_in;

	buf_in = map_sysmem(CONFIG_PRE_CON_BUF_ADDR, CONFIG_PRE_CON_BUF_SZ);
	if (gd->precon_buf_idx > CONFIG_PRE_CON_BUF_SZ)
		in = gd->precon_buf_idx - CONFIG_PRE_CON_BUF_SZ;

	while (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	switch (flushpoint) {
	case PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		break;
	case PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_noserial(stdout, buf_out);
		break;
	}
}
#else
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(int flushpoint) {}
#endif

void putc(const char c)
{
#ifdef CONFIG_SANDBOX
	/* sandbox can send characters to stdout before it has a console */
	if (!gd || !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		return;
	}
#endif
#ifdef CONFIG_DEBUG_UART
	/* if we don't have a console yet, use the debug UART */
	if (!gd || !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		return;
	}
#endif
	if (!gd)
		return;
#ifdef CONFIG_CONSOLE_RECORD
	if ((gd->flags & GD_FLG_RECORD) && gd->console_out.start)
		membuff_putbyte(&gd->console_out, c);
#endif
#ifdef CONFIG_SILENT_CONSOLE
	if (gd->flags & GD_FLG_SILENT)
		return;
#endif

#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return;
#endif

	if (!gd->have_console)
		return pre_console_putc(c);

	if (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} else {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
#ifdef CONFIG_SANDBOX
	/* sandbox can send characters to stdout before it has a console */
	if (!gd || !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		return;
	}
#endif
#ifdef CONFIG_DEBUG_UART
	if (!gd || !(gd->flags & GD_FLG_SERIAL_READY)) {
		while (*s) {
			int ch = *s++;

			printch(ch);
		}
		return;
	}
#endif
	if (!gd)
		return;
#ifdef CONFIG_CONSOLE_RECORD
	if ((gd->flags & GD_FLG_RECORD) && gd->console_out.start)
		membuff_put(&gd->console_out, s, strlen(s));
#endif
#ifdef CONFIG_SILENT_CONSOLE
	if (gd->flags & GD_FLG_SILENT)
		return;
#endif

#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return;
#endif

	if (!gd->have_console)
		return pre_console_puts(s);

	if (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} else {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_RECORD
int console_record_init(void)
{
	int ret;

	ret = membuff_new(&gd->console_out, CONFIG_CONSOLE_RECORD_OUT_SIZE);
	if (ret)
		return ret;
	ret = membuff_new(&gd->console_in, CONFIG_CONSOLE_RECORD_IN_SIZE);

	return ret;
}

void console_record_reset(void)
{
	membuff_purge(&gd->console_out);
	membuff_purge(&gd->console_in);
}

void console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;
}
#endif

/* test if ctrl-c was pressed */
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;
int ctrlc(void)
{
	if (!ctrlc_disabled && gd->have_console) {
		if (tstc()) {
			switch (getc()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				return 1;
			default:
				break;
			}
		}
	}

	return 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
int confirm_yesno(void)
{
	int i;
	char str_input[5];

	/* Flush input */
	while (tstc())
		getc();
	i = 0;
	while (i < sizeof(str_input)) {
		str_input[i] = getc();
		putc(str_input[i]);
		if (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	if (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		return 1;
	return 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
int disable_ctrlc(int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *search_device(int flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
#ifdef CONFIG_VIDCONSOLE_AS_LCD
	if (!dev && !strcmp(name, "lcd"))
		dev = stdio_get_by_name("vidconsole");
#endif

	if (dev && (dev->flags & flags))
		return dev;

	return NULL;
}

int console_assign(int file, const char *devname)
{
	int flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	switch (file) {
	case stdin:
		flag = DEV_FLAGS_INPUT;
		break;
	case stdout:
	case stderr:
		flag = DEV_FLAGS_OUTPUT;
		break;
	default:
		return -1;
	}

	/* Check for valid device name */

	dev = search_device(flag, devname);

	if (dev)
		return console_setfile(file, dev);

	return -1;
}

static void console_update_silent(void)
{
#ifdef CONFIG_SILENT_CONSOLE
	if (env_get("silent") != NULL)
		gd->flags |= GD_FLG_SILENT;
	else
		gd->flags &= ~GD_FLG_SILENT;
#endif
}

int console_announce_r(void)
{
#if !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_noserial(stdout, buf);
#endif

	return 0;
}

/* Called before relocation - use serial functions */
int console_init_f(void)
{
	gd->have_console = 1;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	return 0;
}

void stdio_print_current_devices(void)
{
	/* Print information */
	puts("In:    ");
	if (stdio_devices[stdin] == NULL) {
		puts("No input devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stdin]->name);
	}

	puts("Out:   ");
	if (stdio_devices[stdout] == NULL) {
		puts("No output devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stdout]->name);
	}

	puts("Err:   ");
	if (stdio_devices[stderr] == NULL) {
		puts("No error devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stderr]->name);
	}
}

#if CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
int console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
#ifdef CONFIG_SYS_CONSOLE_ENV_OVERWRITE
	int i;
#endif /* CONFIG_SYS_CONSOLE_ENV_OVERWRITE */
#if CONFIG_IS_ENABLED(CONSOLE_MUX)
	int iomux_err = 0;
#endif

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->printf = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	if (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = search_device(DEV_FLAGS_INPUT,  stdinname);
		outputdev = search_device(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = search_device(DEV_FLAGS_OUTPUT, stderrname);
#if CONFIG_IS_ENABLED(CONSOLE_MUX)
		iomux_err = iomux_doenv(stdin, stdinname);
		iomux_err += iomux_doenv(stdout, stdoutname);
		iomux_err += iomux_doenv(stderr, stderrname);
		if (!iomux_err)
			/* Successful, so skip all the code below. */
			goto done;
#endif
	}
	/* if the devices are overwritten or not found, use default device */
	if (inputdev == NULL) {
		inputdev  = search_device(DEV_FLAGS_INPUT,  "serial");
	}
	if (outputdev == NULL) {
		outputdev = search_device(DEV_FLAGS_OUTPUT, "serial");
	}
	if (errdev == NULL) {
		errdev    = search_device(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	if (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	if (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	if (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

#if CONFIG_IS_ENABLED(CONSOLE_MUX)
done:
#endif

#ifndef CONFIG_SYS_CONSOLE_INFO_QUIET
	stdio_print_current_devices();
#endif /* CONFIG_SYS_CONSOLE_INFO_QUIET */
#ifdef CONFIG_VIDCONSOLE_AS_LCD
	if (strstr(stdoutname, "lcd"))
		printf("Warning: Please change 'lcd' to 'vidconsole' in stdout/stderr environment vars\n");
#endif

#ifdef CONFIG_SYS_CONSOLE_ENV_OVERWRITE
	/* set the environment variables (will overwrite previous env settings) */
	for (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}
#endif /* CONFIG_SYS_CONSOLE_ENV_OVERWRITE */

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

#if 0
	/* If nothing usable installed, use only the initial console */
	if ((stdio_devices[stdin] == NULL) && (stdio_devices[stdout] == NULL))
		return 0;
#endif
	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL);
	return 0;
}

#else /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
int console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	int i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;

	console_update_silent();

#ifdef CONFIG_SPLASH_SCREEN
	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	if (env_get("splashimage") != NULL) {
		if (!(gd->flags & GD_FLG_SILENT))
			outputdev = search_device (DEV_FLAGS_OUTPUT, "serial");
	}
#endif

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		if ((dev->flags & DEV_FLAGS_INPUT) && (inputdev == NULL)) {
			inputdev = dev;
		}
		if ((dev->flags & DEV_FLAGS_OUTPUT) && (outputdev == NULL)) {
			outputdev = dev;
		}
		if(inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	if (outputdev != NULL) {
		console_setfile(stdout, outputdev);
		console_setfile(stderr, outputdev);
#if CONFIG_IS_ENABLED(CONSOLE_MUX)
		console_devices[stdout][0] = outputdev;
		console_devices[stderr][0] = outputdev;
#endif
	}

	/* Initializes input console */
	if (inputdev != NULL) {
		console_setfile(stdin, inputdev);
#if CONFIG_IS_ENABLED(CONSOLE_MUX)
		console_devices[stdin][0] = inputdev;
#endif
	}

#ifndef CONFIG_SYS_CONSOLE_INFO_QUIET
	stdio_print_current_devices();
#endif /* CONFIG_SYS_CONSOLE_INFO_QUIET */

	/* Setting environment variables */
	for (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

#if 0
	/* If nothing usable installed, use only the initial console */
	if ((stdio_devices[stdin] == NULL) && (stdio_devices[stdout] == NULL))
		return 0;
#endif
	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL);
	return 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */
