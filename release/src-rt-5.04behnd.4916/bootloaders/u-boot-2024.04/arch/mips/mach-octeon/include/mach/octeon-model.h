/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_MODEL_H__
#define __OCTEON_MODEL_H__

/*
 * NOTE: These must match what is checked in common-config.mk
 * Defines to represent the different versions of Octeon.
 *
 * IMPORTANT: When the default pass is updated for an Octeon Model,
 * the corresponding change must also be made in the oct-sim script.
 *
 * The defines below should be used with the OCTEON_IS_MODEL() macro to
 * determine what model of chip the software is running on.  Models ending
 * in 'XX' match multiple models (families), while specific models match only
 * that model.  If a pass (revision) is specified, then only that revision
 * will be matched.  Care should be taken when checking for both specific
 * models and families that the specific models are checked for first.
 * While these defines are similar to the processor ID, they are not intended
 * to be used by anything other that the OCTEON_IS_MODEL framework, and
 * the values are subject to change at anytime without notice.
 *
 * NOTE: only the OCTEON_IS_MODEL() macro/function and the OCTEON_CN* macros
 * should be used outside of this file.  All other macros are for internal
 * use only, and may change without notice.
 */

#include <asm/mipsregs.h>

#define OCTEON_FAMILY_MASK      0x00ffff00
#define OCTEON_PRID_MASK	0x00ffffff

/* Flag bits in top byte */
/* Ignores revision in model checks */
#define OM_IGNORE_REVISION        0x01000000
/* Check submodels */
#define OM_CHECK_SUBMODEL         0x02000000
/* Match all models previous than the one specified */
#define OM_MATCH_PREVIOUS_MODELS  0x04000000
/* Ignores the minor revison on newer parts */
#define OM_IGNORE_MINOR_REVISION  0x08000000
#define OM_FLAG_MASK              0xff000000

/* Match all cn5XXX Octeon models. */
#define OM_MATCH_5XXX_FAMILY_MODELS     0x20000000
/* Match all cn6XXX Octeon models. */
#define OM_MATCH_6XXX_FAMILY_MODELS     0x40000000
/* Match all cnf7XXX Octeon models. */
#define OM_MATCH_F7XXX_FAMILY_MODELS    0x80000000
/* Match all cn7XXX Octeon models. */
#define OM_MATCH_7XXX_FAMILY_MODELS     0x10000000
#define OM_MATCH_FAMILY_MODELS		(OM_MATCH_5XXX_FAMILY_MODELS | \
					 OM_MATCH_6XXX_FAMILY_MODELS |	\
					 OM_MATCH_F7XXX_FAMILY_MODELS | \
					 OM_MATCH_7XXX_FAMILY_MODELS)

/*
 * CN7XXX models with new revision encoding
 */

#define OCTEON_CNF75XX_PASS1_0  0x000d9800
#define OCTEON_CNF75XX_PASS1_2  0x000d9802
#define OCTEON_CNF75XX_PASS1_3  0x000d9803
#define OCTEON_CNF75XX          (OCTEON_CNF75XX_PASS1_0 | OM_IGNORE_REVISION)
#define OCTEON_CNF75XX_PASS1_X					\
	(OCTEON_CNF75XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)

#define OCTEON_CN73XX_PASS1_0   0x000d9700
#define OCTEON_CN73XX_PASS1_1   0x000d9701
#define OCTEON_CN73XX_PASS1_2   0x000d9702
#define OCTEON_CN73XX_PASS1_3   0x000d9703
#define OCTEON_CN73XX           (OCTEON_CN73XX_PASS1_0 | OM_IGNORE_REVISION)
#define OCTEON_CN73XX_PASS1_X					\
	(OCTEON_CN73XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)

#define OCTEON_CN72XX		OCTEON_CN73XX

#define OCTEON_CN23XX		OCTEON_CN73XX
#define OCTEON_CN23XX_PASS1_2	OCTEON_CN73XX_PASS1_2
#define OCTEON_CN23XX_PASS1_3	OCTEON_CN73XX_PASS1_3

#define OCTEON_CN70XX_PASS1_0   0x000d9600
#define OCTEON_CN70XX_PASS1_1   0x000d9601
#define OCTEON_CN70XX_PASS1_2   0x000d9602

