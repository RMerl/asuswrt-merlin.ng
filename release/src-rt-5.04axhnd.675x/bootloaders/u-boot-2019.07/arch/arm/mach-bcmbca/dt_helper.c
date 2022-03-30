#include <stdio.h>
#include <malloc.h>
#include <linux/libfdt.h>

#include "dt_helper.h"

static int dtb_setprop(void *fdt, const char *node_path, const char *property,
		uint32_t *val_array, int size)
{
	int offset = fdt_path_offset(fdt, node_path);

	if (offset == -FDT_ERR_NOTFOUND)
		return -1;

	return fdt_setprop(fdt, offset, property, val_array, size);
}

static const void *dtb_getprop(const void *fdt, const char *node_path,
                   const char *property, int *len)
{
	int offset = fdt_path_offset(fdt, node_path);

	if (offset == -FDT_ERR_NOTFOUND)
		return NULL;

	return fdt_getprop(fdt, offset, property, len);
}

static const int dtb_delnode(void *fdt, const char *node_path)
{
	int offset = fdt_path_offset(fdt, node_path);

	if (offset == -FDT_ERR_NOTFOUND)
		return -1;

	return fdt_del_node(fdt, offset);
}
static void dtb_get_addr_size_cells(const char* dtb,
					const char* node_path,
					uint32_t* addr_cell_sz,
					uint32_t* size_cell_sz)
{
	uint32_t* prop;
	int len;
	prop = (uint32_t*)dtb_getprop(dtb, node_path, "#address-cells", &len);
	*addr_cell_sz = prop? cpu_to_fdt32(*prop):OF_NODE_ADDR_CELLS_DEFAULT;
	prop = (uint32_t*)dtb_getprop(dtb, node_path, "#size-cells", &len);
	*size_cell_sz = prop? cpu_to_fdt32(*prop):OF_NODE_SIZE_CELLS_DEFAULT;
}

int dtb_set_reserved_memory(void *dtb_ptr, char* name, uint64_t addr, uint64_t size)
{
	const void* prop;
	char node_name[96];
	uint32_t propval[4];
	int len;
	uint32_t addr_cell_sz, size_cell_sz;
	uint64_t temp;

	if (!dtb_ptr) {
		return -1;
	}

	dtb_get_addr_size_cells(dtb_ptr,"/reserved-memory", &addr_cell_sz, &size_cell_sz);
	if (addr_cell_sz > 2 || size_cell_sz > 2) {
		printf("%s  Cells sizes are not supported %d %d \n", __func__, addr_cell_sz, size_cell_sz);
		return -1;
	}

	sprintf(node_name, "/reserved-memory/%s", name);

	/* assume address and size are 64 bit for 4908. need to read #address-cells and #size_cells
	determine the actual size */
	prop = dtb_getprop(dtb_ptr, node_name, "reg", &len);
	/* sanity check */
	if (prop &&  len == (size_cell_sz+addr_cell_sz)*sizeof(uint32_t) ) {
		if (addr_cell_sz == 1) {
			*propval = cpu_to_fdt32(addr);
		} else {
			temp = cpu_to_fdt64(addr);
			memcpy((unsigned char*)propval, (unsigned char*)&temp, sizeof(uint64_t));
		}

		if (size_cell_sz == 1) {
			*(propval+addr_cell_sz) = cpu_to_fdt32(size);
		} else {
			temp = cpu_to_fdt64(size);
		memcpy((unsigned char*)(propval+addr_cell_sz), (unsigned char*)&temp, sizeof(uint64_t));
		}
		/* setting only size of the memory e.g. size_cell of the 'reg' */
		memcpy((void*)prop, (char*)propval, len);
		dtb_setprop(dtb_ptr, node_name, "reg", (uint32_t*)prop, len);
		return 0;
	}
	printf("WARNING: Node's property %s is not defined\n",node_name);

	return -1;
}

int dtb_del_cma_rsvmem_device(void *dtb_ptr)
{
	char dt_node_name[64];
	int ret;

	if (!dtb_ptr) {
		return -1;
	}

	/* del the cache device node first */
	sprintf(dt_node_name, "/%s", DT_CMA_CACHED_NODE_STR);
	ret = dtb_delnode(dtb_ptr, dt_node_name);

	/* del the uncached device node */
	sprintf(dt_node_name, "/%s", DT_CMA_UNCACHED_NODE_STR);
	ret |= dtb_delnode(dtb_ptr, dt_node_name);

	return ret;
}


