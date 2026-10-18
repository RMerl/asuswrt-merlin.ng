import gdb

GICD_CTLR	=	0x0000
GICD_TYPER	=	0x0004
GICD_IIDR	=	0x0008
GICD_IGROUPR	=	0x0080
GICD_ISENABLER	=	0x0100
GICD_ICENABLER	=	0x0180
GICD_ISPENDR	=	0x0200
GICD_ICPENDR	=	0x0280
GICD_ISACTIVER	=	0x0300
GICD_ICACTIVER	=	0x0380
GICD_IPRIORITYR	=	0x0400
GICD_ITARGETSR  =       0x0800
GICD_ICFGR	=	0x0C00
GICD_NSACR	=	0x0E00
GICD_CPENDSGIR	=	0x0F10
GICD_SPENDSGIR	=	0x0F20

GICC_CTLR	=	0x0000
GICC_PMR	=	0x0004
GICC_BPR	=	0x0008
GICC_IAR	=	0x000C
GICC_RPR	=	0x0014
GICC_HPPIR	=	0x0018
GICC_ABPR	=	0x001C
GICC_AIAR	=	0x0020
GICC_AHPPIR	=	0x0028
GICC_APR	=	0x00D0
GICC_NSAPR	=	0x00E0
GICC_IIDR	=	0x00FC
GICC_DIR	=	0x1000

# Offset wrt cpu interface base
GICH_OFFSET     =       0x3000
GICV_OFFSET     =       0x4000

GICH_HCR        =       0x000
GICH_VTR        =       0x004
GICH_VMCR       =       0x008
GICH_MISR       =       0x010
GICH_EISR0      =       0x020
GICH_ELSR0      =       0x030
GICH_APR0       =       0x0F0
GICH_LR0        =       0x100
GICH_LR1        =       0x104
GICH_LR2        =       0x108
GICH_LR3        =       0x10C

NR_SGI          =       16
NR_CPUS         =       4

def parse_and_cast(var, type_str):
    try:
        addr = str(var).split()[0]
        addr_type = gdb.lookup_type(type_str).pointer()
        val = gdb.parse_and_eval(addr).cast(addr_type).dereference()
        return val
    except gdb.error as e:
        gdb.write("Error parse_and_cast exception: {e}\n")
        return None

def parse_and_cast_struct(var, struct_name):
    try:
        addr = str(var).split()[0]
        addr_type = gdb.lookup_type(struct_name).pointer()
        val = gdb.parse_and_eval(addr).cast(addr_type).dereference()
        return val
    except gdb.error as e:
        gdb.write("Error parse_and_cast_struct: {e}\n")
        return None

def get_32bit_value(var):
    val = parse_and_cast(var, 'unsigned int')
    if val is None:
        gdb.write("val is None parse_and_cast return\n")
        return None
    return int(val)

def get_gic_chip_data():
    gic_data = parse_and_cast_struct("&gic_data", "struct gic_chip_data")
    if gic_data is None:
        gdb.write("\gic_data is None parse_and_cast_struct return\n")
        return

    dist_base = gic_data['dist_base']['common_base']
    cpu_base = (gic_data['cpu_base'])['common_base']
    gic_irqs = gic_data['gic_irqs']

    return gic_data, dist_base, cpu_base, gic_irqs

def dump_gic_chip_data(gic_data, dist_base, cpu_base, gic_irqs):
    if gic_data is None:
        gdb.write("Error: chip_data not found\n")
        return

    gic_data_address = str((gic_data['chip'])['name']).split()[0]
    gdb.write("\nGeneric Interrupt Controller Driver:\n")
    gdb.write("------------------------------------\n")

    gdb.write(
        "\t{:32s}\t{:s}\n".format(
            ((gic_data['chip'])['name']).string(), (gic_data_address)
        )
    )
    if dist_base is None:
        gdb.write("Error: Could not get GICD base\n")
        return
    gdb.write("\t{:32s}\t{:s}\n".format("dist_base", str(dist_base)))

    if cpu_base is None:
        gdb.write("Error: Could not get GICC base\n")
        return
    gdb.write("\t{:32s}\t{:s}\n".format("cpu_base", str(cpu_base)))

    if gic_irqs is None:
        gdb.write("Error: Could not get GIC_IRQS\n")
        return
    gdb.write("\t{:32s}\t{:s}\n".format("gic_irqs", str(gic_irqs)))
    gdb.write("\n")

