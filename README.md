# PHP-mdbm

*Unfortunately, Not ready for use. It won't be much longer...*

- MDBM is a super-fast memory-mapped key/value store.
- MDBM is an ndbm work-alike hashed database library based on sdbm which is based on Per-Aake Larson’s Dynamic Hashing algorithms.
- MDBM is a high-performance, memory-mapped hash database similar to the homegrown libhash.
- The records stored in a mdbm database may have keys and values of arbitrary and variable lengths.

[![Build Status](https://travis-ci.org/torden/php-mdbm.svg?branch=master)](https://travis-ci.org/torden/php-mdbm)

## API (Support)

the following is list of support api on now.

|*API*|
|---|
|mdbm_log_minlevel|
|mdbm_open|
|mdbm_close|
|mdbm_truncate|
|mdbm_sync|
|mdbm_fsync|
|mdbm_lock|
|mdbm_unlock|
|mdbm_islocked|
|mdbm_isowned|
|mdbm_lock_reset|
|mdbm_delete_lockfiles|
|mdbm_preload|
|mdbm_get_errno|
|mdbm_get_version|
|mdbm_get_size|
|mdbm_get_page_size|
|mdbm_set_hash|
|mdbm_get_hash|
|mdbm_get_limit_size|
|mdbm_store|
|mdbm_fetch|
|mdbm_delete|
|mdbm_first|
|mdbm_next|
|mdbm_firstkey|
|mdbm_nextkey|
|mdbm_count_records|

## Support two compatibility version

### PHP

|*Version*|*Support*|*Test*|*Develop*|
|---|---|---|---|
|5.x|yes|always|doing|
|7.x|yes(as soon)|as soon|as soon|

### MDBM

|*branch or release ver.*|*Support*|*Test*|*Comment*|
|---|---|---|---|
|master|yes|always|
|4.x|yes|always|

## Install

Download from github

```shell
git clone https://github.com/torden/php-mdbm
```

Compile (for PHP5.x)

```shell
cd php-mdbm/php5/
$PHP_INSTALLED_PATH/bin/phpize
./configure --with-mdbm=/usr/local/mdbm/
make
make test
make install
```

## Example

### Creating and populating a database

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS, 0666, 0,0);
if($db == false) {
	echo "Unable to create database";
	exit(2);
}

mdbm_lock($db);

for($i=0;$i<65535;$i++) {

    $v = rand(1,65535);
    //$rv = mdbm_store($db, $i, $v, MDBM_INSERT); same below
    $rv = mdbm_store($db, $i, $v);
    if($rv == false) {
    	echo "failed to store";
    	break;
    }
}

mdbm_unlock($db);

mdbm_sync(); //optional flush to disk
mdbm_close($db);
?>
```

### Fetching and updating records in-place

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db == false) {
	echo "Unable to open database";
	exit(2);
}

for($i=0;$i<65535;$i++) {

    $v = rand(1,65535);
    $val = mdbm_fetch($db, $v);
    if($val == false) {
    	echo "failed to store";
    	break;
    }

    printf("store : val=%s\n", $val);
}

mdbm_close($db);
?>
```

### Replacing(Updating) value of records in-place

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db == false) {
	echo "Unable to open database";
	exit(2);
}

for($i=0;$i<10;$i++) {

	$val = mdbm_fetch($db, $i);
    if($val == false) {
    	echo "failed to fetch";
    	break;
    }

    $v = rand(1,65535);

    printf("replace : val=%s to %s\n", $val, $v);

    $rv = mdbm_store($db, $v, MDBM_REPLACE);	
    if($rv == false) {
    	echo "failed to store(replace)";
    	break;
    }
}

mdbm_close($db);
?>
```

### Deleting records in-place

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db == false) {
	echo "Unable to open database";
	exit(2);
}

for($i=0;$i<10;$i++) {

	$v = rand(1,65535);
	$val = mdbm_fetch($db, $v);
    if($val == false) {
    	echo "failed to fetch";
    	break;
    }

    printf("delete : val=%s\n", $val);

    $rv = mdbm_delete($db, $v);	
    if($rv == false) {
    	echo "failed to delete";
    	break;
    }
}

mdbm_sync(); //optional flush to disk
mdbm_close($db);
?>
```


### Adding/replacing records

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db == false) {
	echo "Unable to open database";
	exit(2);
}

for($i=65536;$i<655360;$i++) {

	mdbm_lock($db);
	$rv = mdbm_store($db, $v, MDBM_REPLACE);
	mdbm_unlock($db);

    if($rv == false) {
    	echo "failed to store(replace)";
    	break;
    }
}

mdbm_close($db);
?>
```


### Iterating over all records

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db == false) {
	echo "Unable to open database";
	exit(2);
}

$kv = mdbm_first($db);
if($kv == false) {
	echo "failed to get a first record";
}

while($kv) {

    print_r($kv);
    $kv = mdbm_next($db);
}

$rv = mdbm_close($db);
if ($rv == false) {
    FAIL();
}
?>
```


### Iterating over all keys

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db == false) {
	echo "Unable to open database";
	exit(2);
}

$key = mdbm_firstkey($db);
if($key == false) {
	echo "failed to get a first record";
}

while($key != false) {

    echo "$kv\n";
    $key = mdbm_nextkey($db);
}

$rv = mdbm_close($db);
if ($rv == false) {
    FAIL();
}
?>
```


## Link

- [Yahoo! MDBM](https://github.com/yahoo/mdbm)
- [MDBM::Concept](http://yahoo.github.io/mdbm/guide/concepts.html)
- [MDBM::Build](https://github.com/yahoo/mdbm/blob/master/README.build)
- [MDBM::Document](http://yahoo.github.io/mdbm/)
- [MDBM::FAQ](http://yahoo.github.io/mdbm/guide/faq.html)
- [DBM](https://en.wikipedia.org/wiki/Dbm)
- [MDBM::Macro(const)](http://yahoo.github.io/mdbm/api/mdbm_8h.html)
- [Go-mdbm](https://github.com/torden/go-mdbm)


---
Please feel free. I hope it is helpful for you
