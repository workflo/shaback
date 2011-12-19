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

# Requires PyCrypto: http://www.dlitz.net/software/pycrypto/
# Requires GDBM3

import getopt, sys
import os
from stat import *
import hashlib
import shutil
import tempfile
import csv
import socket
from datetime import datetime, date, time, timedelta
import fnmatch
import zlib
import gzip
import cStringIO
import re
import string
import glob

# Try to load PyCrypto:
try:
    from Crypto.Cipher import AES
    HAVE_AES = True
except ImportError as err:
    HAVE_AES = False

# Try to load DBM:
HAVE_GDBM = False
try:
    import gdbm
    HAVE_DBM = True
    HAVE_GDBM = True
except ImportError as err:
    try:
	import dbm
	gdbm=dbm
	HAVE_DBM = True
    except ImportError as err:
	HAVE_DBM = False
	sys.stderr.write("python-gdbm not installed. No local index caching!\n\n")

VERSION = '0.7.1'
CIPHER_BLOCK_SIZE = 16
BLOCK_SIZE = CIPHER_BLOCK_SIZE * 64 * 8       # Must be a multiple of CIPHER_BLOCK_SIZE!
SPLIT_BLOCK_SIZE = 1024 * 1024                # Must be a multiple of CIPHER_BLOCK_SIZE!
ALL_COMMANDS = ['backup', 'gc', 'init', 'restore', 'list', 'extract', 'help', 'cleanup']


def main():
    global verbose, debug, force
    global ALL_COMMANDS

    try:
        opts, args = getopt.getopt(sys.argv[1:], "ha:zvx:X:Z:d:n:tofy",
                                   ["help", "verbose", "debug",
                                    "archive=",  
                                    "exclude=", "exclude-from=",
                                    "one-file-system",
                                    "destination=", "name=",
                                    "totals", "force", "yes",
                                    "aes=", "cache=",
				    "gc"])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err) + "\n\n")
        usage()
        sys.exit(2)

    archiveDir = None
    verbose = False
    debug = False
    name = socket.gethostname()
    excludes = []
    destinationDir = '.'
    totals = False
    force = False
    yes = False
    gc = False
    oneFileSystem = False
    aesSecret = None
    cacheFile = None
    retentionPolicy = '7,30,365'

    for o, a in opts:
        if o in ("-v", "--verbose"):
            verbose = True
        elif o in ("-d", "--destination"):
            destinationDir = a
        elif o in ("--debug"):
            debug = True
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-a", "--archive"):
            archiveDir = a
        elif o in ("-x", "--exclude"):
            excludes += [a];
        elif o in ("-X", "--exclude-from"):
            excludes += loadGlobPatterns(a);
        elif o in ("-n", "--name"):
            name = a
        elif o in ("-t", "--totals"):
            totals = True
        elif o in ("-f", "--force"):
            force = True
        elif o in ("-y", "--yes"):
            yes = True
        elif o in ("--gc"):
            gc = True
        elif o in ("-o", "--one-file-system"):
            oneFileSystem = True
        elif o in ("--aes"):
            if HAVE_AES:
                aesSecret = a
            else:
                sys.stderr.write("AES encryption not available. Missing PyCrypt: http://www.dlitz.net/software/pycrypto/\n")
                sys.exit(2)
        elif o in ("--cache"):
            cacheFile = a
        elif o in ("--retention"):
            retentionPolicy = a
        else:
            sys.stderr.write("Unhandled option '" + o + "'.\n\n")
            sys.exit(1)

    if len(args) == 0:
        sys.stderr.write("No Operation given. What do you want to do today?\n\n")
        usage()
        sys.exit(1)

    command = args[0]
    filenames = args[1:]

    if command not in ALL_COMMANDS:
        sys.stderr.write("'" + command + "' is not a valid shaback command.\n\n")        
        usage()
        sys.exit(1)

    if command == 'init':
        createArchive()
        return

    if command == 'help':
        if len(filenames) == 0:
            usage()
            sys.exit(1)
        else:    
            usage(filenames[0])
            return

    if command == 'restore':
        if len(filenames) == 0:
            sys.stderr.write("Missing INDEXFILE. Which backup set do you want to restore?\n\n")
            usage('restore')
            sys.exit(1)
        indexFile = filenames[0]
        filenames = filenames[1:]
        archive = Archive(os.path.dirname(os.path.dirname(indexFile)), name, excludes)
        archive.setAesSecret(aesSecret)
        archive.restore(indexFile, destinationDir, filenames)
        return


    if not archiveDir:
        archiveDir = os.getcwd()

    archive = Archive(archiveDir, name, excludes)
    archive.setTotals(totals)
    archive.setOneFileSystem(oneFileSystem)
    archive.setAesSecret(aesSecret)
    archive.setCacheFile(cacheFile)
    archive.setRetentionPolicy(retentionPolicy)
    
    if not archive.isValidArchive():
        sys.stderr.write("The given directory '" + archiveDir + "' is not a valid archive directory.\n")
        sys.exit(2)

    if command in ('backup'):
        if len(filenames) == 0:
            sys.stderr.write("Nothing to backup.\n\n")
            usage('backup')
            sys.exit(1)
        archive.lock(False)
        try:
            archive.start()
            for filename in filenames:
                archive.walkFiles(os.path.abspath(filename), 0)
            archive.finish()
        finally:
            archive.unlock()
    elif command in ('extract'):
        if len(filenames) == 0:
            sys.stderr.write("Missing DESTINATION. To which archive to you want to copy stuff?\n\n")
            usage('extract')
            sys.exit(1)
        if len(filenames) == 1:
            sys.stderr.write("Nothing to extract.\n\n")
            usage('extract')
            sys.exit(1)
        destArchive = Archive(filenames[0], name, excludes)
        if not destArchive.isValidArchive():
            sys.stderr.write("The given destination directory '" + filenames[0] + "' is not a valid archive directory.\n")
            sys.exit(2)
        
        indexFiles = filenames[1:]
        for indexFile in indexFiles:
            archive.extract(destArchive, IndexFile(archive, indexFile))
    elif command in ('list'):
        l = archive.list()
        for indexFile in l:
            print (indexFile.filename)
        return
    elif command in ('fsck'):
        archive.lock(True)
        try:
	    archive.fsck(gc, yes)
        finally:
            archive.unlock()
    elif command in ('gc'):
        archive.lock(True)
        try:
            archive.gc(yes)
        finally:
            archive.unlock()
    elif command in ('cleanup'):
        archive.cleanup(yes)
        

