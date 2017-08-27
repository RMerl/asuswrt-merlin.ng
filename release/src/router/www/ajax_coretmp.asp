fanctrl_info = [<% get_fanctrl_info(); %>];
cpuTemp = [<% get_cpu_temperature(); %>];
if(typeof cpuTemp[0] != "undefined")
	fanctrl_info = ["0", cpuTemp[0], cpuTemp[0], "0"];
curr_rxData = fanctrl_info[3];
curr_coreTmp_2 = fanctrl_info[1];
curr_coreTmp_5 = fanctrl_info[2];
curr_coreTmp_2 = (parseInt(curr_coreTmp_2)+parseInt(curr_coreTmp_5))*0.5;