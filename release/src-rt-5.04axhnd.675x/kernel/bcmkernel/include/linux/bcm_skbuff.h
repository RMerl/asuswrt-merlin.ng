#ifndef _BCM_SKBUFF_H
#define _BCM_SKBUFF_H

#include <linux/net.h>

#define CONFIG_SKBSHINFO_HAS_DIRTYP	1

struct blog_t; /* defined(CONFIG_BLOG) */
struct net_device;

#ifndef NULL_STMT
#define NULL_STMT		do { /* NULL BODY */ } while (0)
#endif

typedef void (*RecycleFuncP)(void *nbuff_p, unsigned long context, uint32_t flags);

#define SKB_DATA_RECYCLE	(1 << 0)
#define SKB_DATA_NO_RECYCLE	(~SKB_DATA_RECYCLE)
#define SKB_RECYCLE		(1 << 1)
#define SKB_NO_RECYCLE		(~SKB_RECYCLE)
#define SKB_RECYCLE_NOFREE	(1 << 2) /* DO NOT USE */
#define SKB_RECYCLE_FPM_DATA	(1 << 3) /* Data buffer from Runner FPM pool */
#define SKB_RNR_FLOOD		(1 << 4) /* Data buffer flooded by Runner to flooding-capable ports */
/* Indicates whether a sk_buf or a data buffer is in BPM pristine state */
#define SKB_BPM_PRISTINE	(1 << 5)
/* UDP Speed Test flags */
#define SKB_RNR_UDPSPDT_BASIC	(1 << 6)
#define SKB_RNR_UDPSPDT_IPERF3	(1 << 7)

#define SKB_RNR_FLAGS		(SKB_RNR_FLOOD | SKB_RNR_UDPSPDT_BASIC | SKB_RNR_UDPSPDT_IPERF3)
#define SKB_RNR_UDPSPDT_FLAGS	(SKB_RNR_UDPSPDT_BASIC | SKB_RNR_UDPSPDT_IPERF3)

#define SKB_BPM_TAINTED(skb)						\
({									\
	((struct sk_buff *)skb)->recycle_flags &= ~SKB_BPM_PRISTINE;	\
	(skb_shinfo(skb))->dirty_p = NULL;				\
})


#define SKB_DATA_PRISTINE(skb)						\
({									\
	(skb_shinfo(skb))->dirty_p = ((struct sk_buff *)skb)->head;	\
})

struct fkbuff;

extern void skb_frag_xmit4(struct sk_buff *origskb, struct net_device *txdev,
			   uint32_t is_pppoe, uint32_t minMtu, void *ip_p);
extern void skb_frag_xmit6(struct sk_buff *origskb, struct net_device *txdev,
			   uint32_t is_pppoe, uint32_t minMtu, void *ip_p);
extern struct sk_buff *skb_xlate(struct fkbuff *fkb_p);
extern struct sk_buff *skb_xlate_dp(struct fkbuff *fkb_p, uint8_t *dirty_p);
extern int skb_avail_headroom(const struct sk_buff *skb);
extern void skb_bpm_tainted(struct sk_buff *skb);

extern int skb_avail_headroom(const struct sk_buff *skb);

extern void skb_cb_zero(struct sk_buff *skb);

extern size_t skb_size(void);
extern size_t skb_aligned_size(void);
extern int skb_layout_test(int head_offset, int tail_offset, int end_offset);

/**
 *	skb_headerinit	-	initialize a socket buffer header
 *	@headroom: reserved headroom size
 *	@datalen: data buffer size, data buffer is allocated by caller
 *	@skb: skb allocated by caller
 *	@data: data buffer allocated by caller
 *	@recycle_hook: callback function to free data buffer and skb
 *	@recycle_context: context value passed to recycle_hook, param1
 *  @blog_p: pass a blog to a skb for logging
 *
 *	Initializes the socket buffer and assigns the data buffer to it.
 *	Both the sk_buff and the pointed data buffer are pre-allocated.
 *
 */
