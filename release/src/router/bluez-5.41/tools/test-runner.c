/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/reboot.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "tools/hciattach.h"

#ifndef WAIT_ANY
#define WAIT_ANY (-1)
#endif

#define CMDLINE_MAX 2048
#ifndef RTCONFIG_LANTIQ
#define MS_STRICTATIME (1<<24)
#endif

static const char *own_binary;
static char **test_argv;
static int test_argc;

static bool run_auto = false;
static bool start_dbus = false;
static int num_devs = 0;
static const char *qemu_binary = NULL;
static const char *kernel_image = NULL;

static const char *qemu_table[] = {
	"qemu-system-x86_64",
	"qemu-system-i386",
	"/usr/bin/qemu-system-x86_64",
	"/usr/bin/qemu-system-i386",
	NULL
};

static const char *find_qemu(void)
{
	int i;

	for (i = 0; qemu_table[i]; i++) {
		struct stat st;

		if (!stat(qemu_table[i], &st))
			return qemu_table[i];
	}

	return NULL;
}

static const char *kernel_table[] = {
	"bzImage",
	"arch/x86/boot/bzImage",
	"vmlinux",
	"arch/x86/boot/vmlinux",
	NULL
};

static const char *find_kernel(void)
{
	int i;

	for (i = 0; kernel_table[i]; i++) {
		struct stat st;

		if (!stat(kernel_table[i], &st))
			return kernel_table[i];
	}

	return NULL;
}

static const struct {
	const char *target;
	const char *linkpath;
} dev_table[] = {
	{ "/proc/self/fd",	"/dev/fd"	},
	{ "/proc/self/fd/0",	"/dev/stdin"	},
	{ "/proc/self/fd/1",	"/dev/stdout"	},
	{ "/proc/self/fd/2",	"/dev/stderr"	},
	{ }
};

static const struct {
	const char *fstype;
	const char *target;
	const char *options;
	unsigned long flags;
} mount_table[] = {
	{ "sysfs",    "/sys",     NULL,        MS_NOSUID|MS_NOEXEC|MS_NODEV },
	{ "proc",     "/proc",    NULL,        MS_NOSUID|MS_NOEXEC|MS_NODEV },
	{ "devtmpfs", "/dev",     "mode=0755", MS_NOSUID|MS_STRICTATIME },
	{ "devpts",   "/dev/pts", "mode=0620", MS_NOSUID|MS_NOEXEC },
	{ "tmpfs",    "/dev/shm", "mode=1777", MS_NOSUID|MS_NODEV|MS_STRICTATIME },
	{ "tmpfs",    "/run",     "mode=0755", MS_NOSUID|MS_NODEV|MS_STRICTATIME },
	{ "tmpfs",    "/tmp",              NULL, 0 },
	{ "debugfs",  "/sys/kernel/debug", NULL, 0 },
	{ }
};

static const char *config_table[] = {
	"/var/lib/bluetooth",
	"/etc/bluetooth",
	"/etc/dbus-1",
	"/usr/share/dbus-1",
	NULL
};

static void prepare_sandbox(void)
{
	int i;

	for (i = 0; mount_table[i].fstype; i++) {
		struct stat st;

		if (lstat(mount_table[i].target, &st) < 0) {
			printf("Creating %s\n", mount_table[i].target);
			mkdir(mount_table[i].target, 0755);
		}

		printf("Mounting %s to %s\n", mount_table[i].fstype,
						mount_table[i].target);

		if (mount(mount_table[i].fstype,
				mount_table[i].target,
				mount_table[i].fstype,
				mount_table[i].flags,
				mount_table[i].options) < 0)
			perror("Failed to mount filesystem");
	}

	for (i = 0; dev_table[i].target; i++) {
		printf("Linking %s to %s\n", dev_table[i].linkpath,
						dev_table[i].target);

		if (symlink(dev_table[i].target, dev_table[i].linkpath) < 0)
			perror("Failed to create device symlink");
	}

	printf("Creating new session group leader\n");
	setsid();

	printf("Setting controlling terminal\n");
	ioctl(STDIN_FILENO, TIOCSCTTY, 1);

	for (i = 0; config_table[i]; i++) {
		printf("Creating %s\n", config_table[i]);

		if (mount("tmpfs", config_table[i], "tmpfs",
				MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME,
				"mode=0755") < 0)
			perror("Failed to create filesystem");
	}
}

