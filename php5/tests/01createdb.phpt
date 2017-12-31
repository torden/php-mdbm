--TEST--
MDBM create db
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS, 0666, 0,0);
$rv = mdbm_close($db);
if ($rv == 1) {
    OK();
}

$db2 = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDONLY|MDBM_O_ASYNC, 0666, 0,0);
$rv = mdbm_close($db2);
if ($rv == 1) {
    OK();
}

?>
--EXPECT--
OK
OK
