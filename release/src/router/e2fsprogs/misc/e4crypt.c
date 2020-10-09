/*
 * e4crypt.c - ext4 encryption management utility
 *
 * Copyright (c) 2014 Google, Inc.
 *	SHA512 implementation from libtomcrypt.
 *
 * Authors: Michael Halcrow <mhalcrow@google.com>,
 *	Ildar Muslukhov <ildarm@google.com>
 */

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "config.h"
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#if !defined(HAVE_ADD_KEY) || !defined(HAVE_KEYCTL)
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYS_KEY_H
#include <sys/key.h>
#endif

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "uuid/uuid.h"

/* special process keyring shortcut IDs */
#define KEY_SPEC_THREAD_KEYRING		-1
#define KEY_SPEC_PROCESS_KEYRING	-2
#define KEY_SPEC_SESSION_KEYRING	-3
#define KEY_SPEC_USER_KEYRING		-4
#define KEY_SPEC_USER_SESSION_KEYRING	-5
#define KEY_SPEC_GROUP_KEYRING		-6

#define KEYCTL_GET_KEYRING_ID		0
#define KEYCTL_JOIN_SESSION_KEYRING	1
#define KEYCTL_DESCRIBE			6
#define KEYCTL_SEARCH			10
#define KEYCTL_SESSION_TO_PARENT	18

typedef __s32 key_serial_t;

#define EXT4_KEY_REF_STR_BUF_SIZE ((EXT4_KEY_DESCRIPTOR_SIZE * 2) + 1)

#ifndef EXT4_IOC_GET_ENCRYPTION_PWSALT
#define EXT4_IOC_GET_ENCRYPTION_PWSALT	_IOW('f', 20, __u8[16])
#endif

#define OPT_VERBOSE	0x0001
#define OPT_QUIET	0x0002

int options;

#ifndef HAVE_KEYCTL
static long keyctl(int cmd, ...)
{
	va_list va;
	unsigned long arg2, arg3, arg4, arg5;

	va_start(va, cmd);
	arg2 = va_arg(va, unsigned long);
	arg3 = va_arg(va, unsigned long);
	arg4 = va_arg(va, unsigned long);
	arg5 = va_arg(va, unsigned long);
	va_end(va);
	return syscall(__NR_keyctl, cmd, arg2, arg3, arg4, arg5);
}
#endif

#ifndef HAVE_ADD_KEY
static key_serial_t add_key(const char *type, const char *description,
			    const void *payload, size_t plen,
			    key_serial_t keyring)
{
	return syscall(__NR_add_key, type, description, payload,
		       plen, keyring);
}
#endif

static const unsigned char *hexchars = (const unsigned char *) "0123456789abcdef";
static const size_t hexchars_size = 16;

#define SHA512_LENGTH 64
#define EXT2FS_KEY_TYPE_LOGON "logon"
#define EXT2FS_KEY_DESC_PREFIX "ext4:"
#define EXT2FS_KEY_DESC_PREFIX_SIZE 5

#define EXT4_IOC_SET_ENCRYPTION_POLICY      _IOR('f', 19, struct ext4_encryption_policy)
#define EXT4_IOC_GET_ENCRYPTION_POLICY      _IOW('f', 21, struct ext4_encryption_policy)

static int int_log2(int arg)
{
	int     l = 0;

	arg >>= 1;
	while (arg) {
		l++;
		arg >>= 1;
	}
	return l;
}

static void validate_paths(int argc, char *argv[], int path_start_index)
{
	int x;
	int valid = 1;
	struct stat st;

	for (x = path_start_index; x < argc; x++) {
		int ret = access(argv[x], W_OK);
		if (ret) {
		invalid:
			perror(argv[x]);
			valid = 0;
			continue;
		}
		ret = stat(argv[x], &st);
		if (ret < 0)
			goto invalid;
		if (!S_ISDIR(st.st_mode)) {
			fprintf(stderr, "%s is not a directory\n", argv[x]);
			goto invalid;
		}
	}
	if (!valid)
		exit(1);
}

