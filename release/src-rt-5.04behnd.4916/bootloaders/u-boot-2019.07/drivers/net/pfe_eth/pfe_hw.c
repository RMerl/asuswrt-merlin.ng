// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */
#include <net/pfe_eth/pfe_eth.h>
#include <net/pfe_eth/pfe/pfe_hw.h>

static struct pe_info pe[MAX_PE];

/*
 * Initializes the PFE library.
 * Must be called before using any of the library functions.
 */
void pfe_lib_init(void)
{
	int pfe_pe_id;

	for (pfe_pe_id = CLASS0_ID; pfe_pe_id <= CLASS_MAX_ID; pfe_pe_id++) {
		pe[pfe_pe_id].dmem_base_addr =
			(u32)CLASS_DMEM_BASE_ADDR(pfe_pe_id);
		pe[pfe_pe_id].pmem_base_addr =
			(u32)CLASS_IMEM_BASE_ADDR(pfe_pe_id);
		pe[pfe_pe_id].pmem_size = (u32)CLASS_IMEM_SIZE;
		pe[pfe_pe_id].mem_access_wdata =
			(void *)CLASS_MEM_ACCESS_WDATA;
		pe[pfe_pe_id].mem_access_addr = (void *)CLASS_MEM_ACCESS_ADDR;
		pe[pfe_pe_id].mem_access_rdata = (void *)CLASS_MEM_ACCESS_RDATA;
	}

	for (pfe_pe_id = TMU0_ID; pfe_pe_id <= TMU_MAX_ID; pfe_pe_id++) {
		if (pfe_pe_id == TMU2_ID)
			continue;
		pe[pfe_pe_id].dmem_base_addr =
			(u32)TMU_DMEM_BASE_ADDR(pfe_pe_id - TMU0_ID);
		pe[pfe_pe_id].pmem_base_addr =
			(u32)TMU_IMEM_BASE_ADDR(pfe_pe_id - TMU0_ID);
		pe[pfe_pe_id].pmem_size = (u32)TMU_IMEM_SIZE;
		pe[pfe_pe_id].mem_access_wdata = (void *)TMU_MEM_ACCESS_WDATA;
		pe[pfe_pe_id].mem_access_addr = (void *)TMU_MEM_ACCESS_ADDR;
		pe[pfe_pe_id].mem_access_rdata = (void *)TMU_MEM_ACCESS_RDATA;
	}
}

/*
 * Writes a buffer to PE internal memory from the host
 * through indirect access registers.
 *
 * @param[in] id	       PE identification (CLASS0_ID, ..., TMU0_ID,
 *				..., UTIL_ID)
 * @param[in] mem_access_addr	DMEM destination address (must be 32bit
 *				aligned)
 * @param[in] src		Buffer source address
 * @param[in] len		Number of bytes to copy
 */
static void pe_mem_memcpy_to32(int id, u32 mem_access_addr, const void *src,
			       unsigned int len)
{
	u32 offset = 0, val, addr;
	unsigned int len32 = len >> 2;
	int i;

	addr = mem_access_addr | PE_MEM_ACCESS_WRITE |
		PE_MEM_ACCESS_BYTE_ENABLE(0, 4);

	for (i = 0; i < len32; i++, offset += 4, src += 4) {
		val = *(u32 *)src;
		writel(cpu_to_be32(val), pe[id].mem_access_wdata);
		writel(addr + offset, pe[id].mem_access_addr);
	}

	len = (len & 0x3);
	if (len) {
		val = 0;

		addr = (mem_access_addr | PE_MEM_ACCESS_WRITE |
			PE_MEM_ACCESS_BYTE_ENABLE(0, len)) + offset;

		for (i = 0; i < len; i++, src++)
			val |= (*(u8 *)src) << (8 * i);

		writel(cpu_to_be32(val), pe[id].mem_access_wdata);
		writel(addr, pe[id].mem_access_addr);
	}
}

/*
 * Writes a buffer to PE internal data memory (DMEM) from the host
 * through indirect access registers.
 * @param[in] id	PE identification (CLASS0_ID, ..., TMU0_ID,
 *			..., UTIL_ID)
 * @param[in] dst	DMEM destination address (must be 32bit
 *			aligned)
 * @param[in] src	Buffer source address
 * @param[in] len	Number of bytes to copy
 */
static void pe_dmem_memcpy_to32(int id, u32 dst, const void *src,
				unsigned int len)
{
	pe_mem_memcpy_to32(id, pe[id].dmem_base_addr | dst | PE_MEM_ACCESS_DMEM,
			   src, len);
}