def loadGlobPatterns(filename):
    excludes = []
    file = open(filename, 'rb')
    try:
        while True:
            line = file.readline()
            if len(line) == 0:
                break
            line = line.rstrip("\n\r ").lstrip()
            if len(line) == 0 or line[0] == '#':
                continue
            else:
                excludes += [line]
    finally:
        file.close()
    return excludes

class Archive:
    def __init__(self, destination, name, excludes):
        now = datetime.now()
        self.name = name
        self.baseDir = destination
        self.filesDir = os.path.join(destination, "files")
        self.indexDir = os.path.join(destination, "index")
        self.locksDir = os.path.join(destination, "locks")
        self.indexFile = os.path.join(self.indexDir, "%s-%04d%02d%02d_%02d%02d%02d.shaback" % (name, now.year, now.month, now.day, now.hour, now.minute, now.second))
        self.indexFileCipher = None
        self.cacheFile = os.path.join(destination, "cache.gdbm")
        self.cacheDb = None
        self.globalLockFile = os.path.join(self.locksDir, "lock")
        self.localLockFile = os.path.join(self.locksDir, "%s-%i.lock" % (self.name, os.getpid()))
        self.keepGlobalLock = False
        self.excludes = excludes
        self.oneFileSystem = False
        self.filesInspected = 0
        self.filesSaved = 0
        self.bytesInspected = 0
        self.bytesSaved = 0
        self.errors = 0
        self.totals = False
        self.startDate = now
        self.aesKey = None
        

    def lock(self, keepGlobalLock):
        open(self.localLockFile, 'w', os.O_CREAT | os.O_EXCL)
        try:
            os.symlink(self.localLockFile, self.globalLockFile)
        except OSError as err:
            self.unlock()
            if err.errno == 17:
                self.error("Archive is locked. Unable to acquire global lock. Check lock files in `" + self.locksDir + "'.")
            else:
                raise err
            sys.exit(2)

        if keepGlobalLock:
            self.keepGlobalLock = True
            otherLocks = glob.glob(os.path.join(self.locksDir, '*.lock'))
            if len(otherLocks) > 1:
                self.unlock()
                self.error("Non-exclusive locks exist. Check lock files in `" + self.locksDir + "'.")
                for f in otherLocks:
                    if f != self.localLockFile:
                        self.error(" - " + f)
                sys.exit(2)

                
        if not keepGlobalLock:
            os.unlink(self.globalLockFile)
        

    def unlock(self):
        os.unlink(self.localLockFile)
        if self.keepGlobalLock:
            os.unlink(self.globalLockFile)


    #
    # Indicates whether the specified directory looks like a
    # shaback archive.
    #
    def isValidArchive(self):
        if not os.path.isdir(self.filesDir):
            return False
        if not os.path.isdir(self.indexDir):
            return False
        if not os.path.isdir(self.locksDir):
            return False
        return True


    #
    # Creates the index file.
    #
    def start(self):
        global verbose, HAVE_DBM, HAVE_GDBM

        [fh, self.tmpIndexFilename] = tempfile.mkstemp(suffix='.shaback')

        self.indexFileOutput = ShabackOutputStream(self.tmpIndexFilename, 'wb', self.aesKey)
        self.indexFileWriter = csv.writer(self.indexFileOutput, delimiter=';', quotechar='"')

        if verbose:
            print("Temporary index file: " + self.tmpIndexFilename)

        self.indexFileWriter.writerow(['sha1', 'path', 'st_mode', 'st_uid', 'st_gid', 'st_atime', 'st_mtime', 'st_ctime', 'link', 'flags'])
        if HAVE_GDBM:
            self.cacheDb = gdbm.open(self.cacheFile, 'cf')
        elif HAVE_DBM:
            self.cacheDb = gdbm.open(self.cacheFile, 'c')


    def finish(self):
        global HAVE_DBM
        self.indexFileOutput.close()
        shutil.move(self.tmpIndexFilename, self.indexFile)

        self.endDate = datetime.now()
        
        if self.totals:
            if self.errors > 0:
                print
            print("New index file:  %s" % (self.indexFile))
            print("Files inspected: %12d" % (self.filesInspected))
            print("Bytes inspected: %12d" % (self.bytesInspected))
            print("Files saved:     %12d" % (self.filesSaved))
            print("Bytes saved:     %12d" % (self.bytesSaved))
            print("Errors:          %12d" % (self.errors))

        if HAVE_DBM:
            self.cacheDb.close()

        if self.errors > 0:
            sys.exit(3)

    def makeArchiveFilename(self, hashvalue):
        return os.path.join(self.filesDir, hashvalue[0:2], hashvalue[2:4], hashvalue[4:])

    def handleFile(self, filename, stat):
        global verbose
        global debug
        global BLOCK_SIZE
            
        self.filesInspected += 1
        self.bytesInspected += stat[ST_SIZE]

        backupFile = BackupFile(self, filename, stat)

        #flock = fcntl.flock(f, fcntl.LOCK_SH)

        try:
            backupFile.open()
            backupFile.calculateHashValue()
            backupFile.backup()

            if verbose:
                if backupFile.alreadyBackedUp:
                    print(". " + filename)
                elif backupFile.isSplit:
                    print("S " + filename)
                else:
                    print("B " + filename)

        finally:
            #fcntl.flock(f, fcntl.LOCK_UN)
            backupFile.close()

    def handleDirectory(self, filename, stat, depth):
        self.indexFileWriter.writerow(['dir'.ljust(40), filename, 
                                       stat[ST_MODE], stat[ST_UID], stat[ST_GID], 
                                       stat[ST_ATIME], stat[ST_MTIME], stat[ST_CTIME], 
                                       '', ''])

        if depth > 0 and self.oneFileSystem and filename != "/" and os.path.ismount(filename):
            return
        
        for child in os.listdir(filename):
            self.walkFiles(os.path.join(filename, child), depth +1)


    def handleSymlink(self, filename, stat):
        dest = os.readlink(filename)
        self.indexFileWriter.writerow(['symlink'.ljust(40), filename,
                                       stat[ST_MODE], stat[ST_UID], stat[ST_GID],
                                       stat[ST_ATIME], stat[ST_MTIME], stat[ST_CTIME],
                                       dest, ''])


    def walkFiles(self, filename, depth):
        filename = os.path.normpath(filename)
        if self.excludeFile(filename):
            return
        try:
            stat = os.lstat(filename)
            mode = stat[ST_MODE]
            if S_ISLNK(mode):
                self.handleSymlink(filename, stat)
            elif S_ISDIR(mode):
                self.handleDirectory(filename, stat, depth)
            elif S_ISREG(mode):
                self.handleFile(filename, stat)
        except OSError as err:
            self.error(err)


    def excludeFile(self, filename):
        for pattern in self.excludes:
            if fnmatch.fnmatch(filename, pattern):
                return True
        return False

    def setTotals(self, totals):
        self.totals = totals

    def setOneFileSystem(self, oneFileSystem):
        self.oneFileSystem = oneFileSystem;

    def setAesSecret(self, aesSecret):
        if aesSecret is not None:
            self.aesKey = hashlib.sha256(aesSecret).digest()

    def setCacheFile(self, cacheFile):
        if cacheFile:
            self.cacheFile = cacheFile

    def setRetentionPolicy(self, retentionPolicy):
        self.retentionPolicy = string.split(retentionPolicy, ',')
        if len(self.retentionPolicy) <> 3:
            self.error('Invalid retention policy. Should be of the following format: "X,Y,Z"')
            sys.exit(1)
        
    def gc(self, yes):
        gc = GarbageCollection(self)
        gc.run(yes)

    def cleanup(self, yes):
        cu = Cleanup(self)
        cu.run(yes)

    def restore(self, indexFile, destinationDir, filenames):
        global verbose

        for idx in range(0, len(filenames)):
            if filenames[idx][-1:] == '/':
                filenames[idx] = filenames[idx][0:-1]

        self.destinationDir = os.path.abspath(destinationDir)

        try:
            self.indexFile = IndexFile(self, os.path.basename(indexFile))
        except IOError as err:
            self.error("Not a valid shaback index file: '" + indexFile + "'")
            sys.exit(2)

        for entry in self.indexFile.iterate():
            if entry.isHeader:
                continue

            if self.excludeFile(entry.filename):
                if verbose:
                    print("   excluding " + filename)
            elif len(filenames) == 0:
                self.restoreFile(entry)
            else:
                for prefix in filenames:
                    if fnmatch.fnmatch(entry.filename, prefix) or fnmatch.fnmatch(entry.filename, prefix + "/**"):
                        self.restoreFile(entry)
                    

    def restoreFile(self, entry):
        global verbose, BLOCK_SIZE

        if os.path.isabs(entry.filename):
            entry.filename = entry.filename[1:]
        destinationFile = os.path.join(self.destinationDir, entry.filename)
        destinationDir = os.path.dirname(destinationFile)

        if not os.path.isdir(destinationDir):
            os.makedirs(destinationDir)

        if entry.isDir:
            if verbose:
                print(destinationFile)
            if not os.path.isdir(destinationFile):
                os.makedirs(destinationFile)
        elif entry.isSymlink:
            if os.path.lexists(destinationFile):
                os.remove(destinationFile)
            os.symlink(entry.linkDestination, destinationFile)
            if verbose:
                print(destinationFile)
            return
        else:
            if verbose:
                print(destinationFile)
            sourceFile = self.makeArchiveFilename(entry.sha1)

            if os.path.isfile(sourceFile):
                isBlockList = False
                fh = ShabackInputStream(sourceFile, 'rb', self.aesKey)
            elif os.path.isfile(sourceFile + ".blist"):
                isBlockList = True
                fh = gzip.open(sourceFile + ".blist", "rt")
            else:
                self.error("ERROR: Archived file content not found: " + sourceFile)
                return
            try:
                if os.path.lexists(destinationFile):
                    os.remove(destinationFile)

                outfh = open(destinationFile, 'wb')

                try:
                    if isBlockList:
                        bl = SplitFileReader(self, entry.sha1)
                        for partSha1 in bl.iterateParts():
                            self.appendFileContents(outfh, partSha1)
                    else:
                        while True:
                            buffer = fh.read(BLOCK_SIZE)
                            if len(buffer) == 0:
                                break
                            outfh.write(buffer)
                finally:
                    outfh.close()
            finally:
                fh.close()

        try:
            os.lchown(destinationFile, entry.uid, entry.gid)
        except OSError as err:
            self.error(err)

        try:
            os.utime(destinationFile, (entry.atime, entry.mtime))
            os.chmod(destinationFile, entry.mode)
        except OSError as err:
            self.error(err)


    def appendFileContents(self, outfh, sha1):
        global verbose, BLOCK_SIZE

        sourceFile = self.makeArchiveFilename(sha1)

        if os.path.isfile(sourceFile):
            fh = ShabackInputStream(sourceFile, 'rb', self.aesKey)
        else:
            self.error("ERROR: Archived file content not found: " + sourceFile)
            return
        try:
            while True:
                buffer = fh.read(BLOCK_SIZE)
                if len(buffer) == 0:
                    break
                outfh.write(buffer)
        finally:
            fh.close()


    def extract(self, destArchive, indexFile):
        if verbose:
            print(" reading index: " + indexFile.filename)

        for entry in indexFile.iterate():
            if entry.isFile:
                if verbose:
                    print(entry.archivePath())
                # if not exists...:
                try:
                    shutil.copyfile(entry.archivePath(), os.path.join(destArchive.filesDir, destArchive.makeArchiveFilename(entry.sha1)))
                except IOError as err:
                    self.error(err)

        shutil.copyfile(indexFile.path, os.path.join(destArchive.indexDir, indexFile.filename))
            

    def list(self):
        l = []
        for child in os.listdir(self.indexDir):
            if fnmatch.fnmatch(child, "*-????????_??????.shaback"):
                l.append(IndexFile(self, child))
        return l
            

    def listForName(self):
        l = []
        for child in os.listdir(self.indexDir):
            if fnmatch.fnmatch(child, self.name + "-????????_??????.shaback") or fnmatch.fnmatch(child, self.name + "-????????_??????.shaback.gz"):
                l.append(IndexFile(self, child))
        l.sort(None, lambda e: e.filename)
        return l


    def error(self, err):
        self.errors += 1
        sys.stderr.write(str(err) + "\n")

