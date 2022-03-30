// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <os.h>
#include <asm/state.h>

/* Main state record for the sandbox */
static struct sandbox_state main_state;
static struct sandbox_state *state;	/* Pointer to current state record */

static int state_ensure_space(int extra_size)
{
	void *blob = state->state_fdt;
	int used, size, free;
	void *buf;
	int ret;

	used = fdt_off_dt_strings(blob) + fdt_size_dt_strings(blob);
	size = fdt_totalsize(blob);
	free = size - used;
	if (free > extra_size)
		return 0;

	size = used + extra_size;
	buf = os_malloc(size);
	if (!buf)
		return -ENOMEM;

	ret = fdt_open_into(blob, buf, size);
	if (ret) {
		os_free(buf);
		return -EIO;
	}

	os_free(blob);
	state->state_fdt = buf;
	return 0;
}

static int state_read_file(struct sandbox_state *state, const char *fname)
{
	loff_t size;
	int ret;
	int fd;

	ret = os_get_filesize(fname, &size);
	if (ret < 0) {
		printf("Cannot find sandbox state file '%s'\n", fname);
		return -ENOENT;
	}
	state->state_fdt = os_malloc(size);
	if (!state->state_fdt) {
		puts("No memory to read sandbox state\n");
		return -ENOMEM;
	}
	fd = os_open(fname, OS_O_RDONLY);
	if (fd < 0) {
		printf("Cannot open sandbox state file '%s'\n", fname);
		ret = -EPERM;
		goto err_open;
	}
	if (os_read(fd, state->state_fdt, size) != size) {
		printf("Cannot read sandbox state file '%s'\n", fname);
		ret = -EIO;
		goto err_read;
	}
	os_close(fd);

	return 0;
err_read:
	os_close(fd);
err_open:
	os_free(state->state_fdt);
	state->state_fdt = NULL;

	return ret;
}

/***
 * sandbox_read_state_nodes() - Read state associated with a driver
 *
 * This looks through all compatible nodes and calls the read function on
 * each one, to read in the state.
 *
 * If nothing is found, it still calls the read function once, to set up a
 * single global state for that driver.
 *
 * @state: Sandbox state
 * @io: Method to use for reading state
 * @blob: FDT containing state
 * @return 0 if OK, -EINVAL if the read function returned failure
 */
int sandbox_read_state_nodes(struct sandbox_state *state,
			     struct sandbox_state_io *io, const void *blob)
{
	int count;
	int node;
	int ret;

	debug("   - read %s\n", io->name);
	if (!io->read)
		return 0;

	node = -1;
	count = 0;
	while (blob) {
		node = fdt_node_offset_by_compatible(blob, node, io->compat);
		if (node < 0)
			return 0;	/* No more */
		debug("   - read node '%s'\n", fdt_get_name(blob, node, NULL));
		ret = io->read(blob, node);
		if (ret) {
			printf("Unable to read state for '%s'\n", io->compat);
			return -EINVAL;
		}
		count++;
	}

	/*
	 * If we got no saved state, call the read function once without a
	 * node, to set up the global state.
	 */
	if (count == 0) {
		debug("   - read global\n");
		ret = io->read(NULL, -1);
		if (ret) {
			printf("Unable to read global state for '%s'\n",
			       io->name);
			return -EINVAL;
		}
	}

	return 0;
}

int sandbox_read_state(struct sandbox_state *state, const char *fname)
{
	struct sandbox_state_io *io;
	const void *blob;
	bool got_err;
	int ret;

	if (state->read_state && fname) {
		ret = state_read_file(state, fname);
		if (ret == -ENOENT && state->ignore_missing_state_on_read)
			ret = 0;
		if (ret)
			return ret;
	}

	/* Call all the state read functions */
	got_err = false;
	blob = state->state_fdt;
	io = ll_entry_start(struct sandbox_state_io, state_io);
	for (; io < ll_entry_end(struct sandbox_state_io, state_io); io++) {
		ret = sandbox_read_state_nodes(state, io, blob);
		if (ret < 0)
			got_err = true;
	}

	if (state->read_state && fname) {
		debug("Read sandbox state from '%s'%s\n", fname,
		      got_err ? " (with errors)" : "");
	}

	return got_err ? -1 : 0;
}

/***
 * sandbox_write_state_node() - Write state associated with a driver
 *
 * This calls the write function to write out global state for that driver.
 *
 * TODO(sjg@chromium.org): Support writing out state from multiple drivers
 * of the same time. We don't need this yet,and it will be much easier to
 * do when driver model is available.
 *
 * @state: Sandbox state
 * @io: Method to use for writing state
 * @return 0 if OK, -EIO if there is a fatal error (such as out of space
 * for adding the data), -EINVAL if the write function failed.
 */
int sandbox_write_state_node(struct sandbox_state *state,
			     struct sandbox_state_io *io)
{
	void *blob;
	int node;
	int ret;

	if (!io->write)
		return 0;

	ret = state_ensure_space(SANDBOX_STATE_MIN_SPACE);
	if (ret) {
		printf("Failed to add more space for state\n");
		return -EIO;
	}

	/* The blob location can change when the size increases */
	blob = state->state_fdt;
	node = fdt_node_offset_by_compatible(blob, -1, io->compat);
	if (node == -FDT_ERR_NOTFOUND) {
		node = fdt_add_subnode(blob, 0, io->name);
		if (node < 0) {
			printf("Cannot create node '%s': %s\n", io->name,
			       fdt_strerror(node));
			return -EIO;
		}

		if (fdt_setprop_string(blob, node, "compatible", io->compat)) {
			puts("Cannot set compatible\n");
			return -EIO;
		}
	} else if (node < 0) {
		printf("Cannot access node '%s': %s\n", io->name,
		       fdt_strerror(node));
		return -EIO;
	}
	debug("Write state for '%s' to node %d\n", io->compat, node);
	ret = io->write(blob, node);
	if (ret) {
		printf("Unable to write state for '%s'\n", io->compat);
		return -EINVAL;
	}

