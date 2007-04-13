## Makefile.am for DirectFB/src/drawable

drawabledir = $(MODULEDIR)/drawable

INCLUDES = \
	-I$(top_builddir)/lib		\
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src

AM_CPPFLAGS = \
    -DDATADIR=\"${RUNTIME_SYSROOT}@DATADIR@\"       \
    -DSOPATH=\"@SOPATH@\"       \
    -DMODULEDIR=\"${RUNTIME_SYSROOT}@MODULEDIR@\"   \
    "-DBUILDTIME=\"`date -u "+%Y-%m-%d %H:%M"`\""

internalincludedir = $(INTERNALINCLUDEDIR)/drawable

internalinclude_HEADERS = \
	drawable.h	\	
	drawable_manager.h \		
	drawops.h \
	drawable_types.h 


noinst_LTLIBRARIES = libdirectfb_drawable.la

libdirectfb_drawable_la_SOURCES =	\
	drawable_manager.c \
	drawable.c \
	drawops.c

