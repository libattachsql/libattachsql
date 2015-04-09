sudo apt-get install -y zlib1g-dev clang libssl-dev

CC="clang"
CXX="clang++"
.ci/install-libuv.sh
autoreconf -fi
./configure --enable-debug
make
make check 2>/dev/null
