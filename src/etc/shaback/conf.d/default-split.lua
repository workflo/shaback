-- General
addSplitPattern('*.log');

-- VmWare, VirtualBox
addSplitPattern('*.vmdk');
addSplitPattern('*.vdi');

-- Oracle
addSplitPattern('*.dbf')
addSplitPattern('*/oradata/*.log')

-- MySQL
addSplitPattern('*.MYI')
addSplitPattern('*.MYD')
addSplitPattern('*/ibdata*')
