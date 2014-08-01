# configure.ac -*- autoconf -*- vim: filetype=config
#
# DDM4
# Copyright (C) 2012 Data Differential, http://datadifferential.com/
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.

AC_COPYRIGHT([2014 Hewlett-Packard Development Company, L.P.])
#AC_REVISION([m4_esyscmd_s([git describe --always])])
AC_PREREQ([2.69])

# NOTE: Major version should be < 100 and minor/patch < 256 or bad things will
# happen in src/asql/utility.cc
AC_INIT([libattachsql],[0.1.0],[andrew@linuxjedi.co.uk],[ddm4],[https://github.com/HPATG/libattachsql])

# Setup the compilers early on
AC_PROG_CC([cc gcc clang])
AC_PROG_CXX([c++ g++ clang++])

# Must come before AM_INIT_AUTOMAKE.
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])

# Automake version before 1.13 (when the serial-tests option was dnl still the default) still defined the badly obsolete macro.  'AC_PROG_INSTALL'.
AM_INIT_AUTOMAKE(1.11 no-define color-tests -Wno-portability subdir-objects foreign tar-ustar m4_ifndef([AC_PROG_INSTALL], [serial-tests]))

AC_ARG_PROGRAM
AC_DEFUN([PROTECT_AC_USE_SYSTEM_EXTENSIONS],
         [AX_SAVE_FLAGS
          AC_LANG_PUSH([C++])
          AC_USE_SYSTEM_EXTENSIONS
          AC_LANG_POP([C++])
          AX_RESTORE_FLAGS
          ])
PROTECT_AC_USE_SYSTEM_EXTENSIONS

AC_CONFIG_HEADERS([config.h:config.in])dnl Keep filename to 8.3 for MS-DOS.
AC_CONFIG_SRCDIR([bin/cflags.c])

# shared library versioning
LIBATTACHSQL_LIBRARY_VERSION=1:0:0
#                            | | |
#                     +------+ | +---+
#                     |        |     |
#                  current:revision:age
#                     |        |     |
#                     |        |     +- increment if interfaces have been added
#                     |        |        set to zero if interfaces have been removed or changed
#                     |        +- increment if source code has changed
#                     |           set to zero if current is incremented
#                     +- increment if interfaces have been added, removed or changed
AC_SUBST([LIBATTACHSQL_LIBRARY_VERSION])

LT_PREREQ([2.2])
LT_INIT
LT_LANG([C++])

AC_PROG_CC_C99
AS_IF([test "x${ac_cv_prog_cc_c99}" == "xno"],[AC_MSG_ERROR([No c99 compatible compiler found])])

AX_PLATFORM

CONFIG_EXTRA

AC_DEFUN([CHECK_FOR_MINGW],
         [AS_IF([test "x${TARGET_WINDOWS}" == "xtrue"],
               [AX_APPEND_LINK_FLAGS([-static-libstdc++],[LDFLAGS])
               AX_APPEND_LINK_FLAGS([-lmingw64],[LIBS])
               AX_APPEND_LINK_FLAGS([-lssp],[LIBS])
               ])
         ])
CHECK_FOR_MINGW

# Specialty checks
AX_CXX_CINTTYPES

# Checks for programs.
AC_CHECK_PROGS([LSB_RELEASE],[lsb_release])
AC_CHECK_PROGS([DPKG_GENSYMBOLS],[dpkg-gensymbols])
AC_CHECK_PROGS([PERL],[perl])
AC_CHECK_PROGS([WINE],['wine'])
AX_PROG_SPHINX_BUILD(,[AC_MSG_WARN([sphinx-build version 1.0 or greater is required to build man pages])])
#AX_PROG_VALGRIND([memcheck],[--error-exitcode=1 --leak-check=yes --track-fds=yes --malloc-fill=A5 --free-fill=DE --suppressions=valgrind.supp])
AX_WITH_PROG([LCOV],[lcov])
AX_WITH_PROG([LCOV_GENHTML],[genhtml])
AC_CHECK_PROGS([RPMBUILD],[rpmbuild --nocheck])
AC_CHECK_PROGS([RPMDEV_SETUPTREE],[rpmdev-setuptree])
AC_CHECK_PROGS([RPM],[rpm])

