/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  ELF Program Loader File: cfe_ldr_elf.c
    *  
    *  This program parses ELF executables and loads them into memory.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_console.h"
#include "cfe_error.h"
#include "cfe_devfuncs.h"
#include "cfe_timer.h"
#include "cfe_mem.h"

#include "cfe.h"
#include "cfe_loader.h"
#include "cfe_fileops.h"
#include "elf.h"

#include "cfe_boot.h"

static int is_64bit = 0;

#define ELF_EHDR_SIZE (is_64bit ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr))
#define ELF_PHDR_SIZE (is_64bit ? sizeof(Elf64_Phdr) : sizeof(Elf32_Phdr))
#define ELF_SHDR_SIZE (is_64bit ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr))

#define ELF_GET_MACHINE(ep)     (Elf_Half)(is_64bit ? ep->ehdr64.e_machine : ep->ehdr32.e_machine)
#define ELF_GET_ENTRY(ep)       (Elf64_Addr)(is_64bit ? ep->ehdr64.e_entry : ep->ehdr32.e_entry)
#define ELF_GET_PHOFF(ep)       (Elf64_Off)(is_64bit ? ep->ehdr64.e_phoff : ep->ehdr32.e_phoff)
#define ELF_GET_SHOFF(ep)       (Elf64_Off)(is_64bit ? ep->ehdr64.e_shoff : ep->ehdr32.e_shoff)
#define ELF_GET_PHENTSIZE(ep)   (Elf_Half)(is_64bit ? ep->ehdr64.e_phentsize : ep->ehdr32.e_phentsize)
#define ELF_GET_PHNUM(ep)       (Elf_Half)(is_64bit ? ep->ehdr64.e_phnum : ep->ehdr32.e_phnum)
#define ELF_GET_SHNUM(ep)       (Elf_Half)(is_64bit ? ep->ehdr64.e_shnum : ep->ehdr32.e_shnum)

#define PHDR_GET_TYPE(ph)       (Elf_Word)(is_64bit ? ((Elf64_Phdr*)ph)->p_type : ((Elf32_Phdr*)ph)->p_type)
#define PHDR_GET_FLAGS(ph)      (Elf_Word)(is_64bit ? ((Elf64_Phdr*)ph)->p_flags : ((Elf32_Phdr*)ph)->p_flags)
#define PHDR_GET_OFF(ph)        (Elf64_Off)(is_64bit ? ((Elf64_Phdr*)ph)->p_offset : ((Elf32_Phdr*)ph)->p_offset)
#define PHDR_GET_VADDR(ph)      (Elf64_Addr)(is_64bit ? ((Elf64_Phdr*)ph)->p_vaddr : ((Elf32_Phdr*)ph)->p_vaddr)
#define PHDR_GET_PADDR(ph)      (Elf64_Addr)(is_64bit ? ((Elf64_Phdr*)ph)->p_paddr : ((Elf32_Phdr*)ph)->p_paddr)
#define PHDR_GET_FILESZ(ph)     (Elf64_Xword)(is_64bit ? ((Elf64_Phdr*)ph)->p_filesz : ((Elf32_Phdr*)ph)->p_filesz)
#define PHDR_GET_MEMSZ(ph)      (Elf64_Xword)(is_64bit ? ((Elf64_Phdr*)ph)->p_memsz : ((Elf32_Phdr*)ph)->p_memsz)

/*  *********************************************************************
    *  Externs
    ********************************************************************* */

int bcm63xx_cfe_elfload(cfe_loadargs_t *la);

const cfe_loader_t elfloader = {
    "elf",
    bcm63xx_cfe_elfload,
    0};

/*  *********************************************************************
    *  readprogsegment(fsctx,ref,addr,size)
    *
    *  Read a program segment, generally corresponding to one
    *  section of the file.
    *
    *  Input parameters:
    *     fsctx - file I/O dispatch
    *     ref - reference data for open file handle
    *     addr - target virtual address
    *     size - size of region to read
    *
    *  Return value:
    *     Number of bytes copied or <0 if error occured
    ********************************************************************* */

static int readprogsegment(fileio_ctx_t *fsctx,void *ref,
    void *addr,int size,int flags)
{
    int res;

    if (flags & LOADFLG_NOISY) xprintf("0x%p/%d ",addr,size);

    res = fs_read(fsctx,ref,addr,size);

    if (res < 0) return CFE_ERR_IOERR;
    if (res != size) return CFE_ERR_BADELFFMT;

    return size;
}


/*  *********************************************************************
    *  readclearbss(addr,size)
    *
    *  Process a BSS section, zeroing memory corresponding to
    *  the BSS.
    *
    *  Input parameters:
    *     addr - address to zero
    *     size - length of area to zero
    *
    *  Return value:
    *     number of zeroed bytes or <0 if error occured
    ********************************************************************* */

static int readclearbss(void *addr,int size,int flags)
{

    if (flags & LOADFLG_NOISY) xprintf("0x%p/%d ",addr,size);

    if (size > 0) memset(addr,0,size);
    return size;
}


