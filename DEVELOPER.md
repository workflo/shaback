The Repository
---------------------------

The repository is a directory with the following structure:

	<REPO_DIR>
     	cache/
		files/
			00/00/
     		...
			ff/fe/
			ff/ff/
		index/
		locks/
		repo.properties

### cache/

### files/

The `files/` directory is the actual repository. It contains 256 sub directories (named `00`, `01`, `02`, ..., `fe`, `ff`) each of which also contains 256 sub directories (named `00`, `01`, `02`, ..., `fe`, `ff`).

### index/

Contains a `.sroot` file for each backup run. The file's name is comprised of the backup set (or host) name and a timestamp. The file's content is the bare ID of the root tree index file.

### locks/

Contains lock files during the runtime of a backup or garbage collection run.
Locks a necessary because garbage collection requires exclusive access to the repository. Backups and restoring files from a backup may be performed concurrently.

### repo.properties

This file contains configuration concerning the repository as a whole: It defines which data compression and encryption algorithms to use. These settings must be set **before** any backups are performed and may not be altered afterwards.
The file format is that of a standard Java(tm) properties file with the following keys:

	version = 2
	compression = Deflate
	encrytion = Blowfish
	digest = SHA1

## Tree index files


Libraries used
---------------------------

- openssl: http://www.openssl.org/docs/
- zlib: http://zlib.net/manual.html
- bzip2: http://bzip.org/docs.html
- GDBM: http://www.gnu.org.ua/software/gdbm/manual/gdbm.html
- Lua: http://www.lua.org/manual/5.1/
