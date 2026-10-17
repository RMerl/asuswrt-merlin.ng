/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Fetch and Add Unit.
 */

/**
 * @file
 *
 * Interface to the hardware Fetch and Add Unit.
 *
 */

#ifndef __CVMX_HWFAU_H__
#define __CVMX_HWFAU_H__

typedef int cvmx_fau_reg64_t;
typedef int cvmx_fau_reg32_t;
typedef int cvmx_fau_reg16_t;
typedef int cvmx_fau_reg8_t;

#define CVMX_FAU_REG_ANY -1

/*
 * Octeon Fetch and Add Unit (FAU)
 */

#define CVMX_FAU_LOAD_IO_ADDRESS cvmx_build_io_address(0x1e, 0)
#define CVMX_FAU_BITS_SCRADDR	 63, 56
#define CVMX_FAU_BITS_LEN	 55, 48
#define CVMX_FAU_BITS_INEVAL	 35, 14
#define CVMX_FAU_BITS_TAGWAIT	 13, 13
#define CVMX_FAU_BITS_NOADD	 13, 13
#define CVMX_FAU_BITS_SIZE	 12, 11
#define CVMX_FAU_BITS_REGISTER	 10, 0

#define CVMX_FAU_MAX_REGISTERS_8 (2048)

typedef enum {
	CVMX_FAU_OP_SIZE_8 = 0,
	CVMX_FAU_OP_SIZE_16 = 1,
	CVMX_FAU_OP_SIZE_32 = 2,
	CVMX_FAU_OP_SIZE_64 = 3
} cvmx_fau_op_size_t;

/**
 * Tagwait return definition. If a timeout occurs, the error
 * bit will be set. Otherwise the value of the register before
 * the update will be returned.
 */
typedef struct {
	u64 error : 1;
	s64 value : 63;
} cvmx_fau_tagwait64_t;

/**
 * Tagwait return definition. If a timeout occurs, the error
 * bit will be set. Otherwise the value of the register before
 * the update will be returned.
 */
typedef struct {
	u64 error : 1;
	s32 value : 31;
} cvmx_fau_tagwait32_t;

/**
 * Tagwait return definition. If a timeout occurs, the error
 * bit will be set. Otherwise the value of the register before
 * the update will be returned.
 */
typedef struct {
	u64 error : 1;
	s16 value : 15;
} cvmx_fau_tagwait16_t;

/**
 * Tagwait return definition. If a timeout occurs, the error
 * bit will be set. Otherwise the value of the register before
 * the update will be returned.
 */
typedef struct {
	u64 error : 1;
	int8_t value : 7;
} cvmx_fau_tagwait8_t;

/**
 * Asynchronous tagwait return definition. If a timeout occurs,
 * the error bit will be set. Otherwise the value of the
 * register before the update will be returned.
 */
typedef union {
	u64 u64;
	struct {
		u64 invalid : 1;
		u64 data : 63; /* unpredictable if invalid is set */
	} s;
} cvmx_fau_async_tagwait_result_t;

#define SWIZZLE_8  0
#define SWIZZLE_16 0
#define SWIZZLE_32 0

/**
 * @INTERNAL
 * Builds a store I/O address for writing to the FAU
 *
 * @param noadd  0 = Store value is atomically added to the current value
 *               1 = Store value is atomically written over the current value
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 2 for 16 bit access.
 *               - Step by 4 for 32 bit access.
 *               - Step by 8 for 64 bit access.
 * Return: Address to store for atomic update
 */
static inline u64 __cvmx_hwfau_store_address(u64 noadd, u64 reg)
{
	return (CVMX_ADD_IO_SEG(CVMX_FAU_LOAD_IO_ADDRESS) |
		cvmx_build_bits(CVMX_FAU_BITS_NOADD, noadd) |
		cvmx_build_bits(CVMX_FAU_BITS_REGISTER, reg));
}

