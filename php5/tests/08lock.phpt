--TEST--
MDBM locking
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_ANY_LOCKS, 0666, 0,0);
CHECK_FALSE($db);

$rv = mdbm_lock($db);
CHECK_FALSE($rv);

$rv = mdbm_islocked($db);
CHECK_FALSE($rv);

$rv = mdbm_get_lockmode($db);
CHECK_FALSE($rv);

$rv = mdbm_isowned($db);
CHECK_FALSE($rv);

//output to consol
$rv = mdbm_lock_reset(TEST_MDBM);
CHECK_FALSE($rv);

$rv = mdbm_lock($db);
CHECK_FALSE($rv);

$rv = mdbm_islocked($db);
CHECK_FALSE($rv);

$rv = mdbm_isowned($db);
CHECK_FALSE($rv);

$rv = mdbm_delete_lockfiles(TEST_MDBM);
CHECK_FALSE($rv);

?>
--EXPECT--
NOTE: Resetting locks for /tmp/test_phpt.mdbm!
