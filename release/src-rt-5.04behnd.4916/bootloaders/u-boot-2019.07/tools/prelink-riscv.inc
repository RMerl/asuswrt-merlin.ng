// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology
 * Chih-Mao Chen <cmchen@andestech.com>
 *
 * Statically process runtime relocations on RISC-V ELF images
 * so that it can be directly executed when loaded at LMA
 * without fixup. Both RV32 and RV64 are supported.
 */

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define CONCAT3(x, y, z) CONCAT(CONCAT(x, y), z)

#define prelink_nn      CONCAT(prelink, PRELINK_INC_BITS)
#define uintnn_t        CONCAT3(uint, PRELINK_INC_BITS, _t)
#define get_offset_nn   CONCAT(get_offset_, PRELINK_INC_BITS)
#define Elf_Ehdr        CONCAT3(Elf, PRELINK_INC_BITS, _Ehdr)
#define Elf_Phdr        CONCAT3(Elf, PRELINK_INC_BITS, _Phdr)
#define Elf_Rela        CONCAT3(Elf, PRELINK_INC_BITS, _Rela)
#define Elf_Sym         CONCAT3(Elf, PRELINK_INC_BITS, _Sym)
#define Elf_Dyn         CONCAT3(Elf, PRELINK_INC_BITS, _Dyn)
#define Elf_Addr        CONCAT3(Elf, PRELINK_INC_BITS, _Addr)
#define ELF_R_TYPE      CONCAT3(ELF, PRELINK_INC_BITS, _R_TYPE)
#define ELF_R_SYM       CONCAT3(ELF, PRELINK_INC_BITS, _R_SYM)

static void* get_offset_nn (void* data, Elf_Phdr* phdrs, size_t phnum, Elf_Addr addr)
{
	Elf_Phdr *p;

	for (p = phdrs; p < phdrs + phnum; ++p)
		if (p->p_vaddr <= addr && p->p_vaddr + p->p_memsz > addr)
			return data + p->p_offset + (addr - p->p_vaddr);

	return NULL;
}

static void prelink_nn(void *data)
{
	Elf_Ehdr *ehdr = data;
	Elf_Phdr *p;
	Elf_Dyn *dyn;
	Elf_Rela *r;

	if (ehdr->e_machine != EM_RISCV)
		die("Machine type is not RISC-V");

	Elf_Phdr *phdrs = data + ehdr->e_phoff;

	Elf_Dyn *dyns = NULL;
	for (p = phdrs; p < phdrs + ehdr->e_phnum; ++p) {
		if (p->p_type == PT_DYNAMIC) {
			dyns = data + p->p_offset;
			break;
		}
	}

	if (dyns == NULL)
		die("No dynamic section found");

	Elf_Rela *rela_dyn = NULL;
	size_t rela_count = 0;
	Elf_Sym *dynsym = NULL;
	for (dyn = dyns;; ++dyn) {
		if (dyn->d_tag == DT_NULL)
			break;
		else if (dyn->d_tag == DT_RELA)
			rela_dyn = get_offset_nn(data, phdrs, ehdr->e_phnum, + dyn->d_un.d_ptr);
		else if (dyn->d_tag == DT_RELASZ)
			rela_count = dyn->d_un.d_val / sizeof(Elf_Rela);
		else if (dyn->d_tag == DT_SYMTAB)
			dynsym = get_offset_nn(data, phdrs, ehdr->e_phnum, + dyn->d_un.d_ptr);

	}

	if (rela_dyn == NULL)
		die("No .rela.dyn found");

	if (dynsym == NULL)
		die("No .dynsym found");

	for (r = rela_dyn; r < rela_dyn + rela_count; ++r) {
		void* buf = get_offset_nn(data, phdrs, ehdr->e_phnum, r->r_offset);

		if (buf == NULL)
			continue;

		if (ELF_R_TYPE(r->r_info) == R_RISCV_RELATIVE)
			*((uintnn_t*) buf) = r->r_addend;
		else if (ELF_R_TYPE(r->r_info) == R_RISCV_32)
			*((uint32_t*) buf) = dynsym[ELF_R_SYM(r->r_info)].st_value;
		else if (ELF_R_TYPE(r->r_info) == R_RISCV_64)
			*((uint64_t*) buf) = dynsym[ELF_R_SYM(r->r_info)].st_value;
	}
}

#undef prelink_nn
#undef uintnn_t
#undef get_offset_nn
#undef Elf_Ehdr
#undef Elf_Phdr
#undef Elf_Rela
#undef Elf_Sym
#undef Elf_Dyn
#undef Elf_Addr
#undef ELF_R_TYPE
#undef ELF_R_SYM

#undef CONCAT_IMPL
#undef CONCAT
#undef CONCAT3
