#!/bin/bash

autoreconf --install
aclocal
autoheader
automake --add-missing --copy
autoconf
if [ "$1" != "--noconf" ]; then
	./configure --prefix=/usr         # or wherever you want to install it
	#make
	#make install
fi
