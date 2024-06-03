// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-tegra/ivc.h>

#define TEGRA_IVC_ALIGN 64

/*
 * IVC channel reset protocol.
 *
 * Each end uses its tx_channel.state to indicate its synchronization state.
 */
enum ivc_state {
	/*
	 * This value is zero for backwards compatibility with services that
	 * assume channels to be initially zeroed. Such channels are in an
	 * initially valid state, but cannot be asynchronously reset, and must
	 * maintain a valid state at all times.
	 *
	 * The transmitting end can enter the established state from the sync or
	 * ack state when it observes the receiving endpoint in the ack or
	 * established state, indicating that has cleared the counters in our
	 * rx_channel.
	 */
	ivc_state_established = 0,

	/*
	 * If an endpoint is observed in the sync state, the remote endpoint is
	 * allowed to clear the counters it owns asynchronously with respect to
	 * the current endpoint. Therefore, the current endpoint is no longer
	 * allowed to communicate.
	 */
	ivc_state_sync,

	/*
	 * When the transmitting end observes the receiving end in the sync
	 * state, it can clear the w_count and r_count and transition to the ack
	 * state. If the remote endpoint observes us in the ack state, it can
	 * return to the established state once it has cleared its counters.
	 */
	ivc_state_ack
};

/*
 * This structure is divided into two-cache aligned parts, the first is only
 * written through the tx_channel pointer, while the second is only written
 * through the rx_channel pointer. This delineates ownership of the cache lines,
 * which is critical to performance and necessary in non-cache coherent
 * implementations.
 */
struct tegra_ivc_channel_header {
	union {
		/* fields owned by the transmitting end */
		struct {
			uint32_t w_count;
			uint32_t state;
		};
		uint8_t w_align[TEGRA_IVC_ALIGN];
	};
	union {
		/* fields owned by the receiving end */
		uint32_t r_count;
		uint8_t r_align[TEGRA_IVC_ALIGN];
	};
};

static inline void tegra_ivc_invalidate_counter(struct tegra_ivc *ivc,
					struct tegra_ivc_channel_header *h,
					ulong offset)
{
	ulong base = ((ulong)h) + offset;
	invalidate_dcache_range(base, base + TEGRA_IVC_ALIGN);
}

static inline void tegra_ivc_flush_counter(struct tegra_ivc *ivc,
					   struct tegra_ivc_channel_header *h,
					   ulong offset)
{
	ulong base = ((ulong)h) + offset;
	flush_dcache_range(base, base + TEGRA_IVC_ALIGN);
}

static inline ulong tegra_ivc_frame_addr(struct tegra_ivc *ivc,
					 struct tegra_ivc_channel_header *h,
					 uint32_t frame)
{
	BUG_ON(frame >= ivc->nframes);

	return ((ulong)h) + sizeof(struct tegra_ivc_channel_header) +
	       (ivc->frame_size * frame);
}

static inline void *tegra_ivc_frame_pointer(struct tegra_ivc *ivc,
					    struct tegra_ivc_channel_header *ch,
					    uint32_t frame)
{
	return (void *)tegra_ivc_frame_addr(ivc, ch, frame);
}

static inline void tegra_ivc_invalidate_frame(struct tegra_ivc *ivc,
					struct tegra_ivc_channel_header *h,
					unsigned frame)
{
	ulong base = tegra_ivc_frame_addr(ivc, h, frame);
	invalidate_dcache_range(base, base + ivc->frame_size);
}

static inline void tegra_ivc_flush_frame(struct tegra_ivc *ivc,
					 struct tegra_ivc_channel_header *h,
					 unsigned frame)
{
	ulong base = tegra_ivc_frame_addr(ivc, h, frame);
	flush_dcache_range(base, base + ivc->frame_size);
}

static inline int tegra_ivc_channel_empty(struct tegra_ivc *ivc,
					  struct tegra_ivc_channel_header *ch)
{
	/*
	 * This function performs multiple checks on the same values with
	 * security implications, so create snapshots with ACCESS_ONCE() to
	 * ensure that these checks use the same values.
	 */
	uint32_t w_count = ACCESS_ONCE(ch->w_count);
	uint32_t r_count = ACCESS_ONCE(ch->r_count);

	/*
	 * Perform an over-full check to prevent denial of service attacks where
	 * a server could be easily fooled into believing that there's an
	 * extremely large number of frames ready, since receivers are not
	 * expected to check for full or over-full conditions.
	 *
	 * Although the channel isn't empty, this is an invalid case caused by
	 * a potentially malicious peer, so returning empty is safer, because it
	 * gives the impression that the channel has gone silent.
	 */
	if (w_count - r_count > ivc->nframes)
		return 1;

