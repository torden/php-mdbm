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

$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);

//output to consol
$rv = mdbm_lock_reset(TEST_MDBM);
CHECK_FALSE($rv);

//quite
mdbm_log_minlevel(MDBM_LOG_OFF);
$rv = mdbm_lock_reset(TEST_MDBM);
CHECK_FALSE($rv);

$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);

$rv = mdbm_lock($db);
CHECK_FALSE($rv);

$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);

$rv = mdbm_islocked($db);
CHECK_FALSE($rv);

$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);

$rv = mdbm_isowned($db);
CHECK_FALSE($rv);

$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);

$rv = mdbm_delete_lockfiles(TEST_MDBM);
CHECK_FALSE($rv);

$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);

$rv = mdbm_trylock($db);
CHECK_FALSE($rv);
$rv = mdbm_store($db, rand(0,123456789), rand(0,123456789), MDBM_REPLACE);
CHECK_FALSE($rv);
$rv = mdbm_unlock($db);
CHECK_FALSE($rv);


$key = rand(0, 1234567890);

$rv = mdbm_plock($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);
    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);
$rv = mdbm_punlock($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);

$rv = mdbm_tryplock($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);
    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);
$rv = mdbm_punlock($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);

$rv = mdbm_lock_shared($db);
CHECK_FALSE($rv);
    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);
$rv = mdbm_unlock($db);
CHECK_FALSE($rv);

$rv = mdbm_trylock_shared($db);
CHECK_FALSE($rv);
    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);
$rv = mdbm_unlock($db);
CHECK_FALSE($rv);


$rv = mdbm_lock_smart($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);
    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);
$rv = mdbm_unlock_smart($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);

$rv = mdbm_trylock_smart($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);
    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);
$rv = mdbm_unlock_smart($db, $key, MDBM_RW_LOCKS);
CHECK_FALSE($rv);

$rv = mdbm_protect($db, MDBM_PROT_READ);
CHECK_FALSE($rv);

$rv = mdbm_protect($db, MDBM_PROT_WRITE);
CHECK_FALSE($rv);

/*
$rv = mdbm_lock_pages($db);
CHECK_FALSE($rv);

    //Segment fault after excuted mdbm_lock_page
    //$rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    //$rv = mdbm_fetch($db, $key);
    //CHECK_FALSE($rv);

$rv = mdbm_unlock_pages($db);
CHECK_FALSE($rv);
*/

$rv = mdbm_protect($db, MDBM_PROT_ACCESS);
CHECK_FALSE($rv);

    $rv = mdbm_store($db, $key, $key, MDBM_REPLACE);
    CHECK_FALSE($rv);

$rv = mdbm_protect($db, MDBM_PROT_NONE);
CHECK_FALSE($rv);

?>
--EXPECT--
NOTE: Resetting locks for /tmp/test_phpt.mdbm!
NOTE: Resetting locks for /tmp/test_phpt.mdbm!
