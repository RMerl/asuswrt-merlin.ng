/*
 * tc_bpf.c	BPF common code
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Daniel Borkmann <dborkman@redhat.com>
 *		Jiri Pirko <jiri@resnulli.us>
 *		Alexei Starovoitov <ast@plumgrid.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <linux/filter.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#ifdef HAVE_ELF
#include <libelf.h>
#include <gelf.h>
#endif

#include "utils.h"

#include "bpf_elf.h"
#include "bpf_scm.h"

#include "tc_util.h"
#include "tc_bpf.h"

int bpf_parse_string(char *arg, bool from_file, __u16 *bpf_len,
		     char **bpf_string, bool *need_release,
		     const char separator)
{
	char sp;

	if (from_file) {
		size_t tmp_len, op_len = sizeof("65535 255 255 4294967295,");
		char *tmp_string;
		FILE *fp;

		tmp_len = sizeof("4096,") + BPF_MAXINSNS * op_len;
		tmp_string = malloc(tmp_len);
		if (tmp_string == NULL)
			return -ENOMEM;

		memset(tmp_string, 0, tmp_len);

		fp = fopen(arg, "r");
		if (fp == NULL) {
			perror("Cannot fopen");
			free(tmp_string);
			return -ENOENT;
		}

		if (!fgets(tmp_string, tmp_len, fp)) {
			free(tmp_string);
			fclose(fp);
			return -EIO;
		}

		fclose(fp);

		*need_release = true;
		*bpf_string = tmp_string;
	} else {
		*need_release = false;
		*bpf_string = arg;
	}

	if (sscanf(*bpf_string, "%hu%c", bpf_len, &sp) != 2 ||
	    sp != separator) {
		if (*need_release)
			free(*bpf_string);
		return -EINVAL;
	}

	return 0;
}

int bpf_parse_ops(int argc, char **argv, struct sock_filter *bpf_ops,
		  bool from_file)
{
	char *bpf_string, *token, separator = ',';
	int ret = 0, i = 0;
	bool need_release;
	__u16 bpf_len = 0;

	if (argc < 1)
		return -EINVAL;
	if (bpf_parse_string(argv[0], from_file, &bpf_len, &bpf_string,
			     &need_release, separator))
		return -EINVAL;
	if (bpf_len == 0 || bpf_len > BPF_MAXINSNS) {
		ret = -EINVAL;
		goto out;
	}

	token = bpf_string;
	while ((token = strchr(token, separator)) && (++token)[0]) {
		if (i >= bpf_len) {
			fprintf(stderr, "Real program length exceeds encoded "
				"length parameter!\n");
			ret = -EINVAL;
			goto out;
		}

		if (sscanf(token, "%hu %hhu %hhu %u,",
			   &bpf_ops[i].code, &bpf_ops[i].jt,
			   &bpf_ops[i].jf, &bpf_ops[i].k) != 4) {
			fprintf(stderr, "Error at instruction %d!\n", i);
			ret = -EINVAL;
			goto out;
		}

		i++;
	}

	if (i != bpf_len) {
		fprintf(stderr, "Parsed program length is less than encoded"
			"length parameter!\n");
		ret = -EINVAL;
		goto out;
	}
	ret = bpf_len;

out:
	if (need_release)
		free(bpf_string);

	return ret;
}

void bpf_print_ops(FILE *f, struct rtattr *bpf_ops, __u16 len)
{
	struct sock_filter *ops = (struct sock_filter *) RTA_DATA(bpf_ops);
	int i;

	if (len == 0)
		return;

	fprintf(f, "bytecode \'%u,", len);

	for (i = 0; i < len - 1; i++)
		fprintf(f, "%hu %hhu %hhu %u,", ops[i].code, ops[i].jt,
			ops[i].jf, ops[i].k);

	fprintf(f, "%hu %hhu %hhu %u\'", ops[i].code, ops[i].jt,
		ops[i].jf, ops[i].k);
}

const char *bpf_default_section(const enum bpf_prog_type type)
{
	switch (type) {
	case BPF_PROG_TYPE_SCHED_CLS:
		return ELF_SECTION_CLASSIFIER;
	case BPF_PROG_TYPE_SCHED_ACT:
		return ELF_SECTION_ACTION;
	default:
		return NULL;
	}
}

#ifdef HAVE_ELF
struct bpf_elf_sec_data {
	GElf_Shdr sec_hdr;
	char *sec_name;
	Elf_Data *sec_data;
};

struct bpf_map_data {
	int *fds;
	const char *obj;
	struct bpf_elf_st *st;
	struct bpf_elf_map *ent;
};

/* If we provide a small buffer with log level enabled, the kernel
 * could fail program load as no buffer space is available for the
 * log and thus verifier fails. In case something doesn't pass the
 * verifier we still want to hand something descriptive to the user.
 */
