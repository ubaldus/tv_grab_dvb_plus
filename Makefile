#
# Makefile for a Video Disk Recorder plugin
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
#
PLUGIN = loadepg

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).c | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
//CXXFLAGS ?= -O2 -Wall -Woverloaded-virtual
CXXFLAGS ?= -g -Wall -Woverloaded-virtual

### The directory environment:

#VDRDIR = ../../..
VDRDIR = .
LIBDIR = ../../lib
TMPDIR = /tmp

### Allow user defined options to overwrite defaults:

#-include $(VDRDIR)/Make.config

### The version number of VDR's plugin API (taken from VDR's "config.h"):

#APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
APIVERSION = "1.5.5"

# This is required for VDRs older than 1.3.47
ifeq ($(strip $(APIVERSION)),)
  APIVERSION = $(shell sed -ne '/define VDRVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
endif

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include
INCLUDES += -I$(VDRDIR)/include/libsi

DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"'
DEFINES += -DSTANDALONE 

### The object files (add further files here):

OBJS = $(PLUGIN).o libsi/section.o libsi/descriptor.o libsi/si.o libsi/util.o i18n.o tools.o thread.o vdrcompat.o menuitems.o osdbase.o misc.o device.o dvbdevice.o

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Internationalization (I18N):

#ISPO = $(shell grep '\#define APIVERSNUM' $(VDRDIR)/config.h | awk '{if($$3>=10507){printf("yes")}else{printf("no")}}')
ISPO = no

ifeq ($(ISPO),yes)

PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<dpluca@libero.it>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@
endif

### Targets:

#all: libvdr-$(PLUGIN).so i18n

all: loadepg i18n

.PHONY: i18n
i18n: $(I18Nmsgs)

libvdr-$(PLUGIN).so: $(OBJS)
	$(CXX) $(CXXFLAGS) -shared $(OBJS) -o $@
	@cp $@ $(LIBDIR)/$@.$(APIVERSION)

$(PLUGIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ -lpthread -ljpeg -lcap

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tgz core* *~
