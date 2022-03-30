// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot file device/oprom/realmode/x86.c
 *
 * Copyright (C) 2007 Advanced Micro Devices, Inc.
 * Copyright (C) 2009-2010 coresystems GmbH
 */
#include <common.h>
#include <bios_emul.h>
#include <vbe.h>
#include <linux/linkage.h>
#include <asm/cache.h>
#include <asm/processor.h>
#include <asm/i8259.h>
#include <asm/io.h>
#include <asm/post.h>
#include "bios.h"

/* Interrupt handlers for each interrupt the ROM can call */
static int (*int_handler[256])(void);

/* to have a common register file for interrupt handlers */
X86EMU_sysEnv _X86EMU_env;

asmlinkage void (*realmode_call)(u32 addr, u32 eax, u32 ebx, u32 ecx, u32 edx,
				 u32 esi, u32 edi);

asmlinkage void (*realmode_interrupt)(u32 intno, u32 eax, u32 ebx, u32 ecx,
				      u32 edx, u32 esi, u32 edi);

static void setup_realmode_code(void)
{
	memcpy((void *)REALMODE_BASE, &asm_realmode_code,
	       asm_realmode_code_size);

	/* Ensure the global pointers are relocated properly. */
	realmode_call = PTR_TO_REAL_MODE(asm_realmode_call);
	realmode_interrupt = PTR_TO_REAL_MODE(__realmode_interrupt);

	debug("Real mode stub @%x: %d bytes\n", REALMODE_BASE,
	      asm_realmode_code_size);
}

static void setup_rombios(void)
{
	const char date[] = "06/11/99";
	memcpy((void *)0xffff5, &date, 8);

	const char ident[] = "PCI_ISA";
	memcpy((void *)0xfffd9, &ident, 7);

	/* system model: IBM-AT */
	writeb(0xfc, 0xffffe);
}

static int int_exception_handler(void)
{
	/* compatibility shim */
	struct eregs reg_info = {
		.eax = M.x86.R_EAX,
		.ecx = M.x86.R_ECX,
		.edx = M.x86.R_EDX,
		.ebx = M.x86.R_EBX,
		.esp = M.x86.R_ESP,
		.ebp = M.x86.R_EBP,
		.esi = M.x86.R_ESI,
		.edi = M.x86.R_EDI,
		.vector = M.x86.intno,
		.error_code = 0,
		.eip = M.x86.R_EIP,
		.cs = M.x86.R_CS,
		.eflags = M.x86.R_EFLG
	};
	struct eregs *regs = &reg_info;

	debug("Oops, exception %d while executing option rom\n", regs->vector);
	cpu_hlt();

	return 0;
}

static int int_unknown_handler(void)
{
	debug("Unsupported software interrupt #0x%x eax 0x%x\n",
	      M.x86.intno, M.x86.R_EAX);

	return -1;
}

/* setup interrupt handlers for mainboard */
void bios_set_interrupt_handler(int intnum, int (*int_func)(void))
{
	int_handler[intnum] = int_func;
}

static void setup_interrupt_handlers(void)
{
	int i;

	/*
	 * The first 16 int_handler functions are not BIOS services,
	 * but the CPU-generated exceptions ("hardware interrupts")
	 */
	for (i = 0; i < 0x10; i++)
		int_handler[i] = &int_exception_handler;

	/* Mark all other int_handler calls as unknown first */
	for (i = 0x10; i < 0x100; i++) {
		/* Skip if bios_set_interrupt_handler() isn't called first */
		if (int_handler[i])
			continue;

		 /*
		  * Now set the default functions that are actually needed
		  * to initialize the option roms. The board may override
		  * these with bios_set_interrupt_handler()
		 */
		switch (i) {
		case 0x10:
			int_handler[0x10] = &int10_handler;
			break;
		case 0x12:
			int_handler[0x12] = &int12_handler;
			break;
		case 0x16:
			int_handler[0x16] = &int16_handler;
			break;
		case 0x1a:
			int_handler[0x1a] = &int1a_handler;
			break;
		default:
			int_handler[i] = &int_unknown_handler;
			break;
		}
	}
}

