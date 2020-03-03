/* Nehalem/SandBridge/Haswell uncore support */
#include "perf_event_intel_uncore.h"

/* Uncore IMC PCI IDs */
#define PCI_DEVICE_ID_INTEL_SNB_IMC	0x0100
#define PCI_DEVICE_ID_INTEL_IVB_IMC	0x0154
#define PCI_DEVICE_ID_INTEL_IVB_E3_IMC	0x0150
#define PCI_DEVICE_ID_INTEL_HSW_IMC	0x0c00
#define PCI_DEVICE_ID_INTEL_HSW_U_IMC	0x0a04

/* SNB event control */
#define SNB_UNC_CTL_EV_SEL_MASK			0x000000ff
#define SNB_UNC_CTL_UMASK_MASK			0x0000ff00
#define SNB_UNC_CTL_EDGE_DET			(1 << 18)
#define SNB_UNC_CTL_EN				(1 << 22)
#define SNB_UNC_CTL_INVERT			(1 << 23)
#define SNB_UNC_CTL_CMASK_MASK			0x1f000000
#define NHM_UNC_CTL_CMASK_MASK			0xff000000
#define NHM_UNC_FIXED_CTR_CTL_EN		(1 << 0)

#define SNB_UNC_RAW_EVENT_MASK			(SNB_UNC_CTL_EV_SEL_MASK | \
						 SNB_UNC_CTL_UMASK_MASK | \
						 SNB_UNC_CTL_EDGE_DET | \
						 SNB_UNC_CTL_INVERT | \
						 SNB_UNC_CTL_CMASK_MASK)

#define NHM_UNC_RAW_EVENT_MASK			(SNB_UNC_CTL_EV_SEL_MASK | \
						 SNB_UNC_CTL_UMASK_MASK | \
						 SNB_UNC_CTL_EDGE_DET | \
						 SNB_UNC_CTL_INVERT | \
						 NHM_UNC_CTL_CMASK_MASK)

/* SNB global control register */
#define SNB_UNC_PERF_GLOBAL_CTL                 0x391
#define SNB_UNC_FIXED_CTR_CTRL                  0x394
#define SNB_UNC_FIXED_CTR                       0x395

/* SNB uncore global control */
#define SNB_UNC_GLOBAL_CTL_CORE_ALL             ((1 << 4) - 1)
#define SNB_UNC_GLOBAL_CTL_EN                   (1 << 29)

/* SNB Cbo register */
#define SNB_UNC_CBO_0_PERFEVTSEL0               0x700
#define SNB_UNC_CBO_0_PER_CTR0                  0x706
#define SNB_UNC_CBO_MSR_OFFSET                  0x10

/* NHM global control register */
#define NHM_UNC_PERF_GLOBAL_CTL                 0x391
#define NHM_UNC_FIXED_CTR                       0x394
#define NHM_UNC_FIXED_CTR_CTRL                  0x395

/* NHM uncore global control */
#define NHM_UNC_GLOBAL_CTL_EN_PC_ALL            ((1ULL << 8) - 1)
#define NHM_UNC_GLOBAL_CTL_EN_FC                (1ULL << 32)

/* NHM uncore register */
#define NHM_UNC_PERFEVTSEL0                     0x3c0
#define NHM_UNC_UNCORE_PMC0                     0x3b0

DEFINE_UNCORE_FORMAT_ATTR(event, event, "config:0-7");
DEFINE_UNCORE_FORMAT_ATTR(umask, umask, "config:8-15");
DEFINE_UNCORE_FORMAT_ATTR(edge, edge, "config:18");
DEFINE_UNCORE_FORMAT_ATTR(inv, inv, "config:23");
DEFINE_UNCORE_FORMAT_ATTR(cmask5, cmask, "config:24-28");
DEFINE_UNCORE_FORMAT_ATTR(cmask8, cmask, "config:24-31");