	return w_count == r_count;
}

static inline int tegra_ivc_channel_full(struct tegra_ivc *ivc,
					 struct tegra_ivc_channel_header *ch)
{
	/*
	 * Invalid cases where the counters indicate that the queue is over
	 * capacity also appear full.
	 */
	return (ACCESS_ONCE(ch->w_count) - ACCESS_ONCE(ch->r_count)) >=
	       ivc->nframes;
}

static inline void tegra_ivc_advance_rx(struct tegra_ivc *ivc)
{
	ACCESS_ONCE(ivc->rx_channel->r_count) =
			ACCESS_ONCE(ivc->rx_channel->r_count) + 1;

	if (ivc->r_pos == ivc->nframes - 1)
		ivc->r_pos = 0;
	else
		ivc->r_pos++;
}

static inline void tegra_ivc_advance_tx(struct tegra_ivc *ivc)
{
	ACCESS_ONCE(ivc->tx_channel->w_count) =
			ACCESS_ONCE(ivc->tx_channel->w_count) + 1;

	if (ivc->w_pos == ivc->nframes - 1)
		ivc->w_pos = 0;
	else
		ivc->w_pos++;
}

static inline int tegra_ivc_check_read(struct tegra_ivc *ivc)
{
	ulong offset;

	/*
	 * tx_channel->state is set locally, so it is not synchronized with
	 * state from the remote peer. The remote peer cannot reset its
	 * transmit counters until we've acknowledged its synchronization
	 * request, so no additional synchronization is required because an
	 * asynchronous transition of rx_channel->state to ivc_state_ack is not
	 * allowed.
	 */
	if (ivc->tx_channel->state != ivc_state_established)
		return -ECONNRESET;

	/*
	 * Avoid unnecessary invalidations when performing repeated accesses to
	 * an IVC channel by checking the old queue pointers first.
	 * Synchronization is only necessary when these pointers indicate empty
	 * or full.
	 */
	if (!tegra_ivc_channel_empty(ivc, ivc->rx_channel))
		return 0;

	offset = offsetof(struct tegra_ivc_channel_header, w_count);
	tegra_ivc_invalidate_counter(ivc, ivc->rx_channel, offset);
	return tegra_ivc_channel_empty(ivc, ivc->rx_channel) ? -ENOMEM : 0;
}

static inline int tegra_ivc_check_write(struct tegra_ivc *ivc)
{
	ulong offset;

	if (ivc->tx_channel->state != ivc_state_established)
		return -ECONNRESET;

	if (!tegra_ivc_channel_full(ivc, ivc->tx_channel))
		return 0;

	offset = offsetof(struct tegra_ivc_channel_header, r_count);
	tegra_ivc_invalidate_counter(ivc, ivc->tx_channel, offset);
	return tegra_ivc_channel_full(ivc, ivc->tx_channel) ? -ENOMEM : 0;
}

static inline uint32_t tegra_ivc_channel_avail_count(struct tegra_ivc *ivc,
	struct tegra_ivc_channel_header *ch)
{
	/*
	 * This function isn't expected to be used in scenarios where an
	 * over-full situation can lead to denial of service attacks. See the
	 * comment in tegra_ivc_channel_empty() for an explanation about
	 * special over-full considerations.
	 */
	return ACCESS_ONCE(ch->w_count) - ACCESS_ONCE(ch->r_count);
}

int tegra_ivc_read_get_next_frame(struct tegra_ivc *ivc, void **frame)
{
	int result = tegra_ivc_check_read(ivc);
	if (result < 0)
		return result;

	/*
	 * Order observation of w_pos potentially indicating new data before
	 * data read.
	 */
	mb();

	tegra_ivc_invalidate_frame(ivc, ivc->rx_channel, ivc->r_pos);
	*frame = tegra_ivc_frame_pointer(ivc, ivc->rx_channel, ivc->r_pos);

	return 0;
}