static char bpf_log_buf[65536];
static bool bpf_verbose;

static struct bpf_elf_st bpf_st;

static int map_fds[ELF_MAX_MAPS];
static struct bpf_elf_map map_ent[ELF_MAX_MAPS];

static void bpf_dump_error(const char *format, ...)  __check_format_string(1, 2);
static void bpf_dump_error(const char *format, ...)
{
	va_list vl;

	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);

	if (bpf_log_buf[0]) {
		fprintf(stderr, "%s\n", bpf_log_buf);
		memset(bpf_log_buf, 0, sizeof(bpf_log_buf));
	}
}

static void bpf_save_finfo(int file_fd)
{
	struct stat st;
	int ret;

	memset(&bpf_st, 0, sizeof(bpf_st));

	ret = fstat(file_fd, &st);
	if (ret < 0) {
		fprintf(stderr, "Stat of elf file failed: %s\n",
			strerror(errno));
		return;
	}

	bpf_st.st_dev = st.st_dev;
	bpf_st.st_ino = st.st_ino;
}

static void bpf_clear_finfo(void)
{
	memset(&bpf_st, 0, sizeof(bpf_st));
}

static bool bpf_may_skip_map_creation(int file_fd)
{
	struct stat st;
	int ret;

	ret = fstat(file_fd, &st);
	if (ret < 0) {
		fprintf(stderr, "Stat of elf file failed: %s\n",
			strerror(errno));
		return false;
	}

	return (bpf_st.st_dev == st.st_dev) &&
	       (bpf_st.st_ino == st.st_ino);
}

static int bpf_create_map(enum bpf_map_type type, unsigned int size_key,
			  unsigned int size_value, unsigned int max_elem)
{
	union bpf_attr attr = {
		.map_type	= type,
		.key_size	= size_key,
		.value_size	= size_value,
		.max_entries	= max_elem,
	};

	return bpf(BPF_MAP_CREATE, &attr, sizeof(attr));
}

static int bpf_update_map(int fd, const void *key, const void *value,
			  uint64_t flags)
{
	union bpf_attr attr = {
		.map_fd		= fd,
		.key		= bpf_ptr_to_u64(key),
		.value		= bpf_ptr_to_u64(value),
		.flags		= flags,
	};

	return bpf(BPF_MAP_UPDATE_ELEM, &attr, sizeof(attr));
}

static int bpf_prog_load(enum bpf_prog_type type, const struct bpf_insn *insns,
			 unsigned int len, const char *license)
{
	union bpf_attr attr = {
		.prog_type	= type,
		.insns		= bpf_ptr_to_u64(insns),
		.insn_cnt	= len / sizeof(struct bpf_insn),
		.license	= bpf_ptr_to_u64(license),
		.log_buf	= bpf_ptr_to_u64(bpf_log_buf),
		.log_size	= sizeof(bpf_log_buf),
		.log_level	= 1,
	};

	return bpf(BPF_PROG_LOAD, &attr, sizeof(attr));
}

static int bpf_prog_attach(enum bpf_prog_type type, const char *sec,
			   const struct bpf_insn *insns, unsigned int size,
			   const char *license)
{
	int prog_fd = bpf_prog_load(type, insns, size, license);

	if (prog_fd < 0 || bpf_verbose) {
		bpf_dump_error("%s (section \'%s\'): %s\n", prog_fd < 0 ?
			       "BPF program rejected" :
			       "BPF program verification",
			       sec, strerror(errno));
	}

	return prog_fd;
}

static int bpf_map_attach(enum bpf_map_type type, unsigned int size_key,
			  unsigned int size_value, unsigned int max_elem)
{
	int map_fd = bpf_create_map(type, size_key, size_value, max_elem);

	if (map_fd < 0)
		bpf_dump_error("BPF map rejected: %s\n", strerror(errno));

	return map_fd;
}

