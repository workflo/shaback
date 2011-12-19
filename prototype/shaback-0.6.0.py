#!/usr/bin/python
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

import getopt, sys
import os
from stat import *
import hashlib
import shutil
import tempfile
import csv
import socket
from datetime import datetime, date, time
import fnmatch
import zlib
import cStringIO

# Try to load PyCrypto:
try:
    from Crypto.Cipher import AES
    HAVE_AES = True
except ImportError, err:
    HAVE_AES = False

VERSION = '0.6.0'
CIPHER_BLOCK_SIZE = 16
BLOCK_SIZE = CIPHER_BLOCK_SIZE * 64 * 1024    # Must be a multiple of CIPHER_BLOCK_SIZE!
SPLIT_BLOCK_SIZE = BLOCK_SIZE                 # Must be a multiple of CIPHER_BLOCK_SIZE!
ALL_COMMANDS = ['backup', 'gc', 'init', 'restore', 'list', 'extract', 'help']


def main():
    global verbose, debug, force
    global ALL_COMMANDS

    try:
        opts, args = getopt.getopt(sys.argv[1:], "ha:zvx:X:Z:s:S:d:n:tofy",
				   ["help", "verbose", "debug",
                                    "archive=",  
				    "exclude=", "exclude-from=",
				    "one-file-system",
				    "split=", "split-from=",
				    "destination=", "name=",
				    "totals", "force", "yes",
                                    "aes="])
    except getopt.GetoptError, err:
        sys.stderr.write(str(err) + "\n\n")
        usage()
        sys.exit(2)

    archiveDir = None
    verbose = False
    debug = False
    name = socket.getfqdn()
    excludes = []
    splitFiles = []
    destinationDir = '.'
    totals = False
    force = False
    yes = False
    oneFileSystem = False
    aesSecret = None

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
	elif o in ("-s", "--split"):
	    splitFiles += [a]
	elif o in ("-S", "--split-from"):
	    splitFiles += loadGlobPatterns(a);
	elif o in ("-n", "--name"):
	    name = a
	elif o in ("-t", "--totals"):
	    totals = True
	elif o in ("-f", "--force"):
	    force = True
	elif o in ("-y", "--yes"):
	    yes = True
	elif o in ("-o", "--one-file-system"):
	    oneFileSystem = True
        elif o in ("--aes"):
	    if HAVE_AES:
		aesSecret = a
	    else:
		sys.stderr.write("AES encryption not available. Missing PyCrypt: http://www.dlitz.net/software/pycrypto/\n")
		sys.exit(2)
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
    archive.setSplitFiles(splitFiles)
    archive.setOneFileSystem(oneFileSystem)
    archive.setAesSecret(aesSecret)
    
    if not archive.isValidArchive():
	sys.stderr.write("The given directory '" + archiveDir + "' is not a valid archive directory.\n")
	sys.exit(2)

    if command in ('backup'):
	if len(filenames) == 0:
	    sys.stderr.write("Nothing to backup.\n\n")
	    usage('backup')
	    sys.exit(1)
        archive.start()
        for filename in filenames:
            archive.walkFiles(os.path.abspath(filename))
        archive.finish()
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
	    print indexFile.filename
	return
    elif command in ('gc'):
	archive.gc(yes)
	

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
	self.indexFile = os.path.join(self.indexDir, "%s-%04d%02d%02d_%02d%02d%02d.shaback" % (name, now.year, now.month, now.day, now.hour, now.minute, now.second))
	self.indexFileCipher = None
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
        

    #
    # Indicates whether the specified directory looks like a
    # shaback archive.
    #
    def isValidArchive(self):
	if not os.path.isdir(self.filesDir):
	    return False
	if not os.path.isdir(self.indexDir):
	    return False
	return True


    #
    # Creates the index file.
    #
    def start(self):
	global verbose

        [fh, self.tmpIndexFilename] = tempfile.mkstemp(suffix='.shaback')

	self.indexFileOutput = ShabackOutputStream(self.tmpIndexFilename, 'wb', self.aesKey)
	self.indexFileWriter = csv.writer(self.indexFileOutput, delimiter=';', quotechar='"')

	if verbose:
	    print "Temporary index file: " + self.tmpIndexFilename

	self.indexFileWriter.writerow(['sha1', 'path', 'st_mode', 'st_uid', 'st_gid', 'st_atime', 'st_mtime', 'st_ctime', 'link', 'flags'])


    def finish(self):
	self.indexFileOutput.close()
	shutil.move(self.tmpIndexFilename, self.indexFile)

        self.endDate = datetime.now()
        
	if self.totals:
	    if self.errors > 0:
		print
	    print "New index file:  %s" % (self.indexFile)
	    print "Files inspected: %12d" % (self.filesInspected)
	    print "Bytes inspected: %12d" % (self.bytesInspected)
	    print "Files saved:     %12d" % (self.filesSaved)
	    print "Bytes saved:     %12d" % (self.bytesSaved)
	    print "Errors:          %12d" % (self.errors)

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
		    print ". " + filename
		elif backupFile.isSplit:
		    print "S " + filename
		else:
		    print "B " + filename

	finally:
	    #fcntl.flock(f, fcntl.LOCK_UN)
	    backupFile.close()

    def handleDirectory(self, filename, stat):
	self.indexFileWriter.writerow(['dir'.ljust(40), filename, 
				       stat[ST_MODE], stat[ST_UID], stat[ST_GID], 
				       stat[ST_ATIME], stat[ST_MTIME], stat[ST_CTIME], 
				       '', ''])

        if self.oneFileSystem and filename <> "/" and os.path.ismount(filename):
            return
        
	for child in os.listdir(filename):
	    self.walkFiles(os.path.join(filename, child))


    def handleSymlink(self, filename, stat):
	dest = os.readlink(filename)
	self.indexFileWriter.writerow(['symlink'.ljust(40), filename,
				       stat[ST_MODE], stat[ST_UID], stat[ST_GID],
				       stat[ST_ATIME], stat[ST_MTIME], stat[ST_CTIME],
				       dest, ''])


    def walkFiles(self, filename):
	filename = os.path.normpath(filename)
	if self.excludeFile(filename):
	    return
	try:
	    stat = os.lstat(filename)
	    mode = stat[ST_MODE]
	    if S_ISLNK(mode):
		self.handleSymlink(filename, stat)
	    elif S_ISDIR(mode):
		self.handleDirectory(filename, stat)
	    elif S_ISREG(mode):
		self.handleFile(filename, stat)
	except OSError,  err:
	    self.error(err)


    def excludeFile(self, filename):
	for pattern in self.excludes:
	    if fnmatch.fnmatch(filename, pattern):
		return True
	return False

    def splitFile(self, filename):
	for pattern in self.splitFiles:
	    if fnmatch.fnmatch(filename, pattern):
		return True
	return False

    def setTotals(self, totals):
	self.totals = totals

    def setSplitFiles(self, splitFiles):
	self.splitFiles = splitFiles

    def setOneFileSystem(self, oneFileSystem):
	self.oneFileSystem = oneFileSystem;

    def setAesSecret(self, aesSecret):
        if aesSecret is not None:
            self.aesKey = hashlib.sha256(aesSecret).digest()
        
    def gc(self, yes):
	gc = GarbageCollection(self)
	gc.run(yes)

    def restore(self, indexFile, destinationDir, filenames):
	global verbose

	for idx in range(0, len(filenames)):
	    if filenames[idx][-1:] == '/':
		filenames[idx] = filenames[idx][0:-1]

	self.destinationDir = os.path.abspath(destinationDir)

	try:
	    self.indexFile = IndexFile(self, os.path.basename(indexFile))
	except IOError, err:
	    self.error("Not a valid shaback index file: '" + indexFile + "'")
	    sys.exit(2)

	for entry in self.indexFile.iterate():
	    if entry.isHeader:
		continue

	    if self.excludeFile(entry.filename):
		if verbose:
		    print "   excluding " + filename
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
		print destinationFile
	    if not os.path.isdir(destinationFile):
		os.makedirs(destinationFile)
	elif entry.isSymlink:
            if os.path.lexists(destinationFile):
                os.remove(destinationFile)
            os.symlink(entry.linkDestination, destinationFile)
	    if verbose:
		print destinationFile
	    return
	else:
	    if verbose:
		print destinationFile
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
	except OSError, err:
	    self.error(err)

	try:
	    os.utime(destinationFile, (entry.atime, entry.mtime))
	    os.chmod(destinationFile, entry.mode)
	except OSError, err:
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
	    print " reading index: " + indexFile.filename

	for entry in indexFile.iterate():
	    if entry.isFile:
		if verbose:
		    print entry.archivePath()
		# if not exists...:
		try:
		    shutil.copyfile(entry.archivePath(), os.path.join(destArchive.filesDir, destArchive.makeArchiveFilename(entry.sha1)))
		except IOError,  err:
		    self.error(err)

	shutil.copyfile(indexFile.path, os.path.join(destArchive.indexDir, indexFile.filename))
	    

    def list(self):
	l = []
	for child in os.listdir(self.indexDir):
	    if fnmatch.fnmatch(child, "*-????????_??????.shaback"):
		l.append(IndexFile(self, child))
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
	global verbose

	if os.path.isfile(self.archiveFilename):
	    self.alreadyBackedUp = True
	    self.isSplit = False
	elif os.path.isfile(self.archiveFilename + ".blist"):
	    self.alreadyBackedUp = True
	    self.isSplit = True
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
	global verbose, BLOCK_SIZE, SPLIT_BLOCK_SIZE

	if self.stat[ST_SIZE] > SPLIT_BLOCK_SIZE * 5 and self.archive.splitFile(self.sourceFile):
	    self.isSplit = True
	    self.copySplit()
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

	self.archive.filesSaved += 1


    def copySplit(self):
	global verbose, SPLIT_BLOCK_SIZE

	if verbose:
	    print "   splitting file " + self.sourceFile + "..."

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
			print "   . " + blockFilename
		else:
		    if verbose:
			print "   B " + blockFilename

		    output = ShabackOutputStream(blockFilename + '.tmp', 'wb', self.archive.aesKey)
		    try:
			output.write(block)
			self.archive.bytesSaved += l			    
		    finally:
			output.close()
		    shutil.move(blockFilename + '.tmp', blockFilename)

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
	self.tree = []
	for level0 in range(0, 256):
	    level0List = []
	    for level1 in range(0, 256):
		level0List.append({})
	    self.tree.append(level0List)


    def addHashValue(self, sha1):
	index0 = int(sha1[0:2], 16)
	index1 = int(sha1[2:4], 16)
	remainder = sha1[4:]

	if remainder not in self.tree[index0][index1]:
	    self.tree[index0][index1][remainder] = True
	    self.filesToKeep += 1


    def run(self, yes):
	global force, verbose

	for indexFile in self.archive.list():
	    if verbose:
		print " reading index: " + indexFile.filename

	    for entry in indexFile.iterate():
		if entry.isFile:
		    self.addHashValue(entry.sha1)
		    if entry.isSplitFile:
			try:
			    bl = SplitFileReader(self.archive, entry.sha1)
			    for partSha1 in bl.iterateParts():
				self.addHashValue(partSha1)
			except IOError, err:
			    self.errors += 1
			    sys.stderr.write(str(err) + "\n")
			
	if self.errors > 0 and not force:
	    print
	    sys.stderr.write("There where %d errors. Bailing out, no harm has been done to archive! Use '--force' to override.\n" % (self.errors))
	else:
	    for index0 in range(0, 256):
		level0Name = "%02x" % (index0)
		level0Path = os.path.join(self.archive.filesDir, level0Name)

		if verbose:
		    print " scanning " + level0Name

		for index1 in range(0, 256):
		    level1Name = "%02x" % (index1)
		    level1Path = os.path.join(level0Path, level1Name)
		    for filename in os.listdir(level1Path):
			self.filesInArchive += 1
			remainder = filename[0:36]
			if remainder not in self.tree[index0][index1] or filename.endswith('.tmp'):
			    self.filesDeleted += 1
			    path = os.path.join(level1Path, filename)
			    if verbose:
				print " deleting " + filename
			    if yes:
				os.remove(path)

	    print
	    print "Files to keep:          %12d" % self.filesToKeep
	    print "Files deleted:          %12d" % self.filesDeleted
	    print "Files found in archive: %12d" % self.filesInArchive
	    if not yes:
		print
		print "This was a dry run. Nothing has been deleted for real. Use '--yes' for a live run."


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

    for level0 in range(0, 256):
	level0Path = os.path.join('files', "%02x" % (level0))
	for level1 in range(0, 256):
	    level1Path = os.path.join(level0Path, "%02x" % (level1))
	    if not os.path.exists(level1Path):
		os.makedirs(level1Path)

    print "Archive directory created."


