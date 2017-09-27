# Welcome to shaback

## Incremental full backups with global de-duplication.

> As the old admin says: "Backup is not the problem, recovery is."

Shaback is a command line backup and recovery tool for Linux and other POSIX compatible systems. It tries to ease the pain on the day you need a good backup by simply providing a full recovery.

Shaback stores all its backup sets in a single repository using a hashing algorithm (SHA1) to "de-duplicate" partial file contents and even directory meta data. This repository can be either local (like an external disk) or a mounted file share (like NFS, CIFS or SSHFS). Caching of meta data ensures a decent backup performance even on remote repos.

A local repo can easily been mirrored by rsync to maintain an off-site backup.

Multiple machines can be backed up into one repo to benefit from a maximum of global de-duplication.

A garbage collector makes sure that old backups will be deleted and unneeded data will be removed from the repo.

Individual backup sets can be exported to a cpio archive to allow for easy recovery without having to install any special software.

# Features

* Incremental full backups
* Single pass recovery
* global de-duplication, even on file chunks
* optional compression
* optional encryption
* optional tiered repos

# How it feels like

* Initially backup your entire workstation (OS, user files) to a local, external disk; takes 1-2 hours
* Incrementally add changes, for example before shutdown; takes only some seconds or a few minutes
* Keep a history of backup sets, a couple of days, one for each week, month etc
* Copy your local repo to the team file server
* Let team members add their backup sets
* ...but each distro file (fragment) is stored only once

# For virtual machines

* Suspend them or shut them down for backup (consistent state)
* virtual disks compress about 50%
* virtual disks usually change 1-20% every day
* keep several days of consistent VM states
* ...but use even less storage than for a 1:1 copy

# More details

See Wiki for documentation: [Wiki on github](https://github.com/workflo/shaback/wiki)
