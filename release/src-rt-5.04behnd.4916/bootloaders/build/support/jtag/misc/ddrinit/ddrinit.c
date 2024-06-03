//**************************************************************//
// 63138 DDR init script                                        //
// 400MHz DDR3 clock, 1024Mb size                               //
//**************************************************************//

//unsigned char * 0x80002000=(unsigned char *)0x80002000;
//unsigned char * 0x80003000=(unsigned char *)0x80003000;
//

#define NULL (void*)0

unsigned int  GLB_GCFG=0x4;
unsigned int  CHN_TIM_DCMD=0x200;
unsigned int  CHN_TIM_CLKS=0x20c;
unsigned int  CHN_TIM_PHY_ST=0x230;
unsigned int  CHN_CFG_ROW00_0=0x110;
unsigned int  CHN_CFG_ROW00_1=0x114;
unsigned int  CHN_CFG_ROW01_0=0x118;
unsigned int  CHN_CFG_ROW01_1=0x11c;
unsigned int  CHN_CFG_ROW20_0=0x130;
unsigned int  CHN_CFG_ROW20_1=0x134;
unsigned int  CHN_CFG_ROW21_0=0x138;
unsigned int  CHN_CFG_ROW21_1=0x13c;
unsigned int  CHN_CFG_COL00_0=0x150;
unsigned int  CHN_CFG_COL00_1=0x154;
unsigned int  CHN_CFG_COL01_0=0x158;
unsigned int  CHN_CFG_COL01_1=0x15c;
unsigned int  CHN_CFG_COL19_0=0x170;
unsigned int  CHN_CFG_COL20_1=0x174;
unsigned int  CHN_CFG_COL21_0=0x178;
unsigned int  CHN_CFG_COL21_1=0x17c;
unsigned int  CHN_CFG_BNK10=0x190;
unsigned int  CHN_CFG_BNK32=0x194;
unsigned int  CHN_TIM_TIM1_0=0x214;
unsigned int  CHN_TIM_TIM1_1=0x218;
unsigned int  CHN_TIM_TIM2=0x21c;
unsigned int  CHN_TIM_DRAM_CFG=0x234;
unsigned int value=0;
unsigned int flag=0;


#define UART0 0xfffe8600

typedef struct UART
{
	unsigned int ctrl;
	unsigned int baud;
	unsigned int misc_ctrl;
	unsigned int ext_input;
	unsigned int intsttusmask;
	unsigned int fifo;
}UART;
UART *uart=(UART*)UART0;
void init_uart(void)
{
	uart->ctrl=0x00e03700;
	uart->baud=0x0000000d;
		
		
}

void print_uart(char *ptr)
{
	if(ptr != NULL)
	{
		while(ptr[0] != '\0')
		{
			while(!(uart->intsttusmask & (0x1<<5))); 
			uart->fifo=ptr[0];
			ptr++;
		}
	}
}

void done()
{
	while(1);	
}

