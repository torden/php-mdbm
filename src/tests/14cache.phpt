--TEST--
MDBM cache
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

$rv = mdbm_get_cachemode($db);
CHECK_FALSE($rv);

echo mdbm_get_cachemode_name($rv);
echo "\n";

$rv = mdbm_set_cachemode($db, MDBM_CACHEMODE_MAX);
CHECK_FALSE($rv);

$rv = mdbm_get_cachemode($db);
CHECK_FALSE($rv);

echo mdbm_get_cachemode_name($rv);
echo "\n";

$rv = mdbm_close($db);
CHECK_FALSE($rv);
?>
--EXPECT--
none
GDSF