/* Sandy Bridge uncore support */
static void snb_uncore_msr_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;

	if (hwc->idx < UNCORE_PMC_IDX_FIXED)
		wrmsrl(hwc->config_base, hwc->config | SNB_UNC_CTL_EN);
	else
		wrmsrl(hwc->config_base, SNB_UNC_CTL_EN);
}

static void snb_uncore_msr_disable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	wrmsrl(event->hw.config_base, 0);
}

static void snb_uncore_msr_init_box(struct intel_uncore_box *box)
{
	if (box->pmu->pmu_idx == 0) {
		wrmsrl(SNB_UNC_PERF_GLOBAL_CTL,
			SNB_UNC_GLOBAL_CTL_EN | SNB_UNC_GLOBAL_CTL_CORE_ALL);
	}
}

static struct uncore_event_desc snb_uncore_events[] = {
	INTEL_UNCORE_EVENT_DESC(clockticks, "event=0xff,umask=0x00"),
	{ /* end: all zeroes */ },
};

static struct attribute *snb_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_cmask5.attr,
	NULL,
};

static struct attribute_group snb_uncore_format_group = {
	.name		= "format",
	.attrs		= snb_uncore_formats_attr,
};

static struct intel_uncore_ops snb_uncore_msr_ops = {
	.init_box	= snb_uncore_msr_init_box,
	.disable_event	= snb_uncore_msr_disable_event,
	.enable_event	= snb_uncore_msr_enable_event,
	.read_counter	= uncore_msr_read_counter,
};

static struct event_constraint snb_uncore_cbox_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x80, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x83, 0x1),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type snb_uncore_cbox = {
	.name		= "cbox",
	.num_counters   = 2,
	.num_boxes	= 4,
	.perf_ctr_bits	= 44,
	.fixed_ctr_bits	= 48,
	.perf_ctr	= SNB_UNC_CBO_0_PER_CTR0,
	.event_ctl	= SNB_UNC_CBO_0_PERFEVTSEL0,
	.fixed_ctr	= SNB_UNC_FIXED_CTR,
	.fixed_ctl	= SNB_UNC_FIXED_CTR_CTRL,
	.single_fixed	= 1,
	.event_mask	= SNB_UNC_RAW_EVENT_MASK,
	.msr_offset	= SNB_UNC_CBO_MSR_OFFSET,
	.constraints	= snb_uncore_cbox_constraints,
	.ops		= &snb_uncore_msr_ops,
	.format_group	= &snb_uncore_format_group,
	.event_descs	= snb_uncore_events,
};

static struct intel_uncore_type *snb_msr_uncores[] = {
	&snb_uncore_cbox,
	NULL,
};

void snb_uncore_cpu_init(void)
{
	uncore_msr_uncores = snb_msr_uncores;
	if (snb_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		snb_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;
}

enum {
	SNB_PCI_UNCORE_IMC,
};

static struct uncore_event_desc snb_uncore_imc_events[] = {
	INTEL_UNCORE_EVENT_DESC(data_reads,  "event=0x01"),
	INTEL_UNCORE_EVENT_DESC(data_reads.scale, "6.103515625e-5"),
	INTEL_UNCORE_EVENT_DESC(data_reads.unit, "MiB"),

	INTEL_UNCORE_EVENT_DESC(data_writes, "event=0x02"),
	INTEL_UNCORE_EVENT_DESC(data_writes.scale, "6.103515625e-5"),
	INTEL_UNCORE_EVENT_DESC(data_writes.unit, "MiB"),

	{ /* end: all zeroes */ },
};

#define SNB_UNCORE_PCI_IMC_EVENT_MASK		0xff
#define SNB_UNCORE_PCI_IMC_BAR_OFFSET		0x48

/* page size multiple covering all config regs */
#define SNB_UNCORE_PCI_IMC_MAP_SIZE		0x6000

#define SNB_UNCORE_PCI_IMC_DATA_READS		0x1
#define SNB_UNCORE_PCI_IMC_DATA_READS_BASE	0x5050
#define SNB_UNCORE_PCI_IMC_DATA_WRITES		0x2
#define SNB_UNCORE_PCI_IMC_DATA_WRITES_BASE	0x5054
#define SNB_UNCORE_PCI_IMC_CTR_BASE		SNB_UNCORE_PCI_IMC_DATA_READS_BASE

static struct attribute *snb_uncore_imc_formats_attr[] = {
	&format_attr_event.attr,
	NULL,
};

static struct attribute_group snb_uncore_imc_format_group = {
	.name = "format",
	.attrs = snb_uncore_imc_formats_attr,
};

static void snb_uncore_imc_init_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;
	int where = SNB_UNCORE_PCI_IMC_BAR_OFFSET;
	resource_size_t addr;
	u32 pci_dword;

	pci_read_config_dword(pdev, where, &pci_dword);
	addr = pci_dword;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pci_read_config_dword(pdev, where + 4, &pci_dword);
	addr |= ((resource_size_t)pci_dword << 32);
#endif

	addr &= ~(PAGE_SIZE - 1);

	box->io_addr = ioremap(addr, SNB_UNCORE_PCI_IMC_MAP_SIZE);
	box->hrtimer_duration = UNCORE_SNB_IMC_HRTIMER_INTERVAL;
}

