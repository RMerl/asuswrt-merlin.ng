/^debugfs [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
/^dumpe2fs [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
/^e2fsck [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
/^mke2fs [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
/^resize2fs [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
/^tune2fs [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
/^e2image [1-9]\.[0-9]*[^ ]* ([0-9]*-[A-Za-z]*-[0-9]*)/d
s/\\015//g
/automatically checked/d
/^Directory Hash Seed:/d
/Discarding device blocks/d
/^Filesystem created:/d
/^Filesystem flags:/d
/^Filesystem UUID:/d
/^JFS DEBUG:/d
/^Last write time:/d
/^Last mount time:/d
/^Last checked:/d
/^Lifetime writes:/d
/^Maximum mount count:/d
/^Next check after:/d
/^Suggestion:/d
/Reserved blocks uid:/s/ (user .*)//
/Reserved blocks gid:/s/ (group .*)//
/whichever comes first/d
/^  Checksum /d
s/, csum 0x\([0-9a-f]*\)//g
s/ csum 0x\([0-9a-f]*\)//g
/^Checksum:/d
s/while trying to open [^ ]*/while trying to open test.img/
s/he filesystem on [^ ]* /he filesystem on test.img /
s/^[^ ]* contains a \([a-z]*\) file system /test.img contains a \1 file system /
s/MMP block [0-9]* from [^ ]*/MMP block from test.img/
s/safe to mount '.*', MMP/safe to mount 'test.img', MMP/
s/mmp_device_name: .*/mmp_device_name: test.img/
s/mmp_node_name: .*/mmp_node_name: test_node/
s/mmp_update_date: .*/mmp_update_date: test date/
s/mmp_update_time: .*/mmp_update_time: test_time/
s/MMP last updated by '.*' on .*/MMP last updated by 'test_node' on test date/
s/MMP update by '.*' at .*/MMP last updated by 'test_node' on test date/
