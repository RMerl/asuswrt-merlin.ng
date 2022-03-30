// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG, d.peter@mpl.ch.
 */
/*
 * Floppy Disk support
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <image.h>


#undef	FDC_DEBUG

#ifdef	FDC_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/*#if defined(CONFIG_CMD_DATE) */
/*#include <rtc.h> */
/*#endif */

typedef struct {
	int		flags;		/* connected drives ect */
	unsigned long	blnr;		/* Logical block nr */
	uchar		drive;		/* drive no */
	uchar		cmdlen;		/* cmd length */
	uchar		cmd[16];	/* cmd desc */
	uchar		dma;		/* if > 0 dma enabled */
	uchar		result[11];	/* status information */
	uchar		resultlen;	/* lenght of result */
} FDC_COMMAND_STRUCT;

/* flags: only the lower 8bit used:
 * bit 0 if set drive 0 is present
 * bit 1 if set drive 1 is present
 * bit 2 if set drive 2 is present
 * bit 3 if set drive 3 is present
 * bit 4 if set disk in drive 0 is inserted
 * bit 5 if set disk in drive 1 is inserted
 * bit 6 if set disk in drive 2 is inserted
 * bit 7 if set disk in drive 4 is inserted
 */

/* cmd indexes */
#define COMMAND			0
#define DRIVE			1
#define CONFIG0			1
#define SPEC_HUTSRT		1
#define TRACK			2
#define CONFIG1			2
#define SPEC_HLT		2
#define HEAD			3
#define CONFIG2			3
#define SECTOR			4
#define SECTOR_SIZE		5
#define LAST_TRACK		6
#define GAP			7
#define DTL			8
/* result indexes */
#define STATUS_0		0
#define STATUS_PCN		1
#define STATUS_1		1
#define STATUS_2		2
#define STATUS_TRACK		3
#define STATUS_HEAD		4
#define STATUS_SECT		5
#define STATUS_SECT_SIZE	6


/* Register addresses */
#define FDC_BASE	0x3F0
#define FDC_SRA		FDC_BASE + 0	/* Status Register A */
#define FDC_SRB		FDC_BASE + 1	/* Status Register B */
#define FDC_DOR		FDC_BASE + 2	/* Digital Output Register */
#define FDC_TDR		FDC_BASE + 3	/* Tape Drive Register */
#define FDC_DSR		FDC_BASE + 4	/* Data rate Register */
#define FDC_MSR		FDC_BASE + 4	/* Main Status Register */
#define FDC_FIFO	FDC_BASE + 5	/* FIFO */
#define FDC_DIR		FDC_BASE + 6	/* Digital Input Register */
#define FDC_CCR		FDC_BASE + 7	/* Configuration Control */
/* Commands */
#define FDC_CMD_SENSE_INT	0x08
#define FDC_CMD_CONFIGURE	0x13
#define FDC_CMD_SPECIFY		0x03
#define FDC_CMD_RECALIBRATE	0x07
#define FDC_CMD_READ		0x06
#define FDC_CMD_READ_TRACK	0x02
#define FDC_CMD_READ_ID		0x0A
#define FDC_CMD_DUMP_REG	0x0E
#define FDC_CMD_SEEK		0x0F

#define FDC_CMD_SENSE_INT_LEN	0x01
#define FDC_CMD_CONFIGURE_LEN	0x04
#define FDC_CMD_SPECIFY_LEN	0x03
#define FDC_CMD_RECALIBRATE_LEN	0x02
#define FDC_CMD_READ_LEN	0x09
#define FDC_CMD_READ_TRACK_LEN	0x09
#define FDC_CMD_READ_ID_LEN	0x02
#define FDC_CMD_DUMP_REG_LEN	0x01
#define FDC_CMD_SEEK_LEN	0x03

#define FDC_FIFO_THR		0x0C
#define FDC_FIFO_DIS		0x00
#define FDC_IMPLIED_SEEK	0x01
#define FDC_POLL_DIS		0x00
#define FDC_PRE_TRK		0x00
#define FDC_CONFIGURE		FDC_FIFO_THR | (FDC_POLL_DIS<<4) | (FDC_FIFO_DIS<<5) | (FDC_IMPLIED_SEEK << 6)
#define FDC_MFM_MODE		0x01 /* MFM enable */
#define FDC_SKIP_MODE		0x00 /* skip enable */

