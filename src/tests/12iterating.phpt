--TEST--
MDBM iterating
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");
$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS, 0666, 0,0);
CHECK_FALSE($db);

for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);
    $rv = mdbm_store($db, $i, $v, MDBM_REPLACE);
    CHECK_FALSE($rv);
}

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
CHECK_FALSE($db2);

$key = mdbm_firstkey($db2);
while($key != false) {

    echo "$key\n";
    $key = mdbm_nextkey($db2, MDBM_REPLACE);
}

$rv = mdbm_close($db2);
CHECK_FALSE($rv);

?>
--EXPECT--