int tegra_ivc_read_advance(struct tegra_ivc *ivc)
{
	ulong offset;
	int result;

	/*
	 * No read barriers or synchronization here: the caller is expected to
	 * have already observed the channel non-empty. This check is just to
	 * catch programming errors.
	 */
	result = tegra_ivc_check_read(ivc);
	if (result)
		return result;

	tegra_ivc_advance_rx(ivc);
	offset = offsetof(struct tegra_ivc_channel_header, r_count);
	tegra_ivc_flush_counter(ivc, ivc->rx_channel, offset);

	/*
	 * Ensure our write to r_pos occurs before our read from w_pos.
	 */
	mb();

	offset = offsetof(struct tegra_ivc_channel_header, w_count);
	tegra_ivc_invalidate_counter(ivc, ivc->rx_channel, offset);

	if (tegra_ivc_channel_avail_count(ivc, ivc->rx_channel) ==
	    ivc->nframes - 1)
		ivc->notify(ivc);

	return 0;
}

int tegra_ivc_write_get_next_frame(struct tegra_ivc *ivc, void **frame)
{
	int result = tegra_ivc_check_write(ivc);
	if (result)
		return result;

	*frame = tegra_ivc_frame_pointer(ivc, ivc->tx_channel, ivc->w_pos);

	return 0;
}

int tegra_ivc_write_advance(struct tegra_ivc *ivc)
{
	ulong offset;
	int result;

	result = tegra_ivc_check_write(ivc);
	if (result)
		return result;

	tegra_ivc_flush_frame(ivc, ivc->tx_channel, ivc->w_pos);

	/*
	 * Order any possible stores to the frame before update of w_pos.
	 */
	mb();

	tegra_ivc_advance_tx(ivc);
	offset = offsetof(struct tegra_ivc_channel_header, w_count);
	tegra_ivc_flush_counter(ivc, ivc->tx_channel, offset);

	/*
	 * Ensure our write to w_pos occurs before our read from r_pos.
	 */
	mb();

	offset = offsetof(struct tegra_ivc_channel_header, r_count);
	tegra_ivc_invalidate_counter(ivc, ivc->tx_channel, offset);

	if (tegra_ivc_channel_avail_count(ivc, ivc->tx_channel) == 1)
		ivc->notify(ivc);

	return 0;
}

/*
 * ===============================================================
 *  IVC State Transition Table - see tegra_ivc_channel_notified()
 * ===============================================================
 *
 *	local	remote	action
 *	-----	------	-----------------------------------
 *	SYNC	EST	<none>
 *	SYNC	ACK	reset counters; move to EST; notify
 *	SYNC	SYNC	reset counters; move to ACK; notify
 *	ACK	EST	move to EST; notify
 *	ACK	ACK	move to EST; notify
 *	ACK	SYNC	reset counters; move to ACK; notify
 *	EST	EST	<none>
 *	EST	ACK	<none>
 *	EST	SYNC	reset counters; move to ACK; notify
 *
 * ===============================================================
 */
int tegra_ivc_channel_notified(struct tegra_ivc *ivc)
{
	ulong offset;
	enum ivc_state peer_state;

	/* Copy the receiver's state out of shared memory. */
	offset = offsetof(struct tegra_ivc_channel_header, w_count);
	tegra_ivc_invalidate_counter(ivc, ivc->rx_channel, offset);
	peer_state = ACCESS_ONCE(ivc->rx_channel->state);

	if (peer_state == ivc_state_sync) {
		/*
		 * Order observation of ivc_state_sync before stores clearing
		 * tx_channel.
		 */
		mb();

		/*
		 * Reset tx_channel counters. The remote end is in the SYNC
		 * state and won't make progress until we change our state,
		 * so the counters are not in use at this time.
		 */
		ivc->tx_channel->w_count = 0;
		ivc->rx_channel->r_count = 0;

		ivc->w_pos = 0;
		ivc->r_pos = 0;

		/*
		 * Ensure that counters appear cleared before new state can be
		 * observed.
		 */
		mb();

		/*
		 * Move to ACK state. We have just cleared our counters, so it
		 * is now safe for the remote end to start using these values.
		 */
		ivc->tx_channel->state = ivc_state_ack;
		offset = offsetof(struct tegra_ivc_channel_header, w_count);
		tegra_ivc_flush_counter(ivc, ivc->tx_channel, offset);

		/*
		 * Notify remote end to observe state transition.
		 */
		ivc->notify(ivc);
	} else if (ivc->tx_channel->state == ivc_state_sync &&
			peer_state == ivc_state_ack) {
		/*
		 * Order observation of ivc_state_sync before stores clearing
		 * tx_channel.
		 */
		mb();

		/*
		 * Reset tx_channel counters. The remote end is in the ACK
		 * state and won't make progress until we change our state,
		 * so the counters are not in use at this time.
		 */
		ivc->tx_channel->w_count = 0;
		ivc->rx_channel->r_count = 0;

		ivc->w_pos = 0;
		ivc->r_pos = 0;

		/*
		 * Ensure that counters appear cleared before new state can be
		 * observed.
		 */
		mb();

		/*
		 * Move to ESTABLISHED state. We know that the remote end has
		 * already cleared its counters, so it is safe to start
		 * writing/reading on this channel.
		 */
		ivc->tx_channel->state = ivc_state_established;
		offset = offsetof(struct tegra_ivc_channel_header, w_count);
		tegra_ivc_flush_counter(ivc, ivc->tx_channel, offset);

		/*
		 * Notify remote end to observe state transition.
		 */
		ivc->notify(ivc);
	} else if (ivc->tx_channel->state == ivc_state_ack) {
		/*
		 * At this point, we have observed the peer to be in either
		 * the ACK or ESTABLISHED state. Next, order observation of
		 * peer state before storing to tx_channel.
		 */
		mb();

		/*
		 * Move to ESTABLISHED state. We know that we have previously
		 * cleared our counters, and we know that the remote end has
		 * cleared its counters, so it is safe to start writing/reading
		 * on this channel.
		 */
		ivc->tx_channel->state = ivc_state_established;
		offset = offsetof(struct tegra_ivc_channel_header, w_count);
		tegra_ivc_flush_counter(ivc, ivc->tx_channel, offset);

		/*
		 * Notify remote end to observe state transition.
		 */
		ivc->notify(ivc);
	} else {
		/*
		 * There is no need to handle any further action. Either the
		 * channel is already fully established, or we are waiting for
		 * the remote end to catch up with our current state. Refer
		 * to the diagram in "IVC State Transition Table" above.
		 */
	}

	if (ivc->tx_channel->state != ivc_state_established)
		return -EAGAIN;

	return 0;
}

