Synopsis
===========================

Shaback is a backup tool for unixish server and client systems. Shaback stores all data in a designated repository directory. Each repository can be used to store backups of an arbitrary number of hosts and file sets. The way the data is stored ensures that storing duplicates is strictly avoided and each file content is stored only once.

How it works
===========================

The Repository
---------------------------

The repository is a directory with the following structure:
  <repository>
    cache/
    files/
      00/00
      …
      ff/ff
    index/
    locks/
    repo.properties

files/
____________

The files directory is the actual repository. It contains 256 sub directories (named 00, 01, 02, …, ff) each of which also contains 256 sub directories (named 00, 01, 02, …, ff).
