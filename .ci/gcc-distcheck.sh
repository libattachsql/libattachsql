sudo apt-get install -y zlib1g-dev libssl-dev

.ci/install-libuv.sh
autoreconf -fi
./configure --enable-debug
make distcheck 2>/dev/null