#
# Represents a single file to be backed up.
#
class BackupFile:
    def __init__(self, archive, sourceFile, stat):
        self.archive = archive
        self.sourceFile = sourceFile
        self.stat = stat
        self.content = None
        self.oneBlockOnly = False
        self.hashValue = None
        self.fh = None
        self.isSplit = False
        self.alreadyBackedUp = False


    #
    # Opens the source file.
    #
    def open(self):
        self.fh = open(self.sourceFile)

    #
    # Reads the whole file and calculates the hash value.
    # If the file is smaller than BLOCK_SIZE, keeps the
    # file's contents.
    #
    def calculateHashValue(self):
        global BLOCK_SIZE, verbose

        m = hashlib.sha1()
        numBlocks = 0

        while True:
            block = self.fh.read(BLOCK_SIZE)
            if (block == ""):
                break
            self.content = block
            m.update(block)
            numBlocks += 1

        if numBlocks == 0:
            # File is empty.
            self.content = ""
            self.close()
            self.oneBlockOnly = True
        elif numBlocks == 1:
            # File consists of exactly one block, keep it!
            self.close()
            self.oneBlockOnly = True
        else:
            # File has more than one block.
            self.content = None
            self.oneBlockOnly = False

        self.hashValue = m.hexdigest()
        self.archiveFilename = self.archive.makeArchiveFilename(self.hashValue)

    #
    # Checks whether the file needs to be backed up.
    # If so, starts copying to archive.
    #
    def backup(self):
        global verbose, HAVE_DBM

        if HAVE_DBM and self.archive.cacheDb.has_key(self.hashValue):
            self.alreadyBackedUp = True
	elif os.path.isfile(self.archiveFilename):
            self.alreadyBackedUp = True
            self.isSplit = False
            if HAVE_DBM:
                self.archive.cacheDb[self.hashValue] = "s"
        elif os.path.isfile(self.archiveFilename + ".blist"):
            self.alreadyBackedUp = True
            self.isSplit = True
            if HAVE_DBM:
                self.archive.cacheDb[self.hashValue] = "b"
        else:
            self.alreadyBackedUp = False
            self.copyFileToArchive()

        flags = ''
        if self.isSplit:
            flags = 'S'

        # Write index entry:
        self.archive.indexFileWriter.writerow([self.hashValue, self.sourceFile, 
                                               self.stat[ST_MODE], self.stat[ST_UID],
                                               self.stat[ST_GID], self.stat[ST_ATIME],
                                               self.stat[ST_MTIME], self.stat[ST_CTIME],
                                               '', flags])


    #
    # Copies the source file to the archive directory.
    # Decides whether to split the file.
    #
    def copyFileToArchive(self):
        global verbose, BLOCK_SIZE, SPLIT_BLOCK_SIZE, HAVE_DBM

        if self.stat[ST_SIZE] > SPLIT_BLOCK_SIZE * 5:
            self.isSplit = True
            self.copySplit()

	    # Update cache DB:
	    if HAVE_DBM:
		self.archive.cacheDb[self.hashValue] = "b"
        else:
            output = ShabackOutputStream(self.archiveFilename + '.tmp', 'wb', self.archive.aesKey)

            try:
                if self.oneBlockOnly:
                    output.write(self.content)
                else:
                    self.fh.seek(0)
                    while True:
                        buf = self.fh.read(BLOCK_SIZE)
                        if buf == "":
                            break
                        output.write(buf)
            finally:
                output.close()

            self.archive.bytesSaved += self.stat[ST_SIZE]
            shutil.move(self.archiveFilename + '.tmp', self.archiveFilename)

	    # Update cache DB:
	    if HAVE_DBM:
		self.archive.cacheDb[self.hashValue] = "s"

        self.archive.filesSaved += 1


    def copySplit(self):
        global verbose, SPLIT_BLOCK_SIZE, HAVE_DBM

        if verbose:
            print("   splitting file " + self.sourceFile + "...")

        self.archiveFilename += ".blist"
        blockList = gzip.open(self.archiveFilename + ".tmp", "wt")

        try:
            self.fh.seek(0)
            while True:
                m = hashlib.sha1()
                block = self.fh.read(SPLIT_BLOCK_SIZE)
                l = len(block)
                if l == 0:
                    break
                m.update(block)
                partHashValue = m.hexdigest()
                blockList.write(partHashValue + "\n")

                blockFilename = self.archive.makeArchiveFilename(partHashValue)
                if os.path.exists(blockFilename):
                    if verbose:
                        print("   . " + blockFilename)
                else:
                    if verbose:
                        print("   B " + blockFilename)

                    output = ShabackOutputStream(blockFilename + '.tmp', 'wb', self.archive.aesKey)
                    try:
                        output.write(block)
                        self.archive.bytesSaved += l                            
                    finally:
                        output.close()
                    shutil.move(blockFilename + '.tmp', blockFilename)

                    # Update cache DB:
                    if HAVE_DBM:
                        self.archive.cacheDb[partHashValue] = "s"

        finally:
            blockList.close()

        shutil.move(self.archiveFilename + ".tmp", self.archiveFilename)


    #
    # Closes the file handle if still open.
    #
    def close(self):
        if self.fh is not None:
            self.fh.close()
            self.handle = None


