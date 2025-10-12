/*
 * fscryptctl.c - Low level tool for managing keys and policies for the
 * fs/crypto kernel interface. Specifically, this tool:
 *     - Computes the descriptor for a provided key
 *     - Inserts a provided key into the keyring
 *     - Queries the key descriptor for an encrypted directory
 *     - Applies an encryption policy to an empty directory
 *
 * Copyright 2017 Google Inc.
 * Author: Joe Richey (joerichey@google.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "sha512.h"

// Some of the necessary structures, constants, and functions are declared in
// <linux/fs.h> or <keyutils.h> but are either incomplete (depending on your
// kernel version) or require an external library. So to simplify things, they
// are just redeclared here.

/* Begin <linux/fs.h> */
#define FS_MAX_KEY_SIZE 64

struct fscrypt_key {
  uint32_t mode;
  uint8_t raw[FS_MAX_KEY_SIZE];
  uint32_t size;
} __attribute__((packed));

#define FS_KEY_DESCRIPTOR_SIZE 8
#define FS_KEY_DESCRIPTOR_HEX_SIZE ((2 * FS_KEY_DESCRIPTOR_SIZE) + 1)

// Amount of padding
#define FS_POLICY_FLAGS_PAD_4 0x00
#define FS_POLICY_FLAGS_PAD_8 0x01
#define FS_POLICY_FLAGS_PAD_16 0x02
#define FS_POLICY_FLAGS_PAD_32 0x03
#define FS_POLICY_FLAGS_PAD_MASK 0x03

// Encryption algorithms
#define FS_ENCRYPTION_MODE_INVALID 0
#define FS_ENCRYPTION_MODE_AES_256_XTS 1
#define FS_ENCRYPTION_MODE_AES_256_GCM 2
#define FS_ENCRYPTION_MODE_AES_256_CBC 3
#define FS_ENCRYPTION_MODE_AES_256_CTS 4
#define FS_ENCRYPTION_MODE_AES_128_CBC 5
#define FS_ENCRYPTION_MODE_AES_128_CTS 6

// Policy provided via an ioctl on the topmost directory
struct fscrypt_policy {
  uint8_t version;
  uint8_t contents_encryption_mode;
  uint8_t filenames_encryption_mode;
  uint8_t flags;
  uint8_t master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
} __attribute__((packed));

#define FS_IOC_SET_ENCRYPTION_POLICY _IOR('f', 19, struct fscrypt_policy)
#define FS_IOC_GET_ENCRYPTION_POLICY _IOW('f', 21, struct fscrypt_policy)

// Service prefixes for encryption keys
#define FS_KEY_DESC_PREFIX "fscrypt:"
#define EXT4_KEY_DESC_PREFIX "ext4:"  // For ext4 before 4.8 kernel
#define F2FS_KEY_DESC_PREFIX "f2fs:"  // For f2fs before 4.6 kernel
#define MAX_KEY_DESC_PREFIX_SIZE 8
/* End <linux/fs.h> */

/* Begin <keyutils.h> */
typedef int32_t key_serial_t;
#define KEYCTL_GET_KEYRING_ID 0     /* ask for a keyring's ID */
#define KEY_SPEC_SESSION_KEYRING -3 /* current session keyring */

key_serial_t add_key(const char *type, const char *description,
                     const void *payload, size_t plen, key_serial_t ringid) {
  return syscall(__NR_add_key, type, description, payload, plen, ringid);
}

key_serial_t keyctl_get_keyring_ID(key_serial_t id, int create) {
  return syscall(__NR_keyctl, KEYCTL_GET_KEYRING_ID, id, create);
}
/* End <keyutils.h> */

// Human-readable strings for encryption modes, indexed by the encryption mode
#define NUM_ENCRYPTION_MODES 7
const char *const mode_strings[NUM_ENCRYPTION_MODES] = {
    "INVALID",     "AES-256-XTS", "AES-256-GCM", "AES-256-CBC",
    "AES-256-CTS", "AES-128-CBC", "AES-128-CTS"};

