--TEST--
MDBM miscellaneous
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$rv = mdbm_get_hash_value("test", MDBM_HASH_CRC32);
CHECK_EQUALS($rv, 3430166611);

$rv = mdbm_get_hash_value("test", MDBM_HASH_EJB);
CHECK_EQUALS($rv, 146974);

$rv = mdbm_get_hash_value("test", MDBM_HASH_PHONG);
CHECK_EQUALS($rv, 534825436);

$rv = mdbm_get_hash_value("test", MDBM_HASH_OZ);
CHECK_EQUALS($rv, 1195757874);

$rv = mdbm_get_hash_value("test", MDBM_HASH_TOREK);
CHECK_EQUALS($rv, 4282592);

$rv = mdbm_get_hash_value("test", MDBM_HASH_FNV);
CHECK_EQUALS($rv, 3449832747);

$rv = mdbm_get_hash_value("test", MDBM_HASH_STL);
CHECK_EQUALS($rv, 17716);

$rv = mdbm_get_hash_value("test", MDBM_HASH_MD5);
CHECK_EQUALS($rv, 3446378249);

$rv = mdbm_get_hash_value("test", MDBM_HASH_SHA_1);
CHECK_EQUALS($rv, 3851373225);

$rv = mdbm_get_hash_value("test", MDBM_HASH_JENKINS);
CHECK_EQUALS($rv, 3164412494);

$rv = mdbm_get_hash_value("test", MDBM_HASH_HSIEH);
CHECK_EQUALS($rv, 605072156);


$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS, 0666, 0,0);
CHECK_FALSE($db);

$rv = mdbm_set_alignment($db, MDBM_ALIGN_64_BITS);
CHECK_FALSE($db);

$rv = mdbm_enable_stat_operations($db, MDBM_STATS_BASIC | MDBM_STATS_TIMED);
CHECK_FALSE($db);

for($i=0;$i<=4096;$i++) {
    $rv = mdbm_store($db, $i*123, $i*986);
    CHECK_FALSE($rv);
}


$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db2);


$rv = mdbm_enable_stat_operations($db2, MDBM_STATS_BASIC | MDBM_STATS_TIMED);
CHECK_FALSE($db2);



$rv = mdbm_get_alignment($db2);
CHECK_FALSE($rv);


$rv = mdbm_get_stat_time($db2, MDBM_STAT_TYPE_FETCH);
CHECK_FALSE($rv);

$rv = mdbm_get_stat_time($db2, MDBM_STAT_TYPE_STORE);
CHECK_FALSE($rv);

$rv = mdbm_get_stat_time($db2, MDBM_STAT_TYPE_DELETE);
CHECK_FALSE($rv);


$rv = mdbm_reset_stat_operations($db2);
CHECK_FALSE($rv);


$rv = mdbm_get_magic_number($db2);
CHECK_FALSE($rv);
CHECK_EQUALS($rv, 16922980);

$pagenum =  mdbm_get_page($db2, 2000);
CHECK_FALSE($pagenum);
CHECK_GREATER_THAN_EQUAL_TO($pagenum, 0);

$rv = mdbm_chk_page($db2, $pagenum);
CHECK_FALSE($pagenum);

$rv = mdbm_fcopy($db2, "/tmp/aa.mdbm", 0644, MDBM_COPY_LOCK_ALL);
CHECK_FALSE($rv);

$rv = mdbm_close($db2);
CHECK_FALSE($rv);

?>
--EXPECT--
