--TEST--
MDBM iterating
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db);

$kv = mdbm_first($db);
CHECK_FALSE($kv);

while($kv) {

    print_r($kv);
    $kv = mdbm_next($db);
}

$rv = mdbm_close($db);
CHECK_FALSE($rv);

?>
--EXPECT--
Array
(
    [key] => 0
    [value] => 0
)
Array
(
    [key] => 2
    [value] => 2
)
Array
(
    [key] => 3
    [value] => 3
)
Array
(
    [key] => 4
    [value] => 4
)
Array
(
    [key] => 5
    [value] => 5
)
Array
(
    [key] => 6
    [value] => 6
)
Array
(
    [key] => 7
    [value] => 7
)
Array
(
    [key] => 8
    [value] => 8
)