static int hex2byte(const char *hex, size_t hex_size, unsigned char *bytes,
		    size_t bytes_size)
{
	size_t x;
	unsigned char *h, *l;

	if (hex_size % 2)
		return -EINVAL;
	for (x = 0; x < hex_size; x += 2) {
		h = memchr(hexchars, hex[x], hexchars_size);
		if (!h)
			return -EINVAL;
		l = memchr(hexchars, hex[x + 1], hexchars_size);
		if (!l)
			return -EINVAL;
		if ((x >> 1) >= bytes_size)
			return -EINVAL;
		bytes[x >> 1] = (((unsigned char)(h - hexchars) << 4) +
				 (unsigned char)(l - hexchars));
	}
	return 0;
}

/*
 * Salt handling
 */
struct salt {
	unsigned char *salt;
	char key_ref_str[EXT4_KEY_REF_STR_BUF_SIZE];
	unsigned char key_desc[EXT4_KEY_DESCRIPTOR_SIZE];
	unsigned char key[EXT4_MAX_KEY_SIZE];
	size_t salt_len;
};
struct salt *salt_list;
unsigned num_salt;
unsigned max_salt;
char in_passphrase[EXT4_MAX_PASSPHRASE_SIZE];

static struct salt *find_by_salt(unsigned char *salt, size_t salt_len)
{
	unsigned int i;
	struct salt *p;

	for (i = 0, p = salt_list; i < num_salt; i++, p++)
		if ((p->salt_len == salt_len) &&
		    !memcmp(p->salt, salt, salt_len))
			return p;
	return NULL;
}

static void add_salt(unsigned char *salt, size_t salt_len)
{
	if (find_by_salt(salt, salt_len))
		return;
	if (num_salt >= max_salt) {
		max_salt = num_salt + 10;
		salt_list = realloc(salt_list, max_salt * sizeof(struct salt));
		if (!salt_list) {
			fprintf(stderr, "Couldn't allocate salt list\n");
			exit(1);
		}
	}
	salt_list[num_salt].salt = salt;
	salt_list[num_salt].salt_len = salt_len;
	num_salt++;
}

static void clear_secrets(void)
{
	if (salt_list) {
		memset(salt_list, 0, sizeof(struct salt) * max_salt);
		free(salt_list);
		salt_list = NULL;
	}
	memset(in_passphrase, 0, sizeof(in_passphrase));
}

static void die_signal_handler(int signum EXT2FS_ATTR((unused)),
			       siginfo_t *siginfo EXT2FS_ATTR((unused)),
			       void *context EXT2FS_ATTR((unused)))
{
	clear_secrets();
	exit(-1);
}

static void sigcatcher_setup(void)
{
	struct sigaction	sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = die_signal_handler;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGHUP, &sa, 0);
	sigaction(SIGINT, &sa, 0);
	sigaction(SIGQUIT, &sa, 0);
	sigaction(SIGFPE, &sa, 0);
	sigaction(SIGILL, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGABRT, &sa, 0);
	sigaction(SIGPIPE, &sa, 0);
	sigaction(SIGALRM, &sa, 0);
	sigaction(SIGTERM, &sa, 0);
	sigaction(SIGUSR1, &sa, 0);
	sigaction(SIGUSR2, &sa, 0);
	sigaction(SIGPOLL, &sa, 0);
	sigaction(SIGPROF, &sa, 0);
	sigaction(SIGSYS, &sa, 0);
	sigaction(SIGTRAP, &sa, 0);
	sigaction(SIGVTALRM, &sa, 0);
	sigaction(SIGXCPU, &sa, 0);
	sigaction(SIGXFSZ, &sa, 0);
}


#define PARSE_FLAGS_NOTSUPP_OK	0x0001
#define PARSE_FLAGS_FORCE_FN	0x0002

