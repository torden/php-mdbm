#!/bin/bash
#/app/php5/bin/phpize --clean
#/app/php5/bin/phpize
./configure --with-php-config=/app/php5/bin/php-config --with-mdbm=/usr/local/mdbm/
make -j2
/app/php5/bin/php -d extension=./modules/mdbm.so z.php
gdb --args /app/php5/bin/php -d extension=./modules/mdbm.so z.php