/**
 * @INTERNAL
 * Builds a I/O address for accessing the FAU
 *
 * @param tagwait Should the atomic add wait for the current tag switch
 *                operation to complete.
 *                - 0 = Don't wait
 *                - 1 = Wait for tag switch to complete
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 *                - Step by 4 for 32 bit access.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 *                Note: When performing 32 and 64 bit access, only the low
 *                22 bits are available.
 * Return: Address to read from for atomic update
 */
static inline u64 __cvmx_hwfau_atomic_address(u64 tagwait, u64 reg, s64 value)
{
	return (CVMX_ADD_IO_SEG(CVMX_FAU_LOAD_IO_ADDRESS) |
		cvmx_build_bits(CVMX_FAU_BITS_INEVAL, value) |
		cvmx_build_bits(CVMX_FAU_BITS_TAGWAIT, tagwait) |
		cvmx_build_bits(CVMX_FAU_BITS_REGISTER, reg));
}

/**
 * Perform an atomic 64 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * Return: Value of the register before the update
 */
static inline s64 cvmx_hwfau_fetch_and_add64(cvmx_fau_reg64_t reg, s64 value)
{
	return cvmx_read64_int64(__cvmx_hwfau_atomic_address(0, reg, value));
}

/**
 * Perform an atomic 32 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * Return: Value of the register before the update
 */
static inline s32 cvmx_hwfau_fetch_and_add32(cvmx_fau_reg32_t reg, s32 value)
{
	reg ^= SWIZZLE_32;
	return cvmx_read64_int32(__cvmx_hwfau_atomic_address(0, reg, value));
}

/**
 * Perform an atomic 16 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to add.
 * Return: Value of the register before the update
 */
static inline s16 cvmx_hwfau_fetch_and_add16(cvmx_fau_reg16_t reg, s16 value)
{
	reg ^= SWIZZLE_16;
	return cvmx_read64_int16(__cvmx_hwfau_atomic_address(0, reg, value));
}

/**
 * Perform an atomic 8 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 * Return: Value of the register before the update
 */
static inline int8_t cvmx_hwfau_fetch_and_add8(cvmx_fau_reg8_t reg, int8_t value)
{
	reg ^= SWIZZLE_8;
	return cvmx_read64_int8(__cvmx_hwfau_atomic_address(0, reg, value));
}

/**
 * Perform an atomic 64 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 8 for 64 bit access.
 * @param value  Signed value to add.
 *               Note: Only the low 22 bits are available.
 * Return: If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait64_t cvmx_hwfau_tagwait_fetch_and_add64(cvmx_fau_reg64_t reg,
								      s64 value)
{
	union {
		u64 i64;
		cvmx_fau_tagwait64_t t;
	} result;
	result.i64 = cvmx_read64_int64(__cvmx_hwfau_atomic_address(1, reg, value));
	return result.t;
}

/**
 * Perform an atomic 32 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 4 for 32 bit access.
 * @param value  Signed value to add.
 *               Note: Only the low 22 bits are available.
 * Return: If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait32_t cvmx_hwfau_tagwait_fetch_and_add32(cvmx_fau_reg32_t reg,
								      s32 value)
{
	union {
		u64 i32;
		cvmx_fau_tagwait32_t t;
	} result;
	reg ^= SWIZZLE_32;
	result.i32 = cvmx_read64_int32(__cvmx_hwfau_atomic_address(1, reg, value));
	return result.t;
}

/**
 * Perform an atomic 16 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 2 for 16 bit access.
 * @param value  Signed value to add.
 * Return: If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait16_t cvmx_hwfau_tagwait_fetch_and_add16(cvmx_fau_reg16_t reg,
								      s16 value)
{
	union {
		u64 i16;
		cvmx_fau_tagwait16_t t;
	} result;
	reg ^= SWIZZLE_16;
	result.i16 = cvmx_read64_int16(__cvmx_hwfau_atomic_address(1, reg, value));
	return result.t;
}

/**
 * Perform an atomic 8 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 * @param value  Signed value to add.
 * Return: If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait8_t cvmx_hwfau_tagwait_fetch_and_add8(cvmx_fau_reg8_t reg,
								    int8_t value)
{
	union {
		u64 i8;
		cvmx_fau_tagwait8_t t;
	} result;
	reg ^= SWIZZLE_8;
	result.i8 = cvmx_read64_int8(__cvmx_hwfau_atomic_address(1, reg, value));
	return result.t;
}

/**
 * @INTERNAL
 * Builds I/O data for async operations
 *
 * @param scraddr Scratch pad byte address to write to.  Must be 8 byte aligned
 * @param value   Signed value to add.
 *                Note: When performing 32 and 64 bit access, only the low
 *                22 bits are available.
 * @param tagwait Should the atomic add wait for the current tag switch
 *                operation to complete.
 *                - 0 = Don't wait
 *                - 1 = Wait for tag switch to complete
 * @param size    The size of the operation:
 *                - CVMX_FAU_OP_SIZE_8  (0) = 8 bits
 *                - CVMX_FAU_OP_SIZE_16 (1) = 16 bits
 *                - CVMX_FAU_OP_SIZE_32 (2) = 32 bits
 *                - CVMX_FAU_OP_SIZE_64 (3) = 64 bits
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 *                - Step by 4 for 32 bit access.
 *                - Step by 8 for 64 bit access.
 * Return: Data to write using cvmx_send_single
 */
