#!/bin/bash
/app/php7/bin/phpize --clean
/app/php7/bin/phpize
./configure --with-php-config=/app/php7/bin/php-config --with-mdbm=/usr/local/mdbm/
make 
/app/php7/bin/php -d extension=./modules/mdbm.so z.php
gdb --args /app/php7/bin/php -d extension=./modules/mdbm.so z.php
