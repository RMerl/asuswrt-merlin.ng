// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * This is a shell script written in C. It very intentionally still functions like
 * a shell script, calling out to external executables such as ip(8).
 */

#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>

#ifndef WG_PACKAGE_NAME
#define WG_PACKAGE_NAME "com.wireguard.android"
#endif
#ifndef WG_CONFIG_SEARCH_PATHS
#define WG_CONFIG_SEARCH_PATHS "/data/misc/wireguard /data/data/" WG_PACKAGE_NAME "/files"
#endif

#define _printf_(x, y) __attribute__((format(printf, x, y)))
#define _cleanup_(x) __attribute__((cleanup(x)))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static bool is_exiting = false;
static bool binder_available = false;

static void *xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (ret)
		return ret;
	perror("Error: malloc");
	exit(errno);
}

static void *xcalloc(size_t nmemb, size_t size)
{
	void *ret = calloc(nmemb, size);
	if (ret)
		return ret;
	perror("Error: calloc");
	exit(errno);
}

static void *xstrdup(const char *str)
{
	char *ret = strdup(str);
	if (ret)
		return ret;
	perror("Error: strdup");
	exit(errno);
}

static void xregcomp(regex_t *preg, const char *regex, int cflags)
{
	if (regcomp(preg, regex, cflags)) {
		fprintf(stderr, "Error: Regex compilation error\n");
		exit(EBADR);
	}
}

static char *concat(char *first, ...)
{
	va_list args;
	size_t len = 0;
	char *ret;

	va_start(args, first);
	for (char *i = first; i; i = va_arg(args, char *))
		len += strlen(i);
	va_end(args);

	ret = xmalloc(len + 1);
	ret[0] = '\0';

	va_start(args, first);
	for (char *i = first; i; i = va_arg(args, char *))
		strcat(ret, i);
	va_end(args);

	return ret;
}

static char *concat_and_free(char *orig, const char *delim, const char *new_line)
{
	char *ret;

	if (!orig)
		ret = xstrdup(new_line);
	else
		ret = concat(orig, delim, new_line, NULL);
	free(orig);
	return ret;
}

struct command_buffer {
	char *line;
	size_t len;
	FILE *stream;
};

static void free_command_buffer(struct command_buffer *c)
{
	if (!c)
		return;
	if (c->stream)
		pclose(c->stream);
	free(c->line);
}

static void freep(void *p)
{
	free(*(void **)p);
}
static void fclosep(FILE **f)
{
	if (*f)
		fclose(*f);
}
#define _cleanup_free_ _cleanup_(freep)
#define _cleanup_fclose_ _cleanup_(fclosep)
#define _cleanup_regfree_ _cleanup_(regfree)

#define DEFINE_CMD(name) _cleanup_(free_command_buffer) struct command_buffer name = { 0 };

static char *vcmd_ret(struct command_buffer *c, const char *cmd_fmt, va_list args)
{
	_cleanup_free_ char *cmd = NULL;

	if (!c->stream && !cmd_fmt)
		return NULL;
	if (c->stream && cmd_fmt)
		pclose(c->stream);

	if (cmd_fmt) {
		if (vasprintf(&cmd, cmd_fmt, args) < 0) {
			perror("Error: vasprintf");
			exit(errno);
		}

		c->stream = popen(cmd, "r");
		if (!c->stream) {
			perror("Error: popen");
			exit(errno);
		}
	}
	errno = 0;
	if (getline(&c->line, &c->len, c->stream) < 0) {
		if (errno) {
			perror("Error: getline");
			exit(errno);
		}
		return NULL;
	}
	return c->line;
}

_printf_(1, 2) static void cmd(const char *cmd_fmt, ...)
{
	_cleanup_free_ char *cmd = NULL;
	va_list args;
	int ret;

	va_start(args, cmd_fmt);
	if (vasprintf(&cmd, cmd_fmt, args) < 0) {
		perror("Error: vasprintf");
		exit(errno);
	}
	va_end(args);

	printf("[#] %s\n", cmd);
	ret = system(cmd);

	if (ret < 0)
		ret = ESRCH;
	else if (ret > 0)
		ret = WIFEXITED(ret) ? WEXITSTATUS(ret) : EIO;

	if (ret && !is_exiting)
		exit(ret);
}

_printf_(2, 3) static char *cmd_ret(struct command_buffer *c, const char *cmd_fmt, ...)
{
	va_list args;
	char *ret;

	va_start(args, cmd_fmt);
	ret = vcmd_ret(c, cmd_fmt, args);
	va_end(args);
	return ret;
}

_printf_(1, 2) static void cndc(const char *cmd_fmt, ...)
{
	DEFINE_CMD(c);
	int error_code;
	char *ret;
	va_list args;
	_cleanup_free_ char *ndc_fmt = concat("ndc ", cmd_fmt, NULL);

	va_start(args, cmd_fmt);
	printf("[#] ");
	vprintf(ndc_fmt, args);
	printf("\n");
	va_end(args);

	va_start(args, cmd_fmt);
	ret = vcmd_ret(&c, ndc_fmt, args);
	va_end(args);

	if (!ret) {
		fprintf(stderr, "Error: could not call ndc\n");
		exit(ENOSYS);
	}

	error_code = atoi(ret);
	if (error_code >= 400 && error_code < 600) {
		fprintf(stderr, "Error: %s\n", ret);
		exit(ENONET);
	}
}

