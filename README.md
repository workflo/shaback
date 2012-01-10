Synopsis
===========================

Shaback is a backup tool for unixish server and client systems. Shaback stores all data in a designated repository directory. Each repository can be used to store backups of an arbitrary number of hosts and file sets. The way the data is stored ensures that storing duplicates is strictly avoided and each file content is stored only once.

How it's used
===========================

Shaback is a command line tool. Its integrated usage information can be viewed like this:

    shaback --help
    shaback init --help
    shaback backup --help

Create a repository
---------------------------

The first thing to do is create a repository where all data will be stored. A repository can be created on any harddrive or network file share mounted to your host.

    shaback init -r <REPO_DIR>

After the repository has been created, you are encouraged to have a look at the newly created file `<REPO_DIR>/repo.properties`. There you can chose whether to use data compression (Deflate) and/or encryption (Blowfish). This selection **must** be done before the first backup is actually done.

Config file
---------------------------

Shaback can be customized with config files written in [Lua](http://www.lua.org). A file named `~/.shaback.lua` is loaded by default if it exists. You may also use the `--config=<file>` command line option to load further config files after that.

The following listing is a typical example of a simple `~/.shaback.lua` config file:

    repository('/mnt/backup-medium')
    localCache('/var/spool/shaback/cache.gdbm')

    oneFileSystem(true)

    addDir('/home')

    addExcludePattern('*~')
    addExcludePattern('**/*.tmp')

See [CONFIG.md](https://github.com/workflo/shaback/blob/master/CONFIG.md) for a complete list of config options and their explanations.

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

See [DEVELOPER.md](https://github.com/workflo/shaback/blob/master/DEVELOPER.md)

TODO
===========================

- Bootable Linux CD for desaster recovery (Knoppix)
- Text-based GUI (ncurses)

See [TODO.md](https://github.com/workflo/shaback/blob/master/TODO.md) for a complete list of tasks and plans.
