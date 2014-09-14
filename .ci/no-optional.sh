sudo apt-get install -y libuv-dev
sudo apt-get remove -y zlib1g-dev libssl-dev

autoreconf -fi
./configure --enable-debug
make check
