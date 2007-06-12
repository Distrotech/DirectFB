
SUBDIRS = DOM Window 

CLEANFILES = libgpdom.so libgpwindowdom.so libgpwindow_x11.so 

libgpdom.so:
	ln -s  DOM/.libs/libgpdom.so libgpdom.so

libgpwindowdom.so:
	ln -s  Window/.libs/libgpwindowdom.so libgpwindowdom.so

libgpwindow_x11.so:
	ln -s  Window/X11/.libs/libgpwindow_x11.so libgpwindow_x11.so


all-local: libgpdom.so libgpwindowdom.so libgpwindow_x11.so