static void bpf_maps_init(void)
{
	int i;

	memset(map_ent, 0, sizeof(map_ent));
	for (i = 0; i < ARRAY_SIZE(map_fds); i++)
		map_fds[i] = -1;
}

static int bpf_maps_count(void)
{
	int i, count = 0;

	for (i = 0; i < ARRAY_SIZE(map_fds); i++) {
		if (map_fds[i] < 0)
			break;
		count++;
	}

	return count;
}

static void bpf_maps_destroy(void)
{
	int i;

	memset(map_ent, 0, sizeof(map_ent));
	for (i = 0; i < ARRAY_SIZE(map_fds); i++) {
		if (map_fds[i] >= 0)
			close(map_fds[i]);
	}
}

static int bpf_maps_attach(struct bpf_elf_map *maps, unsigned int num_maps)
{
	int i, ret;

	for (i = 0; (i < num_maps) && (num_maps <= ARRAY_SIZE(map_fds)); i++) {
		struct bpf_elf_map *map = &maps[i];

		ret = bpf_map_attach(map->type, map->size_key,
				     map->size_value, map->max_elem);
		if (ret < 0)
			goto err_unwind;

		map_fds[i] = ret;
	}

	return 0;

err_unwind:
	bpf_maps_destroy();
	return ret;
}

static int bpf_fill_section_data(Elf *elf_fd, GElf_Ehdr *elf_hdr, int sec_index,
				 struct bpf_elf_sec_data *sec_data)
{
	GElf_Shdr sec_hdr;
	Elf_Scn *sec_fd;
	Elf_Data *sec_edata;
	char *sec_name;

	memset(sec_data, 0, sizeof(*sec_data));

	sec_fd = elf_getscn(elf_fd, sec_index);
	if (!sec_fd)
		return -EINVAL;

	if (gelf_getshdr(sec_fd, &sec_hdr) != &sec_hdr)
		return -EIO;

	sec_name = elf_strptr(elf_fd, elf_hdr->e_shstrndx,
			      sec_hdr.sh_name);
	if (!sec_name || !sec_hdr.sh_size)
		return -ENOENT;

	sec_edata = elf_getdata(sec_fd, NULL);
	if (!sec_edata || elf_getdata(sec_fd, sec_edata))
		return -EIO;

	memcpy(&sec_data->sec_hdr, &sec_hdr, sizeof(sec_hdr));
	sec_data->sec_name = sec_name;
	sec_data->sec_data = sec_edata;

	return 0;
}

static int bpf_apply_relo_data(struct bpf_elf_sec_data *data_relo,
			       struct bpf_elf_sec_data *data_insn,
			       Elf_Data *sym_tab)
{
	Elf_Data *idata = data_insn->sec_data;
	GElf_Shdr *rhdr = &data_relo->sec_hdr;
	int relo_ent, relo_num = rhdr->sh_size / rhdr->sh_entsize;
	struct bpf_insn *insns = idata->d_buf;
	unsigned int num_insns = idata->d_size / sizeof(*insns);

	for (relo_ent = 0; relo_ent < relo_num; relo_ent++) {
		unsigned int ioff, fnum;
		GElf_Rel relo;
		GElf_Sym sym;

		if (gelf_getrel(data_relo->sec_data, relo_ent, &relo) != &relo)
			return -EIO;

		ioff = relo.r_offset / sizeof(struct bpf_insn);
		if (ioff >= num_insns)
			return -EINVAL;
		if (insns[ioff].code != (BPF_LD | BPF_IMM | BPF_DW))
			return -EINVAL;

		if (gelf_getsym(sym_tab, GELF_R_SYM(relo.r_info), &sym) != &sym)
			return -EIO;

		fnum = sym.st_value / sizeof(struct bpf_elf_map);
		if (fnum >= ARRAY_SIZE(map_fds))
			return -EINVAL;
		if (map_fds[fnum] < 0)
			return -EINVAL;

		insns[ioff].src_reg = BPF_PSEUDO_MAP_FD;
		insns[ioff].imm = map_fds[fnum];
	}

	return 0;
}