void ddrinit(void)
{

	init_uart();

print_uart("\r\nDDR initialization...");

//make sure we use the on alway on rbus clock     
 value=*(volatile unsigned long*)(0x80002000+GLB_GCFG)&0xfffffdff;
 *(unsigned int *) (0x80002000+GLB_GCFG) = value;
 
//config phy_4x_mode

 value=*(volatile unsigned long*)(0x80002000+GLB_GCFG)|(0x1<<0x18);
 *(unsigned int *) (0x80002000+GLB_GCFG) = value;

//Poll the PHY Status Register
poll_phy_sts:
// - check Power_Up to see if PHY is ready to receive register access
// - check SW_Reset
// - check HW_Reset
 value=*(volatile unsigned long*)(0x80002000+CHN_TIM_PHY_ST);
 flag=value&0x1;
 if (flag==0)
      goto poll_phy_sts;   

 flag=value&0x2;
 if (flag==0x2)
      goto poll_phy_sts;   

 flag=value&0x4;
 if (flag==0x4)
      goto poll_phy_sts;   
 

print_uart("\r\nPHY initialization starts...");
//assume 400MHz clock and 1024Mb

   *(unsigned int *) (0x80003000+0x23c ) = 0x000000c1 ;//  WRITE  0x1000023c 0x000000c1 DDR34_CORE_PHY_CONTROL_REGS_DFI_CNTRL;
   *(unsigned int *) (0x80003000+0x040 ) = 0x02001520 ;//  WRITE  0x10000040 0x02001520 DDR34_CORE_PHY_CONTROL_REGS_DRAM_CONFIG;
   *(unsigned int *) (0x80003000+0x044 ) = 0x0f040606 ;//  WRITE  0x10000044 0x0f040606 DDR34_CORE_PHY_CONTROL_REGS_DRAM_TIMING1;
   *(unsigned int *) (0x80003000+0x048 ) = 0x04060506 ;//  WRITE  0x10000048 0x04060506 DDR34_CORE_PHY_CONTROL_REGS_DRAM_TIMING2;
   *(unsigned int *) (0x80003000+0x04c ) = 0x0004402c ;//  WRITE  0x1000004c 0x0004402c DDR34_CORE_PHY_CONTROL_REGS_DRAM_TIMING3;
   *(unsigned int *) (0x80003000+0x050 ) = 0x00000000 ;//  WRITE  0x10000050 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_DRAM_TIMING4;
   *(unsigned int *) (0x80003000+0x008 ) = 0x018d0012 ;//  WRITE  0x10000008 0x018d0012 DDR34_CORE_PHY_CONTROL_REGS_PLL_CONFIG;
   *(unsigned int *) (0x80003000+0x018 ) = 0x004030BF ;//  WRITE  0x10000018 0x004030BF DDR34_CORE_PHY_CONTROL_REGS_PLL_DIVIDERS;
   *(unsigned int *) (0x80003000+0x010 ) = 0x9c000000 ;//  WRITE  0x10000010 0x9c000000 DDR34_CORE_PHY_CONTROL_REGS_PLL_CONTROL2;
   *(unsigned int *) (0x80003000+0x008 ) = 0x018d0010 ;//  WRITE  0x10000008 0x018d0010 DDR34_CORE_PHY_CONTROL_REGS_PLL_CONFIG;
   
phy_wait1:
   value=*(volatile unsigned long*)(0x80003000+0x4) ;//  READ   0x10000004 0x000034b4 DDR34_CORE_PHY_CONTROL_REGS_PLL_STATUS
   flag=value&0x1;
   if (flag!=1)
       goto phy_wait1;
   
   *(unsigned int *) (0x80003000+0x008 ) = 0x018d0000 ;//  WRITE  0x10000008 0x018d0000 DDR34_CORE_PHY_CONTROL_REGS_PLL_CONFIG;
   *(unsigned int *) (0x80003000+0x200 ) = 0x00000820 ;//  WRITE  0x10000200 0x00000820 DDR34_CORE_PHY_CONTROL_REGS_VREF_DAC_CONTROL;
   *(unsigned int *) (0x80003000+0x060 ) = 0x00000000 ;//  WRITE  0x10000060 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_VDL_CALIBRATE;
   *(unsigned int *) (0x80003000+0x060 ) = 0x00000019 ;//  WRITE  0x10000060 0x00000019 DDR34_CORE_PHY_CONTROL_REGS_VDL_CALIBRATE;

phy_wait2:
   value=*(volatile unsigned long*)(0x80003000+0x64) ;//  READ   0x10000064 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_VDL_CALIB_STATUS1
   flag=value&0x1;
   if (flag!=1)
       goto phy_wait2;

   *(unsigned int *) (0x80003000+0x060 ) = 0x00000000 ;//  WRITE  0x10000060 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_VDL_CALIBRATE;
   *(unsigned int *) (0x80003000+0x0fc ) = 0x0001003b ;//  WRITE  0x100000fc 0x0001003b DDR34_CORE_PHY_CONTROL_REGS_VDL_CONTROL_CKE;
   *(unsigned int *) (0x80003000+0x0f0 ) = 0x0001003b ;//  WRITE  0x100000f0 0x0001003b DDR34_CORE_PHY_CONTROL_REGS_VDL_CONTROL_PAR;
   *(unsigned int *) (0x80003000+0x4a0 ) = 0x00010011 ;//  WRITE  0x100004a0 0x00010011 DDR34_CORE_PHY_BYTE_LANE_0_RD_EN_DLY_CYC;
   *(unsigned int *) (0x80003000+0x488 ) = 0x0001002a ;//  WRITE  0x10000488 0x0001002a DDR34_CORE_PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0;
   *(unsigned int *) (0x80003000+0x48c ) = 0x0001002a ;//  WRITE  0x1000048c 0x0001002a DDR34_CORE_PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1;
   *(unsigned int *) (0x80003000+0x6a0 ) = 0x00010011 ;//  WRITE  0x100006a0 0x00010011 DDR34_CORE_PHY_BYTE_LANE_1_RD_EN_DLY_CYC;
   *(unsigned int *) (0x80003000+0x688 ) = 0x0001002a ;//  WRITE  0x10000688 0x0001002a DDR34_CORE_PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0;
   *(unsigned int *) (0x80003000+0x68c ) = 0x0001002a ;//  WRITE  0x1000068c 0x0001002a DDR34_CORE_PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1;
   *(unsigned int *) (0x80003000+0x1f8 ) = 0x0fc7ffff ;// VTT_CONNECTIONS;
   *(unsigned int *) (0x80003000+0x1fc ) = 0x0007ffff ;// VTT_OVERRIDES;
   *(unsigned int *) (0x80003000+0x1f0 ) = 0x00000004 ;// ENABLE_VTT_CTL;
   *(unsigned int *) (0x80003000+0x038 ) = 0x00031bff ;//  WRITE  0x10000038 0x00031bff DDR34_CORE_PHY_CONTROL_REGS_DRIVE_PAD_CTL;
   *(unsigned int *) (0x80003000+0x4cc ) = 0x00031bff ;//  WRITE  0x100004cc 0x00031bff DDR34_CORE_PHY_BYTE_LANE_0_DRIVE_PAD_CTL;
   *(unsigned int *) (0x80003000+0x6cc ) = 0x00031bff ;//  WRITE  0x100006cc 0x00031bff DDR34_CORE_PHY_BYTE_LANE_1_DRIVE_PAD_CTL;
   *(unsigned int *) (0x80003000+0x4d0 ) = 0x000fffff ;//  WRITE  0x100004d0 0x000fffff DDR34_CORE_PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL;
   *(unsigned int *) (0x80003000+0x6d0 ) = 0x000fffff ;//  WRITE  0x100006d0 0x000fffff DDR34_CORE_PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL;

   *(unsigned int *) (0x80003000+0x03c ) = 0x01000000 ;//  WRITE  0x1000003c 0x01000000 DDR34_CORE_PHY_CONTROL_REGS_STATIC_PAD_CTL;
   *(unsigned int *) (0x80003000+0x240 ) = 0x00001800 ;// Set Write ODT;
   *(unsigned int *) (0x80003000+0x4e0 ) = 0x0000023b ;// Set Read ODT for BL0;
   *(unsigned int *) (0x80003000+0x6e0 ) = 0x0000023b ;// Set Read ODT for BL1;
   *(unsigned int *) (0x80003000+0x4d4 ) = 0x00000000 ;//  WRITE  0x100004d4 0x00000000 DDR34_CORE_PHY_BYTE_LANE_0_STATIC_PAD_CTL;
   *(unsigned int *) (0x80003000+0x6d4 ) = 0x00000000 ;//  WRITE  0x100006d4 0x00000000 DDR34_CORE_PHY_BYTE_LANE_1_STATIC_PAD_CTL;
   *(unsigned int *) (0x80003000+0x4d8 ) = 0x00000e02 ;//  WRITE  0x100004d8 0x00000e02 DDR34_CORE_PHY_BYTE_LANE_0_WR_PREAMBLE_MODE;
   *(unsigned int *) (0x80003000+0x6d8 ) = 0x00000e02 ;//  WRITE  0x100006d8 0x00000e02 DDR34_CORE_PHY_BYTE_LANE_1_WR_PREAMBLE_MODE;
   *(unsigned int *) (0x80003000+0x4c8 ) = 0x0000000a ;//  WRITE  0x100004c8 0x0000000a DDR34_CORE_PHY_BYTE_LANE_0_IDLE_PAD_CONTROL;
   *(unsigned int *) (0x80003000+0x6c8 ) = 0x0000000a ;//  WRITE  0x100006c8 0x0000000a DDR34_CORE_PHY_BYTE_LANE_1_IDLE_PAD_CONTROL;

   *(unsigned int *) (0x80003000+0x23c ) = 0x000000c1 ;//  WRITE  0x1000023c 0x000000c1 DDR34_CORE_PHY_CONTROL_REGS_DFI_CNTRL;
   *(unsigned int *) (0x80003000+0x23c ) = 0x000000e1 ;//  WRITE  0x1000023c 0x000000e1 DDR34_CORE_PHY_CONTROL_REGS_DFI_CNTRL;
   *(unsigned int *) (0x80003000+0x23c ) = 0x000000f9 ;//  WRITE  0x1000023c 0x000000f9 DDR34_CORE_PHY_CONTROL_REGS_DFI_CNTRL;
   *(unsigned int *) (0x80003000+0x144 ) = 0x00000000 ;//  WRITE  0x10000144 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_AUX_REG1;
   *(unsigned int *) (0x80003000+0x140 ) = 0x00020200 ;//  WRITE  0x10000140 0x00020200 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_REG1;
   *(unsigned int *) (0x80003000+0x144 ) = 0x00000000 ;//  WRITE  0x10000144 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_AUX_REG1;
   *(unsigned int *) (0x80003000+0x140 ) = 0x00030000 ;//  WRITE  0x10000140 0x00030000 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_REG1;
   *(unsigned int *) (0x80003000+0x144 ) = 0x00000000 ;//  WRITE  0x10000144 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_AUX_REG1;
   *(unsigned int *) (0x80003000+0x140 ) = 0x00010006 ;//  WRITE  0x10000140 0x00010006 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_REG1;
   *(unsigned int *) (0x80003000+0x144 ) = 0x00000000 ;//  WRITE  0x10000144 0x00000000 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_AUX_REG1;
   *(unsigned int *) (0x80003000+0x140 ) = 0x00001520 ;//  WRITE  0x10000140 0x00001520 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_REG1;
   *(unsigned int *) (0x80003000+0x140 ) = 0x00300400 ;//  WRITE  0x10000140 0x00300400 DDR34_CORE_PHY_CONTROL_REGS_COMMAND_REG1;

   value=*(volatile unsigned long*)(0x80003000+0x0) ;
   //WAIT 1.ms

   *(unsigned int *) (0x80003000+0x23c ) = 0x000000f8 ;//  WRITE  0x1000023c 0x000000f8 DDR34_CORE_PHY_CONTROL_REGS_DFI_CNTRL;

// switching the mclksrc muxsel to use PLL clk instead of RCLK
 value=*(volatile unsigned long*)(0x80002000+GLB_GCFG)|(0x1<<0x9);
 *(unsigned int *) (0x80002000+GLB_GCFG) = value;

print_uart("\r\nPHY initialization done");

//Enable CKE
 *(unsigned int *) (0x80002000+CHN_TIM_DCMD) = 0x00000305;

//assume 400MHz clock and 1024Mb
print_uart("\r\nDisable Auto-Refresh");
 *(unsigned int *) (0x80002000+CHN_TIM_CLKS) = ((0x5d<<0x8)|(0x1<<0x10));

//map address bit to row, col and bank address
 *(unsigned int *) (0x80002000+CHN_CFG_ROW00_0) = 0x11100f0e;
 *(unsigned int *) (0x80002000+CHN_CFG_ROW00_1) = 0x15141312;
 *(unsigned int *) (0x80002000+CHN_CFG_ROW01_0) = 0x19181716;
 *(unsigned int *) (0x80002000+CHN_CFG_ROW01_1) = 0x1a;
 *(unsigned int *) (0x80002000+CHN_CFG_COL00_0) = 0x4000000;
 *(unsigned int *) (0x80002000+CHN_CFG_COL00_1) = 0xb0a0905;
 *(unsigned int *) (0x80002000+CHN_CFG_COL01_0) = 0xd0c;
 *(unsigned int *) (0x80002000+CHN_CFG_BNK10  ) = 0x80706;

// set dram timing
 *(unsigned int *) (0x80002000+CHN_TIM_TIM1_0) = 0x00465667;
// set tRFC with the worst case of 8Gb and 400MHz 
 *(unsigned int *) (0x80002000+CHN_TIM_TIM1_1) = 0x22781415;
 *(unsigned int *) (0x80002000+CHN_TIM_TIM2  ) = 0x00004040;

print_uart("\r\nSetting the DDR3 mode");

 value=(*(volatile unsigned long*)(0x80002000+CHN_TIM_DRAM_CFG)&0xfffffff0)|0x1;
 *(unsigned int *) (0x80002000+CHN_TIM_DRAM_CFG) = value;

print_uart("\r\nEnable Auto-Refresh");
 *(unsigned int *) (0x80002000+CHN_TIM_CLKS) = (0x5d<<0x8);
       
// Set the flag indicating that memory initialization is done
// Set memory size to 64MB
 value=*(volatile unsigned long*)(0x80002000+GLB_GCFG)|((0x1<<0x1F)|(0x1<<0x8)|(0x7));
 *(unsigned int *) (0x80002000+GLB_GCFG) = value;

print_uart("\r\nDDR initialization done. Set DDR to 400MHz and 128MB");
//
print_uart("\r\nDDR init successful\n");

	done();

}



/*
void _start()
{

	__asm("mov r0, #0x0");
	__asm("mov r2, #0x0");
	__asm("mov r3, #0x0");
	__asm("mov r4, #0x0");
	__asm("mov r5, #0x0");
	__asm("mov r6, #0x0");
	__asm("mov r7, #0x0");
	__asm("mov r8, #0x0");
	__asm("mov r9, #0x0");
	__asm("mov r10, #0x0");
	__asm("mov r11, #0x0");
	__asm("mov r12, #0x0");
	__asm("mov r13, #0x0");
	__asm("mov r14, #0x0");
	__asm("mov r1, #0xfffe");
	__asm("lsl r1, r1, #0x10");
	__asm("add r1, #0x6000");
	__asm("mov sp, r1");
	__asm("mov r1, #0x0");
	ddrinit();

	
}
*/



