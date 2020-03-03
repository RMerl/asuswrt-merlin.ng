#undef TRACE_SYSTEM
#define TRACE_SYSTEM fence

#if !defined(_TRACE_FENCE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_FENCE_H

#include <linux/tracepoint.h>

struct fence;

TRACE_EVENT(fence_annotate_wait_on,

	/* fence: the fence waiting on f1, f1: the fence to be waited on. */
	TP_PROTO(struct fence *fence, struct fence *f1),

	TP_ARGS(fence, f1),

	TP_STRUCT__entry(
		__string(driver, fence->ops->get_driver_name(fence))
		__string(timeline, fence->ops->get_driver_name(fence))
		__field(unsigned int, context)
		__field(unsigned int, seqno)

		__string(waiting_driver, f1->ops->get_driver_name(f1))
		__string(waiting_timeline, f1->ops->get_timeline_name(f1))
		__field(unsigned int, waiting_context)
		__field(unsigned int, waiting_seqno)
	),

	TP_fast_assign(
		__assign_str(driver, fence->ops->get_driver_name(fence))
		__assign_str(timeline, fence->ops->get_timeline_name(fence))
		__entry->context = fence->context;
		__entry->seqno = fence->seqno;

		__assign_str(waiting_driver, f1->ops->get_driver_name(f1))
		__assign_str(waiting_timeline, f1->ops->get_timeline_name(f1))
		__entry->waiting_context = f1->context;
		__entry->waiting_seqno = f1->seqno;

	),

	TP_printk("driver=%s timeline=%s context=%u seqno=%u "	\
		  "waits on driver=%s timeline=%s context=%u seqno=%u",
		  __get_str(driver), __get_str(timeline), __entry->context,
		  __entry->seqno,
		  __get_str(waiting_driver), __get_str(waiting_timeline),
		  __entry->waiting_context, __entry->waiting_seqno)
);

DECLARE_EVENT_CLASS(fence,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence),

	TP_STRUCT__entry(
		__string(driver, fence->ops->get_driver_name(fence))
		__string(timeline, fence->ops->get_timeline_name(fence))
		__field(unsigned int, context)
		__field(unsigned int, seqno)
	),

	TP_fast_assign(
		__assign_str(driver, fence->ops->get_driver_name(fence))
		__assign_str(timeline, fence->ops->get_timeline_name(fence))
		__entry->context = fence->context;
		__entry->seqno = fence->seqno;
	),

	TP_printk("driver=%s timeline=%s context=%u seqno=%u",
		  __get_str(driver), __get_str(timeline), __entry->context,
		  __entry->seqno)
);

DEFINE_EVENT(fence, fence_emit,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

DEFINE_EVENT(fence, fence_init,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

DEFINE_EVENT(fence, fence_destroy,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

DEFINE_EVENT(fence, fence_enable_signal,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

DEFINE_EVENT(fence, fence_signaled,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

DEFINE_EVENT(fence, fence_wait_start,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

DEFINE_EVENT(fence, fence_wait_end,

	TP_PROTO(struct fence *fence),

	TP_ARGS(fence)
);

#endif /*  _TRACE_FENCE_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