void skb_headerinit(unsigned int headroom, unsigned int datalen,
		    struct sk_buff *skb, unsigned char *data,
		    RecycleFuncP recycle_hook, unsigned long recycle_context,
		    struct blog_t *blog_p);


/* TODO avoid this detail here, nbuff/skbuff should just define this as
 * uint32_t and wl driver should cast this to appropriate structure
 */
typedef union wlFlowInf {
	uint32_t u32;
	union {
		union {
			struct {
				/* Start - Shared fields between ucast and mcast */
				uint32_t is_ucast:1;
				/* wl_prio is 4 bits for nic and 3 bits for dhd. Plan is
				 * to make NIC as 3 bits after more analysis */
				uint32_t wl_prio:4;
				/* End - Shared fields between ucast and mcast */
				uint32_t nic_reserved1:11;
				uint32_t wl_chainidx:16;
			};
			struct {
				uint32_t overlayed_field:16;
				uint32_t ssid_dst:16; /* For bridged traffic we don't have chainidx (0xFE) */
			};
		} nic;

		struct {
			/* Start - Shared fields between ucast and mcast */
			uint32_t is_ucast:1;
			uint32_t wl_prio:4;
			/* End - Shared fields between ucast and mcast */
			/* Start - Shared fields between dhd ucast and dhd mcast */
			uint32_t flowring_idx:10;
			/* End - Shared fields between dhd ucast and dhd mcast */
			uint32_t dhd_reserved:13;
			uint32_t ssid:4;
		} dhd;
	} ucast;
	struct {
		/* Start - Shared fields between ucast and mcast */
		/* for multicast, WFD does not need to populate this flowring_idx, it is used internally by dhd driver */
		uint32_t is_ucast:1;
		uint32_t wl_prio:4;
		/* End - Shared fields between ucast and mcast */
		/* Start - Shared fields between dhd ucast and dhd mcast */
		uint32_t flowring_idx:10;
		/* End - Shared fields between dhd ucast and dhd mcast */
		uint32_t mcast_reserved:1;
		uint32_t ssid_vector:16;
	} mcast;

	struct {
		/* Start - Shared fields b/w ucast, mcast & pktfwd */
		uint32_t is_ucast           : 1;    /* Start - Shared fields b/w ucast, mcast */
		uint32_t wl_prio            : 4;    /* packet priority */
		/* End - Shared fields between ucast, mcast & pktfwd */
		uint32_t pktfwd_reserved    : 7;
		uint32_t ssid               : 4;
		uint32_t pktfwd_key         : 16;   /* pktfwd_key_t : 2b domain, 2b incarn, 12b index */
	} pktfwd;
} wlFlowInf_t;

struct wlan_ext {

	union {
		__u32 wl_cb[6];
		struct {
			/* pktc_cb should hold space for void* and unsigned int */
			unsigned char	pktc_cb[16];
			__u16		pktc_flags; /* wl_flags */
			__u16		dma_index; /* used by HND router for NIC Bulk Tx */
			__u8		wl_flag1;  /* used for blog handle, only need one bit for now */
			__u8		wl_rsvd;
			__u16		wl_flowid; /* cfp flowid */
		};
	} __aligned(8);
};

#define SKB_VLAN_MAX_TAGS	4

struct vlan_ext {
	union {
		struct {
			__u32	reserved:31;
			__u32	restore_rx_vlan:1; /* Restore Rx VLAN at xmit. Used in ONT mode */
		};
		__u32		bcm_flags_word;
	} bcm_flags;
	__u16			vlan_count;
	__u16			vlan_tpid;
	__u32			cfi_save;
	__u32			vlan_header[SKB_VLAN_MAX_TAGS];
	struct net_device	*rxdev;
};

#define MAP_FORWARD_NONE	0
#define MAP_FORWARD_MODE1	1
#define MAP_FORWARD_MODE2	2
#define MAP_FORWARD_MODE3	3 /* MAP-E Pre-Fragmentation */

struct map_ext {
	__u8			map_forward:2;
	__u8			map_mf:1;
	__u32			map_offset;
	__u32			map_id;
};