static void snb_uncore_imc_enable_box(struct intel_uncore_box *box)
{}

static void snb_uncore_imc_disable_box(struct intel_uncore_box *box)
{}

static void snb_uncore_imc_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{}

static void snb_uncore_imc_disable_event(struct intel_uncore_box *box, struct perf_event *event)
{}

static u64 snb_uncore_imc_read_counter(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;

	return (u64)*(unsigned int *)(box->io_addr + hwc->event_base);
}

/*
 * custom event_init() function because we define our own fixed, free
 * running counters, so we do not want to conflict with generic uncore
 * logic. Also simplifies processing
 */
static int snb_uncore_imc_event_init(struct perf_event *event)
{
	struct intel_uncore_pmu *pmu;
	struct intel_uncore_box *box;
	struct hw_perf_event *hwc = &event->hw;
	u64 cfg = event->attr.config & SNB_UNCORE_PCI_IMC_EVENT_MASK;
	int idx, base;

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	pmu = uncore_event_to_pmu(event);
	/* no device found for this pmu */
	if (pmu->func_id < 0)
		return -ENOENT;

	/* Sampling not supported yet */
	if (hwc->sample_period)
		return -EINVAL;

	/* unsupported modes and filters */
	if (event->attr.exclude_user   ||
	    event->attr.exclude_kernel ||
	    event->attr.exclude_hv     ||
	    event->attr.exclude_idle   ||
	    event->attr.exclude_host   ||
	    event->attr.exclude_guest  ||
	    event->attr.sample_period) /* no sampling */
		return -EINVAL;

	/*
	 * Place all uncore events for a particular physical package
	 * onto a single cpu
	 */
	if (event->cpu < 0)
		return -EINVAL;

	/* check only supported bits are set */
	if (event->attr.config & ~SNB_UNCORE_PCI_IMC_EVENT_MASK)
		return -EINVAL;

	box = uncore_pmu_to_box(pmu, event->cpu);
	if (!box || box->cpu < 0)
		return -EINVAL;

	event->cpu = box->cpu;

	event->hw.idx = -1;
	event->hw.last_tag = ~0ULL;
	event->hw.extra_reg.idx = EXTRA_REG_NONE;
	event->hw.branch_reg.idx = EXTRA_REG_NONE;
	/*
	 * check event is known (whitelist, determines counter)
	 */
	switch (cfg) {
	case SNB_UNCORE_PCI_IMC_DATA_READS:
		base = SNB_UNCORE_PCI_IMC_DATA_READS_BASE;
		idx = UNCORE_PMC_IDX_FIXED;
		break;
	case SNB_UNCORE_PCI_IMC_DATA_WRITES:
		base = SNB_UNCORE_PCI_IMC_DATA_WRITES_BASE;
		idx = UNCORE_PMC_IDX_FIXED + 1;
		break;
	default:
		return -EINVAL;
	}

	/* must be done before validate_group */
	event->hw.event_base = base;
	event->hw.config = cfg;
	event->hw.idx = idx;

	/* no group validation needed, we have free running counters */

	return 0;
}

