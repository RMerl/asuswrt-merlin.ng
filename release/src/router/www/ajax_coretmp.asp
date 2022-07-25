curr_coreTmp_0_raw = "<% sysinfo("temperature.0"); %>";
curr_coreTmp_0 = (curr_coreTmp_0_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_0_raw.replace("&deg;C", ""));

curr_coreTmp_1_raw = "<% sysinfo("temperature.1"); %>";
curr_coreTmp_1 = (curr_coreTmp_1_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_1_raw.replace("&deg;C", ""));

curr_coreTmp_2_raw = "<% sysinfo("temperature.2"); %>";
curr_coreTmp_2 = (curr_coreTmp_2_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_2_raw.replace("&deg;C", ""));

curr_coreTmp_3_raw = "<% sysinfo("temperature.3"); %>";
curr_coreTmp_3 = (curr_coreTmp_3_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_3_raw.replace("&deg;C", ""));

curr_cpuTemp = "<% get_cpu_temperature(); %>";
fanctrl_info = "<% get_fanctrl_info(); %>";
