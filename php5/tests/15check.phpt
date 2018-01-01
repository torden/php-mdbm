--TEST--
MDBM check
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

for($i=0;$i<=65535;$i++) {

    $v =  rand(0,65535);
    $rv = mdbm_store($db, $i, $v);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

for($i=0;$i<=100;$i++) {

    $v =  rand(0,65535);
    mdbm_delete($db, $i);
}

$rv = mdbm_close($db);
CHECK_FALSE($rv);

$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db2);

$rv = mdbm_check($db2, MDBM_CHECK_ALL); //verbose=0
CHECK_FALSE($rv);

$rv = mdbm_chk_all_page($db2);
CHECK_FALSE($rv);

$rv = mdbm_close($db2);
CHECK_FALSE($rv);
?>
--EXPECT--