/*
 * Writes a buffer to PE internal program memory (PMEM) from the host
 * through indirect access registers.
 * @param[in] id	PE identification (CLASS0_ID, ..., TMU0_ID,
 *			..., TMU3_ID)
 * @param[in] dst	PMEM destination address (must be 32bit
 *			aligned)
 * @param[in] src	Buffer source address
 * @param[in] len	Number of bytes to copy
 */
static void pe_pmem_memcpy_to32(int id, u32 dst, const void *src,
				unsigned int len)
{
	pe_mem_memcpy_to32(id, pe[id].pmem_base_addr | (dst & (pe[id].pmem_size
				- 1)) | PE_MEM_ACCESS_IMEM, src, len);
}

/*
 * Reads PE internal program memory (IMEM) from the host
 * through indirect access registers.
 * @param[in] id		PE identification (CLASS0_ID, ..., TMU0_ID,
 *				..., TMU3_ID)
 * @param[in] addr		PMEM read address (must be aligned on size)
 * @param[in] size		Number of bytes to read (maximum 4, must not
 *				cross 32bit boundaries)
 * @return			the data read (in PE endianness, i.e BE).
 */
u32 pe_pmem_read(int id, u32 addr, u8 size)
{
	u32 offset = addr & 0x3;
	u32 mask = 0xffffffff >> ((4 - size) << 3);
	u32 val;

	addr = pe[id].pmem_base_addr | ((addr & ~0x3) & (pe[id].pmem_size - 1))
		| PE_MEM_ACCESS_READ | PE_MEM_ACCESS_IMEM |
		PE_MEM_ACCESS_BYTE_ENABLE(offset, size);

	writel(addr, pe[id].mem_access_addr);
	val = be32_to_cpu(readl(pe[id].mem_access_rdata));

	return (val >> (offset << 3)) & mask;
}

/*
 * Writes PE internal data memory (DMEM) from the host
 * through indirect access registers.
 * @param[in] id	PE identification (CLASS0_ID, ..., TMU0_ID,
 *			..., UTIL_ID)
 * @param[in] val	Value to write (in PE endianness, i.e BE)
 * @param[in] addr	DMEM write address (must be aligned on size)
 * @param[in] size	Number of bytes to write (maximum 4, must not
 *			cross 32bit boundaries)
 */
void pe_dmem_write(int id, u32 val, u32 addr, u8 size)
{
	u32 offset = addr & 0x3;

	addr = pe[id].dmem_base_addr | (addr & ~0x3) | PE_MEM_ACCESS_WRITE |
		PE_MEM_ACCESS_DMEM | PE_MEM_ACCESS_BYTE_ENABLE(offset, size);

	/* Indirect access interface is byte swapping data being written */
	writel(cpu_to_be32(val << (offset << 3)), pe[id].mem_access_wdata);
	writel(addr, pe[id].mem_access_addr);
}

/*
 * Reads PE internal data memory (DMEM) from the host
 * through indirect access registers.
 * @param[in] id		PE identification (CLASS0_ID, ..., TMU0_ID,
 *				..., UTIL_ID)
 * @param[in] addr		DMEM read address (must be aligned on size)
 * @param[in] size		Number of bytes to read (maximum 4, must not
 *				cross 32bit boundaries)
 * @return			the data read (in PE endianness, i.e BE).
 */
u32 pe_dmem_read(int id, u32 addr, u8 size)
{
	u32 offset = addr & 0x3;
	u32 mask = 0xffffffff >> ((4 - size) << 3);
	u32 val;

	addr = pe[id].dmem_base_addr | (addr & ~0x3) | PE_MEM_ACCESS_READ |
		PE_MEM_ACCESS_DMEM | PE_MEM_ACCESS_BYTE_ENABLE(offset, size);

	writel(addr, pe[id].mem_access_addr);

	/* Indirect access interface is byte swapping data being read */
	val = be32_to_cpu(readl(pe[id].mem_access_rdata));

	return (val >> (offset << 3)) & mask;
}

/*
 * This function is used to write to CLASS internal bus peripherals (ccu,
 * pe-lem) from the host
 * through indirect access registers.
 * @param[in]	val	value to write
 * @param[in]	addr	Address to write to (must be aligned on size)
 * @param[in]	size	Number of bytes to write (1, 2 or 4)
 *
 */
