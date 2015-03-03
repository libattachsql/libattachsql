sudo apt-get install -y libuv-dev zlib1g-dev clang libssl-dev

CC="clang"
CXX="clang++"
autoreconf -fi
./configure --enable-debug
make
make check 2>/dev/null
