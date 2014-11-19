# SYNOPSIS
#
#   AX_UNIVERSAL_BINARY()
#
# DESCRIPTION
#
#   --enable-universal-binary
#
# LICENSE
#
#  Copyright 2014 Hewlett-Packard Development Company, L.P.
#  All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#  
#      * Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#  
#      * Redistributions in binary form must reproduce the above
#  copyright notice, this list of conditions and the following disclaimer
#  in the documentation and/or other materials provided with the
#  distribution.
#  
#      * The names of its contributors may not be used to endorse or
#  promote products derived from this software without specific prior
#  written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#serial 1

AC_DEFUN([AX_UNIVERSAL_BINARY],
    [AC_ARG_ENABLE([universal-binary],
                   [AC_HELP_STRING([--enable-universal-binary=auto],
                                   [Apple combined x86 & x86_64 binary support])],,
                   [enable_universal_binary=auto])
    have_universal_binary=no
    if test x"enable_universal_binary" != x"no"; then
        AC_CANONICAL_HOST
        AC_MSG_CHECKING([for universal binary support])
        case $host in *-apple-darwin*)
            save_CFLAGS="$CFLAGS"
            save_CXXFLAGS="$CXXFLAGS"
            save_LDFLAGS="$LDFLAGS"
            CFLAGS="$CFLAGS -arch x86_64 -arch i386"
            CXXFLAGS="$CXXFLAGS -arch x86_64 -arch i386"
            LDFLAGS="$LDFLAGS -arch x86_64 -arch i386"
            AC_LINK_IFELSE([AC_LANG_SOURCE([int main() {return 0;}])],
                          [have_universal_binary=yes])
            if test x"$have_universal_binary" = x"no"; then
                CFLAGS="$save_CFLAGS"
                CXXFLAGS="$save_CXXFLAGS"
                LDFLAGS="$save_LDFLAGS"
            fi
            ;;
        esac
        AC_MSG_RESULT($have_universal_binary)
    fi
    case "$have_universal_binary:$enable_universal_binary" in
        no:yes) AC_MSG_ERROR([no universal binary support on this host]) ;;
        yse:*)  AC_MSG_WARN([disabling dependency tracking])
                AM_CONDITIONAL([AMDEP],[false])
                AM_CONDITIONAL([am__fastdepCC],[false])
                AMDEPBACKSLASH=
                ;;
    esac])