#
# Helps iterating over the rows of a given index file.
#
class IndexFile:
    def __init__(self, archive, filename):
        global BLOCK_SIZE

        self.archive = archive
        self.filename = os.path.basename(filename)
        self.path = os.path.join(archive.indexDir, self.filename)

        if not fnmatch.fnmatch(self.filename, "*.shaback"):
            raise IOError("Not a valid shaback index file: '" + self.filename + "'")

        fh = ShabackInputStream(self.path, 'rb', self.archive.aesKey)
        
        try:
            bytes = fh.read(-1)
            self.buffer = cStringIO.StringIO(bytes)
        finally:
            fh.close()

    def iterate(self):
        reader = csv.reader(self.buffer, delimiter=';', quotechar='"')
        for row in reader:
            yield IndexEntry(self.archive, row)

    def timestamp(self):
        m = re.search('.*-([0-9]{8}_[0-9]{6})\.shaback.*', self.filename)
        return datetime.strptime(m.group(1), '%Y%m%d_%H%M%S')


#
# Represents a single index entry.
#
class IndexEntry:
    def __init__(self, archive, row):
        self.archive = archive
        self.sha1 = row[0].strip()
        self.filename = row[1]
        self.flags = None
        self.isFile = False
        self.isDir = (self.sha1 == 'dir')
        self.isSymlink = (self.sha1 == 'symlink')
        self.isHeader = (self.sha1 == 'sha1')
        self.isSplitFile = False
        self.linkDestination = None

        if not self.isHeader:
            self.mode = int(row[2])
            self.uid = int(row[3])
            self.gid = int(row[4])
            self.atime = int(row[5])
            self.mtime = int(row[6])
            self.ctime = int(row[7])

        if self.isSymlink:
            self.linkDestination = row[8]

        if not self.isDir and not self.isSymlink and not self.isHeader:
            self.isFile = True
            if len(row) >= 10:
                self.flags = row[9]
                self.isSplitFile = (self.flags.find('S') > -1)

    def archivePath(self):
        name = self.archive.makeArchiveFilename(self.sha1)
        if self.isSplitFile:
            name += '.blist'
        return name



