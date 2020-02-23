#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/clocksource.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <board.h>
#if defined (CONFIG_BCM947189) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM96878)
#define __BOARD_DRV_ARMV7__
#else
#define __BOARD_DRV_AARCH64__
#endif

#include <bcm_map_part.h>
#include <bcm_intr.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
#define ERR
#define NOERR
void __init brcm_legacy_init(struct device_node *np)
#else
#define ERR -1
#define NOERR 0
int __init brcm_legacy_init(struct device_node *np)
#endif
{
    static int init = 0;
    struct of_phandle_args oirq;
    int i;

    /* it maybe already called by the cpuclk driver early if it is enabled 
       in the device tree */  
    if (init)
        return NOERR;

    printk(KERN_INFO "BRCM Legacy Drivers' Helper, all legacy drivers' IO memories/interrupts should be remapped here\n");
    if(!np)
        return ERR;
  
    // find interrupt parent (gic) node
    oirq.np = of_irq_find_parent(np);

    // interrupt configuration is hard-coded 
    oirq.args_count = 3; // corresponds to '#interrupt-cells = <3>' in gic node of device tree
    oirq.args[0] = GIC_SPI; // interrupt specifier cell 0
    oirq.args[2] = IRQ_TYPE_LEVEL_HIGH; // interrupt specifier cell 2
    printk(KERN_INFO "     Remapping interrupts...\n");
    printk(KERN_INFO "             hwirq      virq\n");
    for(i=0; i < sizeof(bcm_phys_irqs_to_map)/sizeof(bcm_phys_irqs_to_map[0]); i++)
    {
        // gic counts SPIs from 0, so extract SPI_TABLE_OFFSET (32 PPIs)
        oirq.args[1] = bcm_phys_irqs_to_map[i] - SPI_TABLE_OFFSET;
        bcm_legacy_irq_map[oirq.args[1]] = irq_create_of_mapping(&oirq);
        printk(KERN_INFO "             % 4d       % 3d\n", bcm_phys_irqs_to_map[i], bcm_legacy_irq_map[oirq.args[1]]);
    }

    printk(KERN_INFO "     Remapping IO memories...\n");
    printk(KERN_INFO "             phys              virt          size\n");
    for(i=0; i < sizeof(bcm_io_blocks)/sizeof(bcm_io_blocks[0]); i++)
    {
        bcm_io_block_address[bcm_io_blocks[i].index] = (unsigned long)ioremap(bcm_io_blocks[i].address, bcm_io_blocks[i].size);
        printk(KERN_INFO "       %016lx  %016lx  %08x\n", bcm_io_blocks[i].address, bcm_io_block_address[bcm_io_blocks[i].index], bcm_io_blocks[i].size);
    }
    init = 1;
    return NOERR;

}
EXPORT_SYMBOL(bcm_legacy_irq_map);
EXPORT_SYMBOL(bcm_io_block_address);
EXPORT_SYMBOL(bcm_io_blocks);

// little hanky-panky, register driver's init function as if it is clk src init - to be called early in boot
CLOCKSOURCE_OF_DECLARE(brcm_legacy, "brcm,brcm-legacy", brcm_legacy_init);