/* Values are from AOSP repository platform/frameworks/native in libs/binder/ndk/include_ndk/android/binder_status.h. */
enum {
	STATUS_OK = 0,
	STATUS_UNKNOWN_ERROR = -2147483647 - 1,
	STATUS_NO_MEMORY = -ENOMEM,
	STATUS_INVALID_OPERATION = -ENOSYS,
	STATUS_BAD_VALUE = -EINVAL,
	STATUS_BAD_TYPE = STATUS_UNKNOWN_ERROR + 1,
	STATUS_NAME_NOT_FOUND = -ENOENT,
	STATUS_PERMISSION_DENIED = -EPERM,
	STATUS_NO_INIT = -ENODEV,
	STATUS_ALREADY_EXISTS = -EEXIST,
	STATUS_DEAD_OBJECT = -EPIPE,
	STATUS_FAILED_TRANSACTION = STATUS_UNKNOWN_ERROR + 2,
	STATUS_BAD_INDEX = -EOVERFLOW,
	STATUS_NOT_ENOUGH_DATA = -ENODATA,
	STATUS_WOULD_BLOCK = -EWOULDBLOCK,
	STATUS_TIMED_OUT = -ETIMEDOUT,
	STATUS_UNKNOWN_TRANSACTION = -EBADMSG,
	STATUS_FDS_NOT_ALLOWED = STATUS_UNKNOWN_ERROR + 7,
	STATUS_UNEXPECTED_NULL = STATUS_UNKNOWN_ERROR + 8
};
enum {
	EX_NONE = 0,
	EX_SECURITY = -1,
	EX_BAD_PARCELABLE = -2,
	EX_ILLEGAL_ARGUMENT = -3,
	EX_NULL_POINTER = -4,
	EX_ILLEGAL_STATE = -5,
	EX_NETWORK_MAIN_THREAD = -6,
	EX_UNSUPPORTED_OPERATION = -7,
	EX_SERVICE_SPECIFIC = -8,
	EX_PARCELABLE = -9,
	EX_TRANSACTION_FAILED = -129
};
enum {
	FLAG_ONEWAY = 0x01,
};
enum {
	FIRST_CALL_TRANSACTION = 0x00000001,
	LAST_CALL_TRANSACTION = 0x00ffffff
};
struct AIBinder;
struct AParcel;
struct AStatus;
struct AIBinder_Class;
typedef struct AIBinder AIBinder;
typedef struct AParcel AParcel;
typedef struct AStatus AStatus;
typedef struct AIBinder_Class AIBinder_Class;
typedef int32_t binder_status_t;
typedef int32_t binder_exception_t;
typedef uint32_t transaction_code_t;
typedef uint32_t binder_flags_t;
typedef void *(*AIBinder_Class_onCreate)(void *args);
typedef void (*AIBinder_Class_onDestroy)(void *userData);
typedef binder_status_t (*AIBinder_Class_onTransact)(AIBinder *binder, transaction_code_t code, const AParcel *in, AParcel *out);
typedef const char *(*AParcel_stringArrayElementGetter)(const void *arrayData, size_t index, int32_t *outLength);
static AIBinder_Class *(*AIBinder_Class_define)(const char *interfaceDescriptor, AIBinder_Class_onCreate onCreate, AIBinder_Class_onDestroy onDestroy, AIBinder_Class_onTransact onTransact) __attribute__((warn_unused_result));
static bool (*AIBinder_associateClass)(AIBinder *binder, const AIBinder_Class *clazz);
static void (*AIBinder_decStrong)(AIBinder *binder);
static binder_status_t (*AIBinder_prepareTransaction)(AIBinder *binder, AParcel **in);
static binder_status_t (*AIBinder_transact)(AIBinder *binder, transaction_code_t code, AParcel **in, AParcel **out, binder_flags_t flags);
static binder_status_t (*AIBinder_ping)(AIBinder *binder);
static binder_status_t (*AIBinder_dump)(AIBinder *binder, int fd, const char **args, uint32_t numArgs);
static binder_status_t (*AParcel_readStatusHeader)(const AParcel *parcel, AStatus **status);
static binder_status_t (*AParcel_readBool)(const AParcel *parcel, bool *value);
static void (*AParcel_delete)(AParcel *parcel);
static binder_status_t (*AParcel_setDataPosition)(const AParcel *parcel, int32_t position);
static int32_t (*AParcel_getDataPosition)(const AParcel *parcel);
static binder_status_t (*AParcel_writeInt32)(AParcel *parcel, int32_t value);
static binder_status_t (*AParcel_writeStringArray)(AParcel *parcel, const void *arrayData, int32_t length, AParcel_stringArrayElementGetter getter);
static binder_status_t (*AParcel_writeString)(AParcel *parcel, const char *string, int32_t length);
static bool (*AStatus_isOk)(const AStatus *status);
static void (*AStatus_delete)(AStatus *status);
static binder_exception_t (*AStatus_getExceptionCode)(const AStatus *status);
static int32_t (*AStatus_getServiceSpecificError)(const AStatus *status);
static const char* (*AStatus_getMessage)(const AStatus *status);
static binder_status_t (*AStatus_getStatus)(const AStatus *status);
static AIBinder *(*AServiceManager_getService)(const char *instance) __attribute__((__warn_unused_result__));

