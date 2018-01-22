<?php
require_once('./vendor/autoload.php');
use Performance\Performance;

define("TEST1", "/tmp/test_benchmark_1.db");
define("TEST_STR1", "/tmp/test_benchmark_str_1.db");

define("TEST2", "/tmp/test_benchmark_2.db");
define("TEST_STR2", "/tmp/test_benchmark_str_2.db");

define("TEST3", "/tmp/test_benchmark_3.db");
define("TEST_STR3", "/tmp/test_benchmark_str_3.db");


$limit = 100000;
Performance::point("sqlite3 insert(number, number) :: 100,000");
{
    @unlink(TEST1);
    $db = new SQLite3(TEST1);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $db->exec('CREATE TABLE benchmark (key INTEGER, var INTEGER, PRIMARY KEY(key))');
    $db->exec('CREATE INDEX idx_benchmark_key ON benchmark(key)');
    $stmt = $db->prepare('INSERT INTO benchmark VALUES(:key, :val)');

    $db->exec('BEGIN;');
    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', $i, SQLITE3_INTEGER);
        $stmt->bindValue(':val', $v, SQLITE3_INTEGER);
        $rv = $stmt->execute();
        if ($rv === false) {
            Performance::message("Error : insert($i, $v)");
        }
    }
    $db->exec('COMMIT;');
    $db->close();
}
Performance::finish();

Performance::point("sqlite3 insert(string, string) :: 100,000");
{
    @unlink(TEST_STR1);
    $db = new SQLite3(TEST_STR1);
    $db->busytimeout(5000);
    $db->exec('pragma journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $db->exec('CREATE TABLE benchmark (key STRING, var STRING, PRIMARY KEY(key))');
    $db->exec('CREATE INDEX idx_benchmark_key ON benchmark(key)');
    $stmt = $db->prepare('INSERT INTO benchmark VALUES(:key, :val)');

    $db->exec('BEGIN;');
    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', "hello word($i)", SQLITE3_TEXT);
        $stmt->bindValue(':val', "hello php-mdbm($v)", SQLITE3_TEXT);
        $rv = $stmt->execute();
        if ($rv === false) {
            Performance::message("Error : insert($i, $v)");
        }
    }
    $db->exec('COMMIT;');
    $db->close();
}
Performance::finish();


$limit = 1000000;
Performance::point("sqlite3 insert(number, number) :: 1,000,000");
{
    @unlink(TEST2);
    $db = new SQLite3(TEST2);
    $db->busytimeout(5000);
    $db->exec('pragma journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $db->exec('CREATE TABLE benchmark (key INTEGER, var INTEGER, PRIMARY KEY(key))');
    $db->exec('CREATE INDEX idx_benchmark_key ON benchmark(key)');
    $stmt = $db->prepare('INSERT INTO benchmark VALUES(:key, :val)');

    $db->exec('BEGIN;');
    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', $i, SQLITE3_INTEGER);
        $stmt->bindValue(':val', $v, SQLITE3_INTEGER);
        $rv = $stmt->execute();
        if ($rv === false) {
            Performance::message("Error : insert($i, $v)");
        }
    }
    $db->exec('COMMIT;');
    $db->close();
}
Performance::finish();

Performance::point("sqlite3 insert(string, string) :: 1,000,000");
{
    @unlink(TEST_STR2);
    $db = new SQLite3(TEST_STR2);
    $db->busytimeout(5000);
    $db->exec('pragma journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $db->exec('CREATE TABLE benchmark (key STRING, var STRING, PRIMARY KEY(key))');
    $db->exec('CREATE INDEX idx_benchmark_key ON benchmark(key)');
    $stmt = $db->prepare('INSERT INTO benchmark VALUES(:key, :val)');

    $db->exec('BEGIN;');
    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', "hello word($i)", SQLITE3_TEXT);
        $stmt->bindValue(':val', "hello php-mdbm($v)", SQLITE3_TEXT);
        $rv = $stmt->execute();
        if ($rv === false) {
            Performance::message("Error : insert($i, $v)");
        }
    }
    $db->exec('COMMIT;');
    $db->close();
}
Performance::finish();

$limit = 10000000;
Performance::point("sqlite3 insert(number, number):: 10,000,000");
{
    @unlink(TEST3);
    $db = new SQLite3(TEST3);
    $db->busytimeout(5000);
    $db->exec('pragma journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $db->exec('CREATE TABLE benchmark (key INTEGER, var INTEGER, PRIMARY KEY(key))');
    $db->exec('CREATE INDEX idx_benchmark_key ON benchmark(key)');
    $stmt = $db->prepare('INSERT INTO benchmark VALUES(:key, :val)');

    $db->exec('BEGIN;');
    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', $i, SQLITE3_INTEGER);
        $stmt->bindValue(':val', $v, SQLITE3_INTEGER);
        $rv = $stmt->execute();
        if ($rv === false) {
            Performance::message("Error : insert($i, $v)");
        }
    }
    $db->exec('COMMIT;');
    $db->close();

}
Performance::finish();

Performance::point("sqlite3 insert(string, string) :: 10,000,000");
{
    @unlink(TEST_STR3);
    $db = new SQLite3(TEST_STR3);
    $db->busytimeout(5000);
    $db->exec('pragma journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $db->exec('CREATE TABLE benchmark (key STRING, var STRING, PRIMARY KEY(key))');
    $db->exec('CREATE INDEX idx_benchmark_key ON benchmark(key)');
    $stmt = $db->prepare('INSERT INTO benchmark VALUES(:key, :val)');

    $db->exec('BEGIN;');
    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', "hello word($i)", SQLITE3_TEXT);
        $stmt->bindValue(':val', "hello php-mdbm($v)", SQLITE3_TEXT);
        $rv = $stmt->execute();
        if ($rv === false) {
            Performance::message("Error : insert($i, $v)");
        }
    }
    $db->exec('COMMIT;');
    $db->close();
}
Performance::finish();

Performance::results();
?>
