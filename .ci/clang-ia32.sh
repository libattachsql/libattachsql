sudo apt-get install -y libc6-dev libc6-dev:i386 zlib1g-dev:i386 build-essential g++ gcc libtool libssl-dev:i386

sudo apt-get install -y gcc-multilib g++-multilib clang

export CC="clang"
export CXX="clang++"
export CFLAGS="-m32"
export CXXFLAGS="-m32"
export CPPFLAGS="-m32"
export LDFLAGS="-m32"
.ci/install-libuv.sh
autoreconf -fi
./configure --enable-debug
make
make check 2>/dev/null
