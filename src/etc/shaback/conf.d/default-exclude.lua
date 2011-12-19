-- Logs
addExcludePattern('/var/log/dmesg*')
addExcludePattern('/var/log/debug')
addExcludePattern('/var/log/aptitude')
addExcludePattern('/var/log/lastlog')
addExcludePattern('/var/log/messages')
addExcludePattern('/var/log/mail.*')
addExcludePattern('/var/log/Xorg.*')
addExcludePattern('/var/log/syslog')
addExcludePattern('/var/log/*.gz')
addExcludePattern('/var/log/*.log*')

-- Temp
addExcludePattern('/tmp/*')
addExcludePattern('/home/*/tmp/*')
addExcludePattern('*.tmp')
addExcludePattern('/lost+found')
addExcludePattern('/var/tmp/*')
addExcludePattern('/var/run/*.pid')

-- Thumbnails
addExcludePattern('*/.thumbnails')
addExcludePattern('*/Thumbs.db')
addExcludePattern('*/.xvpics')

-- Cache
addExcludePattern('*/.mozilla/*/Cache/*')

-- Debian
addExcludePattern('/var/cache/apt/archives/*.deb')
addExcludePattern('/var/cache/apt/archives/partial/*')
addExcludePattern('/ia32/var/cache/apt/archives/*.deb')
addExcludePattern('/ia32/var/cache/apt/archives/partial/*')

-- Cache files
addExcludePattern('/var/spool/shaback/*')