static char *const qemu_argv[] = {
	"",
	"-nodefaults",
	"-nodefconfig",
	"-no-user-config",
	"-monitor", "none",
	"-display", "none",
	"-machine", "type=q35,accel=kvm:tcg",
	"-m", "192M",
	"-nographic",
	"-vga", "none",
	"-net", "none",
	"-balloon", "none",
	"-no-acpi",
	"-no-hpet",
	"-no-reboot",
	"-fsdev", "local,id=fsdev-root,path=/,readonly,security_model=none",
	"-device", "virtio-9p-pci,fsdev=fsdev-root,mount_tag=/dev/root",
	"-chardev", "stdio,id=chardev-serial0,signal=off",
	"-device", "pci-serial,chardev=chardev-serial0",
	NULL
};

static char *const qemu_envp[] = {
	"HOME=/",
	NULL
};

static void check_virtualization(void)
{
#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))
	uint32_t ecx;

	__asm__ __volatile__("cpuid" : "=c" (ecx) : "a" (1) : "memory");

	if (!!(ecx & (1 << 5)))
		printf("Found support for Virtual Machine eXtensions\n");
#endif
}

static void start_qemu(void)
{
	char cwd[PATH_MAX], initcmd[PATH_MAX], testargs[PATH_MAX];
	char cmdline[CMDLINE_MAX];
	char **argv;
	int i, pos;

	check_virtualization();

	if (!getcwd(cwd, sizeof(cwd)))
		strcat(cwd, "/");

	if (own_binary[0] == '/')
		snprintf(initcmd, sizeof(initcmd), "%s", own_binary);
	else
		snprintf(initcmd, sizeof(initcmd), "%s/%s", cwd, own_binary);

	pos = snprintf(testargs, sizeof(testargs), "%s", test_argv[0]);

	for (i = 1; i < test_argc; i++) {
		int len = sizeof(testargs) - pos;
		pos += snprintf(testargs + pos, len, " %s", test_argv[i]);
	}

	snprintf(cmdline, sizeof(cmdline),
				"console=ttyS0,115200n8 earlyprintk=serial "
				"rootfstype=9p "
				"rootflags=trans=virtio,version=9p2000.L "
				"acpi=off pci=noacpi noapic quiet ro init=%s "
				"TESTHOME=%s TESTDBUS=%u TESTDEVS=%d "
				"TESTAUTO=%u TESTARGS=\'%s\'", initcmd, cwd,
				start_dbus, num_devs, run_auto, testargs);

	argv = alloca(sizeof(qemu_argv) +
				(sizeof(char *) * (4 + (num_devs * 4))));
	memcpy(argv, qemu_argv, sizeof(qemu_argv));

	pos = (sizeof(qemu_argv) / sizeof(char *)) - 1;

	argv[0] = (char *) qemu_binary;

	argv[pos++] = "-kernel";
	argv[pos++] = (char *) kernel_image;
	argv[pos++] = "-append";
	argv[pos++] = (char *) cmdline;

	for (i = 0; i < num_devs; i++) {
		const char *path = "/tmp/bt-server-bredr";
		char *chrdev, *serdev;

		chrdev = alloca(32 + strlen(path));
		sprintf(chrdev, "socket,path=%s,id=bt%d", path, i);

		serdev = alloca(32);
		sprintf(serdev, "pci-serial,chardev=bt%d", i);

		argv[pos++] = "-chardev";
		argv[pos++] = chrdev;
		argv[pos++] = "-device";
		argv[pos++] = serdev;
	}

	argv[pos] = NULL;

	execve(argv[0], argv, qemu_envp);
}

static int open_serial(const char *path)
{
	struct termios ti;
	int fd, saved_ldisc, ldisc = N_HCI;

	fd = open(path, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror("Failed to open serial port");
		return -1;
	}

	if (tcflush(fd, TCIOFLUSH) < 0) {
		perror("Failed to flush serial port");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TIOCGETD, &saved_ldisc) < 0) {
		perror("Failed get serial line discipline");
		close(fd);
		return -1;
	}

	/* Switch TTY to raw mode */
	memset(&ti, 0, sizeof(ti));
	cfmakeraw(&ti);

	ti.c_cflag |= (B115200 | CLOCAL | CREAD);

	/* Set flow control */
	ti.c_cflag |= CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &ti) < 0) {
		perror("Failed to set serial port settings");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TIOCSETD, &ldisc) < 0) {
		perror("Failed set serial line discipline");
		close(fd);
		return -1;
	}

	printf("Switched line discipline from %d to %d\n", saved_ldisc, ldisc);

	return fd;
}

