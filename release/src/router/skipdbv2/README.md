# skipdbv2
rewrite and fix for https://github.com/stevedekorte/skipdb
嵌入式 k/v 数据库，编译大小只有 300k。可以代替nvram，并有nvram更强大的功能。[详细说明](http://koolshare.cn/thread-4850-1-1.html)
为了在嵌入式下更少的占用jffs的空间。建议最多使用配置条目为1w。（merlin正常使用nvram的配置条目为2000条）
当jffs占用大于8M时，会进行一次refresh，重新计算空间。

# 速度
* 写15000条数据需要6s
* time ./test.sh #测试脚本在tests目录
* real	0m6.697s

# 实现功能有：

* dbus set key=value
* dbus ram key=value
* dbus replace key=value
* dbus get key
* dbus list key
* dbus delay key tick path_of_shell.sh #定时运行脚本,脚本情使用绝对路径
* dbus time key H:M:S path_of_shell.sh #绝对时间运行脚本,脚本情使用绝对路径
* dbus export key #将配置导入到脚本
* dbus update key #将脚本配置保存到数据库
* dbus inc key=value #增加数值
* dbus desc key=value #减去数值
* dbus event name path_of_shell.sh #注册一个事件脚本
* dbus fire name #触发一个事件脚本

# 编译方法
* 依赖库 libev，需要自行编译或安装
* cd skipdb
* mkdir build
* cd build
* cmake ..
* make

# 执行方法
* skipd -d /path/of/data
* dbus command params