static	__attribute__((__constructor__(65535))) void load_symbols(void)
{
	void *handle = dlopen("libbinder_ndk.so", RTLD_LAZY);
	binder_available = !!handle;
	if (!binder_available)
		return;

#define X(symb) do {												\
			if (!((symb) = (typeof(symb))dlsym(handle, #symb))) {					\
				fprintf(stderr, "Error: unable to import " #symb " from libbinder_ndk.so\n");	\
				exit(ELIBACC);									\
			}											\
		} while (0)
	X(AIBinder_Class_define);
	X(AIBinder_associateClass);
	X(AIBinder_decStrong);
	X(AIBinder_prepareTransaction);
	X(AIBinder_transact);
	X(AIBinder_ping);
	X(AIBinder_dump);
	X(AParcel_readStatusHeader);
	X(AParcel_readBool);
	X(AParcel_delete);
	X(AParcel_setDataPosition);
	X(AParcel_getDataPosition);
	X(AParcel_writeInt32);
	X(AParcel_writeStringArray);
	X(AParcel_writeString);
	X(AStatus_isOk);
	X(AStatus_delete);
	X(AStatus_getExceptionCode);
	X(AStatus_getServiceSpecificError);
	X(AStatus_getMessage);
	X(AStatus_getStatus);
	X(AServiceManager_getService);
#undef X
}

static void cleanup_binder(AIBinder **binder)
{
	if (*binder)
		AIBinder_decStrong(*binder);
}
static void cleanup_status(AStatus **status)
{
	if (*status)
		AStatus_delete(*status);
}
static void cleanup_parcel(AParcel **parcel)
{
	if (*parcel)
		AParcel_delete(*parcel);
}

#define _cleanup_status_ __attribute__((__cleanup__(cleanup_status)))
#define _cleanup_parcel_ __attribute__((__cleanup__(cleanup_parcel)))
#define _cleanup_binder_ __attribute__((__cleanup__(cleanup_binder)))

static int32_t string_size(const char *str)
{
	return str ? strlen(str) : -1;
}

static int32_t string_array_size(char *const *array)
{
	int32_t size = -1;
	if (!array)
		return size;
	for (size = 0; array[size]; ++size);
	return size;
}

static const char *string_array_getter(const void *array_data, size_t index, int32_t *out_length)
{
	const char **array = (const char **)array_data;
	*out_length = array[index] ? strlen(array[index]) : -1;
	return array[index];
}

static binder_status_t meaningful_binder_status(const AStatus *status_out)
{
	binder_status_t status = STATUS_OK;
	binder_exception_t exc_code;
	int32_t exc_code_service;
	const char *message;

	if (!AStatus_isOk(status_out)) {
		exc_code = AStatus_getExceptionCode(status_out);
		if (exc_code == EX_TRANSACTION_FAILED) {
			status = AStatus_getStatus(status_out);
			fprintf(stderr, "Error: transaction failed: %d\n", status);
		}
		else {
			message = AStatus_getMessage(status_out);

			if (exc_code == EX_SERVICE_SPECIFIC) {
				exc_code_service = AStatus_getServiceSpecificError(status_out);
				fprintf(stderr, "Error: service specific exception code: %d%s%s\n", exc_code_service, message ? ": " : "", message ?: "");
			}
			else
				fprintf(stderr, "Error: exception code: %d%s%s\n", exc_code, message ? ": " : "", message ?: "");

			status = STATUS_FAILED_TRANSACTION;
		}
	}

	return status;
}

/* These values are default values observed in AOSP. */
enum {
	DNSRESOLVER_SAMPLE_VALIDITY = 1800 /* sec */,
	DNSRESOLVER_SUCCESS_THRESHOLD = 25,
	DNSRESOLVER_MIN_SAMPLES = 8,
	DNSRESOLVER_MAX_SAMPLES = 8,
	DNSRESOLVER_BASE_TIMEOUT = 5000 /* msec */,
	DNSRESOLVER_RETRY_COUNT = 2
};

struct dnsresolver_params {
	int32_t netid;
	int32_t sample_validity_seconds;
	int32_t success_threshold;
	int32_t min_samples;
	int32_t max_samples;
	int32_t base_timeout_msec;
	int32_t retry_count;
	char **servers;          /* NULL terminated array of zero-terminated UTF-8 strings */
	char **domains;          /* NULL terminated array of zero-terminated UTF-8 strings */
	char *tls_name;          /* zero-terminated UTF-8 string													 */
	char **tls_servers;      /* NULL terminated array of zero-terminated UTF-8 strings */
	char **tls_fingerprints; /* NULL terminated array of zero-terminated UTF-8 strings */
};

static void *on_create()
{
	fprintf(stderr, "Error: on_create called on proxy object\n");
	exit(ENOTSUP);
	return NULL;
}

static void on_destroy()
{
	fprintf(stderr, "Error: on_destroy called on proxy object\n");
	exit(ENOTSUP);
}

static binder_status_t on_transact()
{
	fprintf(stderr, "Error: on_transact called on a proxy object\n");
	exit(ENOTSUP);
	return 0;
}

static AIBinder *dnsresolver_get_handle(void)
{
	AIBinder *binder;
	AIBinder_Class *clazz;

	if (!binder_available)
		return NULL;

	binder = AServiceManager_getService("dnsresolver");
	if (!binder)
		return NULL;
	clazz = AIBinder_Class_define("android.net.IDnsResolver", &on_create, &on_destroy, &on_transact);
	if (!clazz)
		goto error;

	if (!AIBinder_associateClass(binder, clazz))
		goto error;

	return binder;
error:
	AIBinder_decStrong(binder);
	return NULL;
}

static int32_t dnsresolver_create_network_cache(void *handle, int32_t netid)
{
	AIBinder *const binder = handle;
	binder_status_t status;
	_cleanup_parcel_ AParcel *parcel_in = NULL;
	_cleanup_parcel_ AParcel *parcel_out = NULL;
	_cleanup_status_ AStatus *status_out = NULL;

	status = AIBinder_prepareTransaction(binder, &parcel_in);
	if (status != STATUS_OK)
		return status;

	status = AParcel_writeInt32(parcel_in, netid);
	if (status != STATUS_OK)
		return status;

	status = AIBinder_transact(binder, FIRST_CALL_TRANSACTION + 7 /* createNetworkCache */, &parcel_in, &parcel_out, 0);
	if (status != STATUS_OK)
		return status;

	status = AParcel_readStatusHeader(parcel_out, &status_out);
	if (status != STATUS_OK)
		return status;

	if (!AStatus_isOk(status_out))
		return meaningful_binder_status(status_out);

	return STATUS_OK;
}

static int32_t dnsresolver_set_resolver_configuration(void *handle, const struct dnsresolver_params *params)
{
	AIBinder *const binder = handle;
	binder_status_t status;
	_cleanup_parcel_ AParcel *parcel_in = NULL;
	_cleanup_parcel_ AParcel *parcel_out = NULL;
	_cleanup_status_ AStatus *status_out = NULL;
	int32_t start_position, end_position;

	status = AIBinder_prepareTransaction(binder, &parcel_in);
	if (status != STATUS_OK)
		return status;

	status = AParcel_writeInt32(parcel_in, 1);
	if (status != STATUS_OK)
		return status;

	start_position = AParcel_getDataPosition(parcel_in);
	status = AParcel_writeInt32(parcel_in, 0);
	if (status != STATUS_OK)
		return status;

	status = AParcel_writeInt32(parcel_in, params->netid);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, params->sample_validity_seconds);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, params->success_threshold);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, params->min_samples);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, params->max_samples);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, params->base_timeout_msec);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, params->retry_count);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeStringArray(parcel_in, params->servers, string_array_size(params->servers), &string_array_getter);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeStringArray(parcel_in, params->domains, string_array_size(params->domains), &string_array_getter);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeString(parcel_in, params->tls_name, string_size(params->tls_name));
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeStringArray(parcel_in, params->tls_servers, string_array_size(params->tls_servers), &string_array_getter);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeStringArray(parcel_in, params->tls_fingerprints, string_array_size(params->tls_fingerprints), &string_array_getter);
	if (status != STATUS_OK)
		return status;

	end_position = AParcel_getDataPosition(parcel_in);
	status = AParcel_setDataPosition(parcel_in, start_position);
	if (status != STATUS_OK)
		return status;
	status = AParcel_writeInt32(parcel_in, end_position - start_position);
	if (status != STATUS_OK)
		return status;
	status = AParcel_setDataPosition(parcel_in, end_position);
	if (status != STATUS_OK)
		return status;

	status = AIBinder_transact(binder, FIRST_CALL_TRANSACTION + 2 /* setResolverConfiguration */, &parcel_in, &parcel_out, 0);
	if (status != STATUS_OK)
		return status;

	status = AParcel_readStatusHeader(parcel_out, &status_out);
	if (status != STATUS_OK)
		return status;

	return meaningful_binder_status(status_out);
}