static void class_bus_write(u32 val, u32 addr, u8 size)
{
	u32 offset = addr & 0x3;

	writel((addr & CLASS_BUS_ACCESS_BASE_MASK), CLASS_BUS_ACCESS_BASE);

	addr = (addr & ~CLASS_BUS_ACCESS_BASE_MASK) | PE_MEM_ACCESS_WRITE |
		(size << 24);

	writel(cpu_to_be32(val << (offset << 3)), CLASS_BUS_ACCESS_WDATA);
	writel(addr, CLASS_BUS_ACCESS_ADDR);
}

/*
 * Reads from CLASS internal bus peripherals (ccu, pe-lem) from the host
 * through indirect access registers.
 * @param[in] addr	Address to read from (must be aligned on size)
 * @param[in] size	Number of bytes to read (1, 2 or 4)
 * @return		the read data
 */
static u32 class_bus_read(u32 addr, u8 size)
{
	u32 offset = addr & 0x3;
	u32 mask = 0xffffffff >> ((4 - size) << 3);
	u32 val;

	writel((addr & CLASS_BUS_ACCESS_BASE_MASK), CLASS_BUS_ACCESS_BASE);

	addr = (addr & ~CLASS_BUS_ACCESS_BASE_MASK) | (size << 24);

	writel(addr, CLASS_BUS_ACCESS_ADDR);
	val = be32_to_cpu(readl(CLASS_BUS_ACCESS_RDATA));

	return (val >> (offset << 3)) & mask;
}

/*
 * Writes data to the cluster memory (PE_LMEM)
 * @param[in] dst	PE LMEM destination address (must be 32bit aligned)
 * @param[in] src	Buffer source address
 * @param[in] len	Number of bytes to copy
 */
static void class_pe_lmem_memcpy_to32(u32 dst, const void *src,
				      unsigned int len)
{
	u32 len32 = len >> 2;
	int i;

	for (i = 0; i < len32; i++, src += 4, dst += 4)
		class_bus_write(*(u32 *)src, dst, 4);

	if (len & 0x2) {
		class_bus_write(*(u16 *)src, dst, 2);
		src += 2;
		dst += 2;
	}

	if (len & 0x1) {
		class_bus_write(*(u8 *)src, dst, 1);
		src++;
		dst++;
	}
}

/*
 * Writes value to the cluster memory (PE_LMEM)
 * @param[in] dst	PE LMEM destination address (must be 32bit aligned)
 * @param[in] val	Value to write
 * @param[in] len	Number of bytes to write
 */
static void class_pe_lmem_memset(u32 dst, int val, unsigned int len)
{
	u32 len32 = len >> 2;
	int i;

	val = val | (val << 8) | (val << 16) | (val << 24);

	for (i = 0; i < len32; i++, dst += 4)
		class_bus_write(val, dst, 4);

	if (len & 0x2) {
		class_bus_write(val, dst, 2);
		dst += 2;
	}

	if (len & 0x1) {
		class_bus_write(val, dst, 1);
		dst++;
	}
}

/*
 * Reads data from the cluster memory (PE_LMEM)
 * @param[out] dst	pointer to the source buffer data are copied to
 * @param[in] len	length in bytes of the amount of data to read
 *			from cluster memory
 * @param[in] offset	offset in bytes in the cluster memory where data are
 *			read from
 */
void pe_lmem_read(u32 *dst, u32 len, u32 offset)
{
	u32 len32 = len >> 2;
	int i = 0;

	for (i = 0; i < len32; dst++, i++, offset += 4)
		*dst = class_bus_read(PE_LMEM_BASE_ADDR + offset, 4);

	if (len & 0x03)
		*dst = class_bus_read(PE_LMEM_BASE_ADDR + offset, (len & 0x03));
}

/*
 * Writes data to the cluster memory (PE_LMEM)
 * @param[in] src	pointer to the source buffer data are copied from
 * @param[in] len	length in bytes of the amount of data to write to the
 *				cluster memory
 * @param[in] offset	offset in bytes in the cluster memory where data are
 *				written to
 */
void pe_lmem_write(u32 *src, u32 len, u32 offset)
{
	u32 len32 = len >> 2;
	int i = 0;

	for (i = 0; i < len32; src++, i++, offset += 4)
		class_bus_write(*src, PE_LMEM_BASE_ADDR + offset, 4);

	if (len & 0x03)
		class_bus_write(*src, PE_LMEM_BASE_ADDR + offset, (len &
					0x03));
}

/*
 * Loads an elf section into pmem
 * Code needs to be at least 16bit aligned and only PROGBITS sections are
 * supported
 *
 * @param[in] id	PE identification (CLASS0_ID, ..., TMU0_ID, ...,
 *					TMU3_ID)
 * @param[in] data	pointer to the elf firmware
 * @param[in] shdr	pointer to the elf section header
 */