// Valid amounts of filename padding, indexed by the padding flag
#define NUM_PADDING_VALUES 4
const int padding_values[NUM_PADDING_VALUES] = {4, 8, 16, 32};

/* util-linux style usage */
static void __attribute__((__noreturn__)) usage(FILE *out) {
  fputs(
      "\nUsage:\n"
      "  fscryptctl <command> [arguments] [options]\n"
      "\nCommands:\n"
      "  fscryptctl get_descriptor\n"
      "    Read a key from stdin, and print the hex descriptor of the key.\n"
      "  fscryptctl insert_key\n"
      "    Read a key from stdin, insert the key into the current session\n"
      "    keyring (or the user session keyring if a session keyring does not\n"
      "    exist), and print the descriptor of the key.\n"
      "  fscryptctl get_policy <file or directory>\n"
      "    Print out the encryption policy for the specified path.\n"
      "  fscryptctl set_policy <key descriptor> <directory>\n"
      "    Set up an encryption policy on the specified directory with the\n"
      "    specified key descriptor.\n"
      "\nOptions:\n"
      "    -h, --help\n"
      "        print this help screen\n"
      "    -v, --version\n"
      "        print the version of fscrypt\n"
      "    insert_key\n"
      "        --ext4\n"
      "            for use with an ext4 filesystem before kernel v4.8\n"
      "        --f2fs\n"
      "            for use with an F2FS filesystem before kernel v4.6\n"
      "    set_policy\n"
      "        --contents=<mode>\n"
      "            contents encryption mode (default: AES-256-XTS)\n"
      "        --filenames=<mode>\n"
      "            filenames encryption mode (default: AES-256-CTS)\n"
      "        --padding=<bytes>\n"
      "            bytes of zero padding for filenames (default: 32)\n"
      "\nNotes:\n"
      "  All input keys are 64 bytes long and formatted as binary.\n"
      "  All descriptors are 8 bytes and formatted as hex (16 characters).\n",
      out);

  exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

// For getting/setting policies, our error messages might differ from the
// standard ones for certain errno values.
const char *policy_error(int errno_val) {
  // Only these errno values actually relate to filesystem encryption
  switch (errno_val) {
    case ENOTTY:
      return "your kernel is too old to support filesystem encryption, or the "
             "filesystem you are using does not support encryption";
    case EOPNOTSUPP:
      return "filesystem encryption has been disabled in the kernel config, or "
             "you need to enable encryption on your filesystem (see the README "
             "for more detailed instructions).";
    case ENODATA:
      return "file or directory not encrypted";
    case EEXIST:
      // EINVAL was returned instead of EEXIST on some filesystems before v4.11.
      // However, we do not attempt to work around this.
      return "file or directory already encrypted";
    case EINVAL:
      return "invalid encryption options provided";
    default:
      return strerror(errno_val);
  }
}

// Converts str to an encryption mode. Returns 0 (FS_ENCRYPTION_MODE_INVALID) if
// the string does not correspond to an encryption mode.
static uint8_t string_to_mode(const char *str) {
  uint8_t i;
  for (i = 1; i < NUM_ENCRYPTION_MODES; ++i) {
    if (strcmp(str, mode_strings[i]) == 0) {
      return i;
    }
  }
  return 0;
}

// Converts the encryption mode to a human-readable string. Returns "INVALID" if
// the mode is not a valid encryption mode.
static const char *mode_to_string(uint8_t mode) {
  if (mode >= NUM_ENCRYPTION_MODES) {
    mode = 0;
  }
  return mode_strings[mode];
}

// Converts an amount of padding (as a string) into the appropriate padding
// flag. Returns -1 if the flag is invalid.
static int string_to_padding_flag(const char *str) {
  int i, padding = atoi(str);
  for (i = 0; i < NUM_PADDING_VALUES; ++i) {
    if (padding == padding_values[i]) {
      return i;
    }
  }
  return -1;
}

// Takes an input key descriptor as a byte array and outputs a hex string.
static void key_descriptor_to_hex(const uint8_t bytes[FS_KEY_DESCRIPTOR_SIZE],
                                  char hex[FS_KEY_DESCRIPTOR_HEX_SIZE]) {
  int i;
  for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; ++i) {
    sprintf(hex + 2 * i, "%02x", bytes[i]);
  }
}

