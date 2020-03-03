/*
 * CAAM/SEC 4.x functions for using scatterlists in caam driver
 *
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 */

struct sec4_sg_entry;

/*
 * convert single dma address to h/w link table format
 */
static inline void dma_to_sec4_sg_one(struct sec4_sg_entry *sec4_sg_ptr,
				      dma_addr_t dma, u32 len, u32 offset)
{
	sec4_sg_ptr->ptr = dma;
	sec4_sg_ptr->len = len;
	sec4_sg_ptr->reserved = 0;
	sec4_sg_ptr->buf_pool_id = 0;
	sec4_sg_ptr->offset = offset;
#ifdef DEBUG
	print_hex_dump(KERN_ERR, "sec4_sg_ptr@: ",
		       DUMP_PREFIX_ADDRESS, 16, 4, sec4_sg_ptr,
		       sizeof(struct sec4_sg_entry), 1);
#endif
}

/*
 * convert scatterlist to h/w link table format
 * but does not have final bit; instead, returns last entry
 */
static inline struct sec4_sg_entry *
sg_to_sec4_sg(struct scatterlist *sg, int sg_count,
	      struct sec4_sg_entry *sec4_sg_ptr, u32 offset)
{
	while (sg_count) {
		dma_to_sec4_sg_one(sec4_sg_ptr, sg_dma_address(sg),
				   sg_dma_len(sg), offset);
		sec4_sg_ptr++;
		sg = sg_next(sg);
		sg_count--;
	}
	return sec4_sg_ptr - 1;
}

/*
 * convert scatterlist to h/w link table format
 * scatterlist must have been previously dma mapped
 */
static inline void sg_to_sec4_sg_last(struct scatterlist *sg, int sg_count,
				      struct sec4_sg_entry *sec4_sg_ptr,
				      u32 offset)
{
	sec4_sg_ptr = sg_to_sec4_sg(sg, sg_count, sec4_sg_ptr, offset);
	sec4_sg_ptr->len |= SEC4_SG_LEN_FIN;
}

/* count number of elements in scatterlist */
static inline int __sg_count(struct scatterlist *sg_list, int nbytes,
			     bool *chained)
{
	struct scatterlist *sg = sg_list;
	int sg_nents = 0;

	while (nbytes > 0) {
		sg_nents++;
		nbytes -= sg->length;
		if (!sg_is_last(sg) && (sg + 1)->length == 0)
			*chained = true;
		sg = sg_next(sg);
	}

	return sg_nents;
}

/* derive number of elements in scatterlist, but return 0 for 1 */
static inline int sg_count(struct scatterlist *sg_list, int nbytes,
			     bool *chained)
{
	int sg_nents = __sg_count(sg_list, nbytes, chained);

	if (likely(sg_nents == 1))
		return 0;

	return sg_nents;
}

static int dma_map_sg_chained(struct device *dev, struct scatterlist *sg,
			      unsigned int nents, enum dma_data_direction dir,
			      bool chained)
{
	if (unlikely(chained)) {
		int i;
		for (i = 0; i < nents; i++) {
			dma_map_sg(dev, sg, 1, dir);
			sg = sg_next(sg);
		}
	} else {
		dma_map_sg(dev, sg, nents, dir);
	}
	return nents;
}

static int dma_unmap_sg_chained(struct device *dev, struct scatterlist *sg,
				unsigned int nents, enum dma_data_direction dir,
				bool chained)
{
	if (unlikely(chained)) {
		int i;
		for (i = 0; i < nents; i++) {
			dma_unmap_sg(dev, sg, 1, dir);
			sg = sg_next(sg);
		}
	} else {
		dma_unmap_sg(dev, sg, nents, dir);
	}
	return nents;
}