static void parse_salt(char *salt_str, int flags)
{
	unsigned char buf[EXT4_MAX_SALT_SIZE];
	char *cp = salt_str;
	unsigned char *salt_buf;
	int fd, ret, salt_len = 0;

	if (flags & PARSE_FLAGS_FORCE_FN)
		goto salt_from_filename;
	if (strncmp(cp, "s:", 2) == 0) {
		cp += 2;
		salt_len = strlen(cp);
		if (salt_len >= EXT4_MAX_SALT_SIZE)
			goto invalid_salt;
		strncpy((char *) buf, cp, sizeof(buf));
	} else if (cp[0] == '/') {
	salt_from_filename:
		fd = open(cp, O_RDONLY | O_DIRECTORY);
		if (fd == -1 && errno == ENOTDIR)
			fd = open(cp, O_RDONLY);
		if (fd == -1) {
			perror(cp);
			exit(1);
		}
		ret = ioctl(fd, EXT4_IOC_GET_ENCRYPTION_PWSALT, &buf);
		close(fd);
		if (ret < 0) {
			if (flags & PARSE_FLAGS_NOTSUPP_OK)
				return;
			perror("EXT4_IOC_GET_ENCRYPTION_PWSALT");
			exit(1);
		}
		if (options & OPT_VERBOSE) {
			char tmp[80];
			uuid_unparse(buf, tmp);
			printf("%s has pw salt %s\n", cp, tmp);
		}
		salt_len = 16;
	} else if (strncmp(cp, "f:", 2) == 0) {
		cp += 2;
		goto salt_from_filename;
	} else if (strncmp(cp, "0x", 2) == 0) {
		unsigned char *h, *l;

		cp += 2;
		if (strlen(cp) & 1)
			goto invalid_salt;
		while (*cp) {
			if (salt_len >= EXT4_MAX_SALT_SIZE)
				goto invalid_salt;
			h = memchr(hexchars, *cp++, hexchars_size);
			l = memchr(hexchars, *cp++, hexchars_size);
			if (!h || !l)
				goto invalid_salt;
			buf[salt_len++] =
				(((unsigned char)(h - hexchars) << 4) +
				 (unsigned char)(l - hexchars));
		}
	} else if (uuid_parse(cp, buf) == 0) {
		salt_len = 16;
	} else {
	invalid_salt:
		fprintf(stderr, "Invalid salt: %s\n", salt_str);
		exit(1);
	}
	salt_buf = malloc(salt_len);
	if (!salt_buf) {
		fprintf(stderr, "Couldn't allocate salt\n");
		exit(1);
	}
	memcpy(salt_buf, buf, salt_len);
	add_salt(salt_buf, salt_len);
}

static void set_policy(struct salt *set_salt, int pad,
		       int argc, char *argv[], int path_start_index)
{
	struct salt *salt;
	struct ext4_encryption_policy policy;
	uuid_t	uu;
	int fd;
	int x;
	int rc;

	if ((pad != 4) && (pad != 8) &&
		 (pad != 16) && (pad != 32)) {
		fprintf(stderr, "Invalid padding %d\n", pad);
		exit(1);
	}

	for (x = path_start_index; x < argc; x++) {
		fd = open(argv[x], O_DIRECTORY);
		if (fd == -1) {
			perror(argv[x]);
			exit(1);
		}
		if (set_salt)
			salt = set_salt;
		else {
			if (ioctl(fd, EXT4_IOC_GET_ENCRYPTION_PWSALT,
				  &uu) < 0) {
				perror("EXT4_IOC_GET_ENCRYPTION_PWSALT");
				exit(1);
			}
			salt = find_by_salt(uu, sizeof(uu));
			if (!salt) {
				fprintf(stderr, "Couldn't find salt!?!\n");
				exit(1);
			}
		}
		policy.version = 0;
		policy.contents_encryption_mode =
			EXT4_ENCRYPTION_MODE_AES_256_XTS;
		policy.filenames_encryption_mode =
			EXT4_ENCRYPTION_MODE_AES_256_CTS;
		policy.flags = int_log2(pad >> 2);
		memcpy(policy.master_key_descriptor, salt->key_desc,
		       EXT4_KEY_DESCRIPTOR_SIZE);
		rc = ioctl(fd, EXT4_IOC_SET_ENCRYPTION_POLICY, &policy);
		close(fd);
		if (rc) {
			printf("Error [%s] setting policy.\nThe key descriptor "
			       "[%s] may not match the existing encryption "
			       "context for directory [%s].\n",
			       strerror(errno), salt->key_ref_str, argv[x]);
			continue;
		}
		printf("Key with descriptor [%s] applied to %s.\n",
		       salt->key_ref_str, argv[x]);
	}
}

