dnl $Id$
dnl config.m4 for extension mdbm

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(mdbm, for mdbm support,
dnl Make sure that the comment is aligned:
dnl [  --with-mdbm             Include mdbm support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(mdbm, whether to enable mdbm support,
dnl Make sure that the comment is aligned:
dnl [  --enable-mdbm           Enable mdbm support])

if test "$PHP_MDBM" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-mdbm -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/mdbm.h"  # you most likely want to change this
  dnl if test -r $PHP_MDBM/$SEARCH_FOR; then # path given as parameter
  dnl   MDBM_DIR=$PHP_MDBM
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for mdbm files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       MDBM_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$MDBM_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the mdbm distribution])
  dnl fi

  dnl # --with-mdbm -> add include path
  dnl PHP_ADD_INCLUDE($MDBM_DIR/include)

  dnl # --with-mdbm -> check for lib and symbol presence
  dnl LIBNAME=mdbm # you may want to change this
  dnl LIBSYMBOL=mdbm # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MDBM_DIR/$PHP_LIBDIR, MDBM_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_MDBMLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong mdbm lib version or lib not found])
  dnl ],[
  dnl   -L$MDBM_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(MDBM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(mdbm, mdbm.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