struct spdt_ext {
    uint32_t so_mark;
};

struct bcm_skb_ext {
	struct wlan_ext wlan;
	struct vlan_ext vlan;
	struct map_ext map;
	struct spdt_ext spdt;

	void *tunl; /* used to store tunl pointer */
	union {
		__u32 flags;
		struct {
			__u32	reserved:31;
			__u32	skb_fc_accel:1;/* fcache accelerated skb */
		};
	};

	unsigned char		*clone_wr_head; /* indicates drivers(ex:enet)about writable headroom in aggregated skb */
	unsigned char		*clone_fc_head; /* indicates fcache about writable headroom in aggregated skb */

	struct net_device	*in_dev; /* Physical device where this pkt is received */

	void*			sgs_conn; /* pointing to the sgs connection tracking object */
	struct nf_queue_entry	*q_entry; /* sgs packet queue to reinject to the stack */
};

/* accessor macro */
#define skbuff_bcm_ext_wlan_get(_skb, _field)	((_skb)->bcm_ext.wlan._field)
#define skbuff_bcm_ext_vlan_get(_skb, _field)	((_skb)->bcm_ext.vlan._field)
#define skbuff_bcm_ext_map_get(_skb, _field)	((_skb)->bcm_ext.map._field)
#define skbuff_bcm_ext_sgs_conn_get(_skb)	((_skb)->bcm_ext.sgs_conn)
#define skbuff_bcm_ext_q_entry_get(_skb)	((_skb)->bcm_ext.q_entry)
#define skbuff_bcm_ext_indev_get(_skb)			((_skb)->bcm_ext.in_dev)
#define skbuff_bcm_ext_indev_set(_skb, _dev)	((_skb)->bcm_ext.in_dev = _dev)
#define skbuff_bcm_ext_spdt_get(_skb, _field)	((_skb)->bcm_ext.spdt._field)
#define skbuff_bcm_ext_spdt_set(_skb, _field, _val)	((_skb)->bcm_ext.spdt._field = _val)
#define skbuff_bcm_ext_sgs_conn_set(_skb, _val)	((_skb)->bcm_ext.sgs_conn = _val)
#define skbuff_bcm_ext_q_entry_set(_skb, _val)	((_skb)->bcm_ext.q_entry = _val)

void bcm_skbuff_copy_skb_header(struct sk_buff *new, const struct sk_buff *old);
void bcm_skbuff_skb_clone(struct sk_buff *n, struct sk_buff *skb);
void bcm_skbuff_handle_netif_rx_internal(struct sk_buff *skb);
void bcm_skbuff_handle_netif_receive_skb_core(struct sk_buff *skb);

#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
int bcm_hook_br_handle_frame_finish(struct sk_buff *skb, int state);
int bcm_hook_br_should_deliver(struct sk_buff *skb, int state);
#else /* !CONFIG_BCM_WLAN */
#define bcm_hook_br_handle_frame_finish(skb, state)	0
#define bcm_hook_br_should_deliver(skb, state)		0
#endif /* !CONFIG_BCM_WLAN */

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
void skb_clone_headers_set(struct sk_buff *skb, unsigned int len);
unsigned int skb_writable_headroom(const struct sk_buff *skb);
#endif
void skb_header_free(struct sk_buff *skb);
struct sk_buff *skb_header_alloc(void);

void skb_shinforeset(struct skb_shared_info *skb_shinfo);

/* 
 * sk_buff structure is copied directly from skbuff.h and removing any #ifdef except CONFIG_BCM_KF_NBUFF.
 * This is on-purpose to solve binary incompatibility issue.
 */

#if defined(CONFIG_BCM_KF_MPTCP)
#define BCM_SKB_CB_SIZE		80
#else
#define BCM_SKB_CB_SIZE		48
#endif

struct sk_buff {
	union {
		struct {
			/* These two members must be first. */
			struct sk_buff		*next;
			struct sk_buff		*prev;

