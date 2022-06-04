#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cmocka.h>

#include "libubi.h"
#include "test_lib.h"


static void test_libubi_open(void **state)
{
	libubi_t lib = NULL;
	expect_open(SYSFS_ROOT "/class/ubi", O_RDONLY, 4);
	expect_close(4,0);
	expect_open(SYSFS_ROOT "/class/ubi/version", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);

	lib = libubi_open();
	assert_non_null(lib);
	libubi_close(lib);
	(void) state;
}

static void test_ubi_vol_block_create(void **state)
{
	int mock_fd = 1;
	expect_ioctl_short(UBI_IOCVOLCRBLK, 0);
	int r = ubi_vol_block_create(mock_fd);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_ubi_vol_block_remove(void **state)
{
	int mock_fd = 1;
	expect_ioctl_short(UBI_IOCVOLRMBLK, 0);
	int r = ubi_vol_block_remove(mock_fd);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_ubi_leb_unmap(void **state)
{
	int mock_fd = 1;
	int lnum = 12;
	expect_ioctl(UBI_IOCEBUNMAP, 0, &lnum);
	int r = ubi_leb_unmap(mock_fd, lnum);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_ubi_is_mapped(void **state)
{
	int mock_fd = 1;
	int lnum = 1;
	expect_ioctl(UBI_IOCEBISMAP, 0, &lnum);
	int r = ubi_is_mapped(mock_fd, lnum);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_ubi_update_start(void **state)
{
	int mock_fd = 1;
	long long bytes = 0x1234;

	expect_ioctl(UBI_IOCVOLUP, 0, &bytes);
	int r = ubi_update_start(NULL, mock_fd, bytes);
	assert_int_equal(r, 0);
	(void) state;
}

static libubi_t mock_libubi_open()
{
	expect_open(SYSFS_ROOT "/class/ubi", O_RDONLY, 4);
	expect_close(4,0);
	expect_open(SYSFS_ROOT "/class/ubi/version", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	libubi_t lib = libubi_open();
	assert_non_null(lib);
	return lib;
}

static void test_ubi_dev_present(void **state)
{
	libubi_t lib = mock_libubi_open();
	int r = ubi_dev_present(lib, 0);
	assert_int_equal(r, 1);

	libubi_close(lib);
	(void) state;
}

static void test_ubi_rsvol(void **state)
{
	const char *node = "/foo";
	int vol_id = 0;
	long long bytes = 0xadadaf;
	struct ubi_rsvol_req req;
	memset(&req, 0, sizeof(req));
	req.bytes = bytes;
	req.vol_id = vol_id;
	expect_open(node, O_RDONLY, 4);
	expect_ioctl(UBI_IOCRSVOL, 0, &req);
	expect_close(4, 0);
	int r = ubi_rsvol(NULL, node, vol_id, bytes);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_ubi_rnvols(void **state)
{
	libubi_t lib = mock_libubi_open();
	const char *node = "/foo";
	struct ubi_rnvol_req req;
	memset(&req, 0xaf, sizeof(req));
	expect_open(node, O_RDONLY, 4);
	expect_ioctl(UBI_IOCRNVOL, 0, &req);
	expect_close(4, 0);
	int r = ubi_rnvols(lib, node, &req);
	assert_int_equal(r, 0);

	libubi_close(lib);
	(void) state;
}

static void test_ubi_rmvol(void **state)
{
	libubi_t lib = mock_libubi_open();
	const char *node = "/foo";
	int vol_id = 12;
	expect_open(node, O_RDONLY, 4);
	expect_ioctl(UBI_IOCRMVOL, 0, &vol_id);
	expect_close(4, 0);
	int r = ubi_rmvol(lib, node, vol_id);
	assert_int_equal(r, 0);

	libubi_close(lib);
	(void) state;
}

static void test_ubi_leb_change_start(void **state)
{
	libubi_t lib = mock_libubi_open();
	int mock_fd = 1;
	int lnum = 12;
	int bytes = 48;
	struct ubi_leb_change_req req;
	memset(&req, 0, sizeof(req));
	req.lnum = lnum;
	req.bytes = bytes;
	req.dtype = 3;
	expect_ioctl(UBI_IOCEBCH, 0, &req);
	int r = ubi_leb_change_start(lib, mock_fd, lnum, bytes);
	assert_int_equal(r, 0);

	libubi_close(lib);
	(void) state;
}

static void test_ubi_get_info(void **state)
{
	libubi_t lib = mock_libubi_open();
	struct ubi_info info;
	expect_open(SYSFS_ROOT "/class/misc/ubi_ctrl/dev", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_read(1,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/ubi/version", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	int r = ubi_get_info(lib, &info);
	assert_int_equal(r, 0);
	assert_int_equal(info.dev_count, 1);

	libubi_close(lib);
	(void) state;
}

static void test_ubi_mkvol(void **state)
{
	libubi_t lib = mock_libubi_open();
	const char *node = "/foo";
	const char *vol_name = "testvol";
	int vol_id = 12;
	struct ubi_mkvol_request req;
	struct ubi_mkvol_req rr;
	memset(&rr, 0, sizeof(rr));
	memset(&req, 0, sizeof(req));
	req.vol_id = vol_id;
	req.name = vol_name;
	rr.vol_id = vol_id;
	rr.name_len = strlen(vol_name);
	strncpy(rr.name, vol_name, UBI_MAX_VOLUME_NAME + 1);
	expect_open(node, O_RDONLY, 3);
	expect_ioctl(UBI_IOCMKVOL, 0, &rr);
	expect_close(3,0);
	int r = ubi_mkvol(lib, node, &req);
	assert_int_equal(r, 0);
	assert_int_equal(req.vol_id, vol_id);

	libubi_close(lib);
	(void) state;
}

void test_ubi_remove_dev(void **state)
{
	const char *node = "/foo";
	libubi_t lib = mock_libubi_open();
	int ubi_dev = 0xAA;
	expect_open(node, O_RDONLY, 4);
	expect_ioctl(UBI_IOCDET, 0, &ubi_dev);
	expect_close(4,0);
	int r = ubi_remove_dev(lib, node, ubi_dev);
	assert_int_equal(r, 0);

	libubi_close(lib);
	(void) state;
}

void test_ubi_attach(void **state)
{
	const char *node = "/foo";
	struct ubi_attach_request req;
	struct ubi_attach_req rr;
	memset(&req, 0, sizeof(req));
	memset(&rr, 0, sizeof(rr));
	libubi_t lib = mock_libubi_open();
	req.dev_num = 1;
	req.mtd_num = 1;
	rr.ubi_num = 1;
	rr.mtd_num = 1;
	expect_open(node, O_RDONLY, 4);
	expect_ioctl(UBI_IOCATT, 0, &rr);
	expect_close(4,0);

	int r = ubi_attach(lib, node, &req);
	assert_int_equal(r, 0);

	libubi_close(lib);
	(void) state;
}

void test_ubi_set_property(void **state)
{
	int mock_fd = 1;
	uint8_t prop = 0xad;
	uint64_t val = 0xaabbccdd;
	struct ubi_set_vol_prop_req req;
	memset(&req, 0, sizeof(req));
	req.property = prop;
	req.value = val;
	expect_ioctl(UBI_IOCSETVOLPROP, 0, &req);
	int r = ubi_set_property(mock_fd, prop, val);
	assert_int_equal(r,0);

	(void)state;
}

/* functions to test
 * ubi_get_vol_info
 * ubi_get_vol_info1
 * ubi_get_vol_info1_nm
 * ubi_get_dev_info1
 * ubi_get_dev_info
 * ubi_probe_node
 * ubi_detach_mtd
 * ubi_detach
 * mtd_num2ubi_dev
 */


int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_libubi_open),
		cmocka_unit_test(test_ubi_vol_block_create),
		cmocka_unit_test(test_ubi_vol_block_remove),
		cmocka_unit_test(test_ubi_update_start),
		cmocka_unit_test(test_ubi_dev_present),
		cmocka_unit_test(test_ubi_rsvol),
		cmocka_unit_test(test_ubi_rmvol),
		cmocka_unit_test(test_ubi_rnvols),
		cmocka_unit_test(test_ubi_leb_change_start),
		cmocka_unit_test(test_ubi_get_info),
		cmocka_unit_test(test_ubi_mkvol),
		cmocka_unit_test(test_ubi_leb_unmap),
		cmocka_unit_test(test_ubi_is_mapped),
		cmocka_unit_test(test_ubi_remove_dev),
		cmocka_unit_test(test_ubi_attach),
		cmocka_unit_test(test_ubi_set_property),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