// Takes an input key descriptor as a hex string and outputs a bytes array.
// Returns non-zero if the provided hex string is not formatted correctly.
static int key_descriptor_to_bytes(const char *hex,
                                   uint8_t bytes[FS_KEY_DESCRIPTOR_SIZE]) {
  if (strlen(hex) != FS_KEY_DESCRIPTOR_HEX_SIZE - 1) {
    return -1;
  }

  int i, bytes_converted, chars_read;
  for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; ++i) {
    // We must read two hex characters of input into one byte of buffer.
    bytes_converted = sscanf(hex + 2 * i, "%2hhx%n", bytes + i, &chars_read);
    if (bytes_converted != 1 || chars_read != 2) {
      return -1;
    }
  }
  return 0;
}

// Reads key data from stdin into the provided data buffer. Return 0 on success.
static int read_key(uint8_t key[FS_MAX_KEY_SIZE]) {
  size_t rc = fread(key, 1, FS_MAX_KEY_SIZE, stdin);
  int end = fgetc(stdin);
  // We should read exactly FS_MAX_KEY_SIZE bytes, then hit EOF
  if (rc == FS_MAX_KEY_SIZE && end == EOF && feof(stdin)) {
    return 0;
  }

  fprintf(stderr, "error: input key must be %d bytes\n", FS_MAX_KEY_SIZE);
  return -1;
}

// The descriptor is just the first 8 bytes of a double application of SHA512
// formatted as hex (so 16 characters).
static void compute_descriptor(const uint8_t key[FS_MAX_KEY_SIZE],
                               char descriptor[FS_KEY_DESCRIPTOR_HEX_SIZE]) {
  uint8_t digest1[SHA512_DIGEST_LENGTH];
  SHA512(key, FS_MAX_KEY_SIZE, digest1);

  uint8_t digest2[SHA512_DIGEST_LENGTH];
  SHA512(digest1, SHA512_DIGEST_LENGTH, digest2);

  key_descriptor_to_hex(digest2, descriptor);
  secure_wipe(digest1, SHA512_DIGEST_LENGTH);
  secure_wipe(digest2, SHA512_DIGEST_LENGTH);
}

// Inserts the key into the current session keyring with type logon and the
// service specified by service_prefix.
static int insert_logon_key(const uint8_t key_data[FS_MAX_KEY_SIZE],
                            const char descriptor[FS_KEY_DESCRIPTOR_HEX_SIZE],
                            const char *service_prefix) {
  // We cannot add directly to KEY_SPEC_SESSION_KEYRING, as that will make a new
  // session keyring if one does not exist, rather than adding it to the user
  // session keyring.
  int keyring_id = keyctl_get_keyring_ID(KEY_SPEC_SESSION_KEYRING, 0);
  if (keyring_id < 0) {
    return -1;
  }

  char description[MAX_KEY_DESC_PREFIX_SIZE + FS_KEY_DESCRIPTOR_HEX_SIZE];
  sprintf(description, "%s%s", service_prefix, descriptor);

  struct fscrypt_key key = {.mode = 0, .size = FS_MAX_KEY_SIZE};
  memcpy(key.raw, key_data, FS_MAX_KEY_SIZE);

  int ret =
      add_key("logon", description, &key, sizeof(key), keyring_id) < 0 ? -1 : 0;

  secure_wipe(key.raw, FS_MAX_KEY_SIZE);
  return ret;
}

static int get_policy(const char *path, struct fscrypt_policy *policy) {
  // We can query the policy for a directory or a file in that directory.
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return -1;
  }

  int ret = ioctl(fd, FS_IOC_GET_ENCRYPTION_POLICY, policy);
  close(fd);

  if (ret < 0) {
    // Kernels prior to v4.11 returned ENOENT if the file did not have an
    // encryption policy, newer kernels properly return ENODATA. This lets us
    // print the right error in policy_error regardless of kernel version.
    if (errno == ENOENT) {
      errno = ENODATA;
    }
  }

  return ret;
}