static void pbkdf2_sha512(const char *passphrase, struct salt *salt,
			  unsigned int count,
			  unsigned char derived_key[EXT4_MAX_KEY_SIZE])
{
	size_t passphrase_size = strlen(passphrase);
	unsigned char buf[SHA512_LENGTH + EXT4_MAX_PASSPHRASE_SIZE] = {0};
	unsigned char tempbuf[SHA512_LENGTH] = {0};
	char final[SHA512_LENGTH] = {0};
	unsigned char saltbuf[EXT4_MAX_SALT_SIZE + EXT4_MAX_PASSPHRASE_SIZE] = {0};
	int actual_buf_len = SHA512_LENGTH + passphrase_size;
	int actual_saltbuf_len = EXT4_MAX_SALT_SIZE + passphrase_size;
	unsigned int x, y;
	__u32 *final_u32 = (__u32 *)final;
	__u32 *temp_u32 = (__u32 *)tempbuf;

	if (passphrase_size > EXT4_MAX_PASSPHRASE_SIZE) {
		printf("Passphrase size is %zd; max is %d.\n", passphrase_size,
		       EXT4_MAX_PASSPHRASE_SIZE);
		exit(1);
	}
	if (salt->salt_len > EXT4_MAX_SALT_SIZE) {
		printf("Salt size is %zd; max is %d.\n", salt->salt_len,
		       EXT4_MAX_SALT_SIZE);
		exit(1);
	}
	assert(EXT4_MAX_KEY_SIZE <= SHA512_LENGTH);

	memcpy(saltbuf, salt->salt, salt->salt_len);
	memcpy(&saltbuf[EXT4_MAX_SALT_SIZE], passphrase, passphrase_size);

	memcpy(&buf[SHA512_LENGTH], passphrase, passphrase_size);

	for (x = 0; x < count; ++x) {
		if (x == 0) {
			ext2fs_sha512(saltbuf, actual_saltbuf_len, tempbuf);
		} else {
			/*
			 * buf: [previous hash || passphrase]
			 */
			memcpy(buf, tempbuf, SHA512_LENGTH);
			ext2fs_sha512(buf, actual_buf_len, tempbuf);
		}
		for (y = 0; y < (sizeof(final) / sizeof(*final_u32)); ++y)
			final_u32[y] = final_u32[y] ^ temp_u32[y];
	}
	memcpy(derived_key, final, EXT4_MAX_KEY_SIZE);
}

static int disable_echo(struct termios *saved_settings)
{
	struct termios current_settings;
	int rc = 0;

	rc = tcgetattr(0, &current_settings);
	if (rc)
		return rc;
	*saved_settings = current_settings;
	current_settings.c_lflag &= ~ECHO;
	rc = tcsetattr(0, TCSANOW, &current_settings);

	return rc;
}

static void get_passphrase(char *passphrase, int len)
{
	char *p;
	struct termios current_settings;

	assert(len > 0);
	disable_echo(&current_settings);
	p = fgets(passphrase, len, stdin);
	tcsetattr(0, TCSANOW, &current_settings);
	printf("\n");
	if (!p) {
		printf("Aborting.\n");
		exit(1);
	}
	p = strrchr(passphrase, '\n');
	if (!p)
		p = passphrase + len - 1;
	*p = '\0';
}

struct keyring_map {
	char name[4];
	size_t name_len;
	int code;
};

static const struct keyring_map keyrings[] = {
	{"@us", 3, KEY_SPEC_USER_SESSION_KEYRING},
	{"@u", 2, KEY_SPEC_USER_KEYRING},
	{"@s", 2, KEY_SPEC_SESSION_KEYRING},
	{"@g", 2, KEY_SPEC_GROUP_KEYRING},
	{"@p", 2, KEY_SPEC_PROCESS_KEYRING},
	{"@t", 2, KEY_SPEC_THREAD_KEYRING},
};

static int get_keyring_id(const char *keyring)
{
	unsigned int x;
	char *end;

	/*
	 * If no keyring is specified, by default use either the user
	 * session keyring or the session keyring.  Fetching the
	 * session keyring will return the user session keyring if no
	 * session keyring has been set.
	 */
	if (keyring == NULL)
		return KEY_SPEC_SESSION_KEYRING;
	for (x = 0; x < (sizeof(keyrings) / sizeof(keyrings[0])); ++x) {
		if (strcmp(keyring, keyrings[x].name) == 0) {
			return keyrings[x].code;
		}
	}
	x = strtoul(keyring, &end, 10);
	if (*end == '\0') {
		if (keyctl(KEYCTL_DESCRIBE, x, NULL, 0) < 0)
			return 0;
		return x;
	}
	return 0;
}