static int bpf_fetch_ancillary(int file_fd, Elf *elf_fd, GElf_Ehdr *elf_hdr,
			       bool *sec_done, char *license, unsigned int lic_len,
			       Elf_Data **sym_tab)
{
	int sec_index, ret = -1;

	for (sec_index = 1; sec_index < elf_hdr->e_shnum; sec_index++) {
		struct bpf_elf_sec_data data_anc;

		ret = bpf_fill_section_data(elf_fd, elf_hdr, sec_index,
					    &data_anc);
		if (ret < 0)
			continue;

		/* Extract and load eBPF map fds. */
		if (!strcmp(data_anc.sec_name, ELF_SECTION_MAPS) &&
		    !bpf_may_skip_map_creation(file_fd)) {
			struct bpf_elf_map *maps;
			unsigned int maps_num;

			if (data_anc.sec_data->d_size % sizeof(*maps) != 0)
				return -EINVAL;

			maps = data_anc.sec_data->d_buf;
			maps_num = data_anc.sec_data->d_size / sizeof(*maps);
			memcpy(map_ent, maps, data_anc.sec_data->d_size);

			ret = bpf_maps_attach(maps, maps_num);
			if (ret < 0)
				return ret;

			sec_done[sec_index] = true;
		}
		/* Extract eBPF license. */
		else if (!strcmp(data_anc.sec_name, ELF_SECTION_LICENSE)) {
			if (data_anc.sec_data->d_size > lic_len)
				return -ENOMEM;

			sec_done[sec_index] = true;
			memcpy(license, data_anc.sec_data->d_buf,
			       data_anc.sec_data->d_size);
		}
		/* Extract symbol table for relocations (map fd fixups). */
		else if (data_anc.sec_hdr.sh_type == SHT_SYMTAB) {
			sec_done[sec_index] = true;
			*sym_tab = data_anc.sec_data;
		}
	}

	return ret;
}

static int bpf_fetch_prog_relo(Elf *elf_fd, GElf_Ehdr *elf_hdr, bool *sec_done,
			       enum bpf_prog_type type, const char *sec,
			       const char *license, Elf_Data *sym_tab)
{
	int sec_index, prog_fd = -1;

	for (sec_index = 1; sec_index < elf_hdr->e_shnum; sec_index++) {
		struct bpf_elf_sec_data data_relo, data_insn;
		int ins_index, ret;

		/* Attach eBPF programs with relocation data (maps). */
		ret = bpf_fill_section_data(elf_fd, elf_hdr, sec_index,
					    &data_relo);
		if (ret < 0 || data_relo.sec_hdr.sh_type != SHT_REL)
			continue;

		ins_index = data_relo.sec_hdr.sh_info;

		ret = bpf_fill_section_data(elf_fd, elf_hdr, ins_index,
					    &data_insn);
		if (ret < 0)
			continue;
		if (strcmp(data_insn.sec_name, sec))
			continue;

		ret = bpf_apply_relo_data(&data_relo, &data_insn, sym_tab);
		if (ret < 0)
			continue;

		prog_fd = bpf_prog_attach(type, sec, data_insn.sec_data->d_buf,
					  data_insn.sec_data->d_size, license);
		if (prog_fd < 0)
			continue;

		sec_done[sec_index] = true;
		sec_done[ins_index] = true;
		break;
	}

	return prog_fd;
}

static int bpf_fetch_prog(Elf *elf_fd, GElf_Ehdr *elf_hdr, bool *sec_done,
			  enum bpf_prog_type type, const char *sec,
			  const char *license)
{
	int sec_index, prog_fd = -1;

	for (sec_index = 1; sec_index < elf_hdr->e_shnum; sec_index++) {
		struct bpf_elf_sec_data data_insn;
		int ret;

		/* Attach eBPF programs without relocation data. */
		if (sec_done[sec_index])
			continue;

		ret = bpf_fill_section_data(elf_fd, elf_hdr, sec_index,
					    &data_insn);
		if (ret < 0)
			continue;
		if (strcmp(data_insn.sec_name, sec))
			continue;

		prog_fd = bpf_prog_attach(type, sec, data_insn.sec_data->d_buf,
					  data_insn.sec_data->d_size, license);
		if (prog_fd < 0)
			continue;

		sec_done[sec_index] = true;
		break;
	}

	return prog_fd;
}

static int bpf_fetch_prog_sec(Elf *elf_fd, GElf_Ehdr *elf_hdr, bool *sec_done,
			      enum bpf_prog_type type, const char *sec,
			      const char *license, Elf_Data *sym_tab)
{
	int ret = -1;

	if (sym_tab)
		ret = bpf_fetch_prog_relo(elf_fd, elf_hdr, sec_done, type,
					  sec, license, sym_tab);
	if (ret < 0)
		ret = bpf_fetch_prog(elf_fd, elf_hdr, sec_done, type, sec,
				     license);
	return ret;
}

