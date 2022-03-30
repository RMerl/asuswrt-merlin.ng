// SPDX-License-Identifier: GPL-2.0+
/*
 * FSL PAMU driver
 *
 * Copyright 2012-2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/log2.h>
#include <malloc.h>
#include <asm/fsl_pamu.h>

struct paace *ppaact;
struct paace *sec;
unsigned long fspi;

static inline int __ilog2_roundup_64(uint64_t val)
{
	if ((val & (val - 1)) == 0)
		return __ilog2_u64(val);
	else
		return  __ilog2_u64(val) + 1;
}


static inline int count_lsb_zeroes(unsigned long val)
{
	return ffs(val) - 1;
}

static unsigned int map_addrspace_size_to_wse(uint64_t addrspace_size)
{
	/* window size is 2^(WSE+1) bytes */
	return count_lsb_zeroes(addrspace_size >> PAMU_PAGE_SHIFT) +
		PAMU_PAGE_SHIFT - 1;
}

static unsigned int map_subwindow_cnt_to_wce(uint32_t subwindow_cnt)
{
       /* window count is 2^(WCE+1) bytes */
	return count_lsb_zeroes(subwindow_cnt) - 1;
}

static void pamu_setup_default_xfer_to_host_ppaace(struct paace *ppaace)
{
	set_bf(ppaace->addr_bitfields, PAACE_AF_PT, PAACE_PT_PRIMARY);
	set_bf(ppaace->domain_attr.to_host.coherency_required, PAACE_DA_HOST_CR,
	       PAACE_M_COHERENCE_REQ);
}

static void pamu_setup_default_xfer_to_host_spaace(struct paace *spaace)
{
	set_bf(spaace->addr_bitfields, PAACE_AF_PT, PAACE_PT_SECONDARY);
	set_bf(spaace->domain_attr.to_host.coherency_required, PAACE_DA_HOST_CR,
	       PAACE_M_COHERENCE_REQ);
}

/** Sets up PPAACE entry for specified liodn
 *
 * @param[in] liodn      Logical IO device number
 * @param[in] win_addr   starting address of DSA window
 * @param[in] win-size   size of DSA window
 * @param[in] omi        Operation mapping index -- if ~omi == 0 then omi
				not defined
 * @param[in] stashid    cache stash id for associated cpu -- if ~stashid == 0
				then stashid not defined
 * @param[in] snoopid    snoop id for hardware coherency -- if ~snoopid == 0
				then snoopid not defined
 * @param[in] subwin_cnt number of sub-windows
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
static int pamu_config_ppaace(uint32_t liodn, uint64_t win_addr,
	uint64_t win_size, uint32_t omi,
	uint32_t snoopid, uint32_t stashid,
	uint32_t subwin_cnt)
{
	struct paace *ppaace;

	if ((win_size & (win_size - 1)) || win_size < PAMU_PAGE_SIZE)
		return -1;

	if (win_addr & (win_size - 1))
		return -2;

	if (liodn > NUM_PPAACT_ENTRIES) {
		printf("Entries in PPACT not sufficient\n");
		return -3;
	}

	ppaace = &ppaact[liodn];

	/* window size is 2^(WSE+1) bytes */
	set_bf(ppaace->addr_bitfields, PPAACE_AF_WSE,
	       map_addrspace_size_to_wse(win_size));

	pamu_setup_default_xfer_to_host_ppaace(ppaace);

	if (sizeof(phys_addr_t) > 4)
		ppaace->wbah = (u64)win_addr >> (PAMU_PAGE_SHIFT + 20);
	else
		ppaace->wbah = 0;

	set_bf(ppaace->addr_bitfields, PPAACE_AF_WBAL,
	       (win_addr >> PAMU_PAGE_SHIFT));

	/* set up operation mapping if it's configured */
	if (omi < OME_NUMBER_ENTRIES) {
		set_bf(ppaace->impl_attr, PAACE_IA_OTM, PAACE_OTM_INDEXED);
		ppaace->op_encode.index_ot.omi = omi;
	} else if (~omi != 0) {
		return -3;
	}

	/* configure stash id */
	if (~stashid != 0)
		set_bf(ppaace->impl_attr, PAACE_IA_CID, stashid);

	/* configure snoop id */
	if (~snoopid != 0)
		ppaace->domain_attr.to_host.snpid = snoopid;

	if (subwin_cnt) {
		/* window count is 2^(WCE+1) bytes */
		set_bf(ppaace->impl_attr, PAACE_IA_WCE,
		       map_subwindow_cnt_to_wce(subwin_cnt));
		set_bf(ppaace->addr_bitfields, PPAACE_AF_MW, 0x1);
		ppaace->fspi = fspi;
		fspi = fspi + DEFAULT_NUM_SUBWINDOWS - 1;
	} else {
		set_bf(ppaace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_ALL);
	}

	sync();
	/* Mark the ppace entry valid */
	ppaace->addr_bitfields |= PAACE_V_VALID;
	sync();

	return 0;
}