static int pe_load_pmem_section(int id, const void *data, Elf32_Shdr *shdr)
{
	u32 offset = be32_to_cpu(shdr->sh_offset);
	u32 addr = be32_to_cpu(shdr->sh_addr);
	u32 size = be32_to_cpu(shdr->sh_size);
	u32 type = be32_to_cpu(shdr->sh_type);

	if (((unsigned long)(data + offset) & 0x3) != (addr & 0x3)) {
		printf(
			"%s: load address(%x) and elf file address(%lx) don't have the same alignment\n",
			__func__, addr, (unsigned long)data + offset);

		return -1;
	}

	if (addr & 0x1) {
		printf("%s: load address(%x) is not 16bit aligned\n",
		       __func__, addr);
		return -1;
	}

	if (size & 0x1) {
		printf("%s: load size(%x) is not 16bit aligned\n", __func__,
		       size);
		return -1;
	}

		debug("pmem pe%d @%x len %d\n", id, addr, size);
	switch (type) {
	case SHT_PROGBITS:
		pe_pmem_memcpy_to32(id, addr, data + offset, size);
		break;

	default:
		printf("%s: unsupported section type(%x)\n", __func__, type);
		return -1;
	}

	return 0;
}

/*
 * Loads an elf section into dmem
 * Data needs to be at least 32bit aligned, NOBITS sections are correctly
 * initialized to 0
 *
 * @param[in] id	PE identification (CLASS0_ID, ..., TMU0_ID,
 *			..., UTIL_ID)
 * @param[in] data	pointer to the elf firmware
 * @param[in] shdr	pointer to the elf section header
 */
static int pe_load_dmem_section(int id, const void *data, Elf32_Shdr *shdr)
{
	u32 offset = be32_to_cpu(shdr->sh_offset);
	u32 addr = be32_to_cpu(shdr->sh_addr);
	u32 size = be32_to_cpu(shdr->sh_size);
	u32 type = be32_to_cpu(shdr->sh_type);
	u32 size32 = size >> 2;
	int i;

	if (((unsigned long)(data + offset) & 0x3) != (addr & 0x3)) {
		printf(
			"%s: load address(%x) and elf file address(%lx) don't have the same alignment\n",
			__func__, addr, (unsigned long)data + offset);

		return -1;
	}

	if (addr & 0x3) {
		printf("%s: load address(%x) is not 32bit aligned\n",
		       __func__, addr);
		return -1;
	}

	switch (type) {
	case SHT_PROGBITS:
		debug("dmem pe%d @%x len %d\n", id, addr, size);
		pe_dmem_memcpy_to32(id, addr, data + offset, size);
		break;

	case SHT_NOBITS:
		debug("dmem zero pe%d @%x len %d\n", id, addr, size);
		for (i = 0; i < size32; i++, addr += 4)
			pe_dmem_write(id, 0, addr, 4);

		if (size & 0x3)
			pe_dmem_write(id, 0, addr, size & 0x3);

		break;

	default:
		printf("%s: unsupported section type(%x)\n", __func__, type);
		return -1;
	}

	return 0;
}

/*
 * Loads an elf section into DDR
 * Data needs to be at least 32bit aligned, NOBITS sections are correctly
 *		initialized to 0
 *
 * @param[in] id	PE identification (CLASS0_ID, ..., TMU0_ID,
 *			..., UTIL_ID)
 * @param[in] data	pointer to the elf firmware
 * @param[in] shdr	pointer to the elf section header
 */
static int pe_load_ddr_section(int id, const void *data, Elf32_Shdr *shdr)
{
	u32 offset = be32_to_cpu(shdr->sh_offset);
	u32 addr = be32_to_cpu(shdr->sh_addr);
	u32 size = be32_to_cpu(shdr->sh_size);
	u32 type = be32_to_cpu(shdr->sh_type);
	u32 flags = be32_to_cpu(shdr->sh_flags);

	switch (type) {
	case SHT_PROGBITS:
		debug("ddr  pe%d @%x len %d\n", id, addr, size);
		if (flags & SHF_EXECINSTR) {
			if (id <= CLASS_MAX_ID) {
				/* DO the loading only once in DDR */
				if (id == CLASS0_ID) {
					debug(
						"%s: load address(%x) and elf file address(%lx) rcvd\n"
						, __func__, addr,
						(unsigned long)data + offset);
					if (((unsigned long)(data + offset)
						& 0x3) != (addr & 0x3)) {
						printf(
							"%s: load address(%x) and elf file address(%lx) don't have the same alignment\n",
							__func__, addr,
							(unsigned long)data +
							offset);

						return -1;
					}

					if (addr & 0x1) {
						printf(
							"%s: load address(%x) is not 16bit aligned\n"
							, __func__, addr);
						return -1;
					}

					if (size & 0x1) {
						printf(
							"%s: load length(%x) is not 16bit aligned\n"
							, __func__, size);
						return -1;
					}

					memcpy((void *)DDR_PFE_TO_VIRT(addr),
					       data + offset, size);
				}
			} else {
				printf(
					"%s: unsupported ddr section type(%x) for PE(%d)\n"
					, __func__, type, id);
				return -1;
			}

		} else {
			memcpy((void *)DDR_PFE_TO_VIRT(addr), data + offset,
			       size);
		}

		break;

	case SHT_NOBITS:
		debug("ddr zero pe%d @%x len %d\n", id, addr, size);
		memset((void *)DDR_PFE_TO_VIRT(addr), 0, size);

		break;

	default:
		printf("%s: unsupported section type(%x)\n", __func__, type);
		return -1;
	}

	return 0;
}