#
# A helper to iterate through the sha1 list of a split files's block
# list file.
#
class SplitFileReader:
    def __init__(self, archive, sha1):
        self.sha1 = sha1
        self.filename = archive.makeArchiveFilename(sha1) + '.blist'
        
        if not os.path.isfile(self.filename):
            raise IOError("Block list file not found for: '" + sha1 + "'")

    def iterateParts(self):
        fh = gzip.open(self.filename, "rt")
        try:
            while True:
                blockSha1 = fh.readline()
                if len(blockSha1) == 0:
                    break
                yield blockSha1.rstrip()
        finally:
            fh.close()


#
# Performs the garbage collection.
# Removes files from archive that are no longer referenced from any
# index file.
#
class GarbageCollection:
    def __init__(self, archive):
        self.archive = archive
        self.indexFiles = archive.list()
        self.filesToKeep = 0
        self.filesDeleted = 0
        self.filesInArchive = 0
        self.errors = 0


    def processIndexFile(self, indexFile):
        global verbose, HAVE_HDBM, HAVE_DBM

        indexFileCacheDbFile = os.path.dirname(self.archive.cacheFile) + '/' + indexFile.filename + '.gdbm'
        
        if verbose:
            print (" reading index: " + indexFile.filename)

        # Look for cached index file:
        #if os.path.isfile(indexFileCacheDbFile) or os.path.isfile(indexFileCacheDbFile + '.db'):
        #    self.readFromCacheFile(indexFileCacheDbFile)
        #else:
	self.readFromIndexFile(indexFile, indexFileCacheDbFile)


    def readFromCacheFile(self, indexFileCacheDbFile):
        global verbose, HAVE_HDBM, HAVE_DBM
        if verbose:
            print ("   reading cache file: " + indexFileCacheDbFile)

	indexFileCacheDb = gdbm.open(indexFileCacheDbFile, 'r')
        k = indexFileCacheDb.firstkey()
        while k != None:
            self.archive.cacheDb[k] = "y"
            k = indexFileCacheDb.nextkey(k)
        indexFileCacheDb.close()

        
    def readFromIndexFile(self, indexFile, indexFileCacheDbFile):
        global verbose, HAVE_HDBM, HAVE_DBM
        #if verbose:
        #    print ("   writing cache file: " + indexFileCacheDbFile)

        #if HAVE_GDBM:
        #    indexFileCacheDb = gdbm.open(indexFileCacheDbFile + '.tmp', 'nf')
        #elif HAVE_DBM:
        #    indexFileCacheDb = gdbm.open(indexFileCacheDbFile + '.tmp', 'n')
            
        for entry in indexFile.iterate():
            if entry.isFile:
                self.archive.cacheDb[entry.sha1] = "y"
                #indexFileCacheDb[entry.sha1] = "y"
                    
                if entry.isSplitFile:
                    try:
                        bl = SplitFileReader(self.archive, entry.sha1)
                        for partSha1 in bl.iterateParts():
                            self.archive.cacheDb[partSha1] = "y"
                            #indexFileCacheDb[partSha1] = "y"
                    except IOError as err:
                        self.errors += 1
                        sys.stderr.write(str(err) + "\n")

                        
        #if HAVE_GDBM:
        #    indexFileCacheDb.sync()
        #indexFileCacheDb.close()
        #if HAVE_DBM:
	#    shutil.move(indexFileCacheDbFile + '.tmp.db', indexFileCacheDbFile  + '.db')
	#else:
	#    shutil.move(indexFileCacheDbFile + '.tmp', indexFileCacheDbFile)


    def run(self, yes):
        global force, verbose, HAVE_HDBM, HAVE_DBM

        if HAVE_GDBM:
            self.archive.cacheDb = gdbm.open(self.archive.cacheFile, 'nf')
        elif HAVE_DBM:
            self.archive.cacheDb = gdbm.open(self.archive.cacheFile, 'n')

        for indexFile in self.archive.list():
            self.processIndexFile(indexFile)

        if HAVE_GDBM:
            self.archive.cacheDb.sync()

        if self.errors > 0 and not force:
            print
            sys.stderr.write("There where %d errors. Bailing out, no harm has been done to archive! Use '--force' to override.\n" % (self.errors))
        else:
            for index0 in range(0, 256):
                level0Name = "%02x" % (index0)
                level0Path = os.path.join(self.archive.filesDir, level0Name)

                if verbose:
                    print (" scanning " + level0Name)

                for index1 in range(0, 256):
                    level1Name = "%02x" % (index1)
                    level1Path = os.path.join(level0Path, level1Name)
                    for filename in os.listdir(level1Path):
                        self.filesInArchive += 1
                        remainder = filename[0:36]
                        fullname = level0Name + level1Name + remainder
                        if fullname not in self.archive.cacheDb:
                            self.filesDeleted += 1
                            path = os.path.join(level1Path, filename)
                            if verbose:
                                print (" deleting " + fullname)
                            if yes:
                                os.remove(path)
                        else:
                            self.filesToKeep += 1


            print
            print ("Files to keep:          %12d" % self.filesToKeep)
            print ("Files deleted:          %12d" % self.filesDeleted)
            print ("Files found in archive: %12d" % self.filesInArchive)
            if not yes:
                print
                print ("This was a dry run. Nothing has been deleted for real. Use '--yes' for a live run.")

        self.archive.cacheDb.close()


