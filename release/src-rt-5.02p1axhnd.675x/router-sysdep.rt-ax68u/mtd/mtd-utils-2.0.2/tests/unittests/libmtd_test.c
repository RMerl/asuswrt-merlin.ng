#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cmocka.h>

#include "mtd/mtd-user.h"
#include "libmtd.h"
#include "libmtd_int.h"

#include "test_lib.h"

static libmtd_t mock_libmtd_open()
{
	/* create a mock object for libmtd, not using sysfs */
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/name", O_RDONLY, 4);
	expect_close(4,0);
	libmtd_t lib = libmtd_open();
	assert_non_null(lib);
	return lib;
}

static void test_libmtd_open(void **state)
{
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/name", O_RDONLY, 4);
	expect_close(4,0);
	struct libmtd *lib = libmtd_open();
	assert_non_null(lib);
	assert_string_equal(lib->sysfs_mtd, SYSFS_ROOT "/class/mtd");
	assert_string_equal(lib->mtd, SYSFS_ROOT "/class/mtd/mtd%d");
	assert_string_equal(lib->mtd_name, SYSFS_ROOT "/class/mtd/mtd%d/name");
	assert_string_equal(lib->mtd_dev, SYSFS_ROOT "/class/mtd/mtd%d/dev");
	assert_string_equal(lib->mtd_type, SYSFS_ROOT "/class/mtd/mtd%d/type");
	assert_string_equal(lib->mtd_eb_size, SYSFS_ROOT "/class/mtd/mtd%d/erasesize");
	assert_string_equal(lib->mtd_size, SYSFS_ROOT "/class/mtd/mtd%d/size");
	assert_string_equal(lib->mtd_min_io_size, SYSFS_ROOT "/class/mtd/mtd%d/writesize");
	assert_string_equal(lib->mtd_subpage_size, SYSFS_ROOT "/class/mtd/mtd%d/subpagesize");
	assert_string_equal(lib->mtd_oob_size, SYSFS_ROOT "/class/mtd/mtd%d/oobsize");
	assert_string_equal(lib->mtd_oobavail, SYSFS_ROOT "/class/mtd/mtd%d/oobavail");
	assert_string_equal(lib->mtd_region_cnt, SYSFS_ROOT "/class/mtd/mtd%d/numeraseregions");
	assert_string_equal(lib->mtd_flags, SYSFS_ROOT "/class/mtd/mtd%d/flags");

	libmtd_close(lib);
	(void) state;
}

static void test_mtd_dev_present(void **state)
{
	int ret;
	libmtd_t lib = mock_libmtd_open();
	ret = mtd_dev_present(lib, 0);
	assert_int_equal(ret, 1);
	libmtd_close(lib);
	(void) state;
}

