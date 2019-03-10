/*

Copyright (c) 2017 Denis Bychkov (manover@gmail.com)

This file is released under the GNU General Public License (GPLv2).
The full license text is available at:

http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef OPENSSL_COMPAT_H
#define OPENSSL_COMPAT_H

#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
inline static int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
   /* If the fields p and g in d are NULL, the corresponding input
    * parameters MUST be non-NULL.  q may remain NULL.
    */
   if ((dh->p == NULL && p == NULL) || (dh->g == NULL && g == NULL))
       return 0;

   if (p != NULL)
       dh->p = p;
   if (q != NULL)
       dh->q = q;
   if (g != NULL)
       dh->g = g;

   if (q != NULL)
       dh->length = BN_num_bits(q);

   return 1;
}

inline static void DH_get0_key(const DH *dh, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
   if (pub_key != NULL)
       *pub_key = dh->pub_key;
   if (priv_key != NULL)
       *priv_key = dh->priv_key;
}
#endif /* OPENSSL_VERSION_NUMBER */

#endif /* OPENSSL_COMPAT_H */
