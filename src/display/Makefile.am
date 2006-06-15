## Makefile.am for DirectFB/src/display

INCLUDES = \
	-I$(top_builddir)/lib		\
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/display


noinst_LTLIBRARIES = libdirectfb_display.la


if ARIBDEV_CORE

internalinclude_HEADERS = \
	idirectfbpalette.h		\
	idirectfbsurface.h		\
	idirectfbsurface_layer.h 	\
	idirectfbsurface_window.h 	\
	idirectfbdisplaylayer.h		\
	idirectfbscreen.h		\
	idirectfbsurface_window_arib.h 	\
	idirectfbdisplaylayer_arib.h


libdirectfb_display_la_SOURCES =	\
	idirectfbpalette.c		\
	idirectfbsurface.c		\
	idirectfbsurface_layer.c 	\
	idirectfbsurface_window.c 	\
	idirectfbdisplaylayer.c		\
	idirectfbscreen.c		\
	idirectfbsurface_window_arib.c 	\
	idirectfbdisplaylayer_arib.c

else

internalinclude_HEADERS = \
	idirectfbpalette.h		\
	idirectfbsurface.h		\
	idirectfbsurface_layer.h 	\
	idirectfbsurface_window.h 	\
	idirectfbdisplaylayer.h		\
	idirectfbscreen.h


libdirectfb_display_la_SOURCES =	\
	idirectfbpalette.c		\
	idirectfbsurface.c		\
	idirectfbsurface_layer.c 	\
	idirectfbsurface_window.c 	\
	idirectfbdisplaylayer.c		\
	idirectfbscreen.c

endif