static void generate_key_ref_str(struct salt *salt)
{
	unsigned char key_ref1[SHA512_LENGTH];
	unsigned char key_ref2[SHA512_LENGTH];
	int x;

	ext2fs_sha512(salt->key, EXT4_MAX_KEY_SIZE, key_ref1);
	ext2fs_sha512(key_ref1, SHA512_LENGTH, key_ref2);
	memcpy(salt->key_desc, key_ref2, EXT4_KEY_DESCRIPTOR_SIZE);
	for (x = 0; x < EXT4_KEY_DESCRIPTOR_SIZE; ++x) {
		sprintf(&salt->key_ref_str[x * 2], "%02x",
			salt->key_desc[x]);
	}
	salt->key_ref_str[EXT4_KEY_REF_STR_BUF_SIZE - 1] = '\0';
}

static void insert_key_into_keyring(const char *keyring, struct salt *salt)
{
	int keyring_id = get_keyring_id(keyring);
	struct ext4_encryption_key key;
	char key_ref_full[EXT2FS_KEY_DESC_PREFIX_SIZE +
			  EXT4_KEY_REF_STR_BUF_SIZE];
	int rc;

	if (keyring_id == 0) {
		printf("Invalid keyring [%s].\n", keyring);
		exit(1);
	}
	sprintf(key_ref_full, "%s%s", EXT2FS_KEY_DESC_PREFIX,
		salt->key_ref_str);
	rc = keyctl(KEYCTL_SEARCH, keyring_id, EXT2FS_KEY_TYPE_LOGON,
		    key_ref_full, 0);
	if (rc != -1) {
		if ((options & OPT_QUIET) == 0)
			printf("Key with descriptor [%s] already exists\n",
			       salt->key_ref_str);
		return;
	} else if ((rc == -1) && (errno != ENOKEY)) {
		printf("keyctl_search failed: %s\n", strerror(errno));
		if (errno == EINVAL)
			printf("Keyring [%s] is not available.\n", keyring);
		exit(1);
	}
	key.mode = EXT4_ENCRYPTION_MODE_AES_256_XTS;
	memcpy(key.raw, salt->key, EXT4_MAX_KEY_SIZE);
	key.size = EXT4_MAX_KEY_SIZE;

	/*
	 * We need to do this instead of simply adding the key to
	 * KEY_SPEC_SESSION_KEYRING since trying to add a key to a
	 * session keyring that does not yet exist will cause the
	 * kernel to create a session keyring --- which will then get
	 * garbage collected as soon as e4crypt exits.
	 *
	 * The fact that the keyctl system call and the add_key system
	 * call treats KEY_SPEC_SESSION_KEYRING differently when a
	 * session keyring does not exist is very unfortunate and
	 * confusing, but so it goes...
	 */
	if (keyring_id == KEY_SPEC_SESSION_KEYRING) {
		keyring_id = keyctl(KEYCTL_GET_KEYRING_ID, keyring_id, 0);
		if (keyring_id < 0) {
			printf("Error getting session keyring ID: %s\n",
			       strerror(errno));
			exit(1);
		}
	}
	rc = add_key(EXT2FS_KEY_TYPE_LOGON, key_ref_full, (void *)&key,
		     sizeof(key), keyring_id);
	if (rc == -1) {
		if (errno == EDQUOT) {
			printf("Error adding key to keyring; quota exceeded\n");
		} else {
			printf("Error adding key with key descriptor [%s]: "
			       "%s\n", salt->key_ref_str, strerror(errno));
		}
		exit(1);
	} else {
		if ((options & OPT_QUIET) == 0)
			printf("Added key with descriptor [%s]\n",
			       salt->key_ref_str);
	}
}

static void get_default_salts(void)
{
	FILE	*f = setmntent("/etc/mtab", "r");
	struct mntent *mnt;

	while (f && ((mnt = getmntent(f)) != NULL)) {
		if (strcmp(mnt->mnt_type, "ext4") ||
		    access(mnt->mnt_dir, R_OK))
			continue;
		parse_salt(mnt->mnt_dir, PARSE_FLAGS_NOTSUPP_OK);
	}
	endmntent(f);
}

/* Functions which implement user commands */

