repository(dir)
-------------------------

Tells shaback which repository to use.

    repository('/mnt/backup-medium')

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

addSplitPattern(pattern)
-------------------------

Files matching one of the specified split patterns whoes size exceeds 25MB will be split into blocks of 5MB when stored.
This option is most usefull for large binary files that are usually barely modified. 

    addSplitPattern('*.vmdk')

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
