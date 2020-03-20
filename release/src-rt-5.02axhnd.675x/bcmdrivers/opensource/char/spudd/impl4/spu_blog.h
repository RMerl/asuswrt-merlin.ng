/*
<:copyright-BRCM:2016:GPL/GPL:spu

   Copyright (c) 2016 Broadcom
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#ifndef __SPU_BLOG_H__
#define __SPU_BLOG_H__

void spu_blog_ctx_add(struct iproc_ctx_s *ctx);
void spu_blog_ctx_del(struct iproc_ctx_s *ctx);
void spu_blog_emit_aead(struct iproc_reqctx_s *rctx);
int spu_blog_register(void);
void spu_blog_unregister(void);

#endif /* __SPU_BLOG_H__ */
