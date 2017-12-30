#!/app/php5/bin/php  -d extension=./modules/mdbm.so
<?php

$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS, 0666, 0,0);

echo "[*] mdbm_open : ";
print_r($db);
echo "\n";
printf("[*] mdbm_get_hash : %d\n", mdbm_get_hash($db));
printf("[*] mdbm_get_version : %d\n", mdbm_get_version($db));
printf("[*] mdbm_get_size : %d\n", mdbm_get_size($db));
printf("[*] mdbm_get_page_size : %d\n", mdbm_get_page_size($db));

print_r($db);
echo "\n";

printf("[*] mdbm_set_hash : %d\n", mdbm_set_hash($db, MDBM_MAX_HASH));
printf("[*] mdbm_get_hash : %d\n", mdbm_get_hash($db));

printf("[*] mdbm_set_hash : %d\n", mdbm_set_hash($db, MDBM_HASH_CRC32));
printf("[*] mdbm_get_hash : %d\n", mdbm_get_hash($db));
printf("[*] mdbm_get_limit_size : %d\n", mdbm_get_limit_size($db));


echo "\n-------------------------\n";
for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);

    printf("%s\n", str_repeat("-",80));
    printf("[*] mdbm_lock : %d\n", mdbm_lock($db));

    $rv = mdbm_store($db, $i, $v);
    printf("[*] store(%d,%d)::rv=%d\n", $i, $v,rv);

    printf("[*] mdbm_unlock : %d\n", mdbm_unlock($db));
}


printf("[*] mdbm_fetch(2) : %s\n", mdbm_fetch($db, 2));
printf("[*] mdbm_delete(2) : %d\n", mdbm_delete($db, 2));
printf("[*] mdbm_fetch(2) : %s\n", mdbm_fetch($db, 2));


printf("[*] mdbm_sync : %d\n", mdbm_sync($db));
printf("[*] mdbm_fsync : %d\n", mdbm_fsync($db));
printf("[*] mdbm_close : %d\n", mdbm_close($db));


$db2 = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDONLY, 0666, 0,0);
printf("[*] mdbm_preload : %s\n", mdbm_preload($db2));

for($i=0;$i<3;$i++) {
    $v =  rand(0,10);
    printf("[*] mdbm_fetch(%d) : %s\n", $v, mdbm_fetch($db2, $v));
}


echo "\n";

printf("[*] mdbm_get_errno : %d\n", mdbm_get_errno($db2));
printf("[*] mdbm_get_version : %d\n", mdbm_get_version($db2));
printf("[*] mdbm_get_size : %d\n", mdbm_get_size($db2));
printf("[*] mdbm_get_page_size : %d\n", mdbm_get_page_size($db2));
printf("[*] mdbm_get_hash : %d\n", mdbm_get_hash($db2));


printf("[*] mdbm_close : %d\n", mdbm_close($db2));

echo "\n\n";
?>