/*  *********************************************************************
    *  elfgetshdr(ops,ref,ep)
    *
    *  Get a section header from the ELF file
    *
    *  Input parameters:
    *     ops - file I/O dispatch
    *     ref - reference data for open file
    *     ep - extended header info
    *
    *  Return value:
    *     copy of section header (malloc'd) or NULL if no memory
    ********************************************************************* */
static void *elfgetshdr(fileio_ctx_t *fsctx,void *ref,Elf_Ehdr *ep)
{
    void *shtab;
    unsigned size = ELF_GET_SHNUM(ep) * ELF_SHDR_SIZE;
    Elf64_Off off = ELF_GET_SHOFF(ep);

    shtab = (void *) KMALLOC(size,0);
    if (!shtab) {
        return NULL;
    }

    if (fs_seek(fsctx,ref,off,FILE_SEEK_BEGINNING) != off ||
        fs_read(fsctx,ref,(uint8_t *)shtab,size) != size) {
        KFREE(shtab);
        return NULL;
    }

    return (shtab);
}

/*  *********************************************************************
    *  elfload_internal(ops,ref,entrypt,flags)
    *
    *  Read an ELF file (main routine)
    *
    *  Input parameters:
    *     ops - file I/O dispatch
    *     ref - open file handle
    *     entrypt - filled in with entry vector
    *      flags - generic boot flags
    *
    *  Return value:
    *     0 if ok
    *     else error code
    ********************************************************************* */

static int elfload_internal(fileio_ctx_t *fsctx,void *ref,cfe_loadargs_t *la)
{
    Elf_Ehdr *ep;
    void *phtab = 0;
    void *shtab = 0;
    uintptr_t la_address = ~((uintptr_t)0);
    Elf_Ehdr ehdr;
    Elf_Half e_machine, e_phnum, e_phentsize;
    Elf64_Off e_phoff, e_shoff;
    int phdr_size, ehdr_size;
    unsigned int nbytes;
    int i;
    int res;

    ep = &ehdr;
    if (fs_read(fsctx,ref,(uint8_t *) ep,EI_NIDENT) != EI_NIDENT) {
        return CFE_ERR_IOERR;
    }

    /* check header validity and find if it is 64 bit or 32 bit elf*/
    if (ep->ehdr32.e_ident[EI_MAG0] != ELFMAG0 ||
        ep->ehdr32.e_ident[EI_MAG1] != ELFMAG1 ||
        ep->ehdr32.e_ident[EI_MAG2] != ELFMAG2 ||
        ep->ehdr32.e_ident[EI_MAG3] != ELFMAG3) {
            return CFE_ERR_NOTELF;
    }

#ifdef __long64
    if (ep->ehdr32.e_ident[EI_CLASS] == ELFCLASS64) is_64bit = 1;
#else
    if (ep->ehdr32.e_ident[EI_CLASS] != ELFCLASS32) return CFE_ERR_NOT32BIT;
#endif

#ifdef __MIPSEB
    if (ep->ehdr32.e_ident[EI_DATA] != ELFDATA2MSB) return CFE_ERR_WRONGENDIAN;    /* big endian */
#endif
#if defined(__MIPSEL) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    if (ep->ehdr32.e_ident[EI_DATA] != ELFDATA2LSB) return CFE_ERR_WRONGENDIAN;    /* little endian */
#endif

    if (ep->ehdr32.e_ident[EI_VERSION] != EV_CURRENT) return CFE_ERR_BADELFVERS;

    /* read the rest of actual size of elf header*/
    ehdr_size = ELF_EHDR_SIZE;
    if (fs_read(fsctx,ref,((uint8_t *)ep+EI_NIDENT),ehdr_size-EI_NIDENT) != (ehdr_size-EI_NIDENT)) {
        return CFE_ERR_IOERR;
    }

    e_machine = ELF_GET_MACHINE(ep);
#if defined(CONFIG_ARM64)
    if (e_machine != EM_AARCH64 && e_machine != EM_ARM) return CFE_ERR_NOTARM;
#elif defined(CONFIG_ARM)
    if (e_machine != EM_ARM) return CFE_ERR_NOTARM;
#else
    /* MIPS */
    if (e_machine != EM_MIPS) return CFE_ERR_NOTMIPS;
#endif

    /* Is there a program header? */
    e_phoff = ELF_GET_PHOFF(ep);
    e_phnum = ELF_GET_PHNUM(ep);
    e_phentsize = ELF_GET_PHENTSIZE(ep);
    phdr_size = ELF_PHDR_SIZE;

    if (e_phoff == 0 || e_phnum == 0 ||
        e_phentsize != phdr_size) {
            return CFE_ERR_BADELFFMT;
    }

    /* Load program header */
    nbytes = e_phnum * phdr_size;
    phtab = KMALLOC(nbytes,0);
    if (!phtab) {
        return CFE_ERR_NOMEM;
    }

    if (fs_seek(fsctx,ref,e_phoff,FILE_SEEK_BEGINNING) != e_phoff ||
        fs_read(fsctx,ref,(uint8_t *)phtab,nbytes) != nbytes) {
            KFREE(phtab);
            return CFE_ERR_IOERR;
    }

    /*
     * From now on we've got no guarantee about the file order,
     * even where the section header is.  Hopefully most linkers
     * will put the section header after the program header, when
     * they know that the executable is not demand paged.  We assume
     * that the symbol and string tables always follow the program
     * segments.
     */

    /* read section table (if before first program segment) */
    e_shoff = ELF_GET_SHOFF(ep);
    if (e_shoff < PHDR_GET_OFF(phtab)) {
        shtab = elfgetshdr(fsctx,ref,ep);
    }

    /* load program segments */
    /* We cope with a badly sorted program header, as produced by 
     * older versions of the GNU linker, by loading the segments
     * in file offset order, not in program header order. */

    while (1) {
        Elf64_Off lowest_offset = (Elf64_Off)(-1);
        void *cur_ph = phtab;
        void *ph = NULL;
        Elf64_Off p_offset;
        Elf64_Addr p_paddr;
        Elf64_Xword p_filesz, p_memsz;
        Elf_Word p_type;

        /* find nearest loadable segment */
        for (i = 0; i < e_phnum; i++) {
            p_type = PHDR_GET_TYPE(cur_ph);
            p_offset = PHDR_GET_OFF(cur_ph);
            if ((p_type == PT_LOAD) && (p_offset < lowest_offset)) {
                ph = cur_ph;
                lowest_offset = p_offset;
            }
            cur_ph = (unsigned char*)cur_ph + ELF_PHDR_SIZE;
        }
        if (!ph) {
            break;        /* none found, finished */
        }

        /* load the segment */
        p_offset =  PHDR_GET_OFF(ph);
        p_filesz =  PHDR_GET_FILESZ(ph);
        p_memsz =  PHDR_GET_MEMSZ(ph);
        if (p_filesz) {
            if (fs_seek(fsctx,ref,p_offset,FILE_SEEK_BEGINNING) != p_offset) {
                if (shtab) KFREE(shtab);
                 KFREE(phtab);
                 return CFE_ERR_BADELFFMT;
            }
            p_paddr =  PHDR_GET_PADDR(ph);
            res = readprogsegment(fsctx,ref, (void *)(intptr_t)(signed)p_paddr,
                p_filesz,la->la_flags);
            if (res != p_filesz) {
                if (shtab) KFREE(shtab);
                    KFREE(phtab);
                    return res;
            }
            if (la_address > p_paddr) {
                la_address = (uintptr_t)p_paddr;
            }
        }

        if (p_filesz < p_memsz) {
            res = readclearbss((void *)(intptr_t)(signed)p_paddr + p_filesz, 
                   p_memsz - p_filesz,la->la_flags);
            if (res < 0) {
                if (shtab) KFREE(shtab);
                KFREE(phtab);
                return res;
            }
        }

        ((Elf32_Phdr*)ph)->p_type = PT_NULL; /* remove from consideration */
    }

    KFREE(phtab);

    la->la_entrypt = (intptr_t)ELF_GET_ENTRY(ep);        /* return entry point */
    la->la_address = la_address;        /* return lowest code loading address before entry point */
    return 0;
}



