/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __event_h
#define __event_h

#include <dm/ofnode_decl.h>
#include <linux/types.h>

/**
 * enum event_t - Types of events supported by U-Boot
 *
 * @EVT_DM_PRE_PROBE: Device is about to be probed
 */
enum event_t {
	/**
	 * @EVT_NONE: This zero value is not used for events.
	 */
	EVT_NONE = 0,

	/**
	 * @EVT_TEST: This event is used in unit tests.
	 */
	EVT_TEST,

	/**
	 * @EVT_DM_POST_INIT_F:
	 * This event is triggered after pre-relocation initialization of the
	 * driver model. Its parameter is NULL.
	 * A non-zero return code from the event handler let's the boot process
	 * fail.
	 */
	EVT_DM_POST_INIT_F,

	/**
	 * @EVT_DM_POST_INIT_R:
	 * This event is triggered after post-relocation initialization of the
	 * driver model. Its parameter is NULL.
	 * A non-zero return code from the event handler let's the boot process
	 * fail.
	 */
	EVT_DM_POST_INIT_R,

	/**
	 * @EVT_DM_PRE_PROBE:
	 * This event is triggered before probing a device. Its parameter is the
	 * device to be probed.
	 * A non-zero return code from the event handler lets the device not
	 * being probed.
	 */
	EVT_DM_PRE_PROBE,

	/**
	 * @EVT_DM_POST_PROBE:
	 * This event is triggered after probing a device. Its parameter is the
	 * device that was probed.
	 * A non-zero return code from the event handler leaves the device in
	 * the unprobed state and therefore not usable.
	 */
	EVT_DM_POST_PROBE,

	/**
	 * @EVT_DM_PRE_REMOVE:
	 * This event is triggered after removing a device. Its parameter is
	 * the device to be removed.
	 * A non-zero return code from the event handler stops the removal of
	 * the device before any changes.
	 */
	EVT_DM_PRE_REMOVE,

	/**
	 * @EVT_DM_POST_REMOVE:
	 * This event is triggered before removing a device. Its parameter is
	 * the device that was removed.
	 * A non-zero return code stops from the event handler the removal of
	 * the device after all removal changes. The previous state is not
	 * restored. All children will be gone and the device may not be
	 * functional.
	 */
	EVT_DM_POST_REMOVE,

	/**
	 * @EVT_MISC_INIT_F:
	 * This event is triggered during the initialization sequence before
	 * relocation. Its parameter is NULL.
	 * A non-zero return code from the event handler let's the boot process
	 * fail.
	 */
	EVT_MISC_INIT_F,

	/**
	 * @EVT_FSP_INIT_F:
	 * This event is triggered before relocation to set up Firmware Support
	 * Package.
	 * Where U-Boot relies on binary blobs to handle part of the system
	 * init, this event can be used to set up the blobs. This is used on
	 * some Intel platforms
	 */
	EVT_FSP_INIT_F,

	/**
	 * @EVT_SETTINGS_R:
	 * This event is triggered post-relocation and before console init.
	 * This gives an option to perform any platform-dependent setup, which
	 * needs to take place before show_board_info() (e.g. readout of EEPROM
	 * stored settings).
	 */
	EVT_SETTINGS_R,

	/**
	 * @EVT_LAST_STAGE_INIT:
	 * This event is triggered just before jumping to the main loop.
	 * Some boards need to perform initialisation immediately before control
	 * is passed to the command-line interpreter (e.g. for init that depend
	 * on later phases in the init sequence).
	 *
	 * Some parts can be only initialized if all others (like Interrupts)
	 * are up and running (e.g. the PC-style ISA keyboard).
	 */
	EVT_LAST_STAGE_INIT,

	/**
	 * @EVT_FPGA_LOAD:
	 * The FPGA load hook is called after loading an FPGA with a new binary.
	 * Its parameter is of type struct event_fpga_load and contains
	 * information about the loaded image.
	 */
	EVT_FPGA_LOAD,

	/**
	 * @EVT_FT_FIXUP:
	 * This event is triggered during device-tree fix up after all
	 * other device-tree fixups have been executed.
	 * Its parameter is of type struct event_ft_fixup which contains
	 * the address of the device-tree to fix up and the list of images to be
	 * booted.
	 * A non-zero return code from the event handler let's booting the
	 * images fail.
	 */
	EVT_FT_FIXUP,

	/**
	 * @EVT_MAIN_LOOP:
	 * This event is triggered immediately before calling main_loop() which
	 * is the entry point of the command line. Its parameter is NULL.
	 * A non-zero return value causes the boot to fail.
	 */
	EVT_MAIN_LOOP,

	/**
	 * @EVT_COUNT:
	 * This constants holds the maximum event number + 1 and is used when
	 * looping over all event classes.
	 */
	EVT_COUNT
};

union event_data {
	/**
	 * struct event_data_test  - test data
	 *
	 * @signal: A value to update the state with
	 */
	struct event_data_test {
		int signal;
	} test;

	/**
	 * struct event_dm - driver model event
	 *
	 * @dev: Device this event relates to
	 */
	struct event_dm {
		struct udevice *dev;
	} dm;

	/**
	 * struct event_fpga_load - fpga load event
	 *
	 * @buf: The buffer that was loaded into the fpga
	 * @bsize: The size of the buffer that was loaded into the fpga
	 * @result: Result of the load operation
	 */
	struct event_fpga_load {
		const void *buf;
		size_t bsize;
		int result;
	} fpga_load;

	/**
	 * struct event_ft_fixup - FDT fixup before booting
	 *
	 * @tree: tree to update
	 * @images: images which are being booted
	 */
	struct event_ft_fixup {
		oftree tree;
		struct bootm_headers *images;
	} ft_fixup;
};