static void auto_su(int argc, char *argv[])
{
	char *args[argc + 4];

	if (!getuid())
		return;

	args[0] = "su";
	args[1] = "-p";
	args[2] = "-c";
	memcpy(&args[3], argv, argc * sizeof(*args));
	args[argc + 3] = NULL;

	printf("[$] su -p -c ");
	for (int i = 0; i < argc; ++i)
		printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');

	execvp("su", args);
	exit(errno);
}

static void add_if(const char *iface)
{
	cmd("ip link add %s type wireguard", iface);
}

static void del_if(const char *iface)
{
	DEFINE_CMD(c_rule);
	DEFINE_CMD(c_iptables);
	DEFINE_CMD(c_ip6tables);
	_cleanup_regfree_ regex_t rule_reg = { 0 }, iptables_reg = { 0 };
	regmatch_t matches[2];
	char *netid = NULL;
	_cleanup_free_ char *rule_regex = concat("0xc([0-9a-f]+)/0xcffff lookup ", iface, NULL);
	_cleanup_free_ char *iptables_regex = concat("^-A (.* --comment \"wireguard rule ", iface, "\"[^\n]*)\n*$", NULL);

	xregcomp(&rule_reg, rule_regex, REG_EXTENDED);
	xregcomp(&iptables_reg, iptables_regex, REG_EXTENDED);

	cmd("ip link del %s", iface);

	for (char *rule = cmd_ret(&c_iptables, "iptables-save"); rule; rule = cmd_ret(&c_iptables, NULL)) {
		if (!regexec(&iptables_reg, rule, ARRAY_SIZE(matches), matches, 0)) {
			rule[matches[1].rm_eo] = '\0';
			cmd("iptables -D %s", &rule[matches[1].rm_so]);
		}
	}
	for (char *rule = cmd_ret(&c_ip6tables, "ip6tables-save"); rule; rule = cmd_ret(&c_ip6tables, NULL)) {
		if (!regexec(&iptables_reg, rule, ARRAY_SIZE(matches), matches, 0)) {
			rule[matches[1].rm_eo] = '\0';
			cmd("ip6tables -D %s", &rule[matches[1].rm_so]);
		}
	}

	for (char *rule = cmd_ret(&c_rule, "ip rule show"); rule; rule = cmd_ret(&c_rule, NULL)) {
		if (!regexec(&rule_reg, rule, ARRAY_SIZE(matches), matches, 0)) {
			rule[matches[1].rm_eo] = '\0';
			netid = &rule[matches[1].rm_so];
			break;
		}
	}
	if (netid)
		cndc("network destroy %lu", strtoul(netid, NULL, 16));
}

static bool should_block_ipv6(const char *iface)
{
	DEFINE_CMD(c);
	bool has_ipv6 = false, has_all_none = true;

	for (char *endpoint = cmd_ret(&c, "wg show %s endpoints", iface); endpoint; endpoint = cmd_ret(&c, NULL)) {
		char *start = strchr(endpoint, '\t');

		if (!start)
			continue;
		++start;
		if (start[0] != '(')
			has_all_none = false;
		if (start[0] == '[')
			has_ipv6 = true;
	}
	return !has_ipv6 && !has_all_none;
}

static uint16_t determine_listen_port(const char *iface)
{
	DEFINE_CMD(c);
	unsigned long listen_port = 0;
	char *value;

	cmd("ip link set up dev %s", iface);
	value = cmd_ret(&c, "wg show %s listen-port", iface);
	if (!value)
		goto set_back_down;
	listen_port = strtoul(value, NULL, 10);
	if (listen_port > UINT16_MAX || !listen_port) {
		listen_port = 0;
		goto set_back_down;
	}
set_back_down:
	cmd("ip link set down dev %s", iface);
	return listen_port;
}