def usage(command = None):
    global ALL_COMMANDS, VERSION

    if command is None:
	print "usage: shaback [-v|--verbose] {OPTIONS} COMMAND {ARGS}"
	print
	print "The valid commands are:"
	print "   backup      Backup a set of files or directories."
	print "   extract     Extract backup sets from archive."
	print "   gc          Garbage collection: Delete unused files from archive."
	print "   help        Show more information on a specific command."
	print "   init        Create a new archive directory."
	print "   list        List available backup sets."
	print "   restore     Restore files from archive."
	print
	print "See 'shaback help COMMAND' for more information on a specific command."
	print "Version: " + VERSION
    elif command == 'backup':
	print "usage: shaback -a|--archive              Location of existing archive."
	print "               [-x|--exclude=PATTERN]    Filepattern for files to be excluded."
        print "               [-X|--exclude-from=FILE]  Read filepatterns for files to be excluded from a file."
	print "               [-o|--one-file-system]    Stay on one filesystem."
	print "               [-s|--split=]             Filepattern for files to be split into blocks."
        print "               [-S|--split-from=FILE]    Read filepatterns for files to be split into blocks from a file."
	print "               [-n|--name=NAME]          Host name, defaults to `hostname`."
	print "               [--aes=SECRET]            AES secret."
	print "               [-t|--totals]             Print some statistics when done."
	print "               [-v|--verbose]            Be verbose."
	print "               backup FILES"
    elif command == 'restore':
	print "usage: shaback [-d|--destination=DIR]"
	print "               [--aes=SECRET]            AES secret."
	print "               [-v|--verbose]"
	print "               restore INDEXFILE {FILES}"
    elif command == 'init':
	print "usage: shaback [-f|--force]"
	print "               [-v|--verbose]"
	print "               init"
    elif command == 'extract':
	print "usage: shaback                           Extract backup sets from archive."
	print "               [-a|--archive]            Location of source archive."
	print "               [-v|--verbose]            Be verbose."
	print "               extract DESTINATION       Location of destination archive."
	print "               {INDEXFILES}              Which backup sets to extract."
    elif command == 'gc':
	print "usage: shaback [-y|--yes]                Actually DO a garbage collection."
	print "               [-f|--force]              Force GC ignoring errors."
	print "               [-v|--verbose]            Be verbose."
	print "               gc"
    elif command not in ALL_COMMANDS:
	print "'" + command + "' is not a valid shaback command."
	print
	usage()
	sys.exit(1)

    
if __name__ == "__main__":
    #cProfile.run('main()')
    main()

