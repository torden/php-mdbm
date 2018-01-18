--TEST--
MDBM stats
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db);

$rv = mdbm_get_stats($db);
CHECK_FALSE($rv);

$rv = mdbm_get_db_info($db);
CHECK_FALSE($rv);

$rv = mdbm_get_window_stats($db);
CHECK_FALSE($rv);

$rv = mdbm_get_db_stats($db,MDBM_STAT_NOLOCK);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

?>
--EXPECT--
