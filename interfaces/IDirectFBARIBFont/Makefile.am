## Makefile.am for DirectFB/interfaces/IDirectFBARIBFont


idirectfbaribfontdir = $(MODULEDIR)/interfaces/IDirectFBARIBFont

if SKEL_PROVIDER
SKEL_PROVIDER_LTLIB = libidirectfbaribfont_skel.la
else
SKEL_PROVIDER_LTLIB =
endif

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src 	\
	-DDATADIR=\"@DATADIR@\"

AM_CFLAGS = $(FONTAVENUE_CFLAGS)

idirectfbaribfont_LTLIBRARIES = \
	$(SKEL_PROVIDER_LTLIB)


if BUILD_STATIC
idirectfbaribfont_DATA = $(idirectfbaribfont_LTLIBRARIES:.la=.o)
endif


if SKEL_PROVIDER
libidirectfbaribfont_skel_la_SOURCES = idirectfbaribfont_skel.c
libidirectfbaribfont_skel_la_LIBADD = $(SKEL_LIBS)
libidirectfbaribfont_skel_la_LDFLAGS = -avoid-version -module
endif

include $(top_srcdir)/rules/libobject.make