static void up_if(unsigned int *netid, const char *iface, uint16_t listen_port)
{
	srandom(time(NULL) ^ getpid()); /* Not real randomness. */

	while (*netid < 4096)
		*netid = random() & 0xfffe;

	cmd("wg set %s fwmark 0x20000", iface);
	cmd("iptables -I OUTPUT 1 -m mark --mark 0x20000 -j ACCEPT -m comment --comment \"wireguard rule %s\"", iface);
	cmd("ip6tables -I OUTPUT 1 -m mark --mark 0x20000 -j ACCEPT -m comment --comment \"wireguard rule %s\"", iface);
	if (listen_port) {
		cmd("iptables -I INPUT 1 -p udp --dport %u -j ACCEPT -m comment --comment \"wireguard rule %s\"", listen_port, iface);
		cmd("ip6tables -I INPUT 1 -p udp --dport %u -j %s -m comment --comment \"wireguard rule %s\"", listen_port, should_block_ipv6(iface) ? "DROP" : "ACCEPT", iface);
	}
	cmd("ip link set up dev %s", iface);
	cndc("network create %u vpn 1 1", *netid);
	cndc("network interface add %u %s", *netid, iface);
}

static int compare_uid(const void *a, const void *b)
{
	return *(const uid_t *)a - *(const uid_t *)b;
}

static uid_t *get_uid_list(const char *selected_applications)
{
	_cleanup_fclose_ FILE *package_list = NULL;
	_cleanup_free_ char *line = NULL;
	uid_t package_uid;
	size_t line_len = 0, i;
	const char *comma, *start;
	uid_t *uid_list;

	if (!selected_applications)
		return xcalloc(1, sizeof(*uid_list));

	for (i = 1, comma = selected_applications; comma; comma = strchr(comma + 1, ','), ++i);
	uid_list = xcalloc(i, sizeof(*uid_list));
	i = 0;

	package_list = fopen("/data/system/packages.list", "r");
	if (!package_list) {
		perror("Error: Unable to open package list");
		exit(errno);
	}

	while (getline(&line, &line_len, package_list) >= 0) {
		char *package_name, *package_uid_str, *endptr;

		package_name = line;
		package_uid_str = strchr(package_name, ' ');
		if (!package_uid_str)
			continue;
		*package_uid_str++ = '\0';
		*strchrnul(package_uid_str, ' ') = '\0';
		package_uid = strtoul(package_uid_str, &endptr, 10);
		if (!package_uid || !*package_uid_str || *endptr)
			continue;

		for (start = selected_applications; comma = strchrnul(start, ','), *start; start = comma + 1) {
			ptrdiff_t token_len = comma - start;

			if (token_len && strlen(package_name) == token_len && !strncmp(start, package_name, token_len))
				uid_list[i++] = package_uid;
		}
	}
	qsort(uid_list, i, sizeof(*uid_list), compare_uid);
	return uid_list;
}

static void set_users(unsigned int netid, const char *excluded_applications, const char *included_applications)
{
	_cleanup_free_ uid_t *uids = NULL;
	unsigned int args_per_command = 0;
	_cleanup_free_ char *ranges = NULL;
	char range[22];
	uid_t start;

	if (excluded_applications && included_applications) {
		fprintf(stderr, "Error: only one of ExcludedApplications and IncludedApplications may be specified, but not both\n");
		exit(EEXIST);
	}

	if (excluded_applications || !included_applications) {
		uids = get_uid_list(excluded_applications);
		for (start = 0; *uids; start = *uids + 1, ++uids) {
			if (start > *uids - 1)
				continue;
			else if (start == *uids - 1)
				snprintf(range, sizeof(range), "%u", start);
			else
				snprintf(range, sizeof(range), "%u-%u", start, *uids - 1);
			ranges = concat_and_free(ranges, " ", range);
			if (++args_per_command % 18 == 0) {
				cndc("network users add %u %s", netid, ranges);
				free(ranges);
				ranges = NULL;
			}
		}
		if (start < 99999) {
			snprintf(range, sizeof(range), "%u-99999", start);
			ranges = concat_and_free(ranges, " ", range);
		}
	} else {
		for (uids = get_uid_list(included_applications); *uids; ++uids) {
			snprintf(range, sizeof(range), "%u", *uids);
			ranges = concat_and_free(ranges, " ", range);
			if (++args_per_command % 18 == 0) {
				cndc("network users add %u %s", netid, ranges);
				free(ranges);
				ranges = NULL;
			}
		}
	}

	if (ranges)
		cndc("network users add %u %s", netid, ranges);
}

