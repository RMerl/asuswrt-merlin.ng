#include "nfssdk.h"

#if defined(RTAX89U) || defined(PLAX56_XP4)
typedef a_ulong_t a_uint32_64_t;
#else 
typedef a_uint32_t a_uint32_64_t;
#endif

a_uint32_64_t *ioctl_buff = NULL;

ssdk_init_cfg init_cfg = def_init_cfg;
ssdk_cfg_t ssdk_cfg;
static a_uint32_t flag = 0;

char dev_id_path[] = "/sys/ssdk/dev_id";
static int sw_devid = 0;

int get_devid(void)
{
    return sw_devid;
}

int set_devid(int dev_id)
{
	sw_devid = dev_id;
	return SW_OK;
}

static sw_error_t cmd_socket_init()
{
    sw_error_t rv;

    init_cfg.cpu_mode = HSL_CPU_1;
    init_cfg.reg_mode = HSL_MDIO;
#if defined UK_MINOR_DEV
    init_cfg.nl_prot  = UK_MINOR_DEV;
#else
    init_cfg.nl_prot  = 30;
#endif
    init_cfg.chip_type=CHIP_UNSPECIFIED;
    init_cfg.reg_func.mdio_set = NULL;
    init_cfg.reg_func.mdio_get = NULL;

    rv = ssdk_init(0, &init_cfg);

    if (flag == 0) {
        memset(&ssdk_cfg, 0 ,sizeof(ssdk_cfg_t));
        rv = sw_uk_exec(SW_API_SSDK_CFG, 0, &ssdk_cfg);
        flag = 1;
    }
    return rv;
}

sw_error_t cmd_init(void)
{
    FILE *fp = NULL;
    int dev_id_value;

    ioctl_buff = (a_uint32_64_t *)malloc(IOCTL_BUF_SIZE);

    if((fp = fopen(dev_id_path, "r")) != NULL) {
        fscanf(fp, "%d", &dev_id_value);
        set_devid(dev_id_value);
        fclose(fp);
    } else {
        set_devid(0);
    }
    cmd_socket_init();

    return SW_OK;
}

sw_error_t cmd_exit(void)
{
    free(ioctl_buff);
    ssdk_cleanup();
    flag = 0;

    return SW_OK;
}

