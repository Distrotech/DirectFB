## Makefile.am for DirectFB/src/input

INCLUDES = \
	-I$(top_builddir)/lib		\
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/input


noinst_LTLIBRARIES = libdirectfb_input.la


if ARIBDEV_CORE

internalinclude_HEADERS = \
	idirectfbinputdevice.h	\
	idirectfbinputbuffer.h	\
	idirectfbinputdevice_arib.h


libdirectfb_input_la_SOURCES = \
	idirectfbinputdevice.c	\
	idirectfbinputbuffer.c	\
	idirectfbinputdevice_arib.c

else

internalinclude_HEADERS = \
	idirectfbinputdevice.h	\
	idirectfbinputbuffer.h


libdirectfb_input_la_SOURCES = \
	idirectfbinputdevice.c	\
	idirectfbinputbuffer.c

endif