static void set_dnses(unsigned int netid, const char *dnses)
{
	size_t len = strlen(dnses);
	if (len > (1<<16))
		return;
	_cleanup_free_ char *mutable = xstrdup(dnses);
	_cleanup_free_ char *dns_shell_arglist = xmalloc(len * 4 + 1);
	_cleanup_free_ char *dns_search_shell_arglist = xmalloc(len * 4 + 1);
	_cleanup_free_ char *dns_function_arglist = xmalloc(len * 4 + 1);
	_cleanup_free_ char *dns_search_function_arglist = xmalloc(len * 4 + 1);
	_cleanup_free_ char *arg = xmalloc(len + 4);
	_cleanup_free_ char **dns_list = NULL;
	_cleanup_free_ char **dns_search_list = NULL;
	_cleanup_binder_ AIBinder *handle = NULL;
	_cleanup_regfree_ regex_t regex_ipnothost = { 0 };
	size_t dns_list_size = 0, dns_search_list_size = 0;
	bool is_ip;

	if (!len)
		return;

	xregcomp(&regex_ipnothost, "^[a-zA-Z0-9_=+.-]{1,15}$", REG_EXTENDED | REG_NOSUB);
	for (char *dns = strtok(mutable, ", \t\n"); dns; dns = strtok(NULL, ", \t\n")) {
		if (strchr(dns, '\'') || strchr(dns, '\\'))
			continue;
		++*(!regexec(&regex_ipnothost, dns, 0, NULL, 0) ? &dns_list_size : &dns_search_list_size);
	}
	if (!dns_list_size)
		return;
	dns_list = xcalloc(dns_list_size + 1, sizeof(*dns_list));
	dns_search_list = xcalloc(dns_search_list_size + 1, sizeof(*dns_search_list));
	free(mutable);
	mutable = xstrdup(dnses);

	dns_shell_arglist[0] = '\0';
	dns_search_shell_arglist[0] = '\0';
	dns_function_arglist[0] = '\0';
	dns_search_function_arglist[0] = '\0';
	dns_list_size = 0;
	dns_search_list_size = 0;
	for (char *dns = strtok(mutable, ", \t\n"); dns; dns = strtok(NULL, ", \t\n")) {
		if (strchr(dns, '\'') || strchr(dns, '\\'))
			continue;
		is_ip = !regexec(&regex_ipnothost, dns, 0, NULL, 0);
		snprintf(arg, len + 3, "'%s' ", dns);
		strncat(is_ip ? dns_shell_arglist : dns_search_shell_arglist, arg, len * 4 - 1);
		snprintf(arg, len + 2, (is_ip ? dns_function_arglist[0] : dns_search_function_arglist[0]) == '\0' ? "%s" : ", %s", dns);
		strncat(is_ip ? dns_function_arglist : dns_search_function_arglist, arg, len * 4 - 1);
		*(is_ip ? &dns_list[dns_list_size++] : &dns_search_list[dns_search_list_size++]) = dns;
	}

	if ((handle = dnsresolver_get_handle())) {
		binder_status_t status;

		printf("[#] <binder>::dnsResolver->createNetworkCache(%u)\n", netid);
		status = dnsresolver_create_network_cache(handle, netid);
		if (status != 0) {
			fprintf(stderr, "Error: unable to create network cache\n");
			exit(ENONET);
		}

		struct dnsresolver_params params = {
			.netid = netid,
			.sample_validity_seconds = DNSRESOLVER_SAMPLE_VALIDITY,
			.success_threshold = DNSRESOLVER_SUCCESS_THRESHOLD,
			.min_samples = DNSRESOLVER_MIN_SAMPLES,
			.max_samples = DNSRESOLVER_MAX_SAMPLES,
			.base_timeout_msec = DNSRESOLVER_BASE_TIMEOUT,
			.retry_count = DNSRESOLVER_RETRY_COUNT,
			.servers = dns_list,
			.domains = dns_search_list,
			.tls_name = "",
			.tls_servers = (char *[]){NULL},
			.tls_fingerprints = (char *[]){NULL}
		};

		printf("[#] <binder>::dnsResolver->setResolverConfiguration(%u, [%s], [%s], %d, %d, %d, %d, %d, %d, [], [])\n",
		       netid, dns_function_arglist, dns_search_function_arglist, DNSRESOLVER_SAMPLE_VALIDITY,
		       DNSRESOLVER_SUCCESS_THRESHOLD, DNSRESOLVER_MIN_SAMPLES, DNSRESOLVER_MAX_SAMPLES,
		       DNSRESOLVER_BASE_TIMEOUT, DNSRESOLVER_RETRY_COUNT);
		status = dnsresolver_set_resolver_configuration(handle, &params);

		if (status != 0) {
			fprintf(stderr, "Error: unable to set DNS servers through Binder: %d\n", status);
			exit(ENONET);
		}
	} else
		cndc("resolver setnetdns %u '%s' %s", netid, dns_search_shell_arglist, dns_shell_arglist);
}

static void add_addr(const char *iface, const char *addr)
{
	if (strchr(addr, ':')) {
		cndc("interface ipv6 %s enable", iface);
		cmd("ip -6 addr add '%s' dev %s", addr, iface);
	} else {
		_cleanup_free_ char *mut_addr = strdup(addr);
		char *slash = strchr(mut_addr, '/');
		unsigned char mask = 32;

		if (slash) {
			*slash = '\0';
			mask = atoi(slash + 1);
		}
		cndc("interface setcfg %s '%s' %u", iface, mut_addr, mask);
	}
}

static void set_addr(const char *iface, const char *addrs)
{
	_cleanup_free_ char *mutable = xstrdup(addrs);

	for (char *addr = strtok(mutable, ", \t\n"); addr; addr = strtok(NULL, ", \t\n")) {
		if (strchr(addr, '\'') || strchr(addr, '\\'))
			continue;
		add_addr(iface, addr);
	}
}

static int get_route_mtu(const char *endpoint)
{
	DEFINE_CMD(c_route);
	DEFINE_CMD(c_dev);
	regmatch_t matches[2];
	_cleanup_regfree_ regex_t regex_mtu = { 0 }, regex_dev = { 0 };
	char *route, *mtu, *dev;

	xregcomp(&regex_mtu, "mtu ([0-9]+)", REG_EXTENDED);
	xregcomp(&regex_dev, "dev ([^ ]+)", REG_EXTENDED);

	if (strcmp(endpoint, "default"))
		route = cmd_ret(&c_route, "ip -o route get %s", endpoint);
	else
		route = cmd_ret(&c_route, "ip -o route show %s", endpoint);
	if (!route)
		return -1;

	if (!regexec(&regex_mtu, route, ARRAY_SIZE(matches), matches, 0)) {
		route[matches[1].rm_eo] = '\0';
		mtu = &route[matches[1].rm_so];
	} else if (!regexec(&regex_dev, route, ARRAY_SIZE(matches), matches, 0)) {
		route[matches[1].rm_eo] = '\0';
		dev = &route[matches[1].rm_so];
		route = cmd_ret(&c_dev, "ip -o link show dev %s", dev);
		if (!route)
			return -1;
		if (regexec(&regex_mtu, route, ARRAY_SIZE(matches), matches, 0))
			return -1;
		route[matches[1].rm_eo] = '\0';
		mtu = &route[matches[1].rm_so];
	} else
		return -1;
	return atoi(mtu);
}

