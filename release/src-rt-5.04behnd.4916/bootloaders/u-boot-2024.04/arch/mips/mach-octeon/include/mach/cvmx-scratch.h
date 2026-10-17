/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * This file provides support for the processor local scratch memory.
 * Scratch memory is byte addressable - all addresses are byte addresses.
 */

#ifndef __CVMX_SCRATCH_H__
#define __CVMX_SCRATCH_H__

/* Note: This define must be a long, not a long long in order to compile
	without warnings for both 32bit and 64bit. */
#define CVMX_SCRATCH_BASE (-32768l) /* 0xffffffffffff8000 */

/* Scratch line for LMTST/LMTDMA on Octeon3 models */
#ifdef CVMX_CAVIUM_OCTEON3
#define CVMX_PKO_LMTLINE 2ull
#endif

/**
 * Reads an 8 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * Return: value read
 */
static inline u8 cvmx_scratch_read8(u64 address)
{
	return *CASTPTR(volatile u8, CVMX_SCRATCH_BASE + address);
}

/**
 * Reads a 16 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * Return: value read
 */
static inline u16 cvmx_scratch_read16(u64 address)
{
	return *CASTPTR(volatile u16, CVMX_SCRATCH_BASE + address);
}

/**
 * Reads a 32 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * Return: value read
 */
static inline u32 cvmx_scratch_read32(u64 address)
{
	return *CASTPTR(volatile u32, CVMX_SCRATCH_BASE + address);
}

/**
 * Reads a 64 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * Return: value read
 */
static inline u64 cvmx_scratch_read64(u64 address)
{
	return *CASTPTR(volatile u64, CVMX_SCRATCH_BASE + address);
}

/**
 * Writes an 8 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write8(u64 address, u64 value)
{
	*CASTPTR(volatile u8, CVMX_SCRATCH_BASE + address) = (u8)value;
}

/**
 * Writes a 32 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write16(u64 address, u64 value)
{
	*CASTPTR(volatile u16, CVMX_SCRATCH_BASE + address) = (u16)value;
}

/**
 * Writes a 16 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write32(u64 address, u64 value)
{
	*CASTPTR(volatile u32, CVMX_SCRATCH_BASE + address) = (u32)value;
}

/**
 * Writes a 64 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write64(u64 address, u64 value)
{
	*CASTPTR(volatile u64, CVMX_SCRATCH_BASE + address) = value;
}

#endif /* __CVMX_SCRATCH_H__ */
