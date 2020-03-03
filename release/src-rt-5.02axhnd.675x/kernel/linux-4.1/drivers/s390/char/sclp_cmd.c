/*
 * Copyright IBM Corp. 2007,2012
 *
 * Author(s): Heiko Carstens <heiko.carstens@de.ibm.com>,
 *	      Peter Oberparleiter <peter.oberparleiter@de.ibm.com>
 */

#define KMSG_COMPONENT "sclp_cmd"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/completion.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/memory.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <asm/ctl_reg.h>
#include <asm/chpid.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/sclp.h>

#include "sclp.h"

static void sclp_sync_callback(struct sclp_req *req, void *data)
{
	struct completion *completion = data;

	complete(completion);
}

int sclp_sync_request(sclp_cmdw_t cmd, void *sccb)
{
	return sclp_sync_request_timeout(cmd, sccb, 0);
}

int sclp_sync_request_timeout(sclp_cmdw_t cmd, void *sccb, int timeout)
{
	struct completion completion;
	struct sclp_req *request;
	int rc;

	request = kzalloc(sizeof(*request), GFP_KERNEL);
	if (!request)
		return -ENOMEM;
	if (timeout)
		request->queue_timeout = timeout;
	request->command = cmd;
	request->sccb = sccb;
	request->status = SCLP_REQ_FILLED;
	request->callback = sclp_sync_callback;
	request->callback_data = &completion;
	init_completion(&completion);

	/* Perform sclp request. */
	rc = sclp_add_request(request);
	if (rc)
		goto out;
	wait_for_completion(&completion);

	/* Check response. */
	if (request->status != SCLP_REQ_DONE) {
		pr_warning("sync request failed (cmd=0x%08x, "
			   "status=0x%02x)\n", cmd, request->status);
		rc = -EIO;
	}
out:
	kfree(request);
	return rc;
}

/*
 * CPU configuration related functions.
 */

#define SCLP_CMDW_READ_CPU_INFO		0x00010001
#define SCLP_CMDW_CONFIGURE_CPU		0x00110001
#define SCLP_CMDW_DECONFIGURE_CPU	0x00100001

struct read_cpu_info_sccb {
	struct	sccb_header header;
	u16	nr_configured;
	u16	offset_configured;
	u16	nr_standby;
	u16	offset_standby;
	u8	reserved[4096 - 16];
} __attribute__((packed, aligned(PAGE_SIZE)));

static void sclp_fill_cpu_info(struct sclp_cpu_info *info,
			       struct read_cpu_info_sccb *sccb)
{
	char *page = (char *) sccb;

	memset(info, 0, sizeof(*info));
	info->configured = sccb->nr_configured;
	info->standby = sccb->nr_standby;
	info->combined = sccb->nr_configured + sccb->nr_standby;
	info->has_cpu_type = sclp_fac84 & 0x1;
	memcpy(&info->cpu, page + sccb->offset_configured,
	       info->combined * sizeof(struct sclp_cpu_entry));
}

int sclp_get_cpu_info(struct sclp_cpu_info *info)
{
	int rc;
	struct read_cpu_info_sccb *sccb;

	if (!SCLP_HAS_CPU_INFO)
		return -EOPNOTSUPP;
	sccb = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;
	sccb->header.length = sizeof(*sccb);
	rc = sclp_sync_request_timeout(SCLP_CMDW_READ_CPU_INFO, sccb,
				       SCLP_QUEUE_INTERVAL);
	if (rc)
		goto out;
	if (sccb->header.response_code != 0x0010) {
		pr_warning("readcpuinfo failed (response=0x%04x)\n",
			   sccb->header.response_code);
		rc = -EIO;
		goto out;
	}
	sclp_fill_cpu_info(info, sccb);
out:
	free_page((unsigned long) sccb);
	return rc;
}

struct cpu_configure_sccb {
	struct sccb_header header;
} __attribute__((packed, aligned(8)));