#
# Performs an integrity check with optional garbage collection.
# Removes files from archive that are no longer referenced from any
# index file.
#
class FileSystemCheck:
    def __init__(self, archive):
        self.archive = archive
        self.indexFiles = archive.list()
        self.filesToKeep = 0
        self.filesDeleted = 0
        self.filesInArchive = 0
        self.errors = 0


    def addHashValue(self, sha1, isBlockList):
        if sha1 not in self.archive.cacheDb:
            self.filesToKeep += 1
	    if isBlockList:
		self.archive.cacheDb[sha1] = "b"
	    else:
		self.archive.cacheDb[sha1] = "s"


    def run(self, gc, yes):
        global force, verbose, HAVE_GDBM, HAVE_DBM

        if HAVE_GDBM:
            self.archive.cacheDb = gdbm.open(self.archive.cacheFile, 'nf')
        elif HAVE_DBM:
            self.archive.cacheDb = gdbm.open(self.archive.cacheFile, 'n')

        for indexFile in self.archive.list():
            if verbose:
                print(" reading index: " + indexFile.filename)

            for entry in indexFile.iterate():
                if entry.isFile:
		    filename = entry.archivePath()
		    if os.path.isfile(filename):
			self.addHashValue(entry.sha1, False)
		    elif os.path.isfile(filename + ".blist"):
			self.addHashValue(entry.sha1, True)
			try:
			    bl = SplitFileReader(self.archive, entry.sha1)
			    for partSha1 in bl.iterateParts():
				if os.path.isfile(self.archive.makeArchiveFilename(partSha1)):
				    self.addHashValue(partSha1, False)
				else:
				    self.errors += 1
				    sys.stderr.write("Missing: " + indexFile.filename + ": " + entry.filename + "\n")
			except IOError as err:
			    self.errors += 1
			    sys.stderr.write(str(err) + "\n")
		    else:
			self.errors += 1
			sys.stderr.write("Missing: " + indexFile.filename + ": " + entry.filename + "\n")

        if HAVE_GDBM:
            self.archive.cacheDb.sync()
                        
        if self.errors > 0 and not force:
            print
            sys.stderr.write("There where %d errors. Bailing out, no harm has been done to archive! Use '--force' to override.\n" % (self.errors))
        elif gc:
            for index0 in range(0, 256):
                level0Name = "%02x" % (index0)
                level0Path = os.path.join(self.archive.filesDir, level0Name)

                if verbose:
                    print(" scanning " + level0Name)

                for index1 in range(0, 256):
                    level1Name = "%02x" % (index1)
                    level1Path = os.path.join(level0Path, level1Name)
                    for filename in os.listdir(level1Path):
                        self.filesInArchive += 1
                        remainder = filename[0:36]
			hashValue = level0Name + level1Name + remainder
                        if hashValue not in self.archive.cacheDb or filename.endswith('.tmp'):
                            self.filesDeleted += 1
                            path = os.path.join(level1Path, filename)
                            if verbose:
                                print(" deleting " + filename)
                            if yes:
                                os.remove(path)

            print
            print("Files to keep:          %12d" % self.filesToKeep)
            print("Files deleted:          %12d" % self.filesDeleted)
            print("Files found in archive: %12d" % self.filesInArchive)
            print("Errors:                 %12d" % self.errors)
            if not yes:
                print
                print("This was a dry run. Nothing has been deleted for real. Use '--yes' for a live run.")

        if HAVE_DBM:
            self.archive.cacheDb.close()


