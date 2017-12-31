--TEST--
MDBM create db
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);

//INSERT
for($i=0;$i<10;$i++) {

    $rv = mdbm_lock($db);
    if($rv == false) {
        FAIL();
    }

    
    $rv = mdbm_store($db, $i, $i);
    //$rv = mdbm_store($db, $i, $i, MDBM_INSERT); 
    if($rv == false) {
        FAIL();
    }

    $rv = mdbm_unlock($db);
    if($rv == false) {
        FAIL();
    }
}

$rv = mdbm_close($db);
if ($rv == 1) {
    OK();
}



$db = mdbm_open(TEST_MDBM, MDBM_O_RDONLY, 0666, 0,0);
printf("[*] mdbm_get_errno : %d\n", mdbm_get_errno($db));
printf("[*] mdbm_get_version : %d\n", mdbm_get_version($db));
printf("[*] mdbm_get_size : %d\n", mdbm_get_size($db));
printf("[*] mdbm_get_page_size : %d\n", mdbm_get_page_size($db));
printf("[*] mdbm_get_hash : %d\n", mdbm_get_hash($db));
printf("[*] mdbm_count_records : %d\n", mdbm_count_records($db));
printf("[*] mdbm_get_limit_size : %d\n", mdbm_get_limit_size($db));


$rv = mdbm_close($db);
if ($rv == 1) {
    OK();
}



?>
--EXPECT--
OK
[*] mdbm_get_errno : 0
[*] mdbm_get_version : 3
[*] mdbm_get_size : 8192
[*] mdbm_get_page_size : 4096
[*] mdbm_get_hash : 5
[*] mdbm_count_records : 10
[*] mdbm_get_limit_size : 0
OK