# Checks for libraries.
AX_CXX_GCC_ABI_DEMANGLE

# Checks for header files.
AC_CHECK_HEADERS_ONCE([features.h])

#  We use the header to cross test our m4 rules that also test
AC_DEFUN([CHECK_FOR_CXXABI],
         [AC_LANG_PUSH([C++])
         AS_IF([test "x${TARGET_WINDOWS}" != "xtrue"],
               [AC_CHECK_HEADERS([cxxabi.h])])
         AC_LANG_POP
         ])
CHECK_FOR_CXXABI

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AX_PRINTF_STRERROR

AX_PTHREAD_TIMEDJOIN_NP

#AX_LIBUV(, [AC_MSG_ERROR(could not find libuv)])
AX_CHECK_LIBRARY([LIBUV], [uv.h], [uv], [], [AC_MSG_ERROR(could not find libuv)])
#AX_CHECK_ZLIB
AX_CHECK_OPENSSL([ax_cv_openssl=yes
                  AC_DEFINE([HAVE_OPENSSL], [1], [Define to 1 if you have OpenSSL])
                  ], [
                  ax_cv_openssl=no
                  AC_MSG_WARN(could not find openssl)])
AX_ENDIAN
AX_HEX_VERSION([LIBATTACHSQL],[$VERSION])
AC_SUBST([RPM_RELEASE],[1])

AX_HARDEN_COMPILER_FLAGS

AC_DEFINE_UNQUOTED([C_COMPILER_VENDOR],["$ax_cv_c_compiler_vendor"],[Compiler vendor])
AC_DEFINE_UNQUOTED([CXX_COMPILER_VENDOR],["$ax_cv_cxx_compiler_vendor"],[Compiler vendor])
AC_DEFINE_UNQUOTED([CC],["$CC"],[Compiler information for CC compiler])
AC_DEFINE_UNQUOTED([CXX],["$CXX"],[Compiler information for CXX compiler])
AC_DEFINE_UNQUOTED([CC_VERSION],["$CC_VERSION"],[Version information for CC compiler])
AC_DEFINE_UNQUOTED([CXX_VERSION],["$CXX_VERSION"],[Version information for CXX compiler])
AC_DEFINE_UNQUOTED([CFLAGS],["$CFLAGS"],[CFLAGS used for compiling binary])
AC_DEFINE_UNQUOTED([CXXFLAGS],["$CXXFLAGS"],[CXXFLAGS used for compiling binary])

AS_IF([test -n "${LSB_RELEASE}"],[
       LSB_DESCRIPTION="`lsb_release -d -s`"
       ],[LSB_DESCRIPTION="unknown"])

AX_AM_JOBSERVER([yes])

AC_CONFIG_FILES([Makefile
                 docs/conf.py
                 rpm/spec
                 version.h
                 ])

AC_OUTPUT

echo "---"
echo "Configuration summary for $PACKAGE_NAME version $VERSION"
echo ""
echo "   * Installation prefix:       $prefix"
echo "   * LSB Release:               $LSB_RELEASE"
echo "   * System type:               $host_vendor-$host_os"
echo "   * Host CPU:                  $host_cpu"
echo "   * Compiler Vendor:           $ax_cv_c_compiler_vendor"
echo "   * C Compiler:                $CC"
echo "   * C Compiler Version:        $ax_c_compiler_version"
echo "   * C Flags:                   $CFLAGS"
echo "   * C++ Compiler:              $CXX"
echo "   * C++ Version:               $ax_cxx_compiler_version"
echo "   * C++ Flags:                 $CXXFLAGS"
echo "   * CPP Flags:                 $CPPFLAGS"
echo "   * LIBS Flags:                $LIBS"
echo "   * LDFLAGS Flags:             $LDFLAGS"
echo "   * Assertions enabled:        $ax_enable_assert"
echo "   * Debug enabled:             $ax_enable_debug"
echo "   * Warnings as failure:       $ac_cv_warnings_as_errors"
echo "   * libuv:                     $ax_cv_libuv"
echo "   * zlib:                      $zlib_cv_zlib_h"
echo "   * openssl:                   $ax_cv_openssl"
echo "   * make -j:                   $enable_jobserver"
echo "   * VCS checkout:              $ac_cv_vcs_system"
echo ""
echo "---"