			union {
				struct net_device	*dev;
				/* Some protocols might use this space to store information,
				 * while device pointer would be NULL.
				 * UDP receive path is one user.
				 */
				unsigned long		dev_scratch;
			};
		};
		struct rb_node		rbnode; /* used in netem, ip4 defrag, and tcp stack */
		struct list_head	list;
	};

	union {
		struct sock		*sk;
		int			ip_defrag_offset;
	};

	union {
		ktime_t		tstamp;
		u64		skb_mstamp;
	};
#if defined(CONFIG_BCM_KF_NBUFF)
	__u32 unused;
	union {
		/* 3 bytes unused */
		unsigned int recycle_and_rnr_flags;
		unsigned int recycle_flags;
	};
	/*
	 * Several skb fields have been regrouped together for better data locality
	 * cache performance, 16byte cache line proximity.
	 * In 32 bit architecture, we have 32 bytes of data before this comment.
	 * In 64 bit architecture, we have 52 bytes of data at this point.
	 */

	/*--- members common to fkbuff: begin here ---*/
	struct {
		union {
			/* see fkb_in_skb_test() */
			void 			*fkbInSkb;
			void			*word0;
		};

		/* defined(CONFIG_BLOG), use blog_ptr() */
		struct blog_t		*blog_p;
		unsigned char		*data;

		/* The len in fkb is only 24 bits other 8 bits are used as internal flags
		 * when fkbInSkb is used the max len can be only 24 bits, the bits 31-24
		 * are cleared
		 * currently we don't have a case where len can be >24 bits.
		 */
		union {
			unsigned int	len;
			/* used for fkb_in_skb test */
			__u32		len_word;
		};

		union {
			__u32		mark;
			__u32		dropcount;
			void		*queue;
			/* have to declare the following variation of fkb_mark
			 * for the ease of handling 64 bit vs 32 bit in fcache
			 */
			unsigned long	fkb_mark;
			__u32       fc_ctxt; /* hybrid flow cache context */
		};

		union {
			__u32		priority;
			wlFlowInf_t	wl;
		};

		/* Recycle preallocated skb or data */
		RecycleFuncP		recycle_hook;

		union {
			unsigned long	recycle_context;
			struct sk_buff	*next_free;
			__u32       fpm_num;
		};
#ifdef CONFIG_64BIT
	}  ____cacheline_aligned;
	/*
	 * purposedly making the above fkbuff data structure cacheline aligned
	 * in 64 bit architecture.
	 * This can ensure the offset to the content is fixed into same cacheline.
	 * Main reason we only declare as cacheline_aligned for 64 bit is that
	 * we have manually calculated to ensure that this structure is 32 byte
	 * aligned in 32 bit architecture.  If we add ____cacheline_aligned
	 * also for 32 bit architecture, it will waste 64 byte memory if that
	 * architecture is with 64 byte cache line size (i.e., 63148).
	 */
#else
	};
#endif
	/*--- members common to fkbuff: end here ---*/


#endif

	struct bcm_skb_ext bcm_ext;

	union {
		struct {
			unsigned long	_skb_refdst;
			void		(*destructor)(struct sk_buff *skb);
		};
		struct list_head	tcp_tsorted_anchor;
	};

	struct	sec_path	*sp;
	unsigned long		 _nfct;
	struct nf_bridge_info	*nf_bridge;
#if defined(CONFIG_BCM_KF_NBUFF)
	unsigned int		data_len;
#else
	unsigned int		len,
				data_len;
#endif
	__u16		mac_len,
				hdr_len;

	/* Following fields are _not_ copied in __copy_skb_header()
	 * Note that queue_mapping is here mostly to fill a hole.
	 */
	__u16			queue_mapping;

/* if you move cloned around you also must adapt those constants */
#ifdef __BIG_ENDIAN_BITFIELD
#define CLONED_MASK	(1 << 7)
#else
#define CLONED_MASK	1
#endif
#define CLONED_OFFSET()		offsetof(struct sk_buff, __cloned_offset)

	__u8		__cloned_offset[0];
	__u8		cloned:1,
				nohdr:1,
				fclone:2,
				peeked:1,
				head_frag:1,
				xmit_more:1,
				pfmemalloc:1;

	/* fields enclosed in headers_start/headers_end are copied
	 * using a single memcpy() in __copy_skb_header()
	 */
	/* private: */
	__u32			headers_start[0];
	/* public: */

