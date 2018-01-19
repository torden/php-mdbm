# PHP-mdbm

- PHP-mdbm is a PHP binds to [Yahoo! MDBM C API.](https://github.com/yahoo/mdbm/)
- MDBM is a super-fast memory-mapped key/value store.
- MDBM is an ndbm work-alike hashed database library based on sdbm which is based on Per-Aake Larson’s Dynamic Hashing algorithms.
- MDBM is a high-performance, memory-mapped hash database similar to the homegrown libhash.
- The records stored in a mdbm database may have keys and values of arbitrary and variable lengths.

|Build Stats|PHP-mdbm ver.|License|Y! mdbm ver.|
|:-:|:-:|:-:|:-:|
|[![Build Status](https://travis-ci.org/torden/php-mdbm.svg?branch=master)](https://travis-ci.org/torden/php-mdbm)|[![GitHub version](https://badge.fury.io/gh/torden%2Fphp-mdbm.svg)](https://badge.fury.io/gh/torden%2Fphp-mdbm)| [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)|[![GitHub version](https://badge.fury.io/gh/yahoo%2Fmdbm.svg)](https://badge.fury.io/gh/yahoo%2Fmdbm)|

## Table of Contents

- [API](#api)
- [Install](#install)
- [Example](#example)
- [Link](#link)

## API

Almost Useful APIs Support.

### Not Supported APIs

Unfortunately, the following list are not supported on now.

If you want them, please feel free to raise an issue

|Group|API|
|---|---|
|[Record Iteration](http://yahoo.github.io/mdbm/api/group__RecordIterationGroup.html)|mdbm_iterate|
|[Data Management](http://yahoo.github.io/mdbm/api/group__DataManagementGroup.html)|mdbm_clean, mdbm_prune, mdbm_set_cleanfunc|
|[Statistics](http://yahoo.github.io/mdbm/api/group__StatisticsGroup.html)|mdbm_chunk_iterate|
|[Cache and Backing Store](http://yahoo.github.io/mdbm/api/group__CacheAndBackingStoreGroup.html)|mdbm_set_backingstore|
|[Import and Export](http://yahoo.github.io/mdbm/api/group__ImportExportGroup.html)|mdbm_cdbdump_to_file, mdbm_cdbdump_trailer_and_close, mdbm_cdbdump_add_record, mdbm_dbdump_to_file, mdbm_dbdump_trailer_and_close, mdbm_dbdump_add_record, mdbm_dbdump_export_header, mdbm_dbdump_import_header, mdbm_dbdump_import, mdbm_cdbdump_import|

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

## Support two compatibility version

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

- Ubuntu : See the [pre-build packages for ubuntu](https://github.com/torden/go-mdbm/tree/master/pkg)
- RHEL (CentOS) : See the [documentation for](https://github.com/yahoo/mdbm/blob/master/README.build) build and install

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
make test TESTS='-q -m'
make install
```

## Example

See the [documentation](https://github.com/torden/php-mdbm/blob/master/README.api.md) for more details.

## Link

- [Yahoo! MDBM](https://github.com/yahoo/mdbm)
- [MDBM::Concept](http://yahoo.github.io/mdbm/guide/concepts.html)
- [MDBM::Build](https://github.com/yahoo/mdbm/blob/master/README.build)
- [MDBM::Document](http://yahoo.github.io/mdbm/)
- [MDBM::FAQ](http://yahoo.github.io/mdbm/guide/faq.html)
- [DBM](https://en.wikipedia.org/wiki/Dbm)
- [MDBM::Macro(const)](http://yahoo.github.io/mdbm/api/mdbm_8h.html)
- [Packagist](https://packagist.org/packages/torden/php-mdbm)
- [Go-mdbm](https://github.com/torden/go-mdbm)
- [Py-mdbm](https://github.com/torden/py-mdbm)

---

*Please feel free. I hope it is helpful for you.*