static int attach_proto(const char *path, unsigned int proto,
					unsigned int mandatory_flags,
					unsigned int optional_flags)
{
	unsigned int flags = mandatory_flags | optional_flags;
	int fd, dev_id;

	fd = open_serial(path);
	if (fd < 0)
		return -1;

	if (ioctl(fd, HCIUARTSETFLAGS, flags) < 0) {
		if (errno == EINVAL) {
			if (ioctl(fd, HCIUARTSETFLAGS, mandatory_flags) < 0) {
				perror("Failed to set mandatory flags");
				close(fd);
				return -1;
			}
		} else {
			perror("Failed to set flags");
			close(fd);
			return -1;
		}
	}

	if (ioctl(fd, HCIUARTSETPROTO, proto) < 0) {
		perror("Failed to set protocol");
		close(fd);
		return -1;
	}

	dev_id = ioctl(fd, HCIUARTGETDEVICE);
	if (dev_id < 0) {
		perror("Failed to get device id");
		close(fd);
		return -1;
	}

	printf("Device index %d attached\n", dev_id);

	return fd;
}

static void create_dbus_system_conf(void)
{
	FILE *fp;

	fp = fopen("/etc/dbus-1/system.conf", "we");
	if (!fp)
		return;

	fputs("<!DOCTYPE busconfig PUBLIC "
		"\"-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN\" "
		"\"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n", fp);
	fputs("<busconfig>\n", fp);
	fputs("<type>system</type>\n", fp);
	fputs("<listen>unix:path=/run/dbus/system_bus_socket</listen>\n", fp);
	fputs("<policy context=\"default\">\n", fp);
	fputs("<allow user=\"*\"/>\n", fp);
	fputs("<allow own=\"*\"/>\n", fp);
	fputs("<allow send_type=\"method_call\"/>\n",fp);
	fputs("<allow send_type=\"signal\"/>\n", fp);
	fputs("<allow send_type=\"method_return\"/>\n", fp);
	fputs("<allow send_type=\"error\"/>\n", fp);
	fputs("<allow receive_type=\"method_call\"/>\n",fp);
	fputs("<allow receive_type=\"signal\"/>\n", fp);
	fputs("<allow receive_type=\"method_return\"/>\n", fp);
	fputs("<allow receive_type=\"error\"/>\n", fp);
	fputs("</policy>\n", fp);
	fputs("</busconfig>\n", fp);

	fclose(fp);

	mkdir("/run/dbus", 0755);
}

static pid_t start_dbus_daemon(void)
{
	char *argv[3], *envp[1];
	pid_t pid;
	int i;

	argv[0] = "/usr/bin/dbus-daemon";
	argv[1] = "--system";
	argv[2] = NULL;

	envp[0] = NULL;

	printf("Starting D-Bus daemon\n");

	pid = fork();
	if (pid < 0) {
		perror("Failed to fork new process");
		return -1;
	}

	if (pid == 0) {
		execve(argv[0], argv, envp);
		exit(EXIT_SUCCESS);
	}

	printf("D-Bus daemon process %d created\n", pid);

	for (i = 0; i < 20; i++) {
		struct stat st;

		if (!stat("/run/dbus/system_bus_socket", &st)) {
			printf("Found D-Bus daemon socket\n");
			break;
		}

		usleep(25 * 1000);
	}

	return pid;
}

static const char *daemon_table[] = {
	"bluetoothd",
	"src/bluetoothd",
	"/usr/sbin/bluetoothd",
	"/usr/libexec/bluetooth/bluetoothd",
	NULL
};