static int bpf_fill_prog_arrays(Elf *elf_fd, GElf_Ehdr *elf_hdr, bool *sec_done,
				enum bpf_prog_type type, const char *license,
				Elf_Data *sym_tab)
{
	int sec_index;

	for (sec_index = 1; sec_index < elf_hdr->e_shnum; sec_index++) {
		struct bpf_elf_sec_data data_insn;
		int ret, map_id, key_id, prog_fd;

		if (sec_done[sec_index])
			continue;

		ret = bpf_fill_section_data(elf_fd, elf_hdr, sec_index,
					    &data_insn);
		if (ret < 0)
			continue;

		ret = sscanf(data_insn.sec_name, "%i/%i", &map_id, &key_id);
		if (ret != 2)
			continue;

		if (map_id >= ARRAY_SIZE(map_fds) || map_fds[map_id] < 0)
			return -ENOENT;
		if (map_ent[map_id].type != BPF_MAP_TYPE_PROG_ARRAY ||
		    map_ent[map_id].max_elem <= key_id)
			return -EINVAL;

		prog_fd = bpf_fetch_prog_sec(elf_fd, elf_hdr, sec_done,
					     type, data_insn.sec_name,
					     license, sym_tab);
		if (prog_fd < 0)
			return -EIO;

		ret = bpf_update_map(map_fds[map_id], &key_id, &prog_fd,
				     BPF_ANY);
		if (ret < 0)
			return -ENOENT;

		sec_done[sec_index] = true;
	}

	return 0;
}

int bpf_open_object(const char *path, enum bpf_prog_type type,
		    const char *sec, bool verbose)
{
	char license[ELF_MAX_LICENSE_LEN];
	int file_fd, prog_fd = -1, ret;
	Elf_Data *sym_tab = NULL;
	GElf_Ehdr elf_hdr;
	bool *sec_done;
	Elf *elf_fd;

	if (elf_version(EV_CURRENT) == EV_NONE)
		return -EINVAL;

	file_fd = open(path, O_RDONLY, 0);
	if (file_fd < 0)
		return -errno;

	elf_fd = elf_begin(file_fd, ELF_C_READ, NULL);
	if (!elf_fd) {
		ret = -EINVAL;
		goto out;
	}

	if (gelf_getehdr(elf_fd, &elf_hdr) != &elf_hdr) {
		ret = -EIO;
		goto out_elf;
	}

	sec_done = calloc(elf_hdr.e_shnum, sizeof(*sec_done));
	if (!sec_done) {
		ret = -ENOMEM;
		goto out_elf;
	}

	memset(license, 0, sizeof(license));
	bpf_verbose = verbose;

	if (!bpf_may_skip_map_creation(file_fd))
		bpf_maps_init();

	ret = bpf_fetch_ancillary(file_fd, elf_fd, &elf_hdr, sec_done,
				  license, sizeof(license), &sym_tab);
	if (ret < 0)
		goto out_maps;

	prog_fd = bpf_fetch_prog_sec(elf_fd, &elf_hdr, sec_done, type,
				     sec, license, sym_tab);
	if (prog_fd < 0)
		goto out_maps;

	if (!bpf_may_skip_map_creation(file_fd)) {
		ret = bpf_fill_prog_arrays(elf_fd, &elf_hdr, sec_done,
					   type, license, sym_tab);
		if (ret < 0)
			goto out_prog;
	}

	bpf_save_finfo(file_fd);

	free(sec_done);

	elf_end(elf_fd);
	close(file_fd);

	return prog_fd;

out_prog:
	close(prog_fd);
out_maps:
	bpf_maps_destroy();
	free(sec_done);
out_elf:
	elf_end(elf_fd);
out:
	close(file_fd);
	bpf_clear_finfo();
	return prog_fd;
}

