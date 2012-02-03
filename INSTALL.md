Dependencies
=============================

Debian
------------------------------

	sudo apt-get install cmake g++ libssl-dev libz-dev liblua5.1-dev libgdbm-dev


MacOS X
------------------------------
You need to install XCode and [MacPorts](http://www.macports.org/) first.

	sudo port install cmake lua gdbm openssl zlib


Build from source
==============================

	cd src
	cmake .
	make
	sudo make install

Windows
==============================
Building shaback on Windows is still work in progress. But the following setup looks quite promising:

- Visual Studio C++ (Express) 2010
- git: http://code.google.com/p/msysgit/downloads/list?can=3&q=official+Git
- Lua: http://code.google.com/p/luaforwindows/downloads/list
- ZLib: http://gnuwin32.sourceforge.net/packages/zlib.htm
- GDBM: http://gnuwin32.sourceforge.net/packages/gdbm.htm
- openssl: http://www.slproweb.com/products/Win32OpenSSL.html
