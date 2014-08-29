sudo apt-get install -y libuv-dev zlib1g-dev

autoreconf -fi
./configure --enable-debug
make check