static pid_t start_bluetooth_daemon(const char *home)
{
	const char *daemon = NULL;
	char *argv[3], *envp[2];
	pid_t pid;
	int i;

	if (chdir(home + 5) < 0) {
		perror("Failed to change home directory for daemon");
		return -1;
	}

	for (i = 0; daemon_table[i]; i++) {
		struct stat st;

		if (!stat(daemon_table[i], &st)) {
			daemon = daemon_table[i];
			break;
		}
	}

	if (!daemon) {
		fprintf(stderr, "Failed to locate Bluetooth daemon binary\n");
		return -1;
	}

	printf("Using Bluetooth daemon %s\n", daemon);

	argv[0] = (char *) daemon;
	argv[1] = "--nodetach";
	argv[2] = NULL;

	envp[0] = "DBUS_SYSTEM_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket";
	envp[1] = NULL;

	printf("Starting Bluetooth daemon\n");

	pid = fork();
	if (pid < 0) {
		perror("Failed to fork new process");
		return -1;
	}

	if (pid == 0) {
		execve(argv[0], argv, envp);
		exit(EXIT_SUCCESS);
	}

	printf("Bluetooth daemon process %d created\n", pid);

	return pid;
}

static const char *test_table[] = {
	"mgmt-tester",
	"smp-tester",
	"l2cap-tester",
	"rfcomm-tester",
	"sco-tester",
	"bnep-tester",
	"check-selftest",
	"tools/mgmt-tester",
	"tools/smp-tester",
	"tools/l2cap-tester",
	"tools/rfcomm-tester",
	"tools/sco-tester",
	"tools/bnep-tester",
	"tools/check-selftest",
	NULL
};

