etherstate = "<% sysinfo("ethernet"); %>";
rtkswitch = <% sysinfo("ethernet.rtk"); %>;
hndswitch = <% get_wan_lan_status(); %>;

wlc_0_arr = ["<% sysinfo("conn.wifi.0.assoc"); %>", "<% sysinfo("conn.wifi.0.autho"); %>", "<% sysinfo("conn.wifi.0.authe"); %>"];
wlc_1_arr = ["<% sysinfo("conn.wifi.1.assoc"); %>", "<% sysinfo("conn.wifi.1.autho"); %>", "<% sysinfo("conn.wifi.1.authe"); %>"];
wlc_2_arr = ["<% sysinfo("conn.wifi.2.assoc"); %>", "<% sysinfo("conn.wifi.2.autho"); %>", "<% sysinfo("conn.wifi.2.authe"); %>"];
wlc_3_arr = ["<% sysinfo("conn.wifi.3.assoc"); %>", "<% sysinfo("conn.wifi.3.autho"); %>", "<% sysinfo("conn.wifi.3.authe"); %>"];

conn_stats_arr = ["<% sysinfo("conn.total"); %>","<% sysinfo("conn.active"); %>"];

mem_stats_arr = ["<% sysinfo("memory.total"); %>",  "<% sysinfo("memory.free"); %>", "<% sysinfo("memory.buffer"); %>", 
                 "<% sysinfo("memory.cache"); %>", "<% sysinfo("memory.swap.used"); %>", "<% sysinfo("memory.swap.total"); %>",
	         "<% sysinfo("nvram.used"); %>", "<% sysinfo("jffs.usage"); %>"];

cpu_stats_arr = ["<% sysinfo("cpu.load.1"); %>", "<% sysinfo("cpu.load.5"); %>", "<% sysinfo("cpu.load.15"); %>"];
