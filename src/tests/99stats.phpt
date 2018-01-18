--TEST--
MDBM stats
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db);

$rv = mdbm_get_stats($db);
print_r($rv);


$rv = mdbm_close($db);
CHECK_FALSE($rv);

?>
--EXPECT--
Array
(
    [s_size] => 8192
    [s_page_size] => 4096
    [s_page_count] => 2
    [s_pages_used] => 1
    [s_bytes_used] => 55
    [s_num_entries] => 10
    [s_min_level] => 1
    [s_max_level] => 1
    [s_large_page_size] => 4096
    [s_large_page_count] => 0
    [s_large_threshold] => 3072
    [s_large_pages_used] => 0
    [s_large_num_free_entries] => 0
    [s_large_max_free] => 0
    [s_large_num_entries] => 0
    [s_large_bytes_used] => 0
    [s_large_min_size] => 0
    [s_large_max_size] => 0
    [s_cache_mode] => 0
)