static void test_mtd_mark_bad(void **state)
{
	struct mtd_dev_info mtd;
	loff_t seek;
	int eb = 12;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	seek = (loff_t)eb * mtd.eb_size;
	expect_ioctl(MEMSETBADBLOCK, 0, &seek);
	int r = mtd_mark_bad(&mtd, 4, eb);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_is_bad(void **state)
{
	struct mtd_dev_info mtd;
	loff_t seek;
	int eb = 0x42;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	seek = (loff_t)eb * mtd.eb_size;
	expect_ioctl(MEMGETBADBLOCK, 0, &seek);
	int r = mtd_is_bad(&mtd, 4, eb);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_lock(void **state)
{
	int eb = 0xBA;
	struct mtd_dev_info mtd;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	struct erase_info_user ei;
	memset(&ei, 0, sizeof(ei));
	ei.start = eb * mtd.eb_size;
	ei.length = mtd.eb_size;
	expect_ioctl(MEMLOCK, 0, &ei);
	int r = mtd_lock(&mtd, 4, eb);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_unlock(void **state)
{
	int eb = 0xBA;
	struct mtd_dev_info mtd;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	struct erase_info_user ei;
	memset(&ei, 0, sizeof(ei));
	ei.start = eb * mtd.eb_size;
	ei.length = mtd.eb_size;
	expect_ioctl(MEMUNLOCK, 0, &ei);
	int r = mtd_unlock(&mtd, 4, eb);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_is_locked(void **state)
{
	int eb = 0xBA;
	struct mtd_dev_info mtd;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	struct erase_info_user ei;
	memset(&ei, 0, sizeof(ei));
	ei.start = eb * mtd.eb_size;
	ei.length = mtd.eb_size;
	expect_ioctl(MEMISLOCKED, 0, &ei);
	int r = mtd_is_locked(&mtd, 4, eb);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_regioninfo(void **state)
{
	struct region_info_user req;
	struct region_info_user rr;
	memset(&req, 0, sizeof(req));
	memset(&rr, 0, sizeof(rr));
	int mock_fd = 4;
	int regidx = 0xAA;
	rr.regionindex = regidx;
	expect_ioctl(MEMGETREGIONINFO, 0, &rr);
	int r = mtd_regioninfo(mock_fd, regidx, &req);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_erase_multi(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	struct mtd_dev_info mtd;
	struct erase_info_user ei;
	struct erase_info_user64 ei64;
	int eb = 0x3C;
	int blocks = 3;
	memset(&ei, 0, sizeof(ei));
	memset(&ei64, 0, sizeof(ei64));
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	ei64.start = (uint64_t)eb * mtd.eb_size;
	ei64.length = (uint64_t)mtd.eb_size * blocks;
	ei.start = ei64.start;
	ei.length = ei64.length;
	/* non offs64 first */
	lib->offs64_ioctls = OFFS64_IOCTLS_NOT_SUPPORTED;
	expect_ioctl(MEMERASE, 0, &ei);
	int r = mtd_erase_multi(lib, &mtd, 4, eb, blocks);
	assert_int_equal(r, 0);

	lib->offs64_ioctls = OFFS64_IOCTLS_SUPPORTED;
	expect_ioctl(MEMERASE64, 0, &ei64);
	r = mtd_erase_multi(lib, &mtd, 4, eb, blocks);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void) state;
}

/* this is the same as above but with blocks == 1 and a
 * different function call.
 * libmtd is mapping mtd_erase to mtd_erase_multi with 1 block
 */
static void test_mtd_erase(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	struct mtd_dev_info mtd;
	struct erase_info_user ei;
	struct erase_info_user64 ei64;
	int eb = 0x3C;
	int blocks = 1;
	memset(&ei, 0, sizeof(ei));
	memset(&ei64, 0, sizeof(ei64));
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	ei64.start = (uint64_t)eb * mtd.eb_size;
	ei64.length = (uint64_t)mtd.eb_size * blocks;
	ei.start = ei64.start;
	ei.length = ei64.length;
	/* non offs64 first */
	lib->offs64_ioctls = OFFS64_IOCTLS_NOT_SUPPORTED;
	expect_ioctl(MEMERASE, 0, &ei);
	int r = mtd_erase(lib, &mtd, 4, eb);
	assert_int_equal(r, 0);

	lib->offs64_ioctls = OFFS64_IOCTLS_SUPPORTED;
	expect_ioctl(MEMERASE64, 0, &ei64);
	r = mtd_erase(lib, &mtd, 4, eb);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void) state;

}

static void test_mtd_read(void **state)
{
	int mock_fd = 4;
	int eb = 0xE0;
	int offs = 43;
	int len = 28;
	off_t seek;
	char buf[28];
	struct mtd_dev_info mtd;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	seek = (off_t)eb * mtd.eb_size + offs;
	expect_lseek(seek, SEEK_SET, seek);
	expect_read(len, len);
	int r = mtd_read(&mtd, mock_fd, eb, offs, &buf, len);
	assert_int_equal(r, 0);

	(void) state;
}

static void test_mtd_write_nooob(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	int mock_fd = 4;
	int eb = 0xE0;
	int offs = 64;
	int len = 64;
	off_t seek;
	char buf[64];
	memset(buf, 0xAA, len);
	struct mtd_dev_info mtd;
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	mtd.subpage_size = 64;
	seek = (off_t)eb * mtd.eb_size + offs;
	expect_lseek(seek, SEEK_SET, seek);
	expect_write(buf, len, len);
	int r = mtd_write(lib, &mtd, mock_fd, eb, offs, buf, len, NULL, 0, 0);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void)state;
}

static void test_mtd_write_withoob(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	int mock_fd = 4;
	int eb = 0xE0;
	int offs = 64;
	int len = 64;
	int oob_len = 64;
	uint8_t mode = 3;
	off_t seek;
	char buf[64], oob_data[64];
	struct mtd_dev_info mtd;
	struct mtd_write_req req;
	memset(buf, 0xAA, len);
	memset(oob_data, 0xBA, oob_len);
	memset(&mtd, 0, sizeof(mtd));
	memset(&req, 0, sizeof(req));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	mtd.subpage_size = 64;
	seek = (off_t)eb * mtd.eb_size + offs;
	req.start = seek;
	req.len = len;
	req.ooblen = oob_len;
	req.usr_data = (uint64_t)(unsigned long)buf;
	req.usr_oob = (uint64_t)(unsigned long)oob_data;
	req.mode = mode;
	expect_ioctl(MEMWRITE, 0, &req);
	int r = mtd_write(lib, &mtd, mock_fd, eb, offs, buf, len, oob_data, oob_len, mode);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void) state;
}

static void test_mtd_read_oob(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	struct mtd_dev_info mtd;
	struct mtd_oob_buf64 oob64;
	struct mtd_oob_buf oob;
	int mock_fd = 4;
	uint64_t start = 0, length = 64;
	char buf[64];
	memset(buf, 0xCD, 64);
	memset(&oob, 0, sizeof(oob));
	memset(&oob64, 0, sizeof(oob64));
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	mtd.subpage_size = 64;
	mtd.oob_size = 128;
	oob64.start = start;
	oob64.length = length;
	oob64.usr_ptr = (uint64_t)(unsigned long)buf;
	oob.start = oob64.start;
	oob.length = oob64.length;
	oob.ptr = buf;

	lib->offs64_ioctls = OFFS64_IOCTLS_NOT_SUPPORTED;
	expect_ioctl(MEMREADOOB, 0, &oob);
	int r = mtd_read_oob(lib, &mtd, mock_fd, start, length, buf);
	assert_int_equal(r, 0);

	lib->offs64_ioctls = OFFS64_IOCTLS_SUPPORTED;
	expect_ioctl(MEMREADOOB64, 0, &oob64);
	r = mtd_read_oob(lib, &mtd, mock_fd, start, length, buf);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void) state;
}

/* basically the same as above but write calls */
static void test_mtd_write_oob(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	struct mtd_dev_info mtd;
	struct mtd_oob_buf64 oob64;
	struct mtd_oob_buf oob;
	int mock_fd = 4;
	uint64_t start = 0, length = 64;
	char buf[64];
	memset(buf, 0xCD, 64);
	memset(&oob, 0, sizeof(oob));
	memset(&oob64, 0, sizeof(oob64));
	memset(&mtd, 0, sizeof(mtd));
	mtd.bb_allowed = 1;
	mtd.eb_cnt = 1024;
	mtd.eb_size = 128;
	mtd.subpage_size = 64;
	mtd.oob_size = 128;
	oob64.start = start;
	oob64.length = length;
	oob64.usr_ptr = (uint64_t)(unsigned long)buf;
	oob.start = oob64.start;
	oob.length = oob64.length;
	oob.ptr = buf;

	lib->offs64_ioctls = OFFS64_IOCTLS_NOT_SUPPORTED;
	expect_ioctl(MEMWRITEOOB, 0, &oob);
	int r = mtd_write_oob(lib, &mtd, mock_fd, start, length, buf);
	assert_int_equal(r, 0);

	lib->offs64_ioctls = OFFS64_IOCTLS_SUPPORTED;
	expect_ioctl(MEMWRITEOOB64, 0, &oob64);
	r = mtd_write_oob(lib, &mtd, mock_fd, start, length, buf);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void)state;
}

