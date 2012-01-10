Synopsis
===========================

Shaback is a backup tool for unixish server and client systems. Shaback stores all data in a designated repository directory. Each repository can be used to store backups of an arbitrary number of hosts and file sets. The way the data is stored ensures that storing duplicates is strictly avoided and each file content is stored only once.

How it's used
===========================

Create a repository
---------------------------

The first thing to do is create a repository where all data will be stored. A repository can be created on any harddrive or network file share mounted to your host.

        shaback init -r <REPO_DIR>

After the repository has been created, you are encouraged to have a look at the newly created file `<REPO_DIR>/repo.properties`. There you can chose whether to use data compression (Deflate) and/or encryption (Blowfish). This selection **must** be done before the first backup is actually done.

Config file
---------------------------

Shaback can be customized with config files written in [Lua|http://www.lua.org]. A file names `~/.shaback.lua` is loaded by default if it exists. You may also use the `--config=<file>` command line option to load further config files after that.

The following listing is a typical example of a simple `~/.shaback.lua` config file:

        repository('/mnt/backup-medium')
        localCache('/var/spool/shaback/cache.gdbm')

        oneFileSystem(true)

        addDir('/home')

        addExcludePattern('*~')
        addExcludePattern('**/*.tmp')

See [Config.md|Config.md] for a complete list of config options and their explanations.

Perform backups
---------------------------

If everything is configured correctly, all you need to do to start a backup run is:

        shaback backup

After successfully finishing the backup the name of the newly created index file is printed to stdout. You need this file to restore files from this very backup run later.

Recover from backup
---------------------------


Using data encryption
---------------------------


How it works
===========================

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

TODO
===========================

- Bootable Linux CD for desaster recovery (Knoppix)
- Text-based GUI (ncurses)
