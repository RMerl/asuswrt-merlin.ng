#define ADSL_BASE_ADDR_STR		"adsl"
#define PARAM1_BASE_ADDR_STR		"rdp1"
#define PARAM2_BASE_ADDR_STR		"rdp2"
#define DHD_BASE_ADDR_STR		"dhd0"
#define DHD_BASE_ADDR_STR_1		"dhd1"
#define DHD_BASE_ADDR_STR_2		"dhd2"
#define PLC_BASE_ADDR_STR		"plc"
#define OPTEE_BASE_ADDR_STR		"optee"
#define TZIOC_BASE_ADDR_STR		"tzioc"
#define CMA_BASE_ADDR_STR		"cma"
#define CMA_PAD_BASE_ADDR_STR		"pad0"
#define BUFMEM_BASE_ADDR_STR		"bufmem"
#define RNRMEM_BASE_ADDR_STR		"rnrmem"
#define B15_MEGA_BARRIER		"b15_mega_br"
#define DT_RSVD_PREFIX_STR		"dt_reserved_"
#define DT_RSVD_NODE_STR		"reserved-memory"
#define DT_CMA_CACHED_NODE_STR		"plat_rsvmem_cached_device"
#define DT_CMA_UNCACHED_NODE_STR		"plat_rsvmem_uncached_device"
#define DT_CMA_RSVSIZE_PROP_STR		"rsvd-size"
#define DT_ROOT_NODE			"/"
#define DT_MEMORY_NODE			"memory"
#define OF_NODE_ADDR_CELLS_DEFAULT	0x2
#define OF_NODE_SIZE_CELLS_DEFAULT	0x1

#define DT_CHOSEN_NODE			"chosen"

#define DT_BOOTARGS_PROP		"bootargs"
#define DT_BOOTARGS_MAX_SIZE		1024



int dtb_set_reserved_memory(void *dtb_ptr, char* name, uint64_t addr, uint64_t size);
int dtb_del_cma_rsvmem_device(void *dtb_ptr);
const void *dtb_get_prop(void *dtb_ptr, const char *node_path,
	const char *property, int *len);
int dtb_getprop_reg(void *dtb_ptr,
			const char* node_name_par,
			const char *node_name,
			uint64_t* addr,
			uint64_t* size);
int dtb_getprop_cma_rsv_param(void *dtb_ptr, const char *node_suffix, const char *name, uint64_t* param);
int dtb_getprop_cma_rsvmem_size(void *dtb_ptr, const char *name, uint64_t* size);
int dtb_setprop_cma_rsvmem_size(char* dtb_ptr, const char *name, uint64_t size);
int dtb_del_cma_rsvmem(void* dtb_ptr, const char *name);
int dtb_del_reserved_memory(void* dtb_ptr, char* name);
int dtb_set_bootargs(void *fdt, char* bootargs, int append);