static void run_command(char *cmdname, char *home)
{
	char *argv[9], *envp[3];
	int pos = 0, idx = 0;
	int serial_fd;
	pid_t pid, dbus_pid, daemon_pid;

	if (num_devs) {
		const char *node = "/dev/ttyS1";
		unsigned int basic_flags, extra_flags;

		printf("Attaching BR/EDR controller to %s\n", node);

		basic_flags = (1 << HCI_UART_RESET_ON_INIT);
		extra_flags = (1 << HCI_UART_VND_DETECT);

		serial_fd = attach_proto(node, HCI_UART_H4, basic_flags,
								extra_flags);
	} else
		serial_fd = -1;

	if (start_dbus) {
		create_dbus_system_conf();
		dbus_pid = start_dbus_daemon();
		daemon_pid = start_bluetooth_daemon(home);
	} else {
		dbus_pid = -1;
		daemon_pid = -1;
	}

start_next:
	if (run_auto) {
		if (chdir(home + 5) < 0) {
			perror("Failed to change home test directory");
			return;
		}

		while (1) {
			struct stat st;

			if (!test_table[idx])
				return;

			if (!stat(test_table[idx], &st))
				break;

			idx++;
		}

		argv[0] = (char *) test_table[idx];
		argv[1] = "-q";
		argv[2] = NULL;
	} else {
		while (1) {
			char *ptr;

			ptr = strchr(cmdname, ' ');
			if (!ptr) {
				argv[pos++] = cmdname;
				break;
			}

			*ptr = '\0';
			argv[pos++] = cmdname;
			if (pos > 8)
				break;

			cmdname = ptr + 1;
		}

		argv[pos] = NULL;
	}

	pos = 0;
	envp[pos++] = "TERM=linux";
	if (home)
		envp[pos++] = home;
	envp[pos] = NULL;

	printf("Running command %s\n", argv[0]);

	pid = fork();
	if (pid < 0) {
		perror("Failed to fork new process");
		return;
	}

	if (pid == 0) {
		if (home) {
			printf("Changing into directory %s\n", home + 5);
			if (chdir(home + 5) < 0)
				perror("Failed to change directory");
		}

		execve(argv[0], argv, envp);
		exit(EXIT_SUCCESS);
	}

	printf("New process %d created\n", pid);

	while (1)  {
		pid_t corpse;
		int status;

		corpse = waitpid(WAIT_ANY, &status, 0);
		if (corpse < 0 || corpse == 0)
			continue;

		if (WIFEXITED(status))
			printf("Process %d exited with status %d\n",
						corpse, WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			printf("Process %d terminated with signal %d\n",
						corpse, WTERMSIG(status));
		else if (WIFSTOPPED(status))
			printf("Process %d stopped with signal %d\n",
						corpse, WSTOPSIG(status));
		else if (WIFCONTINUED(status))
			printf("Process %d continued\n", corpse);

		if (corpse == dbus_pid) {
			printf("D-Bus daemon terminated\n");
			dbus_pid = -1;
		}

		if (corpse == daemon_pid) {
			printf("Bluetooth daemon terminated\n");
			daemon_pid = -1;
		}

		if (corpse == pid) {
			if (!run_auto) {
				if (daemon_pid > 0)
					kill(daemon_pid, SIGTERM);
				if (dbus_pid > 0)
					kill(dbus_pid, SIGTERM);
			}
			break;
		}
	}

	if (run_auto) {
		idx++;
		goto start_next;
	}

	if (serial_fd >= 0) {
		close(serial_fd);
		serial_fd = -1;
	}
}

static void run_tests(void)
{
	char cmdline[CMDLINE_MAX], *ptr, *cmds, *home = NULL;
	FILE *fp;

	fp = fopen("/proc/cmdline", "re");
	if (!fp) {
		fprintf(stderr, "Failed to open kernel command line\n");
		return;
	}

	ptr = fgets(cmdline, sizeof(cmdline), fp);
	fclose(fp);

	if (!ptr) {
		fprintf(stderr, "Failed to read kernel command line\n");
		return;
	}

	ptr = strstr(cmdline, "TESTARGS=");
	if (!ptr) {
		fprintf(stderr, "No test command section found\n");
		return;
	}

	cmds = ptr + 10;
	ptr = strchr(cmds, '\'');
	if (!ptr) {
		fprintf(stderr, "Malformed test command section\n");
		return;
	}

	*ptr = '\0';

	ptr = strstr(cmdline, "TESTAUTO=1");
	if (ptr) {
		printf("Automatic test execution requested\n");
		run_auto= true;
	}

	ptr = strstr(cmdline, "TESTDEVS=1");
	if (ptr) {
		printf("Attachment of devices requested\n");
		num_devs = 1;
	}

	ptr = strstr(cmdline, "TESTDBUS=1");
	if (ptr) {
		printf("D-Bus daemon requested\n");
		start_dbus = true;
	}

	ptr = strstr(cmdline, "TESTHOME=");
	if (ptr) {
		home = ptr + 4;
		ptr = strpbrk(home + 9, " \r\n");
		if (ptr)
			*ptr = '\0';
	}

	run_command(cmds, home);
}

static void usage(void)
{
	printf("test-runner - Automated test execution utility\n"
		"Usage:\n");
	printf("\ttest-runner [options] [--] <command> [args]\n");
	printf("Options:\n"
		"\t-a, --auto             Find tests and run them\n"
		"\t-d, --dbus             Start D-Bus daemon\n"
		"\t-u, --unix [path]      Provide serial device\n"
		"\t-q, --qemu <path>      QEMU binary\n"
		"\t-k, --kernel <image>   Kernel image (bzImage)\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "all",     no_argument,       NULL, 'a' },
	{ "auto",    no_argument,       NULL, 'a' },
	{ "unix",    no_argument,       NULL, 'u' },
	{ "dbus",    no_argument,       NULL, 'd' },
	{ "qemu",    required_argument, NULL, 'q' },
	{ "kernel",  required_argument, NULL, 'k' },
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	if (getpid() == 1 && getppid() == 0) {
		prepare_sandbox();
		run_tests();

		sync();
		reboot(RB_AUTOBOOT);
		return EXIT_SUCCESS;
	}

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "audq:k:vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'a':
			run_auto = true;
			break;
		case 'u':
			num_devs = 1;
			break;
		case 'd':
			start_dbus = true;
			break;
		case 'q':
			qemu_binary = optarg;
			break;
		case 'k':
			kernel_image = optarg;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (run_auto) {
		if (argc - optind > 0) {
			fprintf(stderr, "Invalid command line parameters\n");
			return EXIT_FAILURE;
		}
	} else {
		if (argc - optind < 1) {
			fprintf(stderr, "Failed to specify test command\n");
			return EXIT_FAILURE;
		}
	}

	own_binary = argv[0];
	test_argv = argv + optind;
	test_argc = argc - optind;

	if (!qemu_binary) {
		qemu_binary = find_qemu();
		if (!qemu_binary) {
			fprintf(stderr, "No default QEMU binary found\n");
			return EXIT_FAILURE;
		}
	}

	if (!kernel_image) {
		kernel_image = find_kernel();
		if (!kernel_image) {
			fprintf(stderr, "No default kernel image found\n");
			return EXIT_FAILURE;
		}
	}

	printf("Using QEMU binary %s\n", qemu_binary);
	printf("Using kernel image %s\n", kernel_image);

	start_qemu();

	return EXIT_SUCCESS;
}
