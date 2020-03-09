curr_coreTmp_2_raw = "<% sysinfo("temperature.2"); %>";
curr_coreTmp_2 = (curr_coreTmp_2_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_2_raw.replace("&deg;C", ""));

curr_coreTmp_5_raw = "<% sysinfo("temperature.5"); %>";
curr_coreTmp_5 = (curr_coreTmp_5_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_5_raw.replace("&deg;C", ""));

curr_coreTmp_52_raw = "<% sysinfo("temperature.52"); %>";
curr_coreTmp_52 = (curr_coreTmp_52_raw.indexOf("disabled") > 0 ? 0 : curr_coreTmp_52_raw.replace("&deg;C", ""));

curr_cpuTemp = "<% get_cpu_temperature(); %>";
fanctrl_info = "<% get_fanctrl_info(); %>";
