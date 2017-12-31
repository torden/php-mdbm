--TEST--
MDBM compress_tree
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

//generate
$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

for($i=0;$i<=65535;$i++) {

    $v =  rand(0,65535);
    $rv = mdbm_store($db, $i, $v);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

//ramdon delete
$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($rv);

for($i=0;$i<=1000;$i++) {

    $v =  rand(0,65535);
    mdbm_delete($db, $v); //ignore return value
}

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

//compress tree
$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db);

mdbm_compress_tree($db);

$rv = mdbm_close($db);
CHECK_FALSE($rv);
?>
--EXPECT--