static void set_mtu(const char *iface, unsigned int mtu)
{
	DEFINE_CMD(c_endpoints);
	_cleanup_regfree_ regex_t regex_endpoint = { 0 };
	regmatch_t matches[2];
	int endpoint_mtu, next_mtu;

	if (mtu) {
		cndc("interface setmtu %s %u", iface, mtu);
		return;
	}

	xregcomp(&regex_endpoint, "^\\[?([a-z0-9:.]+)\\]?:[0-9]+$", REG_EXTENDED);

	endpoint_mtu = get_route_mtu("default");
	if (endpoint_mtu == -1)
		endpoint_mtu = 1500;

	for (char *endpoint = cmd_ret(&c_endpoints, "wg show %s endpoints", iface); endpoint; endpoint = cmd_ret(&c_endpoints, NULL)) {
		if (regexec(&regex_endpoint, endpoint, ARRAY_SIZE(matches), matches, 0))
			continue;
		endpoint[matches[1].rm_eo] = '\0';
		endpoint = &endpoint[matches[1].rm_so];

		next_mtu = get_route_mtu(endpoint);
		if (next_mtu > 0 && next_mtu < endpoint_mtu)
			endpoint_mtu = next_mtu;
	}

	cndc("interface setmtu %s %d", iface, endpoint_mtu - 80);
}

static void add_route(const char *iface, unsigned int netid, const char *route)
{
	cndc("network route add %u %s %s", netid, iface, route);
}

static void set_routes(const char *iface, unsigned int netid)
{
	DEFINE_CMD(c);

	for (char *allowedips = cmd_ret(&c, "wg show %s allowed-ips", iface); allowedips; allowedips = cmd_ret(&c, NULL)) {
		char *start = strchr(allowedips, '\t');

		if (!start)
			continue;
		++start;
		for (char *allowedip = strtok(start, " \n"); allowedip; allowedip = strtok(NULL, " \n")) {
			if (!strcmp(allowedip, "(none)"))
				continue;
			add_route(iface, netid, allowedip);
		}
	}
}

static void set_config(const char *iface, const char *config)
{
	FILE *config_writer;
	_cleanup_free_ char *cmd = concat("wg setconf ", iface, " /proc/self/fd/0", NULL);
	int ret;

	printf("[#] %s\n", cmd);

	config_writer = popen(cmd, "w");
	if (!config_writer) {
		perror("Error: popen");
		exit(errno);
	}
	if (fputs(config, config_writer) < 0) {
		perror("Error: fputs");
		exit(errno);
	}
	ret = pclose(config_writer);
	if (ret)
		exit(WIFEXITED(ret) ? WEXITSTATUS(ret) : EIO);
}

static void broadcast_change(void)
{
	const char *pkg = getenv("CALLING_PACKAGE");

	if (!pkg || strcmp(pkg, WG_PACKAGE_NAME))
		cmd("am broadcast -a com.wireguard.android.action.REFRESH_TUNNEL_STATES " WG_PACKAGE_NAME);
}

static void print_search_paths(FILE *file, const char *prefix)
{
	_cleanup_free_ char *paths = strdup(WG_CONFIG_SEARCH_PATHS);

	for (char *path = strtok(paths, " "); path; path = strtok(NULL, " "))
		fprintf(file, "%s%s\n", prefix, path);
}

static void cmd_usage(const char *program)
{
	printf( "Usage: %s [ up | down ] [ CONFIG_FILE | INTERFACE ]\n"
		"\n"
		"  CONFIG_FILE is a configuration file, whose filename is the interface name\n"
		"  followed by `.conf'. Otherwise, INTERFACE is an interface name, with\n"
		"  configuration found at:\n\n", program);
	print_search_paths(stdout, "  - ");
	printf( "\n  It is to be readable by wg(8)'s `setconf' sub-command, with the exception\n"
		"  of the following additions to the [Interface] section, which are handled by\n"
		"  this program:\n\n"
		"  - Address: may be specified one or more times and contains one or more\n"
		"    IP addresses (with an optional CIDR mask) to be set for the interface.\n"
		"  - MTU: an optional MTU for the interface; if unspecified, auto-calculated.\n"
		"  - DNS: an optional DNS server to use while the device is up.\n"
		"  - ExcludedApplications: optional blacklist of applications to exclude from the tunnel.\n\n"
		"  - IncludedApplications: optional whitelist of applications to include in the tunnel.\n\n"
		"  See wg-quick(8) for more info and examples.\n");
}

static char *cleanup_iface = NULL;

static void cmd_up_cleanup(void)
{
	is_exiting = true;
	if (cleanup_iface)
		del_if(cleanup_iface);
	free(cleanup_iface);
}

static void cmd_up(const char *iface, const char *config, unsigned int mtu, const char *addrs, const char *dnses, const char *excluded_applications, const char *included_applications)
{
	DEFINE_CMD(c);
	unsigned int netid = 0;
	uint16_t listen_port;

	if (cmd_ret(&c, "ip link show dev %s 2>/dev/null", iface)) {
		fprintf(stderr, "Error: %s already exists\n", iface);
		exit(EEXIST);
	}

	cleanup_iface = xstrdup(iface);
	atexit(cmd_up_cleanup);

	add_if(iface);
	set_config(iface, config);
	listen_port = determine_listen_port(iface);
	up_if(&netid, iface, listen_port);
	set_addr(iface, addrs);
	set_dnses(netid, dnses);
	set_routes(iface, netid);
	set_mtu(iface, mtu);
	set_users(netid, excluded_applications, included_applications);
	broadcast_change();

	free(cleanup_iface);
	cleanup_iface = NULL;
	exit(EXIT_SUCCESS);
}

