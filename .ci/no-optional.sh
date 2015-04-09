sudo apt-get remove -y zlib1g-dev libssl-dev

.ci/install-libuv.sh
autoreconf -fi
./configure --enable-debug
make
make check 2>/dev/null