static void write_idt_stub(void *target, u8 intnum)
{
	unsigned char *codeptr;

	codeptr = (unsigned char *)target;
	memcpy(codeptr, &__idt_handler, __idt_handler_size);
	codeptr[3] = intnum; /* modify int# in the code stub. */
}

static void setup_realmode_idt(void)
{
	struct realmode_idt *idts = NULL;
	int i;

	/*
	 * Copy IDT stub code for each interrupt. This might seem wasteful
	 * but it is really simple
	 */
	 for (i = 0; i < 256; i++) {
		idts[i].cs = 0;
		idts[i].offset = 0x1000 + (i * __idt_handler_size);
		write_idt_stub((void *)((ulong)idts[i].offset), i);
	}

	/*
	 * Many option ROMs use the hard coded interrupt entry points in the
	 * system bios. So install them at the known locations.
	 */

	/* int42 is the relocated int10 */
	write_idt_stub((void *)0xff065, 0x42);
	/* BIOS Int 11 Handler F000:F84D */
	write_idt_stub((void *)0xff84d, 0x11);
	/* BIOS Int 12 Handler F000:F841 */
	write_idt_stub((void *)0xff841, 0x12);
	/* BIOS Int 13 Handler F000:EC59 */
	write_idt_stub((void *)0xfec59, 0x13);
	/* BIOS Int 14 Handler F000:E739 */
	write_idt_stub((void *)0xfe739, 0x14);
	/* BIOS Int 15 Handler F000:F859 */
	write_idt_stub((void *)0xff859, 0x15);
	/* BIOS Int 16 Handler F000:E82E */
	write_idt_stub((void *)0xfe82e, 0x16);
	/* BIOS Int 17 Handler F000:EFD2 */
	write_idt_stub((void *)0xfefd2, 0x17);
	/* ROM BIOS Int 1A Handler F000:FE6E */
	write_idt_stub((void *)0xffe6e, 0x1a);
}

#ifdef CONFIG_FRAMEBUFFER_SET_VESA_MODE
static u8 vbe_get_mode_info(struct vbe_mode_info *mi)
{
	u16 buffer_seg;
	u16 buffer_adr;
	char *buffer;

	debug("VBE: Getting information about VESA mode %04x\n",
	      mi->video_mode);
	buffer = PTR_TO_REAL_MODE(asm_realmode_buffer);
	buffer_seg = (((unsigned long)buffer) >> 4) & 0xff00;
	buffer_adr = ((unsigned long)buffer) & 0xffff;

	realmode_interrupt(0x10, VESA_GET_MODE_INFO, 0x0000, mi->video_mode,
			   0x0000, buffer_seg, buffer_adr);
	memcpy(mi->mode_info_block, buffer, sizeof(struct vbe_mode_info));
	mi->valid = true;

	return 0;
}

static u8 vbe_set_mode(struct vbe_mode_info *mi)
{
	int video_mode = mi->video_mode;

	debug("VBE: Setting VESA mode %#04x\n", video_mode);
	/* request linear framebuffer mode */
	video_mode |= (1 << 14);
	/* don't clear the framebuffer, we do that later */
	video_mode |= (1 << 15);
	realmode_interrupt(0x10, VESA_SET_MODE, video_mode,
			   0x0000, 0x0000, 0x0000, 0x0000);

	return 0;
}

static void vbe_set_graphics(int vesa_mode, struct vbe_mode_info *mode_info)
{
	unsigned char *framebuffer;

	mode_info->video_mode = (1 << 14) | vesa_mode;
	vbe_get_mode_info(mode_info);

	framebuffer = (unsigned char *)(ulong)mode_info->vesa.phys_base_ptr;
	debug("VBE: resolution:  %dx%d@%d\n",
	      le16_to_cpu(mode_info->vesa.x_resolution),
	      le16_to_cpu(mode_info->vesa.y_resolution),
	      mode_info->vesa.bits_per_pixel);
	debug("VBE: framebuffer: %p\n", framebuffer);
	if (!framebuffer) {
		debug("VBE: Mode does not support linear framebuffer\n");
		return;
	}

	mode_info->video_mode &= 0x3ff;
	vbe_set_mode(mode_info);
}
#endif /* CONFIG_FRAMEBUFFER_SET_VESA_MODE */