#define OCTEON_CN70XX_PASS2_0   0x000d9608

#define OCTEON_CN70XX           (OCTEON_CN70XX_PASS1_0 | OM_IGNORE_REVISION)
#define OCTEON_CN70XX_PASS1_X					\
	(OCTEON_CN70XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)
#define OCTEON_CN70XX_PASS2_X					\
	(OCTEON_CN70XX_PASS2_0 | OM_IGNORE_MINOR_REVISION)

#define OCTEON_CN71XX		OCTEON_CN70XX

#define OCTEON_CN78XX_PASS1_0   0x000d9500
#define OCTEON_CN78XX_PASS1_1   0x000d9501
#define OCTEON_CN78XX_PASS2_0   0x000d9508

#define OCTEON_CN78XX           (OCTEON_CN78XX_PASS2_0 | OM_IGNORE_REVISION)
#define OCTEON_CN78XX_PASS1_X					\
	(OCTEON_CN78XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)
#define OCTEON_CN78XX_PASS2_X					\
	(OCTEON_CN78XX_PASS2_0 | OM_IGNORE_MINOR_REVISION)

#define OCTEON_CN76XX		  (0x000d9540 | OM_CHECK_SUBMODEL)

/*
 * CNF7XXX models with new revision encoding
 */
#define OCTEON_CNF71XX_PASS1_0  0x000d9400
#define OCTEON_CNF71XX_PASS1_1  0x000d9401

#define OCTEON_CNF71XX          (OCTEON_CNF71XX_PASS1_0 | OM_IGNORE_REVISION)
#define OCTEON_CNF71XX_PASS1_X					\
	(OCTEON_CNF71XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)

/*
 * CN6XXX models with new revision encoding
 */
#define OCTEON_CN68XX_PASS1_0   0x000d9100
#define OCTEON_CN68XX_PASS1_1   0x000d9101
#define OCTEON_CN68XX_PASS2_0   0x000d9108
#define OCTEON_CN68XX_PASS2_1   0x000d9109
#define OCTEON_CN68XX_PASS2_2   0x000d910a

#define OCTEON_CN68XX           (OCTEON_CN68XX_PASS2_0 | OM_IGNORE_REVISION)
#define OCTEON_CN68XX_PASS1_X					\
	(OCTEON_CN68XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)
#define OCTEON_CN68XX_PASS2_X					\
	(OCTEON_CN68XX_PASS2_0 | OM_IGNORE_MINOR_REVISION)

#define OCTEON_CN68XX_PASS1	OCTEON_CN68XX_PASS1_X
#define OCTEON_CN68XX_PASS2	OCTEON_CN68XX_PASS2_X

#define OCTEON_CN66XX_PASS1_0   0x000d9200
#define OCTEON_CN66XX_PASS1_2   0x000d9202

#define OCTEON_CN66XX           (OCTEON_CN66XX_PASS1_0 | OM_IGNORE_REVISION)
#define OCTEON_CN66XX_PASS1_X					\
	(OCTEON_CN66XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)

#define OCTEON_CN63XX_PASS1_0   0x000d9000
#define OCTEON_CN63XX_PASS1_1   0x000d9001
#define OCTEON_CN63XX_PASS1_2   0x000d9002
#define OCTEON_CN63XX_PASS2_0   0x000d9008
#define OCTEON_CN63XX_PASS2_1   0x000d9009
#define OCTEON_CN63XX_PASS2_2   0x000d900a

#define OCTEON_CN63XX           (OCTEON_CN63XX_PASS2_0 | OM_IGNORE_REVISION)
#define OCTEON_CN63XX_PASS1_X					\
	(OCTEON_CN63XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)
#define OCTEON_CN63XX_PASS2_X					\
	(OCTEON_CN63XX_PASS2_0 | OM_IGNORE_MINOR_REVISION)

/* CN62XX is same as CN63XX with 1 MB cache */
#define OCTEON_CN62XX           OCTEON_CN63XX

#define OCTEON_CN61XX_PASS1_0   0x000d9300
#define OCTEON_CN61XX_PASS1_1   0x000d9301