static int snb_uncore_imc_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	return 0;
}

static void snb_uncore_imc_event_start(struct perf_event *event, int flags)
{
	struct intel_uncore_box *box = uncore_event_to_box(event);
	u64 count;

	if (WARN_ON_ONCE(!(event->hw.state & PERF_HES_STOPPED)))
		return;

	event->hw.state = 0;
	box->n_active++;

	list_add_tail(&event->active_entry, &box->active_list);

	count = snb_uncore_imc_read_counter(box, event);
	local64_set(&event->hw.prev_count, count);

	if (box->n_active == 1)
		uncore_pmu_start_hrtimer(box);
}

static void snb_uncore_imc_event_stop(struct perf_event *event, int flags)
{
	struct intel_uncore_box *box = uncore_event_to_box(event);
	struct hw_perf_event *hwc = &event->hw;

	if (!(hwc->state & PERF_HES_STOPPED)) {
		box->n_active--;

		WARN_ON_ONCE(hwc->state & PERF_HES_STOPPED);
		hwc->state |= PERF_HES_STOPPED;

		list_del(&event->active_entry);

		if (box->n_active == 0)
			uncore_pmu_cancel_hrtimer(box);
	}

	if ((flags & PERF_EF_UPDATE) && !(hwc->state & PERF_HES_UPTODATE)) {
		/*
		 * Drain the remaining delta count out of a event
		 * that we are disabling:
		 */
		uncore_perf_event_update(box, event);
		hwc->state |= PERF_HES_UPTODATE;
	}
}

static int snb_uncore_imc_event_add(struct perf_event *event, int flags)
{
	struct intel_uncore_box *box = uncore_event_to_box(event);
	struct hw_perf_event *hwc = &event->hw;

	if (!box)
		return -ENODEV;

	hwc->state = PERF_HES_UPTODATE | PERF_HES_STOPPED;
	if (!(flags & PERF_EF_START))
		hwc->state |= PERF_HES_ARCH;

	snb_uncore_imc_event_start(event, 0);

	box->n_events++;

	return 0;
}

static void snb_uncore_imc_event_del(struct perf_event *event, int flags)
{
	struct intel_uncore_box *box = uncore_event_to_box(event);
	int i;

	snb_uncore_imc_event_stop(event, PERF_EF_UPDATE);

	for (i = 0; i < box->n_events; i++) {
		if (event == box->event_list[i]) {
			--box->n_events;
			break;
		}
	}
}

static int snb_pci2phy_map_init(int devid)
{
	struct pci_dev *dev = NULL;
	int bus;

	dev = pci_get_device(PCI_VENDOR_ID_INTEL, devid, dev);
	if (!dev)
		return -ENOTTY;

	bus = dev->bus->number;

	uncore_pcibus_to_physid[bus] = 0;

	pci_dev_put(dev);

	return 0;
}

static struct pmu snb_uncore_imc_pmu = {
	.task_ctx_nr	= perf_invalid_context,
	.event_init	= snb_uncore_imc_event_init,
	.add		= snb_uncore_imc_event_add,
	.del		= snb_uncore_imc_event_del,
	.start		= snb_uncore_imc_event_start,
	.stop		= snb_uncore_imc_event_stop,
	.read		= uncore_pmu_event_read,
};

static struct intel_uncore_ops snb_uncore_imc_ops = {
	.init_box	= snb_uncore_imc_init_box,
	.enable_box	= snb_uncore_imc_enable_box,
	.disable_box	= snb_uncore_imc_disable_box,
	.disable_event	= snb_uncore_imc_disable_event,
	.enable_event	= snb_uncore_imc_enable_event,
	.hw_config	= snb_uncore_imc_hw_config,
	.read_counter	= snb_uncore_imc_read_counter,
};

