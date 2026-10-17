// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#define APPLE_RTKIT_PWR_STATE_SLEEP	0x01
#define APPLE_RTKIT_PWR_STATE_QUIESCED	0x10
#define APPLE_RTKIT_PWR_STATE_ON	0x20

struct apple_rtkit_buffer {
	void *buffer;
	u64 dva;
	size_t size;
	bool is_mapped;
};

typedef int (*apple_rtkit_shmem_setup)(void *cookie,
				       struct apple_rtkit_buffer *buf);
typedef void (*apple_rtkit_shmem_destroy)(void *cookie,
					  struct apple_rtkit_buffer *buf);

struct apple_rtkit;

struct apple_rtkit *apple_rtkit_init(struct mbox_chan *chan, void *cookie,
				     apple_rtkit_shmem_setup shmem_setup,
				     apple_rtkit_shmem_destroy shmem_destroy);
void apple_rtkit_free(struct apple_rtkit *rtk);
int apple_rtkit_boot(struct apple_rtkit *rtk);
int apple_rtkit_shutdown(struct apple_rtkit *rtk, int pwrstate);