void bios_run_on_x86(struct udevice *dev, unsigned long addr, int vesa_mode,
		     struct vbe_mode_info *mode_info)
{
	pci_dev_t pcidev = dm_pci_get_bdf(dev);
	u32 num_dev;

	num_dev = PCI_BUS(pcidev) << 8 | PCI_DEV(pcidev) << 3 |
			PCI_FUNC(pcidev);

	/* Needed to avoid exceptions in some ROMs */
	interrupt_init();

	/* Set up some legacy information in the F segment */
	setup_rombios();

	/* Set up C interrupt handlers */
	setup_interrupt_handlers();

	/* Set up real-mode IDT */
	setup_realmode_idt();

	/* Make sure the code is placed. */
	setup_realmode_code();

	debug("Calling Option ROM at %lx, pci device %#x...", addr, num_dev);

	/* Option ROM entry point is at OPROM start + 3 */
	realmode_call(addr + 0x0003, num_dev, 0xffff, 0x0000, 0xffff, 0x0,
		      0x0);
	debug("done\n");

#ifdef CONFIG_FRAMEBUFFER_SET_VESA_MODE
	if (vesa_mode != -1)
		vbe_set_graphics(vesa_mode, mode_info);
#endif
}

asmlinkage int interrupt_handler(u32 intnumber, u32 gsfs, u32 dses,
				 u32 edi, u32 esi, u32 ebp, u32 esp,
				 u32 ebx, u32 edx, u32 ecx, u32 eax,
				 u32 cs_ip, u16 stackflags)
{
	u32 ip;
	u32 cs;
	u32 flags;
	int ret = 0;

	ip = cs_ip & 0xffff;
	cs = cs_ip >> 16;
	flags = stackflags;

#ifdef CONFIG_REALMODE_DEBUG
	debug("oprom: INT# 0x%x\n", intnumber);
	debug("oprom: eax: %08x ebx: %08x ecx: %08x edx: %08x\n",
	      eax, ebx, ecx, edx);
	debug("oprom: ebp: %08x esp: %08x edi: %08x esi: %08x\n",
	      ebp, esp, edi, esi);
	debug("oprom:  ip: %04x      cs: %04x   flags: %08x\n",
	      ip, cs, flags);
	debug("oprom: stackflags = %04x\n", stackflags);
#endif

	/*
	 * Fetch arguments from the stack and put them to a place
	 * suitable for the interrupt handlers
	 */
	M.x86.R_EAX = eax;
	M.x86.R_ECX = ecx;
	M.x86.R_EDX = edx;
	M.x86.R_EBX = ebx;
	M.x86.R_ESP = esp;
	M.x86.R_EBP = ebp;
	M.x86.R_ESI = esi;
	M.x86.R_EDI = edi;
	M.x86.intno = intnumber;
	M.x86.R_EIP = ip;
	M.x86.R_CS = cs;
	M.x86.R_EFLG = flags;

	/* Call the interrupt handler for this interrupt number */
	ret = int_handler[intnumber]();

	/*
	 * This code is quite strange...
	 *
	 * Put registers back on the stack. The assembler code will pop them
	 * later. We force (volatile!) changing the values of the parameters
	 * of this function. We know that they stay alive on the stack after
	 * we leave this function.
	 */
	*(volatile u32 *)&eax = M.x86.R_EAX;
	*(volatile u32 *)&ecx = M.x86.R_ECX;
	*(volatile u32 *)&edx = M.x86.R_EDX;
	*(volatile u32 *)&ebx = M.x86.R_EBX;
	*(volatile u32 *)&esi = M.x86.R_ESI;
	*(volatile u32 *)&edi = M.x86.R_EDI;
	flags = M.x86.R_EFLG;

	/* Pass success or error back to our caller via the CARRY flag */
	if (ret) {
		flags &= ~1; /* no error: clear carry */
	} else {
		debug("int%02x call returned error\n", intnumber);
		flags |= 1;  /* error: set carry */
	}
	*(volatile u16 *)&stackflags = flags;

	return ret;
}