#define FDC_TIME_OUT 100000 /* time out */
#define	FDC_RW_RETRIES		3 /* read write retries */
#define FDC_CAL_RETRIES		3 /* calibration and seek retries */


/* Disk structure */
typedef struct  {
	unsigned int size;	/* nr of sectors total */
	unsigned int sect;	/* sectors per track */
	unsigned int head;	/* nr of heads */
	unsigned int track;	/* nr of tracks */
	unsigned int stretch;	/* !=0 means double track steps */
	unsigned char	gap;	/* gap1 size */
	unsigned char	rate;	/* data rate. |= 0x40 for perpendicular */
	unsigned char	spec1;	/* stepping rate, head unload time */
	unsigned char	fmt_gap;/* gap2 size */
	unsigned char hlt;	/* head load time */
	unsigned char sect_code;/* Sector Size code */
	const char	* name;	/* used only for predefined formats */
} FD_GEO_STRUCT;


/* supported Floppy types (currently only one) */
const static FD_GEO_STRUCT floppy_type[2] = {
	{ 2880,18,2,80,0,0x1B,0x00,0xCF,0x6C,16,2,"H1440" },	/*  7 1.44MB 3.5"   */
	{    0, 0,0, 0,0,0x00,0x00,0x00,0x00, 0,0,NULL    },	/*  end of table    */
};

static FDC_COMMAND_STRUCT cmd; /* global command struct */

/* If the boot drive number is undefined, we assume it's drive 0             */
#ifndef CONFIG_SYS_FDC_DRIVE_NUMBER
#define CONFIG_SYS_FDC_DRIVE_NUMBER 0
#endif

/* Hardware access */
#ifndef CONFIG_SYS_ISA_IO_STRIDE
#define CONFIG_SYS_ISA_IO_STRIDE 1
#endif

#ifndef CONFIG_SYS_ISA_IO_OFFSET
#define CONFIG_SYS_ISA_IO_OFFSET 0
#endif

/* Supporting Functions */
/* reads a Register of the FDC */
unsigned char read_fdc_reg(unsigned int addr)
{
	volatile unsigned char *val =
		(volatile unsigned char *)(CONFIG_SYS_ISA_IO_BASE_ADDRESS +
					   (addr * CONFIG_SYS_ISA_IO_STRIDE) +
					   CONFIG_SYS_ISA_IO_OFFSET);

	return val [0];
}

/* writes a Register of the FDC */
void write_fdc_reg(unsigned int addr, unsigned char val)
{
	volatile unsigned char *tmp =
		(volatile unsigned char *)(CONFIG_SYS_ISA_IO_BASE_ADDRESS +
					   (addr * CONFIG_SYS_ISA_IO_STRIDE) +
					   CONFIG_SYS_ISA_IO_OFFSET);
	tmp[0]=val;
}

/* waits for an interrupt (polling) */
int wait_for_fdc_int(void)
{
	unsigned long timeout;
	timeout = FDC_TIME_OUT;
	while((read_fdc_reg(FDC_SRA)&0x80)==0) {
		timeout--;
		udelay(10);
		if(timeout==0) /* timeout occurred */
			return false;
	}
	return true;
}

/* reads a byte from the FIFO of the FDC and checks direction and RQM bit
   of the MSR. returns -1 if timeout, or byte if ok */
int read_fdc_byte(void)
{
	unsigned long timeout;
	timeout = FDC_TIME_OUT;
	while((read_fdc_reg(FDC_MSR)&0xC0)!=0xC0) {
		/* direction out and ready */
		udelay(10);
		timeout--;
		if(timeout==0) /* timeout occurred */
			return -1;
	}
	return read_fdc_reg(FDC_FIFO);
}

/* if the direction of the FIFO is wrong, this routine is used to
   empty the FIFO. Should _not_ be used */
