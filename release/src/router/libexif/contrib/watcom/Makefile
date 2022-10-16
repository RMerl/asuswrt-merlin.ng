# This makefile is made for OpenWatcom/NT and runs on my system.
# It should not be too hard to make it run on other systems, too.
#
# FIRST, move this makefile and _stdint.h to the libexif main
# directory (the place where you find README and COPYING etc.)
#
# WARNING: I used another makefile as template, so some things might be
# thrown away. Don't hesitate to improve this makefile!
#
# WARNING: This makefile has been made mainly for myself, I didn't
# waste time to make it look "nice".
#
# LICENSE: no restrictions at all, but USE ON YOUR OWN RISK ONLY.
#
# USAGE: Simply type "wmake" or "wmake -f <makefilename>" when
#        being in the libexif main directory. This makefile should
#        be in that directory, too, of course.
#
#   Angela Wrobel <http://www.wrobelnet.de/>


# Uncomment line for desired system
#SYSTEM=DOS
#SYSTEM=OS2
SYSTEM=NT

# The name of your C compiler:
CC= wcl386

# We're using similar constants like wxWidgets
!ifeq FINAL 1
OPTFLAGS= -5r -zp8 -otexan
!else
OPTFLAGS= -5r -od -d2
!endif

LIBEXIFDIR=libexif
TESTEXIFDIR=test

IFLAGS= -i=.
WINVERFLAGS=
EXTRACPPFLAGS=-dEXIF_DONT_CHANGE_MAKER_NOTE
DEBUGCFLAGS=
EXTRACFLAGS=
OUTPUTDIR=$(LIBEXIFDIR)
CPPFLAGS = /dWIN32 /bm /fo=$(OUTPUTDIR)\ /fr -zq $(IFLAGS) $(OPTFLAGS) $(WINVERFLAGS) $(EXTRACPPFLAGS)

# zm and zv as well as the linker options below are used to make the resulting
# .exe smaller
CFLAGS = $(CPPFLAGS) $(DEBUGCFLAGS) $(EXTRACFLAGS) /zm

# Link-time cc options:
!ifeq SYSTEM DOS
LDFLAGS= -zq -l=dos4g
!else ifeq SYSTEM OS2
LDFLAGS= -zq -l=os2v2
!else ifeq SYSTEM NT
LDFLAGS= -zq -l=nt
!endif

# End of configurable options.



LIBOBJECTS = &
	$(LIBEXIFDIR)\exif-byte-order.obj $(LIBEXIFDIR)\exif-content.obj &
	$(LIBEXIFDIR)\exif-data.obj $(LIBEXIFDIR)\exif-entry.obj &
	$(LIBEXIFDIR)\exif-format.obj $(LIBEXIFDIR)\exif-ifd.obj &
	$(LIBEXIFDIR)\exif-loader.obj $(LIBEXIFDIR)\exif-log.obj &
	$(LIBEXIFDIR)\exif-mem.obj &
	$(LIBEXIFDIR)\exif-mnote-data.obj $(LIBEXIFDIR)\exif-tag.obj &
        $(LIBEXIFDIR)\exif-utils.obj &
	$(LIBEXIFDIR)\exif-mnote-data-olympus.obj &
	$(LIBEXIFDIR)\mnote-olympus-entry.obj &
	$(LIBEXIFDIR)\mnote-olympus-tag.obj &
	$(LIBEXIFDIR)\exif-mnote-data-pentax.obj &
	$(LIBEXIFDIR)\mnote-pentax-entry.obj &
	$(LIBEXIFDIR)\mnote-pentax-tag.obj &
	$(LIBEXIFDIR)\exif-mnote-data-canon.obj &
	$(LIBEXIFDIR)\mnote-canon-entry.obj &
	$(LIBEXIFDIR)\mnote-canon-tag.obj &
	$(LIBEXIFDIR)\exif-mnote-data-fuji.obj &
	$(LIBEXIFDIR)\mnote-fuji-entry.obj &
	$(LIBEXIFDIR)\mnote-fuji-tag.obj 


#CFLAGS = /dWIN32 /bm /fr -zq -i=. -5r -od -d2 /d2 /zm /fo=$(LIBEXIFDIR)\

all : libexif.lib test-mem.exe test-mnote.exe test-value.exe
#	test-tree.exe 

libexif.lib: $(LIBOBJECTS)
	- del libexif.lib
	* wlib -n libexif.lib $(LIBOBJECTS)

$(LIBEXIFDIR)\exif-byte-order.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-byte-order.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-content.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-content.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-data.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-data.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-entry.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-entry.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-format.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-format.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-ifd.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-ifd.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-loader.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-loader.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-log.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-log.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-mem.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-mem.c
	$(CC) -c $(CFLAGS) $[*

$(LIBEXIFDIR)\exif-mnote-data.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-mnote-data.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-tag.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-tag.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-utils.obj : .AUTODEPEND $(LIBEXIFDIR)\exif-utils.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-mnote-data-olympus.obj : .AUTODEPEND $(LIBEXIFDIR)\olympus\exif-mnote-data-olympus.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-olympus-entry.obj : .AUTODEPEND $(LIBEXIFDIR)\olympus\mnote-olympus-entry.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-olympus-tag.obj : .AUTODEPEND $(LIBEXIFDIR)\olympus\mnote-olympus-tag.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-mnote-data-pentax.obj : .AUTODEPEND $(LIBEXIFDIR)\pentax\exif-mnote-data-pentax.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-pentax-entry.obj : .AUTODEPEND $(LIBEXIFDIR)\pentax\mnote-pentax-entry.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-pentax-tag.obj : .AUTODEPEND $(LIBEXIFDIR)\pentax\mnote-pentax-tag.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-mnote-data-canon.obj : .AUTODEPEND $(LIBEXIFDIR)\canon\exif-mnote-data-canon.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-canon-entry.obj : .AUTODEPEND $(LIBEXIFDIR)\canon\mnote-canon-entry.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-canon-tag.obj : .AUTODEPEND $(LIBEXIFDIR)\canon\mnote-canon-tag.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\exif-mnote-data-fuji.obj : .AUTODEPEND $(LIBEXIFDIR)\fuji\exif-mnote-data-fuji.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-fuji-entry.obj : .AUTODEPEND $(LIBEXIFDIR)\fuji\mnote-fuji-entry.c
	$(CC) -c $(CFLAGS) $[*  

$(LIBEXIFDIR)\mnote-fuji-tag.obj : .AUTODEPEND $(LIBEXIFDIR)\fuji\mnote-fuji-tag.c
	$(CC) -c $(CFLAGS) $[*  


test-mem.exe : .AUTODEPEND $(TESTEXIFDIR)\test-mem.c
	$(CC) $(CFLAGS) $(LDFRLAGS) $< libexif.lib

test-mnote.exe : .AUTODEPEND $(TESTEXIFDIR)\test-mnote.c
	$(CC) $(CFLAGS) $(LDFRLAGS) $< libexif.lib

test-tree.exe : .AUTODEPEND $(TESTEXIFDIR)\test-tree.c
	$(CC) $(CFLAGS) $(LDFRLAGS) $< libexif.lib

test-value.exe : .AUTODEPEND $(TESTEXIFDIR)\test-value.c
	$(CC) $(CFLAGS) $(LDFRLAGS) $< libexif.lib


clean: .SYMBOLIC
	- del $(LIBEXIFDIR)\*.obj
	- del *.exe
	- del libexif.lib

