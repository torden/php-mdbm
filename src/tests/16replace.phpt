--TEST--
MDBM replace
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$dummy1 = mdbm_open(TEST_MDBM_REPLACE1, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($dummy1);
for($i=0;$i<2018;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($dummy1, $i, $v);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($dummy1);
CHECK_FALSE($rv);

$rv = mdbm_close($dummy1);
CHECK_FALSE($rv);

$dummy2 = mdbm_open(TEST_MDBM_REPLACE2, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($dummy2);
for($i=0;$i<2018;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($dummy2, $i, $v);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($dummy2);
CHECK_FALSE($rv);

$rv = mdbm_close($dummy2);
CHECK_FALSE($rv);



$db2 = mdbm_open(TEST_MDBM_REPLACE1, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db2);

$rv = mdbm_replace_db($db2, TEST_MDBM_REPLACE2);
CHECK_FALSE($rv);

$rv = mdbm_close($db2);
CHECK_FALSE($rv);


$dummy1 = mdbm_open(TEST_MDBM_REPLACE1, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($dummy1);
for($i=0;$i<2018;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($dummy1, $i, $v);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($dummy1);
CHECK_FALSE($rv);

$rv = mdbm_close($dummy1);
CHECK_FALSE($rv);

$dummy2 = mdbm_open(TEST_MDBM_REPLACE2, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($dummy2);
for($i=0;$i<2018;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($dummy2, $i, $v);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($dummy2);
CHECK_FALSE($rv);

$rv = mdbm_close($dummy2);
CHECK_FALSE($rv);
$rv = mdbm_replace_file(TEST_MDBM_REPLACE2, TEST_MDBM_REPLACE1);
CHECK_FALSE($rv);
?>
--EXPECT--
