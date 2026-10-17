/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ifndef _SEMIHOSTING_H
#define _SEMIHOSTING_H

/*
 * These are the encoded instructions used to indicate a semihosting trap. They
 * are named like SMH_ISA_INSN, where ISA is the instruction set (e.g.
 * AArch64), and INSN is the mneumonic for the instruction.
 */
#define SMH_A64_HLT 0xD45E0000
#define SMH_A32_SVC 0xEF123456
#define SMH_A32_HLT 0xE10F0070
#define SMH_T32_SVC 0xDFAB
#define SMH_T32_HLT 0xBABC

/**
 * smh_trap() - ARCH-specific semihosting call.
 *
 * Semihosting library/driver can use this function to do the
 * actual semihosting calls.
 *
 * Return: Error code defined by semihosting spec.
 */

long smh_trap(unsigned int sysnum, void *addr);

#if CONFIG_IS_ENABLED(SEMIHOSTING_FALLBACK)
/**
 * semihosting_enabled() - Determine whether semihosting is supported
 *
 * Semihosting-based drivers should call this function before making other
 * semihosting calls.
 *
 * Return: %true if a debugger is attached which supports semihosting, %false
 *         otherwise
 */
bool semihosting_enabled(void);

/**
 * disable_semihosting() - Cause semihosting_enabled() to return false
 *
 * If U-Boot ever receives an unhandled exception caused by a semihosting trap,
 * the trap handler should call this function.
 */
void disable_semihosting(void);
#else
static inline bool semihosting_enabled(void)
{
	return CONFIG_IS_ENABLED(SEMIHOSTING);
}

static inline void disable_semihosting(void)
{
}
#endif

/**
 * enum smh_open_mode - Numeric file modes for use with smh_open()
 * MODE_READ: 'r'
 * MODE_BINARY: 'b'
 * MODE_PLUS: '+'
 * MODE_WRITE: 'w'
 * MODE_APPEND: 'a'
 *
 * These modes represent the mode string used by fopen(3) in a form which can
 * be passed to smh_open(). These do NOT correspond directly to %O_RDONLY,
 * %O_CREAT, etc; see fopen(3) for details. In particular, @MODE_PLUS
 * effectively results in adding %O_RDWR, and @MODE_WRITE will add %O_TRUNC.
 * For compatibility, @MODE_BINARY should be added when opening non-text files
 * (such as images).
 */
enum smh_open_mode {
	MODE_READ	= 0x0,
	MODE_BINARY	= 0x1,
	MODE_PLUS	= 0x2,
	MODE_WRITE	= 0x4,
	MODE_APPEND	= 0x8,
};

/**
 * smh_open() - Open a file on the host
 * @fname: The name of the file to open
 * @mode: The mode to use when opening the file
 *
 * Return: Either a file descriptor or a negative error on failure
 */
long smh_open(const char *fname, enum smh_open_mode mode);

/**
 * smh_read() - Read data from a file
 * @fd: A file descriptor returned from smh_open()
 * @memp: Pointer to a buffer of memory of at least @len bytes
 * @len: The number of bytes to read
 *
 * Return:
 * * The number of bytes read on success, with 0 indicating %EOF
 * * A negative error on failure
 */
long smh_read(long fd, void *memp, size_t len);

/**
 * smh_write() - Write data to a file
 * @fd: A file descriptor returned from smh_open()
 * @memp: Pointer to a buffer of memory of at least @len bytes
 * @len: The number of bytes to read
 * @written: Pointer which will be updated with the actual bytes written
 *
 * Return: 0 on success or negative error on failure
 */
long smh_write(long fd, const void *memp, size_t len, ulong *written);

/**
 * smh_close() - Close an open file
 * @fd: A file descriptor returned from smh_open()
 *
 * Return: 0 on success or negative error on failure
 */
long smh_close(long fd);

/**
 * smh_flen() - Get the length of a file
 * @fd: A file descriptor returned from smh_open()
 *
 * Return: The length of the file, in bytes, or a negative error on failure
 */
long smh_flen(long fd);

/**
 * smh_seek() - Seek to a position in a file
 * @fd: A file descriptor returned from smh_open()
 * @pos: The offset (in bytes) to seek to
 *
 * Return: 0 on success or negative error on failure
 */
long smh_seek(long fd, long pos);

/**
 * smh_getc() - Read a character from stdin
 *
 * Return: The character read, or a negative error on failure
 */
int smh_getc(void);

/**
 * smh_putc() - Print a character on stdout
 * @ch: The character to print
 */
void smh_putc(char ch);

/**
 * smh_write0() - Print a nul-terminated string on stdout
 * @s: The string to print
 */
void smh_puts(const char *s);

#endif /* _SEMIHOSTING_H */
