--TEST--
MDBM iterate
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);

for($i=0;$i<10;$i++) {
    $rv = mdbm_store($db, $i, $i*1234567);
    if($rv == false) {
        FAIL();
    }
}

$rv = mdbm_sync($db);
if ($rv == false) {
    FAIL();
}

$rv = mdbm_close($db);
if ($rv == true) {
    OK();
}


$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDONLY, 0666, 0,0);

printf("[*] mdbm_first : %s\n", print_r(mdbm_first($db2), true));
printf("[*] mdbm_next: %s\n", print_r(mdbm_next($db2), true));
printf("[*] mdbm_firstkey: %s\n", print_r(mdbm_firstkey($db2), true));
printf("[*] mdbm_nextkey: %s\n", print_r(mdbm_nextkey($db2), true));
printf("[*] mdbm_nextkey: %s\n", print_r(mdbm_nextkey($db2), true));
printf("[*] mdbm_nextkey: %s\n", print_r(mdbm_nextkey($db2), true));
printf("[*] mdbm_count_records : %d\n", mdbm_count_records($db2));

$rv = mdbm_close($db2);
if ($rv == false) {
    FAIL();
}


?>
--EXPECT--
OK
[*] mdbm_first : Array
(
    [key] => 0
    [value] => 0
)

[*] mdbm_next: Array
(
    [key] => 1
    [value] => 1234567
)

[*] mdbm_firstkey: 0
[*] mdbm_nextkey: 1
[*] mdbm_nextkey: 2
[*] mdbm_nextkey: 3
[*] mdbm_count_records : 10
