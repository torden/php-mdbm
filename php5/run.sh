#!/bin/bash
#/app/php5/bin/phpize --clean
#/app/php5/bin/phpize
make clean
./configure --with-php-config=/app/php5/bin/php-config --with-mdbm=/usr/local/mdbm/
make 
/app/php5/bin/php -d extension=./modules/mdbm.so z.php
gdb --args /app/php5/bin/php -d extension=./modules/mdbm.so z.php
