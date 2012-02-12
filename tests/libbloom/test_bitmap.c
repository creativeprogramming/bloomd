#include <check.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "bitmap.h"

/*
bloom_bitmap *bitmap_from_file(int fileno, size_t len) {
*/

START_TEST(make_anonymous_bitmap)
{
    // Use -1 for anonymous
    bloom_bitmap map;
    int res = bitmap_from_file(-1, 4096, &map);
    fail_unless(res == 0);
}
END_TEST

START_TEST(make_bitmap_zero_size)
{
    bloom_bitmap map;
    int res = bitmap_from_file(-1, 0, &map);
    fail_unless(res == -EINVAL);
}
END_TEST

START_TEST(make_bitmap_bad_fileno)
{
    bloom_bitmap map;
    int res = bitmap_from_file(500, 4096, &map);
    fail_unless(res == -EBADF);
}
END_TEST

/*
bloom_bitmap *bitmap_from_filename(char* filename, size_t len, int create, int resize) {
*/

START_TEST(make_bitmap_nofile)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/asdf123", 4096, 0, 0, &map);
    fail_unless(res == -ENOENT);
}
END_TEST

START_TEST(make_bitmap_nofile_create)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_nofile_create", 4096, 1, 1, &map);
    unlink("/tmp/mmap_nofile_create");
    fail_unless(res == 0);
}
END_TEST

START_TEST(make_bitmap_resize)
{
    int fh = open("/tmp/mmap_resize", O_RDWR|O_CREAT);
    fchmod(fh, 0777);
    fail_unless(fh > 0);
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_resize", 4096, 0, 1, &map);
    unlink("/tmp/mmap_resize");
    fail_unless(res == 0);
}
END_TEST

/*
 * int bitmap_flush(bloom_bitmap *map) {
 */
START_TEST(flush_bitmap_anonymous)
{
    bloom_bitmap map;
    int res = bitmap_from_file(-1, 4096, &map);
    fail_unless(res == 0);
    fail_unless(bitmap_flush(&map) == 0);
}
END_TEST

START_TEST(flush_bitmap_file)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_flush_bitmap", 8196, 1, 1, &map);
    fail_unless(res == 0);
    fail_unless(bitmap_flush(&map) == 0);
    unlink("/tmp/mmap_flush_bitmap");
}
END_TEST

START_TEST(flush_bitmap_null)
{
    fail_unless(bitmap_flush(NULL) == -EINVAL);
}
END_TEST

/*
 * int bitmap_close(bloom_bitmap *map) {
 */

START_TEST(close_bitmap_anonymous)
{
    bloom_bitmap map;
    int res = bitmap_from_file(-1, 4096, &map);
    fail_unless(res == 0);
    fail_unless(bitmap_close(&map) == 0);
    fail_unless(map.mmap == NULL);
}
END_TEST

START_TEST(close_bitmap_file)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_close_bitmap", 8196, 1, 1, &map);
    fail_unless(res == 0);
    fail_unless(bitmap_close(&map) == 0);
    fail_unless(map.mmap == NULL);
    unlink("/tmp/mmap_close_bitmap");
}
END_TEST

START_TEST(close_bitmap_null)
{
    fail_unless(bitmap_close(NULL) == -EINVAL);
}
END_TEST

START_TEST(double_close_bitmap_file)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_close_bitmap", 8196, 1, 1, &map);
    fail_unless(res == 0);
    fail_unless(bitmap_close(&map) == 0);
    fail_unless(map.mmap == NULL);
    unlink("/tmp/mmap_close_bitmap");
    fail_unless(bitmap_close(&map) < 0);
}
END_TEST

/*
 *#define BITMAP_GETBIT(map, idx)
 */
START_TEST(getbit_bitmap_anonymous_zero)
{
    bloom_bitmap map;
    bitmap_from_file(-1, 4096, &map);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        fail_unless(BITMAP_GETBIT((&map), idx) == 0);
    }
}
END_TEST