static void test_mtd_get_info(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	struct mtd_info info;
	memset(&info, 0, sizeof(info));
	int r = mtd_get_info(lib, &info);
	assert_int_equal(info.sysfs_supported, 1);
	assert_int_equal(info.highest_mtd_num, 0);
	assert_int_equal(info.lowest_mtd_num, 0);
	assert_int_equal(info.mtd_dev_cnt, 1);
	assert_int_equal(r, 0);

	libmtd_close(lib);
	(void)state;
}

static void test_mtd_get_dev_info1(void **state)
{
	struct libmtd *lib = mock_libmtd_open();
	struct mtd_dev_info info;
	int dev_num = 0;
	memset(&info, 0, sizeof(info));
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/dev", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_read(1,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/name", O_RDONLY, 0);
	expect_read_real(128,0);
	expect_read(1,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/type", O_RDONLY, 4);
	expect_read(65,0);
	expect_read(1,0);
	expect_close(4,0);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/erasesize", O_RDONLY, 0);
	expect_read_real(50, 0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/size", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/writesize", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/subpagesize", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/oobsize", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/oobavail", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/numeraseregions", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	expect_open(SYSFS_ROOT "/class/mtd/mtd0/flags", O_RDONLY, 0);
	expect_read_real(50,0);
	expect_close(3,1);
	int r = mtd_get_dev_info1(lib, dev_num, &info);
	assert_int_equal(r, 0);
	/* TODO check values */

	libmtd_close(lib);
	(void)state;
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_libmtd_open),
		cmocka_unit_test(test_mtd_is_bad),
		cmocka_unit_test(test_mtd_mark_bad),
		cmocka_unit_test(test_mtd_lock),
		cmocka_unit_test(test_mtd_unlock),
		cmocka_unit_test(test_mtd_is_locked),
		cmocka_unit_test(test_mtd_regioninfo),
		cmocka_unit_test(test_mtd_erase_multi),
		cmocka_unit_test(test_mtd_erase),
		cmocka_unit_test(test_mtd_read),
		cmocka_unit_test(test_mtd_write_nooob),
		cmocka_unit_test(test_mtd_write_withoob),
		cmocka_unit_test(test_mtd_read_oob),
		cmocka_unit_test(test_mtd_write_oob),
		cmocka_unit_test(test_mtd_dev_present),
		cmocka_unit_test(test_mtd_get_info),
		cmocka_unit_test(test_mtd_get_dev_info1),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