int fdc_need_more_output(void)
{
	unsigned char c;
	while((read_fdc_reg(FDC_MSR)&0xC0)==0xC0)	{
			c=(unsigned char)read_fdc_byte();
			printf("Error: more output: %x\n",c);
	}
	return true;
}


/* writes a byte to the FIFO of the FDC and checks direction and RQM bit
   of the MSR */
int write_fdc_byte(unsigned char val)
{
	unsigned long timeout;
	timeout = FDC_TIME_OUT;
	while((read_fdc_reg(FDC_MSR)&0xC0)!=0x80) {
		/* direction in and ready for byte */
		timeout--;
		udelay(10);
		fdc_need_more_output();
		if(timeout==0) /* timeout occurred */
			return false;
	}
	write_fdc_reg(FDC_FIFO,val);
	return true;
}

/* sets up all FDC commands and issues it to the FDC. If
   the command causes direct results (no Execution Phase)
   the result is be read as well. */

int fdc_issue_cmd(FDC_COMMAND_STRUCT *pCMD,FD_GEO_STRUCT *pFG)
{
	int i;
	unsigned long head,track,sect,timeout;
	track = pCMD->blnr / (pFG->sect * pFG->head); /* track nr */
	sect =  pCMD->blnr % (pFG->sect * pFG->head); /* remaining blocks */
	head = sect / pFG->sect; /* head nr */
	sect =  sect % pFG->sect; /* remaining blocks */
	sect++; /* sectors are 1 based */
	PRINTF("Cmd 0x%02x Track %ld, Head %ld, Sector %ld, Drive %d (blnr %ld)\n",
		pCMD->cmd[0],track,head,sect,pCMD->drive,pCMD->blnr);

	if(head|=0) { /* max heads = 2 */
		pCMD->cmd[DRIVE]=pCMD->drive | 0x04; /* head 1 */
		pCMD->cmd[HEAD]=(unsigned char) head; /* head register */
	}
	else {
		pCMD->cmd[DRIVE]=pCMD->drive; /* head 0 */
		pCMD->cmd[HEAD]=(unsigned char) head; /* head register */
	}
	pCMD->cmd[TRACK]=(unsigned char) track; /* track */
	switch (pCMD->cmd[COMMAND]) {
		case FDC_CMD_READ:
			pCMD->cmd[SECTOR]=(unsigned char) sect; /* sector */
			pCMD->cmd[SECTOR_SIZE]=pFG->sect_code; /* sector size code */
			pCMD->cmd[LAST_TRACK]=pFG->sect; /* End of track */
			pCMD->cmd[GAP]=pFG->gap; /* gap */
			pCMD->cmd[DTL]=0xFF; /* DTL */
			pCMD->cmdlen=FDC_CMD_READ_LEN;
			pCMD->cmd[COMMAND]|=(FDC_MFM_MODE<<6); /* set MFM bit */
			pCMD->cmd[COMMAND]|=(FDC_SKIP_MODE<<5); /* set Skip bit */
			pCMD->resultlen=0;  /* result only after execution */
			break;
		case FDC_CMD_SEEK:
			pCMD->cmdlen=FDC_CMD_SEEK_LEN;
			pCMD->resultlen=0;  /* no result */
			break;
		case FDC_CMD_CONFIGURE:
			pCMD->cmd[CONFIG0]=0;
			pCMD->cmd[CONFIG1]=FDC_CONFIGURE; /* FIFO Threshold, Poll, Enable FIFO */
			pCMD->cmd[CONFIG2]=FDC_PRE_TRK;	/* Precompensation Track */
			pCMD->cmdlen=FDC_CMD_CONFIGURE_LEN;
			pCMD->resultlen=0;  /* no result */
			break;
		case FDC_CMD_SPECIFY:
			pCMD->cmd[SPEC_HUTSRT]=pFG->spec1;
			pCMD->cmd[SPEC_HLT]=(pFG->hlt)<<1; /* head load time */
			if(pCMD->dma==0)
				pCMD->cmd[SPEC_HLT]|=0x1; /* no dma */
			pCMD->cmdlen=FDC_CMD_SPECIFY_LEN;
			pCMD->resultlen=0;  /* no result */
			break;
		case FDC_CMD_DUMP_REG:
			pCMD->cmdlen=FDC_CMD_DUMP_REG_LEN;
			pCMD->resultlen=10;  /* 10 byte result */
			break;
		case FDC_CMD_READ_ID:
			pCMD->cmd[COMMAND]|=(FDC_MFM_MODE<<6); /* set MFM bit */
			pCMD->cmdlen=FDC_CMD_READ_ID_LEN;
			pCMD->resultlen=7;  /* 7 byte result */
			break;
		case FDC_CMD_RECALIBRATE:
			pCMD->cmd[DRIVE]&=0x03; /* don't set the head bit */
			pCMD->cmdlen=FDC_CMD_RECALIBRATE_LEN;
			pCMD->resultlen=0;  /* no result */
			break;
			break;
		case FDC_CMD_SENSE_INT:
			pCMD->cmdlen=FDC_CMD_SENSE_INT_LEN;
			pCMD->resultlen=2;
			break;
	}
	for(i=0;i<pCMD->cmdlen;i++) {
		/* PRINTF("write cmd%d = 0x%02X\n",i,pCMD->cmd[i]); */
		if (write_fdc_byte(pCMD->cmd[i]) == false) {
			PRINTF("Error: timeout while issue cmd%d\n",i);
			return false;
		}
	}
	timeout=FDC_TIME_OUT;
	for(i=0;i<pCMD->resultlen;i++) {
		while((read_fdc_reg(FDC_MSR)&0xC0)!=0xC0) {
			timeout--;
			if(timeout==0) {
				PRINTF(" timeout while reading result%d MSR=0x%02X\n",i,read_fdc_reg(FDC_MSR));
				return false;
			}
		}
		pCMD->result[i]=(unsigned char)read_fdc_byte();
	}
	return true;
}