static int do_cpu_configure(sclp_cmdw_t cmd)
{
	struct cpu_configure_sccb *sccb;
	int rc;

	if (!SCLP_HAS_CPU_RECONFIG)
		return -EOPNOTSUPP;
	/*
	 * This is not going to cross a page boundary since we force
	 * kmalloc to have a minimum alignment of 8 bytes on s390.
	 */
	sccb = kzalloc(sizeof(*sccb), GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;
	sccb->header.length = sizeof(*sccb);
	rc = sclp_sync_request_timeout(cmd, sccb, SCLP_QUEUE_INTERVAL);
	if (rc)
		goto out;
	switch (sccb->header.response_code) {
	case 0x0020:
	case 0x0120:
		break;
	default:
		pr_warning("configure cpu failed (cmd=0x%08x, "
			   "response=0x%04x)\n", cmd,
			   sccb->header.response_code);
		rc = -EIO;
		break;
	}
out:
	kfree(sccb);
	return rc;
}

int sclp_cpu_configure(u8 cpu)
{
	return do_cpu_configure(SCLP_CMDW_CONFIGURE_CPU | cpu << 8);
}

int sclp_cpu_deconfigure(u8 cpu)
{
	return do_cpu_configure(SCLP_CMDW_DECONFIGURE_CPU | cpu << 8);
}

#ifdef CONFIG_MEMORY_HOTPLUG

static DEFINE_MUTEX(sclp_mem_mutex);
static LIST_HEAD(sclp_mem_list);
static u8 sclp_max_storage_id;
static unsigned long sclp_storage_ids[256 / BITS_PER_LONG];
static int sclp_mem_state_changed;

struct memory_increment {
	struct list_head list;
	u16 rn;
	int standby;
};

struct assign_storage_sccb {
	struct sccb_header header;
	u16 rn;
} __packed;

int arch_get_memory_phys_device(unsigned long start_pfn)
{
	if (!sclp_rzm)
		return 0;
	return PFN_PHYS(start_pfn) >> ilog2(sclp_rzm);
}

static unsigned long long rn2addr(u16 rn)
{
	return (unsigned long long) (rn - 1) * sclp_rzm;
}

static int do_assign_storage(sclp_cmdw_t cmd, u16 rn)
{
	struct assign_storage_sccb *sccb;
	int rc;

	sccb = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;
	sccb->header.length = PAGE_SIZE;
	sccb->rn = rn;
	rc = sclp_sync_request_timeout(cmd, sccb, SCLP_QUEUE_INTERVAL);
	if (rc)
		goto out;
	switch (sccb->header.response_code) {
	case 0x0020:
	case 0x0120:
		break;
	default:
		pr_warning("assign storage failed (cmd=0x%08x, "
			   "response=0x%04x, rn=0x%04x)\n", cmd,
			   sccb->header.response_code, rn);
		rc = -EIO;
		break;
	}
out:
	free_page((unsigned long) sccb);
	return rc;
}

static int sclp_assign_storage(u16 rn)
{
	unsigned long long start;
	int rc;

	rc = do_assign_storage(0x000d0001, rn);
	if (rc)
		return rc;
	start = rn2addr(rn);
	storage_key_init_range(start, start + sclp_rzm);
	return 0;
}

static int sclp_unassign_storage(u16 rn)
{
	return do_assign_storage(0x000c0001, rn);
}

struct attach_storage_sccb {
	struct sccb_header header;
	u16 :16;
	u16 assigned;
	u32 :32;
	u32 entries[0];
} __packed;

static int sclp_attach_storage(u8 id)
{
	struct attach_storage_sccb *sccb;
	int rc;
	int i;

	sccb = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;
	sccb->header.length = PAGE_SIZE;
	rc = sclp_sync_request_timeout(0x00080001 | id << 8, sccb,
				       SCLP_QUEUE_INTERVAL);
	if (rc)
		goto out;
	switch (sccb->header.response_code) {
	case 0x0020:
		set_bit(id, sclp_storage_ids);
		for (i = 0; i < sccb->assigned; i++) {
			if (sccb->entries[i])
				sclp_unassign_storage(sccb->entries[i] >> 16);
		}
		break;
	default:
		rc = -EIO;
		break;
	}
out:
	free_page((unsigned long) sccb);
	return rc;
}

static int sclp_mem_change_state(unsigned long start, unsigned long size,
				 int online)
{
	struct memory_increment *incr;
	unsigned long long istart;
	int rc = 0;

	list_for_each_entry(incr, &sclp_mem_list, list) {
		istart = rn2addr(incr->rn);
		if (start + size - 1 < istart)
			break;
		if (start > istart + sclp_rzm - 1)
			continue;
		if (online)
			rc |= sclp_assign_storage(incr->rn);
		else
			sclp_unassign_storage(incr->rn);
		if (rc == 0)
			incr->standby = online ? 0 : 1;
	}
	return rc ? -EIO : 0;
}

static bool contains_standby_increment(unsigned long start, unsigned long end)
{
	struct memory_increment *incr;
	unsigned long istart;

	list_for_each_entry(incr, &sclp_mem_list, list) {
		istart = rn2addr(incr->rn);
		if (end - 1 < istart)
			continue;
		if (start > istart + sclp_rzm - 1)
			continue;
		if (incr->standby)
			return true;
	}
	return false;
}

static int sclp_mem_notifier(struct notifier_block *nb,
			     unsigned long action, void *data)
{
	unsigned long start, size;
	struct memory_notify *arg;
	unsigned char id;
	int rc = 0;

	arg = data;
	start = arg->start_pfn << PAGE_SHIFT;
	size = arg->nr_pages << PAGE_SHIFT;
	mutex_lock(&sclp_mem_mutex);
	for_each_clear_bit(id, sclp_storage_ids, sclp_max_storage_id + 1)
		sclp_attach_storage(id);
	switch (action) {
	case MEM_GOING_OFFLINE:
		/*
		 * We do not allow to set memory blocks offline that contain
		 * standby memory. This is done to simplify the "memory online"
		 * case.
		 */
		if (contains_standby_increment(start, start + size))
			rc = -EPERM;
		break;
	case MEM_ONLINE:
	case MEM_CANCEL_OFFLINE:
		break;
	case MEM_GOING_ONLINE:
		rc = sclp_mem_change_state(start, size, 1);
		break;
	case MEM_CANCEL_ONLINE:
		sclp_mem_change_state(start, size, 0);
		break;
	case MEM_OFFLINE:
		sclp_mem_change_state(start, size, 0);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	if (!rc)
		sclp_mem_state_changed = 1;
	mutex_unlock(&sclp_mem_mutex);
	return rc ? NOTIFY_BAD : NOTIFY_OK;
}

static struct notifier_block sclp_mem_nb = {
	.notifier_call = sclp_mem_notifier,
};

static void __init align_to_block_size(unsigned long long *start,
				       unsigned long long *size)
{
	unsigned long long start_align, size_align, alignment;

	alignment = memory_block_size_bytes();
	start_align = roundup(*start, alignment);
	size_align = rounddown(*start + *size, alignment) - start_align;

	pr_info("Standby memory at 0x%llx (%lluM of %lluM usable)\n",
		*start, size_align >> 20, *size >> 20);
	*start = start_align;
	*size = size_align;
}

static void __init add_memory_merged(u16 rn)
{
	static u16 first_rn, num;
	unsigned long long start, size;

	if (rn && first_rn && (first_rn + num == rn)) {
		num++;
		return;
	}
	if (!first_rn)
		goto skip_add;
	start = rn2addr(first_rn);
	size = (unsigned long long) num * sclp_rzm;
	if (start >= VMEM_MAX_PHYS)
		goto skip_add;
	if (start + size > VMEM_MAX_PHYS)
		size = VMEM_MAX_PHYS - start;
	if (memory_end_set && (start >= memory_end))
		goto skip_add;
	if (memory_end_set && (start + size > memory_end))
		size = memory_end - start;
	align_to_block_size(&start, &size);
	if (size)
		add_memory(0, start, size);
skip_add:
	first_rn = rn;
	num = 1;
}

static void __init sclp_add_standby_memory(void)
{
	struct memory_increment *incr;

	list_for_each_entry(incr, &sclp_mem_list, list)
		if (incr->standby)
			add_memory_merged(incr->rn);
	add_memory_merged(0);
}

static void __init insert_increment(u16 rn, int standby, int assigned)
{
	struct memory_increment *incr, *new_incr;
	struct list_head *prev;
	u16 last_rn;

	new_incr = kzalloc(sizeof(*new_incr), GFP_KERNEL);
	if (!new_incr)
		return;
	new_incr->rn = rn;
	new_incr->standby = standby;
	last_rn = 0;
	prev = &sclp_mem_list;
	list_for_each_entry(incr, &sclp_mem_list, list) {
		if (assigned && incr->rn > rn)
			break;
		if (!assigned && incr->rn - last_rn > 1)
			break;
		last_rn = incr->rn;
		prev = &incr->list;
	}
	if (!assigned)
		new_incr->rn = last_rn + 1;
	if (new_incr->rn > sclp_rnmax) {
		kfree(new_incr);
		return;
	}
	list_add(&new_incr->list, prev);
}

static int sclp_mem_freeze(struct device *dev)
{
	if (!sclp_mem_state_changed)
		return 0;
	pr_err("Memory hotplug state changed, suspend refused.\n");
	return -EPERM;
}

struct read_storage_sccb {
	struct sccb_header header;
	u16 max_id;
	u16 assigned;
	u16 standby;
	u16 :16;
	u32 entries[0];
} __packed;

static const struct dev_pm_ops sclp_mem_pm_ops = {
	.freeze		= sclp_mem_freeze,
};

static struct platform_driver sclp_mem_pdrv = {
	.driver = {
		.name	= "sclp_mem",
		.pm	= &sclp_mem_pm_ops,
	},
};

static int __init sclp_detect_standby_memory(void)
{
	struct platform_device *sclp_pdev;
	struct read_storage_sccb *sccb;
	int i, id, assigned, rc;

	if (OLDMEM_BASE) /* No standby memory in kdump mode */
		return 0;
	if ((sclp_facilities & 0xe00000000000ULL) != 0xe00000000000ULL)
		return 0;
	rc = -ENOMEM;
	sccb = (void *) __get_free_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		goto out;
	assigned = 0;
	for (id = 0; id <= sclp_max_storage_id; id++) {
		memset(sccb, 0, PAGE_SIZE);
		sccb->header.length = PAGE_SIZE;
		rc = sclp_sync_request(0x00040001 | id << 8, sccb);
		if (rc)
			goto out;
		switch (sccb->header.response_code) {
		case 0x0010:
			set_bit(id, sclp_storage_ids);
			for (i = 0; i < sccb->assigned; i++) {
				if (!sccb->entries[i])
					continue;
				assigned++;
				insert_increment(sccb->entries[i] >> 16, 0, 1);
			}
			break;
		case 0x0310:
			break;
		case 0x0410:
			for (i = 0; i < sccb->assigned; i++) {
				if (!sccb->entries[i])
					continue;
				assigned++;
				insert_increment(sccb->entries[i] >> 16, 1, 1);
			}
			break;
		default:
			rc = -EIO;
			break;
		}
		if (!rc)
			sclp_max_storage_id = sccb->max_id;
	}
	if (rc || list_empty(&sclp_mem_list))
		goto out;
	for (i = 1; i <= sclp_rnmax - assigned; i++)
		insert_increment(0, 1, 0);
	rc = register_memory_notifier(&sclp_mem_nb);
	if (rc)
		goto out;
	rc = platform_driver_register(&sclp_mem_pdrv);
	if (rc)
		goto out;
	sclp_pdev = platform_device_register_simple("sclp_mem", -1, NULL, 0);
	rc = PTR_ERR_OR_ZERO(sclp_pdev);
	if (rc)
		goto out_driver;
	sclp_add_standby_memory();
	goto out;
out_driver:
	platform_driver_unregister(&sclp_mem_pdrv);
out:
	free_page((unsigned long) sccb);
	return rc;
}
__initcall(sclp_detect_standby_memory);

#endif /* CONFIG_MEMORY_HOTPLUG */

/*
 * PCI I/O adapter configuration related functions.
 */
#define SCLP_CMDW_CONFIGURE_PCI			0x001a0001
#define SCLP_CMDW_DECONFIGURE_PCI		0x001b0001

#define SCLP_RECONFIG_PCI_ATPYE			2

struct pci_cfg_sccb {
	struct sccb_header header;
	u8 atype;		/* adapter type */
	u8 reserved1;
	u16 reserved2;
	u32 aid;		/* adapter identifier */
} __packed;

static int do_pci_configure(sclp_cmdw_t cmd, u32 fid)
{
	struct pci_cfg_sccb *sccb;
	int rc;

	if (!SCLP_HAS_PCI_RECONFIG)
		return -EOPNOTSUPP;

	sccb = (struct pci_cfg_sccb *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;

	sccb->header.length = PAGE_SIZE;
	sccb->atype = SCLP_RECONFIG_PCI_ATPYE;
	sccb->aid = fid;
	rc = sclp_sync_request(cmd, sccb);
	if (rc)
		goto out;
	switch (sccb->header.response_code) {
	case 0x0020:
	case 0x0120:
		break;
	default:
		pr_warn("configure PCI I/O adapter failed: cmd=0x%08x  response=0x%04x\n",
			cmd, sccb->header.response_code);
		rc = -EIO;
		break;
	}
out:
	free_page((unsigned long) sccb);
	return rc;
}

int sclp_pci_configure(u32 fid)
{
	return do_pci_configure(SCLP_CMDW_CONFIGURE_PCI, fid);
}
EXPORT_SYMBOL(sclp_pci_configure);

int sclp_pci_deconfigure(u32 fid)
{
	return do_pci_configure(SCLP_CMDW_DECONFIGURE_PCI, fid);
}
EXPORT_SYMBOL(sclp_pci_deconfigure);

/*
 * Channel path configuration related functions.
 */

#define SCLP_CMDW_CONFIGURE_CHPATH		0x000f0001
#define SCLP_CMDW_DECONFIGURE_CHPATH		0x000e0001
#define SCLP_CMDW_READ_CHPATH_INFORMATION	0x00030001

struct chp_cfg_sccb {
	struct sccb_header header;
	u8 ccm;
	u8 reserved[6];
	u8 cssid;
} __attribute__((packed));

static int do_chp_configure(sclp_cmdw_t cmd)
{
	struct chp_cfg_sccb *sccb;
	int rc;

	if (!SCLP_HAS_CHP_RECONFIG)
		return -EOPNOTSUPP;
	/* Prepare sccb. */
	sccb = (struct chp_cfg_sccb *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;
	sccb->header.length = sizeof(*sccb);
	rc = sclp_sync_request(cmd, sccb);
	if (rc)
		goto out;
	switch (sccb->header.response_code) {
	case 0x0020:
	case 0x0120:
	case 0x0440:
	case 0x0450:
		break;
	default:
		pr_warning("configure channel-path failed "
			   "(cmd=0x%08x, response=0x%04x)\n", cmd,
			   sccb->header.response_code);
		rc = -EIO;
		break;
	}
out:
	free_page((unsigned long) sccb);
	return rc;
}

/**
 * sclp_chp_configure - perform configure channel-path sclp command
 * @chpid: channel-path ID
 *
 * Perform configure channel-path command sclp command for specified chpid.
 * Return 0 after command successfully finished, non-zero otherwise.
 */
int sclp_chp_configure(struct chp_id chpid)
{
	return do_chp_configure(SCLP_CMDW_CONFIGURE_CHPATH | chpid.id << 8);
}

/**
 * sclp_chp_deconfigure - perform deconfigure channel-path sclp command
 * @chpid: channel-path ID
 *
 * Perform deconfigure channel-path command sclp command for specified chpid
 * and wait for completion. On success return 0. Return non-zero otherwise.
 */
int sclp_chp_deconfigure(struct chp_id chpid)
{
	return do_chp_configure(SCLP_CMDW_DECONFIGURE_CHPATH | chpid.id << 8);
}

struct chp_info_sccb {
	struct sccb_header header;
	u8 recognized[SCLP_CHP_INFO_MASK_SIZE];
	u8 standby[SCLP_CHP_INFO_MASK_SIZE];
	u8 configured[SCLP_CHP_INFO_MASK_SIZE];
	u8 ccm;
	u8 reserved[6];
	u8 cssid;
} __attribute__((packed));

/**
 * sclp_chp_read_info - perform read channel-path information sclp command
 * @info: resulting channel-path information data
 *
 * Perform read channel-path information sclp command and wait for completion.
 * On success, store channel-path information in @info and return 0. Return
 * non-zero otherwise.
 */
int sclp_chp_read_info(struct sclp_chp_info *info)
{
	struct chp_info_sccb *sccb;
	int rc;

	if (!SCLP_HAS_CHP_INFO)
		return -EOPNOTSUPP;
	/* Prepare sccb. */
	sccb = (struct chp_info_sccb *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!sccb)
		return -ENOMEM;
	sccb->header.length = sizeof(*sccb);
	rc = sclp_sync_request(SCLP_CMDW_READ_CHPATH_INFORMATION, sccb);
	if (rc)
		goto out;
	if (sccb->header.response_code != 0x0010) {
		pr_warning("read channel-path info failed "
			   "(response=0x%04x)\n", sccb->header.response_code);
		rc = -EIO;
		goto out;
	}
	memcpy(info->recognized, sccb->recognized, SCLP_CHP_INFO_MASK_SIZE);
	memcpy(info->standby, sccb->standby, SCLP_CHP_INFO_MASK_SIZE);
	memcpy(info->configured, sccb->configured, SCLP_CHP_INFO_MASK_SIZE);
out:
	free_page((unsigned long) sccb);
	return rc;
}

bool sclp_has_sprp(void)
{
	return !!(sclp_fac84 & 0x2);
}
