Synopsis
===========================

Shaback is a backup tool for unixish server and client systems. Shaback stores all data in a designated repository directory. Each repository can be used to store backups of an arbitrary number of hosts and file sets. The way the data is stored ensures that storing duplicates is strictly avoided and each file content is stored only once.

How it works
===========================

The Repository
---------------------------

The repository is a directory with the following structure:

	<REPO_DIR>
     	cache/
		files/
			00/00
     		...
			ff/ff
		index/
		locks/
		repo.properties

cache/
---------------------------

files/
---------------------------

The `files/` directory is the actual repository. It contains 256 sub directories (named `00`, `01`, `02`, ..., `fe`, `ff`) each of which also contains 256 sub directories (named `00`, `01`, `02`, ..., `fe`, `ff`).

index/
---------------------------

locks/
---------------------------

Contains lock files during the runtime of a backup or garbage collection run.
Locks a necessary because garbage collection requires exclusive access to the repository. Backups and restoring files from a backup may be performed concurrently.

repo.properties
---------------------------

This file contains configuration concerning the repository as a whole: It defines which data compression and encryption algorithms to use. These settings must be set *before* any backups are performed and may not be altered afterwards.
The file format is that of a standard Java(tm) properties file with the following keys:

	compression = Deflate
	encrytion = Blowfish
	digest = SHA1

Tree index files
...........................
