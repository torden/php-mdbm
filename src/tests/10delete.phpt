--TEST--
MDBM delete
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM_DELETE, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

for($i=0;$i<12;$i++) {
    $rv = mdbm_store($db, $i, $i);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

$db2 = mdbm_open(TEST_MDBM_DELETE, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db2);

$val = mdbm_fetch($db2,9);
CHECK_FALSE($rv);

$val = mdbm_fetch($db2,0);
CHECK_FALSE($rv);

?>
--EXPECT--
