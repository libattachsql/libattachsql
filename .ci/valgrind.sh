sudo apt-get install -y libuv-dev zlib1g-dev valgrind

autoreconf -fi
./configure --enable-debug
TESTS_ENVIRONMENT="./libtool --mode=execute valgrind --error-exitcode=1 --leak-check=yes --track-fds=yes --malloc-fill=A5 --free-fill=DE" make check
