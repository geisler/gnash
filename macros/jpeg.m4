dnl  
dnl    Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl  
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

dnl  
dnl 

dnl $Id: jpeg.m4,v 1.20 2006/11/04 00:00:30 rsavoye Exp $

AC_DEFUN([GNASH_PATH_JPEG],
[
  AC_ARG_ENABLE(jpeg, AC_HELP_STRING([--enable-jpeg], [Enable support for jpeg images]),
  [case "${enableval}" in
    yes) jpeg=yes ;;
    no)  jpeg=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-jpeg option]) ;;
  esac], jpeg=yes)

  if test x"$jpeg" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(jpeg_incl, AC_HELP_STRING([--with-jpeg-incl], [Directory where libjpeg header is]), with_jpeg_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_jpeg_incl,[
    if test x"${with_jpeg_incl}" != x ; then
      if test -f ${with_jpeg_incl}/jpeglib.h ; then
	ac_cv_path_jpeg_incl=-I`(cd ${with_jpeg_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_jpeg_incl} directory doesn't contain jpeglib.h])
      fi
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_jpeg_incl}" = x; then
      AC_CHECK_HEADERS(jpeglib.h, [ac_cv_path_jpeg_incl=""],[
      if test x"${ac_cv_path_jpeg_incl}" = x; then
        incllist="${prefix}/${target_alias}/include ${prefix}/include /sw/include /opt/local/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/jpeglib.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_jpeg_incl="-I$i"
	      break
            else
	      ac_cv_path_jpeg_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      if test x"${ac_cv_path_jpeg_incl}" != x"/usr/include"; then
	ac_cv_path_jpeg_incl="${ac_cv_path_jpeg_incl}"
       else
	ac_cv_path_jpeg_incl=""
      fi
    fi

    AC_MSG_CHECKING([for libjpeg header])
    AC_MSG_RESULT(${ac_cv_path_jpeg_incl})

    if test x"${ac_cv_path_jpeg_incl}" != x ; then
      JPEG_CFLAGS="${ac_cv_path_jpeg_incl}"
      AC_MSG_RESULT(${ac_cv_path_jpeg_incl})
    else
      JPEG_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(jpeg_lib, AC_HELP_STRING([--with-jpeg-lib], [directory where jpeg library is]), with_jpeg_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_jpeg_lib,[
      if test x"${with_jpeg_lib}" != x ; then
        if test -f ${with_jpeg_lib}/libjpeg.a -o -f ${with_jpeg_lib}/libjpeg.so; then
	  ac_cv_path_jpeg_lib=`(cd ${with_jpeg_lib}; pwd)`
        else
	  AC_MSG_ERROR([${with_jpeg_lib} directory doesn't contain libjpeg.])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_jpeg_incl}" = x; then
        AC_CHECK_LIB(jpeg, jpeg_mem_init, [ac_cv_path_jpeg_lib=""],[
          libslist="${prefix}/${target_alias}/lib ${prefix}/lib64 ${prefix}/lib32 ${prefix}/lib /usr/lib64 /usr/lib32 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libjpeg.a -o -f $i/libjpeg.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_jpeg_lib="-L$i"
	        break
              else
	        ac_cv_path_jpeg_lib=""
	        break
	      fi
	    fi
          done])
      else
        if test -f ${ac_cv_path_jpeg_lib}/libjpeg.a -o -f ${ac_cv_path_jpeg_lib}/libjpeg.so; then

          if test x"${ac_cv_path_jpeg_lib}" != x"/usr/lib"; then
	    ac_cv_path_jpeg_lib="-L${ac_cv_path_jpeg_lib}"
           else
	    ac_cv_path_jpeg_lib=""
          fi
        fi
      fi
	AC_MSG_CHECKING([for libjpeg library])
	AC_MSG_RESULT(${ac_cv_path_jpeg_lib})

      if test x"${ac_cv_path_jpeg_lib}" != x ; then
        JPEG_LIBS="${ac_cv_path_jpeg_lib}"
      else
        JPEG_LIBS=""
      fi
    fi

  if test x"${ac_cv_path_jpeg_lib}" != x ; then
      JPEG_LIBS="${ac_cv_path_jpeg_lib} -ljpeg"
  else
      JPEG_LIBS="-ljpeg"
  fi

  AM_CONDITIONAL(HAVE_JPEG, [test x$jpeg = xyes])

  AC_SUBST(JPEG_CFLAGS)
  AC_SUBST(JPEG_LIBS)
])
