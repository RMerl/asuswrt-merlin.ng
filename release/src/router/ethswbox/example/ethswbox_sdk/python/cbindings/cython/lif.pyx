""" Example cython interface definition """
from libc.stdint cimport (uint16_t, uint32_t, uint64_t,
                          int8_t, int16_t, int32_t, int64_t,uint8_t)

from cpython.ref cimport PyObject						  

cdef extern from "../../../src/lif/lif_api.h":

	int32_t lif_mdio_init(char* lib)
	int32_t lif_mdio_deinit(char* lib)
	int32_t lif_mdio_open(char* lib, uint8_t clk_pin, uint8_t data_pin)
	int32_t lif_mdio_close(char* lib, uint8_t clk_pin, uint8_t data_pin)
	int32_t lif_mdio_c22_read(uint8_t lif_id, uint8_t pad, uint8_t dad)
	int32_t lif_mdio_c22_write(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t val)
	int32_t lif_mdio_c45_read(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t reg)
	int32_t lif_mdio_c45_write(uint8_t lif_id, uint8_t pad, uint8_t dad, uint16_t reg, uint16_t val)
	int32_t lif_scan(char* lib)
	int32_t lif_get_cpin(uint8_t lif_id)
	int32_t lif_get_dpin(uint8_t lif_id)
	int32_t lif_get_nr_phys(uint8_t lif_id)
	int32_t lif_get_phy_addr(uint8_t lif_id, uint8_t phy)
	int32_t lif_get_phy_id(uint8_t lif_id, uint8_t phy)

	
def py_lif_mdio_init(lib):
	return lif_mdio_init(lib)

def py_lif_mdio_deinit(lib):
	return lif_mdio_deinit(lib) 

def py_lif_mdio_open(lib, clk_pin, data_pin):
	return lif_mdio_open(lib, clk_pin, data_pin)

def py_lif_mdio_close(lib, clk_pin, data_pin):
	return lif_mdio_close(lib, clk_pin, data_pin)

def py_lif_mdio_c22_read(lif_id, addr, regnum):
	return lif_mdio_c22_read( lif_id, addr, regnum)

def py_lif_mdio_c22_write(lif_id, addr, regnum, val):
	return lif_mdio_c22_write(lif_id, addr, regnum, val)

def py_lif_mdio_c45_read(lif_id, pad, dad, reg):
	return lif_mdio_c45_read(lif_id, pad, dad, reg)

def py_lif_mdio_c45_write(lif_id, pad, dad, reg, val):
	return lif_mdio_c45_write(lif_id, pad, dad, reg, val)

def py_lif_scan(lib):
	return lif_scan(lib)

def	py_lif_get_cpin(lif_id):
	return lif_get_cpin(lif_id)

def	py_lif_get_dpin(lif_id):
	return lif_get_dpin(lif_id)

def	py_lif_get_nr_phys(lif_id):
	return lif_get_nr_phys(lif_id)

def	py_lif_get_phy_addr(lif_id, addr):
	return lif_get_phy_addr(lif_id, addr)

def	py_lif_get_phy_id(lif_id, addr):
	return lif_get_phy_id(lif_id, addr)




# cdef extern from "../../../src/api/gpy/api_gpy.h":

# 	int32_t api_gpy_open (char* lib, uint8_t clk_pin, uint8_t data_pin)
# 	int32_t api_gpy_close (char* lib, uint8_t clk_pin, uint8_t data_pin)
# 	int32_t api_gpy_init (char* lib)

# 	void* api_gpy_create (char* lib, uint8_t clk_pin, uint8_t data_pin)


# 	int32_t api_gpy_internal_read(void *mdiobus_data, uint16_t addr, uint32_t regnum)
# 	int32_t api_gpy_internal_write(void *mdiobus_data, uint16_t addr, uint32_t regnum, uint16_t val)

# 	int32_t gpy_get_cpin (uint8_t lif_id)
# 	int32_t gpy_get_dpin (uint8_t lif_id)
# 	int32_t gpy_get_nr_phys (uint8_t lif_id)
# 	int32_t gpy_get_phy_addr (uint8_t lif_id ,uint8_t phy)
# 	int32_t gpy_get_phy_id (uint8_t lif_id,uint8_t phy)
# 	void* gpy_get_struc (uint8_t lif_id)

# 	int32_t api_gpy_scan (char* lib)
# 	int32_t api_gpy_update_pins (uint8_t lif_id)
# 	int32_t api_gpy_read (uint8_t lif_id, uint16_t addr, uint32_t regnum)
# 	int32_t api_gpy_write (uint8_t lif_id, uint16_t addr, uint32_t regnum, uint16_t val)



# def py_api_gpy_open(lib, clk_pin, data_pin):
# 	return api_gpy_open(lib, clk_pin, data_pin)
	
# def py_api_gpy_close(lib, clk_pin, data_pin):
# 	return api_gpy_close(lib, clk_pin, data_pin)
	
# def py_api_gpy_init(lib):
# 	return api_gpy_init(lib)

# def py_api_gpy_internal_read(lif, addr, regnum):
# 	return api_gpy_internal_read(<void *>lif, addr, regnum)

# def py_api_gpy_internal_write(lif, addr, regnum, val):
# 	return api_gpy_internal_write(<void *>lif, addr, regnum, val)

# def	py_gpy_get_cpin(lif_id):
# 	return gpy_get_cpin(lif_id)

# def	py_gpy_get_dpin(lif_id):
# 	return gpy_get_dpin(lif_id)

# def	py_gpy_get_nr_phys(lif_id):
# 	return gpy_get_nr_phys(lif_id)

# def	py_gpy_get_phy_addr(lif_id, addr):
# 	return gpy_get_phy_addr(lif_id, addr)

# def	py_gpy_get_phy_id(lif_id, addr):
# 	return gpy_get_phy_id(lif_id, addr)

# def py_api_gpy_scan(lib):
# 	return api_gpy_scan(lib)

# def py_api_gpy_update_pins(lif_id):
# 	return api_gpy_update_pins(lif_id)

# def py_api_gpy_read(lif_id, addr, regnum):
# 	return api_gpy_read(lif_id, addr, regnum)

# def py_api_gpy_write(lif_id, addr, regnum, val):
# 	return api_gpy_write(lif_id, addr, regnum, val)