static void cmd_down(const char *iface)
{
	DEFINE_CMD(c);
	bool found = false;

	char *ifaces = cmd_ret(&c, "wg show interfaces");
	if (ifaces) {
		for (char *eiface = strtok(ifaces, " \n"); eiface; eiface = strtok(NULL, " \n")) {
			if (!strcmp(iface, eiface)) {
				found = true;
				break;
			}
		}
	}
	if (!found) {
		fprintf(stderr, "Error: %s is not a WireGuard interface\n", iface);
		exit(EMEDIUMTYPE);
	}

	del_if(iface);
	broadcast_change();
	exit(EXIT_SUCCESS);
}

static void parse_options(char **iface, char **config, unsigned int *mtu, char **addrs, char **dnses, char **excluded_applications, char **included_applications, const char *arg)
{
	_cleanup_fclose_ FILE *file = NULL;
	_cleanup_free_ char *line = NULL;
	_cleanup_free_ char *filename = NULL;
	_cleanup_free_ char *paths = strdup(WG_CONFIG_SEARCH_PATHS);
	_cleanup_regfree_ regex_t regex_iface = { 0 }, regex_conf = { 0 };
	regmatch_t matches[2];
	struct stat sbuf;
	size_t n = 0;
	bool in_interface_section = false;

	*iface = *config = *addrs = *dnses = NULL;
	*mtu = 0;

	xregcomp(&regex_iface, "^[a-zA-Z0-9_=+.-]{1,15}$", REG_EXTENDED | REG_NOSUB);
	xregcomp(&regex_conf, "/?([a-zA-Z0-9_=+.-]{1,15})\\.conf$", REG_EXTENDED);

	if (!regexec(&regex_iface, arg, 0, NULL, 0)) {
		for (char *path = strtok(paths, " "); path; path = strtok(NULL, " ")) {
			free(filename);
			if (asprintf(&filename, "%s/%s.conf", path, arg) < 0) {
				perror("Error: asprintf");
				exit(errno);
			}
			file = fopen(filename, "r");
			if (file)
				break;
		}
		if (!file) {
			fprintf(stderr, "Error: Unable to find configuration file for `%s' in:\n", arg);
			print_search_paths(stderr, "- ");
			exit(errno);
		}
	} else {
		filename = xstrdup(arg);
		file = fopen(filename, "r");
		if (!file) {
			fprintf(stderr, "Error: Unable to find configuration file at `%s'\n", filename);
			exit(errno);
		}
	}

	if (regexec(&regex_conf, filename, ARRAY_SIZE(matches), matches, 0)) {
		fprintf(stderr, "Error: The config file must be a valid interface name, followed by .conf\n");
		exit(EINVAL);
	}

	if (fstat(fileno(file), &sbuf) < 0) {
		perror("Error: fstat");
		exit(errno);
	}
	if (sbuf.st_mode & 0007)
		fprintf(stderr, "Warning: `%s' is world accessible\n", filename);

	filename[matches[1].rm_eo] = '\0';
	*iface = xstrdup(&filename[matches[1].rm_so]);

	while (getline(&line, &n, file) >= 0) {
		size_t len = strlen(line), j = 0;
		if (len > (1<<16))
			return;
		_cleanup_free_ char *clean = xmalloc(len + 1);

		for (size_t i = 0; i < len; ++i) {
			if (!isspace(line[i]))
				clean[j++] = line[i];
		}
		clean[j] = '\0';

		if (clean[0] == '[')
			in_interface_section = false;
		if (!strcasecmp(clean, "[Interface]"))
			in_interface_section = true;
		if (in_interface_section) {
			if (!strncasecmp(clean, "Address=", 8) && j > 8) {
				*addrs = concat_and_free(*addrs, ",", clean + 8);
				continue;
			} else if (!strncasecmp(clean, "DNS=", 4) && j > 4) {
				*dnses = concat_and_free(*dnses, ",", clean + 4);
				continue;
			} else if (!strncasecmp(clean, "ExcludedApplications=", 21) && j > 4) {
				*excluded_applications = concat_and_free(*excluded_applications, ",", clean + 21);
				continue;
			} else if (!strncasecmp(clean, "IncludedApplications=", 21) && j > 4) {
				*included_applications = concat_and_free(*included_applications, ",", clean + 21);
				continue;
			} else if (!strncasecmp(clean, "MTU=", 4) && j > 4) {
				*mtu = atoi(clean + 4);
				continue;
			}
		}
		*config = concat_and_free(*config, "", line);
	}

	if (!*iface)
		*iface = xstrdup("");
	if (!*config)
		*config = xstrdup("");
	if (!*addrs)
		*addrs = xstrdup("");
	if (!*dnses)
		*dnses = xstrdup("");
}

int main(int argc, char *argv[])
{
	_cleanup_free_ char *iface = NULL;
	_cleanup_free_ char *config = NULL;
	_cleanup_free_ char *addrs = NULL;
	_cleanup_free_ char *dnses = NULL;
	_cleanup_free_ char *excluded_applications = NULL;
	_cleanup_free_ char *included_applications = NULL;
	unsigned int mtu;

	if (argc == 2 && (!strcmp(argv[1], "help") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")))
		cmd_usage(argv[0]);
	else if (argc == 3 && !strcmp(argv[1], "up")) {
		auto_su(argc, argv);
		parse_options(&iface, &config, &mtu, &addrs, &dnses, &excluded_applications, &included_applications, argv[2]);
		cmd_up(iface, config, mtu, addrs, dnses, excluded_applications, included_applications);
	} else if (argc == 3 && !strcmp(argv[1], "down")) {
		auto_su(argc, argv);
		parse_options(&iface, &config, &mtu, &addrs, &dnses, &excluded_applications, &included_applications, argv[2]);
		cmd_down(iface);
	} else {
		cmd_usage(argv[0]);
		return 1;
	}
	return 0;
}
