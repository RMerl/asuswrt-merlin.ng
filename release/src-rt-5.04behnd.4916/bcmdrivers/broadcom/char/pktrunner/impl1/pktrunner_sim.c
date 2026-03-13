#include <rdpa_api.h>

extern bdmf_object_handle ip_class;

static int runner_ip_class_tcp_ack_prio_set(uint32_t enable)
{
    int rc = rdpa_ip_class_tcp_ack_prio_set(ip_class, enable);
    return rc;
}

static int runner_ip_class_tos_mflows_set(uint32_t enable)
{
    int rc = rdpa_ip_class_tos_mflows_set(ip_class, enable);
    return rc;
}

int __init runnerHost_construct(void)
{
    runner_ip_class_tcp_ack_prio_set(1);
    runner_ip_class_tos_mflows_set(1);

    return 0;
}

void __exit runnerHost_destruct(void)
{
}
