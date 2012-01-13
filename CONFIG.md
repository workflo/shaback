repository(dir)
-------------------------

Tells shaback which repository to use.

    repository('/mnt/backup-medium')

localCache(cachefile)
-------------------------

Tells shaback where to store the local cache file.

    localCache('/tmp/shaback-cache.gdbm')

oneFileSystem(bool)
-------------------------

Determines whether shaback should stick to one file system.

    oneFileSystem(true)

addDir(dir)
-------------------------

Adds a directory to the list of source directories to be backed up.

    addDir('/home')
    addDir('/Users')

clearDirs()
-------------------------

Clears the list of directories to be backed up.

addExcludePattern(pattern)
-------------------------

Adds an exclude pattern. `*` and `?` can be used as globbing wildcards. (See `man fnmatch` for details.)

    addExcludePattern('*/*~')

cryptoPassword(pw)
-------------------------

Sets the password used to encrypt and decrypt backup data.

    cryptoPassword('xyz')

backupName(name)
-------------------------

Sets the name of this backup set. The name is reflected in the suffix of the index file name created in your `<REPO_DIR>/index/` directory.

    backupName('myBackupName')

showTotals(bool)
-------------------------

Determines whether totals are to be printed at the end of backup, restore and garbage collection.

    showTotals(true)
