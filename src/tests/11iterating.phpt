--TEST--
MDBM iterating
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM_ITER, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

//INSERT
for($i=0;$i<10;$i++) {

    $rv = mdbm_store($db, $i, $i);
    CHECK_FALSE($rv);
}
$rv = mdbm_close($db);
CHECK_FALSE($rv);


$db2 = mdbm_open(TEST_MDBM_ITER, MDBM_O_RDONLY, 0666, 0,0);
$kv = mdbm_first($db2);
CHECK_FALSE($kv);

while($kv) {

    print_r($kv);
    $kv = mdbm_next($db2);
}

$rv = mdbm_close($db2);
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
    [key] => 1
    [value] => 1
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
Array
(
    [key] => 9
    [value] => 9
)
