AM_CPPFLAGS = -Wall -I@top_srcdir@/include

bin_PROGRAMS = tv_grab_dvb_plus

tv_grab_dvb_plus_LDADD = ../libsi/libsi.a
tv_grab_dvb_plus_SOURCES = \
	device.cpp \
	filter.cpp \
	loadepg.cpp \
	main.cpp \
	misc.cpp \
	osdbase.cpp \
	thread.cpp \
	tools.cpp \
	vdrcompat.cpp \
	\
	chanid.c \
	crc32.c \
	dvbepg.c \
	dvbinfo.c \
	dvbtext.c \
	freesathuffman.c \
	freesattables.c \
	log.c \
	lookup.c \
	stats.c \
	\
	chanid.h \
	constants.h \
	crc32.h \
	dvbepg.h \
	dvbinfo.h \
	dvbtext.h \
	filter.h \
	formats.h \
	freesatepg.h \
	freesathuffman.h \
	freesattables.h \
	loadepg.h \
	log.h \
	lookup.h \
	sitables.h \
	stats.h
