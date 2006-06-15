## Makefile.am for DirectFB/src/windows

INCLUDES = \
	-I$(top_builddir)/lib		\
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/windows


noinst_LTLIBRARIES = libdirectfb_windows.la


if ARIBDEV_CORE
internalinclude_HEADERS = \
	idirectfbwindow.h	\
	idirectfbwindow_arib.h

libdirectfb_windows_la_SOURCES = \
	idirectfbwindow.c	\
	idirectfbwindow_arib.c

else

internalinclude_HEADERS = \
	idirectfbwindow.h

libdirectfb_windows_la_SOURCES = \
	idirectfbwindow.c
endif