static int
bpf_map_set_send(int fd, struct sockaddr_un *addr, unsigned int addr_len,
		 const struct bpf_map_data *aux, unsigned int entries)
{
	struct bpf_map_set_msg msg;
	int *cmsg_buf, min_fd;
	char *amsg_buf;
	int i;

	memset(&msg, 0, sizeof(msg));

	msg.aux.uds_ver = BPF_SCM_AUX_VER;
	msg.aux.num_ent = entries;

	strncpy(msg.aux.obj_name, aux->obj, sizeof(msg.aux.obj_name));
	memcpy(&msg.aux.obj_st, aux->st, sizeof(msg.aux.obj_st));

	cmsg_buf = bpf_map_set_init(&msg, addr, addr_len);
	amsg_buf = (char *)msg.aux.ent;

	for (i = 0; i < entries; i += min_fd) {
		int ret;

		min_fd = min(BPF_SCM_MAX_FDS * 1U, entries - i);
		bpf_map_set_init_single(&msg, min_fd);

		memcpy(cmsg_buf, &aux->fds[i], sizeof(aux->fds[0]) * min_fd);
		memcpy(amsg_buf, &aux->ent[i], sizeof(aux->ent[0]) * min_fd);

		ret = sendmsg(fd, &msg.hdr, 0);
		if (ret <= 0)
			return ret ? : -1;
	}

	return 0;
}

static int
bpf_map_set_recv(int fd, int *fds,  struct bpf_map_aux *aux,
		 unsigned int entries)
{
	struct bpf_map_set_msg msg;
	int *cmsg_buf, min_fd;
	char *amsg_buf, *mmsg_buf;
	unsigned int needed = 1;
	int i;

	cmsg_buf = bpf_map_set_init(&msg, NULL, 0);
	amsg_buf = (char *)msg.aux.ent;
	mmsg_buf = (char *)&msg.aux;

	for (i = 0; i < min(entries, needed); i += min_fd) {
		struct cmsghdr *cmsg;
		int ret;

		min_fd = min(entries, entries - i);
		bpf_map_set_init_single(&msg, min_fd);

		ret = recvmsg(fd, &msg.hdr, 0);
		if (ret <= 0)
			return ret ? : -1;

		cmsg = CMSG_FIRSTHDR(&msg.hdr);
		if (!cmsg || cmsg->cmsg_type != SCM_RIGHTS)
			return -EINVAL;
		if (msg.hdr.msg_flags & MSG_CTRUNC)
			return -EIO;
		if (msg.aux.uds_ver != BPF_SCM_AUX_VER)
			return -ENOSYS;

		min_fd = (cmsg->cmsg_len - sizeof(*cmsg)) / sizeof(fd);
		if (min_fd > entries || min_fd <= 0)
			return -EINVAL;

		memcpy(&fds[i], cmsg_buf, sizeof(fds[0]) * min_fd);
		memcpy(&aux->ent[i], amsg_buf, sizeof(aux->ent[0]) * min_fd);
		memcpy(aux, mmsg_buf, offsetof(struct bpf_map_aux, ent));

		needed = aux->num_ent;
	}

	return 0;
}

int bpf_send_map_fds(const char *path, const char *obj)
{
	struct sockaddr_un addr;
	struct bpf_map_data bpf_aux;
	int fd, ret;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot open socket: %s\n",
			strerror(errno));
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path));

	ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		fprintf(stderr, "Cannot connect to %s: %s\n",
			path, strerror(errno));
		return -1;
	}

	memset(&bpf_aux, 0, sizeof(bpf_aux));

	bpf_aux.fds = map_fds;
	bpf_aux.ent = map_ent;

	bpf_aux.obj = obj;
	bpf_aux.st = &bpf_st;

	ret = bpf_map_set_send(fd, &addr, sizeof(addr), &bpf_aux,
			       bpf_maps_count());
	if (ret < 0)
		fprintf(stderr, "Cannot send fds to %s: %s\n",
			path, strerror(errno));

	close(fd);
	return ret;
}

int bpf_recv_map_fds(const char *path, int *fds, struct bpf_map_aux *aux,
		     unsigned int entries)
{
	struct sockaddr_un addr;
	int fd, ret;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot open socket: %s\n",
			strerror(errno));
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path));

	ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		fprintf(stderr, "Cannot bind to socket: %s\n",
			strerror(errno));
		return -1;
	}

	ret = bpf_map_set_recv(fd, fds, aux, entries);
	if (ret < 0)
		fprintf(stderr, "Cannot recv fds from %s: %s\n",
			path, strerror(errno));

	unlink(addr.sun_path);
	close(fd);
	return ret;
}
#endif /* HAVE_ELF */