static inline u64 __cvmx_fau_iobdma_data(u64 scraddr, s64 value, u64 tagwait,
					 cvmx_fau_op_size_t size, u64 reg)
{
	return (CVMX_FAU_LOAD_IO_ADDRESS | cvmx_build_bits(CVMX_FAU_BITS_SCRADDR, scraddr >> 3) |
		cvmx_build_bits(CVMX_FAU_BITS_LEN, 1) |
		cvmx_build_bits(CVMX_FAU_BITS_INEVAL, value) |
		cvmx_build_bits(CVMX_FAU_BITS_TAGWAIT, tagwait) |
		cvmx_build_bits(CVMX_FAU_BITS_SIZE, size) |
		cvmx_build_bits(CVMX_FAU_BITS_REGISTER, reg));
}

/**
 * Perform an async atomic 64 bit add. The old value is
 * placed in the scratch memory at byte address scraddr.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_fetch_and_add64(u64 scraddr, cvmx_fau_reg64_t reg, s64 value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 0, CVMX_FAU_OP_SIZE_64, reg));
}

/**
 * Perform an async atomic 32 bit add. The old value is
 * placed in the scratch memory at byte address scraddr.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_fetch_and_add32(u64 scraddr, cvmx_fau_reg32_t reg, s32 value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 0, CVMX_FAU_OP_SIZE_32, reg));
}

/**
 * Perform an async atomic 16 bit add. The old value is
 * placed in the scratch memory at byte address scraddr.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to add.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_fetch_and_add16(u64 scraddr, cvmx_fau_reg16_t reg, s16 value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 0, CVMX_FAU_OP_SIZE_16, reg));
}

/**
 * Perform an async atomic 8 bit add. The old value is
 * placed in the scratch memory at byte address scraddr.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_fetch_and_add8(u64 scraddr, cvmx_fau_reg8_t reg, int8_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 0, CVMX_FAU_OP_SIZE_8, reg));
}

/**
 * Perform an async atomic 64 bit add after the current tag
 * switch completes.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 *                If a timeout occurs, the error bit (63) will be set. Otherwise
 *                the value of the register before the update will be
 *                returned
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_tagwait_fetch_and_add64(u64 scraddr, cvmx_fau_reg64_t reg,
							    s64 value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 1, CVMX_FAU_OP_SIZE_64, reg));
}

/**
 * Perform an async atomic 32 bit add after the current tag
 * switch completes.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 *                If a timeout occurs, the error bit (63) will be set. Otherwise
 *                the value of the register before the update will be
 *                returned
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_tagwait_fetch_and_add32(u64 scraddr, cvmx_fau_reg32_t reg,
							    s32 value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 1, CVMX_FAU_OP_SIZE_32, reg));
}

/**
 * Perform an async atomic 16 bit add after the current tag
 * switch completes.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 *                If a timeout occurs, the error bit (63) will be set. Otherwise
 *                the value of the register before the update will be
 *                returned
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to add.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_tagwait_fetch_and_add16(u64 scraddr, cvmx_fau_reg16_t reg,
							    s16 value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 1, CVMX_FAU_OP_SIZE_16, reg));
}

/**
 * Perform an async atomic 8 bit add after the current tag
 * switch completes.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 *                If a timeout occurs, the error bit (63) will be set. Otherwise
 *                the value of the register before the update will be
 *                returned
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 * Return: Placed in the scratch pad register
 */
