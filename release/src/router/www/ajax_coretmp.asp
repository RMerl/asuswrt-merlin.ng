curr_coreTmp_wl0_raw = "<% sysinfo("temperature.0"); %>";
curr_coreTmp_wl0 = (curr_coreTmp_wl0_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_wl0_raw.replace("&deg;C", ""));

curr_coreTmp_wl1_raw = "<% sysinfo("temperature.1"); %>";
curr_coreTmp_wl1 = (curr_coreTmp_wl1_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_wl1_raw.replace("&deg;C", ""));

curr_coreTmp_wl2_raw = "<% sysinfo("temperature.2"); %>";
curr_coreTmp_wl2 = (curr_coreTmp_wl2_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_wl2_raw.replace("&deg;C", ""));

curr_coreTmp_wl3_raw = "<% sysinfo("temperature.3"); %>";
curr_coreTmp_wl3 = (curr_coreTmp_wl3_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_wl3_raw.replace("&deg;C", ""));

curr_cpuTemp = "<% get_cpu_temperature(); %>";
fanctrl_info = "<% get_fanctrl_info(); %>";