/* selects the drive assigned in the cmd structur and
   switches on the Motor */
void select_fdc_drive(FDC_COMMAND_STRUCT *pCMD)
{
	unsigned char val;

	val=(1<<(4+pCMD->drive))|pCMD->drive|0xC; /* set reset, dma gate and motor bits */
	if((read_fdc_reg(FDC_DOR)&val)!=val) {
		write_fdc_reg(FDC_DOR,val);
		for(val=0;val<255;val++)
			udelay(500); /* wait some time to start motor */
	}
}

/* switches off the Motor of the specified drive */
void stop_fdc_drive(FDC_COMMAND_STRUCT *pCMD)
{
	unsigned char val;

	val=(1<<(4+pCMD->drive))|pCMD->drive; /* sets motor bits */
	write_fdc_reg(FDC_DOR,(read_fdc_reg(FDC_DOR)&~val));
}

/* issues a recalibrate command, waits for interrupt and
 * issues a sense_interrupt */
int fdc_recalibrate(FDC_COMMAND_STRUCT *pCMD,FD_GEO_STRUCT *pFG)
{
	pCMD->cmd[COMMAND]=FDC_CMD_RECALIBRATE;
	if (fdc_issue_cmd(pCMD, pFG) == false)
		return false;
	while (wait_for_fdc_int() != true);

	pCMD->cmd[COMMAND]=FDC_CMD_SENSE_INT;
	return(fdc_issue_cmd(pCMD,pFG));
}

/* issues a recalibrate command, waits for interrupt and
 * issues a sense_interrupt */
int fdc_seek(FDC_COMMAND_STRUCT *pCMD,FD_GEO_STRUCT *pFG)
{
	pCMD->cmd[COMMAND]=FDC_CMD_SEEK;
	if (fdc_issue_cmd(pCMD, pFG) == false)
		return false;
	while (wait_for_fdc_int() != true);

	pCMD->cmd[COMMAND]=FDC_CMD_SENSE_INT;
	return(fdc_issue_cmd(pCMD,pFG));
}

/* terminates current command, by not servicing the FIFO
 * waits for interrupt and fills in the result bytes */
int fdc_terminate(FDC_COMMAND_STRUCT *pCMD)
{
	int i;
	for(i=0;i<100;i++)
		udelay(500); /* wait 500usec for fifo overrun */
	while((read_fdc_reg(FDC_SRA)&0x80)==0x00); /* wait as long as no int has occurred */
	for(i=0;i<7;i++) {
		pCMD->result[i]=(unsigned char)read_fdc_byte();
	}
	return true;
}

