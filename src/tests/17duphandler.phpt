--TEST--
MDBM dup handle
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($db, $i, $v, MDBM_REPLACE);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$db2 = mdbm_dup_handle($db);
CHECK_FALSE($rv);
for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($db2, $i, $v, MDBM_REPLACE);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($db2);
CHECK_FALSE($rv);



$rv = mdbm_close($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db2);
CHECK_FALSE($rv);

?>
--EXPECT--
