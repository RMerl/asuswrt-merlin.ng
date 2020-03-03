#ifndef __CEPH_FEATURES
#define __CEPH_FEATURES

/*
 * feature bits
 */
#define CEPH_FEATURE_UID            (1ULL<<0)
#define CEPH_FEATURE_NOSRCADDR      (1ULL<<1)
#define CEPH_FEATURE_MONCLOCKCHECK  (1ULL<<2)
#define CEPH_FEATURE_FLOCK          (1ULL<<3)
#define CEPH_FEATURE_SUBSCRIBE2     (1ULL<<4)
#define CEPH_FEATURE_MONNAMES       (1ULL<<5)
#define CEPH_FEATURE_RECONNECT_SEQ  (1ULL<<6)
#define CEPH_FEATURE_DIRLAYOUTHASH  (1ULL<<7)
#define CEPH_FEATURE_OBJECTLOCATOR  (1ULL<<8)
#define CEPH_FEATURE_PGID64         (1ULL<<9)
#define CEPH_FEATURE_INCSUBOSDMAP   (1ULL<<10)
#define CEPH_FEATURE_PGPOOL3        (1ULL<<11)
#define CEPH_FEATURE_OSDREPLYMUX    (1ULL<<12)
#define CEPH_FEATURE_OSDENC         (1ULL<<13)
#define CEPH_FEATURE_OMAP           (1ULL<<14)
#define CEPH_FEATURE_MONENC         (1ULL<<15)
#define CEPH_FEATURE_QUERY_T        (1ULL<<16)
#define CEPH_FEATURE_INDEP_PG_MAP   (1ULL<<17)
#define CEPH_FEATURE_CRUSH_TUNABLES (1ULL<<18)
#define CEPH_FEATURE_CHUNKY_SCRUB   (1ULL<<19)
#define CEPH_FEATURE_MON_NULLROUTE  (1ULL<<20)
#define CEPH_FEATURE_MON_GV         (1ULL<<21)
#define CEPH_FEATURE_BACKFILL_RESERVATION (1ULL<<22)
#define CEPH_FEATURE_MSG_AUTH	    (1ULL<<23)
#define CEPH_FEATURE_RECOVERY_RESERVATION (1ULL<<24)
#define CEPH_FEATURE_CRUSH_TUNABLES2 (1ULL<<25)
#define CEPH_FEATURE_CREATEPOOLID   (1ULL<<26)
#define CEPH_FEATURE_REPLY_CREATE_INODE   (1ULL<<27)
#define CEPH_FEATURE_OSD_HBMSGS     (1ULL<<28)
#define CEPH_FEATURE_MDSENC         (1ULL<<29)
#define CEPH_FEATURE_OSDHASHPSPOOL  (1ULL<<30)
#define CEPH_FEATURE_MON_SINGLE_PAXOS (1ULL<<31)
#define CEPH_FEATURE_OSD_SNAPMAPPER (1ULL<<32)
#define CEPH_FEATURE_MON_SCRUB      (1ULL<<33)
#define CEPH_FEATURE_OSD_PACKED_RECOVERY (1ULL<<34)
#define CEPH_FEATURE_OSD_CACHEPOOL (1ULL<<35)
#define CEPH_FEATURE_CRUSH_V2      (1ULL<<36)  /* new indep; SET_* steps */
#define CEPH_FEATURE_EXPORT_PEER   (1ULL<<37)
#define CEPH_FEATURE_OSD_ERASURE_CODES (1ULL<<38)
#define CEPH_FEATURE_OSD_TMAP2OMAP (1ULL<<38)   /* overlap with EC */
/* The process supports new-style OSDMap encoding. Monitors also use
   this bit to determine if peers support NAK messages. */
