

lib_LTLIBRARIES = libgpruntime.la

INCLUDES = -I$(srcdir)
LIBS += -ldl

gpruntimeincludedir = $(includedir)/gp


gpruntimeinclude_HEADERS =  \
	GPAtomicString.h \
	GPHashtable.h \
	GPLibrary.h \
	GPList.h \
	GPPlugin.h \
	GPRuntime.h \
	GPTree.h \
	GPType.h

libgpruntime_la_SOURCES =	\
	obstack.c \
	GPList.c \
	GPTree.c \
	GPRuntime.c

