/* source: xio-openssl.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_openssl_included
#define __xio_openssl_included 1

#if WITH_OPENSSL	/* make this address configure dependend */

#define SSLIO_BASE 0x53530000	/* "SSxx" */
#define SSLIO_MASK 0xffff0000

extern const struct addrdesc addr_openssl;
extern const struct addrdesc addr_openssl_listen;

extern const struct optdesc opt_openssl_cipherlist;
extern const struct optdesc opt_openssl_method;
extern const struct optdesc opt_openssl_verify;
extern const struct optdesc opt_openssl_certificate;
extern const struct optdesc opt_openssl_key;
extern const struct optdesc opt_openssl_dhparam;
extern const struct optdesc opt_openssl_cafile;
extern const struct optdesc opt_openssl_capath;
extern const struct optdesc opt_openssl_egd;
extern const struct optdesc opt_openssl_pseudo;
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
extern const struct optdesc opt_openssl_compress;
#endif
#if WITH_FIPS
extern const struct optdesc opt_openssl_fips;
#endif
extern const struct optdesc opt_openssl_commonname;

extern int
   _xioopen_openssl_prepare(struct opt *opts, struct single *xfd,
			    bool server, bool *opt_ver, const char *opt_cert,
			    SSL_CTX **ctx);
extern int
   _xioopen_openssl_connect(struct single *xfd,  bool opt_ver,
			    const char *opt_commonname,
			    SSL_CTX *ctx, int level);
extern int
   _xioopen_openssl_listen(struct single *xfd, bool opt_ver,
			   const char *opt_commonname,
			   SSL_CTX *ctx, int level);
extern int xioclose_openssl(xiofile_t *xfd);
extern int xioshutdown_openssl(xiofile_t *xfd, int how);
extern ssize_t xioread_openssl(struct single *file, void *buff, size_t bufsiz);
extern ssize_t xiopending_openssl(struct single *pipe);
extern ssize_t xiowrite_openssl(struct single *file, const void *buff, size_t bufsiz);

#if WITH_FIPS
extern int xio_reset_fips_mode(void);
#endif /* WITH_FIPS */

#endif /* WITH_OPENSSL */

#endif /* !defined(__xio_openssl_included) */
