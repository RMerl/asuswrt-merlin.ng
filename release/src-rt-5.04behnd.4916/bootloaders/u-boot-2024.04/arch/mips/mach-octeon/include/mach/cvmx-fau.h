/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Interface to the hardware Fetch and Add Unit.
 */

#ifndef __CVMX_FAU_H__
#define __CVMX_FAU_H__

extern u8 *cvmx_fau_regs_ptr;

/**
 * Initializes fau, on devices with FAU hw this is a noop.
 */
int cvmx_fau_init(void);

/**
 * Return the location of emulated FAU register
 */
static inline u8 *__cvmx_fau_sw_addr(int reg)
{
	if (cvmx_unlikely(!cvmx_fau_regs_ptr))
		cvmx_fau_init();
	return (cvmx_fau_regs_ptr + reg);
}

/**
 * Perform an atomic 64 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * @return Value of the register before the update
 */
static inline int64_t cvmx_fau_fetch_and_add64(cvmx_fau_reg64_t reg,
					       int64_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_fetch_and_add64(reg, value);

	return __atomic_fetch_add(CASTPTR(int64_t, __cvmx_fau_sw_addr(reg)),
				  value, __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 32 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to add.
 *                Note: Only the low 22 bits are available.
 * @return Value of the register before the update
 */
static inline int32_t cvmx_fau_fetch_and_add32(cvmx_fau_reg32_t reg,
					       int32_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_fetch_and_add32(reg, value);

	reg ^= SWIZZLE_32;
	return __atomic_fetch_add(CASTPTR(int32_t, __cvmx_fau_sw_addr(reg)),
				  value, __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 16 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to add.
 * @return Value of the register before the update
 */
static inline int16_t cvmx_fau_fetch_and_add16(cvmx_fau_reg16_t reg,
					       int16_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_fetch_and_add16(reg, value);

	reg ^= SWIZZLE_16;
	return __atomic_fetch_add(CASTPTR(int16_t, __cvmx_fau_sw_addr(reg)),
				  value, __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 8 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 * @return Value of the register before the update
 */
static inline int8_t cvmx_fau_fetch_and_add8(cvmx_fau_reg8_t reg, int8_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_fetch_and_add8(reg, value);

	reg ^= SWIZZLE_8;
	return __atomic_fetch_add(CASTPTR(int8_t, __cvmx_fau_sw_addr(reg)),
				  value, __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 64 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 8 for 64 bit access.
 * @param value  Signed value to add.
 *               Note: Only the low 22 bits are available.
 * @return If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait64_t
cvmx_fau_tagwait_fetch_and_add64(cvmx_fau_reg64_t reg, int64_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_tagwait_fetch_and_add64(reg, value);

	/* not implemented yet.*/
	return (cvmx_fau_tagwait64_t){ 1, 0 };
}

/**
 * Perform an atomic 32 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 4 for 32 bit access.
 * @param value  Signed value to add.
 *               Note: Only the low 22 bits are available.
 * @return If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait32_t
cvmx_fau_tagwait_fetch_and_add32(cvmx_fau_reg32_t reg, int32_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_tagwait_fetch_and_add32(reg, value);

	/* not implemented yet.*/
	return (cvmx_fau_tagwait32_t){ 1, 0 };
}

/**
 * Perform an atomic 16 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 *               - Step by 2 for 16 bit access.
 * @param value  Signed value to add.
 * @return If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait16_t
cvmx_fau_tagwait_fetch_and_add16(cvmx_fau_reg16_t reg, int16_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_tagwait_fetch_and_add16(reg, value);

	/* not implemented yet.*/
	return (cvmx_fau_tagwait16_t){ 1, 0 };
}

/**
 * Perform an atomic 8 bit add after the current tag switch
 * completes
 *
 * @param reg    FAU atomic register to access. 0 <= reg < 2048.
 * @param value  Signed value to add.
 * @return If a timeout occurs, the error bit will be set. Otherwise
 *         the value of the register before the update will be
 *         returned
 */
static inline cvmx_fau_tagwait8_t
cvmx_fau_tagwait_fetch_and_add8(cvmx_fau_reg8_t reg, int8_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU))
		return cvmx_hwfau_tagwait_fetch_and_add8(reg, value);

	/* not implemented yet.*/
	return (cvmx_fau_tagwait8_t){ 1, 0 };
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
 * @return Placed in the scratch pad register
 */
static inline void
cvmx_fau_async_fetch_and_add64(u64 scraddr, cvmx_fau_reg64_t reg, int64_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_fetch_and_add64(scraddr, reg, value);
		return;
	}
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int64_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
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
 * @return Placed in the scratch pad register
 */
static inline void
cvmx_fau_async_fetch_and_add32(u64 scraddr, cvmx_fau_reg32_t reg, int32_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_fetch_and_add32(scraddr, reg, value);
		return;
	}
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int32_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
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
 * @return Placed in the scratch pad register
 */
static inline void
cvmx_fau_async_fetch_and_add16(u64 scraddr, cvmx_fau_reg16_t reg, int16_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_fetch_and_add16(scraddr, reg, value);
		return;
	}
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int16_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
}

/**
 * Perform an async atomic 8 bit add. The old value is
 * placed in the scratch memory at byte address scraddr.
 *
 * @param scraddr Scratch memory byte address to put response in.
 *                Must be 8 byte aligned.
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 * @return Placed in the scratch pad register
 */
static inline void
cvmx_fau_async_fetch_and_add8(u64 scraddr, cvmx_fau_reg8_t reg, int8_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_fetch_and_add8(scraddr, reg, value);
		return;
	}
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int8_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
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
 * @return Placed in the scratch pad register
 */
static inline void cvmx_fau_async_tagwait_fetch_and_add64(u64 scraddr,
							  cvmx_fau_reg64_t reg,
							  int64_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_tagwait_fetch_and_add64(scraddr, reg, value);
		return;
	}

	/* Broken.  Where is the tag wait? */
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int64_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
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
 * @return Placed in the scratch pad register
 */
static inline void cvmx_fau_async_tagwait_fetch_and_add32(u64 scraddr,
							  cvmx_fau_reg32_t reg,
							  int32_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_tagwait_fetch_and_add32(scraddr, reg, value);
		return;
	}
	/* Broken.  Where is the tag wait? */
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int32_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
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
 * @return Placed in the scratch pad register
 */
static inline void cvmx_fau_async_tagwait_fetch_and_add16(u64 scraddr,
							  cvmx_fau_reg16_t reg,
							  int16_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_tagwait_fetch_and_add16(scraddr, reg, value);
		return;
	}
	/* Broken.  Where is the tag wait? */
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int16_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
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
 * @return Placed in the scratch pad register
 */
static inline void cvmx_fau_async_tagwait_fetch_and_add8(u64 scraddr,
							 cvmx_fau_reg8_t reg,
							 int8_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_async_tagwait_fetch_and_add8(scraddr, reg, value);
		return;
	}
	/* Broken.  Where is the tag wait? */
	cvmx_scratch_write64(
		scraddr,
		__atomic_fetch_add(CASTPTR(int8_t, __cvmx_fau_sw_addr(reg)),
				   value, __ATOMIC_SEQ_CST));
}

/**
 * Perform an atomic 64 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to add.
 */
static inline void cvmx_fau_atomic_add64(cvmx_fau_reg64_t reg, int64_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_add64(reg, value);
		return;
	}
	/* Ignored fetch values should be optimized away */
	__atomic_add_fetch(CASTPTR(int64_t, __cvmx_fau_sw_addr(reg)), value,
			   __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 32 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to add.
 */
static inline void cvmx_fau_atomic_add32(cvmx_fau_reg32_t reg, int32_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_add32(reg, value);
		return;
	}
	reg ^= SWIZZLE_32;
	/* Ignored fetch values should be optimized away */
	__atomic_add_fetch(CASTPTR(int32_t, __cvmx_fau_sw_addr(reg)), value,
			   __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 16 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to add.
 */
static inline void cvmx_fau_atomic_add16(cvmx_fau_reg16_t reg, int16_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_add16(reg, value);
		return;
	}
	reg ^= SWIZZLE_16;
	/* Ignored fetch values should be optimized away */
	__atomic_add_fetch(CASTPTR(int16_t, __cvmx_fau_sw_addr(reg)), value,
			   __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 8 bit add
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to add.
 */
static inline void cvmx_fau_atomic_add8(cvmx_fau_reg8_t reg, int8_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_add8(reg, value);
		return;
	}
	reg ^= SWIZZLE_8;
	/* Ignored fetch values should be optimized away */
	__atomic_add_fetch(CASTPTR(int8_t, __cvmx_fau_sw_addr(reg)), value,
			   __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 64 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 8 for 64 bit access.
 * @param value   Signed value to write.
 */
static inline void cvmx_fau_atomic_write64(cvmx_fau_reg64_t reg, int64_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_write64(reg, value);
		return;
	}
	__atomic_store_n(CASTPTR(int64_t, __cvmx_fau_sw_addr(reg)), value,
			 __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 32 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 4 for 32 bit access.
 * @param value   Signed value to write.
 */
static inline void cvmx_fau_atomic_write32(cvmx_fau_reg32_t reg, int32_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_write32(reg, value);
		return;
	}
	reg ^= SWIZZLE_32;
	__atomic_store_n(CASTPTR(int32_t, __cvmx_fau_sw_addr(reg)), value,
			 __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 16 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 *                - Step by 2 for 16 bit access.
 * @param value   Signed value to write.
 */
static inline void cvmx_fau_atomic_write16(cvmx_fau_reg16_t reg, int16_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_write16(reg, value);
		return;
	}
	reg ^= SWIZZLE_16;
	__atomic_store_n(CASTPTR(int16_t, __cvmx_fau_sw_addr(reg)), value,
			 __ATOMIC_SEQ_CST);
}

/**
 * Perform an atomic 8 bit write
 *
 * @param reg     FAU atomic register to access. 0 <= reg < 2048.
 * @param value   Signed value to write.
 */
static inline void cvmx_fau_atomic_write8(cvmx_fau_reg8_t reg, int8_t value)
{
	if (octeon_has_feature(OCTEON_FEATURE_FAU)) {
		cvmx_hwfau_atomic_write8(reg, value);
		return;
	}
	reg ^= SWIZZLE_8;
	__atomic_store_n(CASTPTR(int8_t, __cvmx_fau_sw_addr(reg)), value,
			 __ATOMIC_SEQ_CST);
}

/** Allocates 64bit FAU register.
 *  @param reserve base address to reserve
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau64_alloc(int reserve);

/** Allocates 32bit FAU register.
 *  @param reserve base address to reserve
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau32_alloc(int reserve);

/** Allocates 16bit FAU register.
 *  @param reserve base address to reserve
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau16_alloc(int reserve);

/** Allocates 8bit FAU register.
 *  @param reserve base address to reserve
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau8_alloc(int reserve);

/** Frees the specified FAU register.
 *  @param address base address of register to release.
 *  @return 0 on success; -1 on failure
 */
int cvmx_fau_free(int address);

/** Display the fau registers array
 */
void cvmx_fau_show(void);

#endif /* __CVMX_FAU_H__ */
