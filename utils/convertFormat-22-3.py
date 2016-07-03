#!/usr/bin/python2.7
# -*-python-*-

#
# shaback - A hashsum based backup tool
# Copyright (C) 2010 Florian Wolff  (florian@donuz.de)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#


import getopt, sys
import os
import shutil
import string



def main():
    if not os.path.exists('files') or not os.path.exists('index'):
        sys.stderr.write("This is not a shaback repo!\n")
        sys.exit(3)

    for level0 in range(0, 16*16*16):
        level0Path = os.path.join('files', "%03x" % (level0))
        if not os.path.exists(level0Path):
            os.makedirs(level0Path)

    for level0 in range(0, 256):
        level0Path = os.path.join('files', "%02x" % (level0))
        for level1 in range(0, 256):
            dir22 = os.path.join(level0Path, "%02x" % (level1))
            dir3 = os.path.join('files', "%03x" % (level0 *16 + (level1 >> 4)))
            prefix3 = ("%02x" % level1)[1:]
            sys.stdout.write(" " + dir22 + " -> " + dir3 + "\n")

            for file22 in os.listdir(dir22):
                file3 = os.path.join(dir3, prefix3 + file22)
                sys.stdout.write("   - " + file22 + " -> " + file3 + "\n")
                shutil.move(os.path.join(dir22, file22), file3)

            os.rmdir(dir22)
        os.rmdir(level0Path)


if __name__ == "__main__":
    #cProfile.run('main()')
    main()