/* reads data from FDC, seek commands are issued automatic */
int fdc_read_data(unsigned char *buffer, unsigned long blocks,FDC_COMMAND_STRUCT *pCMD, FD_GEO_STRUCT *pFG)
{
  /* first seek to start address */
	unsigned long len,readblk,i,timeout,ii,offset;
	unsigned char c,retriesrw,retriescal;
	unsigned char *bufferw; /* working buffer */
	int sect_size;
	int flags;

	flags=disable_interrupts(); /* switch off all Interrupts */
	select_fdc_drive(pCMD); /* switch on drive */
	sect_size=0x080<<pFG->sect_code;
	retriesrw=0;
	retriescal=0;
	offset=0;
	if (fdc_seek(pCMD, pFG) == false) {
		stop_fdc_drive(pCMD);
		if (flags)
			enable_interrupts();
		return false;
	}
	if((pCMD->result[STATUS_0]&0x20)!=0x20) {
		printf("Seek error Status: %02X\n",pCMD->result[STATUS_0]);
		stop_fdc_drive(pCMD);
		if (flags)
			enable_interrupts();
		return false;
	}
	/* now determine the next seek point */
	/*	lastblk=pCMD->blnr + blocks; */
	/*	readblk=(pFG->head*pFG->sect)-(pCMD->blnr%(pFG->head*pFG->sect)); */
	readblk=pFG->sect-(pCMD->blnr%pFG->sect);
	PRINTF("1st nr of block possible read %ld start %ld\n",readblk,pCMD->blnr);
	if(readblk>blocks) /* is end within 1st track */
		readblk=blocks; /* yes, correct it */
	PRINTF("we read %ld blocks start %ld\n",readblk,pCMD->blnr);
	bufferw = &buffer[0]; /* setup working buffer */
	do {
retryrw:
		len=sect_size * readblk;
		pCMD->cmd[COMMAND]=FDC_CMD_READ;
		if (fdc_issue_cmd(pCMD, pFG) == false) {
			stop_fdc_drive(pCMD);
			if (flags)
				enable_interrupts();
			return false;
		}
		for (i=0;i<len;i++) {
			timeout=FDC_TIME_OUT;
			do {
				c=read_fdc_reg(FDC_MSR);
				if((c&0xC0)==0xC0) {
					bufferw[i]=read_fdc_reg(FDC_FIFO);
					break;
				}
				if((c&0xC0)==0x80) { /* output */
					PRINTF("Transfer error transferred: at %ld, MSR=%02X\n",i,c);
					if(i>6) {
						for(ii=0;ii<7;ii++) {
							pCMD->result[ii]=bufferw[(i-7+ii)];
						} /* for */
					}
					if(retriesrw++>FDC_RW_RETRIES) {
						if (retriescal++>FDC_CAL_RETRIES) {
							stop_fdc_drive(pCMD);
							if (flags)
								enable_interrupts();
							return false;
						}
						else {
							PRINTF(" trying to recalibrate Try %d\n",retriescal);
							if (fdc_recalibrate(pCMD, pFG) == false) {
								stop_fdc_drive(pCMD);
								if (flags)
									enable_interrupts();
								return false;
							}
							retriesrw=0;
							goto retrycal;
						} /* else >FDC_CAL_RETRIES */
					}
					else {
						PRINTF("Read retry %d\n",retriesrw);
						goto retryrw;
					} /* else >FDC_RW_RETRIES */
				}/* if output */
				timeout--;
			} while (true);
		} /* for len */
		/* the last sector of a track or all data has been read,
		 * we need to get the results */
		fdc_terminate(pCMD);
		offset+=(sect_size*readblk); /* set up buffer pointer */
		bufferw = &buffer[offset];
		pCMD->blnr+=readblk; /* update current block nr */
		blocks-=readblk; /* update blocks */
		if(blocks==0)
			break; /* we are finish */
		/* setup new read blocks */
		/*	readblk=pFG->head*pFG->sect; */
		readblk=pFG->sect;
		if(readblk>blocks)
			readblk=blocks;
retrycal:
		/* a seek is necessary */
		if (fdc_seek(pCMD, pFG) == false) {
			stop_fdc_drive(pCMD);
			if (flags)
				enable_interrupts();
			return false;
		}
		if((pCMD->result[STATUS_0]&0x20)!=0x20) {
			PRINTF("Seek error Status: %02X\n",pCMD->result[STATUS_0]);
			stop_fdc_drive(pCMD);
			return false;
		}
	} while (true); /* start over */
	stop_fdc_drive(pCMD); /* switch off drive */
	if (flags)
		enable_interrupts();
	return true;
}

