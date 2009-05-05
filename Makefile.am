SUBDIRS = libsi src

EXTRA_DIST = bootstrap.sh conf include doc experiments

install-data-local:
	mkdir -p $(datadir)/xmltv/tv_grab_dvb_plus
	tar --exclude=".svn" -cf - -C conf . | (cd $(datadir)/xmltv/tv_grab_dvb_plus; tar xf -)