static int pamu_config_spaace(uint32_t liodn,
	uint64_t subwin_size, uint64_t subwin_addr, uint64_t size,
	uint32_t omi, uint32_t snoopid, uint32_t stashid)
{
	struct paace *paace;
	/* Align start addr of subwin to subwindoe size */
	uint64_t sec_addr = subwin_addr & ~(subwin_size - 1);
	uint64_t end_addr = subwin_addr + size;
	int size_shift = __ilog2_u64(subwin_size);
	uint64_t win_size = 0;
	uint32_t index, swse;
	unsigned long fspi_idx;

	/* Recalculate the size */
	size = end_addr - sec_addr;

	if (!subwin_size)
		return -1;

	if (liodn > NUM_PPAACT_ENTRIES) {
		printf("LIODN No programmed %d > no. of PPAACT entries %d\n",
		       liodn, NUM_PPAACT_ENTRIES);
		return -1;
	}

	while (sec_addr < end_addr) {
		debug("sec_addr < end_addr is %llx < %llx\n", sec_addr,
		      end_addr);
		paace = &ppaact[liodn];
		if (!paace)
			return -1;
		fspi_idx = paace->fspi;

		/* Calculating the win_size here as if we map in index 0,
			paace entry woudl need to  be programmed for SWSE */
		win_size = end_addr - sec_addr;
		win_size = 1 << __ilog2_roundup_64(win_size);

		if (win_size > subwin_size)
			win_size = subwin_size;
		else if (win_size < PAMU_PAGE_SIZE)
			win_size = PAMU_PAGE_SIZE;

		debug("win_size is %llx\n", win_size);

		swse = map_addrspace_size_to_wse(win_size);
		index = sec_addr >> size_shift;

		if (index == 0) {
			set_bf(paace->win_bitfields, PAACE_WIN_SWSE, swse);
			set_bf(paace->addr_bitfields, PAACE_AF_AP,
			       PAACE_AP_PERMS_ALL);
			sec_addr += subwin_size;
			continue;
		}

		paace = sec + fspi_idx + index - 1;

		debug("SPAACT:Writing at location %p, index %d\n", paace,
		      index);

		pamu_setup_default_xfer_to_host_spaace(paace);
		set_bf(paace->addr_bitfields, SPAACE_AF_LIODN, liodn);
		set_bf(paace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_ALL);

		/* configure snoop id */
		if (~snoopid != 0)
			paace->domain_attr.to_host.snpid = snoopid;

		if (paace->addr_bitfields & PAACE_V_VALID) {
			debug("Reached overlap condition\n");
			debug("%d < %d\n", get_bf(paace->win_bitfields,
						  PAACE_WIN_SWSE), swse);
			if (get_bf(paace->win_bitfields, PAACE_WIN_SWSE) < swse)
				set_bf(paace->win_bitfields, PAACE_WIN_SWSE,
				       swse);
		} else {
			set_bf(paace->win_bitfields, PAACE_WIN_SWSE, swse);
		}

		paace->addr_bitfields |= PAACE_V_VALID;
		sec_addr += subwin_size;
	}

	return 0;
}

int pamu_init(void)
{
	u32 base_addr = CONFIG_SYS_PAMU_ADDR;
	struct ccsr_pamu *regs;
	u32 i = 0;
	u64 ppaact_phys, ppaact_lim, ppaact_size;
	u64 spaact_phys, spaact_lim, spaact_size;

	ppaact_size = sizeof(struct paace) * NUM_PPAACT_ENTRIES;
	spaact_size = sizeof(struct paace) * NUM_SPAACT_ENTRIES;

	/* Allocate space for Primary PAACT Table */
#if (defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_PPAACT_ADDR))
	ppaact = (void *)CONFIG_SPL_PPAACT_ADDR;
#else
	ppaact = memalign(PAMU_TABLE_ALIGNMENT, ppaact_size);
	if (!ppaact)
		return -1;
#endif
	memset(ppaact, 0, ppaact_size);

	/* Allocate space for Secondary PAACT Table */
#if (defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_SPAACT_ADDR))
	sec = (void *)CONFIG_SPL_SPAACT_ADDR;
#else
	sec = memalign(PAMU_TABLE_ALIGNMENT, spaact_size);
	if (!sec)
		return -1;
#endif
	memset(sec, 0, spaact_size);

	ppaact_phys = virt_to_phys((void *)ppaact);
	ppaact_lim = ppaact_phys + ppaact_size;

	spaact_phys = (uint64_t)virt_to_phys((void *)sec);
	spaact_lim = spaact_phys + spaact_size;

	/* Configure all PAMU's */
	for (i = 0; i < CONFIG_NUM_PAMU; i++) {
		regs = (struct ccsr_pamu *)base_addr;

		out_be32(&regs->ppbah, ppaact_phys >> 32);
		out_be32(&regs->ppbal, (uint32_t)ppaact_phys);

		out_be32(&regs->pplah, (ppaact_lim) >> 32);
		out_be32(&regs->pplal, (uint32_t)ppaact_lim);

		if (sec != NULL) {
			out_be32(&regs->spbah, spaact_phys >> 32);
			out_be32(&regs->spbal, (uint32_t)spaact_phys);
			out_be32(&regs->splah, spaact_lim >> 32);
			out_be32(&regs->splal, (uint32_t)spaact_lim);
		}
		sync();

		base_addr += PAMU_OFFSET;
	}

	return 0;
}