/*
 * Loads an elf section into pe lmem
 * Data needs to be at least 32bit aligned, NOBITS sections are correctly
 * initialized to 0
 *
 * @param[in] id	PE identification (CLASS0_ID,..., CLASS5_ID)
 * @param[in] data	pointer to the elf firmware
 * @param[in] shdr	pointer to the elf section header
 */
static int pe_load_pe_lmem_section(int id, const void *data, Elf32_Shdr *shdr)
{
	u32 offset = be32_to_cpu(shdr->sh_offset);
	u32 addr = be32_to_cpu(shdr->sh_addr);
	u32 size = be32_to_cpu(shdr->sh_size);
	u32 type = be32_to_cpu(shdr->sh_type);

	if (id > CLASS_MAX_ID) {
		printf("%s: unsupported pe-lmem section type(%x) for PE(%d)\n",
		       __func__, type, id);
		return -1;
	}

	if (((unsigned long)(data + offset) & 0x3) != (addr & 0x3)) {
		printf(
			"%s: load address(%x) and elf file address(%lx) don't have the same alignment\n",
			__func__, addr, (unsigned long)data + offset);

		return -1;
	}

	if (addr & 0x3) {
		printf("%s: load address(%x) is not 32bit aligned\n",
		       __func__, addr);
		return -1;
	}

	debug("lmem  pe%d @%x len %d\n", id, addr, size);

	switch (type) {
	case SHT_PROGBITS:
		class_pe_lmem_memcpy_to32(addr, data + offset, size);
		break;

	case SHT_NOBITS:
		class_pe_lmem_memset(addr, 0, size);
		break;

	default:
		printf("%s: unsupported section type(%x)\n", __func__, type);
		return -1;
	}

	return 0;
}

/*
 * Loads an elf section into a PE
 * For now only supports loading a section to dmem (all PE's), pmem (class and
 * tmu PE's), DDDR (util PE code)
 * @param[in] id PE identification (CLASS0_ID, ..., TMU0_ID,
 * ..., UTIL_ID)
 * @param[in] data	pointer to the elf firmware
 * @param[in] shdr	pointer to the elf section header
 */
int pe_load_elf_section(int id, const void *data, Elf32_Shdr *shdr)
{
	u32 addr = be32_to_cpu(shdr->sh_addr);
	u32 size = be32_to_cpu(shdr->sh_size);

	if (IS_DMEM(addr, size))
		return pe_load_dmem_section(id, data, shdr);
	else if (IS_PMEM(addr, size))
		return pe_load_pmem_section(id, data, shdr);
	else if (IS_PFE_LMEM(addr, size))
		return 0;
	else if (IS_PHYS_DDR(addr, size))
		return pe_load_ddr_section(id, data, shdr);
	else if (IS_PE_LMEM(addr, size))
		return pe_load_pe_lmem_section(id, data, shdr);

	printf("%s: unsupported memory range(%x)\n", __func__, addr);

	return 0;
}

/**************************** BMU ***************************/
/*
 * Resets a BMU block.
 * @param[in] base	BMU block base address
 */
static inline void bmu_reset(void *base)
{
	writel(CORE_SW_RESET, base + BMU_CTRL);

	/* Wait for self clear */
	while (readl(base + BMU_CTRL) & CORE_SW_RESET)
		;
}

/*
 * Enabled a BMU block.
 * @param[in] base	BMU block base address
 */
void bmu_enable(void *base)
{
	writel(CORE_ENABLE, base + BMU_CTRL);
}

