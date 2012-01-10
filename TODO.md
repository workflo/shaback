Restore
-----------------------
- Search for different versions of a given file (by name)
- Build Knoppix with shaback

Backup
-----------------------
- Pinuts:
  - lock/unlock
  - Split large files
    - Research on VMDK
  - ACL
  - Write report file after backup

Garbage collection
-----------------------
- FSCK
- Cleanup (delete old index root files)

Funny optimizations
-----------------------
- Sort dir entries by inode

Config
-----------------------
- Allow different hashing algorithms (SHA-1, SHA-256)
- Allow different encryption algorithms (Twofish, AES)
- Lua:
  - Determine hostname
  - system()
  - postBackup hook: write report, rsync repository, send email etc.
  - backupError hook
  - send email

Restore GUI
-----------------------
- Have a ncurses based UI
  - Local cache for tree files