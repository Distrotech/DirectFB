## Makefile.am for DirectFB/src/media

INCLUDES = \
	-I$(top_builddir)/lib		\
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/media


noinst_LTLIBRARIES = libdirectfb_media.la


if ARIBDEV_CORE

internalinclude_HEADERS = \
	idirectfbdatabuffer.h		\
	idirectfbfont.h			\
	idirectfbaribfont.h		\
	idirectfbaribdrcs.h		\
	idirectfbimageprovider.h	\
	idirectfbvideoprovider.h

libdirectfb_media_la_SOURCES = \
	idirectfbdatabuffer.c		\
	idirectfbdatabuffer_file.c	\
	idirectfbdatabuffer_memory.c	\
	idirectfbdatabuffer_streamed.c	\
	idirectfbfont.c			\
	idirectfbaribfont.c		\
	idirectfbaribdrcs.c		\
	idirectfbimageprovider.c		\
	idirectfbvideoprovider.c

else

internalinclude_HEADERS = \
	idirectfbdatabuffer.h		\
	idirectfbfont.h			\
	idirectfbimageprovider.h	\
	idirectfbvideoprovider.h

libdirectfb_media_la_SOURCES = \
	idirectfbdatabuffer.c		\
	idirectfbdatabuffer_file.c	\
	idirectfbdatabuffer_memory.c	\
	idirectfbdatabuffer_streamed.c	\
	idirectfbfont.c			\
	idirectfbimageprovider.c		\
	idirectfbvideoprovider.c

endif