class ShabackOutputStream:
    def __init__(self, filename, mode, aesKey):
        self.filename = filename;
        self.compress = zlib.compressobj()
        self.fh = open(filename, mode)
        self.output = CipherOutputStream(self.fh, aesKey)

    def write(self, buffer):
        zipped = self.compress.compress(buffer)
        self.output.write(zipped)

    def close(self):
        zipped = self.compress.flush()
        self.output.write(zipped)
        self.output.close()
        self.fh.close()

class CipherOutputStream:
    def __init__(self, output, aesKey):
        self.output = output
        if aesKey is None:
            self.cipher = None
        else:
            self.cipher = AES.new(aesKey, AES.MODE_CBC)
        self.remainder = None

    def write(self, buffer):
        global CIPHER_BLOCK_SIZE
        if self.cipher is None:
            self.output.write(buffer)
        else:
            if self.remainder is not None:
                buffer = self.remainder + buffer
                self.remainder = None
                
            l = len(buffer)
            if (l % CIPHER_BLOCK_SIZE) > 0:
                numBlocks = l // CIPHER_BLOCK_SIZE
                self.remainder = buffer[numBlocks * CIPHER_BLOCK_SIZE:]
                buffer = buffer[:numBlocks * CIPHER_BLOCK_SIZE]
            self.output.write(self.cipher.encrypt(buffer))

    def flush(self):
        global CIPHER_BLOCK_SIZE
        if self.cipher is not None and self.remainder is not None:
            l = len(self.remainder)
            if (l % CIPHER_BLOCK_SIZE) > 0:
                padding = ''
                for i in range(CIPHER_BLOCK_SIZE - (l % CIPHER_BLOCK_SIZE)):
                    padding += '\0'
                self.remainder += padding
            self.output.write(self.cipher.encrypt(self.remainder))
        
    def close(self):
        self.flush()


class ShabackInputStream:
    def __init__(self, filename, mode, aesKey):
        self.fh = open(filename, mode);
        self.decompress = zlib.decompressobj()
        self.input = CipherInputStream(self.fh, aesKey)

    def read(self, length):
        # flush() fehlt!
        return self.decompress.decompress(self.input.read(length))

    def close(self):
        self.fh.close()

class CipherInputStream:
    def __init__(self, input, aesKey):
        self.input = input
        if aesKey is None:
            self.cipher = None
        else:
            self.cipher = AES.new(aesKey, AES.MODE_CBC)

    def read(self, length):
        global CIPHER_BLOCK_SIZE
        buffer = self.input.read(length)
        if self.cipher is not None:
            l = len(buffer)
            if (l % CIPHER_BLOCK_SIZE) > 0:
                padding = ''
                for i in range(CIPHER_BLOCK_SIZE - (l % CIPHER_BLOCK_SIZE)):
                    padding += '\0'
                buffer += padding
            buffer = self.cipher.decrypt(buffer)
        return buffer