static inline void cvmx_hwfau_async_tagwait_fetch_and_add8(u64 scraddr, cvmx_fau_reg8_t reg,
							   int8_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data(scraddr, value, 1, CVMX_FAU_OP_SIZE_8, reg));
}

/**
 * Perform an atomic 64 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 */
static inline void cvmx_hwfau_atomic_add64(cvmx_fau_reg64_t reg, s64 value)
{
	cvmx_write64_int64(__cvmx_hwfau_store_address(0, reg), value);
}

/**
 * Perform an atomic 32 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to add.
 */
static inline void cvmx_hwfau_atomic_add32(cvmx_fau_reg32_t reg, s32 value)
{
	reg ^= SWIZZLE_32;
	cvmx_write64_int32(__cvmx_hwfau_store_address(0, reg), value);
}

/**
 * Perform an atomic 16 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to add.
 */
static inline void cvmx_hwfau_atomic_add16(cvmx_fau_reg16_t reg, s16 value)
{
	reg ^= SWIZZLE_16;
	cvmx_write64_int16(__cvmx_hwfau_store_address(0, reg), value);
}

/**
 * Perform an atomic 8 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 */
static inline void cvmx_hwfau_atomic_add8(cvmx_fau_reg8_t reg, int8_t value)
{
	reg ^= SWIZZLE_8;
	cvmx_write64_int8(__cvmx_hwfau_store_address(0, reg), value);
}

/**
 * Perform an atomic 64 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to write.
 */
static inline void cvmx_hwfau_atomic_write64(cvmx_fau_reg64_t reg, s64 value)
{
	cvmx_write64_int64(__cvmx_hwfau_store_address(1, reg), value);
}

/**
 * Perform an atomic 32 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to write.
 */
static inline void cvmx_hwfau_atomic_write32(cvmx_fau_reg32_t reg, s32 value)
{
	reg ^= SWIZZLE_32;
	cvmx_write64_int32(__cvmx_hwfau_store_address(1, reg), value);
}

/**
 * Perform an atomic 16 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to write.
 */
static inline void cvmx_hwfau_atomic_write16(cvmx_fau_reg16_t reg, s16 value)
{
	reg ^= SWIZZLE_16;
	cvmx_write64_int16(__cvmx_hwfau_store_address(1, reg), value);
}

/**
 * Perform an atomic 8 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to write.
 */
static inline void cvmx_hwfau_atomic_write8(cvmx_fau_reg8_t reg, int8_t value)
{
	reg ^= SWIZZLE_8;
	cvmx_write64_int8(__cvmx_hwfau_store_address(1, reg), value);
}

/** Allocates 64bit FAU register.
 *  Return: value is the base address of allocated FAU register
 */
int cvmx_fau64_alloc(int reserve);

/** Allocates 32bit FAU register.
 *  Return: value is the base address of allocated FAU register
 */
int cvmx_fau32_alloc(int reserve);

/** Allocates 16bit FAU register.
 *  Return: value is the base address of allocated FAU register
 */
int cvmx_fau16_alloc(int reserve);

/** Allocates 8bit FAU register.
 *  Return: value is the base address of allocated FAU register
 */
int cvmx_fau8_alloc(int reserve);

/** Frees the specified FAU register.
 *  @param address Base address of register to release.
 *  Return: 0 on success; -1 on failure
 */
int cvmx_fau_free(int address);

/** Display the fau registers array
 */
void cvmx_fau_show(void);

#endif /* __CVMX_HWFAU_H__ */