struct cmd_desc {
	const char *cmd_name;
	void (*cmd_func)(int, char **, const struct cmd_desc *);
	const char *cmd_desc;
	const char *cmd_help;
	int cmd_flags;
};

#define CMD_HIDDEN 	0x0001

static void do_help(int argc, char **argv, const struct cmd_desc *cmd);

#define add_key_desc "adds a key to the user's keyring"
#define add_key_help \
"e4crypt add_key -S salt [ -k keyring ] [-v] [-q] [ -p pad ] [ path ... ]\n\n" \
"Prompts the user for a passphrase and inserts it into the specified\n" \
"keyring.  If no keyring is specified, e4crypt will use the session\n" \
"keyring if it exists or the user session keyring if it does not.\n\n" \
"If one or more directory paths are specified, e4crypt will try to\n" \
"set the policy of those directories to use the key just entered by\n" \
"the user.\n"

static void do_add_key(int argc, char **argv, const struct cmd_desc *cmd)
{
	struct salt *salt;
	char *keyring = NULL;
	int i, opt, pad = 4;
	unsigned j;

	while ((opt = getopt(argc, argv, "k:S:p:vq")) != -1) {
		switch (opt) {
		case 'k':
			/* Specify a keyring. */
			keyring = optarg;
			break;
		case 'p':
			pad = atoi(optarg);
			break;
		case 'S':
			/* Salt value for passphrase. */
			parse_salt(optarg, 0);
			break;
		case 'v':
			options |= OPT_VERBOSE;
			break;
		case 'q':
			options |= OPT_QUIET;
			break;
		default:
		case '?':
			if (opt != '?')
				fprintf(stderr, "Unrecognized option: %c\n",
					opt);
			fputs("USAGE:\n  ", stderr);
			fputs(cmd->cmd_help, stderr);
			exit(1);
		}
	}
	if (num_salt == 0)
		get_default_salts();
	if (num_salt == 0) {
		fprintf(stderr, "No salt values available\n");
		exit(1);
	}
	validate_paths(argc, argv, optind);
	for (i = optind; i < argc; i++)
		parse_salt(argv[i], PARSE_FLAGS_FORCE_FN);
	printf("Enter passphrase (echo disabled): ");
	get_passphrase(in_passphrase, sizeof(in_passphrase));
	for (j = 0, salt = salt_list; j < num_salt; j++, salt++) {
		pbkdf2_sha512(in_passphrase, salt,
			      EXT4_PBKDF2_ITERATIONS, salt->key);
		generate_key_ref_str(salt);
		insert_key_into_keyring(keyring, salt);
	}
	if (optind != argc)
		set_policy(NULL, pad, argc, argv, optind);
	clear_secrets();
	exit(0);
}

#define set_policy_desc "sets a policy for directories"
#define set_policy_help \
"e4crypt set_policy [ -p pad ] policy path ... \n\n" \
"Sets the policy for the directories specified on the command line.\n" \
"All directories must be empty to set the policy; if the directory\n" \
"already has a policy established, e4crypt will validate that it the\n" \
"policy matches what was specified.  A policy is an encryption key\n" \
"identifier consisting of 16 hexadecimal characters.\n"

static void do_set_policy(int argc, char **argv, const struct cmd_desc *cmd)
{
	struct salt saltbuf;
	int c, pad = 4;

	while ((c = getopt (argc, argv, "p:")) != EOF) {
		switch (c) {
		case 'p':
			pad = atoi(optarg);
			break;
		}
	}

	if (argc < optind + 2) {
		fprintf(stderr, "Missing required argument(s).\n\n");
		fputs("USAGE:\n  ", stderr);
		fputs(cmd->cmd_help, stderr);
		exit(1);
	}

	if ((strlen(argv[optind]) != (EXT4_KEY_DESCRIPTOR_SIZE * 2)) ||
	    hex2byte(argv[optind], (EXT4_KEY_DESCRIPTOR_SIZE * 2),
		     saltbuf.key_desc, EXT4_KEY_DESCRIPTOR_SIZE)) {
		printf("Invalid key descriptor [%s]. Valid characters "
		       "are 0-9 and a-f, lower case.  "
		       "Length must be %d.\n",
		       argv[optind], (EXT4_KEY_DESCRIPTOR_SIZE * 2));
			exit(1);
	}
	validate_paths(argc, argv, optind+1);
	strcpy(saltbuf.key_ref_str, argv[optind]);
	set_policy(&saltbuf, pad, argc, argv, optind+1);
	exit(0);
}

