Dependencies
=============================

Debian
------------------------------

	sudo apt-get install cmake g++ libssl-dev libz-dev liblua5.1-dev liblzo2-dev


MacOS X
------------------------------

	sudo port install cmake
	sudo port install lua
	sudo port install gdbm
	sudo port install openssl


Build from source
==============================

	cd src
	cmake .
	make


Windows
==============================
Building shaback on Windows is still work in progress. But the following setup looks quite promising:

- Visual Studio C++ (Express) 2010
- git: http://code.google.com/p/msysgit/downloads/list?can=3&q=official+Git
- Lua: http://code.google.com/p/luaforwindows/downloads/list
- ZLib: http://gnuwin32.sourceforge.net/packages/zlib.htm
- GDBM: http://gnuwin32.sourceforge.net/packages/gdbm.htm
- openssl: http://www.slproweb.com/products/Win32OpenSSL.html
