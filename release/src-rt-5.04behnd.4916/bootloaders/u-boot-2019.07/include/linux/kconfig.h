#ifndef __LINUX_KCONFIG_H
#define __LINUX_KCONFIG_H

#include <generated/autoconf.h>

/*
 * Helper macros to use CONFIG_ options in C/CPP expressions. Note that
 * these only work with boolean and tristate options.
 */

/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
#define __ARG_PLACEHOLDER_1 0,
#define config_enabled(cfg) _config_enabled(cfg)
#define _config_enabled(value) __config_enabled(__ARG_PLACEHOLDER_##value)
#define __config_enabled(arg1_or_junk) ___config_enabled(arg1_or_junk 1, 0)
#define ___config_enabled(__ignored, val, ...) val

/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y' or 'm',
 * 0 otherwise.
 *
 */
#define IS_ENABLED(option) \
	(config_enabled(option) || config_enabled(option##_MODULE))

/*
 * IS_BUILTIN(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y', 0
 * otherwise. For boolean options, this is equivalent to
 * IS_ENABLED(CONFIG_FOO).
 */
#define IS_BUILTIN(option) config_enabled(option)

/*
 * IS_MODULE(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'm', 0
 * otherwise.
 */
#define IS_MODULE(option) config_enabled(option##_MODULE)

/*
 * U-Boot add-on: Helper macros to reference to different macros
 * (CONFIG_ or CONFIG_SPL_ prefixed), depending on the build context.
 */
#ifdef CONFIG_SPL_BUILD
#define _IS_SPL 1
#endif

#ifdef CONFIG_TPL_BUILD
#define _IS_TPL 1
#endif

#if defined(CONFIG_TPL_BUILD)
#define config_val(cfg) _config_val(_IS_TPL, cfg)
#define _config_val(x, cfg) __config_val(x, cfg)
#define __config_val(x, cfg) ___config_val(__ARG_PLACEHOLDER_##x, cfg)
#define ___config_val(arg1_or_junk, cfg)  \
	____config_val(arg1_or_junk CONFIG_TPL_##cfg, CONFIG_##cfg)
#define ____config_val(__ignored, val, ...) val
#else
#define config_val(cfg) _config_val(_IS_SPL, cfg)
#define _config_val(x, cfg) __config_val(x, cfg)
#define __config_val(x, cfg) ___config_val(__ARG_PLACEHOLDER_##x, cfg)
#define ___config_val(arg1_or_junk, cfg)  \
	____config_val(arg1_or_junk CONFIG_SPL_##cfg, CONFIG_##cfg)
#define ____config_val(__ignored, val, ...) val
#endif

/*
 * CONFIG_VAL(FOO) evaluates to the value of
 *  CONFIG_FOO if CONFIG_SPL_BUILD is undefined,
 *  CONFIG_SPL_FOO if CONFIG_SPL_BUILD is defined.
 */
#define CONFIG_VAL(option)  config_val(option)

/*
 * CONFIG_IS_ENABLED(FOO) evaluates to
 *  1 if CONFIG_SPL_BUILD is undefined and CONFIG_FOO is set to 'y' or 'm',
 *  1 if CONFIG_SPL_BUILD is defined and CONFIG_SPL_FOO is set to 'y' or 'm',
 *  0 otherwise.
 */
#define CONFIG_IS_ENABLED(option) \
	(config_enabled(CONFIG_VAL(option)) ||		\
	 config_enabled(CONFIG_VAL(option##_MODULE)))

/*
 * CONFIG_IS_BUILTIN(FOO) evaluates to
 *  1 if CONFIG_SPL_BUILD is undefined and CONFIG_FOO is set to 'y',
 *  1 if CONFIG_SPL_BUILD is defined and CONFIG_SPL_FOO is set to 'y',
 *  0 otherwise.
 */
#define CONFIG_IS_BUILTIN(option) config_enabled(CONFIG_VAL(option))

/*
 * CONFIG_IS_MODULE(FOO) evaluates to
 *  1 if CONFIG_SPL_BUILD is undefined and CONFIG_FOO is set to 'm',
 *  1 if CONFIG_SPL_BUILD is defined and CONFIG_SPL_FOO is set to 'm',
 *  0 otherwise.
 */
#define CONFIG_IS_MODULE(option) config_enabled(CONFIG_VAL(option##_MODULE))

#endif /* __LINUX_KCONFIG_H */
