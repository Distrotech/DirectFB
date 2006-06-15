EXTRA_DIST = directfb.png

if ARIBDEV_CORE
docs_headers = \
	$(top_srcdir)/include/directfb.h \
    $(top_srcdir)/include/directfb_arib.h \
	$(top_srcdir)/include/directfb_keyboard.h
else
docs_headers = \
	$(top_srcdir)/include/directfb.h \
	$(top_srcdir)/include/directfb_keyboard.h
endif

all-local: stamp-docs

stamp-docs: $(top_srcdir)/tools/gendoc.pl $(docs_headers) Makefile.am
	rootme=`pwd`; \
	$(PERL) $(top_srcdir)/tools/gendoc.pl $(docs_headers) \
	&& echo timestamp > $$rootme/stamp-docs

distclean-local:
	rm -f *.html stamp-docs
