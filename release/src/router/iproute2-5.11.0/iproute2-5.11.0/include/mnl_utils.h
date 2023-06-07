/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __MNL_UTILS_H__
#define __MNL_UTILS_H__ 1

struct mnlu_gen_socket {
	struct mnl_socket *nl;
	char *buf;
	uint32_t family;
	unsigned int seq;
	uint8_t version;
};

int mnlu_gen_socket_open(struct mnlu_gen_socket *nlg, const char *family_name,
			 uint8_t version);
void mnlu_gen_socket_close(struct mnlu_gen_socket *nlg);
struct nlmsghdr *mnlu_gen_socket_cmd_prepare(struct mnlu_gen_socket *nlg,
					     uint8_t cmd, uint16_t flags);
int mnlu_gen_socket_sndrcv(struct mnlu_gen_socket *nlg, const struct nlmsghdr *nlh,
			   mnl_cb_t data_cb, void *data);

struct mnl_socket *mnlu_socket_open(int bus);
struct nlmsghdr *mnlu_msg_prepare(void *buf, uint32_t nlmsg_type, uint16_t flags,
				  void *extra_header, size_t extra_header_size);
int mnlu_socket_recv_run(struct mnl_socket *nl, unsigned int seq, void *buf, size_t buf_size,
			 mnl_cb_t cb, void *data);

#endif /* __MNL_UTILS_H__ */
