sudo apt-get install -y libuv-dev zlib1g-dev clang-3.4 libssl-dev

CC="clang"
CXX="clang++"
autoreconf -fi
./configure --enable-debug
make check
