AM_CFLAGS = @CURSES_CFLAGS@ -DCURSESINC="@CURSESINC@"
LDADD = @CURSESLIB@

bin_PROGRAMS = alsamixer
alsamixer_SOURCES = card_select.c card_select.h \
		cli.c \
		colors.c colors.h \
		device_name.c device_name.h \
		die.c die.h \
		mainloop.c mainloop.h \
		mem.c mem.h \
		mixer_controls.c mixer_controls.h \
		mixer_display.c mixer_display.h \
		mixer_widget.c mixer_widget.h \
		proc_files.c proc_files.h \
		textbox.c textbox.h \
		utils.c utils.h \
		widget.c widget.h
man_MANS = alsamixer.1
EXTRA_DIST = alsamixer.1
alsamixer_INCLUDES = -I$(top_srcdir)/include

#LDFLAGS = -static
#CFLAGS += -g -Wall