static struct intel_uncore_type snb_uncore_imc = {
	.name		= "imc",
	.num_counters   = 2,
	.num_boxes	= 1,
	.fixed_ctr_bits	= 32,
	.fixed_ctr	= SNB_UNCORE_PCI_IMC_CTR_BASE,
	.event_descs	= snb_uncore_imc_events,
	.format_group	= &snb_uncore_imc_format_group,
	.perf_ctr	= SNB_UNCORE_PCI_IMC_DATA_READS_BASE,
	.event_mask	= SNB_UNCORE_PCI_IMC_EVENT_MASK,
	.ops		= &snb_uncore_imc_ops,
	.pmu		= &snb_uncore_imc_pmu,
};

static struct intel_uncore_type *snb_pci_uncores[] = {
	[SNB_PCI_UNCORE_IMC]	= &snb_uncore_imc,
	NULL,
};

static const struct pci_device_id snb_uncore_pci_ids[] = {
	{ /* IMC */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SNB_IMC),
		.driver_data = UNCORE_PCI_DEV_DATA(SNB_PCI_UNCORE_IMC, 0),
	},
	{ /* end: all zeroes */ },
};

static const struct pci_device_id ivb_uncore_pci_ids[] = {
	{ /* IMC */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_IMC),
		.driver_data = UNCORE_PCI_DEV_DATA(SNB_PCI_UNCORE_IMC, 0),
	},
	{ /* IMC */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_E3_IMC),
		.driver_data = UNCORE_PCI_DEV_DATA(SNB_PCI_UNCORE_IMC, 0),
	},
	{ /* end: all zeroes */ },
};

static const struct pci_device_id hsw_uncore_pci_ids[] = {
	{ /* IMC */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HSW_IMC),
		.driver_data = UNCORE_PCI_DEV_DATA(SNB_PCI_UNCORE_IMC, 0),
	},
	{ /* IMC */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HSW_U_IMC),
		.driver_data = UNCORE_PCI_DEV_DATA(SNB_PCI_UNCORE_IMC, 0),
	},
	{ /* end: all zeroes */ },
};

static struct pci_driver snb_uncore_pci_driver = {
	.name		= "snb_uncore",
	.id_table	= snb_uncore_pci_ids,
};

static struct pci_driver ivb_uncore_pci_driver = {
	.name		= "ivb_uncore",
	.id_table	= ivb_uncore_pci_ids,
};

static struct pci_driver hsw_uncore_pci_driver = {
	.name		= "hsw_uncore",
	.id_table	= hsw_uncore_pci_ids,
};

struct imc_uncore_pci_dev {
	__u32 pci_id;
	struct pci_driver *driver;
};
#define IMC_DEV(a, d) \
	{ .pci_id = PCI_DEVICE_ID_INTEL_##a, .driver = (d) }

static const struct imc_uncore_pci_dev desktop_imc_pci_ids[] = {
	IMC_DEV(SNB_IMC, &snb_uncore_pci_driver),
	IMC_DEV(IVB_IMC, &ivb_uncore_pci_driver),    /* 3rd Gen Core processor */
	IMC_DEV(IVB_E3_IMC, &ivb_uncore_pci_driver), /* Xeon E3-1200 v2/3rd Gen Core processor */
	IMC_DEV(HSW_IMC, &hsw_uncore_pci_driver),    /* 4th Gen Core Processor */
	IMC_DEV(HSW_U_IMC, &hsw_uncore_pci_driver),  /* 4th Gen Core ULT Mobile Processor */
	{  /* end marker */ }
};


#define for_each_imc_pci_id(x, t) \
	for (x = (t); (x)->pci_id; x++)

static struct pci_driver *imc_uncore_find_dev(void)
{
	const struct imc_uncore_pci_dev *p;
	int ret;

	for_each_imc_pci_id(p, desktop_imc_pci_ids) {
		ret = snb_pci2phy_map_init(p->pci_id);
		if (ret == 0)
			return p->driver;
	}
	return NULL;
}

static int imc_uncore_pci_init(void)
{
	struct pci_driver *imc_drv = imc_uncore_find_dev();

	if (!imc_drv)
		return -ENODEV;

	uncore_pci_uncores = snb_pci_uncores;
	uncore_pci_driver = imc_drv;

	return 0;
}

int snb_uncore_pci_init(void)
{
	return imc_uncore_pci_init();
}

int ivb_uncore_pci_init(void)
{
	return imc_uncore_pci_init();
}
int hsw_uncore_pci_init(void)
{
	return imc_uncore_pci_init();
}

/* end of Sandy Bridge uncore support */

/* Nehalem uncore support */
static void nhm_uncore_msr_disable_box(struct intel_uncore_box *box)
{
	wrmsrl(NHM_UNC_PERF_GLOBAL_CTL, 0);
}

static void nhm_uncore_msr_enable_box(struct intel_uncore_box *box)
{
	wrmsrl(NHM_UNC_PERF_GLOBAL_CTL, NHM_UNC_GLOBAL_CTL_EN_PC_ALL | NHM_UNC_GLOBAL_CTL_EN_FC);
}

static void nhm_uncore_msr_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;

	if (hwc->idx < UNCORE_PMC_IDX_FIXED)
		wrmsrl(hwc->config_base, hwc->config | SNB_UNC_CTL_EN);
	else
		wrmsrl(hwc->config_base, NHM_UNC_FIXED_CTR_CTL_EN);
}