/*
 * Disables a BMU block.
 * @param[in] base	BMU block base address
 */
static inline void bmu_disable(void *base)
{
	writel(CORE_DISABLE, base + BMU_CTRL);
}

/*
 * Sets the configuration of a BMU block.
 * @param[in] base	BMU block base address
 * @param[in] cfg	BMU configuration
 */
static inline void bmu_set_config(void *base, struct bmu_cfg *cfg)
{
	writel(cfg->baseaddr, base + BMU_UCAST_BASE_ADDR);
	writel(cfg->count & 0xffff, base + BMU_UCAST_CONFIG);
	writel(cfg->size & 0xffff, base + BMU_BUF_SIZE);

	/* Interrupts are never used */
	writel(0x0, base + BMU_INT_ENABLE);
}

/*
 * Initializes a BMU block.
 * @param[in] base	BMU block base address
 * @param[in] cfg	BMU configuration
 */
void bmu_init(void *base, struct bmu_cfg *cfg)
{
	bmu_disable(base);

	bmu_set_config(base, cfg);

	bmu_reset(base);
}

/**************************** GPI ***************************/
/*
 * Resets a GPI block.
 * @param[in] base	GPI base address
 */
static inline void gpi_reset(void *base)
{
	writel(CORE_SW_RESET, base + GPI_CTRL);
}

/*
 * Enables a GPI block.
 * @param[in] base	GPI base address
 */
void gpi_enable(void *base)
{
	writel(CORE_ENABLE, base + GPI_CTRL);
}

/*
 * Disables a GPI block.
 * @param[in] base	GPI base address
 */
void gpi_disable(void *base)
{
	writel(CORE_DISABLE, base + GPI_CTRL);
}

/*
 * Sets the configuration of a GPI block.
 * @param[in] base	GPI base address
 * @param[in] cfg	GPI configuration
 */
static inline void gpi_set_config(void *base, struct gpi_cfg *cfg)
{
	writel(CBUS_VIRT_TO_PFE(BMU1_BASE_ADDR + BMU_ALLOC_CTRL), base
	       + GPI_LMEM_ALLOC_ADDR);
	writel(CBUS_VIRT_TO_PFE(BMU1_BASE_ADDR + BMU_FREE_CTRL), base
	       + GPI_LMEM_FREE_ADDR);
	writel(CBUS_VIRT_TO_PFE(BMU2_BASE_ADDR + BMU_ALLOC_CTRL), base
	       + GPI_DDR_ALLOC_ADDR);
	writel(CBUS_VIRT_TO_PFE(BMU2_BASE_ADDR + BMU_FREE_CTRL), base
	       + GPI_DDR_FREE_ADDR);
	writel(CBUS_VIRT_TO_PFE(CLASS_INQ_PKTPTR), base + GPI_CLASS_ADDR);
	writel(DDR_HDR_SIZE, base + GPI_DDR_DATA_OFFSET);
	writel(LMEM_HDR_SIZE, base + GPI_LMEM_DATA_OFFSET);
	writel(0, base + GPI_LMEM_SEC_BUF_DATA_OFFSET);
	writel(0, base + GPI_DDR_SEC_BUF_DATA_OFFSET);
	writel((DDR_HDR_SIZE << 16) | LMEM_HDR_SIZE, base + GPI_HDR_SIZE);
	writel((DDR_BUF_SIZE << 16) | LMEM_BUF_SIZE, base + GPI_BUF_SIZE);

	writel(((cfg->lmem_rtry_cnt << 16) | (GPI_DDR_BUF_EN << 1) |
		GPI_LMEM_BUF_EN), base + GPI_RX_CONFIG);
	writel(cfg->tmlf_txthres, base + GPI_TMLF_TX);
	writel(cfg->aseq_len, base + GPI_DTX_ASEQ);

	/*Make GPI AXI transactions non-bufferable */
	writel(0x1, base + GPI_AXI_CTRL);
}

/*
 * Initializes a GPI block.
 * @param[in] base	GPI base address
 * @param[in] cfg	GPI configuration
 */
void gpi_init(void *base, struct gpi_cfg *cfg)
{
	gpi_reset(base);

	gpi_disable(base);

	gpi_set_config(base, cfg);
}

/**************************** CLASSIFIER ***************************/
/*
 * Resets CLASSIFIER block.
 */
static inline void class_reset(void)
{
	writel(CORE_SW_RESET, CLASS_TX_CTRL);
}

/*
 * Enables all CLASS-PE's cores.
 */
void class_enable(void)
{
	writel(CORE_ENABLE, CLASS_TX_CTRL);
}