void pamu_enable(void)
{
	u32 i = 0;
	u32 base_addr = CONFIG_SYS_PAMU_ADDR;
	for (i = 0; i < CONFIG_NUM_PAMU; i++) {
		setbits_be32((void *)base_addr + PAMU_PCR_OFFSET,
			     PAMU_PCR_PE);
		sync();
		base_addr += PAMU_OFFSET;
	}
}

void pamu_reset(void)
{
	u32 i  = 0;
	u32 base_addr = CONFIG_SYS_PAMU_ADDR;
	struct ccsr_pamu *regs;

	for (i = 0; i < CONFIG_NUM_PAMU; i++) {
		regs = (struct ccsr_pamu *)base_addr;
	/* Clear PPAACT Base register */
		out_be32(&regs->ppbah, 0);
		out_be32(&regs->ppbal, 0);
		out_be32(&regs->pplah, 0);
		out_be32(&regs->pplal, 0);
		out_be32(&regs->spbah, 0);
		out_be32(&regs->spbal, 0);
		out_be32(&regs->splah, 0);
		out_be32(&regs->splal, 0);

		clrbits_be32((void *)regs + PAMU_PCR_OFFSET, PAMU_PCR_PE);
		sync();
		base_addr += PAMU_OFFSET;
	}
}

void pamu_disable(void)
{
	u32 i  = 0;
	u32 base_addr = CONFIG_SYS_PAMU_ADDR;


	for (i = 0; i < CONFIG_NUM_PAMU; i++) {
		clrbits_be32((void *)base_addr + PAMU_PCR_OFFSET, PAMU_PCR_PE);
		sync();
		base_addr += PAMU_OFFSET;
	}
}


static uint64_t find_max(uint64_t arr[], int num)
{
	int i = 0;
	int max = 0;
	for (i = 1 ; i < num; i++)
		if (arr[max] < arr[i])
			max = i;

	return arr[max];
}

static uint64_t find_min(uint64_t arr[], int num)
{
	int i = 0;
	int min = 0;
	for (i = 1 ; i < num; i++)
		if (arr[min] > arr[i])
			min = i;

	return arr[min];
}

static uint32_t get_win_cnt(uint64_t size)
{
	uint32_t win_cnt = DEFAULT_NUM_SUBWINDOWS;

	while (win_cnt && (size/win_cnt) < PAMU_PAGE_SIZE)
		win_cnt >>= 1;

	return win_cnt;
}

int config_pamu(struct pamu_addr_tbl *tbl, int num_entries, uint32_t liodn)
{
	int i = 0;
	int ret = 0;
	uint32_t num_sec_windows = 0;
	uint32_t num_windows = 0;
	uint64_t min_addr, max_addr;
	uint64_t size;
	uint64_t subwin_size;
	int sizebit;

	min_addr = find_min(tbl->start_addr, num_entries);
	max_addr = find_max(tbl->end_addr, num_entries);
	size = max_addr - min_addr + 1;

	if (!size)
		return -1;

	sizebit = __ilog2_roundup_64(size);
	size = 1 << sizebit;
	debug("min start_addr is %llx\n", min_addr);
	debug("max end_addr is %llx\n", max_addr);
	debug("size found is  %llx\n", size);

	if (size < PAMU_PAGE_SIZE)
		size = PAMU_PAGE_SIZE;

	while (1) {
		min_addr = min_addr & ~(size - 1);
		if (min_addr + size > max_addr)
			break;
		size <<= 1;
		if (!size)
			return -1;
	}
	debug("PAACT :Base addr is %llx\n", min_addr);
	debug("PAACT : Size is %llx\n", size);
	num_windows = get_win_cnt(size);
	/* For a single window, no spaact entries are required
	 * sec_sub_window count = 0 */
	if (num_windows > 1)
		num_sec_windows = num_windows;
	else
		num_sec_windows = 0;

	ret = pamu_config_ppaace(liodn, min_addr,
			size , -1, -1, -1, num_sec_windows);

	if (ret < 0)
		return ret;

	debug("configured ppace\n");

	if (num_sec_windows) {
		subwin_size = size >> count_lsb_zeroes(num_sec_windows);
		debug("subwin_size is %llx\n", subwin_size);

		for (i = 0; i < num_entries; i++) {
			ret = pamu_config_spaace(liodn,
				subwin_size, tbl->start_addr[i] - min_addr,
				tbl->size[i], -1, -1, -1);

			if (ret < 0)
				return ret;
		}
	}

	return ret;
}