const void *dtb_get_prop(void *dtb_ptr, const char *node_path,
	const char *property, int *len)
{

	return dtb_getprop(dtb_ptr, node_path, property, len);
}


int dtb_getprop_reg(void *dtb_ptr,
			const char* node_name_par,
			const char *node_name,
			uint64_t* addr,
			uint64_t* size)
{
	uint32_t addr_cell_sz, size_cell_sz;
	const uint32_t* prop;
	int len;
	char node_path[96];
	uint64_t temp;

	if (!dtb_ptr) {
		return -1;
	}
	sprintf(node_path, "/%s", node_name_par);
	dtb_get_addr_size_cells(dtb_ptr, node_path, &addr_cell_sz, &size_cell_sz);
	if (addr_cell_sz > 2 || size_cell_sz > 2) {
		printf("%s  Cells sizes are not supported %d %d \n", __func__, addr_cell_sz, size_cell_sz);
		return -1;
	}
	sprintf(node_path, "/%s/%s", node_name_par, node_name);
	prop = (const uint32_t*)dtb_getprop(dtb_ptr, node_path, "reg", &len);
	if (prop && len == (size_cell_sz+addr_cell_sz)*sizeof(uint32_t) ) {
		if (size_cell_sz == 1) {
			*size = (uint64_t)fdt32_to_cpu(*(prop+addr_cell_sz));
		} else {
			memcpy((unsigned char*)&temp, (unsigned char*)(prop+addr_cell_sz), sizeof(uint64_t));
			*size = fdt64_to_cpu(temp);
		}

		if (addr_cell_sz == 1) {
			*addr = (uint64_t)fdt32_to_cpu(*prop);
		} else {
			memcpy((unsigned char*)&temp, (unsigned char*)prop, sizeof(uint64_t));
			*addr = fdt64_to_cpu(temp);
		}
		return 0;
	}
	return -1;
}

static int _dtb_getprop_cma_rsv_param(const void* dtb_ptr, char *node_name, const char *name, uint64_t* param)
{
    const uint32_t* prop;
    int len;

    prop = (const uint32_t*)dtb_getprop(dtb_ptr, node_name, name, &len);
    if (prop && len == sizeof(uint32_t) ) {
        *param = (uint64_t)fdt32_to_cpu(*prop); 
        return 0;
    }

    return -1;
}

static int _dtb_getprop_cma_rsvmem_size(const void* dtb_ptr, char *node_name, uint64_t* size)
{
	const uint32_t* prop;
	int len;

	prop = (const uint32_t*)dtb_getprop(dtb_ptr, node_name, DT_CMA_RSVSIZE_PROP_STR, &len);
	if (prop && len == sizeof(uint32_t) ) {
		*size = (uint64_t)fdt32_to_cpu(*prop);
		return 0;
	}

	return -1;
}

static int _dtb_setprop_cma_rsvmem_size(void* dtb_ptr, char *node_name, uint64_t size)
{
	const uint32_t* prop;
	int len;
	uint32_t propval;

	prop = (const uint32_t*)dtb_getprop(dtb_ptr, node_name, DT_CMA_RSVSIZE_PROP_STR, &len);
	if (prop && len == sizeof(uint32_t) ) {
		propval = cpu_to_fdt32(size);

		memcpy((void*)prop, (char*)&propval, len);
		dtb_setprop(dtb_ptr, node_name, DT_CMA_RSVSIZE_PROP_STR, (uint32_t*)prop, len);

		return 0;
	}

	return -1;
}

static int  _dtb_del_cma_rsvmem(void* dtb_ptr, char *node_name)
{
	return dtb_delnode(dtb_ptr, node_name);
}

int dtb_getprop_cma_rsv_param(void *dtb_ptr, const char *node_suffix, const char *name, uint64_t* param)
{
	char dt_node_name[64];
	int ret;

	if (!dtb_ptr) {
		return -1;
	}

	/* try the cache node first */
	sprintf(dt_node_name, "/%s/%s%s", DT_CMA_CACHED_NODE_STR, DT_RSVD_PREFIX_STR, node_suffix);
	ret = _dtb_getprop_cma_rsv_param(dtb_ptr, dt_node_name, name, param);
	if (ret != 0) {
		/* try the uncached node */
		sprintf(dt_node_name, "/%s/%s%s", DT_CMA_UNCACHED_NODE_STR, DT_RSVD_PREFIX_STR, node_suffix);
		ret = _dtb_getprop_cma_rsv_param(dtb_ptr, dt_node_name, name, param);
	}

	return ret;
}

