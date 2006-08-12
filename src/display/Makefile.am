## Makefile.am for DirectFB/src/display

INCLUDES = \
	-I$(top_builddir)/lib		\
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src

AM_CPPFLAGS = \
     -DDATADIR=\"${RUNTIME_SYSROOT}@DATADIR@\"         \
     -DSOPATH=\"@SOPATH@\"         \
     -DMODULEDIR=\"${RUNTIME_SYSROOT}@MODULEDIR@\"     \
     "-DBUILDTIME=\"`date -u "+%Y-%m-%d %H:%M"`\""


internalincludedir = $(INTERNALINCLUDEDIR)/display

internalinclude_HEADERS = \
	idirectfbpalette.h		\
	idirectfbsurface.h		\
	idirectfbdisplaylayer.h		\
	idirectfbscreen.h


noinst_LTLIBRARIES = libdirectfb_display.la

libdirectfb_display_la_SOURCES =	\
	idirectfbpalette.c		\
	idirectfbsurface.c		\
	idirectfbdisplaylayer.c		\
	idirectfbscreen.c