def dump_gicd_registers(dist_base, gic_irqs):
    if dist_base is None:
        gdb.write("Error: Could not get Distributor base address\n")
        return

    gdb.write("\nDistributor Registers:\n")
    gdb.write("----------------------\n")

    gicd_reg_val = get_32bit_value(dist_base + GICD_CTLR)
    if gicd_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICD_CTRL:", hex(gicd_reg_val)))

    gicd_reg_val = get_32bit_value(dist_base + GICD_TYPER)

    if gicd_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICD_TYPER:", hex(gicd_reg_val)))
    gicd_reg_val = get_32bit_value(dist_base + GICD_IIDR)

    if gicd_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICD_IIDR:", hex(gicd_reg_val)))

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base + GICD_IGROUPR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_IGROUPR{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1
                    ), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base + GICD_ISENABLER + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ISENABLER{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1
                    ), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base + GICD_ICENABLER + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ICENABLER{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1
                    ), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_ISPENDR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ISPENDR{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_ICPENDR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ICPENDR{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_ISACTIVER + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ISACTIVER{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 32):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_ICACTIVER + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ICACTIVER{} IRQ#({} to {})".format(
                        i, i * 32, (i + 1) * 32 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 4):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_IPRIORITYR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_IPRIORITYR{} IRQ#({} to {})".format(
                        i, i * 4, (i + 1) * 4 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 4):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_ITARGETSR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ITARGETSR{} IRQ#({} to {})".format(
                        i, i * 4, (i + 1) * 4 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 16):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_ICFGR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_ICFGR{} IRQ#({} to {})".format(
                        i, i * 16, (i + 1) * 16 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, gic_irqs / 16):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_NSACR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_NSACR{} IRQ#({} to {})".format(
                        i, i * 16, (i + 1) * 16 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, NR_SGI // 4):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_CPENDSGIR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_CPENDSGIR{} IRQ#({} to {})".format(
                        i, i * 4, (i + 1) * 4 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

    for i in range(0, NR_SGI // 4):
        offset = i * 4
        gicd_reg_val = get_32bit_value(dist_base +  GICD_SPENDSGIR + offset)
        if gicd_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format(
                    "GICD_SPENDSGIR{} IRQ#({} to {})".format(
                        i, i * 4, (i + 1) * 4 - 1), hex(gicd_reg_val)
                )
            )
    gdb.write("\n")

def dump_gicc_registers(cpu_base):
    if cpu_base is None:
        gdb.write("Error: GIC CPU_BASE not available\n")
        return

    gdb.write("\nCPU Interface Registers:\n")
    gdb.write("------------------------\n")

    gicc_reg_val = get_32bit_value(cpu_base + GICC_CTLR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_CTRL:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_PMR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_PMR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_BPR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_BPR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_IAR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_IAR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_RPR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_RPR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_HPPIR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_HPPIR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_ABPR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_ABPR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_AIAR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_AIAR:", hex(gicc_reg_val)))

    gicc_reg_val = get_32bit_value(cpu_base + GICC_AHPPIR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_AHPPIR:", hex(gicc_reg_val)))

    for i in range(0, NR_CPUS):
        offset = i * 4
        gicc_reg_val = get_32bit_value(cpu_base +  GICC_APR + offset)
        if gicc_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICC_APR{} CPU#{}".format(i, i),
                hex(gicc_reg_val))
            )
    gdb.write("\n")

    for i in range(0, NR_CPUS):
        offset = i * 4
        gicc_reg_val = get_32bit_value(cpu_base +  GICC_NSAPR + offset)
        if gicc_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICC_NSAPR{} CPU#{}".format(i, i),
                hex(gicc_reg_val))
            )
    gdb.write("\n")

    gicc_reg_val = get_32bit_value(cpu_base + GICC_IIDR)
    if gicc_reg_val is not None:
        gdb.write("\t{:32s}\t{:s}\n".format("GICC_IIDR:", hex(gicc_reg_val)))
    gdb.write("\n")