/* Scan all drives and check if drive is present and disk is inserted */
int fdc_check_drive(FDC_COMMAND_STRUCT *pCMD, FD_GEO_STRUCT *pFG)
{
	int i,drives,state;
  /* OK procedure of data book is satisfied.
	 * trying to get some information over the drives */
	state=0; /* no drives, no disks */
	for(drives=0;drives<4;drives++) {
		pCMD->drive=drives;
		select_fdc_drive(pCMD);
		pCMD->blnr=0; /* set to the 1st block */
		if (fdc_recalibrate(pCMD, pFG) == false)
			continue;
		if((pCMD->result[STATUS_0]&0x10)==0x10)
			continue;
		/* ok drive connected check for disk */
		state|=(1<<drives);
		pCMD->blnr=pFG->size; /* set to the last block */
		if (fdc_seek(pCMD, pFG) == false)
			continue;
		pCMD->blnr=0; /* set to the 1st block */
		if (fdc_recalibrate(pCMD, pFG) == false)
			continue;
		pCMD->cmd[COMMAND]=FDC_CMD_READ_ID;
		if (fdc_issue_cmd(pCMD, pFG) == false)
			continue;
		state|=(0x10<<drives);
	}
	stop_fdc_drive(pCMD);
	for(i=0;i<4;i++) {
		PRINTF("Floppy Drive %d %sconnected %sDisk inserted %s\n",i,
			((state&(1<<i))==(1<<i)) ? "":"not ",
			((state&(0x10<<i))==(0x10<<i)) ? "":"no ",
			((state&(0x10<<i))==(0x10<<i)) ? pFG->name : "");
	}
	pCMD->flags=state;
	return true;
}


/**************************************************************************
* int fdc_setup
* setup the fdc according the datasheet
* assuming in PS2 Mode
*/
int fdc_setup(int drive, FDC_COMMAND_STRUCT *pCMD, FD_GEO_STRUCT *pFG)
{
	int i;

#ifdef CONFIG_SYS_FDC_HW_INIT
	fdc_hw_init ();
#endif
	/* first, we reset the FDC via the DOR */
	write_fdc_reg(FDC_DOR,0x00);
	for(i=0; i<255; i++) /* then we wait some time */
		udelay(500);
	/* then, we clear the reset in the DOR */
	pCMD->drive=drive;
	select_fdc_drive(pCMD);
	/* initialize the CCR */
	write_fdc_reg(FDC_CCR,pFG->rate);
	/* then initialize the DSR */
	write_fdc_reg(FDC_DSR,pFG->rate);
	if (wait_for_fdc_int() == false) {
			PRINTF("Time Out after writing CCR\n");
			return false;
	}
	/* now issue sense Interrupt and status command
	 * assuming only one drive present (drive 0) */
	pCMD->dma=0; /* we don't use any dma at all */
	for(i=0;i<4;i++) {
		/* issue sense interrupt for all 4 possible drives */
		pCMD->cmd[COMMAND]=FDC_CMD_SENSE_INT;
		if (fdc_issue_cmd(pCMD, pFG) == false) {
			PRINTF("Sense Interrupt for drive %d failed\n",i);
		}
	}
	/* issue the configure command */
	pCMD->drive=drive;
	select_fdc_drive(pCMD);
	pCMD->cmd[COMMAND]=FDC_CMD_CONFIGURE;
	if (fdc_issue_cmd(pCMD, pFG) == false) {
		PRINTF(" configure timeout\n");
		stop_fdc_drive(pCMD);
		return false;
	}
	/* issue specify command */
	pCMD->cmd[COMMAND]=FDC_CMD_SPECIFY;
	if (fdc_issue_cmd(pCMD, pFG) == false) {
		PRINTF(" specify timeout\n");
		stop_fdc_drive(pCMD);
		return false;

	}
	/* then, we clear the reset in the DOR */
	/* fdc_check_drive(pCMD,pFG);	*/
	/*	write_fdc_reg(FDC_DOR,0x04); */

	return true;
}

