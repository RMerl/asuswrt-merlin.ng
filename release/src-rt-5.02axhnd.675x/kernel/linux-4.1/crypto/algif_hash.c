/*
 * algif_hash: User-space interface for hash algorithms
 *
 * This file provides the user-space API for hash algorithms.
 *
 * Copyright (c) 2010 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <crypto/hash.h>
#include <crypto/if_alg.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/net.h>
#include <net/sock.h>

struct hash_ctx {
	struct af_alg_sgl sgl;

	u8 *result;

	struct af_alg_completion completion;

	unsigned int len;
	bool more;

	struct ahash_request req;
};

struct algif_hash_tfm {
	struct crypto_ahash *hash;
	bool has_key;
};

static int hash_sendmsg(struct socket *sock, struct msghdr *msg,
			size_t ignored)
{
	int limit = ALG_MAX_PAGES * PAGE_SIZE;
	struct sock *sk = sock->sk;
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;
	long copied = 0;
	int err;

	if (limit > sk->sk_sndbuf)
		limit = sk->sk_sndbuf;

	lock_sock(sk);
	if (!ctx->more) {
		err = af_alg_wait_for_completion(crypto_ahash_init(&ctx->req),
						&ctx->completion);
		if (err)
			goto unlock;
	}

	ctx->more = 0;

	while (msg_data_left(msg)) {
		int len = msg_data_left(msg);

		if (len > limit)
			len = limit;

		len = af_alg_make_sg(&ctx->sgl, &msg->msg_iter, len);
		if (len < 0) {
			err = copied ? 0 : len;
			goto unlock;
		}

		ahash_request_set_crypt(&ctx->req, ctx->sgl.sg, NULL, len);

		err = af_alg_wait_for_completion(crypto_ahash_update(&ctx->req),
						 &ctx->completion);
		af_alg_free_sg(&ctx->sgl);
		if (err)
			goto unlock;

		copied += len;
		iov_iter_advance(&msg->msg_iter, len);
	}

	err = 0;

	ctx->more = msg->msg_flags & MSG_MORE;
	if (!ctx->more) {
		ahash_request_set_crypt(&ctx->req, NULL, ctx->result, 0);
		err = af_alg_wait_for_completion(crypto_ahash_final(&ctx->req),
						 &ctx->completion);
	}

unlock:
	release_sock(sk);

	return err ?: copied;
}

static ssize_t hash_sendpage(struct socket *sock, struct page *page,
			     int offset, size_t size, int flags)
{
	struct sock *sk = sock->sk;
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;
	int err;

	if (flags & MSG_SENDPAGE_NOTLAST)
		flags |= MSG_MORE;

	lock_sock(sk);
	sg_init_table(ctx->sgl.sg, 1);
	sg_set_page(ctx->sgl.sg, page, size, offset);

	ahash_request_set_crypt(&ctx->req, ctx->sgl.sg, ctx->result, size);

	if (!(flags & MSG_MORE)) {
		if (ctx->more)
			err = crypto_ahash_finup(&ctx->req);
		else
			err = crypto_ahash_digest(&ctx->req);
	} else {
		if (!ctx->more) {
			err = crypto_ahash_init(&ctx->req);
			err = af_alg_wait_for_completion(err, &ctx->completion);
			if (err)
				goto unlock;
		}

		err = crypto_ahash_update(&ctx->req);
	}

	err = af_alg_wait_for_completion(err, &ctx->completion);
	if (err)
		goto unlock;

	ctx->more = flags & MSG_MORE;

unlock:
	release_sock(sk);

	return err ?: size;
}

static int hash_recvmsg(struct socket *sock, struct msghdr *msg, size_t len,
			int flags)
{
	struct sock *sk = sock->sk;
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;
	unsigned ds = crypto_ahash_digestsize(crypto_ahash_reqtfm(&ctx->req));
	int err;

	if (len > ds)
		len = ds;
	else if (len < ds)
		msg->msg_flags |= MSG_TRUNC;

	lock_sock(sk);
	if (ctx->more) {
		ctx->more = 0;
		ahash_request_set_crypt(&ctx->req, NULL, ctx->result, 0);
		err = af_alg_wait_for_completion(crypto_ahash_final(&ctx->req),
						 &ctx->completion);
		if (err)
			goto unlock;
	}

	err = memcpy_to_msg(msg, ctx->result, len);

unlock:
	release_sock(sk);

	return err ?: len;
}

static int hash_accept(struct socket *sock, struct socket *newsock, int flags)
{
	struct sock *sk = sock->sk;
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;
	struct ahash_request *req = &ctx->req;
	char state[crypto_ahash_statesize(crypto_ahash_reqtfm(req)) ? : 1];
	struct sock *sk2;
	struct alg_sock *ask2;
	struct hash_ctx *ctx2;
	int err;

	err = crypto_ahash_export(req, state);
	if (err)
		return err;

	err = af_alg_accept(ask->parent, newsock);
	if (err)
		return err;

	sk2 = newsock->sk;
	ask2 = alg_sk(sk2);
	ctx2 = ask2->private;
	ctx2->more = 1;

	err = crypto_ahash_import(&ctx2->req, state);
	if (err) {
		sock_orphan(sk2);
		sock_put(sk2);
	}

	return err;
}

static struct proto_ops algif_hash_ops = {
	.family		=	PF_ALG,

	.connect	=	sock_no_connect,
	.socketpair	=	sock_no_socketpair,
	.getname	=	sock_no_getname,
	.ioctl		=	sock_no_ioctl,
	.listen		=	sock_no_listen,
	.shutdown	=	sock_no_shutdown,
	.getsockopt	=	sock_no_getsockopt,
	.mmap		=	sock_no_mmap,
	.bind		=	sock_no_bind,
	.setsockopt	=	sock_no_setsockopt,
	.poll		=	sock_no_poll,

	.release	=	af_alg_release,
	.sendmsg	=	hash_sendmsg,
	.sendpage	=	hash_sendpage,
	.recvmsg	=	hash_recvmsg,
	.accept		=	hash_accept,
};

static int hash_check_key(struct socket *sock)
{
	int err = 0;
	struct sock *psk;
	struct alg_sock *pask;
	struct algif_hash_tfm *tfm;
	struct sock *sk = sock->sk;
	struct alg_sock *ask = alg_sk(sk);

	lock_sock(sk);
	if (ask->refcnt)
		goto unlock_child;

	psk = ask->parent;
	pask = alg_sk(ask->parent);
	tfm = pask->private;

	err = -ENOKEY;
	lock_sock_nested(psk, SINGLE_DEPTH_NESTING);
	if (!tfm->has_key)
		goto unlock;

	if (!pask->refcnt++)
		sock_hold(psk);

	ask->refcnt = 1;
	sock_put(psk);

	err = 0;

unlock:
	release_sock(psk);
unlock_child:
	release_sock(sk);

	return err;
}

static int hash_sendmsg_nokey(struct socket *sock, struct msghdr *msg,
			      size_t size)
{
	int err;

	err = hash_check_key(sock);
	if (err)
		return err;

	return hash_sendmsg(sock, msg, size);
}

static ssize_t hash_sendpage_nokey(struct socket *sock, struct page *page,
				   int offset, size_t size, int flags)
{
	int err;

	err = hash_check_key(sock);
	if (err)
		return err;

	return hash_sendpage(sock, page, offset, size, flags);
}

static int hash_recvmsg_nokey(struct socket *sock, struct msghdr *msg,
			      size_t ignored, int flags)
{
	int err;

	err = hash_check_key(sock);
	if (err)
		return err;

	return hash_recvmsg(sock, msg, ignored, flags);
}

static int hash_accept_nokey(struct socket *sock, struct socket *newsock,
			     int flags)
{
	int err;

	err = hash_check_key(sock);
	if (err)
		return err;

	return hash_accept(sock, newsock, flags);
}

static struct proto_ops algif_hash_ops_nokey = {
	.family		=	PF_ALG,

	.connect	=	sock_no_connect,
	.socketpair	=	sock_no_socketpair,
	.getname	=	sock_no_getname,
	.ioctl		=	sock_no_ioctl,
	.listen		=	sock_no_listen,
	.shutdown	=	sock_no_shutdown,
	.getsockopt	=	sock_no_getsockopt,
	.mmap		=	sock_no_mmap,
	.bind		=	sock_no_bind,
	.setsockopt	=	sock_no_setsockopt,
	.poll		=	sock_no_poll,

	.release	=	af_alg_release,
	.sendmsg	=	hash_sendmsg_nokey,
	.sendpage	=	hash_sendpage_nokey,
	.recvmsg	=	hash_recvmsg_nokey,
	.accept		=	hash_accept_nokey,
};

static void *hash_bind(const char *name, u32 type, u32 mask)
{
	struct algif_hash_tfm *tfm;
	struct crypto_ahash *hash;

	tfm = kzalloc(sizeof(*tfm), GFP_KERNEL);
	if (!tfm)
		return ERR_PTR(-ENOMEM);

	hash = crypto_alloc_ahash(name, type, mask);
	if (IS_ERR(hash)) {
		kfree(tfm);
		return ERR_CAST(hash);
	}

	tfm->hash = hash;

	return tfm;
}

static void hash_release(void *private)
{
	struct algif_hash_tfm *tfm = private;

	crypto_free_ahash(tfm->hash);
	kfree(tfm);
}

static int hash_setkey(void *private, const u8 *key, unsigned int keylen)
{
	struct algif_hash_tfm *tfm = private;
	int err;

	err = crypto_ahash_setkey(tfm->hash, key, keylen);
	tfm->has_key = !err;

	return err;
}

static void hash_sock_destruct(struct sock *sk)
{
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;

	sock_kzfree_s(sk, ctx->result,
		      crypto_ahash_digestsize(crypto_ahash_reqtfm(&ctx->req)));
	sock_kfree_s(sk, ctx, ctx->len);
	af_alg_release_parent(sk);
}

static int hash_accept_parent_nokey(void *private, struct sock *sk)
{
	struct hash_ctx *ctx;
	struct alg_sock *ask = alg_sk(sk);
	struct algif_hash_tfm *tfm = private;
	struct crypto_ahash *hash = tfm->hash;
	unsigned len = sizeof(*ctx) + crypto_ahash_reqsize(hash);
	unsigned ds = crypto_ahash_digestsize(hash);

	ctx = sock_kmalloc(sk, len, GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->result = sock_kmalloc(sk, ds, GFP_KERNEL);
	if (!ctx->result) {
		sock_kfree_s(sk, ctx, len);
		return -ENOMEM;
	}

	memset(ctx->result, 0, ds);

	ctx->len = len;
	ctx->more = 0;
	af_alg_init_completion(&ctx->completion);

	ask->private = ctx;

	ahash_request_set_tfm(&ctx->req, hash);
	ahash_request_set_callback(&ctx->req, CRYPTO_TFM_REQ_MAY_BACKLOG,
				   af_alg_complete, &ctx->completion);

	sk->sk_destruct = hash_sock_destruct;

	return 0;
}

static int hash_accept_parent(void *private, struct sock *sk)
{
	struct algif_hash_tfm *tfm = private;

	if (!tfm->has_key && crypto_ahash_has_setkey(tfm->hash))
		return -ENOKEY;

	return hash_accept_parent_nokey(private, sk);
}

static const struct af_alg_type algif_type_hash = {
	.bind		=	hash_bind,
	.release	=	hash_release,
	.setkey		=	hash_setkey,
	.accept		=	hash_accept_parent,
	.accept_nokey	=	hash_accept_parent_nokey,
	.ops		=	&algif_hash_ops,
	.ops_nokey	=	&algif_hash_ops_nokey,
	.name		=	"hash",
	.owner		=	THIS_MODULE
};

static int __init algif_hash_init(void)
{
	return af_alg_register_type(&algif_type_hash);
}

static void __exit algif_hash_exit(void)
{
	int err = af_alg_unregister_type(&algif_type_hash);
	BUG_ON(err);
}

module_init(algif_hash_init);
module_exit(algif_hash_exit);
MODULE_LICENSE("GPL");