START_TEST(getbit_bitmap_anonymous_one)
{
    bloom_bitmap map;
    bitmap_from_file(-1, 4096, &map);
    memset(map.mmap, 255, 4096);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        fail_unless(BITMAP_GETBIT((&map), idx) == 1);
    }
}
END_TEST

START_TEST(getbit_bitmap_file_zero)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_getbit_zero", 4096, 1, 1, &map);
    fail_unless(res == 0);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        fail_unless(BITMAP_GETBIT((&map), idx) == 0);
    }
    unlink("/tmp/mmap_getbit_zero");
}
END_TEST

START_TEST(getbit_bitmap_file_one)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_getbit_one", 4096, 1, 1, &map);
    fail_unless(res == 0);
    memset(map.mmap, 255, 4096);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        fail_unless(BITMAP_GETBIT((&map), idx) == 1);
    }
    unlink("/tmp/mmap_getbit_one");
}
END_TEST

START_TEST(getbit_bitmap_anonymous_one_onebyte)
{
    bloom_bitmap map;
    bitmap_from_file(-1, 4096, &map);
    map.mmap[1] = 128;
    fail_unless(BITMAP_GETBIT((&map), 8) == 1);
}
END_TEST


/*
 *#define BITMAP_SETBIT(map, idx, val)
 */
START_TEST(setbit_bitmap_anonymous_one_byte)
{
    bloom_bitmap map;
    bitmap_from_file(-1, 4096, &map);
    BITMAP_SETBIT((&map), 1, 1);
    fail_unless(map.mmap[0] == 64);
}
END_TEST

START_TEST(setbit_bitmap_anonymous_one_byte_aligned)
{
    bloom_bitmap map;
    bitmap_from_file(-1, 4096, &map);
    BITMAP_SETBIT((&map), 8, 1);
    fail_unless(map.mmap[1] == 128);
}
END_TEST


START_TEST(setbit_bitmap_anonymous_one)
{
    bloom_bitmap map;
    bitmap_from_file(-1, 4096, &map);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        BITMAP_SETBIT((&map), idx, 1);
    }
    for (int idx = 0; idx < 4096; idx++) {
        fail_unless(map.mmap[idx] == 255);
    }
}
END_TEST

START_TEST(setbit_bitmap_file_one)
{
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_setbit_one", 4096, 1, 1, &map);
    fail_unless(res == 0);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        BITMAP_SETBIT((&map), idx, 1);
    }
    for (int idx = 0; idx < 4096; idx++) {
        fail_unless(map.mmap[idx] == 255);
    }
    unlink("/tmp/mmap_setbit_one");
}
END_TEST

/**
 * Test that flush does indeed write to disk
 */
START_TEST(flush_does_write) {
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_flush_write", 4096, 1, 1, &map);
    fchmod(map.fileno, 0777);
    fail_unless(res == 0);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        BITMAP_SETBIT((&map), idx, 1);
    }
    bitmap_flush(&map);

    bloom_bitmap map2;
    res = bitmap_from_filename("/tmp/mmap_flush_write", 4096, 0, 0, &map2);
    fail_unless(res == 0);
    for (int idx = 0; idx < 4096; idx++) {
        fail_unless(map2.mmap[idx] == 255);
    }
    unlink("/tmp/mmap_flush_write");
}
END_TEST

START_TEST(close_does_flush) {
    bloom_bitmap map;
    int res = bitmap_from_filename("/tmp/mmap_close_flush", 4096, 1, 1, &map);
    fchmod(map.fileno, 0777);
    fail_unless(res == 0);
    for (int idx = 0; idx < 4096*8 ; idx++) {
        BITMAP_SETBIT((&map), idx, 1);
    }
    bitmap_close(&map);

    res = bitmap_from_filename("/tmp/mmap_close_flush", 4096, 0, 0, &map);
    fail_unless(res == 0);
    for (int idx = 0; idx < 4096; idx++) {
        fail_unless(map.mmap[idx] == 255);
    }
    unlink("/tmp/mmap_close_flush");
}
END_TEST