static sw_error_t
cmd_api_func(sw_api_func_t *fp, a_uint32_t nr_param, a_uint32_64_t * args)
{
    a_uint32_64_t *p = &args[2];
    sw_error_t rv;
    sw_error_t(*func) ();

    func = fp->func;

    switch (nr_param)
    {
        case 0:
            rv = (func) ();
            break;
        case 1:
            rv = (func) (p[0]);
            break;
        case 2:
            rv = (func) (p[0], p[1]);
            break;
        case 3:
            rv = (func) (p[0], p[1], p[2]);
            break;
        case 4:
            rv = (func) (p[0], p[1], p[2], p[3]);
            break;
        case 5:
            rv = (func) (p[0], p[1], p[2], p[3], p[4]);
            break;
        case 6:
            rv = (func) (p[0], p[1], p[2], p[3], p[4], p[5]);
            break;
        case 7:
            rv = (func) (p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
            break;
        case 8:
            rv = (func) (p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
            break;
        case 9:
            rv = (func) (p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]);
            break;
        case 10:
            rv = (func) (p[0], p[1], p[2], p[3], p[4], p[5],
                         p[6], p[7], p[8], p[9]);
            break;
        default:
            rv = SW_OUT_OF_RANGE;
    }

    *(a_uint32_64_t *) args[1] = rv;

    return rv;
}

sw_error_t
cmd_exec_api(a_uint32_64_t *arg_val) //CT8 is a_uint32_t
{
    sw_error_t rv;
    sw_api_t sw_api;

    sw_api.api_id = arg_val[0];

    rv = sw_api_get(&sw_api);
    if (rv != SW_OK) 
        return rv;

    /*save cmd return value */
    arg_val[1] = (a_uint32_64_t) ioctl_buff;
    /*save set device id */
    arg_val[2] = arg_val[2]; //get_devid();

    rv = cmd_api_func(sw_api.api_fp, sw_api.api_nr, arg_val);
    if (rv != SW_OK) 
        return rv;

    //rv = cmd_api_output(sw_api.api_pp, sw_api.api_nr, arg_val);
    //SW_RTN_ON_ERROR(rv);

    return rv;
}

#if defined(RTAX89U)
static int rtax89u_get_port(int p0, int p1)
{
    int port;
    switch (p0) {
    case 1:
        switch (p1) {
        case 1:
            port = 7;
            break;
        case 2:
            port = 6;
            break;
        case 3:
            port = 5;
            break;
        case 4:
            port = 4;
            break;
        case 5:
            port = 3;
            break;
        case 6:
            port = 8;
            break;
        default:
            port = 0;
            break;
        }
        break;
    case 2:
        port = 2;
        break;
    case 3:
        port = 1;
        break;
    case 4:
        port = 0; //wan
        break;
    case 5:
        port = 5; // 10G SFP+
        break;
    case 6:
        port = 6; // 10G RJ45
        break;
    default:
        port = 0;
        break;
    }

    return port;
}
#endif // defined(RTAX89U)

static int mac_is_empty(unsigned char *mac)
{
    char macstr[18];
    char null_mac[18] = "00:00:00:00:00:00";

    if (!memcmp(mac2str(mac, macstr), null_mac, 18)) 
        return 1;
    else 
        return 0;
}

#if defined(RTAX89U)
int qca_handle_mac_table(ethsw_mac_table *e0, ethsw_mac_table *e1, ethsw_mac_table *tbl)
{
    int i, j;
    int cnt = 0;

    for (i=0; i < e0->count; i++) {
        if (mac_is_empty(e0->entry[i].mac))
            continue;

        if (e0->entry[i].port >= 4)
            continue;

        for (j=0; j < e1->count; j++) {
            if (mac_is_empty(e1->entry[j].mac))
                continue;

            if (!memcmp(e0->entry[i].mac, e1->entry[j].mac, ETHER_ADDR_LEN)) {
                tbl->entry[cnt].vid = e1->entry[j].vid;
                tbl->entry[cnt].port = rtax89u_get_port(e0->entry[i].port, e1->entry[j].port);
                memcpy(tbl->entry[cnt].mac, e1->entry[j].mac, ETHER_ADDR_LEN);
                cnt++;
                break;
            }
        }
    }

    tbl->count = cnt;
    tbl->len = tbl->count * sizeof(ethsw_mac_entry) + 8;

    return 0;
}
#endif

int qca_fdb_mac_table_get(ethsw_mac_table *tbl)
{
    a_uint32_64_t arg_val[5];
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_fdb_op_t *fdb_op;
    ethsw_mac_table *e0, *e1, t0, t1;

    cmd_init();

    fdb_op = (fal_fdb_op_t *)   (ioctl_buff + sizeof(sw_error_t)/4);
    e0     = (ethsw_mac_table *)(ioctl_buff + sizeof(sw_error_t)/4 + sizeof(fal_fdb_op_t)/4);

    memset(fdb_op, 0, sizeof(fal_fdb_op_t));
    memset(e0,     0, sizeof(ethsw_mac_table));

    arg_val[0] = SW_API_FDB_EXT_MAC_TABLE;
    arg_val[1] = (a_uint32_64_t) ioctl_buff;
    arg_val[2] = get_devid();
    arg_val[3] = (a_uint32_64_t) fdb_op;
    arg_val[4] = (a_uint32_64_t) e0;

    rtn = cmd_exec_api(arg_val);

    if((rtn != SW_OK) && (rtn != SW_NO_MORE)) {
        printf("error");
        cmd_exit();
        return 0;
    }

    memcpy(&t0, e0, sizeof(ethsw_mac_table));

#if defined(RTAX89U)
    fdb_op = (fal_fdb_op_t *)   (ioctl_buff + sizeof(sw_error_t)/4);
    e1     = (ethsw_mac_table *)(ioctl_buff + sizeof(sw_error_t)/4 + sizeof(fal_fdb_op_t)/4);

    memset(fdb_op, 0, sizeof(fal_fdb_op_t));
    memset(e1,     0, sizeof(ethsw_mac_table));

    arg_val[0] = SW_API_FDB_EXT_MAC_TABLE;
    arg_val[1] = (a_uint32_64_t) ioctl_buff;
    arg_val[2] = 1;
    arg_val[3] = (a_uint32_64_t) fdb_op;
    arg_val[4] = (a_uint32_64_t) e1;

    rtn = cmd_exec_api(arg_val);

    if((rtn != SW_OK) && (rtn != SW_NO_MORE)) {
        printf("error");
        cmd_exit();
        return 0;
    }

    memcpy(&t1, e1, sizeof(ethsw_mac_table));

    qca_handle_mac_table(&t0, &t1, tbl);
#else // CT8
    memcpy(tbl, &t0, sizeof(ethsw_mac_table));
#endif

    cmd_exit();

    return 0;
}
