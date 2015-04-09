sudo apt-get install -y zlib1g-dev valgrind libssl-dev

.ci/install-libuv.sh
autoreconf -fi
./configure --enable-debug
TESTS_ENVIRONMENT="./libtool --mode=execute valgrind --error-exitcode=1 --leak-check=yes --track-fds=no --malloc-fill=A5 --free-fill=DE --suppressions=valgrind.supp" make check 2>/dev/null
