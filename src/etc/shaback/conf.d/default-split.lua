-- VmWare
addSplitPattern('*.vmdk');

-- Oracle
addSplitPattern('*.dbf')
addSplitPattern('*/oradata/*.log')

-- MySQL
addSplitPattern('*.MYI')
addSplitPattern('*.MYD')
addSplitPattern('*/ibdata*')