/**
 * struct event - an event that can be sent and received
 *
 * @type: Event type
 * @data: Data for this particular event
 */
struct event {
	enum event_t type;
	union event_data data;
};

/* Flags for event spy */
enum evspy_flags {
	EVSPYF_SIMPLE	= 1 << 0,
};

/** Function type for event handlers */
typedef int (*event_handler_t)(void *ctx, struct event *event);

/** Function type for simple event handlers */
typedef int (*event_handler_simple_t)(void);

/**
 * struct evspy_info - information about an event spy
 *
 * @func: Function to call when the event is activated (must be first)
 * @type: Event type
 * @flags: Flags for this spy
 * @id: Event id string
 */
struct evspy_info {
	event_handler_t func;
	u8 type;
	u8 flags;
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
	const char *id;
#endif
};

/**
 * struct evspy_info_simple - information about an event spy
 *
 * THis is the 'simple' record, the only difference being the handler function
 *
 * @func: Function to call when the event is activated (must be first)
 * @type: Event type
 * @flags: Flags for this spy
 * @id: Event id string
 */
struct evspy_info_simple {
	event_handler_simple_t func;
	u8 type;
	u8 flags;
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
	const char *id;
#endif
};

/* Declare a new event spy */
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
#define _ESPY_REC(_type, _func)   { _func, _type, 0, #_func, }
#define _ESPY_REC_SIMPLE(_type, _func)  { _func, _type, EVSPYF_SIMPLE, #_func, }
#else
#define _ESPY_REC(_type, _func)   { _func, _type, }
#define _ESPY_REC_SIMPLE(_type, _func)  { _func, _type, EVSPYF_SIMPLE }
#endif

static inline const char *event_spy_id(struct evspy_info *spy)
{
#if CONFIG_IS_ENABLED(EVENT_DEBUG)
	return spy->id;
#else
	return "?";
#endif
}

/*
 * It seems that LTO will drop list entries if it decides they are not used,
 * although the conditions that cause this are unclear.
 *
 * The example found is the following:
 *
 * static int sandbox_misc_init_f(void *ctx, struct event *event)
 * {
 *    return sandbox_early_getopt_check();
 * }
 * EVENT_SPY_FULL(EVT_MISC_INIT_F, sandbox_misc_init_f);
 *
 * where EVENT_SPY_FULL uses ll_entry_declare()
 *
 * In this case, LTO decides to drop the sandbox_misc_init_f() function
 * (which is fine) but then drops the linker-list entry too. This means
 * that the code no longer works, in this case sandbox no-longer checks its
 * command-line arguments properly.
 *
 * Without LTO, the KEEP() command in the .lds file is enough to keep the
 * entry around. But with LTO it seems that the entry has already been
 * dropped before the link script is considered.
 *
 * The only solution I can think of is to mark linker-list entries as 'used'
 * using an attribute. This should be safe, since we don't actually want to drop
 * any of these. However this does slightly limit LTO's optimisation choices.
 *
 * Another issue has come up, only with clang: using 'static' makes it throw
 * away the linker-list entry sometimes, e.g. with the EVT_FT_FIXUP entry in
 * vbe_simple.c - so for now, make it global.
 */
#define EVENT_SPY_FULL(_type, _func) \
	__used ll_entry_declare(struct evspy_info, _type ## _3_ ## _func, \
		evspy_info) = _ESPY_REC(_type, _func)

/* Simple spy with no function arguemnts */
#define EVENT_SPY_SIMPLE(_type, _func) \
	__used ll_entry_declare(struct evspy_info_simple, \
		_type ## _3_ ## _func, \
		evspy_info) = _ESPY_REC_SIMPLE(_type, _func)

/**
 * event_register - register a new spy
 *
 * @id: Spy ID
 * @type: Event type to subscribe to
 * @func: Function to call when the event is sent
 * @ctx: Context to pass to the function
 * @return 0 if OK, -ve on error
 */
int event_register(const char *id, enum event_t type, event_handler_t func,
		   void *ctx);

/** event_show_spy_list( - Show a list of event spies */
void event_show_spy_list(void);

/**
 * event_type_name() - Get the name of an event type
 *
 * @type: Type to check
 * Return: Name of event, or "(unknown)" if not known
 */
const char *event_type_name(enum event_t type);

/**
 * event_notify() - notify spies about an event
 *
 * It is possible to pass in union event_data here but that may not be
 * convenient if the data is elsewhere, or is one of the members of the union.
 * So this uses a void * for @data, with a separate @size.
 *
 * @type: Event type
 * @data: Event data to be sent (e.g. union_event_data)
 * @size: Size of data in bytes
 * @return 0 if OK, -ve on error
 */
int event_notify(enum event_t type, void *data, int size);

#if CONFIG_IS_ENABLED(EVENT)
/**
 * event_notify_null() - notify spies about an event
 *
 * Data is NULL and the size is 0
 *
 * @type: Event type
 * @return 0 if OK, -ve on error
 */
int event_notify_null(enum event_t type);
#else
static inline int event_notify_null(enum event_t type)
{
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(EVENT_DYNAMIC)
/**
 * event_uninit() - Clean up dynamic events
 *
 * This removes all dynamic event handlers
 */
int event_uninit(void);

/**
 * event_uninit() - Set up dynamic events
 *
 * Init a list of dynamic event handlers, so that these can be added as
 * needed
 */
int event_init(void);
#else
static inline int event_uninit(void)
{
	return 0;
}

static inline int event_init(void)
{
	return 0;
}
#endif

#endif
