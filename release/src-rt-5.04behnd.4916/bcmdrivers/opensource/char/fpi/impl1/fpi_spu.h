#ifndef __FPI_SPU_H_INCLUDED__
#define __FPI_SPU_H_INCLUDED__

#if defined(CONFIG_XFRM) && defined(CONFIG_BCM_SPU_HW_OFFLOAD)
int fpi_ctx_to_blog_spu(fpi_flow_t *flow_p, Blog_t *blog_p);
void fpi_cleanup_blog_spu(Blog_t *blog_p);
#else
static int fpi_ctx_to_blog_spu(fpi_flow_t *flow_p, Blog_t *blog_p)
{
	return -1;
}

static void fpi_cleanup_blog_spu(Blog_t *blog_p)
{
}
#endif

#endif // #ifndef __FPI_SPU_H_INCLUDED__
