If tc release one firmware, please file to tc_fw folder.

When release one asus bin file, please use md5 to have it hash value for checking version.
Please write the info to tc_fw_history.txt

001 = the asus generated bin file number

---

click CmdLineMode.exe

type the following command.

1. get md5 number

2. modify tc_fw_history.txt

3. tcfw_packer asus_tc.bin ras DSL 001

(or 002, 003, ....)

4. modify tc_ver.h