class Cleanup:
    def __init__(self, archive):
        self.archive = archive
        

    def run(self, yes):
        global force, verbose

        indexFiles = self.archive.listForName()
        
        if len(indexFiles) == 0:
            sys.stderr.write ("No index files found for host `" + self.archive.name + "'.\n")
            sys.exit(1)

        deltaKeep = timedelta(int(self.archive.retentionPolicy[0]))
        deltaWeekly = timedelta(int(self.archive.retentionPolicy[1]))
        deltaMonthly = timedelta(int(self.archive.retentionPolicy[2]))
        now = datetime.now()

        if verbose:
            print ("Keep all for %s" % (deltaKeep))
            print ("Keep weekly for %s" % (deltaWeekly))
            print ("Keep monthly for %s" % (deltaMonthly))

        # Delete _old_ backups:
        toDelete = filter(lambda x: now-x.timestamp() > deltaMonthly, indexFiles)

        upper = (now - deltaKeep).replace(minute=0, second=0, hour=0, microsecond=0)

        # Keep weekly backups:
        while True:
            lower = upper - timedelta(7)
            if lower <= now - deltaWeekly:
                break
            inRange = filter(lambda x: x.timestamp() >= lower and x.timestamp() < upper, indexFiles)

            #if verbose:
            #    print ("Keep weekly backup between %s and %s" % (lower, upper))
            if len(inRange) > 1:
                toDelete += inRange[1:]
                #if verbose:
                #    print ("Keeping  %-50s | %s" % (inRange[0].filename, inRange[0].timestamp()))
                #    for indexFile in inRange[1:]:
                #        print ("  %-50s | %s" % (indexFile.filename, indexFile.timestamp()))
            upper = lower

        # Keep monthly backups:
        while True:
            lower = upper - timedelta(30)
            if lower <= now - deltaMonthly:
                break
            inRange = filter(lambda x: x.timestamp() >= lower and x.timestamp() < upper, indexFiles)

            #if verbose:
            #    print ("Keep monthly backup between %s and %s" % (lower, upper))
            if len(inRange) > 1:
                toDelete += inRange[1:]
                #if verbose:
                #    print ("Keeping  %-50s | %s" % (inRange[0].filename, inRange[0].timestamp()))
                #    for indexFile in inRange[1:]:
                #        print ("  %-50s | %s" % (indexFile.filename, indexFile.timestamp()))
            upper = lower

        toDelete.sort(None, lambda e: e.filename)

        if verbose:
            print ("Deleting index files:")
            for indexFile in toDelete:
                print (indexFile.path)

        if yes:
            for indexFile in toDelete:
                os.remove(indexFile.path)



def createArchive():
    global force

    if not force:
        if os.path.exists('files') or os.path.exists('index'):
            sys.stderr.write("This seems to be an archive directory already!\n")
            sys.exit(3)
        if len(os.listdir('.')) > 0:
            sys.stderr.write("This is not an empty directory!\n")
            sys.exit(3)

    if not os.path.exists('index'):
        os.mkdir('index')
    if not os.path.exists('locks'):
        os.mkdir('locks')

    for level0 in range(0, 256):
        level0Path = os.path.join('files', "%02x" % (level0))
        for level1 in range(0, 256):
            level1Path = os.path.join(level0Path, "%02x" % (level1))
            if not os.path.exists(level1Path):
                os.makedirs(level1Path)

    print("Archive directory created.")


def usage(command = None):
    global ALL_COMMANDS, VERSION

    if command is None:
        print("usage: shaback [-v|--verbose] {OPTIONS} COMMAND {ARGS}")
        print
        print("The valid commands are:")
        print("   backup      Backup a set of files or directories.")
        print("   gc          Garbage collection: Delete unused files from archive")
        print("   extract     Extract backup sets from archive.")
        #print("   fsck        Perform integrity check with optional garbage collection.")
        print("   help        Show more information on a specific command.")
        print("   init        Create a new archive directory.")
        print("   list        List available backup sets.")
        print("   restore     Restore files from archive.")
        print("   cleanup     Delete old index files")
        print
        print("See 'shaback help COMMAND' for more information on a specific command.")
        print("Version: " + VERSION)
    elif command == 'backup':
        print("usage: shaback -a|--archive              Location of existing archive.")
        print("               [-x|--exclude=PATTERN]    Filepattern for files to be excluded.")
        print("               [-X|--exclude-from=FILE]  Read filepatterns for files to be excluded from a file.")
        print("               [-o|--one-file-system]    Stay on one filesystem.")
        print("               [-n|--name=NAME]          Host name, defaults to `hostname`.")
        print("               [--aes=SECRET]            AES secret.")
        print("               [--cache=CACHEFILE]       Use alternate GDBM3 index cache file.")
        print("               [-t|--totals]             Print some statistics when done.")
        print("               [-v|--verbose]            Be verbose.")
        print("               backup FILES")
    elif command == 'restore':
        print("usage: shaback [-d|--destination=DIR]")
        print("               [--aes=SECRET]            AES secret.")
        print("               [-v|--verbose]")
        print("               restore INDEXFILE {FILES}")
    elif command == 'init':
        print("usage: shaback [-f|--force]")
        print("               [-v|--verbose]")
        print("               init")
    elif command == 'extract':
        print("usage: shaback                           Extract backup sets from archive.")
        print("               [-a|--archive]            Location of source archive.")
        print("               [-v|--verbose]            Be verbose.")
        print("               extract DESTINATION       Location of destination archive.")
        print("               {INDEXFILES}              Which backup sets to extract.")
    elif command == 'fsck':
        print("usage: shaback [--gc]                    Perform a garbage collection.")
        print("               [-y|--yes]                Actually DO a garbage collection.")
        print("               [-f|--force]              Force GC ignoring errors.")
        print("               [-v|--verbose]            Be verbose.")
        print("               fsck")
    elif command == 'gc':
        print ("usage: shaback [-y|--yes]                Actually DO a garbage collection.")
        print ("               [-f|--force]              Force GC ignoring errors.")
        print ("               [-v|--verbose]            Be verbose.")
        print ("               gc")
    elif command == 'cleanup':
        print ("usage: shaback [-y|--yes]                Actually DO delete old index files")
        print ("               [--retention=d,w,m]       Sets a certain retention policy.")
        print ("                                         Keep every single backup for d days,")
        print ("                                         keep a weekly backup for w more days,")
        print ("                                         keep a monthly backup for m more days,")
        print ("                                         delete older backups.")
        print ("               [-n|--name=NAME]          Host name, defaults to `hostname`.")
        print ("               cleanup")
    elif command not in ALL_COMMANDS:
        print("'" + command + "' is not a valid shaback command.")
        print
        usage()
        sys.exit(1)

    
if __name__ == "__main__":
    #cProfile.run('main()')
    main()

