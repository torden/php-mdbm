#!/app/php5/bin/php  -d extension=./modules/mdbm.so
<?php

$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS, 0666, 0,0);

echo "\n-------------------------\n";
echo "db : ";
print_r($db);
$rv = mdbm_close($db);

echo "\n-------------------------\n";
echo "rv : ";
print_r($rv);

echo "\n-------------------------\n";
echo "db : ";
print_r($db);

mdbm_close($db);
echo "\n\n";
?>
