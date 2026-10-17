/bin/mkdir /data/root
/bin/mkdir /data/root/.ssh
/bin/cp /data/authorized_keys /data/root/.ssh
/bin/chmod 700 /data/root/.ssh
/bin/chmod 700 /data/root
/bin/cp /data/passwd /etc
/bin/cp /data/passwd /var
/bin/sshd -B -a &
