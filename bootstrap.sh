#!/bin/bash

autoreconf --install
aclocal
autoheader
automake --add-missing --copy
autoconf
./configure --prefix=/usr         # or wherever you want to install it
#make
#make install
