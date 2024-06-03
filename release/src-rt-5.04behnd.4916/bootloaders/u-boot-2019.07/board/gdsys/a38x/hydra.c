#include <common.h>
#include <console.h> /* ctrlc */
#include <asm/io.h>

#include "hydra.h"

enum {
	HWVER_100 = 0,
	HWVER_110 = 1,
	HWVER_120 = 2,
};

static struct pci_device_id hydra_supported[] = {
	{ 0x6d5e, 0xcdc1 },
	{}
};

static struct ihs_fpga *fpga;

struct ihs_fpga *get_fpga(void)
{
	return fpga;
}

void print_hydra_version(uint index)
{
	u32 versions = readl(&fpga->versions);
	u32 fpga_version = readl(&fpga->fpga_version);

	uint hardware_version = versions & 0xf;

	printf("FPGA%u: mapped to %p\n       ", index, fpga);

	switch (hardware_version) {
	case HWVER_100:
		printf("HW-Ver 1.00\n");
		break;

	case HWVER_110:
		printf("HW-Ver 1.10\n");
		break;

	case HWVER_120:
		printf("HW-Ver 1.20\n");
		break;

	default:
		printf("HW-Ver %d(not supported)\n",
		       hardware_version);
		break;
	}

	printf("       FPGA V %d.%02d\n",
	       fpga_version / 100, fpga_version % 100);
}

void hydra_initialize(void)
{
	uint i;
	pci_dev_t devno;

	/* Find and probe all the matching PCI devices */
	for (i = 0; (devno = pci_find_devices(hydra_supported, i)) >= 0; i++) {
		u32 val;

		/* Try to enable I/O accesses and bus-mastering */
		val = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
		pci_write_config_dword(devno, PCI_COMMAND, val);

		/* Make sure it worked */
		pci_read_config_dword(devno, PCI_COMMAND, &val);
		if (!(val & PCI_COMMAND_MEMORY)) {
			puts("Can't enable I/O memory\n");
			continue;
		}
		if (!(val & PCI_COMMAND_MASTER)) {
			puts("Can't enable bus-mastering\n");
			continue;
		}

		/* read FPGA details */
		fpga = pci_map_bar(devno, PCI_BASE_ADDRESS_0,
				   PCI_REGION_MEM);

		print_hydra_version(i);
	}
}

#define REFL_PATTERN (0xdededede)
#define REFL_PATTERN_INV (~REFL_PATTERN)

int do_hydrate(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint k = 0;
	void __iomem *pcie2_base = (void __iomem *)(MVEBU_REG_PCIE_BASE +
						    0x4000);

	if (!fpga)
		return -1;

	while (1) {
		u32 res;

		writel(REFL_PATTERN, &fpga->reflection_low);
		res = readl(&fpga->reflection_low);
		if (res != REFL_PATTERN_INV)
			printf("round %u: read %08x, expected %08x\n",
			       k, res, REFL_PATTERN_INV);
		writel(REFL_PATTERN_INV, &fpga->reflection_low);
		res = readl(&fpga->reflection_low);
		if (res != REFL_PATTERN)
			printf("round %u: read %08x, expected %08x\n",
			       k, res, REFL_PATTERN);

		res = readl(pcie2_base + 0x118) & 0x1f;
		if (res)
			printf("FrstErrPtr %u\n", res);
		res = readl(pcie2_base + 0x104);
		if (res) {
			printf("Uncorrectable Error Status 0x%08x\n", res);
			writel(res, pcie2_base + 0x104);
		}

		if (!(++k % 10000))
			printf("round %u\n", k);

		if (ctrlc())
			break;
	}

	return 0;
}

U_BOOT_CMD(
	hydrate,	1,	0,	do_hydrate,
	"hydra reflection test",
	"hydra reflection test"
);