static int set_policy(const char *path, const struct fscrypt_policy *policy) {
  // Policies can only be set on directories
  int fd = open(path, O_RDONLY | O_DIRECTORY);
  if (fd < 0) {
    return -1;
  }

  int ret = ioctl(fd, FS_IOC_SET_ENCRYPTION_POLICY, policy);
  close(fd);

  return ret;
}

/* Functions for various actions, return 0 on success, non-zero on failure. */

// Get the descriptor for some key data passed via stdin. Provided key data must
// have length FS_MAX_KEY_SIZE. Output will be formatted as hex.
static int cmd_get_descriptor(int argc, char *const argv[]) {
  if (argc != 2) {
    fputs("error: unexpected arguments\n", stderr);
    return EXIT_FAILURE;
  }

  int ret = EXIT_SUCCESS;
  uint8_t key[FS_MAX_KEY_SIZE];

  if (read_key(key)) {
    ret = EXIT_FAILURE;
    goto cleanup;
  }

  char descriptor[FS_KEY_DESCRIPTOR_HEX_SIZE];
  compute_descriptor(key, descriptor);
  puts(descriptor);

cleanup:
  secure_wipe(key, FS_MAX_KEY_SIZE);
  return ret;
}

// Insert a key read from stdin into the current session keyring. This has the
// effect of unlocking files encrypted with that key.
static int cmd_insert_key(int argc, char *const argv[]) {
  // Which prefix will be used in this program, changed via command line flag.
  const char *service_prefix = FS_KEY_DESC_PREFIX;

  static const struct option insert_key_options[] = {
      {"ext4", no_argument, NULL, 'e'},
      {"f2fs", no_argument, NULL, 'f'},
      {NULL, 0, NULL, 0}};

  int ch;
  while ((ch = getopt_long(argc, argv, "", insert_key_options, NULL)) != -1) {
    switch (ch) {
      case 'e':
        service_prefix = EXT4_KEY_DESC_PREFIX;
        break;
      case 'f':
        service_prefix = F2FS_KEY_DESC_PREFIX;
        break;
      default:
        usage(stderr);
    }
  }

  // This command does not need additional arguments
  if (argc != optind + 1) {
    fputs("error: unexpected arguments\n", stderr);
    return EXIT_FAILURE;
  }

  int ret = EXIT_SUCCESS;
  uint8_t key[FS_MAX_KEY_SIZE];
  if (read_key(key)) {
    ret = EXIT_FAILURE;
    goto cleanup;
  }

  char descriptor[FS_KEY_DESCRIPTOR_HEX_SIZE];
  compute_descriptor(key, descriptor);
  if (insert_logon_key(key, descriptor, service_prefix)) {
    fprintf(stderr, "error: inserting key: %s\n", strerror(errno));
    ret = EXIT_FAILURE;
    goto cleanup;
  }
  puts(descriptor);

cleanup:
  secure_wipe(key, FS_MAX_KEY_SIZE);
  return ret;
}

// For a specified file or directory with encryption enabled, print the
// corresponding policy to stdout. Key descriptor will be formatted as hex.
static int cmd_get_policy(int argc, char *const argv[]) {
  if (argc != 3) {
    fputs("error: must specify a single file or directory\n", stderr);
    return EXIT_FAILURE;
  }
  const char *path = argv[2];

  struct fscrypt_policy policy;
  if (get_policy(path, &policy)) {
    fprintf(stderr, "error: getting policy for %s: %s\n", path,
            policy_error(errno));
    return EXIT_FAILURE;
  }

  // Pretty print the policy (includes key descriptor and flags)
  char descriptor[FS_KEY_DESCRIPTOR_HEX_SIZE];
  key_descriptor_to_hex(policy.master_key_descriptor, descriptor);
  int padding = padding_values[policy.flags & FS_POLICY_FLAGS_PAD_MASK];

  printf("Encryption policy for %s:\n", path);
  printf("\tPolicy Version: %d\n", policy.version);
  printf("\tKey Descriptor: %s\n", descriptor);
  printf("\tContents: %s\n", mode_to_string(policy.contents_encryption_mode));
  printf("\tFilenames: %s\n", mode_to_string(policy.filenames_encryption_mode));
  printf("\tPadding: %d\n", padding);

  return EXIT_SUCCESS;
}