#define OCTEON_CN61XX           (OCTEON_CN61XX_PASS1_0 | OM_IGNORE_REVISION)
#define OCTEON_CN61XX_PASS1_X					\
	(OCTEON_CN61XX_PASS1_0 | OM_IGNORE_MINOR_REVISION)

/* CN60XX is same as CN61XX with 512 KB cache */
#define OCTEON_CN60XX           OCTEON_CN61XX

/* This matches the complete family of CN3xxx CPUs, and not subsequent models */
#define OCTEON_CN6XXX						\
	(OCTEON_CN63XX_PASS1_0 | OM_MATCH_6XXX_FAMILY_MODELS)
#define OCTEON_CNF7XXX						\
	(OCTEON_CNF71XX_PASS1_0 | OM_MATCH_F7XXX_FAMILY_MODELS)
#define OCTEON_CN7XXX						\
	(OCTEON_CN78XX_PASS1_0 | OM_MATCH_7XXX_FAMILY_MODELS)

/*
 * The revision byte (low byte) has two different encodings.
 * CN3XXX:
 *
 *     bits
 *     <7:5>: reserved (0)
 *     <4>:   alternate package
 *     <3:0>: revision
 *
 * CN5XXX and older models:
 *
 *     bits
 *     <7>:   reserved (0)
 *     <6>:   alternate package
 *     <5:3>: major revision
 *     <2:0>: minor revision
 */

/* Masks used for the various types of model/family/revision matching */
#define OCTEON_38XX_FAMILY_MASK      0x00ffff00
#define OCTEON_38XX_FAMILY_REV_MASK  0x00ffff0f
#define OCTEON_38XX_MODEL_MASK       0x00ffff10
#define OCTEON_38XX_MODEL_REV_MASK				\
	(OCTEON_38XX_FAMILY_REV_MASK | OCTEON_38XX_MODEL_MASK)

/* CN5XXX and later use different layout of bits in the revision ID field */
#define OCTEON_58XX_FAMILY_MASK      OCTEON_38XX_FAMILY_MASK
#define OCTEON_58XX_FAMILY_REV_MASK  0x00ffff3f
#define OCTEON_58XX_MODEL_MASK       0x00ffff40
#define OCTEON_58XX_MODEL_REV_MASK				\
	(OCTEON_58XX_FAMILY_REV_MASK | OCTEON_58XX_MODEL_MASK)
#define OCTEON_58XX_MODEL_MINOR_REV_MASK		\
	(OCTEON_58XX_MODEL_REV_MASK & 0x00ffff38)
#define OCTEON_5XXX_MODEL_MASK       0x00ff0fc0

#define __OCTEON_MATCH_MASK__(X, Y, Z)		     \
	({					     \
		typeof(X) x = (X);		     \
		typeof(Y) y = (Y);		     \
		typeof(Z) z = (Z);		     \
		(x & z) == (y & z);		     \
	 })

/*
 * __OCTEON_IS_MODEL_COMPILE__(arg_model, chip_model)
 * returns true if chip_model is identical or belong to the OCTEON
 * model group specified in arg_model.
 */

/* Helper macros to make to following macro compacter */
#define OM_MASK			OM_FLAG_MASK
#define OM_MATCH_MASK		__OCTEON_MATCH_MASK__
#define OM_MATCH_PREVIOUS	OM_MATCH_PREVIOUS_MODELS