#define get_policy_desc "get the encryption for directories"
#define get_policy_help \
"e4crypt get_policy path ... \n\n" \
"Gets the policy for the directories specified on the command line.\n"

static void do_get_policy(int argc, char **argv, const struct cmd_desc *cmd)
{
	struct ext4_encryption_policy policy;
	int i, j, fd, rc;

	if (argc < 2) {
		fprintf(stderr, "Missing required argument(s).\n\n");
		fputs("USAGE:\n  ", stderr);
		fputs(cmd->cmd_help, stderr);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		fd = open(argv[i], O_RDONLY);
		if (fd == -1) {
			perror(argv[i]);
			exit(1);
		}
		rc = ioctl(fd, EXT4_IOC_GET_ENCRYPTION_POLICY, &policy);
		close(fd);
		if (rc) {
			printf("Error getting policy for %s: %s\n",
			       argv[i], strerror(errno));
			continue;
		}
		printf("%s: ", argv[i]);
		for (j = 0; j < EXT4_KEY_DESCRIPTOR_SIZE; j++) {
			printf("%02x", (unsigned char) policy.master_key_descriptor[j]);
		}
		fputc('\n', stdout);
	}
	exit(0);
}

#define new_session_desc "give the invoking process a new session keyring"
#define new_session_help \
"e4crypt new_session\n\n" \
"Give the invoking process (typically a shell) a new session keyring,\n" \
"discarding its old session keyring.\n"

static void do_new_session(int argc, char **argv EXT2FS_ATTR((unused)),
			   const struct cmd_desc *cmd)
{
	long keyid, ret;

	if (argc > 1) {
		fputs("Excess arguments\n\n", stderr);
		fputs(cmd->cmd_help, stderr);
		exit(1);
	}
	keyid = keyctl(KEYCTL_JOIN_SESSION_KEYRING, NULL);
	if (keyid < 0) {
		perror("KEYCTL_JOIN_SESSION_KEYRING");
		exit(1);
	}
	ret = keyctl(KEYCTL_SESSION_TO_PARENT, NULL);
	if (ret < 0) {
		perror("KEYCTL_SESSION_TO_PARENT");
		exit(1);
	}
	printf("Switched invoking process to new session keyring %ld\n", keyid);
	exit(0);
}

#define CMD(name) { #name, do_##name, name##_desc, name##_help, 0 }
#define _CMD(name) { #name, do_##name, NULL, NULL, CMD_HIDDEN }

const struct cmd_desc cmd_list[] = {
	_CMD(help),
	CMD(add_key),
	CMD(get_policy),
	CMD(new_session),
	CMD(set_policy),
	{ NULL, NULL, NULL, NULL, 0 }
};

static void do_help(int argc, char **argv,
		    const struct cmd_desc *cmd EXT2FS_ATTR((unused)))
{
	const struct cmd_desc *p;

	if (argc > 1) {
		for (p = cmd_list; p->cmd_name; p++) {
			if (p->cmd_flags & CMD_HIDDEN)
				continue;
			if (strcmp(p->cmd_name, argv[1]) == 0) {
				putc('\n', stdout);
				fputs("USAGE:\n  ", stdout);
				fputs(p->cmd_help, stdout);
				exit(0);
			}
		}
		printf("Unknown command: %s\n\n", argv[1]);
	}

	fputs("Available commands:\n", stdout);
	for (p = cmd_list; p->cmd_name; p++) {
		if (p->cmd_flags & CMD_HIDDEN)
			continue;
		printf("  %-20s %s\n", p->cmd_name, p->cmd_desc);
	}
	printf("\nTo get more information on a command, "
	       "type 'e4crypt help cmd'\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	const struct cmd_desc *cmd;

	if (argc < 2)
		do_help(argc, argv, cmd_list);

	sigcatcher_setup();
	for (cmd = cmd_list; cmd->cmd_name; cmd++) {
		if (strcmp(cmd->cmd_name, argv[1]) == 0) {
			cmd->cmd_func(argc-1, argv+1, cmd);
			exit(0);
		}
	}
	printf("Unknown command: %s\n\n", argv[1]);
	do_help(1, argv, cmd_list);
	return 0;
}