	return 0;
}

int sandbox_write_state(struct sandbox_state *state, const char *fname)
{
	struct sandbox_state_io *io;
	bool got_err;
	int size;
	int ret;
	int fd;

	/* Create a state FDT if we don't have one */
	if (!state->state_fdt) {
		size = 0x4000;
		state->state_fdt = os_malloc(size);
		if (!state->state_fdt) {
			puts("No memory to create FDT\n");
			return -ENOMEM;
		}
		ret = fdt_create_empty_tree(state->state_fdt, size);
		if (ret < 0) {
			printf("Cannot create empty state FDT: %s\n",
			       fdt_strerror(ret));
			ret = -EIO;
			goto err_create;
		}
	}

	/* Call all the state write funtcions */
	got_err = false;
	io = ll_entry_start(struct sandbox_state_io, state_io);
	ret = 0;
	for (; io < ll_entry_end(struct sandbox_state_io, state_io); io++) {
		ret = sandbox_write_state_node(state, io);
		if (ret == -EIO)
			break;
		else if (ret)
			got_err = true;
	}

	if (ret == -EIO) {
		printf("Could not write sandbox state\n");
		goto err_create;
	}

	ret = fdt_pack(state->state_fdt);
	if (ret < 0) {
		printf("Cannot pack state FDT: %s\n", fdt_strerror(ret));
		ret = -EINVAL;
		goto err_create;
	}
	size = fdt_totalsize(state->state_fdt);
	fd = os_open(fname, OS_O_WRONLY | OS_O_CREAT);
	if (fd < 0) {
		printf("Cannot open sandbox state file '%s'\n", fname);
		ret = -EIO;
		goto err_create;
	}
	if (os_write(fd, state->state_fdt, size) != size) {
		printf("Cannot write sandbox state file '%s'\n", fname);
		ret = -EIO;
		goto err_write;
	}
	os_close(fd);

	debug("Wrote sandbox state to '%s'%s\n", fname,
	      got_err ? " (with errors)" : "");

	return 0;
err_write:
	os_close(fd);
err_create:
	os_free(state->state_fdt);

	return ret;
}

int state_setprop(int node, const char *prop_name, const void *data, int size)
{
	void *blob;
	int len;
	int ret;

	fdt_getprop(state->state_fdt, node, prop_name, &len);

	/* Add space for the new property, its name and some overhead */
	ret = state_ensure_space(size - len + strlen(prop_name) + 32);
	if (ret)
		return ret;

	/* This should succeed, barring a mutiny */
	blob = state->state_fdt;
	ret = fdt_setprop(blob, node, prop_name, data, size);
	if (ret) {
		printf("%s: Unable to set property '%s' in node '%s': %s\n",
		       __func__, prop_name, fdt_get_name(blob, node, NULL),
			fdt_strerror(ret));
		return -ENOSPC;
	}

	return 0;
}

struct sandbox_state *state_get_current(void)
{
	assert(state);
	return state;
}

void state_set_skip_delays(bool skip_delays)
{
	struct sandbox_state *state = state_get_current();

	state->skip_delays = skip_delays;
}

bool state_get_skip_delays(void)
{
	struct sandbox_state *state = state_get_current();

	return state->skip_delays;
}

void state_reset_for_test(struct sandbox_state *state)
{
	/* No reset yet, so mark it as such. Always allow power reset */
	state->last_sysreset = SYSRESET_COUNT;
	state->sysreset_allowed[SYSRESET_POWER] = true;

	memset(&state->wdt, '\0', sizeof(state->wdt));
	memset(state->spi, '\0', sizeof(state->spi));

	/*
	 * Set up the memory tag list. Use the top of emulated SDRAM for the
	 * first tag number, since that address offset is outside the legal
	 * range, and can be assumed to be a tag.
	 */
	INIT_LIST_HEAD(&state->mapmem_head);
	state->next_tag = state->ram_size;
}

int state_init(void)
{
	state = &main_state;

	state->ram_size = CONFIG_SYS_SDRAM_SIZE;
	state->ram_buf = os_malloc(state->ram_size);
	assert(state->ram_buf);

	state_reset_for_test(state);
	/*
	 * Example of how to use GPIOs:
	 *
	 * sandbox_gpio_set_direction(170, 0);
	 * sandbox_gpio_set_value(170, 0);
	 */
	return 0;
}

int state_uninit(void)
{
	int err;

	state = &main_state;

	if (state->write_ram_buf) {
		err = os_write_ram_buf(state->ram_buf_fname);
		if (err) {
			printf("Failed to write RAM buffer\n");
			return err;
		}
	}

	if (state->write_state) {
		if (sandbox_write_state(state, state->state_fname)) {
			printf("Failed to write sandbox state\n");
			return -1;
		}
	}

	/* Remove old memory file if required */
	if (state->ram_buf_rm && state->ram_buf_fname)
		os_unlink(state->ram_buf_fname);

	/* Delete this at the last moment so as not to upset gdb too much */
	if (state->jumped_fname)
		os_unlink(state->jumped_fname);

	if (state->state_fdt)
		os_free(state->state_fdt);
	memset(state, '\0', sizeof(*state));

	return 0;
}