/*
 * Disables all CLASS-PE's cores.
 */
void class_disable(void)
{
	writel(CORE_DISABLE, CLASS_TX_CTRL);
}

/*
 * Sets the configuration of the CLASSIFIER block.
 * @param[in] cfg	CLASSIFIER configuration
 */
static inline void class_set_config(struct class_cfg *cfg)
{
	if (PLL_CLK_EN == 0) {
		/* Clock ratio: for 1:1 the value is 0 */
		writel(0x0, CLASS_PE_SYS_CLK_RATIO);
	} else {
		/* Clock ratio: for 1:2 the value is 1 */
		writel(0x1, CLASS_PE_SYS_CLK_RATIO);
	}
	writel((DDR_HDR_SIZE << 16) | LMEM_HDR_SIZE, CLASS_HDR_SIZE);
	writel(LMEM_BUF_SIZE, CLASS_LMEM_BUF_SIZE);
	writel(CLASS_ROUTE_ENTRY_SIZE(CLASS_ROUTE_SIZE) |
		CLASS_ROUTE_HASH_SIZE(cfg->route_table_hash_bits),
		CLASS_ROUTE_HASH_ENTRY_SIZE);
	writel(HASH_CRC_PORT_IP | QB2BUS_LE, CLASS_ROUTE_MULTI);

	writel(cfg->route_table_baseaddr, CLASS_ROUTE_TABLE_BASE);
	memset((void *)DDR_PFE_TO_VIRT(cfg->route_table_baseaddr), 0,
	       ROUTE_TABLE_SIZE);

	writel(CLASS_PE0_RO_DM_ADDR0_VAL, CLASS_PE0_RO_DM_ADDR0);
	writel(CLASS_PE0_RO_DM_ADDR1_VAL, CLASS_PE0_RO_DM_ADDR1);
	writel(CLASS_PE0_QB_DM_ADDR0_VAL, CLASS_PE0_QB_DM_ADDR0);
	writel(CLASS_PE0_QB_DM_ADDR1_VAL, CLASS_PE0_QB_DM_ADDR1);
	writel(CBUS_VIRT_TO_PFE(TMU_PHY_INQ_PKTPTR), CLASS_TM_INQ_ADDR);

	writel(23, CLASS_AFULL_THRES);
	writel(23, CLASS_TSQ_FIFO_THRES);

	writel(24, CLASS_MAX_BUF_CNT);
	writel(24, CLASS_TSQ_MAX_CNT);

	/*Make Class AXI transactions non-bufferable */
	writel(0x1, CLASS_AXI_CTRL);

	/*Make Util AXI transactions non-bufferable */
	/*Util is disabled in U-boot, do it from here */
	writel(0x1, UTIL_AXI_CTRL);
}

/*
 * Initializes CLASSIFIER block.
 * @param[in] cfg	CLASSIFIER configuration
 */
void class_init(struct class_cfg *cfg)
{
	class_reset();

	class_disable();

	class_set_config(cfg);
}

/**************************** TMU ***************************/
/*
 * Enables TMU-PE cores.
 * @param[in] pe_mask	TMU PE mask
 */
void tmu_enable(u32 pe_mask)
{
	writel(readl(TMU_TX_CTRL) | (pe_mask & 0xF), TMU_TX_CTRL);
}

/*
 * Disables TMU cores.
 * @param[in] pe_mask	TMU PE mask
 */
void tmu_disable(u32 pe_mask)
{
	writel(readl(TMU_TX_CTRL) & ~(pe_mask & 0xF), TMU_TX_CTRL);
}

/*
 * Initializes TMU block.
 * @param[in] cfg	TMU configuration
 */