#define __OCTEON_IS_MODEL_COMPILE__(A, B)				\
	({								\
	typeof(A) a = (A);						\
	typeof(B) b = (B);						\
	(((((((a) & OM_MASK) == (OM_IGNORE_REVISION | OM_CHECK_SUBMODEL)) && \
	    OM_MATCH_MASK((b), (a), OCTEON_58XX_MODEL_MASK)) ||		\
	   ((((a) & OM_MASK) == 0) &&					\
	    OM_MATCH_MASK((b), (a), OCTEON_58XX_FAMILY_REV_MASK)) ||	\
	   ((((a) & OM_MASK) == OM_IGNORE_MINOR_REVISION) &&		\
	    OM_MATCH_MASK((b), (a), OCTEON_58XX_MODEL_MINOR_REV_MASK)) || \
	   ((((a) & OM_MASK) == OM_CHECK_SUBMODEL) &&			\
	    OM_MATCH_MASK((b), (a), OCTEON_58XX_MODEL_MASK)) ||		\
	   ((((a) & OM_MASK) == OM_IGNORE_REVISION) &&			\
	    OM_MATCH_MASK((b), (a), OCTEON_58XX_FAMILY_MASK)) ||	\
	   ((((a) & (OM_MATCH_5XXX_FAMILY_MODELS)) ==			\
	     OM_MATCH_5XXX_FAMILY_MODELS) &&				\
	    ((b & OCTEON_PRID_MASK) < OCTEON_CN63XX_PASS1_0)) ||	\
	   ((((a) & (OM_MATCH_6XXX_FAMILY_MODELS)) ==			\
	     OM_MATCH_6XXX_FAMILY_MODELS) &&				\
	    ((b & OCTEON_PRID_MASK) >= OCTEON_CN63XX_PASS1_0) &&	\
	    ((b & OCTEON_PRID_MASK) < OCTEON_CNF71XX_PASS1_0)) ||	\
	   ((((a) & (OM_MATCH_F7XXX_FAMILY_MODELS)) ==			\
	     OM_MATCH_F7XXX_FAMILY_MODELS) &&				\
	    ((b & OCTEON_PRID_MASK) >= OCTEON_CNF71XX_PASS1_0) &&	\
	    ((b & OCTEON_PRID_MASK) < OCTEON_CN78XX_PASS1_0)) ||	\
	   ((((a) & (OM_MATCH_7XXX_FAMILY_MODELS)) ==			\
	     OM_MATCH_7XXX_FAMILY_MODELS) && ((b & OCTEON_PRID_MASK) >=	\
					      OCTEON_CN78XX_PASS1_0)) || \
	   ((((a) & (OM_MATCH_PREVIOUS)) == OM_MATCH_PREVIOUS) &&	\
	    (((b) & OCTEON_58XX_MODEL_MASK) < ((a) & OCTEON_58XX_MODEL_MASK))) \
		  )));							\
	})

#ifndef __ASSEMBLY__

#ifndef OCTEON_IS_MODEL

static inline int __octeon_is_model_runtime_internal__(u32 model)
{
	u32 cpuid = read_c0_prid();

	return __OCTEON_IS_MODEL_COMPILE__(model, cpuid);
}

static inline int __octeon_is_model_runtime__(u32 model)
{
	return __octeon_is_model_runtime_internal__(model);
}

/*
 * The OCTEON_IS_MODEL macro should be used for all Octeon model checking done
 * in a program.
 * This should be kept runtime if at all possible  and must be conditionalized
 * with OCTEON_IS_COMMON_BINARY() if runtime checking support is required.
 *
 * Use of the macro in preprocessor directives ( #if OCTEON_IS_MODEL(...) )
 * is NOT SUPPORTED, and should be replaced with CVMX_COMPILED_FOR()
 * I.e.:
 * #if OCTEON_IS_MODEL(OCTEON_CN56XX)  ->  #if CVMX_COMPILED_FOR(OCTEON_CN56XX)
 */
#define OCTEON_IS_MODEL(x)	__octeon_is_model_runtime__(x)
#define OCTEON_IS_COMMON_BINARY() 1
#undef OCTEON_MODEL
#endif

#define OCTEON_IS_OCTEON2()						\
	(OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_MODEL(OCTEON_CNF71XX))

#define OCTEON_IS_OCTEON3()	OCTEON_IS_MODEL(OCTEON_CN7XXX)

const char *octeon_model_get_string(u32 chip_id);
const char *octeon_model_get_string_buffer(u32 chip_id, char *buffer);

/**
 * Return the octeon family, i.e., ProcessorID of the PrID register.
 *
 * Return: the octeon family on success, ((u32)-1) on error.
 */
static inline u32 cvmx_get_octeon_family(void)
{
	return (read_c0_prid() & OCTEON_FAMILY_MASK);
}

#endif /* __ASSEMBLY__ */

#endif /* __OCTEON_MODEL_H__ */
