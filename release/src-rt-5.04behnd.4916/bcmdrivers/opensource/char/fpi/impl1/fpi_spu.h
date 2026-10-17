#ifndef __FPI_SPU_H_INCLUDED__
#define __FPI_SPU_H_INCLUDED__

#if defined(CONFIG_XFRM) && defined(CONFIG_BCM_SPU_HW_OFFLOAD)
void fpi_key_to_blog_spu(fpi_key_t *key_p, Blog_t *blog_p);
int fpi_ctx_to_blog_spu(fpi_flow_t *flow_p, Blog_t *blog_p, uint8_t ipv6_tun);
#else
static void fpi_key_to_blog_spu(fpi_key_t *key_p, Blog_t *blog_p)
{
}

static int fpi_ctx_to_blog_spu(fpi_flow_t *flow_p, Blog_t *blog_p, uint8_t ipv6_tun)
{
	return -1;
}
#endif

#endif // #ifndef __FPI_SPU_H_INCLUDED__
