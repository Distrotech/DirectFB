DIST_SUBDIRS = runtime
SUBDIRS = runtime plugins player
LDFLAGS +=--no-undefined
DISTCLEANFILES = config.cache

MAINTAINERCLEANFILES = \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/autoscan.log \
	$(srcdir)/compile \
	$(srcdir)/config.guess \
	$(srcdir)/config.h.in \
	$(srcdir)/config.sub \
	$(srcdir)/configure.scan \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/mkinstalldirs \
	$(srcdir)/gp.pc \
	$(srcdir)/configure \
	`find "$(srcdir)" -type f -name Makefile.in -print`\
	`find "$(srcdir)" -type f -name Makefile -print`

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gp.pc