/****************************************************************************
 * main routine do_fdcboot
 */
int do_fdcboot (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	FD_GEO_STRUCT *pFG = (FD_GEO_STRUCT *)floppy_type;
	FDC_COMMAND_STRUCT *pCMD = &cmd;
	unsigned long addr,imsize;
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	image_header_t *hdr;  /* used for fdc boot */
#endif
	unsigned char boot_drive;
	int i,nrofblk;
#if defined(CONFIG_FIT)
	const void *fit_hdr = NULL;
#endif

	switch (argc) {
	case 1:
		addr = CONFIG_SYS_LOAD_ADDR;
		boot_drive=CONFIG_SYS_FDC_DRIVE_NUMBER;
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_drive=CONFIG_SYS_FDC_DRIVE_NUMBER;
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_drive=simple_strtoul(argv[2], NULL, 10);
		break;
	default:
		return CMD_RET_USAGE;
	}
	/* setup FDC and scan for drives  */
	if (fdc_setup(boot_drive, pCMD, pFG) == false) {
		printf("\n** Error in setup FDC **\n");
		return 1;
	}
	if (fdc_check_drive(pCMD, pFG) == false) {
		printf("\n** Error in check_drives **\n");
		return 1;
	}
	if((pCMD->flags&(1<<boot_drive))==0) {
		/* drive not available */
		printf("\n** Drive %d not availabe **\n",boot_drive);
		return 1;
	}
	if((pCMD->flags&(0x10<<boot_drive))==0) {
		/* no disk inserted */
		printf("\n** No disk inserted in drive %d **\n",boot_drive);
		return 1;
	}
	/* ok, we have a valid source */
	pCMD->drive=boot_drive;
	/* read first block */
	pCMD->blnr=0;
	if (fdc_read_data((unsigned char *)addr, 1, pCMD, pFG) == false) {
		printf("\nRead error:");
		for(i=0;i<7;i++)
			printf("result%d: 0x%02X\n",i,pCMD->result[i]);
		return 1;
	}

	switch (genimg_get_format ((void *)addr)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *)addr;
		image_print_contents (hdr);

		imsize = image_get_image_size (hdr);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *)addr;
		puts ("Fit image detected...\n");

		imsize = fit_get_size (fit_hdr);
		break;
#endif
	default:
		puts ("** Unknown image type\n");
		return 1;
	}

	nrofblk=imsize/512;
	if((imsize%512)>0)
		nrofblk++;
	printf("Loading %ld Bytes (%d blocks) at 0x%08lx..\n",imsize,nrofblk,addr);
	pCMD->blnr=0;
	if (fdc_read_data((unsigned char *)addr, nrofblk, pCMD, pFG) == false) {
		/* read image block */
		printf("\nRead error:");
		for(i=0;i<7;i++)
			printf("result%d: 0x%02X\n",i,pCMD->result[i]);
		return 1;
	}
	printf("OK %ld Bytes loaded.\n",imsize);

	flush_cache (addr, imsize);

#if defined(CONFIG_FIT)
	/* This cannot be done earlier, we need complete FIT image in RAM first */
	if (genimg_get_format ((void *)addr) == IMAGE_FORMAT_FIT) {
		if (!fit_check_format (fit_hdr)) {
			puts ("** Bad FIT image format\n");
			return 1;
		}
		fit_print_contents (fit_hdr);
	}
#endif

	/* Loading ok, update default load address */
	load_addr = addr;

	return bootm_maybe_autostart(cmdtp, argv[0]);
}

U_BOOT_CMD(
	fdcboot,	3,	1,	do_fdcboot,
	"boot from floppy device",
	"loadAddr drive"
);