def dump_gich_registers(cpu_base):
    if cpu_base is None:
        gdb.write("Error: GIC CPU_BASE not available\n")
        return

    gdb.write("\nVirtual Interface Control Registers:\n")
    gdb.write("------------------------------------\n")

    for i in range(0, NR_CPUS):
        gich_base_offset = cpu_base + GICH_OFFSET + i * 200

        gich_reg_val = get_32bit_value(gich_base_offset + GICH_HCR)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_HCR{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_VTR)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_VTR{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_VMCR)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_VMCR{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_MISR)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_MISR{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_EISR0)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_EISR0{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_ELSR0)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_ELSR0{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_APR0)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_APR0{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_LR0)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_LR0{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_LR1)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_LR1{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_LR2)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_LR2{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
        gich_reg_val = get_32bit_value(gich_base_offset + GICH_LR3)
        if gich_reg_val is not None:
            gdb.write(
                "\t{:32s}\t{:s}\n".format("GICH_LR3{} CPU#{}".format(i, i),
                    hex(gich_reg_val))
            )
    gdb.write("\n")

class LxGicdDump(gdb.Command):
    def __init__(self):
        super(LxGicdDump, self).__init__("lx-gicd", gdb.COMMAND_DATA)

    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv (args)
        if len (argv) != 0:
            raise gdb.GdbError ("LxGIC takes no arguments\n")
        try:
            _, dist_base, _, gic_irqs = get_gic_chip_data()
            dump_gicd_registers(dist_base, gic_irqs)
        except KeyboardInterrupt:
            gdb.write("\n")
        except Exception as e:
            gdb.write("Error:", e)

LxGicdDump()

class LxGiccDump(gdb.Command):
    def __init__(self):
        super(LxGiccDump, self).__init__("lx-gicc", gdb.COMMAND_DATA)
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv (args)
        if len (argv) != 0:
            raise gdb.GdbError ("LxGICD takes no arguments\n")
        try:
            _,_,cpu_base,_ = get_gic_chip_data()
            dump_gicc_registers(cpu_base)
        except KeyboardInterrupt:
            gdb.write("\n")
        except Exception as e:
            gdb.write("Error:", e)

LxGiccDump()

class LxGichDump(gdb.Command):
    def __init__(self):
        super(LxGichDump, self).__init__("lx-gich", gdb.COMMAND_DATA)
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv (args)
        if len (argv) != 0:
            raise gdb.GdbError ("LxGICH takes no arguments\n")
        try:
            _,_,cpu_base,_ = get_gic_chip_data()
            dump_gich_registers(cpu_base)
        except KeyboardInterrupt:
            gdb.write("\n")
        except Exception as e:
            gdb.write("Error:", e)

LxGichDump()

class LxGicDump(gdb.Command):
    def __init__(self):
        super(LxGicDump, self).__init__("lx-gic", gdb.COMMAND_DATA)
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)
        if len(argv) != 0:
            raise gdb.GdbError("LxGIC takes no arguments\n")
        try:
            gic_data = get_gic_chip_data()
       	    dump_gic_chip_data(
                gic_data[0], gic_data[1], gic_data[2], gic_data[3])
        except KeyboardInterrupt:
            gdb.write("\n")
        except Exception as e:
            gdb.write("Error:", e)

LxGicDump()
