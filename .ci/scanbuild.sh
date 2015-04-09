sudo apt-get install -y zlib1g-dev clang-3.4 libssl-dev

CC="clang"
CXX="clang++"
.ci/install-libuv.sh
autoreconf -fi
./configure --enable-debug
scan-build --use-cc=clang --use-c++=clang++ --status-bugs make
