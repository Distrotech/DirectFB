
LDFLAGS +=--no-undefined

INCLUDES =  -I$(top_srcdir)/runtime -I$(top_srcdir)

bin_PROGRAMS = gpplayer

gpplayer_LDADD = \
	$(top_builddir)/runtime/libgpruntime.la 

gpplayerincludedir = $(includedir)/gp

gpplayerinclude_HEADERS = 

gpplayer_SOURCES =	\
	main.c