#define CEPH_FEATURE_OSDMAP_ENC    (1ULL<<39)
#define CEPH_FEATURE_MDS_INLINE_DATA     (1ULL<<40)
#define CEPH_FEATURE_CRUSH_TUNABLES3     (1ULL<<41)
#define CEPH_FEATURE_OSD_PRIMARY_AFFINITY (1ULL<<41)  /* overlap w/ tunables3 */
#define CEPH_FEATURE_MSGR_KEEPALIVE2   (1ULL<<42)
#define CEPH_FEATURE_OSD_POOLRESEND    (1ULL<<43)
#define CEPH_FEATURE_ERASURE_CODE_PLUGINS_V2 (1ULL<<44)
#define CEPH_FEATURE_OSD_SET_ALLOC_HINT (1ULL<<45)
#define CEPH_FEATURE_OSD_FADVISE_FLAGS (1ULL<<46)
#define CEPH_FEATURE_OSD_REPOP         (1ULL<<46)   /* overlap with fadvise */
#define CEPH_FEATURE_OSD_OBJECT_DIGEST  (1ULL<<46)  /* overlap with fadvise */
#define CEPH_FEATURE_OSD_TRANSACTION_MAY_LAYOUT (1ULL<<46) /* overlap w/ fadvise */
#define CEPH_FEATURE_MDS_QUOTA      (1ULL<<47)
#define CEPH_FEATURE_CRUSH_V4      (1ULL<<48)  /* straw2 buckets */
#define CEPH_FEATURE_OSD_MIN_SIZE_RECOVERY (1ULL<<49)
// duplicated since it was introduced at the same time as MIN_SIZE_RECOVERY
#define CEPH_FEATURE_OSD_PROXY_FEATURES (1ULL<<49)  /* overlap w/ above */

/*
 * The introduction of CEPH_FEATURE_OSD_SNAPMAPPER caused the feature
 * vector to evaluate to 64 bit ~0.  To cope, we designate 1ULL << 63
 * to mean 33 bit ~0, and introduce a helper below to do the
 * translation.
 *
 * This was introduced by ceph.git commit
 *   9ea02b84104045c2ffd7e7f4e7af512953855ecd v0.58-657-g9ea02b8
 * and fixed by ceph.git commit
 *   4255b5c2fb54ae40c53284b3ab700fdfc7e61748 v0.65-263-g4255b5c
 */
#define CEPH_FEATURE_RESERVED (1ULL<<63)

static inline u64 ceph_sanitize_features(u64 features)
{
	if (features & CEPH_FEATURE_RESERVED) {
		/* everything through OSD_SNAPMAPPER */
		return 0x1ffffffffull;
	} else {
		return features;
	}
}

/*
 * Features supported.
 */
#define CEPH_FEATURES_SUPPORTED_DEFAULT		\
	(CEPH_FEATURE_NOSRCADDR |		\
	 CEPH_FEATURE_RECONNECT_SEQ |		\
	 CEPH_FEATURE_PGID64 |			\
	 CEPH_FEATURE_PGPOOL3 |			\
	 CEPH_FEATURE_OSDENC |			\
	 CEPH_FEATURE_CRUSH_TUNABLES |		\
	 CEPH_FEATURE_MSG_AUTH |		\
	 CEPH_FEATURE_CRUSH_TUNABLES2 |		\
	 CEPH_FEATURE_REPLY_CREATE_INODE |	\
	 CEPH_FEATURE_OSDHASHPSPOOL |		\
	 CEPH_FEATURE_OSD_CACHEPOOL |		\
	 CEPH_FEATURE_CRUSH_V2 |		\
	 CEPH_FEATURE_EXPORT_PEER |		\
	 CEPH_FEATURE_OSDMAP_ENC |		\
	 CEPH_FEATURE_CRUSH_TUNABLES3 |		\
	 CEPH_FEATURE_OSD_PRIMARY_AFFINITY |	\
	 CEPH_FEATURE_CRUSH_V4)

#define CEPH_FEATURES_REQUIRED_DEFAULT   \
	(CEPH_FEATURE_NOSRCADDR |	 \
	 CEPH_FEATURE_RECONNECT_SEQ |	 \
	 CEPH_FEATURE_PGID64 |		 \
	 CEPH_FEATURE_PGPOOL3 |		 \
	 CEPH_FEATURE_OSDENC)

#endif