/* if you move pkt_type around you also must adapt those constants */
#ifdef __BIG_ENDIAN_BITFIELD
#define PKT_TYPE_MAX	(7 << 5)
#else
#define PKT_TYPE_MAX	7
#endif
#define PKT_TYPE_OFFSET()	offsetof(struct sk_buff, __pkt_type_offset)

	__u8			__pkt_type_offset[0];
	__u8			pkt_type:3;
	__u8			ignore_df:1;
	__u8			nf_trace:1;
	__u8			ip_summed:2;
	__u8			ooo_okay:1;

	__u8			l4_hash:1;
	__u8			sw_hash:1;
	__u8			wifi_acked_valid:1;
	__u8			wifi_acked:1;
	__u8			no_fcs:1;
	/* Indicates the inner headers are valid in the skbuff. */
	__u8			encapsulation:1;
	__u8			encap_hdr_csum:1;
	__u8			csum_valid:1;

	__u8			csum_complete_sw:1;
	__u8			csum_level:2;
	__u8			csum_not_inet:1;
	__u8			dst_pending_confirm:1;
	__u8			ndisc_nodetype:2;
	__u8			ipvs_property:1;

	__u8			inner_protocol_type:1;
	__u8			remcsum_offload:1;
	__u8			offload_fwd_mark:1;
	__u8			offload_mr_fwd_mark:1;
	__u8			tc_skip_classify:1;
	__u8			tc_at_ingress:1;
	__u8			tc_redirected:1;
	__u8			tc_from_ingress:1;
	__u8			decrypted:1;
	__u16			tc_index;	/* traffic control index */

	union {
		__wsum		csum;
		struct {
			__u16	csum_start;
			__u16	csum_offset;
		};
	};
#ifdef CONFIG_BCM_KF_NBUFF
#else
	__u32			priority;
#endif
	int			skb_iif;
	__u32			hash;
	__be16			vlan_proto;
	__u16			vlan_tci;
	union {
		unsigned int	napi_id;
		unsigned int	sender_cpu;
	};
	__u32		secmark;

#if defined(CONFIG_BCM_KF_NBUFF)
	__u32		reserved_tailroom;
#else
	union {
		__u32		mark;
		__u32		reserved_tailroom;
	};
#endif

	union {
		__be16		inner_protocol;
		__u8		inner_ipproto;
	};

	__u16			inner_transport_header;
	__u16			inner_network_header;
	__u16			inner_mac_header;

	__be16			protocol;
	__u16			transport_header;
	__u16			network_header;
	__u16			mac_header;

	/* private: */
	__u32			headers_end[0];

	/*
	 * This is the control buffer. It is free to use for every
	 * layer. Please put your private variables there. If you
	 * want to keep them across layers you have to do a skb_clone()
	 * first. This is owned by whoever has the skb queued ATM.
	 */
	char			cb[BCM_SKB_CB_SIZE] __aligned(8);
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	void			*cb_next;
	struct nf_queue_entry   *nf_queue_entry;
	__u8			imq_flags:IMQ_F_BITS;
#endif

/*
 * ------------------------------- CAUTION!!! ---------------------------------
 * Do NOT add a new field or modify any existing field(except cb) before this 
 * line to the beginning of the struct sk_buff. Doing so will cause 
 * struct sk_buff to be incompatible with the compiled binaries and may cause 
 * the binary only modules to crash.
 * ---------------------------------------------------------------------------
 */

	/* public: */

	/* These elements must be at the end, see alloc_skb() for details.  */
	sk_buff_data_t		tail;
	sk_buff_data_t		end;
#if defined(CONFIG_BCM_KF_NBUFF)
	unsigned char		*head;
#else
	unsigned char		*head,
				*data;
#endif
	unsigned int		truesize;
	refcount_t		users;
};


#endif	/* _BCM_SKBUFF_H */