// Apply a policy (i.e. the specified descriptor) to the specified directory.
// The policy options defaults can be overridden by command-line options.
static int cmd_set_policy(int argc, char *const argv[]) {
  // As Kernel version 4.9, the only policy field that has multiple valid
  // options is "flags", which sets the amount of zero padding on filenames.
  struct fscrypt_policy policy = {
      .version = 0,
      .contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS,
      .filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS,
      // Use maximum zero-padding to leak less info about filename length
      .flags = FS_POLICY_FLAGS_PAD_32};

  // Use the command-line options to modify the policy
  static const struct option insert_key_options[] = {
      {"contents", required_argument, NULL, 'c'},
      {"filenames", required_argument, NULL, 'f'},
      {"padding", required_argument, NULL, 'p'},
      {NULL, 0, NULL, 0}};

  int ch, padding_flag;
  while ((ch = getopt_long(argc, argv, "", insert_key_options, NULL)) != -1) {
    switch (ch) {
      case 'c':
        policy.contents_encryption_mode = string_to_mode(optarg);
        if (policy.contents_encryption_mode == FS_ENCRYPTION_MODE_INVALID) {
          fprintf(stderr, "error: invalid contents mode: %s\n", optarg);
          return EXIT_FAILURE;
        }
        break;
      case 'f':
        policy.filenames_encryption_mode = string_to_mode(optarg);
        if (policy.filenames_encryption_mode == FS_ENCRYPTION_MODE_INVALID) {
          fprintf(stderr, "error: invalid filenames mode: %s\n", optarg);
          return EXIT_FAILURE;
        }
        break;
      case 'p':
        padding_flag = string_to_padding_flag(optarg);
        if (padding_flag < 0) {
          fprintf(stderr, "error: invalid padding: %s\n", optarg);
          return EXIT_FAILURE;
        }
        policy.flags = padding_flag;
        break;
      default:
        usage(stderr);
    }
  }

  // We should have exactly 2 more arguments
  if (argc != optind + 3) {
    fputs("error: must specify a key descriptor and directory\n", stderr);
    return EXIT_FAILURE;
  }
  const char *descriptor = argv[optind + 1];
  const char *path = argv[optind + 2];

  // Copy the descriptor into the policy, requires changing format.
  if (key_descriptor_to_bytes(descriptor, policy.master_key_descriptor)) {
    fprintf(stderr, "error: invalid descriptor: %s\n", descriptor);
    return EXIT_FAILURE;
  }

  if (set_policy(path, &policy)) {
    fprintf(stderr, "error: setting policy for %s: %s\n", path,
            policy_error(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int main(int argc, char *const argv[]) {
  // Check for the help flag
  int i;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      usage(stdout);
    }
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      puts(VERSION);
      return EXIT_SUCCESS;
    }
  }

  if (argc < 2) {
    fputs("error: no command specified\n", stderr);
    usage(stderr);
  }
  const char *command = argv[1];

  if (strcmp(command, "get_descriptor") == 0) {
    return cmd_get_descriptor(argc, argv);
  } else if (strcmp(command, "insert_key") == 0) {
    return cmd_insert_key(argc, argv);
  } else if (strcmp(command, "get_policy") == 0) {
    return cmd_get_policy(argc, argv);
  } else if (strcmp(command, "set_policy") == 0) {
    return cmd_set_policy(argc, argv);
  }

  fprintf(stderr, "error: invalid command: %s\n", command);
  usage(stderr);
}