static struct attribute *nhm_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_cmask8.attr,
	NULL,
};

static struct attribute_group nhm_uncore_format_group = {
	.name = "format",
	.attrs = nhm_uncore_formats_attr,
};

static struct uncore_event_desc nhm_uncore_events[] = {
	INTEL_UNCORE_EVENT_DESC(clockticks,                "event=0xff,umask=0x00"),
	INTEL_UNCORE_EVENT_DESC(qmc_writes_full_any,       "event=0x2f,umask=0x0f"),
	INTEL_UNCORE_EVENT_DESC(qmc_normal_reads_any,      "event=0x2c,umask=0x0f"),
	INTEL_UNCORE_EVENT_DESC(qhl_request_ioh_reads,     "event=0x20,umask=0x01"),
	INTEL_UNCORE_EVENT_DESC(qhl_request_ioh_writes,    "event=0x20,umask=0x02"),
	INTEL_UNCORE_EVENT_DESC(qhl_request_remote_reads,  "event=0x20,umask=0x04"),
	INTEL_UNCORE_EVENT_DESC(qhl_request_remote_writes, "event=0x20,umask=0x08"),
	INTEL_UNCORE_EVENT_DESC(qhl_request_local_reads,   "event=0x20,umask=0x10"),
	INTEL_UNCORE_EVENT_DESC(qhl_request_local_writes,  "event=0x20,umask=0x20"),
	{ /* end: all zeroes */ },
};

static struct intel_uncore_ops nhm_uncore_msr_ops = {
	.disable_box	= nhm_uncore_msr_disable_box,
	.enable_box	= nhm_uncore_msr_enable_box,
	.disable_event	= snb_uncore_msr_disable_event,
	.enable_event	= nhm_uncore_msr_enable_event,
	.read_counter	= uncore_msr_read_counter,
};

static struct intel_uncore_type nhm_uncore = {
	.name		= "",
	.num_counters   = 8,
	.num_boxes	= 1,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.event_ctl	= NHM_UNC_PERFEVTSEL0,
	.perf_ctr	= NHM_UNC_UNCORE_PMC0,
	.fixed_ctr	= NHM_UNC_FIXED_CTR,
	.fixed_ctl	= NHM_UNC_FIXED_CTR_CTRL,
	.event_mask	= NHM_UNC_RAW_EVENT_MASK,
	.event_descs	= nhm_uncore_events,
	.ops		= &nhm_uncore_msr_ops,
	.format_group	= &nhm_uncore_format_group,
};

static struct intel_uncore_type *nhm_msr_uncores[] = {
	&nhm_uncore,
	NULL,
};

void nhm_uncore_cpu_init(void)
{
	uncore_msr_uncores = nhm_msr_uncores;
}

/* end of Nehalem uncore support */
