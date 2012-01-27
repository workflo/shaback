Restore
-----------------------
- Have a ncurses based UI
  - Local cache for tree files
- Search for different versions of a given file (by name)
- Build Knoppix with shaback

Backup
-----------------------
- Automatically exclude cache file and repository from backup
- also backup files and directories specified on the command line
- Pinuts:
  - Split large files
    - Research on VMDK
  - ACL
  - Write report file after backup

Garbage collection
-----------------------
- FSCK
- Cleanup (delete old index root files)

Config
-----------------------
- Allow different encryption algorithms (Twofish, AES)
- Allow different hashing algorithms (SHA-1, SHA-256)
- Lua:
  - Determine hostname
  - system()
  - postBackup hook: write report, rsync repository, send email etc.
  - backupError hook
  - send email

Meta
-----------------------
- Have Debian packages

Funny optimizations
-----------------------
- Backup: Sort dir entries by inode
- Backup: Store smaller files within tree file
- Restore files in the order of their storage directories (i.e. SHA-1 digests)