void tegra_ivc_channel_reset(struct tegra_ivc *ivc)
{
	ulong offset;

	ivc->tx_channel->state = ivc_state_sync;
	offset = offsetof(struct tegra_ivc_channel_header, w_count);
	tegra_ivc_flush_counter(ivc, ivc->tx_channel, offset);
	ivc->notify(ivc);
}

static int check_ivc_params(ulong qbase1, ulong qbase2, uint32_t nframes,
			    uint32_t frame_size)
{
	int ret = 0;

	BUG_ON(offsetof(struct tegra_ivc_channel_header, w_count) &
	       (TEGRA_IVC_ALIGN - 1));
	BUG_ON(offsetof(struct tegra_ivc_channel_header, r_count) &
	       (TEGRA_IVC_ALIGN - 1));
	BUG_ON(sizeof(struct tegra_ivc_channel_header) &
	       (TEGRA_IVC_ALIGN - 1));

	if ((uint64_t)nframes * (uint64_t)frame_size >= 0x100000000) {
		pr_err("tegra_ivc: nframes * frame_size overflows\n");
		return -EINVAL;
	}

	/*
	 * The headers must at least be aligned enough for counters
	 * to be accessed atomically.
	 */
	if ((qbase1 & (TEGRA_IVC_ALIGN - 1)) ||
	    (qbase2 & (TEGRA_IVC_ALIGN - 1))) {
		pr_err("tegra_ivc: channel start not aligned\n");
		return -EINVAL;
	}

	if (frame_size & (TEGRA_IVC_ALIGN - 1)) {
		pr_err("tegra_ivc: frame size not adequately aligned\n");
		return -EINVAL;
	}

	if (qbase1 < qbase2) {
		if (qbase1 + frame_size * nframes > qbase2)
			ret = -EINVAL;
	} else {
		if (qbase2 + frame_size * nframes > qbase1)
			ret = -EINVAL;
	}

	if (ret) {
		pr_err("tegra_ivc: queue regions overlap\n");
		return ret;
	}

	return 0;
}

int tegra_ivc_init(struct tegra_ivc *ivc, ulong rx_base, ulong tx_base,
		   uint32_t nframes, uint32_t frame_size,
		   void (*notify)(struct tegra_ivc *))
{
	int ret;

	if (!ivc)
		return -EINVAL;

	ret = check_ivc_params(rx_base, tx_base, nframes, frame_size);
	if (ret)
		return ret;

	ivc->rx_channel = (struct tegra_ivc_channel_header *)rx_base;
	ivc->tx_channel = (struct tegra_ivc_channel_header *)tx_base;
	ivc->w_pos = 0;
	ivc->r_pos = 0;
	ivc->nframes = nframes;
	ivc->frame_size = frame_size;
	ivc->notify = notify;

	return 0;
}
