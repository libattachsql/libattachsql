sudo apt-get install -y libuv-dev zlib1g-dev libssl-dev

autoreconf -fi
./configure --enable-debug
make distcheck