/*  *********************************************************************
    *  cfe_elfload(ops,file,flags)
    *
    *  Read an ELF file (main entry point)
    *
    *  Input parameters:
    *      ops - fileio dispatch
    *      file - name of file to read
    *      ept - where to put entry point
    *      flags - load flags
    *
    *  Return value:
    *      0 if ok, else error code
    ********************************************************************* */

int bcm63xx_cfe_elfload(cfe_loadargs_t *la)
{
    fileio_ctx_t *fsctx;
    void *ref;
    int res;

    /*
     * Look up the file system type and get a context
     */


    res = fs_init(la->la_filesys,&fsctx,la->la_device);
    if (res != 0) {
        return res;
    }

    /*
     * Turn on compression if we're doing that.
     */

    if (la->la_flags & LOADFLG_COMPRESSED) {
        res = fs_hook(fsctx,"z");
        if (res != 0) {
            return res;
        }
    }

    /*
     * Open the remote file
     */

    res = fs_open(fsctx,&ref,la->la_filename,FILE_MODE_READ);
    if (res != 0) {
        fs_uninit(fsctx);
            return CFE_ERR_FILENOTFOUND;
    }

    /*
     * Load the image.
     */

    la->la_entrypt = 0;
    res = elfload_internal(fsctx,ref,la);
    if( is_64bit )
       la->la_flags |= LOADFLG_64BITIMG;
    else
       la->la_flags &= ~LOADFLG_64BITIMG;

    /*
     * All done, release resources
     */

    fs_close(fsctx,ref);
    fs_uninit(fsctx);

    return res;
}
