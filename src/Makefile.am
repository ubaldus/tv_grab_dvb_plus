bin_PROGRAMS = tv_grab_dvb_plus
tv_grab_dvb_plus_SOURCES = \
	device.cpp \
	dvbdevice.cpp \
	filter.cpp \
	freesatepg.h \
	freesathuffman.h \
	freesattables.h \
	loadepg.cpp \
	main.cpp \
	menuitems.cpp \
	misc.cpp \
	osdbase.cpp \
	thread.cpp \
	tools.cpp \
	vdrcompat.cpp \
	\
	chanid.c \
	crc32.c \
	dvbepg.c \
	dvbinfotables.c \
	dvblangidents.c \
	dvbtexticonv.c \
	freesathuffman.c \
	freesattables.c \
	lookup.c \
	\
	chanid.h \
	constants.h \
	crc32.h \
	dvb_epg.h \
	formats.h \
	i18n.h \
	loadepg.h \
	lookup.h \
	sitables.h
tv_grab_dvb_plus_LDADD = ../libsi/libsi.a

AM_CPPFLAGS = -DSTANDALONE -I@top_srcdir@/include
