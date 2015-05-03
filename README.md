Welcome to shaback 2.

Shaback is a command line backup and recovery tool for Linux and other POSIX compatible systems.
Shaback stores all its backup sets in a single repository using a hashing algorithm (SHA1) to "de-duplicate" file contents and even directory meta data. This repository can be either local (like an external disk) or a mounted file share (like NFS, CIFS or SSHFS). Caching of meta data ensures a decent backup performance even on remote repos.

A local repo can easily been mirrored by rsync to maintain an off-site backup.

Multiple machines can be backed up into one repo to benefit from a maximum of de-deplication.

A garbage collector makes sure that old backups will be deleted and unneeded data will be removed from the repo.

See Wiki for documentation: [Wiki on github](https://github.com/workflo/shaback/wiki)
