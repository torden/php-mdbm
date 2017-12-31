--TEST--
MDBM generate data on locking
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_ANY_LOCKS, 0666, 0,0);

$rv = mdbm_lock($db);
if($rv == false) {
    FAIL();
}

$rv = mdbm_islocked($db);
if($rv == false) {
    FAIL();
}

$rv = mdbm_isowned($db);
if($rv == false) {
    FAIL();
}

//output to consol
$rv = mdbm_lock_reset(TEST_MDBM);
if($rv == false) {
    FAIL();
}

$rv = mdbm_lock($db);
if($rv == false) {
    FAIL();
}

$rv = mdbm_islocked($db);
if($rv == false) {
    FAIL();
}

$rv = mdbm_isowned($db);
if($rv == false) {
    FAIL();
}


$rv = mdbm_delete_lockfiles(TEST_MDBM);
if($rv == false) {
    FAIL();
}


?>
--EXPECT--
NOTE: Resetting locks for /tmp/test_phpt.mdbm!
