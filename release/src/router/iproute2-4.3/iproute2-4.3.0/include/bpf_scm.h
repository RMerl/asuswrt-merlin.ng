#ifndef __BPF_SCM__
#define __BPF_SCM__

#include <sys/types.h>
#include <sys/socket.h>

#include "utils.h"
#include "bpf_elf.h"

#define BPF_SCM_AUX_VER		1
#define BPF_SCM_MAX_FDS		ELF_MAX_MAPS
#define BPF_SCM_MSG_SIZE	1024

struct bpf_elf_st {
	dev_t st_dev;
	ino_t st_ino;
};

struct bpf_map_aux {
	unsigned short uds_ver;
	unsigned short num_ent;
	char obj_name[64];
	struct bpf_elf_st obj_st;
	struct bpf_elf_map ent[BPF_SCM_MAX_FDS];
};

struct bpf_map_set_msg {
	struct msghdr hdr;
	struct iovec iov;
	char msg_buf[BPF_SCM_MSG_SIZE];
	struct bpf_map_aux aux;
};

static inline int *bpf_map_set_init(struct bpf_map_set_msg *msg,
				    struct sockaddr_un *addr,
				    unsigned int addr_len)
{
	const unsigned int cmsg_ctl_len = sizeof(int) * BPF_SCM_MAX_FDS;
	struct cmsghdr *cmsg;

	msg->iov.iov_base = &msg->aux;
	msg->iov.iov_len = sizeof(msg->aux);

	msg->hdr.msg_iov = &msg->iov;
	msg->hdr.msg_iovlen = 1;

	msg->hdr.msg_name = (struct sockaddr *)addr;
	msg->hdr.msg_namelen = addr_len;

	BUILD_BUG_ON(sizeof(msg->msg_buf) < cmsg_ctl_len);
	msg->hdr.msg_control = &msg->msg_buf;
	msg->hdr.msg_controllen	= cmsg_ctl_len;

	cmsg = CMSG_FIRSTHDR(&msg->hdr);
	cmsg->cmsg_len = msg->hdr.msg_controllen;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type	= SCM_RIGHTS;

	return (int *)CMSG_DATA(cmsg);
}

static inline void bpf_map_set_init_single(struct bpf_map_set_msg *msg,
					   int num)
{
	struct cmsghdr *cmsg;

	msg->hdr.msg_controllen = CMSG_LEN(sizeof(int) * num);
	msg->iov.iov_len = offsetof(struct bpf_map_aux, ent) +
			   sizeof(struct bpf_elf_map) * num;

	cmsg = CMSG_FIRSTHDR(&msg->hdr);
	cmsg->cmsg_len = msg->hdr.msg_controllen;
}

#endif /* __BPF_SCM__ */