void tmu_init(struct tmu_cfg *cfg)
{
	int q, phyno;

	/* keep in soft reset */
	writel(SW_RESET, TMU_CTRL);

	/*Make Class AXI transactions non-bufferable */
	writel(0x1, TMU_AXI_CTRL);

	/* enable EMAC PHY ports */
	writel(0x3, TMU_SYS_GENERIC_CONTROL);

	writel(750, TMU_INQ_WATERMARK);

	writel(CBUS_VIRT_TO_PFE(EGPI1_BASE_ADDR + GPI_INQ_PKTPTR),
	       TMU_PHY0_INQ_ADDR);
	writel(CBUS_VIRT_TO_PFE(EGPI2_BASE_ADDR + GPI_INQ_PKTPTR),
	       TMU_PHY1_INQ_ADDR);

	writel(CBUS_VIRT_TO_PFE(HGPI_BASE_ADDR + GPI_INQ_PKTPTR),
	       TMU_PHY3_INQ_ADDR);
	writel(CBUS_VIRT_TO_PFE(HIF_NOCPY_RX_INQ0_PKTPTR), TMU_PHY4_INQ_ADDR);
	writel(CBUS_VIRT_TO_PFE(UTIL_INQ_PKTPTR), TMU_PHY5_INQ_ADDR);
	writel(CBUS_VIRT_TO_PFE(BMU2_BASE_ADDR + BMU_FREE_CTRL),
	       TMU_BMU_INQ_ADDR);

	/* enabling all 10 schedulers [9:0] of each TDQ  */
	writel(0x3FF, TMU_TDQ0_SCH_CTRL);
	writel(0x3FF, TMU_TDQ1_SCH_CTRL);
	writel(0x3FF, TMU_TDQ3_SCH_CTRL);

	if (PLL_CLK_EN == 0) {
		/* Clock ratio: for 1:1 the value is 0 */
		writel(0x0, TMU_PE_SYS_CLK_RATIO);
	} else {
		/* Clock ratio: for 1:2 the value is 1 */
		writel(0x1, TMU_PE_SYS_CLK_RATIO);
	}

	/* Extra packet pointers will be stored from this address onwards */
	debug("TMU_LLM_BASE_ADDR %x\n", cfg->llm_base_addr);
	writel(cfg->llm_base_addr, TMU_LLM_BASE_ADDR);

	debug("TMU_LLM_QUE_LEN %x\n", cfg->llm_queue_len);
	writel(cfg->llm_queue_len,	TMU_LLM_QUE_LEN);

	writel(5, TMU_TDQ_IIFG_CFG);
	writel(DDR_BUF_SIZE, TMU_BMU_BUF_SIZE);

	writel(0x0, TMU_CTRL);

	/* MEM init */
	writel(MEM_INIT, TMU_CTRL);

	while (!(readl(TMU_CTRL) & MEM_INIT_DONE))
		;

	/* LLM init */
	writel(LLM_INIT, TMU_CTRL);

	while (!(readl(TMU_CTRL) & LLM_INIT_DONE))
		;

	/* set up each queue for tail drop */
	for (phyno = 0; phyno < 4; phyno++) {
		if (phyno == 2)
			continue;
		for (q = 0; q < 16; q++) {
			u32 qmax;

			writel((phyno << 8) | q, TMU_TEQ_CTRL);
			writel(BIT(22), TMU_TEQ_QCFG);

			if (phyno == 3)
				qmax = DEFAULT_TMU3_QDEPTH;
			else
				qmax = (q == 0) ? DEFAULT_Q0_QDEPTH :
					DEFAULT_MAX_QDEPTH;

			writel(qmax << 18, TMU_TEQ_HW_PROB_CFG2);
			writel(qmax >> 14, TMU_TEQ_HW_PROB_CFG3);
		}
	}
	writel(0x05, TMU_TEQ_DISABLE_DROPCHK);
	writel(0, TMU_CTRL);
}

/**************************** HIF ***************************/
/*
 * Enable hif tx DMA and interrupt
 */
void hif_tx_enable(void)
{
	writel(HIF_CTRL_DMA_EN, HIF_TX_CTRL);
}

/*
 * Disable hif tx DMA and interrupt
 */
void hif_tx_disable(void)
{
	u32 hif_int;

	writel(0, HIF_TX_CTRL);

	hif_int = readl(HIF_INT_ENABLE);
	hif_int &= HIF_TXPKT_INT_EN;
	writel(hif_int, HIF_INT_ENABLE);
}

/*
 * Enable hif rx DMA and interrupt
 */
void hif_rx_enable(void)
{
	writel((HIF_CTRL_DMA_EN | HIF_CTRL_BDP_CH_START_WSTB), HIF_RX_CTRL);
}

/*
 * Disable hif rx DMA and interrupt
 */
void hif_rx_disable(void)
{
	u32 hif_int;

	writel(0, HIF_RX_CTRL);

	hif_int = readl(HIF_INT_ENABLE);
	hif_int &= HIF_RXPKT_INT_EN;
	writel(hif_int, HIF_INT_ENABLE);
}

/*
 * Initializes HIF copy block.
 */
void hif_init(void)
{
	/* Initialize HIF registers */
	writel(HIF_RX_POLL_CTRL_CYCLE << 16 | HIF_TX_POLL_CTRL_CYCLE,
	       HIF_POLL_CTRL);
	/* Make HIF AXI transactions non-bufferable */
	writel(0x1, HIF_AXI_CTRL);
}