int dtb_getprop_cma_rsvmem_size(void *dtb_ptr, const char *name, uint64_t* size)
{
	return dtb_getprop_cma_rsv_param(dtb_ptr, name, DT_CMA_RSVSIZE_PROP_STR, size);
}


int dtb_setprop_cma_rsvmem_size(char* dtb_ptr, const char *name, uint64_t size)
{
	char dt_node_name[64];
	int ret;

	if (!dtb_ptr) {
		return -1;
	}

	/* try the cache node first */
	sprintf(dt_node_name, "/%s/%s%s", DT_CMA_CACHED_NODE_STR, DT_RSVD_PREFIX_STR, name);
	ret = _dtb_setprop_cma_rsvmem_size(dtb_ptr, dt_node_name, size);
	if (ret != 0) {
		/* try the uncached node */
		sprintf(dt_node_name, "/%s/%s%s", DT_CMA_UNCACHED_NODE_STR, DT_RSVD_PREFIX_STR, name);
		ret = _dtb_setprop_cma_rsvmem_size(dtb_ptr, dt_node_name, size);
	}

	return ret;
}

int dtb_del_cma_rsvmem(void* dtb_ptr, const char *name)
{
	char dt_node_name[64];
	int ret;

	if (!dtb_ptr) {
		return -1;
	}

	/* try the cache node first */
	sprintf(dt_node_name, "/%s/%s%s", DT_CMA_CACHED_NODE_STR, DT_RSVD_PREFIX_STR, name);
	ret = _dtb_del_cma_rsvmem(dtb_ptr, dt_node_name);
	if (ret != 0) {
		/* try the uncached node */
		sprintf(dt_node_name, "/%s/%s%s", DT_CMA_UNCACHED_NODE_STR, DT_RSVD_PREFIX_STR, name);
		ret = _dtb_del_cma_rsvmem(dtb_ptr, dt_node_name);
	}

	return ret;
}

int dtb_del_reserved_memory(void* dtb_ptr, char* name)
{
	char node_name[96];
	sprintf(node_name, "/reserved-memory/%s", name);
	return dtb_delnode(dtb_ptr, node_name);
}


int dtb_set_bootargs(void *fdt, char* bootargs, int append )
{
	const void *propdata =NULL;
	int nodeoffset, res = -1,proplen = 0;

	char *node = NULL,*prop=NULL,**err_msg = &prop, *new_bootargs = NULL;
	if (!fdt) {
		return res;
	}
	node = malloc(sizeof(DT_CHOSEN_NODE)+sizeof(DT_ROOT_NODE));
	if (!node) {
		goto err_out;
	}
	sprintf(node,"%s%s",DT_ROOT_NODE,DT_CHOSEN_NODE);
	nodeoffset = fdt_path_offset(fdt, node);
	if (nodeoffset == -FDT_ERR_NOTFOUND) {
		*err_msg = node;
		goto err_out;
	}
	prop = DT_BOOTARGS_PROP;
	propdata = fdt_getprop(fdt, nodeoffset, prop, &proplen);

	/* edit this to include mmc device root node in DT_BOOTARGS */
	new_bootargs = malloc(DT_BOOTARGS_MAX_SIZE);
	if (!new_bootargs) {
		goto err_out;
	}
	memset(new_bootargs, 0x0, DT_BOOTARGS_MAX_SIZE);
	if (append) {
		if (proplen+strlen(bootargs)+1 > DT_BOOTARGS_MAX_SIZE) {
			printf("Not enough space to append boot arg %s\n", bootargs);
			goto err_out;
		}

		if (propdata) {
			strncpy(new_bootargs, propdata, DT_BOOTARGS_MAX_SIZE);
		}
		strncat(new_bootargs, " ", DT_BOOTARGS_MAX_SIZE);
		strncat(new_bootargs, bootargs, DT_BOOTARGS_MAX_SIZE);
	} else {
		strncpy(new_bootargs, bootargs, DT_BOOTARGS_MAX_SIZE);
	}

	res = fdt_setprop_string(fdt, nodeoffset, prop, new_bootargs);
	if (res) {
		goto err_out;
	}

	res = 0;
err_out:
	free(node);
	if (new_bootargs) {
		free(new_bootargs);
	}
	if (res && *err_msg) {
		printf("Error accessing %s\n",*err_msg);
	}
	return res;
}
