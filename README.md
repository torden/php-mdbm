# PHP-mdbm

- PHP-mdbm is a PHP binds to [Yahoo! MDBM C API.](https://github.com/yahoo/mdbm/)
- MDBM is a super-fast memory-mapped key/value store.
- MDBM is an ndbm work-alike hashed database library based on sdbm which is based on Per-Aake Larson’s Dynamic Hashing algorithms.
- MDBM is a high-performance, memory-mapped hash database similar to the homegrown libhash.
- The records stored in a mdbm database may have keys and values of arbitrary and variable lengths.

|Build Stats|PHP-mdbm ver.|License|Y! mdbm ver.|
|:-:|:-:|:-:|:-:|
[![Build Status](https://github.com/torden/php-mdbm/actions/workflows/php-mdbm.yml/badge.svg)](https://github.com/torden/php-mdbm/actions)|[![GitHub version](https://img.shields.io/github/v/release/torden/go-strutil)](https://github.com/torden/go-strutil)| [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)|[![GitHub version](https://img.shields.io/github/v/release/yahoo/mdbm)](https://github.com/yahoo/mdbm)|

## Table of Contents

- [API](#api)
- [Install](#install)
- [Example](https://github.com/torden/php-mdbm/blob/master/README.api.md)
- [Benchmark](#benchmark)
- [Link](#link)

## API

Almost Useful APIs Support.

### Not Supported APIs

Unfortunately, the following list are not supported on now.

|Group|API|
|---|---|
|[Record Iteration](http://yahoo.github.io/mdbm/api/group__RecordIterationGroup.html)|mdbm_iterate|
|[Data Management](http://yahoo.github.io/mdbm/api/group__DataManagementGroup.html)|mdbm_clean, mdbm_prune, mdbm_set_cleanfunc|
|[Statistics](http://yahoo.github.io/mdbm/api/group__StatisticsGroup.html)|mdbm_chunk_iterate|
|[Cache and Backing Store](http://yahoo.github.io/mdbm/api/group__CacheAndBackingStoreGroup.html)|mdbm_set_backingstore|
|[Import and Export](http://yahoo.github.io/mdbm/api/group__ImportExportGroup.html)|mdbm_cdbdump_to_file, mdbm_cdbdump_trailer_and_close, mdbm_cdbdump_add_record, mdbm_dbdump_to_file, mdbm_dbdump_trailer_and_close, mdbm_dbdump_add_record, mdbm_dbdump_export_header, mdbm_dbdump_import_header, mdbm_dbdump_import, mdbm_cdbdump_import|

*If you want them, please feel free to raise an issue*

#### Deprecated APIs

|*API*|*STATUS*|*COMMENT*|
|---|---|---|
|mdbm_save|DEPRECATED|mdbm_save is only supported for V2 MDBMs.|
|mdbm_restore|DEPRECATED|mdbm_restore is only supported for V2 MDBMs.|
|mdbm_sethash|DEPRECATED|Legacy version of mdbm_set_hash() This function has inconsistent naming, an error return value. It will be removed in a future version.|

#### Only a V2 implementation

|*API*|*STATUS*|*COMMENT*|
|---|---|---|
|mdbm_stat_all_page|V3 not supported|There is only a V2 implementation. V3 not currently supported.|
|mdbm_stat_header|V3 not supported|There is only a V2 implementation. V3 not currently supported.|

## Support Two Versions Compatibility

### PHP

|*Version*|*Support*|*Test*|
|---|---|---|
|5.2.x ~ 5.6.x|yes|always|
|7.0 ~ 7.2|yes|always|

### MDBM

|*branch or release ver.*|*Support*|*Test*|*Comment*|
|---|---|---|---|
|master|yes|always|
|4.x|yes|always|

## Install

### MDBM
- [MDBM Packages](https://github.com/torden/mdbm)

### php-mdbm

#### Use the composer (packagist)

See the [composer documentation](https://packagist.org/) for use composer

```shell
composer require torden/php-mdbm
```

#### Use the source code

```shell
git clone https://github.com/torden/php-mdbm
```

##### Or Download the tarball [(tag or release](https://github.com/torden/php-mdbm/releases))

```shell
wget https://github.com/torden/go-mdbm/archive/vX.X.X.tar.gz
tar xvzf vX.X.X.tar.gz
```

##### Compile

```shell
cd php-mdbm/src/
$PHP_INSTALLED_PATH/bin/phpize
./configure --with-php-config=$PHP_INSTALLED_PATH/bin/php-config --with-mdbm=/usr/local/mdbm/
make
#make test TESTS='-q -m'
#make test TESTS='-q'
make install
```

##### Configuration

```shell
echo "extension=mdbm.so" >> php.ini
```

##### Check

```shell
$PHP_INSTALLED_PATH/bin/php -i | fgrep -i mdbm
...
mdbm
MDBM Support => enable
MDBM API Version => 4
PHP MDBM Version => 0.1.0
...
```

## Example

See the [documentation](https://github.com/torden/php-mdbm/blob/master/README.api.md) for more details.

## Benchmark

The following is results of PHP-mdbm vs PHP SQlite3 benchmarks for simple data storing and random fetching.

See the [Source Code](https://github.com/torden/php-mdbm/tree/master/benchmark).

### Prepare for a Benchmark

```
cd php-mdbm/src/benchmark/
composer install
```

### Spec

#### Host

|Type|Spec|
|---|---|
|CPU|Inte i-7|
|RAM|DDR4 32G|
|HDD|Nvme M.2 SSD|

#### VM

|Type|Spec|
|---|---|
|Machine|VM(VirtualBox)|
|OS|Ubuntu 17.10.1 (Artful Aardvark)|
|CPU|2 vCore|
|RAM|8G|

#### Software

|Type|Version|Comment|
|---|---|---|
|PHP|7.0.26|---|
|php-mdbm|v0.1.0|---|
|mdbm|master branch|---|
|SQLite3|3.19.3|Async, Transaction, journal_mode = wal|

### Simple INSERTs

```
php src/mdbm_simple_store.php -live
```

```
 Created by B. van Hoekelen version 2.3.2 PHP v7.0.26
 Max memory 128M, max execution time unlimited on 2018-01-21 01:33:02

   Label                                                         Time        Memory        Peak
---------------------------------------------------------------------------------------------------
 > Calibrate point                                               2.86 μs |    0.00 KB |    2.00 MB
 > mdbm store(number, number) :: 100,000                       297.88 ms |    0.00 KB |    2.00 MB
 > mdbm store(string, string) :: 100,000                       329.36 ms |    0.00 KB |    2.00 MB
 > mdbm store(number, number) :: 1,000,000                       2.94  s |    0.00 KB |    2.00 MB
 > mdbm store(string, string) :: 1,000,000                       3.36  s |    0.00 KB |    2.00 MB
 > mdbm store(number, number):: 10,000,000                       70.4  s |    0.00 KB |    2.00 MB
 > mdbm store(string, string) :: 10,000,000                    125.46  s |    0.00 KB |    2.00 MB
---------------------------------------------------------------------------------------------------
   Total 7 taken                             01-21 01:36:25    202.78  s      0.00 KB      2.00 MB
```

### Simple Random FETCHs

```
php src/mdbm_simple_fetch.php -live
```

```
 Created by B. van Hoekelen version 2.3.2 PHP v7.0.26
 Max memory 128M, max execution time unlimited on 2018-01-21 01:37:49

   Label                                                         Time        Memory        Peak
---------------------------------------------------------------------------------------------------
 > Calibrate point                                               4.05 μs |    0.00 KB |    2.00 MB
 > mdbm random fetch(number) :: 100,000                        140.18 ms |    0.00 KB |    2.00 MB
 > mdbm random fetch(string) :: 100,000                        161.95 ms |    0.00 KB |    2.00 MB
 > mdbm random fetch(number) :: 1,000,000                        1.44  s |    0.00 KB |    2.00 MB
 > mdbm random fetch(string) :: 1,000,000                        1.63  s |    0.00 KB |    2.00 MB
 > mdbm random fetch(number):: 10,000,000                       15.07  s |    0.00 KB |    2.00 MB
 > mdbm random fetch(number) :: 10,000,000                      16.97  s |    0.00 KB |    2.00 MB
---------------------------------------------------------------------------------------------------
   Total 7 taken                             01-21 01:38:24     35.42  s      0.00 KB      2.00 MB
```


### Simple Random FETCHs with Use the Preload API

```
php src/mdbm_simple_preload_fetch.php -live
```

```
 Created by B. van Hoekelen version 2.3.2 PHP v7.0.26
 Max memory 128M, max execution time unlimited on 2018-01-21 01:39:36

   Label                                                         Time        Memory        Peak
---------------------------------------------------------------------------------------------------
 > Calibrate point                                               4.05 μs |    0.00 KB |    2.00 MB
 > mdbm::preload random fetch(number) :: 100,000               138.52 ms |    0.00 KB |    2.00 MB
 > mdbm::preload random fetch(string) :: 100,000               154.26 ms |    0.00 KB |    2.00 MB
 > mdbm::preload random fetch(number) :: 1,000,000                1.4  s |    0.00 KB |    2.00 MB
 > mdbm::preload random fetch(string) :: 1,000,000               1.58  s |    0.00 KB |    2.00 MB
 > mdbm::preload random fetch(number):: 10,000,000              14.83  s |    0.00 KB |    2.00 MB
 > mdbm::preload random fetch(number) :: 10,000,000             16.73  s |    0.00 KB |    2.00 MB
---------------------------------------------------------------------------------------------------
   Total 7 taken                             01-21 01:40:11     34.83  s      0.00 KB      2.00 MB
```

## SQLite3

### Simple INSERTs

```
php src/sqlite3_simple_store.php -live
```

```
PRAGMA synchronous=OFF;
pragma journal_mode = wal;
Transaction(BEGIN,COMMIT)

 Created by B. van Hoekelen version 2.3.2 PHP v7.0.26
 Max memory 128M, max execution time unlimited on 2018-01-22 00:44:56

   Label                                                         Time        Memory        Peak
---------------------------------------------------------------------------------------------------
 > Calibrate point                                               4.05 μs |    0.00 KB |    2.00 MB
 > sqlite3 insert(number, number) :: 100,000                    761.78 ms |    0.00 KB |    2.00 MB
 > sqlite3 insert(string, string) :: 100,000                      1.17  s |    0.00 KB |    2.00 MB
 > sqlite3 insert(number, number) :: 1,000,000                    7.93  s |    0.00 KB |    2.00 MB
 > sqlite3 insert(string, string) :: 1,000,000                   12.29  s |    0.00 KB |    2.00 MB
 > sqlite3 insert(number, number):: 10,000,000                   80.24  s |    0.00 KB |    2.00 MB
 > sqlite3 insert(string, string) :: 10,000,000                 127.54  s |    0.00 KB |    2.00 MB
---------------------------------------------------------------------------------------------------
   Total 7 taken                             01-22 01:00:25    229.93  s      0.00 KB      2.00 MB
```


### Simple Random FETCHs

```
php src/sqlite3_simple_fetch.php -live
```

```
PRAGMA synchronous=OFF;
pragma journal_mode = wal;

 Created by B. van Hoekelen version 2.3.2 PHP v7.0.26
 Max memory 128M, max execution time unlimited on 2018-01-22 01:23:53

   Label                                                         Time        Memory        Peak
---------------------------------------------------------------------------------------------------
 > Calibrate point                                               4.05 μs |    0.00 KB |    2.00 MB
 > sqlite3 random fetch(number) :: 100,000                       2.16  s |    0.00 KB |    2.00 MB
 > sqlite3 random fetch(string) :: 100,000                       3.16  s |    0.00 KB |    2.00 MB
 > sqlite3 random fetch(number) :: 1,000,000                    25.94  s |    0.00 KB |    2.00 MB
 > sqlite3 random fetch(string) :: 1,000,000                    34.84  s |    0.00 KB |    2.00 MB
 > sqlite3 random fetch(number):: 10,000,000                   275.47  s |    0.00 KB |    2.00 MB
 > sqlite3 random fetch(string) :: 10,000,000                  397.42  s |    0.00 KB |    2.00 MB
---------------------------------------------------------------------------------------------------
   Total 7 taken                             01-22 01:36:12    738.99  s      0.00 KB      2.00 MB
```

## Link

- [Yahoo! MDBM](https://github.com/yahoo/mdbm)
- [MDBM::Concept](http://yahoo.github.io/mdbm/guide/concepts.html)
- [MDBM::Build](https://github.com/yahoo/mdbm/blob/master/README.build)
- [MDBM::Document](http://yahoo.github.io/mdbm/)
- [MDBM::FAQ](http://yahoo.github.io/mdbm/guide/faq.html)
- [DBM](https://en.wikipedia.org/wiki/Dbm)
- [MDBM::Macro(const)](http://yahoo.github.io/mdbm/api/mdbm_8h.html)
- [Packagist](https://packagist.org/packages/torden/php-mdbm)
- [MDBM Packages](https://github.com/torden/mdbm)
- [Go-mdbm](https://github.com/torden/go-mdbm)
- [Py-mdbm](https://github.com/torden/py-mdbm)
- [Upgrading PHP extensions from PHP5 to NG](https://wiki.php.net/phpng-upgrading)

---

*Please feel free. I hope it is helpful for you.*
