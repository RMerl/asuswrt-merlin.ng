smtp
^220[\x09-\x0d -~]* (e?smtp|simple mail)
userspace pattern=^220[\x09-\x0d -~]* (E?SMTP|[Ss]imple [Mm]ail)
userspace flags=REG_NOSUB REG_EXTENDED
