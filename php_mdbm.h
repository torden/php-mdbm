/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_MDBM_H
#define PHP_MDBM_H

extern zend_module_entry mdbm_module_entry;
#define phpext_mdbm_ptr &mdbm_module_entry

#define PHP_MDBM_VERSION "0.0.1" 

#ifdef PHP_WIN32
#	define PHP_MDBM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_MDBM_API __attribute__ ((visibility("default")))
#else
#	define PHP_MDBM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(mdbm);
PHP_MSHUTDOWN_FUNCTION(mdbm);
PHP_RINIT_FUNCTION(mdbm);
PHP_RSHUTDOWN_FUNCTION(mdbm);
PHP_MINFO_FUNCTION(mdbm);

PHP_FUNCTION(mdbm_open);
PHP_FUNCTION(mdbm_close);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(mdbm)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(mdbm)
*/


#ifdef ZTS
#define MDBM_G(v) TSRMG(mdbm_globals_id, zend_mdbm_globals *, v)
#else
#define MDBM_G(v) (mdbm_globals.v)
#endif

#endif	/* PHP_MDBM_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
