#include <linux/version.h>

/*

In case if the upstream api changed or it's signature change, please use the *NEW* api in the file that calls it.
e.g. in 4.19 netif_is_ip6gretap got changed to is_ip6gretap_dev in 5.15
replace all the instances with is_ip6gretap_dev in the xyz.c file and then in this file under 4.19 section create macro as below 
#define netif_is_ip6gretap is_ip6gretap_dev

The goal is when it comes to decommissioning 4.19, we can just empty out this file.
This way we reduce the clutter in the xyz.c file like below

xyc.c file 
if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
    netif_is_ip6gretap(a,b,c)
else
    is_ip6gretap_dev(a,b,c)
endif

Similar thing can also be done if the function signature has changed, as done below here where acess_ok was changed from 3 parameters
in 4.19 to 2 in 5.15


*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))

#define ACCESS_OK(a, b, c) access_ok(a, b,c )

#define DEV_CHANGE_FLAGS(a, b) dev_change_flags(a, b);

#define IOREMAP ioremap_nocache 

#define add_cpu cpu_up 
#define remove_cpu cpu_down 

#define netif_is_ip6gretap is_ip6gretap_dev
#define netif_is_gretap is_gretap_dev 

#define redirected tc_redirected
#define from_ingress tc_from_ingress

#define __state state


#define device_create_managed_software_node(a,b,c)  platform_device_add_properties(a, b)

#else //#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))

#define DEV_CHANGE_FLAGS(a, b) dev_change_flags(a, b, NULL);

#define ACCESS_OK(a, b, c)  access_ok(b,c)

#define IOREMAP ioremap

#define secpath_put(a)
#define secpath_get(a)

#endif //#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))


